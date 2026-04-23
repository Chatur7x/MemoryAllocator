#ifndef GLOBAL_HEAP_H
#define GLOBAL_HEAP_H

#include <stddef.h>
#include "portability.h"
#include "block.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBAL_PAGE_SIZE (64 * 1024)
#define MIN_ALLOC_SIZE (16 * 1024)

typedef struct GlobalHeap {
    Block* free_list;
    mutex_t lock;
    size_t total_allocated;
    size_t total_freed;
} GlobalHeap;

void global_heap_init(GlobalHeap* heap);

void* global_heap_alloc(GlobalHeap* heap, size_t size);

void global_heap_free(GlobalHeap* heap, void* ptr);

void global_heap_destroy(GlobalHeap* heap);

size_t global_heap_total_allocated(GlobalHeap* heap);

size_t global_heap_total_freed(GlobalHeap* heap);

size_t global_heap_available(GlobalHeap* heap);

#ifdef __cplusplus
}
#endif

#endif