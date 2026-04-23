#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BenchmarkResult {
    const char* name;
    double time_ms;
    size_t operations;
    double ops_per_sec;
    size_t total_bytes;
} BenchmarkResult;

typedef struct BenchmarkConfig {
    size_t num_iterations;
    size_t num_threads;
    size_t min_size;
    size_t max_size;
    size_t num_sizes;
} BenchmarkConfig;

void benchmark_run_all(BenchmarkResult* results, size_t* result_count);

void benchmark_single_thread_malloc(BenchmarkResult* result);

void benchmark_single_thread_my_alloc(BenchmarkResult* result);

void benchmark_multi_thread_my_alloc(BenchmarkResult* result, int num_threads);

void benchmark_mixed_alloc_free_my_alloc(BenchmarkResult* result);

void benchmark_compare_with_glibc(BenchmarkResult* results, size_t* count);

void benchmark_compare_with_jemalloc(BenchmarkResult* results, size_t* count);

void benchmark_print_results(BenchmarkResult* results, size_t count);

void benchmark_save_results_json(BenchmarkResult* results, size_t count, const char* filename);

void* benchmark_alloc_fn(size_t size);
void benchmark_free_fn(void* ptr);

double get_time_ms(void);

#ifdef __cplusplus
}
#endif

#endif