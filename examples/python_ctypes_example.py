#!/usr/bin/env python3
"""
Python ctypes example for the Merkle tree C library.

This module demonstrates how to use Python's ctypes library to interface
with the compiled Merkle tree shared library. It provides a Python wrapper
around the C functions for creating trees, computing hashes, and managing memory.

Requirements:
    - libmerkle.so compiled in the parent directory
    - Python 3.x with ctypes (standard library)

Example:
    $ python3 python_ctypes_example.py
    Root hash: a1554130b3c21a2ae8884cdb7a4993a337ad1aed4d1dcffece16a590899a80eb
"""

import ctypes
import os
from typing import List, Optional

# ==============================================================================
# LIBRARY LOADING
# ==============================================================================

# Load the shared library
# Construct path to libmerkle.so in the parent directory
lib_path = os.path.join(os.path.dirname(__file__), '..', 'libmerkle.so')

# Validate library exists before loading
if not os.path.exists(lib_path):
    raise FileNotFoundError(f"Merkle library not found at: {lib_path}")

# Load the C library using ctypes.CDLL
merkle = ctypes.CDLL(lib_path)

# ==============================================================================
# FUNCTION SIGNATURE DEFINITIONS
# ==============================================================================

# Define argument and return types for all C functions we'll use
# This is critical for ctypes to properly marshal data between Python and C

# merkle_tree_t* create_merkle_tree(const void **data, const size_t *sizes, 
#                                   size_t count, size_t branching_factor)
merkle.create_merkle_tree.restype = ctypes.c_void_p  # Returns opaque pointer to merkle_tree_t
merkle.create_merkle_tree.argtypes = [
    ctypes.POINTER(ctypes.c_void_p),  # const void **data - array of data pointers
    ctypes.POINTER(ctypes.c_size_t),  # const size_t *sizes - array of data sizes
    ctypes.c_size_t,                  # size_t count - number of data blocks
    ctypes.c_size_t                   # size_t branching_factor - tree branching factor
]

# void dealloc_merkle_tree(merkle_tree_t *tree)
merkle.dealloc_merkle_tree.restype = None           # void return
merkle.dealloc_merkle_tree.argtypes = [ctypes.c_void_p]  # merkle_tree_t *tree

# merkle_error_t get_tree_hash(const merkle_tree_t *tree, unsigned char copy_into[HASH_SIZE])
merkle.get_tree_hash.restype = ctypes.c_int         # Returns merkle_error_t (enum)
merkle.get_tree_hash.argtypes = [
    ctypes.c_void_p,        # const merkle_tree_t *tree
    ctypes.c_char * 32      # unsigned char copy_into[32] - SHA-256 hash buffer
]

# ==============================================================================
# CONSTANTS
# ==============================================================================

# Merkle tree error codes (from Merkle.h)
MERKLE_SUCCESS = 0
MERKLE_NULL_ARG = -1
MERKLE_BAD_ARG = -2
MERKLE_BAD_LEN = -3
MERKLE_FAILED_MEM_ALLOC = -4
MERKLE_FAILED_TREE_BUILD = -5
MERKLE_PROOF_INVALID = -6

# Hash size in bytes (SHA-256)
HASH_SIZE = 32

# ==============================================================================
# PYTHON WRAPPER FUNCTIONS
# ==============================================================================

