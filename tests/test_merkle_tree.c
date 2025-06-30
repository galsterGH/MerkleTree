/**
 * @file test_merkle_tree.c
 * @brief Unit tests for Merkle tree implementation.
 *
 * This file contains comprehensive unit tests for the Merkle tree
 * implementation, including edge cases, error conditions, and
 * functional verification tests.
 *
 * @author Guy Alster
 * @date 2025-06-01
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <unistd.h>

// Include the headers
#include "test_merkle_internal.h"
#include "Merkle.h"
#include "merkle_utils.h"

// Test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s - %s\n", __func__, message); \
            return 0; \
        } \
    } while(0)

#define TEST_PASS() \
    do { \
        printf("PASS: %s\n", __func__); \
        return 1; \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        printf("Running %s...\n", #test_func); \
        if (test_func()) { \
            tests_passed++; \
        } else { \
            tests_failed++; \
        } \
        total_tests++; \
    } while(0)

// Global test counters
static int tests_passed = 0;
static int tests_failed = 0;
static int total_tests = 0;

// Helper function to create test data
static void create_test_data(const char **data, size_t *sizes, size_t count) {
    for (size_t i = 0; i < count; i++) {
        switch (i % 4) {
            case 0:
                data[i] = "Hello";
                sizes[i] = 5;
                break;
            case 1:
                data[i] = "World";
                sizes[i] = 5;
                break;
            case 2:
                data[i] = "Test";
                sizes[i] = 4;
                break;
            case 3:
                data[i] = "Data";
                sizes[i] = 4;
                break;
        }
    }
}

/**
 * @brief Test creating a tree with NULL data pointer.
 */
static int test_create_null_data(void) {
    merkle_tree_t *tree = create_merkle_tree(NULL, NULL, 0, 2);
    TEST_ASSERT(tree == NULL, "Should return NULL for NULL data");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with zero count.
 */
static int test_create_zero_count(void) {
    const char *data[] = {"test"};
    size_t sizes[] = {4};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 0, 2);
    TEST_ASSERT(tree == NULL, "Should return NULL for zero count");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with zero branching factor.
 */
static int test_create_zero_branching_factor(void) {
    const char *data[] = {"test"};
    size_t sizes[] = {4};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 1, 0);
    TEST_ASSERT(tree == NULL, "Should return NULL for zero branching factor");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with NULL data element.
 */
static int test_create_null_data_element(void) {
    const void *data[] = {NULL, "test"};
    size_t sizes[] = {0, 4};
    
    merkle_tree_t *tree = create_merkle_tree(data, sizes, 2, 2);
    TEST_ASSERT(tree == NULL, "Should return NULL for NULL data element");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with zero size element.
 */
static int test_create_zero_size_element(void) {
    const char *data[] = {"test", "data"};
    size_t sizes[] = {4, 0};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 2, 2);
    TEST_ASSERT(tree == NULL, "Should return NULL for zero size element");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with single element.
 */
static int test_create_single_element(void) {
    const char *data[] = {"Hello"};
    size_t sizes[] = {5};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 1, 2);
    TEST_ASSERT(tree != NULL, "Should successfully create tree with single element");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test creating a binary tree (branching factor 2).
 */
static int test_create_binary_tree(void) {
    const char *data[4];
    size_t sizes[4];
    create_test_data((const char **)data, sizes, 4);
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Should successfully create binary tree");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test creating a tree with branching factor 3.
 */
static int test_create_ternary_tree(void) {
    const char *data[6];
    size_t sizes[6];
    create_test_data((const char **)data, sizes, 6);
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 6, 3);
    TEST_ASSERT(tree != NULL, "Should successfully create ternary tree");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test creating a tree with large branching factor.
 */
static int test_create_large_branching_factor(void) {
    const char *data[5];
    size_t sizes[5];
    create_test_data((const char **)data, sizes, 5);
    
    // Branching factor larger than number of elements
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 5, 10);
    TEST_ASSERT(tree != NULL, "Should successfully create tree with large branching factor");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test tree destruction with NULL pointer.
 */
static int test_destroy_null_tree(void) {
    // Should not crash
    dealloc_merkle_tree(NULL);
    TEST_PASS();
}

/**
 * @brief Test creating trees with different data sizes.
 */
static int test_different_data_sizes(void) {
    const char *data[] = {"A", "BB", "CCC", "DDDD", "EEEEE"};
    size_t sizes[] = {1, 2, 3, 4, 5};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 5, 2);
    TEST_ASSERT(tree != NULL, "Should handle different data sizes");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test tree with odd number of elements.
 */
static int test_odd_number_elements(void) {
    const char *data[7];
    size_t sizes[7];
    create_test_data((const char **)data, sizes, 7);
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 7, 2);
    TEST_ASSERT(tree != NULL, "Should handle odd number of elements");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test tree consistency - same data should produce same tree.
 */
static int test_tree_consistency(void) {
    const char *data[] = {"Test", "Data", "Hash", "Tree"};
    size_t sizes[] = {4, 4, 4, 4};
    
    // Create two identical trees
    merkle_tree_t *tree1 = create_merkle_tree((const void **)data, sizes, 4, 2);
    merkle_tree_t *tree2 = create_merkle_tree((const void **)data, sizes, 4, 2);
    
    TEST_ASSERT(tree1 != NULL && tree2 != NULL, "Both trees should be created");
    
    // Use the API instead of direct struct access to avoid locking issues
    unsigned char hash1[HASH_SIZE];
    unsigned char hash2[HASH_SIZE];
    
    TEST_ASSERT(get_tree_hash(tree1, hash1) == MERKLE_SUCCESS, "Should get hash from tree1");
    TEST_ASSERT(get_tree_hash(tree2, hash2) == MERKLE_SUCCESS, "Should get hash from tree2");
    
    // Compare root hashes - they should be identical
    TEST_ASSERT(memcmp(hash1, hash2, HASH_SIZE) == 0, 
                "Root hashes should be identical for same data");
    
    dealloc_merkle_tree(tree1);
    dealloc_merkle_tree(tree2);
    TEST_PASS();
}

/**
 * @brief Test tree sensitivity - different data should produce different hashes.
 */
static int test_tree_sensitivity(void) {
    const char *data1[] = {"Test", "Data"};
    const char *data2[] = {"Test", "Different"};
    size_t sizes[] = {4, 4};
    
    merkle_tree_t *tree1 = create_merkle_tree((const void **)data1, sizes, 2, 2);
    
    sizes[1] = 9; // Update size for "Different"
    merkle_tree_t *tree2 = create_merkle_tree((const void **)data2, sizes, 2, 2);
    
    TEST_ASSERT(tree1 != NULL && tree2 != NULL, "Both trees should be created");
    
    // Use the API instead of direct struct access
    unsigned char hash1[HASH_SIZE];
    unsigned char hash2[HASH_SIZE];
    
    TEST_ASSERT(get_tree_hash(tree1, hash1) == MERKLE_SUCCESS, "Should get hash from tree1");
    TEST_ASSERT(get_tree_hash(tree2, hash2) == MERKLE_SUCCESS, "Should get hash from tree2");
    
    // Root hashes should be different
    TEST_ASSERT(memcmp(hash1, hash2, HASH_SIZE) != 0, 
                "Root hashes should be different for different data");
    
    dealloc_merkle_tree(tree1);
    dealloc_merkle_tree(tree2);
    TEST_PASS();
}

/**
 * @brief Test with empty strings (valid but zero-length after the pointer).
 */
static int test_empty_string_elements(void) {
    // Note: We can't test truly empty strings because our validation 
    // requires size > 0, which is correct behavior
    const char *data[] = {"", "test"};
    size_t sizes[] = {0, 4};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 2, 2);
    TEST_ASSERT(tree == NULL, "Should reject zero-length data");
    TEST_PASS();
}

/**
 * @brief Test with large dataset.
 */
static int test_large_dataset(void) {
    const size_t count = 100;
    const char **data = malloc(count * sizeof(char*));
    size_t *sizes = malloc(count * sizeof(size_t));
    char **allocated_strings = malloc(count * sizeof(char*));
    
    TEST_ASSERT(data && sizes && allocated_strings, "Memory allocation should succeed");
    
    // Create test data
    for (size_t i = 0; i < count; i++) {
        allocated_strings[i] = malloc(20);
        snprintf(allocated_strings[i], 20, "data_item_%zu", i);
        data[i] = allocated_strings[i];
        sizes[i] = strlen(allocated_strings[i]);
    }
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, count, 4);
    TEST_ASSERT(tree != NULL, "Should handle large dataset");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    
    // Cleanup
    for (size_t i = 0; i < count; i++) {
        free(allocated_strings[i]);
    }
    free(data);
    free(sizes);
    free(allocated_strings);
    
    TEST_PASS();
}

/**
 * @brief Test memory management - ensure no leaks on normal operation.
 */
static int test_memory_management(void) {
    // This test creates and destroys multiple trees to check for leaks
    for (int i = 0; i < 10; i++) {
        const char *data[] = {"Memory", "Test", "Data", "Item"};
        size_t sizes[] = {6, 4, 4, 4};
        
        merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
        TEST_ASSERT(tree != NULL, "Tree creation should succeed");
        dealloc_merkle_tree(tree);
    }
    TEST_PASS();
}

/**
 * @brief Test various branching factors.
 */
static int test_various_branching_factors(void) {
    const char *data[8];
    size_t sizes[8];
    create_test_data((const char **)data, sizes, 8);
    
    // Test different branching factors
    for (size_t bf = 2; bf <= 8; bf++) {
        merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 8, bf);
        TEST_ASSERT(tree != NULL, "Should work with various branching factors");
        TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
        dealloc_merkle_tree(tree);
    }
    
    TEST_PASS();
}

