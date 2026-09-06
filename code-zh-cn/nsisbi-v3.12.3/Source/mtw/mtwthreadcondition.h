/*
 * mtwthreadcondition.h
 *
 * This file is a part of the multithread wrapper for NSIS.
 * 
 * This condition code was taken from TinyCThread.
 *
 * Copyright (c) 2012 Marcus Geelnard
 * Copyright (c) 2013-2016 Evan Nemerson
 *
 * zlib/libpng license.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __MTWTHREADCONDITION_H__
#define __MTWTHREADCONDITION_H__

#ifdef _WIN32

#if defined (__cplusplus)
extern "C" {
#endif

#include "../Platform.h"
#include "mtwthreads.h"

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

#if defined (__cplusplus)
}
#endif

#endif // ~WIN32

#endif
