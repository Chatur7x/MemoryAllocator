#include "benchmark.h"
#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef DEBUG_MODE
#undef DEBUG_MODE
#endif

#define MAX_RESULTS 16
#define MAX_THREADS 16

typedef struct {
    void* (*alloc_fn)(size_t);
    void (*free_fn)(void*);
    const char* name;
} AllocatorFn;

static AllocatorFn allocators[] = {
    { benchmark_alloc_fn, benchmark_free_fn, "my_malloc" },
    { malloc, free, "glibc malloc" },
    { NULL, NULL, NULL }
};

typedef struct ThreadArg {
    void* (*alloc_fn)(size_t);
    void (*free_fn)(void*);
    size_t num_iterations;
    size_t min_size;
    size_t max_size;
    int thread_id;
} ThreadArg;

#ifdef PLATFORM_UNIX
#include <pthread.h>
static pthread_t threads[MAX_THREADS];
#elif defined(PLATFORM_WINDOWS)
#include <windows.h>
static HANDLE threads[MAX_THREADS];
static DWORD thread_ids[MAX_THREADS];
#endif

double get_time_ms(void) {
#ifdef PLATFORM_UNIX
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
#elif defined(PLATFORM_WINDOWS)
    return (double)GetTickCount();
#endif
}

static size_t current_test_size = 64;

void* benchmark_alloc_fn(size_t size) {
    current_test_size = size;
    return my_malloc(size);
}

void benchmark_free_fn(void* ptr) {
    my_free(ptr);
}

static size_t get_random_size(size_t min, size_t max) {
    if (max <= min) return min;
    return min + (rand() % (max - min + 1));
}

void* thread_worker(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    
    for (size_t i = 0; i < targ->num_iterations; i++) {
        size_t size = get_random_size(targ->min_size, targ->max_size);
        void* ptr = targ->alloc_fn(size);
        
        if (ptr != NULL) {
            memset(ptr, 0xAA, size);
            targ->free_fn(ptr);
        }
    }
    
    return NULL;
}

void benchmark_single_thread_malloc(BenchmarkResult* result) {
    const size_t iterations = 100000;
    const size_t size = 64;
    
    double start = get_time_ms();
    
    for (size_t i = 0; i < iterations; i++) {
        void* ptr = malloc(size);
        if (ptr) {
            memset(ptr, 0xAA, size);
            free(ptr);
        }
    }
    
    double end = get_time_ms();
    
    result->name = "glibc_malloc";
    result->time_ms = end - start;
    result->operations = iterations;
    result->ops_per_sec = (iterations * 1000.0) / result->time_ms;
    result->total_bytes = iterations * size;
}

void benchmark_single_thread_my_alloc(BenchmarkResult* result) {
    const size_t iterations = 100000;
    const size_t size = 64;
    
    allocator_init();
    
    double start = get_time_ms();
    
    for (size_t i = 0; i < iterations; i++) {
        void* ptr = my_malloc(size);
        if (ptr) {
            memset(ptr, 0xAA, size);
            my_free(ptr);
        }
    }
    
    double end = get_time_ms();
    
    result->name = "my_malloc";
    result->time_ms = end - start;
    result->operations = iterations;
    result->ops_per_sec = (iterations * 1000.0) / result->time_ms;
    result->total_bytes = iterations * size;
}

void benchmark_multi_thread_my_alloc(BenchmarkResult* result, int num_threads) {
    const size_t iterations = 10000;
    const size_t min_size = 32;
    const size_t max_size = 256;
    
    allocator_init();
    
    ThreadArg args[MAX_THREADS];
    
    for (int i = 0; i < num_threads; i++) {
        args[i].alloc_fn = my_malloc;
        args[i].free_fn = my_free;
        args[i].num_iterations = iterations;
        args[i].min_size = min_size;
        args[i].max_size = max_size;
        args[i].thread_id = i;
    }
    
    double start = get_time_ms();
    
#ifdef PLATFORM_UNIX
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, thread_worker, &args[i]);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
#elif defined(PLATFORM_WINDOWS)
    for (int i = 0; i < num_threads; i++) {
        threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_worker, &args[i], 0, &thread_ids[i]);
    }
    
    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
    
    for (int i = 0; i < num_threads; i++) {
        CloseHandle(threads[i]);
    }
