/*
 *
 * Copyright (C) 2023-2024 Intel Corporation
 *
 * Under the Apache License v2.0 with LLVM Exceptions. See LICENSE.TXT.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 */

#ifndef UMF_UTILS_CONCURRENCY_H
#define UMF_UTILS_CONCURRENCY_H 1

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>

#ifndef __cplusplus
#include <stdatomic.h>
#else /* __cplusplus */
#include <atomic>
#define _Atomic(X) std::atomic<X>
#endif /* __cplusplus */

#endif /* _WIN32 */

#include "utils_sanitizers.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct os_mutex_t {
#ifdef _WIN32
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
} os_mutex_t;

size_t util_mutex_get_size(void);
os_mutex_t *util_mutex_init(void *ptr);
void util_mutex_destroy_not_free(os_mutex_t *m);
int util_mutex_lock(os_mutex_t *mutex);
int util_mutex_unlock(os_mutex_t *mutex);

#if defined(_WIN32)
#define UTIL_ONCE_FLAG INIT_ONCE
#define UTIL_ONCE_FLAG_INIT INIT_ONCE_STATIC_INIT
#else
#define UTIL_ONCE_FLAG pthread_once_t
#define UTIL_ONCE_FLAG_INIT PTHREAD_ONCE_INIT
#endif

void util_init_once(UTIL_ONCE_FLAG *flag, void (*onceCb)(void));

#if defined(_WIN32)
static __inline unsigned char util_lssb_index(long long value) {
    unsigned long ret;
    _BitScanForward64(&ret, value);
    return (unsigned char)ret;
}
static __inline unsigned char util_mssb_index(long long value) {
    unsigned long ret;
    _BitScanReverse64(&ret, value);
    return (unsigned char)ret;
}

// There is no good way to do atomic_load on windows...
#define util_atomic_load_acquire(object, dest)                                 \
    do {                                                                       \
        *dest = InterlockedOr64Acquire((LONG64 volatile *)object, 0);          \
    } while (0)

#define util_atomic_store_release(object, desired)                             \
    InterlockedExchange64((LONG64 volatile *)object, (LONG64)desired)
#define util_atomic_increment(object)                                          \
    InterlockedIncrement64((LONG64 volatile *)object)
#define util_fetch_and_add64(ptr, value)                                       \
    InterlockedExchangeAdd64((LONG64 *)(ptr), value)
#else
#define util_lssb_index(x) ((unsigned char)__builtin_ctzll(x))
#define util_mssb_index(x) ((unsigned char)(63 - __builtin_clzll(x)))
#define util_atomic_load_acquire(object, dest)                                 \
    do {                                                                       \
        utils_annotate_acquire((void *)object);                                \
        __atomic_load(object, dest, memory_order_acquire);                     \
    } while (0)

#define util_atomic_store_release(object, desired)                             \
    do {                                                                       \
        __atomic_store_n(object, desired, memory_order_release);               \
        utils_annotate_release((void *)object);                                \
    } while (0)

#define util_atomic_increment(object)                                          \
    __atomic_add_fetch(object, 1, __ATOMIC_ACQ_REL)
#define util_fetch_and_add64 __sync_fetch_and_add
#endif

#ifdef __cplusplus
}
#endif

#endif /* UMF_UTILS_CONCURRENCY_H */
