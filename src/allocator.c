#include "allocator.h"
#include "block.h"
#include "global_heap.h"
#include "thread_cache.h"
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

static _Atomic size_t total_allocated = 0;
static _Atomic size_t total_freed = 0;
static _Atomic size_t total_allocations = 0;
static _Atomic size_t total_frees = 0;

void allocator_init(void) {
    init_default_heap();
}

void allocator_destroy(void) {
    GlobalHeap* heap = get_default_heap();
    global_heap_destroy(heap);
}

void* my_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    size = align_size(size);
    
    void* ptr = thread_cache_alloc(size);
    
    if (ptr != NULL) {
        atomic_fetch_add(&total_allocated, size);
        atomic_fetch_add(&total_allocations, 1);
        memset(ptr, 0, size);
    }
    
    return ptr;
}

void* my_calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) {
        return NULL;
    }
    
    size_t total_size = nmemb * size;
    if (total_size / size != nmemb) {
        return NULL;
    }
    
    void* ptr = my_malloc(total_size);
    
    if (ptr != NULL) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

void* my_realloc(void* ptr, size_t new_size) {
    if (ptr == NULL) {
        return my_malloc(new_size);
    }
    
    if (new_size == 0) {
        my_free(ptr);
        return NULL;
    }
    
    Block* block = block_from_ptr(ptr);
    
    if (!block_is_valid(block)) {
        return NULL;
    }
    
    size_t old_size = block->size;
    
    if (new_size <= old_size) {
        return ptr;
    }
    
    void* new_ptr = my_malloc(new_size);
    
    if (new_ptr == NULL) {
        return NULL;
    }
    
    memcpy(new_ptr, ptr, old_size);
    my_free(ptr);
    
    return new_ptr;
}

void my_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    Block* block = block_from_ptr(ptr);
    
    if (!block_is_valid(block)) {
        return;
    }
    
    size_t size = block->size;
    
    atomic_fetch_add(&total_freed, size);
    atomic_fetch_add(&total_frees, 1);
    
    thread_cache_free(ptr);
}

AllocatorStats get_allocator_stats(void) {
    AllocatorStats stats;
    stats.total_allocated = atomic_load(&total_allocated);
    stats.total_freed = atomic_load(&total_freed);
    stats.current_in_use = stats.total_allocated - stats.total_freed;
    stats.total_allocations = atomic_load(&total_allocations);
    stats.total_frees = atomic_load(&total_frees);
    return stats;
}

void reset_allocator_stats(void) {
    atomic_store(&total_allocated, 0);
    atomic_store(&total_freed, 0);
    atomic_store(&total_allocations, 0);
    atomic_store(&total_frees, 0);
}

bool is_allocated_by_me(void* ptr) {
    if (ptr == NULL) {
        return false;
    }
    
    Block* block = block_from_ptr(ptr);
    return block_is_valid(block);
}