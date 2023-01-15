/*
   LZ4 - Fast LZ compression algorithm
   Header File
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
 * lz4compress.h
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

#ifndef __LZ4COMPRESS_H
#define __LZ4COMPRESS_H

#include "lz4common.h"

#if defined (__cplusplus)
extern "C" {
#endif

/*
 * lz4.h provides block compression functions, and gives full buffer control to programmer.
 * If you need to generate inter-operable compressed data (respecting LZ4 frame specification),
 * and can let the library handle its own memory, please use lz4frame.h instead.
*/

/**************************************
*  Local Utils
**************************************/
int LZ4_sizeofState();


/**************************************
*  Simple Functions
**************************************/

int LZ4_compress_default(const char* source, char* dest, int sourceSize, int maxDestSize);
int LZ4_decompress_safe (const char* source, char* dest, int compressedSize, int maxDecompressedSize);

/*
LZ4_compress_default() :
    Compresses 'sourceSize' bytes from buffer 'source'
    into already allocated 'dest' buffer of size 'maxDestSize'.
    Compression is guaranteed to succeed if 'maxDestSize' >= LZ4_compressBound(sourceSize).
    It also runs faster, so it's a recommended setting.
    If the function cannot compress 'source' into a more limited 'dest' budget,
    compression stops *immediately*, and the function result is zero.
    As a consequence, 'dest' content is not valid.
    This function never writes outside 'dest' buffer, nor read outside 'source' buffer.
        sourceSize  : Max supported value is LZ4_MAX_INPUT_VALUE
        maxDestSize : full or partial size of buffer 'dest' (which must be already allocated)
        return : the number of bytes written into buffer 'dest' (necessarily <= maxOutputSize)
              or 0 if compression fails

LZ4_decompress_safe() :
    compressedSize : is the precise full size of the compressed block.
    maxDecompressedSize : is the size of destination buffer, which must be already allocated.
    return : the number of bytes decompressed into destination buffer (necessarily <= maxDecompressedSize)
             If destination buffer is not large enough, decoding will stop and output an error code (<0).
             If the source stream is detected malformed, the function will stop decoding and return a negative result.
             This function is protected against buffer overflow exploits, including malicious data packets.
             It never writes outside output buffer, nor reads outside input buffer.
*/


/**************************************
*  Advanced Functions
**************************************/
//#define LZ4_MAX_INPUT_SIZE        0x7E000000   /* 2 113 929 216 bytes */
//#define LZ4_COMPRESSBOUND(isize)  ((unsigned)(isize) > (unsigned)LZ4_MAX_INPUT_SIZE ? 0 : (isize) + ((isize)/255) + 16)

/*
LZ4_compress_fast() :
    Same as LZ4_compress_default(), but allows to select an "acceleration" factor.
    The larger the acceleration value, the faster the algorithm, but also the lesser the compression.
    It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed.
    An acceleration value of "1" is the same as regular LZ4_compress_default()
    Values <= 0 will be replaced by ACCELERATION_DEFAULT (see lz4.c), which is 1.
*/
int LZ4_compress_fast (const char* source, char* dest, int sourceSize, int maxDestSize, int acceleration);


/*
LZ4_compress_fast_extState() :
    Same compression function, just using an externally allocated memory space to store compression state.
    Use LZ4_sizeofState() to know how much memory must be allocated,
    and allocate it on 8-bytes boundaries (using malloc() typically).
    Then, provide it as 'void* state' to compression function.
*/
int LZ4_sizeofState(void);
int LZ4_compress_fast_extState (void* state, const char* source, char* dest, int inputSize, int maxDestSize, int acceleration);


/*
LZ4_compress_destSize() :
    Reverse the logic, by compressing as much data as possible from 'source' buffer
    into already allocated buffer 'dest' of size 'targetDestSize'.
    This function either compresses the entire 'source' content into 'dest' if it's large enough,
    or fill 'dest' buffer completely with as much data as possible from 'source'.
        *sourceSizePtr : will be modified to indicate how many bytes where read from 'source' to fill 'dest'.
                         New value is necessarily <= old value.
        return : Nb bytes written into 'dest' (necessarily <= targetDestSize)
              or 0 if compression fails
*/
int LZ4_compress_destSize (const char* source, char* dest, int* sourceSizePtr, int targetDestSize);


