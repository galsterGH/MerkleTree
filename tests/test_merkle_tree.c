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

// Include the headers
#include "test_merkle_internal.h"
#include "Merkle.h"
#include "Utils.h"

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
static int test_create_null_data() {
    merkle_tree_t *tree = merkle_tree_create(NULL, NULL, 0, 2);
    TEST_ASSERT(tree == NULL, "Should return NULL for NULL data");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with zero count.
 */
static int test_create_zero_count() {
    const char *data[] = {"test"};
    size_t sizes[] = {4};
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 0, 2);
    TEST_ASSERT(tree == NULL, "Should return NULL for zero count");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with zero branching factor.
 */
static int test_create_zero_branching_factor() {
    const char *data[] = {"test"};
    size_t sizes[] = {4};
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 1, 0);
    TEST_ASSERT(tree == NULL, "Should return NULL for zero branching factor");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with NULL data element.
 */
static int test_create_null_data_element() {
    const void *data[] = {NULL, "test"};
    size_t sizes[] = {0, 4};
    
    merkle_tree_t *tree = merkle_tree_create(data, sizes, 2, 2);
    TEST_ASSERT(tree == NULL, "Should return NULL for NULL data element");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with zero size element.
 */
static int test_create_zero_size_element() {
    const char *data[] = {"test", "data"};
    size_t sizes[] = {4, 0};
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 2, 2);
    TEST_ASSERT(tree == NULL, "Should return NULL for zero size element");
    TEST_PASS();
}

/**
 * @brief Test creating a tree with single element.
 */
static int test_create_single_element() {
    const char *data[] = {"Hello"};
    size_t sizes[] = {5};
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 1, 2);
    TEST_ASSERT(tree != NULL, "Should successfully create tree with single element");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    merkle_tree_destroy(tree);
    TEST_PASS();
}

/**
 * @brief Test creating a binary tree (branching factor 2).
 */
static int test_create_binary_tree() {
    const char *data[4];
    size_t sizes[4];
    create_test_data((const char **)data, sizes, 4);
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 4, 2);
    TEST_ASSERT(tree != NULL, "Should successfully create binary tree");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    merkle_tree_destroy(tree);
    TEST_PASS();
}

/**
 * @brief Test creating a tree with branching factor 3.
 */
static int test_create_ternary_tree() {
    const char *data[6];
    size_t sizes[6];
    create_test_data((const char **)data, sizes, 6);
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 6, 3);
    TEST_ASSERT(tree != NULL, "Should successfully create ternary tree");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    merkle_tree_destroy(tree);
    TEST_PASS();
}

/**
 * @brief Test creating a tree with large branching factor.
 */
static int test_create_large_branching_factor() {
    const char *data[5];
    size_t sizes[5];
    create_test_data((const char **)data, sizes, 5);
    
    // Branching factor larger than number of elements
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 5, 10);
    TEST_ASSERT(tree != NULL, "Should successfully create tree with large branching factor");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    merkle_tree_destroy(tree);
    TEST_PASS();
}

/**
 * @brief Test tree destruction with NULL pointer.
 */
static int test_destroy_null_tree() {
    // Should not crash
    merkle_tree_destroy(NULL);
    TEST_PASS();
}

/**
 * @brief Test creating trees with different data sizes.
 */
static int test_different_data_sizes() {
    const char *data[] = {"A", "BB", "CCC", "DDDD", "EEEEE"};
    size_t sizes[] = {1, 2, 3, 4, 5};
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 5, 2);
    TEST_ASSERT(tree != NULL, "Should handle different data sizes");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    merkle_tree_destroy(tree);
    TEST_PASS();
}

/**
 * @brief Test tree with odd number of elements.
 */
static int test_odd_number_elements() {
    const char *data[7];
    size_t sizes[7];
    create_test_data((const char **)data, sizes, 7);
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 7, 2);
    TEST_ASSERT(tree != NULL, "Should handle odd number of elements");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    merkle_tree_destroy(tree);
    TEST_PASS();
}

