/*
   LZ4 - Fast LZ compression algorithm
   Copyright (C) 2011-2015, Yann Collet.

   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   - LZ4 source repository : https://github.com/Cyan4973/lz4
   - LZ4 public forum : https://groups.google.com/forum/#!forum/lz4c
*/

/*
 * lz4common.h
 * 
 * This file is a part of the LZ4 compression module for NSIS.
 * 
 * Copyright (C) 2016-2017 Jason Ross (JasonFriday13)
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __LZ4COMMON_H
#define __LZ4COMMON_H

#define LZ4_MAX_INPUT_SIZE        0x7E000000   /* 2 113 929 216 bytes */
#define LZ4_COMPRESSBOUND(isize)  ((unsigned)(isize) > (unsigned)LZ4_MAX_INPUT_SIZE ? 0 : (isize) + ((isize)/255) + 16)

#define LZ4_STREAM_END 1
#define LZ4_OK 0
#define LZ4_DATA_ERROR -2
/* we don't really care what the problem is... */
#define LZ4_NOT_ENOUGH_MEM -2

#define LZ4_BLOCK_HEADER_SIZE  2
#define LZ4_BLOCK_BASE_SIZE    65536
#define LZ4_BLOCK_DATA_SIZE    (LZ4_BLOCK_BASE_SIZE - (LZ4_COMPRESSBOUND(LZ4_BLOCK_BASE_SIZE + 2) - LZ4_BLOCK_BASE_SIZE))
#if LZ4_BLOCK_BASE_SIZE > 65536
  #error Block sizes bigger than 65536 are not supported at this time.
#endif
#define LZ4_BLOCK_SIZE         (LZ4_BLOCK_HEADER_SIZE + LZ4_BLOCK_DATA_SIZE)
#define LZ4_BLOCK_BUF_SIZE     (LZ4_COMPRESSBOUND(LZ4_BLOCK_DATA_SIZE + 2)) /* Add 2 to prevent rounding down errors */

#define _LZ4_STREAMDICT

#if !defined(_LZ4_STREAMDICT)
  #define _LZ4_BLOCKONLY
#endif


/**************************************
*  Version
**************************************/
#define LZ4_VERSION_MAJOR    1    /* for breaking interface changes  */
#define LZ4_VERSION_MINOR    7    /* for new (non-breaking) interface capabilities */
#define LZ4_VERSION_RELEASE  1    /* for tweaks, bug-fixes, or development */
#define LZ4_VERSION_NUMBER (LZ4_VERSION_MAJOR *100*100 + LZ4_VERSION_MINOR *100 + LZ4_VERSION_RELEASE)
int LZ4_versionNumber (void);


/**************************************
*  Tuning parameter
**************************************/
/*
 * LZ4_MEMORY_USAGE :
 * Memory usage formula : N->2^N Bytes (examples : 10 -> 1KB; 12 -> 4KB ; 16 -> 64KB; 20 -> 1MB; etc.)
 * Increasing memory usage improves compression ratio
 * Reduced memory usage can improve speed, due to cache effect
 * Default value is 14, for 16KB, which nicely fits into Intel x86 L1 cache
 */
#define LZ4_MEMORY_USAGE 14


#include <stdlib.h>   /* malloc, calloc, free */
#include "../Platform.h"
#ifdef _WIN32
#include <windows.h>
#define ALLOCATOR(n,s) GlobalAlloc(GPTR, ((n)*(s)))
#define FREEMEM        GlobalFree
#else
#define ALLOCATOR(n,s) calloc(n,s)
#define FREEMEM        free
#endif
#include <string.h>   /* memset, memcpy */
#ifdef EXEHEAD
#ifndef MEM_INIT
void *MEM_INIT(void *mem, int c, size_t len);
#endif
#ifndef MEM_CPY
void *MEM_CPY(void *out, const void *in, size_t len);
#endif
#ifndef MEM_MOVE
void *MEM_MOVE(void *out, const void *in, size_t len);
#endif
#else
#ifndef MEM_INIT
#define MEM_INIT       memset
#endif
#ifndef MEM_CPY
#define MEM_CPY        memcpy
#endif
#ifndef MEM_MOVE
#define MEM_MOVE       memmove
#endif
#endif

/**************************************
*  CPU Feature Detection
**************************************/
/*
 * LZ4_FORCE_SW_BITCOUNT
 * Define this parameter if your target system or compiler does not support hardware bit count
 */
#if defined(_MSC_VER) && defined(_WIN32_WCE)   /* Visual Studio for Windows CE does not support Hardware bit count */
#  define LZ4_FORCE_SW_BITCOUNT
#endif

/**************************************
*  Compiler Options
**************************************/

