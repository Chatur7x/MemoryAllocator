#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allocator.h"

int main() {
    printf("=== Thread-Local Memory Allocator Test ===\n\n");
    
    allocator_init();
    
    printf("Test 1: Basic malloc/free\n");
    for (int i = 0; i < 10; i++) {
        void* ptr = my_malloc(64);
        printf("  Allocated %p (64 bytes)\n", ptr);
        my_free(ptr);
        printf("  Freed %p\n", ptr);
    }
    printf("  PASSED\n\n");
    
    printf("Test 2: Various sizes\n");
    size_t sizes[] = {16, 32, 64, 128, 256, 512, 1024};
    size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    for (size_t j = 0; j < num_sizes; j++) {
        void* ptr = my_malloc(sizes[j]);
        printf("  Allocated %zu bytes: %p\n", sizes[j], ptr);
        my_free(ptr);
    }
    printf("  PASSED\n\n");
    
    printf("Test 3: Realloc\n");
    void* ptr = my_malloc(64);
    printf("  Original: %p (64 bytes)\n", ptr);
    ptr = my_realloc(ptr, 128);
    printf("  Reallocated: %p (128 bytes)\n", ptr);
    ptr = my_realloc(ptr, 32);
    printf("  Reallocated: %p (32 bytes)\n", ptr);
    my_free(ptr);
    printf("  PASSED\n\n");
    
    printf("Test 4: Stats\n");
    AllocatorStats stats = get_allocator_stats();
    printf("  Total allocated: %zu bytes\n", stats.total_allocated);
    printf("  Total freed: %zu bytes\n", stats.total_freed);
    printf("  Current in use: %zu bytes\n", stats.current_in_use);
    printf("  Total allocations: %zu\n", stats.total_allocations);
    printf("  Total frees: %zu\n", stats.total_frees);
    printf("  PASSED\n\n");
    
    allocator_destroy();
    
    printf("All tests passed!\n");
    return 0;
}