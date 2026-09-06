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

#ifdef _WIN32
  #include <windows.h>
  #define _ALLOC(x) GlobalAlloc(GPTR, (x))
  #define _FREE GlobalFree
#else
  #include <stdlib.h>
  #define _ALLOC(x) malloc(x)
  #define _FREE  free
#endif

typedef struct {
  int compressor;
  unsigned int num_threads;
  int level;
  int dict_size;
} CINFO;

#if defined (__cplusplus)
}
#endif

#endif