/* LZ4_GCC_VERSION is defined into lz4.h */
#if (LZ4_GCC_VERSION >= 302) || (__INTEL_COMPILER >= 800) || defined(__clang__)
#  define expect(expr,value)    (__builtin_expect ((expr),(value)) )
#else
#  define expect(expr,value)    (expr)
#endif

#define likely(expr)     expect((expr) != 0, 1)
#define unlikely(expr)   expect((expr) != 0, 0)


/**************************************
*  Basic Types
**************************************/
#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)   /* C99 */
# include <stdint.h>
  typedef  uint8_t BYTE;
  typedef uint16_t U16;
  typedef uint32_t U32;
  typedef  int32_t S32;
  typedef uint64_t U64;
#else
  typedef unsigned char       BYTE;
  typedef unsigned short      U16;
  typedef unsigned int        U32;
  typedef   signed int        S32;
  typedef          UINT64     U64;
#endif


/**************************************
*  Reading and writing into memory
**************************************/
#define STEPSIZE sizeof(size_t)

unsigned LZ4_64bits(void);
unsigned LZ4_isLittleEndian(void);
U16 LZ4_read16(const void* memPtr);
U16 LZ4_readLE16(const void* memPtr);
void LZ4_writeLE16(void* memPtr, U16 value);
U32 LZ4_read32(const void* memPtr);
U64 LZ4_read64(const void* memPtr);
size_t LZ4_read_ARCH(const void* p);

void LZ4_copy4(void* dstPtr, const void* srcPtr);
void LZ4_copy8(void* dstPtr, const void* srcPtr);

/* customized version of memcpy, which may overwrite up to 7 bytes beyond dstEnd */
void LZ4_wildCopy(void* dstPtr, const void* srcPtr, void* dstEnd);


/**************************************
*  Common Constants
**************************************/
#define MINMATCH 4

#define COPYLENGTH 8
#define LASTLITERALS 5
#define MFLIMIT (COPYLENGTH+MINMATCH)
static const int LZ4_minLength = (MFLIMIT+1);

#define KB *(1 <<10)
#define MB *(1 <<20)
#define GB *(1U<<30)

#define ML_BITS  4
#define ML_MASK  ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)


/**************************************
*  Common Utils
**************************************/
#define LZ4_STATIC_ASSERT(c)    { enum { LZ4_static_assert = 1/(int)(!!(c)) }; }   /* use only *after* variable declarations */


/**************************************
*  Common functions
**************************************/
unsigned LZ4_NbCommonBytes (size_t val);

unsigned LZ4_count(const BYTE* pIn, const BYTE* pMatch, const BYTE* pInLimit);


/**************************************
*  Local Constants
**************************************/
#define LZ4_HASHLOG   (LZ4_MEMORY_USAGE-2)
//#define HASHTABLESIZE (1 << LZ4_MEMORY_USAGE)
#define HASH_SIZE_U32 (1 << LZ4_HASHLOG)       /* required as macro for static allocation */

static const int LZ4_64Klimit = ((64 KB) + (MFLIMIT-1));
static const U32 LZ4_skipTrigger = 6;  /* Increase this value ==> compression run slower on incompressible data */


#ifndef LZ4_COMMONDEFS_ONLY
/**************************************
*  Local Structures and types
**************************************/
typedef struct {
    U32 hashTable[HASH_SIZE_U32];
    U32 currentOffset;
    U32 initCheck;
    const BYTE* dictionary;
    BYTE* bufferStart;   /* obsolete, used for slideInputBuffer */
    U32 dictSize;
} LZ4_stream_t_internal;

typedef enum { notLimited = 0, limitedOutput = 1 } limitedOutput_directive;
typedef enum { byPtr, byU32, byU16 } tableType_t;

typedef enum { noDict = 0, withPrefix64k, usingExtDict } dict_directive;
typedef enum { noDictIssue = 0, dictSmall } dictIssue_directive;

typedef enum { endOnOutputSize = 0, endOnInputSize = 1 } endCondition_directive;
typedef enum { full = 0, partial = 1 } earlyEnd_directive;


/**************************************
*  Local Utils
**************************************/
int LZ4_versionNumber (void);

/*
LZ4_compressBound() :
    Provides the maximum size that LZ4 compression may output in a "worst case" scenario (input data not compressible)
    This function is primarily useful for memory allocation purposes (destination buffer size).
    Macro LZ4_COMPRESSBOUND() is also provided for compilation-time evaluation (stack memory allocation for example).
    Note that LZ4_compress_default() compress faster when dest buffer size is >= LZ4_compressBound(srcSize)
        inputSize  : max supported value is LZ4_MAX_INPUT_SIZE
        return : maximum output size in a "worst case" scenario
              or 0, if input size is too large ( > LZ4_MAX_INPUT_SIZE)
*/
int LZ4_compressBound(int inputSize);

#endif


#endif
