#ifndef _MERKLE_H_
#define _MERKLE_H_
#include <stddef.h>

#define MMalloc(x) Merkle_Malloc(x, __FILE__, __LINE__)
#define MFree(x) Merkle_Free(x, __FILE__, __LINE__)

typedef enum {
  Success = 0,
  E_NULL_ARG = 1,
  E_BAD_LEN = 2,
  E_FAILED_TREE_BUILD = 3
} ErrorCode;

typedef ErrorCode M_Error;

struct MerkleTree;
struct MerkleTree *merkle_tree_create(const void **data, const size_t *size,
                                      unsigned int count);
void merkle_free_destroy(struct MerkleTree *tree);

#endif // _MERKLE_H_