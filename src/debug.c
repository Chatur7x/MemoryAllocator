#include "debug.h"
#include "block.h"
#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG_MODE

static AllocationRecord* allocation_list = NULL;
static size_t allocation_count = 0;

void debug_init(void) {
    allocation_list = NULL;
    allocation_count = 0;
}

void debug_destroy(void) {
    if (allocation_count > 0) {
        fprintf(stderr, "[DEBUG] Memory leaks detected: %zu allocations\n", allocation_count);
        debug_print_allocations();
    }
    
    AllocationRecord* current = allocation_list;
    while (current != NULL) {
        AllocationRecord* next = current->next;
        free(current);
        current = next;
    }
    allocation_list = NULL;
    allocation_count = 0;
}

static bool is_valid_pointer(void* ptr) {
    AllocationRecord* current = allocation_list;
    while (current != NULL) {
        if (current->ptr == ptr) {
            return true;
        }
        current = current->next;
    }
    return false;
}

void* debug_malloc(size_t size, const char* file, int line) {
    void* ptr = my_malloc(size);
    
    if (ptr == NULL) {
        return NULL;
    }
    
    AllocationRecord* record = (AllocationRecord*)malloc(sizeof(AllocationRecord));
    if (record == NULL) {
        return ptr;
    }
    
    record->ptr = ptr;
    record->size = size;
    record->file = file;
    record->line = line;
    record->next = allocation_list;
    allocation_list = record;
    allocation_count++;
    
    return ptr;
}

void debug_free(void* ptr, const char* file, int line) {
    if (ptr == NULL) {
        return;
    }
    
    if (!is_valid_pointer(ptr)) {
        fprintf(stderr, "[DEBUG] Error: Attempting to free invalid pointer at %s:%d\n", file, line);
        return;
    }
    
    AllocationRecord* current = allocation_list;
    AllocationRecord* prev = NULL;
    
    while (current != NULL) {
        if (current->ptr == ptr) {
            if (prev) {
                prev->next = current->next;
            } else {
                allocation_list = current->next;
            }
            
            free(current);
            allocation_count--;
            break;
        }
        prev = current;
        current = current->next;
    }
    
    my_free(ptr);
}

bool debug_check_double_free(void* ptr) {
    AllocationRecord* current = allocation_list;
    while (current != NULL) {
        if (current->ptr == ptr) {
            return true;
        }
        current = current->next;
    }
    return false;
}

bool debug_check_use_after_free(void* ptr) {
    return !is_valid_pointer(ptr);
}

void debug_print_allocations(void) {
    if (allocation_list == NULL) {
        printf("[DEBUG] No active allocations\n");
        return;
    }
    
    printf("[DEBUG] Active allocations:\n");
    AllocationRecord* current = allocation_list;
    while (current != NULL) {
        printf("  - %p: %zu bytes at %s:%d\n", 
               current->ptr, current->size, current->file, current->line);
        current = current->next;
    }
}

size_t debug_get_leak_count(void) {
    return allocation_count;
}

#endif