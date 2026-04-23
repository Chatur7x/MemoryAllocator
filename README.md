# Thread-Local Memory Allocator

A custom memory allocator in C that uses **per-thread caches** to reduce lock contention in multi-threaded programs. Demonstrates core systems programming concepts including memory management, thread synchronization, and performance optimization.

![C](https://img.shields.io/badge/C-11-blue?logo=c)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-green)

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Application                           │
└─────────────────────┬───────────────────────────────┘
                      │
         ┌────────────┴────────────┐
         │   my_malloc()        │
         │   my_free()         │
         │   my_realloc()       │
         └────────┬───────────┘
                  │
    ┌─────────────┴─────────────┐
    │   Thread Local Cache    │ ◄─── FAST PATH (no lock)
    │   ┌──────────────┐  │
    │   │ Free List   │  │
    │   │ (TLS)      │  │
    │   └──────────────┘  │
    └──────────┬───────────┘
               │
    ┌──────────┴──────────┐
    │   Global Heap      │ ◄─── SLOW PATH (lock)
    │   ┌──────────┐   │
    │   │ Mutex   │   │
    │   │         │   │
    │   │ mmap/   │   │
    │   │ VirtualAlloc  │
    │   └──────────┘   │
    └──────────────────┘
```

## Features

| Feature | Description |
|---------|------------|
| **Thread-Local Cache** | Each thread has own free list - no locks in fast path |
| **Global Heap** | Shared memory pool with mutex protection |
| **Block Splitting** | Split large blocks for smaller allocations |
| **Block Coalescing** | Merge adjacent free blocks |
| **my_realloc()** | Resize existing allocations |
| **8-byte Alignment** | Proper memory alignment |
| **Debug Mode** | Detect double-free, use-after-free |
| **Memory Stats** | Allocation tracking |
| **mmap Fallback** | Large allocations |
| **Cross-Platform** | Linux (pthread) + Windows (CriticalSection) |

## Project Structure

```
thread-allocator/
├── 📂 include/
│   └── 📄 allocator.h          # Public API
├── 📂 src/
│   ├── 📄 portability.h       # Platform abstraction
│   ├── 📄 block.c/h         # Block management
│   ├── 📄 global_heap.c/h   # Global heap
│   ├── 📄 thread_cache.c/h  # Thread-local cache
│   ├── 📄 allocator.c      # Main API
│   ├── 📄 debug.c/h        # Debug mode
│   ├── 📄 benchmark.c     # Performance tests
│   └── 📄 test_allocator.c  # Basic tests
├── 📄 Makefile                 # Build
├── 📄 .gitignore               # Git ignore
└── 📄 README.md                # Docs
```

## Build & Run

### Linux/macOS

```bash
# Build
make

# Run tests
make test

# Run benchmark
make benchmark

# Build debug version
make debug
```

### Windows

```bash
# Using MinGW/GCC
make

# Or compile manually
gcc -o allocator_test src\test_allocator.c src\*.c -Iinclude -Isrc
```

## Usage

```c
#include "allocator.h"

int main() {
    allocator_init();
    
    // Basic allocation
    void* ptr = my_malloc(64);
    
    // Free
    my_free(ptr);
    
    // Realloc
    ptr = my_realloc(ptr, 128);
    
    // Statistics
    AllocatorStats stats = get_allocator_stats();
    printf("Current in use: %zu bytes\n", stats.current_in_use);
    
    allocator_destroy();
    return 0;
}
```

## API Reference

| Function | Description |
|----------|------------|
| `my_malloc(size)` | Allocate memory |
| `my_calloc(nmemb, size)` | Allocate and zero-initialize |
| `my_realloc(ptr, new_size)` | Resize allocation |
| `my_free(ptr)` | Free memory |
| `allocator_init()` | Initialize allocator |
| `allocator_destroy()` | Cleanup allocator |
| `get_allocator_stats()` | Get allocation statistics |

## Benchmark

Run benchmarks against glibc malloc:

```bash
make benchmark
```

Results are saved to `benchmark_results.json`:

```json
[
  {
    "name": "glibc_malloc",
    "time_ms": 12.50,
    "operations": 100000,
    "ops_per_sec": 8000000.00,
    "total_bytes": 6400000
  },
  {
    "name": "my_malloc",
    "time_ms": 8.20,
    "operations": 100000,
    "ops_per_sec": 12195121.95,
    "total_bytes": 6400000
  }
]
```

## Configuration

Edit defines in source files:

```c
#define LOCAL_CACHE_MAX 64     // Blocks per thread cache
#define BATCH_SIZE 8        // Blocks to grab from global
#define MIN_BLOCK_SIZE 32     // Minimum block size
#define MIN_ALLOC_SIZE 16384 // Minimum allocation from OS
```

## Debug Mode

Enable debug mode at compile time:

```bash
make debug
```

Features:
- Double-free detection
- Use-after-free detection
- Memory leak reporting
- Allocation tracking with file:line info

## Performance Tips

| Technique | Impact |
|-----------|--------|
| Thread-local cache | ~5-10x faster for repeated alloc/free |
| Batching | Reduces lock contention |
| Block splitting | Reduces memory waste |
| Coalescing | Reuses freed memory |

## Concepts Demonstrated

- **Memory Management** - Block metadata, splitting, coalescing
- **Thread Synchronization** - Mutex, TLS (thread-local storage)
- **Performance Optimization** - Lock contention reduction
- **Platform Abstraction** - Cross-platform compatibility
- **Benchmarking** - Performance measurement and comparison