/***********************************************
*  Streaming Compression Functions
***********************************************/
#define LZ4_STREAMSIZE_U64 ((1 << (LZ4_MEMORY_USAGE-3)) + 4)
#define LZ4_STREAMSIZE     (LZ4_STREAMSIZE_U64 * sizeof(INT64))
/*
 * LZ4_stream_t
 * information structure to track an LZ4 stream.
 * important : init this structure content before first use !
 * note : only allocated directly the structure if you are statically linking LZ4
 *        If you are using liblz4 as a DLL, please use below construction methods instead.
 */
typedef struct { INT64 table[LZ4_STREAMSIZE_U64]; } LZ4_stream_t;

/*
 * LZ4_resetStream
 * Use this function to init an allocated LZ4_stream_t structure
 */
void LZ4_resetStream (LZ4_stream_t* streamPtr);

/*
 * LZ4_createStream will allocate and initialize an LZ4_stream_t structure
 * LZ4_freeStream releases its memory.
 * In the context of a DLL (liblz4), please use these methods rather than the static struct.
 * They are more future proof, in case of a change of LZ4_stream_t size.
 */
LZ4_stream_t* LZ4_createStream(void);
int           LZ4_freeStream (LZ4_stream_t* streamPtr);

/*
 * LZ4_loadDict
 * Use this function to load a static dictionary into LZ4_stream.
 * Any previous data will be forgotten, only 'dictionary' will remain in memory.
 * Loading a size of 0 is allowed.
 * Return : dictionary size, in bytes (necessarily <= 64 KB)
 */
int LZ4_loadDict (LZ4_stream_t* streamPtr, const char* dictionary, int dictSize);

/*
 * LZ4_compress_fast_continue
 * Compress buffer content 'src', using data from previously compressed blocks as dictionary to improve compression ratio.
 * Important : Previous data blocks are assumed to still be present and unmodified !
 * 'dst' buffer must be already allocated.
 * If maxDstSize >= LZ4_compressBound(srcSize), compression is guaranteed to succeed, and runs faster.
 * If not, and if compressed data cannot fit into 'dst' buffer size, compression stops, and function returns a zero.
 */
int LZ4_compress_fast_continue (LZ4_stream_t* streamPtr, const char* src, char* dst, int srcSize, int maxDstSize, int acceleration);

/*
 * LZ4_saveDict
 * If previously compressed data block is not guaranteed to remain available at its memory location
 * save it into a safer place (char* safeBuffer)
 * Note : you don't need to call LZ4_loadDict() afterwards,
 *        dictionary is immediately usable, you can therefore call LZ4_compress_fast_continue()
 * Return : saved dictionary size in bytes (necessarily <= dictSize), or 0 if error
 */
int LZ4_saveDict (LZ4_stream_t* streamPtr, char* safeBuffer, int dictSize);

/* NSIS wrappers */

typedef struct _LZ4_COMPSTATE {
#ifdef _LZ4_STREAMDICT
    LZ4_stream_t lz4Stream;
    char pDictSave[65536];
    char *pInOverflow; /* For overflow input data incase compression is increasing the size */
    int inOverflow;
#endif
    char pInBufBase[LZ4_BLOCK_DATA_SIZE];
    char *pInBuf;
    char pEncBuf[LZ4_BLOCK_BUF_SIZE + LZ4_BLOCK_HEADER_SIZE];
    int encCount;
    int recurse;
    int dictSize;
    int level;
    int finish;
  } LZ4_COMPSTATE;

typedef struct
{
  /* io */
  char *next_in;          /* next input byte */
  unsigned int avail_in;  /* number of bytes available at next_in */

  char *next_out;         /* next output byte should be put there */
  unsigned int avail_out; /* remaining free space at next_out */
  
  LZ4_COMPSTATE LZ4_State;
} lz4_compstream;

int LZ4_Init(lz4_compstream *stream, int level, int dict_size);
int LZ4_End(lz4_compstream *stream);
int LZ4_Compress(lz4_compstream *stream, int finish);


#if defined (__cplusplus)
}
#endif

#endif //__LZ4COMPRESS_H