/**
 * @brief Test tree consistency - same data should produce same tree.
 */
static int test_tree_consistency() {
    const char *data[] = {"Test", "Data", "Hash", "Tree"};
    size_t sizes[] = {4, 4, 4, 4};
    
    // Create two identical trees
    merkle_tree_t *tree1 = merkle_tree_create((const void **)data, sizes, 4, 2);
    merkle_tree_t *tree2 = merkle_tree_create((const void **)data, sizes, 4, 2);
    
    TEST_ASSERT(tree1 != NULL && tree2 != NULL, "Both trees should be created");
    
    // Compare root hashes - they should be identical
    TEST_ASSERT(memcmp(tree1->root->hash, tree2->root->hash, 32) == 0, 
                "Root hashes should be identical for same data");
    
    merkle_tree_destroy(tree1);
    merkle_tree_destroy(tree2);
    TEST_PASS();
}

/**
 * @brief Test tree sensitivity - different data should produce different hashes.
 */
static int test_tree_sensitivity() {
    const char *data1[] = {"Test", "Data"};
    const char *data2[] = {"Test", "Different"};
    size_t sizes[] = {4, 4};
    
    merkle_tree_t *tree1 = merkle_tree_create((const void **)data1, sizes, 2, 2);
    
    sizes[1] = 9; // Update size for "Different"
    merkle_tree_t *tree2 = merkle_tree_create((const void **)data2, sizes, 2, 2);
    
    TEST_ASSERT(tree1 != NULL && tree2 != NULL, "Both trees should be created");
    
    // Root hashes should be different
    TEST_ASSERT(memcmp(tree1->root->hash, tree2->root->hash, 32) != 0, 
                "Root hashes should be different for different data");
    
    merkle_tree_destroy(tree1);
    merkle_tree_destroy(tree2);
    TEST_PASS();
}

/**
 * @brief Test with empty strings (valid but zero-length after the pointer).
 */
static int test_empty_string_elements() {
    // Note: We can't test truly empty strings because our validation 
    // requires size > 0, which is correct behavior
    const char *data[] = {"", "test"};
    size_t sizes[] = {0, 4};
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 2, 2);
    TEST_ASSERT(tree == NULL, "Should reject zero-length data");
    TEST_PASS();
}

/**
 * @brief Test with large dataset.
 */
static int test_large_dataset() {
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
    
    merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, count, 4);
    TEST_ASSERT(tree != NULL, "Should handle large dataset");
    TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
    
    merkle_tree_destroy(tree);
    
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
static int test_memory_management() {
    // This test creates and destroys multiple trees to check for leaks
    for (int i = 0; i < 10; i++) {
        const char *data[] = {"Memory", "Test", "Data", "Item"};
        size_t sizes[] = {6, 4, 4, 4};
        
        merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 4, 2);
        TEST_ASSERT(tree != NULL, "Tree creation should succeed");
        merkle_tree_destroy(tree);
    }
    TEST_PASS();
}

/**
 * @brief Test various branching factors.
 */
static int test_various_branching_factors() {
    const char *data[8];
    size_t sizes[8];
    create_test_data((const char **)data, sizes, 8);
    
    // Test different branching factors
    for (size_t bf = 2; bf <= 8; bf++) {
        merkle_tree_t *tree = merkle_tree_create((const void **)data, sizes, 8, bf);
        TEST_ASSERT(tree != NULL, "Should work with various branching factors");
        TEST_ASSERT(tree->root != NULL, "Root should not be NULL");
        merkle_tree_destroy(tree);
    }
    
    TEST_PASS();
}

/**
 * @brief Print test summary and return overall result.
 */
static int print_test_summary() {
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", 
           total_tests > 0 ? (100.0 * tests_passed / total_tests) : 0.0);
    
    return tests_failed == 0 ? 0 : 1;
}

/**
 * @brief Main test runner.
 */
int main() {
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
    
    return print_test_summary();
}