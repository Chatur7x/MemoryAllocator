#include "thread_cache.h"
#include "global_heap.h"
#include <stdlib.h>
#include <string.h>

extern GlobalHeap* get_default_heap(void);

void* thread_cache_alloc(size_t size) {
    size_t total_size = size + BLOCK_HEADER_SIZE;
    total_size = align_size(total_size);
    
    if (local_block_count > 0) {
        Block* block = local_free_list;
        if (block != NULL && block->size >= total_size) {
            remove_block(&local_free_list, block);
            local_block_count--;
            
            if (block->size >= total_size + MIN_BLOCK_SIZE) {
                split_block(block, total_size);
                add_block(&local_free_list, local_free_list);
            }
            
            return block_to_data(block);
        }
        
        Block* prev = NULL;
        Block* current = local_free_list;
        while (current != NULL) {
            if (current->is_free && current->size >= total_size) {
                if (prev) {
                    prev->next = current->next;
                } else {
                    local_free_list = current->next;
                }
                local_block_count--;
                
                if (current->size >= total_size + MIN_BLOCK_SIZE) {
                    split_block(current, total_size);
                    add_block(&local_free_list, local_free_list);
                }
                
                return block_to_data(current);
            }
            prev = current;
            current = current->next;
        }
    }
    
    GlobalHeap* heap = get_default_heap();
    
    size_t batch_size = MIN_ALLOC_SIZE;
    void* batch = global_heap_alloc(heap, batch_size);
    
    if (batch == NULL) {
        return NULL;
    }
    
    Block* batch_block = block_from_ptr(batch);
    
    if (batch_block->size >= total_size + (MIN_BLOCK_SIZE * BATCH_SIZE)) {
        split_block(batch_block, total_size);
        
        Block* remaining = batch_block->next;
        while (remaining != NULL) {
            add_block(&local_free_list, remaining);
            local_block_count++;
            remaining = remaining->next;
        }
    } else {
        batch_block->size = align_size(total_size);
        batch_block->is_free = false;
    }
    
    return block_to_data(batch_block);
}

void thread_cache_free(void* ptr) {
    if (ptr == NULL) return;
    
    Block* block = block_from_ptr(ptr);
    
    if (!block_is_valid(block)) {
        return;
    }
    
    if (local_block_count >= LOCAL_CACHE_MAX) {
        GlobalHeap* heap = get_default_heap();
        global_heap_free(heap, ptr);
        return;
    }
    
    block->is_free = true;
    add_block(&local_free_list, block);
    local_block_count++;
    
    if (local_block_count > LOCAL_CACHE_MAX / 2) {
        GlobalHeap* heap = get_default_heap();
        
        Block* current = local_free_list;
        size_t count = 0;
        
        while (current != NULL && count < BATCH_SIZE) {
            Block* next = current->next;
            remove_block(&local_free_list, current);
            global_heap_free(heap, block_to_data(current));
            local_block_count--;
            count++;
            current = next;
        }
    }
}

size_t thread_cache_available(void) {
    size_t available = 0;
    Block* current = local_free_list;
    
    while (current != NULL) {
        available += current->size;
        current = current->next;
    }
    
    return available;
}

void thread_cache_flush(void) {
    if (local_free_list == NULL) return;
    
    GlobalHeap* heap = get_default_heap();
    
    Block* current = local_free_list;
    while (current != NULL) {
        Block* next = current->next;
        global_heap_free(heap, block_to_data(current));
        current = next;
    }
    
    local_free_list = NULL;
    local_block_count = 0;
}

bool thread_cache_empty(void) {
    return local_free_list == NULL;
}