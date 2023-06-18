/*
 * Majority of this file taken from pool.c / .h in Zstandard.
 *
 * Copyright (c) Yann Collet, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

/*
 * Some of this file taken from threading.c / .h in Zstandard.
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
 * mtwthreads.c
 * 
 * This file is a part of the multithread wrapper for NSIS.
 * 
 * Modifications - 2021-2023 Jason Ross (JasonFriday13)
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#include "mtwcommon.h"
#include "mtwthreads.h"

// Windows minimalist Pthread Wrapper, based on :
// http://www.cse.wustl.edu/~schmidt/win32-cv-1.html

#if 1 // For debugging purposes if we want to turn off threading.

#ifdef _WIN32

static long unsigned int __stdcall worker(void *arg)
{
    MT_THREAD_T* const thread = (MT_THREAD_T*) arg;
    thread->arg = thread->start_routine(thread->arg);
    return 0;
}

int mt_thread_create(MT_THREAD_T* thread, const void* unused, void* (*start_routine) (void*), void* arg)
{
    (void)unused;
    thread->arg = arg;
    thread->start_routine = start_routine;
    thread->handle = CreateThread(NULL, 0, worker, thread, 0, NULL);

    if (!thread->handle)
        return GetLastError();
    else
        return 0;
}

int mt_thread_join(MT_THREAD_T thread, void **value_ptr)
{
    DWORD result;

    if (!thread.handle) return 0;

    result = WaitForSingleObject(thread.handle, INFINITE);
    CloseHandle(thread.handle);

    switch (result) {
    case WAIT_OBJECT_0:
        if (value_ptr) *value_ptr = thread.arg;
        return 0;
    case WAIT_ABANDONED:
    default:
        return GetLastError();
    }
}

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

#define _CONDITION_EVENT_ONE 0
#define _CONDITION_EVENT_ALL 1

int mt_condition_init(MT_CONDITION_T *cond)
{
  cond->mWaitersCount = 0;

  /* Init critical section */
  InitializeCriticalSection(&cond->mWaitersCountLock);

  /* Init events */
  cond->mEvents[_CONDITION_EVENT_ONE] = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (cond->mEvents[_CONDITION_EVENT_ONE] == NULL)
  {
    cond->mEvents[_CONDITION_EVENT_ALL] = NULL;
    return 1;
  }
  cond->mEvents[_CONDITION_EVENT_ALL] = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (cond->mEvents[_CONDITION_EVENT_ALL] == NULL)
  {
    CloseHandle(cond->mEvents[_CONDITION_EVENT_ONE]);
    cond->mEvents[_CONDITION_EVENT_ONE] = NULL;
    return 1;
  }

  return 0;
}

void mt_condition_destroy(MT_CONDITION_T *cond)
{
  if (cond->mEvents[_CONDITION_EVENT_ONE] != NULL)
  {
    CloseHandle(cond->mEvents[_CONDITION_EVENT_ONE]);
  }
  if (cond->mEvents[_CONDITION_EVENT_ALL] != NULL)
  {
    CloseHandle(cond->mEvents[_CONDITION_EVENT_ALL]);
  }
  DeleteCriticalSection(&cond->mWaitersCountLock);
}

int mt_condition_wait(MT_CONDITION_T *cond, MT_MUTEX_T *mtx)
{
  DWORD result;
  int lastWaiter;

  /* Increment number of waiters */
  EnterCriticalSection(&cond->mWaitersCountLock);
  ++ cond->mWaitersCount;
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* Release the mutex while waiting for the condition (will decrease
     the number of waiters when done)... */
  MT_MUTEX_UNLOCK(mtx);

  /* Wait for either event to become signaled due to cnd_signal() or
     cnd_broadcast() being called */
  result = WaitForMultipleObjects(2, cond->mEvents, FALSE, INFINITE);
  if (result == WAIT_TIMEOUT)
  {
    /* The mutex is locked again before the function returns, even if an error occurred */
    MT_MUTEX_LOCK(mtx);
    return 0;
  }
  else if (result == WAIT_FAILED)
  {
    /* The mutex is locked again before the function returns, even if an error occurred */
    MT_MUTEX_LOCK(mtx);
    return 0;
  }

  /* Check if we are the last waiter */
  EnterCriticalSection(&cond->mWaitersCountLock);
  -- cond->mWaitersCount;
  lastWaiter = (result == (WAIT_OBJECT_0 + _CONDITION_EVENT_ALL)) &&
               (cond->mWaitersCount == 0);
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* If we are the last waiter to be notified to stop waiting, reset the event */
  if (lastWaiter)
  {
    if (ResetEvent(cond->mEvents[_CONDITION_EVENT_ALL]) == 0)
    {
      /* The mutex is locked again before the function returns, even if an error occurred */
      MT_MUTEX_LOCK(mtx);
      return 0;
    }
  }

  /* Re-acquire the mutex */
  MT_MUTEX_LOCK(mtx);

  return 1;
}

