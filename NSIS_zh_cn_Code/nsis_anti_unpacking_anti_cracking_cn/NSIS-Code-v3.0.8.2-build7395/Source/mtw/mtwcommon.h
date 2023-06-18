/*
 * mtwcommon.h
 *
 * This file is a part of the multithread wrapper for NSIS.
 * 
 * Copyright (C) 2021-2023 Jason Ross (JasonFriday13)
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __MTWCOMMON_H__
#define __MTWCOMMON_H__

#if defined (__cplusplus)
extern "C" {
#endif

#include "linkedlist.h"
#include "mtwthreads.h"

#ifdef _WIN32
  #include <windows.h>
  #define _ALLOC(x) GlobalAlloc(GPTR, (x))
  #define _FREE GlobalFree
#else
  #include <stdlib.h>
  #define _ALLOC(x) malloc(x)
  #define _FREE  free
#endif

#define MTW_STREAM_END 1
#define MTW_OK 0
#define MTW_DATA_ERROR -2
/* we don't really care what the problem is... */
#define MTW_NOT_ENOUGH_MEM -2

#define MTW_BLOCK_HEADER_SIZE  3
#define MTW_BLOCK_DATA_SIZE    (1 << 20) // 1MB
#define MTW_BLOCK_SIZE         (MTW_BLOCK_HEADER_SIZE + MTW_BLOCK_DATA_SIZE)
#define MTW_BLOCK_BUF_SIZE     (MTW_BLOCK_DATA_SIZE + 1024 + (MTW_BLOCK_DATA_SIZE / 10))

typedef struct
{
  struct VARLIST handle;
  MT_MUTEX_T mutex;
} DATA_HANDLE;

#if defined (__cplusplus)
}
#endif

#endif
