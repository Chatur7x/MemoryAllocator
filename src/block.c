#include "block.h"
#include <stdio.h>
#include <stdlib.h>

size_t align_size(size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

Block* block_from_ptr(void* ptr) {
    if (ptr == NULL) return NULL;
    return (Block*)((char*)ptr - BLOCK_HEADER_SIZE);
}

void* block_to_data(Block* block) {
    if (block == NULL) return NULL;
    return (char*)block + BLOCK_HEADER_SIZE;
}

Block* split_block(Block* block, size_t size) {
    if (block == NULL || block->size < size + MIN_BLOCK_SIZE) {
        return NULL;
    }

    size_t remaining = block->size - size;
    block->size = size;

    Block* new_block = (Block*)((char*)block + BLOCK_HEADER_SIZE + size);
    new_block->size = remaining - BLOCK_HEADER_SIZE;
    new_block->is_free = true;
    new_block->next = block->next;
    new_block->prev = block;
    new_block->magic = BLOCK_MAGIC;
    new_block->is_large = block->is_large;

    if (block->next) {
        block->next->prev = new_block;
    }
    block->next = new_block;

    return block;
}

Block* coalesce_block(Block* block) {
    if (block == NULL || !block->is_free) {
        return block;
    }

    Block* prev = block->prev;
    Block* next = block->next;

    if (prev && prev->is_free && (char*)prev + BLOCK_HEADER_SIZE + prev->size == (char*)block) {
        prev->size += BLOCK_HEADER_SIZE + block->size;
        prev->next = next;
        if (next) next->prev = prev;
        block = prev;
    }

    if (next && next->is_free && (char*)block + BLOCK_HEADER_SIZE + block->size == (char*)next) {
        block->size += BLOCK_HEADER_SIZE + next->size;
        block->next = next->next;
        if (next->next) next->next->prev = block;
    }

    return block;
}

Block* find_free_block(Block** head, size_t size) {
    if (head == NULL || *head == NULL) {
        return NULL;
    }

    Block* current = *head;
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void add_block(Block** head, Block* block) {
    if (head == NULL || block == NULL) {
        return;
    }

    block->prev = NULL;
    block->next = *head;

    if (*head != NULL) {
        (*head)->prev = block;
    }
    *head = block;
}

void remove_block(Block** head, Block* block) {
    if (head == NULL || block == NULL) {
        return;
    }

    if (block->prev) {
        block->prev->next = block->next;
    } else {
        *head = block->next;
    }

    if (block->next) {
        block->next->prev = block->prev;
    }
}

size_t get_block_size(Block* block) {
    if (block == NULL) return 0;
    return block->size;
}

bool block_is_valid(Block* block) {
    return block != NULL && block->magic == BLOCK_MAGIC;
}

bool block_is_large(Block* block) {
    return block != NULL && block->is_large;
}