int mt_condition_signal(MT_CONDITION_T *cond)
{
  int haveWaiters;

  /* Are there any waiters? */
  EnterCriticalSection(&cond->mWaitersCountLock);
  haveWaiters = (cond->mWaitersCount > 0);
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* If we have any waiting threads, send them a signal */
  if(haveWaiters)
  {
    if (SetEvent(cond->mEvents[_CONDITION_EVENT_ONE]) == 0)
    {
      return 0;
    }
  }

  return 1;
}

int mt_condition_broadcast(MT_CONDITION_T *cond)
{
  int haveWaiters;

  /* Are there any waiters? */
  EnterCriticalSection(&cond->mWaitersCountLock);
  haveWaiters = (cond->mWaitersCount > 0);
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* If we have any waiting threads, send them a signal */
  if(haveWaiters)
  {
    if (SetEvent(cond->mEvents[_CONDITION_EVENT_ALL]) == 0)
    {
      return 0;
    }
  }

  return 1;
}

#endif

/* End of windows wrapper functions */

void my_copymem(void *dest, const void *src, size_t len)
{
  size_t i;
  for (i = 0; i < len; i++)
    *((char *)dest + i) = *((char *)src + i);
}

/* mt_thread() :
 * Work thread for the thread pool.
 * Waits for jobs and executes them.
 * @returns : NULL on failure else non-null.
 */
static void* mt_thread(void* opaque) {
//static thrd_start_t mt_thread(void* opaque) {
  MT_THREAD_CTX* const ctx = (MT_THREAD_CTX*)opaque;
  if (!ctx) { return NULL; }
  for (;;) {
    /* Lock the mutex and wait for a non-empty queue or until shutdown */
    MT_MUTEX_LOCK(&ctx->queue_mutex);

    while ( ctx->queue_empty
      || (ctx->num_threads_busy >= ctx->thread_limit) ) {
      if (ctx->shutdown) {
        /* even if !queueEmpty, (possible if numThreadsBusy >= threadLimit),
         * a few threads will be shutdown while !queueEmpty,
         * but enough threads will remain active to finish the queue */
        MT_MUTEX_UNLOCK(&ctx->queue_mutex);
        return opaque;
      }
      MT_CONDITION_WAIT(&ctx->queue_pop_condition, &ctx->queue_mutex);
    }
    /* Pop a job off the queue */
    {
      JOB_HANDLE const job = ctx->queue[ctx->queue_head];
      ctx->queue_head = (ctx->queue_head + 1) % ctx->queue_size;
      ctx->num_threads_busy++;
      ctx->queue_empty = (ctx->queue_head == ctx->queue_tail);
      /* Unlock the mutex, signal a pusher, and run the job */
      MT_CONDITION_SIGNAL(&ctx->queue_push_condition);
      MT_MUTEX_UNLOCK(&ctx->queue_mutex);

      job.function(job.opaque);

      /* If the intended queue size was 0, signal after finishing job */
      MT_MUTEX_LOCK(&ctx->queue_mutex);
      ctx->num_threads_busy--;
      MT_CONDITION_SIGNAL(&ctx->queue_push_condition);
      MT_MUTEX_UNLOCK(&ctx->queue_mutex);
    }
  }  /* for (;;) */
//    assert(0);  /* Unreachable */
}