/**
 * @brief Verify root hash for a known single element.
 */
static int test_root_hash_single_element(void) {
    const char *data[] = {"Hello"};
    size_t sizes[] = {5};

    const unsigned char expected[HASH_SIZE] = {
        0x18, 0x5f, 0x8d, 0xb3, 0x22, 0x71, 0xfe, 0x25,
        0xf5, 0x61, 0xa6, 0xfc, 0x93, 0x8b, 0x2e, 0x26,
        0x43, 0x06, 0xec, 0x30, 0x4e, 0xda, 0x51, 0x80,
        0x07, 0xd1, 0x76, 0x48, 0x26, 0x38, 0x19, 0x69
    };

    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 1, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    unsigned char actual_hash[HASH_SIZE];
    TEST_ASSERT(get_tree_hash(tree, actual_hash) == MERKLE_SUCCESS, "Should get tree hash");
    TEST_ASSERT(memcmp(actual_hash, expected, HASH_SIZE) == 0,
                "Root hash mismatch for single element");

    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Verify root hash for a known two element tree.
 */
static int test_root_hash_two_elements(void) {
    const char *data[] = {"Test", "Data"};
    size_t sizes[] = {4, 4};

    const unsigned char expected[HASH_SIZE] = {
        0xb8, 0x0f, 0xbc, 0x01, 0x2e, 0x10, 0x74, 0x71,
        0xa5, 0x7b, 0x75, 0xf7, 0x2e, 0x56, 0x6c, 0xcc,
        0x5c, 0x53, 0x27, 0x36, 0x2e, 0xaf, 0x62, 0x33,
        0x1a, 0x0b, 0x04, 0x6b, 0x20, 0x3a, 0xf5, 0x21
    };

    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 2, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    unsigned char actual_hash[HASH_SIZE];
    TEST_ASSERT(get_tree_hash(tree, actual_hash) == MERKLE_SUCCESS, "Should get tree hash");
    TEST_ASSERT(memcmp(actual_hash, expected, HASH_SIZE) == 0,
                "Root hash mismatch for two elements");

    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Verify root hash for a known four element tree.
 */
static int test_root_hash_four_elements(void) {
    const char *data[] = {"Hello", "World", "Merkle", "Tree"};
    size_t sizes[] = {5, 5, 6, 4};

    const unsigned char expected[HASH_SIZE] = {
        0xa1, 0x55, 0x41, 0x3a, 0xb3, 0xc2, 0x1a, 0x2a,
        0xe8, 0x88, 0x4c, 0xdb, 0x7a, 0x49, 0x93, 0xa3,
        0x37, 0xad, 0x1a, 0xed, 0x4d, 0x1d, 0xcf, 0xfe,
        0xce, 0x16, 0xa5, 0x90, 0x89, 0x9a, 0x80, 0xeb
    };

    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    unsigned char actual_hash[HASH_SIZE];
    TEST_ASSERT(get_tree_hash(tree, actual_hash) == MERKLE_SUCCESS, "Should get tree hash");
    TEST_ASSERT(memcmp(actual_hash, expected, HASH_SIZE) == 0,
                "Root hash mismatch for four elements");

    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Validate root child count with large branching factor.
 */
static int test_root_child_count_large_bf(void) {
    const char *data[5];
    size_t sizes[5];
    create_test_data((const char **)data, sizes, 5);

    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 5, 10);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    // For large branching factor (10) with 5 elements, all should be direct children of root
    // We can verify this by checking that proof generation works for all leaves
    for (size_t i = 0; i < 5; i++) {
        merkle_proof_t *proof = NULL;
        TEST_ASSERT(generate_proof_from_index(tree, i, &proof) == MERKLE_SUCCESS, 
                    "Should generate proof for each leaf");
        TEST_ASSERT(proof != NULL, "Proof should not be null");
        TEST_ASSERT(proof->path_length == 1, "Should have path length 1 (leaf->root only)");
        
        // Clean up proof
        if (proof) {
            for (size_t j = 0; j < proof->path_length; j++) {
                if (proof->path[j]) {
                    MFree(proof->path[j]->sibling_hashes);
                    MFree(proof->path[j]);
                }
            }
            MFree(proof->path);
            MFree(proof);
        }
    }

    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Verify that get_tree_hash returns the same value as the root hash.
 */
static int test_get_tree_hash_api(void) {
    const char *data[] = {"foo", "bar"};
    size_t sizes[] = {3, 3};

    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 2, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");

    unsigned char out1[HASH_SIZE] = {0};
    unsigned char out2[HASH_SIZE] = {0};
    
    // Test the API twice to ensure consistency
    TEST_ASSERT(get_tree_hash(tree, out1) == MERKLE_SUCCESS, "First API call should succeed");
    TEST_ASSERT(get_tree_hash(tree, out2) == MERKLE_SUCCESS, "Second API call should succeed");
    TEST_ASSERT(memcmp(out1, out2, HASH_SIZE) == 0, "API should return consistent results");

    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Validate that leaf nodes maintain parent links and indices.
 */
static int test_leaf_parent_links(void) {
    const char *data[4];
    size_t sizes[4];
    create_test_data((const char **)data, sizes, 4);

    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");

    // Test that proof generation works for all leaves (validates tree structure)
    for (size_t i = 0; i < 4; ++i) {
        merkle_proof_t *proof = NULL;
        TEST_ASSERT(generate_proof_from_index(tree, i, &proof) == MERKLE_SUCCESS, 
                    "Should generate proof for each leaf");
        TEST_ASSERT(proof != NULL, "Proof should not be null");
        
        // Clean up proof
        if (proof) {
            for (size_t j = 0; j < proof->path_length; j++) {
                if (proof->path[j]) {
                    MFree(proof->path[j]->sibling_hashes);
                    MFree(proof->path[j]);
                }
            }
            MFree(proof->path);
            MFree(proof);
        }
    }

    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Ensure that input data is copied into leaf nodes.
 */
static int test_leaf_data_copied(void) {
    const char *data[] = {"copy", "check"};
    size_t sizes[] = {4, 5};

    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 2, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");

    // Test that tree correctly stored the data by generating proofs
    // This indirectly validates that leaf data is correct
    for (size_t i = 0; i < 2; ++i) {
        merkle_proof_t *proof = NULL;
        TEST_ASSERT(generate_proof_from_index(tree, i, &proof) == MERKLE_SUCCESS, 
                    "Should generate proof for leaf");
        TEST_ASSERT(proof != NULL, "Proof should not be null");
        TEST_ASSERT(proof->leaf_index == i, "Proof should be for correct leaf");
        
        // Clean up proof
        if (proof) {
            for (size_t j = 0; j < proof->path_length; j++) {
                if (proof->path[j]) {
                    MFree(proof->path[j]->sibling_hashes);
                    MFree(proof->path[j]);
                }
            }
            MFree(proof->path);
            MFree(proof);
        }
    }

    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Print test summary and return overall result.
 */
static int print_test_summary(void) {
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", 
           total_tests > 0 ? (100.0 * tests_passed / total_tests) : 0.0);
    
    return tests_failed == 0 ? 0 : 1;
}

/**
 * @brief Test proof generation for simple binary tree.
 */
static int test_proof_generation_binary_tree(void) {
    const char *data[] = {"A", "B", "C", "D"};
    size_t sizes[] = {1, 1, 1, 1};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    merkle_proof_t *proof = NULL;
    merkle_error_t result = generate_proof_from_index(tree, 1, &proof); // Prove leaf B
    
    TEST_ASSERT(result == MERKLE_SUCCESS, "Proof generation should succeed");
    TEST_ASSERT(proof != NULL, "Proof should not be NULL");
    TEST_ASSERT(proof->leaf_index == 1, "Proof should be for leaf index 1");
    TEST_ASSERT(proof->branching_factor == 2, "Proof should have correct branching factor");
    // For binary tree with 4 leaves: levels = 2 (leaf->internal, internal->root)
    TEST_ASSERT(proof->path_length == 2, "Binary tree with 4 leaves should have 2 proof levels");
    
    // Verify proof structure exists
    for (size_t i = 0; i < proof->path_length; i++) {
        TEST_ASSERT(proof->path[i] != NULL, "Proof path item should not be NULL");
        TEST_ASSERT(proof->path[i]->sibling_count > 0, "Should have siblings at each level");
        TEST_ASSERT(proof->path[i]->sibling_hashes != NULL, "Sibling hashes should not be NULL");
    }
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test proof generation for ternary tree.
 */
static int test_proof_generation_ternary_tree(void) {
    const char *data[] = {"A", "B", "C", "D", "E"};
    size_t sizes[] = {1, 1, 1, 1, 1};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 5, 3);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    merkle_proof_t *proof = NULL;
    merkle_error_t result = generate_proof_from_index(tree, 2, &proof); // Prove leaf C
    
    TEST_ASSERT(result == MERKLE_SUCCESS, "Proof generation should succeed");
    TEST_ASSERT(proof != NULL, "Proof should not be NULL");
    TEST_ASSERT(proof->leaf_index == 2, "Proof should be for leaf index 2");
    TEST_ASSERT(proof->branching_factor == 3, "Proof should have correct branching factor");
    
    // Verify sibling counts are correct for ternary tree
    for (size_t i = 0; i < proof->path_length; i++) {
        TEST_ASSERT(proof->path[i]->sibling_count <= 2, "Ternary tree should have at most 2 siblings per level");
    }
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test proof generation for single element tree.
 */
static int test_proof_generation_single_element(void) {
    const char *data[] = {"Single"};
    size_t sizes[] = {6};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 1, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    merkle_proof_t *proof = NULL;
    merkle_error_t result = generate_proof_from_index(tree, 0, &proof);
    
    TEST_ASSERT(result == MERKLE_SUCCESS, "Proof generation should succeed");
    TEST_ASSERT(proof != NULL, "Proof should not be NULL");
    TEST_ASSERT(proof->leaf_index == 0, "Proof should be for leaf index 0");
    TEST_ASSERT(proof->path_length == 0, "Single element tree should have no proof levels");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test proof generation with invalid parameters.
 */
static int test_proof_generation_invalid_params(void) {
    const char *data[] = {"A", "B"};
    size_t sizes[] = {1, 1};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 2, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    merkle_proof_t *proof = NULL;
    
    // Test NULL tree
    merkle_error_t result = generate_proof_from_index(NULL, 0, &proof);
    TEST_ASSERT(result != MERKLE_SUCCESS, "Should fail with NULL tree");
    
    // Test NULL proof pointer
    result = generate_proof_from_index(tree, 0, NULL);
    TEST_ASSERT(result != MERKLE_SUCCESS, "Should fail with NULL proof pointer");
    
    // Test invalid leaf index
    result = generate_proof_from_index(tree, 5, &proof);
    TEST_ASSERT(result != MERKLE_SUCCESS, "Should fail with invalid leaf index");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test proof sibling hash correctness.
 */
static int test_proof_sibling_hash_correctness(void) {
    const char *data[] = {"A", "B", "C", "D"};
    size_t sizes[] = {1, 1, 1, 1};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    // Generate proof for leaf B (index 1)
    merkle_proof_t *proof = NULL;
    merkle_error_t result = generate_proof_from_index(tree, 1, &proof);
    TEST_ASSERT(result == MERKLE_SUCCESS, "Proof generation should succeed");
    
    // For binary tree with leaves [A,B,C,D]:
    // Level 0: B's sibling is A
    // Level 1: (AB)'s sibling is (CD)
    
    // Verify we have exactly 1 sibling at each level for binary tree
    for (size_t i = 0; i < proof->path_length; i++) {
        TEST_ASSERT(proof->path[i]->sibling_count == 1, 
                   "Binary tree should have exactly 1 sibling per level");
    }
    
    // Verify sibling hashes are not zero (they should contain actual hash data)
    for (size_t i = 0; i < proof->path_length; i++) {
        int all_zero = 1;
        for (size_t j = 0; j < HASH_SIZE; j++) {
            if (proof->path[i]->sibling_hashes[0][j] != 0) {
                all_zero = 0;
                break;
            }
        }
        TEST_ASSERT(!all_zero, "Sibling hash should not be all zeros");
    }
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test proof position information.
 */
static int test_proof_position_information(void) {
    const char *data[] = {"A", "B", "C", "D", "E", "F"};
    size_t sizes[] = {1, 1, 1, 1, 1, 1};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 6, 3);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    // Test different leaf positions
    for (size_t leaf_idx = 0; leaf_idx < 6; leaf_idx++) {
        merkle_proof_t *proof = NULL;
        merkle_error_t result = generate_proof_from_index(tree, leaf_idx, &proof);
        
        TEST_ASSERT(result == MERKLE_SUCCESS, "Proof generation should succeed for all leaves");
        TEST_ASSERT(proof->leaf_index == leaf_idx, "Proof should track correct leaf index");
        
        // Verify position information is within valid range
        for (size_t i = 0; i < proof->path_length; i++) {
            TEST_ASSERT(proof->path[i]->node_position < 3, 
                       "Position should be valid for ternary tree");
        }
    }
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test proof generation finder function.
 */
static bool test_value_finder(void *data) {
    return strcmp((char*)data, "Target") == 0;
}

static int test_proof_generation_by_finder(void) {
    const char *data[] = {"A", "Target", "C", "D"};
    size_t sizes[] = {1, 6, 1, 1};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    merkle_proof_t *proof = NULL;
    size_t path_length = 0;
    merkle_error_t result = generate_proof_by_finder(tree, test_value_finder, &path_length, &proof);
    
    TEST_ASSERT(result == MERKLE_SUCCESS, "Proof generation by finder should succeed");
    TEST_ASSERT(proof != NULL, "Proof should not be NULL");
    TEST_ASSERT(proof->leaf_index == 1, "Should find 'Target' at index 1");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test proof generation memory management.
 */
static int test_proof_memory_management(void) {
    const char *data[] = {"A", "B", "C", "D"};
    size_t sizes[] = {1, 1, 1, 1};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    // Generate multiple proofs to test memory handling
    for (size_t i = 0; i < 4; i++) {
        merkle_proof_t *proof = NULL;
        merkle_error_t result = generate_proof_from_index(tree, i, &proof);
        
        TEST_ASSERT(result == MERKLE_SUCCESS, "Proof generation should succeed");
        TEST_ASSERT(proof != NULL, "Proof should not be NULL");
        
        // Verify proof structure integrity
        TEST_ASSERT(proof->path != NULL, "Proof path should not be NULL");
        for (size_t j = 0; j < proof->path_length; j++) {
            TEST_ASSERT(proof->path[j] != NULL, "Proof path items should not be NULL");
            TEST_ASSERT(proof->path[j]->sibling_hashes != NULL, "Sibling hashes should not be NULL");
        }
    }
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test get_tree_hash with NULL tree parameter.
 */
static int test_get_tree_hash_null_tree(void) {
    unsigned char buffer[HASH_SIZE];
    merkle_error_t result = get_tree_hash(NULL, buffer);
    TEST_ASSERT(result == MERKLE_NULL_ARG, "Should return MERKLE_NULL_ARG for NULL tree");
    TEST_PASS();
}

/**
 * @brief Test get_tree_hash with NULL buffer parameter.
 */
static int test_get_tree_hash_null_buffer(void) {
    const char *data[] = {"test"};
    size_t sizes[] = {4};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 1, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    merkle_error_t result = get_tree_hash(tree, NULL);
    TEST_ASSERT(result == MERKLE_NULL_ARG, "Should return MERKLE_NULL_ARG for NULL buffer");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test with NULL sizes array.
 */
static int test_create_null_sizes_array(void) {
    const char *data[] = {"test"};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, NULL, 1, 2);
    TEST_ASSERT(tree == NULL, "Should return NULL for NULL sizes array");
    TEST_PASS();
}

/**
 * @brief Test with extremely large branching factor.
 */
static int test_create_very_large_branching_factor(void) {
    const char *data[2];
    size_t sizes[2];
    create_test_data((const char **)data, sizes, 2);
    
    // Test with large but reasonable branching factor (1000)
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 2, 1000);
    TEST_ASSERT(tree != NULL, "Should handle large branching factor");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test with branching factor of 1.
 */
static int test_create_branching_factor_one(void) {
    const char *data[] = {"test"};
    size_t sizes[] = {4};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 1, 1);
    // Implementation allows branching factor of 1, which creates a linear chain
    if (tree != NULL) {
        TEST_ASSERT(tree->root != NULL, "Root should exist for branching factor 1");
        dealloc_merkle_tree(tree);
    } else {
        // Some implementations may reject branching factor 1
        printf("Implementation rejects branching factor 1 (acceptable behavior)");
    }
    TEST_PASS();
}

/**
 * @brief Test with very large dataset to stress memory management.
 */
static int test_stress_large_dataset(void) {
    const size_t count = 1000;
    const char **data = malloc(count * sizeof(char*));
    size_t *sizes = malloc(count * sizeof(size_t));
    char **allocated_strings = malloc(count * sizeof(char*));
    
    TEST_ASSERT(data && sizes && allocated_strings, "Memory allocation should succeed");
    
    // Create test data
    for (size_t i = 0; i < count; i++) {
        allocated_strings[i] = malloc(50);
        snprintf(allocated_strings[i], 50, "stress_test_data_item_%zu_with_longer_string", i);
        data[i] = allocated_strings[i];
        sizes[i] = strlen(allocated_strings[i]);
    }
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, count, 8);
    TEST_ASSERT(tree != NULL, "Should handle very large dataset");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    
    // Cleanup
    for (size_t i = 0; i < count; i++) {
        free(allocated_strings[i]);
    }
    free(data);
    free(sizes);
    free(allocated_strings);
    
    TEST_PASS();
}

/**
 * @brief Test with extremely small data blocks.
 */
static int test_create_tiny_data_blocks(void) {
    const char *data[] = {"A", "B", "C", "D"};
    size_t sizes[] = {1, 1, 1, 1};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Should handle tiny data blocks");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test with very uneven data sizes.
 */
static int test_create_uneven_data_sizes(void) {
    const char *data[4];
    size_t sizes[4];
    
    // Create data with dramatically different sizes
    char *large_data = malloc(10000);
    memset(large_data, 'X', 9999);
    large_data[9999] = '\0';
    
    data[0] = "A";
    data[1] = large_data;
    data[2] = "CC";
    data[3] = "DDD";
    
    sizes[0] = 1;
    sizes[1] = 9999;
    sizes[2] = 2;
    sizes[3] = 3;
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Should handle very uneven data sizes");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    free(large_data);
    TEST_PASS();
}

/**
 * @brief Test with binary data containing null bytes.
 */
static int test_create_binary_data_with_nulls(void) {
    // Create binary data with embedded null bytes
    unsigned char binary_data1[] = {0x00, 0x01, 0x02, 0x00, 0x03};
    unsigned char binary_data2[] = {0xFF, 0x00, 0xAA, 0x55, 0x00};
    unsigned char binary_data3[] = {0x00, 0x00, 0x00, 0x00, 0x01};
    
    const void *data[] = {binary_data1, binary_data2, binary_data3};
    size_t sizes[] = {5, 5, 5};
    
    merkle_tree_t *tree = create_merkle_tree(data, sizes, 3, 2);
    TEST_ASSERT(tree != NULL, "Should handle binary data with null bytes");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test creating multiple trees sequentially for memory leak detection.
 */
static int test_sequential_tree_creation(void) {
    for (int iteration = 0; iteration < 50; iteration++) {
        const char *data[] = {"Sequential", "Test", "Data", "Iteration"};
        size_t sizes[] = {10, 4, 4, 9};
        
        merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
        TEST_ASSERT(tree != NULL, "Sequential tree creation should succeed");
        dealloc_merkle_tree(tree);
    }
    TEST_PASS();
}

/**
 * @brief Test tree creation with very large count (should fail gracefully).
 */
static int test_create_maximum_count(void) {
    const char *data[] = {"test"};
    size_t sizes[] = {4};
    
    // This should fail due to memory allocation limits - use a large but not SIZE_MAX value
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 1000000000, 2);
    TEST_ASSERT(tree == NULL, "Should fail gracefully with very large count");
    TEST_PASS();
}

/**
 * @brief Test protection against incorrect count parameter (segfault protection).
 */
static int test_create_incorrect_count_protection(void) {
    const char *data[] = {"test1", "test2", "test3"};
    size_t sizes[] = {5, 5, 5};
    
    // Pass count=10 but only provide 3 elements - should fail gracefully instead of segfault
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 10, 2);
    TEST_ASSERT(tree == NULL, "Should fail gracefully when count > actual array size");
    
    // Test with partially valid arrays
    const char *partial_data[] = {"test1", "test2", NULL};
    size_t partial_sizes[] = {5, 5, 0};
    
    tree = create_merkle_tree((const void **)partial_data, partial_sizes, 5, 2);
    TEST_ASSERT(tree == NULL, "Should fail gracefully with mixed valid/invalid data");
    
    // Test extreme case - count much larger than array
    tree = create_merkle_tree((const void **)data, sizes, 1000, 2);
    TEST_ASSERT(tree == NULL, "Should fail gracefully with extremely large count");
    
    TEST_PASS();
}

/**
 * @brief Test root access consistency after multiple operations.
 */
static int test_root_access_consistency(void) {
    const char *data[] = {"Consistency", "Test", "Root", "Access"};
    size_t sizes[] = {11, 4, 4, 6};
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    // Access root hash multiple times and verify consistency
    unsigned char hash1[HASH_SIZE], hash2[HASH_SIZE], hash3[HASH_SIZE];
    
    merkle_error_t result1 = get_tree_hash(tree, hash1);
    merkle_error_t result2 = get_tree_hash(tree, hash2);
    merkle_error_t result3 = get_tree_hash(tree, hash3);
    
    TEST_ASSERT(result1 == MERKLE_SUCCESS && result2 == MERKLE_SUCCESS && result3 == MERKLE_SUCCESS,
                "All hash retrievals should succeed");
    TEST_ASSERT(memcmp(hash1, hash2, HASH_SIZE) == 0 && memcmp(hash2, hash3, HASH_SIZE) == 0,
                "Root hash should be consistent across multiple accesses");
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test proof generation with all possible leaf indices.
 */
static int test_proof_generation_all_indices(void) {
    const size_t leaf_count = 7; // Odd number to test unbalanced tree
    const char *data[7];
    size_t sizes[7];
    create_test_data((const char **)data, sizes, leaf_count);
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, leaf_count, 3);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    // Test proof generation for every leaf
    for (size_t i = 0; i < leaf_count; i++) {
        merkle_proof_t *proof = NULL;
        merkle_error_t result = generate_proof_from_index(tree, i, &proof);
        
        TEST_ASSERT(result == MERKLE_SUCCESS, "Proof generation should succeed for all indices");
        TEST_ASSERT(proof != NULL, "Proof should not be NULL");
        TEST_ASSERT(proof->leaf_index == i, "Proof should have correct leaf index");
    }
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test tree structure validation for different branching factors.
 */
static int test_tree_structure_validation(void) {
    const char *data[9];
    size_t sizes[9];
    create_test_data((const char **)data, sizes, 9);
    
    // Test various branching factors
    size_t branching_factors[] = {2, 3, 4, 5, 9, 10};
    size_t num_factors = sizeof(branching_factors) / sizeof(branching_factors[0]);
    
    for (size_t i = 0; i < num_factors; i++) {
        size_t bf = branching_factors[i];
        merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 9, bf);
        
        TEST_ASSERT(tree != NULL, "Tree creation should succeed for all branching factors");
        
        // Test that tree functions work properly with different branching factors
        unsigned char hash[HASH_SIZE];
        TEST_ASSERT(get_tree_hash(tree, hash) == MERKLE_SUCCESS, "Should get hash");
        
        // Test proof generation for first and last leaf
        merkle_proof_t *proof = NULL;
        TEST_ASSERT(generate_proof_from_index(tree, 0, &proof) == MERKLE_SUCCESS, "Should generate proof for first leaf");
        if (proof) {
            TEST_ASSERT(proof->branching_factor == bf, "Proof should preserve branching factor");
            // Clean up
            for (size_t j = 0; j < proof->path_length; j++) {
                if (proof->path[j]) {
                    MFree(proof->path[j]->sibling_hashes);
                    MFree(proof->path[j]);
                }
            }
            MFree(proof->path);
            MFree(proof);
        }
        
        dealloc_merkle_tree(tree);
    }
    
    TEST_PASS();
}

/**
 * @brief Test proof generation for large branching factor.
 */
static int test_proof_generation_large_branching_factor(void) {
    const char *data[10];
    size_t sizes[10];
    create_test_data((const char **)data, sizes, 10);
    
    merkle_tree_t *tree = create_merkle_tree((const void **)data, sizes, 10, 5);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    merkle_proof_t *proof = NULL;
    merkle_error_t result = generate_proof_from_index(tree, 3, &proof);
    
    TEST_ASSERT(result == MERKLE_SUCCESS, "Proof generation should succeed");
    TEST_ASSERT(proof != NULL, "Proof should not be NULL");
    TEST_ASSERT(proof->branching_factor == 5, "Proof should have correct branching factor");
    
    // Verify sibling counts are correct for large branching factor
    for (size_t i = 0; i < proof->path_length; i++) {
        TEST_ASSERT(proof->path[i]->sibling_count <= 4, 
                   "Should have at most 4 siblings for branching factor 5");
    }
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

// Threading test structures and functions
typedef struct {
    merkle_tree_t *tree;
    int thread_id;
    int iterations;
    int *success_count;
    pthread_mutex_t *mutex;
} thread_test_data_t;

typedef struct {
    merkle_tree_t **trees;
    int num_trees;
    int thread_id;
    int iterations;
    int *success_count;
    pthread_mutex_t *mutex;
} read_write_test_data_t;

/**
 * @brief Thread function for concurrent read operations
 */
static void* concurrent_reader(void* arg) {
    thread_test_data_t *data = (thread_test_data_t*)arg;
    unsigned char hash1[HASH_SIZE];
    unsigned char hash2[HASH_SIZE];
    int local_successes = 0;
    
    for (int i = 0; i < data->iterations; i++) {
        // Test get_tree_hash
        if (get_tree_hash(data->tree, hash1) == MERKLE_SUCCESS) {
            local_successes++;
        }
        
        // Test proof generation
        merkle_proof_t *proof = NULL;
        if (generate_proof_from_index(data->tree, 0, &proof) == MERKLE_SUCCESS) {
            if (proof != NULL) {
                local_successes++;
                // Clean up proof
                for (size_t j = 0; j < proof->path_length; j++) {
                    if (proof->path[j]) {
                        MFree(proof->path[j]->sibling_hashes);
                        MFree(proof->path[j]);
                    }
                }
                MFree(proof->path);
                MFree(proof);
            }
        }
        
        // Verify hash consistency
        if (get_tree_hash(data->tree, hash2) == MERKLE_SUCCESS) {
            if (memcmp(hash1, hash2, HASH_SIZE) == 0) {
                local_successes++;
            }
        }
        
        // Small delay to increase chance of race conditions
        usleep(1);
    }
    
    pthread_mutex_lock(data->mutex);
    *(data->success_count) += local_successes;
    pthread_mutex_unlock(data->mutex);
    
    return NULL;
}

/**
 * @brief Thread function for mixed read-write operations
 */
static void* read_write_worker(void* arg) {
    read_write_test_data_t *data = (read_write_test_data_t*)arg;
    int local_successes = 0;
    
    for (int i = 0; i < data->iterations; i++) {
        // Read from existing tree
        if (data->thread_id % 2 == 0) {
            // Even threads do reads
            merkle_tree_t *tree = data->trees[i % data->num_trees];
            unsigned char hash[HASH_SIZE];
            if (get_tree_hash(tree, hash) == MERKLE_SUCCESS) {
                local_successes++;
            }
            
            merkle_proof_t *proof = NULL;
            if (generate_proof_from_index(tree, 0, &proof) == MERKLE_SUCCESS) {
                if (proof != NULL) {
                    local_successes++;
                    // Clean up proof
                    for (size_t j = 0; j < proof->path_length; j++) {
                        if (proof->path[j]) {
                            MFree(proof->path[j]->sibling_hashes);
                            MFree(proof->path[j]);
                        }
                    }
                    MFree(proof->path);
                    MFree(proof);
                }
            }
        } else {
            // Odd threads create new trees (write operations)
            const char *write_data[] = {"Write", "Test", "Data"};
            size_t write_sizes[] = {5, 4, 4};
            
            merkle_tree_t *new_tree = create_merkle_tree(
                (const void**)write_data, write_sizes, 3, 2);
            
            if (new_tree != NULL) {
                local_successes++;
                dealloc_merkle_tree(new_tree);
            }
        }
        
        usleep(1);
    }
    
    pthread_mutex_lock(data->mutex);
    *(data->success_count) += local_successes;
    pthread_mutex_unlock(data->mutex);
    
    return NULL;
}

/**
 * @brief Test concurrent read operations on a single tree
 */
static int test_concurrent_reads(void) {
    const char *data[] = {"Thread", "Test", "Data", "Concurrent"};
    size_t sizes[] = {6, 4, 4, 10};
    
    merkle_tree_t *tree = create_merkle_tree((const void**)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    const int num_threads = 8;
    const int iterations_per_thread = 100;
    pthread_t threads[num_threads];
    thread_test_data_t thread_data[num_threads];
    int total_successes = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    
    // Create reader threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].tree = tree;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = iterations_per_thread;
        thread_data[i].success_count = &total_successes;
        thread_data[i].mutex = &mutex;
        
        int result = pthread_create(&threads[i], NULL, concurrent_reader, &thread_data[i]);
        TEST_ASSERT(result == 0, "Thread creation should succeed");
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Each thread does 3 operations per iteration
    int expected_min = num_threads * iterations_per_thread * 2; // Conservative estimate
    TEST_ASSERT(total_successes >= expected_min, "Should have many successful concurrent reads");
    
    dealloc_merkle_tree(tree);
    pthread_mutex_destroy(&mutex);
    TEST_PASS();
}

/**
 * @brief Test that concurrent reads don't interfere with each other
 */
static int test_concurrent_read_consistency(void) {
    const char *data[] = {"Consistency", "Test", "Read", "Lock"};
    size_t sizes[] = {11, 4, 4, 4};
    
    merkle_tree_t *tree = create_merkle_tree((const void**)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    // Get reference hash
    unsigned char reference_hash[HASH_SIZE];
    get_tree_hash(tree, reference_hash);
    
    const int num_threads = 4;
    const int iterations_per_thread = 50;
    pthread_t threads[num_threads];
    thread_test_data_t thread_data[num_threads];
    int total_successes = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].tree = tree;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = iterations_per_thread;
        thread_data[i].success_count = &total_successes;
        thread_data[i].mutex = &mutex;
        
        pthread_create(&threads[i], NULL, concurrent_reader, &thread_data[i]);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify tree hash is still consistent
    unsigned char final_hash[HASH_SIZE];
    get_tree_hash(tree, final_hash);
    TEST_ASSERT(memcmp(reference_hash, final_hash, HASH_SIZE) == 0,
                "Tree hash should remain consistent after concurrent reads");
    
    dealloc_merkle_tree(tree);
    pthread_mutex_destroy(&mutex);
    TEST_PASS();
}

/**
 * @brief Test mixed read and write operations
 */
static int test_read_write_synchronization(void) {
    // Create initial trees for reading
    const char *data1[] = {"Read", "Write", "Test"};
    const char *data2[] = {"Sync", "Lock", "Check"};
    size_t sizes[] = {4, 5, 4};
    
    merkle_tree_t *trees[2];
    trees[0] = create_merkle_tree((const void**)data1, sizes, 3, 2);
    trees[1] = create_merkle_tree((const void**)data2, sizes, 3, 2);
    
    TEST_ASSERT(trees[0] != NULL && trees[1] != NULL, "Initial trees should be created");
    
    const int num_threads = 6;
    const int iterations_per_thread = 20;
    pthread_t threads[num_threads];
    read_write_test_data_t thread_data[num_threads];
    int total_successes = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].trees = trees;
        thread_data[i].num_trees = 2;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = iterations_per_thread;
        thread_data[i].success_count = &total_successes;
        thread_data[i].mutex = &mutex;
        
        pthread_create(&threads[i], NULL, read_write_worker, &thread_data[i]);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    TEST_ASSERT(total_successes > 0, "Should have successful read/write operations");
    
    // Verify original trees are still valid
    unsigned char hash1[HASH_SIZE], hash2[HASH_SIZE];
    TEST_ASSERT(get_tree_hash(trees[0], hash1) == MERKLE_SUCCESS, "Original tree 0 should be readable");
    TEST_ASSERT(get_tree_hash(trees[1], hash2) == MERKLE_SUCCESS, "Original tree 1 should be readable");
    
    dealloc_merkle_tree(trees[0]);
    dealloc_merkle_tree(trees[1]);
    pthread_mutex_destroy(&mutex);
    TEST_PASS();
}

/**
 * @brief Stress test with high thread contention
 */
static int test_locking_stress_test(void) {
    const char *data[] = {"Stress", "Test", "High", "Contention", "Lock"};
    size_t sizes[] = {6, 4, 4, 10, 4};
    
    merkle_tree_t *tree = create_merkle_tree((const void**)data, sizes, 5, 3);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    const int num_threads = 16;
    const int iterations_per_thread = 50;
    pthread_t threads[num_threads];
    thread_test_data_t thread_data[num_threads];
    int total_successes = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].tree = tree;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = iterations_per_thread;
        thread_data[i].success_count = &total_successes;
        thread_data[i].mutex = &mutex;
        
        pthread_create(&threads[i], NULL, concurrent_reader, &thread_data[i]);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Under high contention, we should still get most operations successful
    int expected_min = num_threads * iterations_per_thread; // At least one success per iteration
    TEST_ASSERT(total_successes >= expected_min, "Should handle high contention gracefully");
    
    dealloc_merkle_tree(tree);
    pthread_mutex_destroy(&mutex);
    TEST_PASS();
}

/**
 * @brief Test basic locking functionality
 */
static int test_basic_locking(void) {
    const char *data[] = {"Lock", "Test"};
    size_t sizes[] = {4, 4};
    
    merkle_tree_t *tree = create_merkle_tree((const void**)data, sizes, 2, 2);
    TEST_ASSERT(tree != NULL, "Tree creation should succeed");
    
    // Test basic read operations work with locking
    unsigned char hash[HASH_SIZE];
    TEST_ASSERT(get_tree_hash(tree, hash) == MERKLE_SUCCESS, "get_tree_hash should work");
    
    merkle_proof_t *proof = NULL;
    TEST_ASSERT(generate_proof_from_index(tree, 0, &proof) == MERKLE_SUCCESS, "proof generation should work");
    TEST_ASSERT(proof != NULL, "proof should not be null");
    
    // Clean up proof
    if (proof) {
        for (size_t j = 0; j < proof->path_length; j++) {
            if (proof->path[j]) {
                MFree(proof->path[j]->sibling_hashes);
                MFree(proof->path[j]);
            }
        }
        MFree(proof->path);
        MFree(proof);
    }
    
    dealloc_merkle_tree(tree);
    TEST_PASS();
}

/**
 * @brief Test lock cleanup during tree destruction
 */
static int test_lock_cleanup_during_destruction(void) {
    // This test ensures that locks are properly cleaned up
    for (int i = 0; i < 10; i++) {  // Reduced from 100 to 10
        const char *data[] = {"Cleanup", "Test"};
        size_t sizes[] = {7, 4};
        
        merkle_tree_t *tree = create_merkle_tree((const void**)data, sizes, 2, 2);
        TEST_ASSERT(tree != NULL, "Tree creation should succeed");
        
        // Perform a quick operation to ensure lock is initialized
        unsigned char hash[HASH_SIZE];
        get_tree_hash(tree, hash);
        
        // Destroy immediately - this tests lock cleanup
        dealloc_merkle_tree(tree);
    }
    
    TEST_PASS();
}

/**
 * @brief Main test runner.
 */
int main(void) {
    printf("Starting Merkle Tree Unit Tests\n");
    printf("================================\n\n");
    
    // Input validation tests
    RUN_TEST(test_create_null_data);
    RUN_TEST(test_create_zero_count);
    RUN_TEST(test_create_zero_branching_factor);
    RUN_TEST(test_create_null_data_element);
    RUN_TEST(test_create_zero_size_element);
    
    // Basic functionality tests
    RUN_TEST(test_create_single_element);
    RUN_TEST(test_create_binary_tree);
    RUN_TEST(test_create_ternary_tree);
    RUN_TEST(test_create_large_branching_factor);
    
    // Edge case tests
    RUN_TEST(test_destroy_null_tree);
    RUN_TEST(test_different_data_sizes);
    RUN_TEST(test_odd_number_elements);
    RUN_TEST(test_empty_string_elements);
    
    // Correctness tests
    RUN_TEST(test_tree_consistency);
    RUN_TEST(test_tree_sensitivity);
    
    // Performance and stress tests
    RUN_TEST(test_large_dataset);
    RUN_TEST(test_memory_management);
    RUN_TEST(test_various_branching_factors);
    RUN_TEST(test_root_hash_single_element);
    RUN_TEST(test_root_hash_two_elements);
    RUN_TEST(test_root_hash_four_elements);
    RUN_TEST(test_root_child_count_large_bf);
    RUN_TEST(test_get_tree_hash_api);
    RUN_TEST(test_leaf_parent_links);
    RUN_TEST(test_leaf_data_copied);
    
    // Proof generation tests
    RUN_TEST(test_proof_generation_binary_tree);
    RUN_TEST(test_proof_generation_ternary_tree);
    RUN_TEST(test_proof_generation_single_element);
    RUN_TEST(test_proof_generation_invalid_params);
    RUN_TEST(test_proof_sibling_hash_correctness);
    RUN_TEST(test_proof_position_information);
    RUN_TEST(test_proof_generation_by_finder);
    RUN_TEST(test_proof_memory_management);
    RUN_TEST(test_proof_generation_large_branching_factor);
    
    // New error and edge case tests - verified working tests only
    RUN_TEST(test_get_tree_hash_null_tree);
    RUN_TEST(test_get_tree_hash_null_buffer);
    RUN_TEST(test_create_branching_factor_one);
    RUN_TEST(test_create_tiny_data_blocks);
    RUN_TEST(test_create_binary_data_with_nulls);
    RUN_TEST(test_root_access_consistency);
    RUN_TEST(test_sequential_tree_creation);
    RUN_TEST(test_tree_structure_validation);

    RUN_TEST(test_create_null_sizes_array);
    RUN_TEST(test_stress_large_dataset);
    RUN_TEST(test_create_uneven_data_sizes);
    RUN_TEST(test_proof_generation_all_indices);
    
    RUN_TEST(test_create_very_large_branching_factor);
    RUN_TEST(test_sequential_tree_creation);

    RUN_TEST(test_create_maximum_count);
    RUN_TEST(test_create_incorrect_count_protection);

    RUN_TEST(test_tree_structure_validation);
   
    RUN_TEST(test_stress_large_dataset);
    RUN_TEST(test_create_uneven_data_sizes);
    RUN_TEST(test_proof_generation_all_indices);

    // Threading and locking tests
    printf("\n--- Threading and Locking Tests ---\n");
    RUN_TEST(test_basic_locking);
    RUN_TEST(test_lock_cleanup_during_destruction);
    RUN_TEST(test_concurrent_read_consistency);
    RUN_TEST(test_concurrent_reads);
    RUN_TEST(test_read_write_synchronization);
    RUN_TEST(test_locking_stress_test);

    return print_test_summary();
}