def build_tree(data: List[bytes], branching_factor: int = 2) -> ctypes.c_void_p:
    """
    Build a Merkle tree from a list of byte strings.
    
    This function wraps the C create_merkle_tree function, handling the complex
    memory management and type conversions required by ctypes.
    
    Args:
        data: List of byte strings to include as leaf nodes in the tree.
              Each element should be bytes or bytearray.
        branching_factor: Number of children each internal node can have.
                         Default is 2 (binary tree).
    
    Returns:
        ctypes.c_void_p: Opaque pointer to the created merkle_tree_t structure.
                        This pointer must be passed to dealloc_merkle_tree() when done.
    
    Raises:
        RuntimeError: If tree creation fails (e.g., out of memory, invalid data).
        TypeError: If data contains non-bytes elements.
    
    Example:
        >>> tree = build_tree([b'hello', b'world', b'merkle', b'tree'])
        >>> # Use tree for operations...
        >>> merkle.dealloc_merkle_tree(tree)  # Clean up when done
    """
    # Validate input
    if not data:
        raise ValueError("Data list cannot be empty")
    
    # Ensure all elements are bytes
    for i, block in enumerate(data):
        if not isinstance(block, (bytes, bytearray)):
            raise TypeError(f"Element {i} must be bytes or bytearray, got {type(block)}")
    
    count = len(data)
    
    # Create ctypes array types for the required C arrays
    # These correspond to the C function signature:
    # create_merkle_tree(const void **data, const size_t *sizes, size_t count, size_t branching_factor)
    arr_type = ctypes.c_void_p * count    # Array of void pointers for data
    size_type = ctypes.c_size_t * count   # Array of size_t for data lengths

    # Instantiate the arrays
    data_ptrs = arr_type()   # Will hold pointers to each data block
    sizes = size_type()      # Will hold the size of each data block

    # Convert Python bytes objects to C-compatible format
    buffers = []  # Keep references to prevent garbage collection
    for i, block in enumerate(data):
        # Create a C string buffer that contains the data
        # This allocates memory that C can access
        buf = ctypes.create_string_buffer(block)
        buffers.append(buf)  # Prevent Python from garbage collecting
        
        # Cast the buffer to a void pointer and store in our array
        data_ptrs[i] = ctypes.cast(buf, ctypes.c_void_p)
        sizes[i] = len(block)

    # Call the C function to create the Merkle tree
    # The C function will copy the data, so our buffers can be freed after this call
    tree = merkle.create_merkle_tree(data_ptrs, sizes, count, branching_factor)
    
    # Check if tree creation succeeded (NULL pointer indicates failure)
    if not tree:
        raise RuntimeError('Failed to create Merkle tree - likely out of memory or invalid parameters')
    
    return tree


def get_root_hash(tree: ctypes.c_void_p) -> bytes:
    """
    Get the root hash of a Merkle tree.
    
    Args:
        tree: Opaque pointer to merkle_tree_t returned by build_tree().
    
    Returns:
        bytes: 32-byte SHA-256 hash of the tree root.
    
    Raises:
        RuntimeError: If hash retrieval fails.
        ValueError: If tree pointer is invalid.
    
    Example:
        >>> tree = build_tree([b'data1', b'data2'])
        >>> root_hash = get_root_hash(tree)
        >>> print(f"Root hash: {root_hash.hex()}")
    """
    if not tree:
        raise ValueError("Tree pointer cannot be null")
    
    # Create a buffer to receive the hash
    # SHA-256 produces 32-byte hashes
    hash_buf = (ctypes.c_char * HASH_SIZE)()
    
    # Call the C function to get the hash
    result = merkle.get_tree_hash(tree, hash_buf)
    
    # Check if the operation succeeded
    if result != MERKLE_SUCCESS:
        error_messages = {
            MERKLE_NULL_ARG: "Null argument passed to get_tree_hash",
            MERKLE_BAD_ARG: "Invalid tree or tree not properly constructed"
        }
        error_msg = error_messages.get(result, f"Unknown error code: {result}")
        raise RuntimeError(f"Failed to get tree hash: {error_msg}")
    
    # Convert the C char array to Python bytes
    return bytes(hash_buf.raw)


def cleanup_tree(tree: ctypes.c_void_p) -> None:
    """
    Clean up a Merkle tree and free all associated memory.
    
    This function MUST be called for every tree created with build_tree()
    to prevent memory leaks.
    
    Args:
        tree: Opaque pointer to merkle_tree_t returned by build_tree().
              After calling this function, the pointer becomes invalid.
    
    Note:
        The tree pointer becomes invalid after this call and should not be used again.
    
    Example:
        >>> tree = build_tree([b'data'])
        >>> # ... use tree ...
        >>> cleanup_tree(tree)  # Always clean up!
    """
    if tree:
        merkle.dealloc_merkle_tree(tree)


# ==============================================================================
# EXAMPLE USAGE
# ==============================================================================

if __name__ == '__main__':
    """
    Example demonstrating basic Merkle tree usage.
    
    This example:
    1. Creates a binary Merkle tree from sample data
    2. Computes and displays the root hash
    3. Properly cleans up memory
    """
    print("Merkle Tree Python ctypes Example")
    print("=================================")
    
    # Sample data for the tree
    sample_data = [b'foo', b'bar', b'baz']
    print(f"Input data: {[data.decode() for data in sample_data]}")
    
    try:
        # Create the Merkle tree
        print("\nCreating Merkle tree...")
        tree = build_tree(sample_data, branching_factor=2)
        print("✓ Tree created successfully")
        
        # Get the root hash
        print("\nComputing root hash...")
        root_hash = get_root_hash(tree)
        print(f"✓ Root hash: {root_hash.hex()}")
        
        # Clean up memory
        print("\nCleaning up...")
        cleanup_tree(tree)
        print("✓ Memory cleaned up")
        
    except Exception as e:
        print(f"✗ Error: {e}")
        
    print("\nExample completed!")