#endif
    
    double end = get_time_ms();
    
    char name[64];
    snprintf(name, sizeof(name), "my_malloc_%d_threads", num_threads);
    
    result->name = "my_malloc";
    result->time_ms = end - start;
    result->operations = iterations * num_threads;
    result->ops_per_sec = (result->operations * 1000.0) / result->time_ms;
    result->total_bytes = result->operations * ((min_size + max_size) / 2);
}

void benchmark_mixed_alloc_free_my_alloc(BenchmarkResult* result) {
    const size_t num_allocations = 50000;
    void** pointers = (void**)malloc(sizeof(void*) * num_allocations);
    
    if (pointers == NULL) return;
    
    allocator_init();
    
    for (size_t i = 0; i < num_allocations / 2; i++) {
        size_t size = get_random_size(16, 128);
        pointers[i] = my_malloc(size);
    }
    
    double start = get_time_ms();
    
    for (size_t i = 0; i < num_allocations / 2; i++) {
        size_t size = get_random_size(16, 128);
        pointers[i] = my_malloc(size);
    }
    
    for (size_t i = 0; i < num_allocations / 2; i++) {
        my_free(pointers[i]);
    }
    
    double end = get_time_ms();
    
    result->name = "my_malloc_mixed";
    result->time_ms = end - start;
    result->operations = num_allocations;
    result->ops_per_sec = (num_allocations * 1000.0) / result->time_ms;
    result->total_bytes = num_allocations * 64;
    
    for (size_t i = num_allocations / 2; i < num_allocations; i++) {
        if (pointers[i] != NULL) {
            my_free(pointers[i]);
        }
    }
    
    free(pointers);
}

void benchmark_compare_with_glibc(BenchmarkResult* results, size_t* count) {
    *count = 0;
    
    BenchmarkResult single_glibc;
    benchmark_single_thread_malloc(&single_glibc);
    results[(*count)++] = single_glibc;
    
    BenchmarkResult single_my;
    benchmark_single_thread_my_alloc(&single_my);
    results[(*count)++] = single_my;
    
    BenchmarkResult multi_2;
    benchmark_multi_thread_my_alloc(&multi_2, 2);
    results[(*count)++] = multi_2;
    
    BenchmarkResult multi_4;
    benchmark_multi_thread_my_alloc(&multi_4, 4);
    results[(*count)++] = multi_4;
    
    BenchmarkResult multi_8;
    benchmark_multi_thread_my_alloc(&multi_8, 8);
    results[(*count)++] = multi_8;
    
    BenchmarkResult mixed;
    benchmark_mixed_alloc_free_my_alloc(&mixed);
    results[(*count)++] = mixed;
}

void benchmark_print_results(BenchmarkResult* results, size_t count) {
    printf("\n========== BENCHMARK RESULTS ==========\n");
    printf("%-30s %12s %12s %12s\n", "Test Name", "Time (ms)", "Operations", "Ops/sec");
    printf("----------------------------------------------------------\n");
    
    for (size_t i = 0; i < count; i++) {
        printf("%-30s %12.2f %12zu %12.2f\n",
               results[i].name,
               results[i].time_ms,
               results[i].operations,
               results[i].ops_per_sec);
    }
    
    printf("==========================================\n\n");
}

void benchmark_save_results_json(BenchmarkResult* results, size_t count, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return;
    }
    
    fprintf(fp, "[\n");
    
    for (size_t i = 0; i < count; i++) {
        fprintf(fp, "  {\n");
        fprintf(fp, "    \"name\": \"%s\",\n", results[i].name);
        fprintf(fp, "    \"time_ms\": %.2f,\n", results[i].time_ms);
        fprintf(fp, "    \"operations\": %zu,\n", results[i].operations);
        fprintf(fp, "    \"ops_per_sec\": %.2f,\n", results[i].ops_per_sec);
        fprintf(fp, "    \"total_bytes\": %zu\n", results[i].total_bytes);
        fprintf(fp, "  }%s\n", (i < count - 1) ? "," : "");
    }
    
    fprintf(fp, "]\n");
    
    fclose(fp);
    printf("Results saved to %s\n", filename);
}

void benchmark_run_all(BenchmarkResult* results, size_t* result_count) {
    benchmark_compare_with_glibc(results, result_count);
    benchmark_print_results(results, *result_count);
    benchmark_save_results_json(results, *result_count, "benchmark_results.json");
}