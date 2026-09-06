/*
 * mt_bzip2_decompress.h
 * 
 * This file is a part of the multithread wrapper for NSIS.
 * 
 * Copyright (C) 2021-2023, 2026 Jason Ross (JasonFriday13)
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __MTBZIP2DECOMPRESS_H__
#define __MTBZIP2DECOMPRESS_H__

#include "../mtw/mtwthreads.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct _MTBZIP2_DECSTATE {
    char *pInBuf;
    unsigned int inCount;
    unsigned int blockLength;
    char *pDecBuf;
    char *pDecSeeker;
    char *pDecBufEnd;

    UINT64 d_in_seq_number, d_out_seq_number;
    int init, d_in_finish;
    size_t d_num_threads;
    MT_THREAD_CTX *d_ctx;

    int stage;
  } MTBZIP2_DECSTATE;

typedef struct
{
  /* io */
  unsigned char *next_in;  /* next input byte */
  unsigned int avail_in;   /* number of bytes available at next_in */

  unsigned char *next_out; /* next output byte should be put there */
  unsigned int avail_out;  /* remaining free space at next_out */

  MTBZIP2_DECSTATE mt_bzip2_state;
} mtbzip2_decstream;

int dmtbzip2Init(mtbzip2_decstream *s, unsigned int num_threads);
int dmtbzip2Decode(mtbzip2_decstream *s);
void dmtbzip2End(mtbzip2_decstream *s);

#if defined (__cplusplus)
}
#endif

#endif