/*! mt_create_advanced() */
MT_THREAD_CTX* mt_create_advanced(size_t num_threads, size_t queue_size)
{
  MT_THREAD_CTX* ctx;
  /* Check parameters */
  if (!num_threads) return NULL;
  /* Allocate the context and zero initialize */
  ctx = (MT_THREAD_CTX*)_ALLOC(sizeof(MT_THREAD_CTX));
  if (!ctx) return NULL;
  /* Initialize the job queue.
   * It needs one extra space since one space is wasted to differentiate
   * empty and full queues.
   */
  ctx->queue_size = queue_size + 1;
  ctx->queue = (JOB_HANDLE*)_ALLOC(ctx->queue_size * sizeof(JOB_HANDLE));
  ctx->queue_head = 0;
  ctx->queue_tail = 0;
  ctx->num_threads_busy = 0;
  ctx->queue_empty = 1;
  {
    int error = 0;
    error |= MT_MUTEX_INIT(&ctx->queue_mutex, NULL);
    error |= MT_CONDITION_INIT(&ctx->queue_push_condition, NULL);
    error |= MT_CONDITION_INIT(&ctx->queue_pop_condition, NULL);
    if (error) { _FREE(ctx); return NULL; }
  }
  ctx->shutdown = 0;
  /* Allocate space for thread handles */
  ctx->threads = (MT_THREAD_T*)_ALLOC(num_threads * sizeof(MT_THREAD_T));
  ctx->thread_capacity = 0;
  /* Check for errors */
  if (!ctx->threads || !ctx->queue) { _FREE(ctx); return NULL; }
  /* Initialize the threads */
  {
    size_t i;
    for (i = 0; i < num_threads; ++i)
    {
      if (MT_THREAD_CREATE(&ctx->threads[i], NULL, &mt_thread, ctx))
      {
        ctx->thread_capacity = i;
        _FREE(ctx);
        return NULL;
      }
    }
    ctx->thread_capacity = num_threads;
    ctx->thread_limit = num_threads;
  }
  return ctx;
}

/*! mt_create() : public access point */
MT_THREAD_CTX* mt_create(size_t num_threads) {
  return mt_create_advanced(num_threads, 0);
}

/*! mt_ctx_join() :
    Shutdown the queue, wake any sleeping threads, and join all of the threads.
*/
static void mt_ctx_join(MT_THREAD_CTX* ctx)
{
  if (!ctx) return;
  /* Shut down the queue */
  MT_MUTEX_LOCK(&ctx->queue_mutex);
  ctx->shutdown = 1;
  MT_MUTEX_UNLOCK(&ctx->queue_mutex);
  /* Wake up sleeping threads */
  MT_CONDITION_BROADCAST(&ctx->queue_push_condition);
  MT_CONDITION_BROADCAST(&ctx->queue_pop_condition);
  /* Join all of the threads */
  if (ctx)
  {
    size_t i;
    for (i = 0; i < ctx->thread_capacity; ++i)
    {
      MT_THREAD_JOIN(ctx->threads[i], NULL);  /* note : could fail */
    }
  }
}

void mt_free(MT_THREAD_CTX *ctx)
{
  if (!ctx) return;
  mt_ctx_join(ctx);
  MT_MUTEX_DESTROY(&ctx->queue_mutex);
  MT_CONDITION_DESTROY(&ctx->queue_push_condition);
  MT_CONDITION_DESTROY(&ctx->queue_pop_condition);
  _FREE(ctx->queue);
  _FREE(ctx->threads);
  _FREE(ctx);
}

/*! mt_wait() :
 *  Waits for all queued jobs to finish executing.
 */
void mt_wait(MT_THREAD_CTX* ctx)
{
  if (!ctx) return;
  MT_MUTEX_LOCK(&ctx->queue_mutex);
  while(!ctx->queue_empty || ctx->num_threads_busy > 0)
  {
    MT_CONDITION_WAIT(&ctx->queue_push_condition, &ctx->queue_mutex);
  }
  MT_MUTEX_UNLOCK(&ctx->queue_mutex);
}

size_t mt_sizeof(const MT_THREAD_CTX* ctx)
{
  if (ctx == NULL) return 0;  /* supports sizeof NULL */
  return sizeof(*ctx) + ctx->queue_size * sizeof(JOB_HANDLE) + ctx->thread_capacity * sizeof(MT_THREAD_T);
}


