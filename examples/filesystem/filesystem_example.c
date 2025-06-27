#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../include/Merkle.h"

/* Helper function to read an entire file into memory */
static int read_file(const char *path, unsigned char **buffer, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    struct stat st;
    if (stat(path, &st) != 0) { fclose(f); return 0; }
    *size = st.st_size;
    *buffer = malloc(*size);
    if (!*buffer) { fclose(f); return 0; }
    if (fread(*buffer, 1, *size, f) != *size) { fclose(f); free(*buffer); return 0; }
    fclose(f);
    return 1;
}

int main(void) {
    const char *dir = "data";
    DIR *d = opendir(dir);
    if (!d) {
        perror("opendir");
        return 1;
    }

    size_t cap = 4;
    size_t count = 0;
    unsigned char **data = malloc(cap * sizeof(*data));
    size_t *sizes = malloc(cap * sizeof(*sizes));
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (ent->d_type != DT_REG) continue;
        if (count == cap) {
            cap *= 2;
            data = realloc(data, cap * sizeof(*data));
            sizes = realloc(sizes, cap * sizeof(*sizes));
        }
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if (!read_file(path, &data[count], &sizes[count])) {
            fprintf(stderr, "Failed to read %s\n", path);
            closedir(d);
            return 1;
        }
        count++;
    }
    closedir(d);

    if (count == 0) {
        fprintf(stderr, "No files found\n");
        return 1;
    }

    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, count, 2);
    if (!tree) {
        fprintf(stderr, "Failed to build Merkle tree\n");
        return 1;
    }

    unsigned char root[HASH_SIZE];
    if (get_tree_hash(tree, root) != MERKLE_SUCCESS) {
        fprintf(stderr, "Failed to get root hash\n");
        dealloc_merkle_tree(tree);
        return 1;
    }

    printf("Root hash: ");
    for (size_t i = 0; i < HASH_SIZE; ++i) printf("%02x", root[i]);
    printf("\n");

    /* Generate and verify proof for the second file (index 1) */
    if (count > 1) {
        merkle_proof_t *proof = NULL;
        if (generate_proof_from_index(tree, 1, &proof) == MERKLE_SUCCESS) {
            if (verify_proof(proof, root, data[1], sizes[1]) == MERKLE_SUCCESS) {
                printf("Proof for file 2 verified successfully!\n");
            } else {
                printf("Verification failed!\n");
            }
        } else {
            printf("Failed to generate proof for file 2\n");
        }
    }

    dealloc_merkle_tree(tree);
    for (size_t i = 0; i < count; ++i) free(data[i]);
    free(data);
    free(sizes);
    return 0;
}

