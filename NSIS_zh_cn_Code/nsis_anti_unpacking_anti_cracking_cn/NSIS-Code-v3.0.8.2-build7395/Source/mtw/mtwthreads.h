/*
 * Some parts of this file taken from threading.h in Zstandard.
 *
 * Copyright (c) 2016 Tino Reichardt
 * All rights reserved.
 *
 * You can contact the author at:
 * - zstdmt source repository: https://github.com/mcmilk/zstdmt
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

/*
 * mtwthreads.h
 *
 * This file is a part of the multithread wrapper for NSIS.
 * 
 * Modifications - Copyright (C) 2021-2023 Jason Ross (JasonFriday13)
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __MTWTHREADS_H__
#define __MTWTHREADS_H__

#if defined (__cplusplus)
extern "C" {
#endif

#include "../Platform.h"

#ifdef _WIN32

// Mutex
#define MT_MUTEX_T                   CRITICAL_SECTION
#define MT_MUTEX_INIT(a, b)          ((void)(b), InitializeCriticalSection((a)), 0)
#define MT_MUTEX_DESTROY(a)          DeleteCriticalSection((a))
#define MT_MUTEX_LOCK(a)             EnterCriticalSection((a))
#define MT_MUTEX_UNLOCK(a)           LeaveCriticalSection((a))

// Condition variable
/* 
 * This condition code was taken from TinyCThread.
 *
 * Copyright (c) 2012 Marcus Geelnard
 * Copyright (c) 2013-2016 Evan Nemerson
 *
 * zlib/libpng license.
 *
 * Modifications - 2023 JasonFriday13 to emulate 'CONDITION_VARIABLE' properly
 */

typedef struct {
  HANDLE mEvents[2];                  /* Signal and broadcast event HANDLEs. */
  unsigned int mWaitersCount;         /* Count of the number of waiters. */
  CRITICAL_SECTION mWaitersCountLock; /* Serialize access to mWaitersCount. */
} MT_CONDITION_T;

int mt_condition_init(MT_CONDITION_T *cond);
void mt_condition_destroy(MT_CONDITION_T *cond);
int mt_condition_wait(MT_CONDITION_T *cond, MT_MUTEX_T *mtx);
int mt_condition_signal(MT_CONDITION_T *cond);
int mt_condition_broadcast(MT_CONDITION_T *cond);

#define MT_CONDITION_INIT(a, b)      mt_condition_init((a))
#define MT_CONDITION_DESTROY(a)      mt_condition_destroy((a))
#define MT_CONDITION_WAIT(a, b)      mt_condition_wait((a), (b))
#define MT_CONDITION_SIGNAL(a)       mt_condition_signal((a))
#define MT_CONDITION_BROADCAST(a)    mt_condition_broadcast((a))

// Thread
typedef struct {
    HANDLE handle;
    void* (*start_routine)(void*);
    void* arg;
} MT_THREAD_T;

int mt_thread_create(MT_THREAD_T* thread, const void* unused, void* (*start_routine) (void*), void* arg);
int mt_thread_join(MT_THREAD_T thread, void** value_ptr);

#define MT_THREAD_CREATE mt_thread_create
#define MT_THREAD_JOIN   mt_thread_join

#else // !WIN32
#include <pthread.h>

#define MT_MUTEX_T                   pthread_mutex_t
#define MT_MUTEX_INIT(a, b)          pthread_mutex_init((a), (b))
#define MT_MUTEX_DESTROY(a)          pthread_mutex_destroy((a))
#define MT_MUTEX_LOCK(a)             pthread_mutex_lock((a))
#define MT_MUTEX_UNLOCK(a)           pthread_mutex_unlock((a))

#define MT_CONDITION_T               pthread_cond_t
#define MT_CONDITION_INIT(a, b)      pthread_cond_init((a), (b))
#define MT_CONDITION_DESTROY(a)      pthread_cond_destroy((a))
#define MT_CONDITION_WAIT(a, b)      pthread_cond_wait((a), (b))
#define MT_CONDITION_SIGNAL(a)       pthread_cond_signal((a))
#define MT_CONDITION_BROADCAST(a)    pthread_cond_broadcast((a))

#define MT_THREAD_T                  pthread_t
#define MT_THREAD_CREATE(a, b, c, d) pthread_create((a), (b), (c), (d))
#define MT_THREAD_JOIN(a, b)         pthread_join((a),(b))

#endif // ~WIN32

typedef void (*data_function)(void*);

typedef struct
{
  data_function function;
  size_t *opaque;
} JOB_HANDLE;

struct MT_THREAD_CTX_S {
  /* Keep track of threads */
  MT_THREAD_T* threads;
  size_t thread_capacity;
  size_t thread_limit;

  /* Circular buffer queue */
  JOB_HANDLE *queue;
  size_t queue_head;
  size_t queue_tail;
  size_t queue_size;

  /* Number of threads busy on work */
  size_t num_threads_busy;
  /* If the queue is empty */
  int queue_empty;

  /* This mutex protects the queue */
  MT_MUTEX_T queue_mutex;
  /* Condition variable for pushers to wait on when the queue is full */
  MT_CONDITION_T queue_push_condition;
  /* Condition variables for poppers to wait on when the queue is empty */
  MT_CONDITION_T queue_pop_condition;
  /* If the queue is shutting down */
  int shutdown;
};

typedef struct MT_THREAD_CTX_S MT_THREAD_CTX;

MT_THREAD_CTX* mt_create(size_t num_threads);
void mt_free(MT_THREAD_CTX *ctx);

void mt_add(MT_THREAD_CTX* ctx, data_function function, void* opaque);

#if defined (__cplusplus)
}
#endif

#endif
