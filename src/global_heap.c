#include "global_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PLATFORM_UNIX
#include <sys/mman.h>
#endif

static GlobalHeap default_heap;

void global_heap_init(GlobalHeap* heap) {
    if (heap == NULL) return;
    
    heap->free_list = NULL;
    MUTEX_INIT(&heap->lock);
    heap->total_allocated = 0;
    heap->total_freed = 0;
}

void global_heap_destroy(GlobalHeap* heap) {
    if (heap == NULL) return;
    
    MUTEX_LOCK(&heap->lock);
    
    Block* current = heap->free_list;
    while (current != NULL) {
        Block* next = current->next;
        
#ifdef PLATFORM_UNIX
        munmap(current, BLOCK_HEADER_SIZE + current->size);
#elif defined(PLATFORM_WINDOWS)
        VirtualFree(current, 0, MEM_RELEASE);
#endif
        current = next;
    }
    
    heap->free_list = NULL;
    MUTEX_UNLOCK(&heap->lock);
    MUTEX_DESTROY(&heap->lock);
}

static void* allocate_pages(size_t size) {
    size = align_size(size);
    
#ifdef PLATFORM_UNIX
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
#elif defined(PLATFORM_WINDOWS)
    void* ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE,
                           PAGE_READWRITE);
    if (ptr == NULL) {
        return NULL;
    }
#endif
    return ptr;
}

void* global_heap_alloc(GlobalHeap* heap, size_t size) {
    if (heap == NULL || size == 0) return NULL;
    
    size_t total_size = align_size(size + BLOCK_HEADER_SIZE);
    
    MUTEX_LOCK(&heap->lock);
    
    Block* block = find_free_block(&heap->free_list, total_size);
    
    if (block != NULL) {
        if (block->size >= total_size + MIN_BLOCK_SIZE) {
            Block* remaining = split_block(block, total_size);
            if (remaining != NULL) {
                add_block(&heap->free_list, remaining);
            }
        }
        
        remove_block(&heap->free_list, block);
        block->is_free = false;
        block->magic = BLOCK_MAGIC;
        heap->total_allocated += block->size;
        
        MUTEX_UNLOCK(&heap->lock);
        return block_to_data(block);
    }
    
    MUTEX_UNLOCK(&heap->lock);
    
    size_t alloc_size = (total_size > MIN_ALLOC_SIZE) ? total_size : MIN_ALLOC_SIZE;
    void* raw = allocate_pages(alloc_size);
    
    if (raw == NULL) {
        return NULL;
    }
    
    Block* new_block = (Block*)raw;
    new_block->size = alloc_size - BLOCK_HEADER_SIZE;
    new_block->is_free = false;
    new_block->next = NULL;
    new_block->prev = NULL;
    new_block->magic = BLOCK_MAGIC;
    new_block->is_large = (total_size > MIN_ALLOC_SIZE);
    
    MUTEX_LOCK(&heap->lock);
    heap->total_allocated += new_block->size;
    MUTEX_UNLOCK(&heap->lock);
    
    return block_to_data(new_block);
}

void global_heap_free(GlobalHeap* heap, void* ptr) {
    if (heap == NULL || ptr == NULL) return;
    
    Block* block = block_from_ptr(ptr);
    
    if (!block_is_valid(block)) {
        return;
    }
    
    MUTEX_LOCK(&heap->lock);
    
    block->is_free = true;
    add_block(&heap->free_list, block);
    coalesce_block(block);
    
    heap->total_freed += block->size;
    
    MUTEX_UNLOCK(&heap->lock);
}

size_t global_heap_total_allocated(GlobalHeap* heap) {
    if (heap == NULL) return 0;
    return heap->total_allocated;
}

size_t global_heap_total_freed(GlobalHeap* heap) {
    if (heap == NULL) return 0;
    return heap->total_freed;
}

size_t global_heap_available(GlobalHeap* heap) {
    if (heap == NULL) return 0;
    size_t available = 0;
    
    MUTEX_LOCK(&heap->lock);
    
    Block* current = heap->free_list;
    while (current != NULL) {
        available += current->size;
        current = current->next;
    }
    
    MUTEX_UNLOCK(&heap->lock);
    
    return available;
}

GlobalHeap* get_default_heap() {
    return &default_heap;
}

void init_default_heap() {
    global_heap_init(&default_heap);
}