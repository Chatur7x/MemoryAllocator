#ifndef BLOCK_H
#define BLOCK_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "portability.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_MAGIC 0xDEADBEEF
#define MIN_BLOCK_SIZE 32
#define ALIGNMENT 8

typedef struct Block {
    size_t size;
    bool is_free;
    struct Block* next;
    struct Block* prev;
    uint32_t magic;
    bool is_large;
} Block;

#define BLOCK_HEADER_SIZE sizeof(Block)
#define BLOCK_FOOTER_SIZE sizeof(size_t)

size_t align_size(size_t size);

Block* block_from_ptr(void* ptr);

void* block_to_data(Block* block);

Block* split_block(Block* block, size_t size);

Block* coalesce_block(Block* block);

Block* find_free_block(Block** head, size_t size);

void add_block(Block** head, Block* block);

void remove_block(Block** head, Block* block);

size_t get_block_size(Block* block);

bool block_is_valid(Block* block);

bool block_is_large(Block* block);

#ifdef __cplusplus
}
#endif

#endif