/* @return : 0 on success, 1 on error */
static int mt_resize_internal(MT_THREAD_CTX* ctx, size_t num_threads)
{
  if (!ctx) return 1;
  if (num_threads <= ctx->thread_capacity)
  {
    if (!num_threads) return 1;
    ctx->thread_limit = num_threads;
    return 0;
  }
  /* numThreads > threadCapacity */
  {
    MT_THREAD_T* const threads_handle = (MT_THREAD_T*)_ALLOC(num_threads * sizeof(MT_THREAD_T));
    if (!threads_handle) return 1;
    /* replace existing thread pool */
    my_copymem(threads_handle, ctx->threads, ctx->thread_capacity * sizeof(*threads_handle));
    _FREE(ctx->threads);
    ctx->threads = threads_handle;
    /* Initialize additional threads */
    {
      size_t thread_id = 0;
      for (num_threads = ctx->thread_capacity; thread_id < num_threads; ++thread_id)
      {
        if (MT_THREAD_CREATE(&threads_handle[thread_id], NULL, &mt_thread, ctx))
        {
          ctx->thread_capacity = thread_id;
          return 1;
        }
      }
    }   
  }
  /* successfully expanded */
  ctx->thread_capacity = num_threads;
  ctx->thread_limit = num_threads;
  return 0;
}

/* @return : 0 on success, 1 on error */
int mt_resize(MT_THREAD_CTX* ctx, size_t num_threads)
{
  int result;
  if (ctx == NULL) return 1;
  MT_MUTEX_LOCK(&ctx->queue_mutex);
  result = mt_resize_internal(ctx, num_threads);
  MT_CONDITION_BROADCAST(&ctx->queue_pop_condition);
  MT_MUTEX_UNLOCK(&ctx->queue_mutex);
  return result;
}

/**
 * Returns 1 if the queue is full and 0 otherwise.
 *
 * When queueSize is 1 (pool was created with an intended queueSize of 0),
 * then a queue is empty if there is a thread free _and_ no job is waiting.
 */
static int mt_is_queue_full(MT_THREAD_CTX const* ctx)
{
  if (ctx->queue_size > 1)
  {
    return ctx->queue_head == ((ctx->queue_tail + 1) % ctx->queue_size);
  }
  else
  {
    return (ctx->num_threads_busy == ctx->thread_limit) || !ctx->queue_empty;
  }
}


static void mt_add_internal(MT_THREAD_CTX* ctx, data_function function, void *opaque)
{
  JOB_HANDLE const job = { function, opaque };
  if (!ctx) return;
  if (ctx->shutdown) return;

  ctx->queue_empty = 0;
  ctx->queue[ctx->queue_tail] = job;
  ctx->queue_tail = (ctx->queue_tail + 1) % ctx->queue_size;
  MT_CONDITION_SIGNAL(&ctx->queue_pop_condition);
}

void mt_add(MT_THREAD_CTX* ctx, data_function function, void* opaque)
{
  if (!ctx) return;
  MT_MUTEX_LOCK(&ctx->queue_mutex);
  /* Wait until there is space in the queue for the new job */
  while (mt_is_queue_full(ctx) && (!ctx->shutdown))
  {
      MT_CONDITION_WAIT(&ctx->queue_push_condition, &ctx->queue_mutex);
  }
  mt_add_internal(ctx, function, opaque);
  MT_MUTEX_UNLOCK(&ctx->queue_mutex);
}


int mt_try_add(MT_THREAD_CTX* ctx, data_function function, size_t* opaque)
{
  if (!ctx) return 0;
  MT_MUTEX_LOCK(&ctx->queue_mutex);
  if (mt_is_queue_full(ctx)) {
      MT_MUTEX_UNLOCK(&ctx->queue_mutex);
      return 0;
  }
  mt_add_internal(ctx, function, opaque);
  MT_MUTEX_UNLOCK(&ctx->queue_mutex);
  return 1;
}


#else  /* USE_MULTITHREAD  not defined */

/* ========================== */
/* No multi-threading support */
/* ========================== */

MT_THREAD_CTX ctx;

MT_THREAD_CTX* mt_create_advanced(size_t num_threads, size_t queue_size)
{
    (void)num_threads;
    (void)queue_size;
    return &ctx;
}

MT_THREAD_CTX* mt_create(size_t num_threads)
{
    return mt_create_advanced(num_threads, 0);
}

void mt_free(MT_THREAD_CTX *ctx) {
    if (!ctx) return;
    (void)ctx;
}

void mt_add(MT_THREAD_CTX* ctx, data_function function, void* opaque) {
    (void)ctx;
    function(opaque);
}

int mt_try_add(MT_THREAD_CTX* ctx, data_function function, void* opaque) {
    (void)ctx;
    function(opaque);
    return 1;
}


#endif 
