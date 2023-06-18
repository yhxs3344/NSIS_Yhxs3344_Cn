/*
 * mtwdecompress.h
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

#ifndef _MTWDECOMPRESS_H_
#define _MTWDECOMPRESS_H_

#if defined (__cplusplus)
extern "C" {
#endif

#define MTW_STREAM_END 1
#define MTW_OK 0
#define MTW_DATA_ERROR -2
/* we don't really care what the problem is... */
#define MTW_NOT_ENOUGH_MEM -2

typedef struct _MTW_DECSTATE {
    char *pInBuf;
    unsigned int inCount;
    unsigned int blockLength;
    char *pDecBuf;
    char *pDecSeeker;
    char *pDecBufEnd;

    unsigned int num_threads;

    int stage;
  } MTW_DECSTATE;

typedef struct
{
  /* io */
  unsigned char *next_in;  /* next input byte */
  unsigned int avail_in;   /* number of bytes available at next_in */

  unsigned char *next_out; /* next output byte should be put there */
  unsigned int avail_out;  /* remaining free space at next_out */

  MTW_DECSTATE MTW_State;
} mtw_decstream;

int dmtwInit(mtw_decstream *s, unsigned int num_threads, int is_verbose);
int dmtwDecode(mtw_decstream *s);
void dmtwEnd(mtw_decstream *s);


#if defined (__cplusplus)
}
#endif

#endif //__LZ4DECOMPRESS_H
