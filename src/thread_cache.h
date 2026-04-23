#ifndef THREAD_CACHE_H
#define THREAD_CACHE_H

#include <stddef.h>
#include <stdbool.h>
#include "portability.h"
#include "block.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOCAL_CACHE_MAX 64
#define BATCH_SIZE 8

typedef struct ThreadCache {
    Block* free_list;
    size_t block_count;
} ThreadCache;

THREAD_LOCAL Block* local_free_list = NULL;
THREAD_LOCAL size_t local_block_count = 0;

void* thread_cache_alloc(size_t size);

void thread_cache_free(void* ptr);

size_t thread_cache_available(void);

void thread_cache_flush(void);

bool thread_cache_empty(void);

#ifdef __cplusplus
}
#endif

#endif