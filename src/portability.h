#ifndef PORTABILITY_H
#define PORTABILITY_H

#ifdef _WIN32
    #define PLATFORM_WINDOWS 1
    #include <windows.h>
#else
    #define PLATFORM_UNIX 1
    #include <pthread.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <stdint.h>
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PLATFORM_WINDOWS

    #define THREAD_LOCAL __declspec(thread)

    typedef CRITICAL_SECTION mutex_t;

    #define MUTEX_INIT(m) InitializeCriticalSection(m)
    #define MUTEX_LOCK(m) EnterCriticalSection(m)
    #define MUTEX_UNLOCK(m) LeaveCriticalSection(m)
    #define MUTEX_DESTROY(m) DeleteCriticalSection(m)

    #define page_size() (4096)

#else

    #define THREAD_LOCAL __thread

    typedef pthread_mutex_t mutex_t;

    #define MUTEX_INIT(m) pthread_mutex_init(m, NULL)
    #define MUTEX_LOCK(m) pthread_mutex_lock(m)
    #define MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
    #define MUTEX_DESTROY(m) pthread_mutex_destroy(m)

    #define page_size() (sysconf(_SC_PAGESIZE))

#endif

#define DEFAULT_PAGE_SIZE 4096

#ifdef __cplusplus
}
#endif

#endif