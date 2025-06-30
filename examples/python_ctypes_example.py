import ctypes
import os

# Load the shared library
lib_path = os.path.join(os.path.dirname(__file__), '..', 'libmerkle.so')
merkle = ctypes.CDLL(lib_path)

# Define argument and return types
merkle.create_merkle_tree.restype = ctypes.c_void_p
merkle.create_merkle_tree.argtypes = [ctypes.POINTER(ctypes.c_void_p),
                                      ctypes.POINTER(ctypes.c_size_t),
                                      ctypes.c_size_t,
                                      ctypes.c_size_t]

merkle.dealloc_merkle_tree.restype = None
merkle.dealloc_merkle_tree.argtypes = [ctypes.c_void_p]

merkle.get_tree_hash.restype = ctypes.c_int
merkle.get_tree_hash.argtypes = [ctypes.c_void_p,
                                 ctypes.c_char * 32]

def build_tree(data):
    count = len(data)
    arr_type = ctypes.c_void_p * count
    size_type = ctypes.c_size_t * count

    data_ptrs = arr_type()
    sizes = size_type()

    for i, block in enumerate(data):
        buf = ctypes.create_string_buffer(block)
        data_ptrs[i] = ctypes.cast(buf, ctypes.c_void_p)
        sizes[i] = len(block)

    tree = merkle.create_merkle_tree(data_ptrs, sizes, count, 2)
    if not tree:
        raise RuntimeError('Failed to create Merkle tree')
    return tree

if __name__ == '__main__':
    tree = build_tree([b'foo', b'bar', b'baz'])
    hash_buf = (ctypes.c_char * 32)()
    if merkle.get_tree_hash(tree, hash_buf) == 0:
        print('Root hash:', hash_buf.raw.hex())
    merkle.dealloc_merkle_tree(tree)
