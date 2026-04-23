#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void* my_malloc(size_t size);
void* my_calloc(size_t nmemb, size_t size);
void* my_realloc(void* ptr, size_t size);
void my_free(void* ptr);

void allocator_init(void);
void allocator_destroy(void);

typedef struct AllocatorStats {
    size_t total_allocated;
    size_t total_freed;
    size_t current_in_use;
    size_t total_allocations;
    size_t total_frees;
} AllocatorStats;

AllocatorStats get_allocator_stats(void);
void reset_allocator_stats(void);

bool is_allocated_by_me(void* ptr);

#ifdef __cplusplus
}
#endif

#endif