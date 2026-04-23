#ifndef DEBUG_H
#define DEBUG_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG_MODE

#define DEBUG_ENABLED 1

typedef struct DebugHeader {
    uint32_t magic;
    size_t size;
    void* caller;
    const char* file;
    int line;
} DebugHeader;

#define DEBUG_HEADER_SIZE sizeof(DebugHeader)
#define DEBUG_MAGIC 0xCAFEBABE

typedef struct AllocationRecord {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct AllocationRecord* next;
} AllocationRecord;

void debug_init(void);
void debug_destroy(void);

void* debug_malloc(size_t size, const char* file, int line);
void debug_free(void* ptr, const char* file, int line);

bool debug_check_double_free(void* ptr);
bool debug_check_use_after_free(void* ptr);

void debug_print_allocations(void);
size_t debug_get_leak_count(void);

#define my_malloc(size) debug_malloc(size, __FILE__, __LINE__)
#define my_free(ptr) debug_free(ptr, __FILE__, __LINE__)

#else

#define DEBUG_ENABLED 0

#define my_malloc(size) my_malloc(size)
#define my_free(ptr) my_free(ptr)

#endif

#ifdef __cplusplus
}
#endif

#endif