/*
 * mtwdecompress.c
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

#include "../Platform.h"
#include "mtwcommon.h"
#include "mtwthreads.h"
#include "mtwdecompress.h"

enum {
  MTWD_START = 0,
  MTWD_GETLENGTHFIRST,
  MTWD_GETLENGTHSECOND,
  MTWD_GETLENGTHTHIRD,
  MTWD_COPYIN,
  MTWD_COPYINFULL,
  MTWD_COPYINPARTIAL,
  MTWD_DECOMPRESSBLOCK,
  MTWD_PROCESSSTATE,
  MTWD_HAVEDECOMPRESSED,
  MTWD_HAVEOUT,
  MTWD_STREAMEND,
} MTWD_STAGE;

#ifdef NSIS_COMPRESS_USE_ZLIB
#include "../zlib/ZLIB.H"
#define inflateEnd(x)
#endif

#ifdef NSIS_COMPRESS_USE_BZIP2
#include "../bzip2/bzlib.h"

#define z_stream DState
#define inflateInit(x) BZ2_bzDecompressInit(x)
#define inflateReset(x) BZ2_bzDecompressInit(x)

#define inflate(x) BZ2_decompress(x)
#define inflateEnd(x)
#define Z_OK BZ_OK
#define Z_STREAM_END BZ_STREAM_END
#endif//NSIS_COMPRESS_USE_BZIP2

#ifdef NSIS_COMPRESS_USE_LZMA
#include "../7zip/LZMADecode.h"
#define z_stream lzma_stream
#define inflateInit(x) lzmaInit(x)
#define inflateReset(x) lzmaInit(x)
#define inflate(x) lzmaDecode(x)
#define inflateEnd(x) lzmaEnd(x)
#define Z_OK LZMA_OK
#define Z_STREAM_END LZMA_STREAM_END
#endif

#ifdef NSIS_COMPRESS_USE_LZ4
#include "../lz4/lz4decompress.h"
#define z_stream lz4_decstream
#define inflateInit(x) lz4Init(x)
#define inflateReset(x) lz4Init(x)
#define inflate(x) lz4Decode(x)
#define inflateEnd(x)
#define Z_OK LZ4_OK
#define Z_STREAM_END LZ4_STREAM_END
#endif

enum {
  D_CHECK_INPUT = 0,
  D_DECOMPRESS,
  D_WRITE_OUT,
  D_EXIT,
};

DATA_HANDLE d_data_out;
MT_THREAD_CTX *d_ctx;

unsigned int decompress_thread(void *arg);

UINT64 d_in_seq_number = 0, d_out_seq_number = 0;
int d_in_finish = 0, d_out_finish = 0, finished = 0;
size_t d_num_threads = 1;
unsigned int init = 0;

typedef struct 
{
  char *data;
  unsigned int size;
  UINT64 seq_num;
} JOB;

int set_next_in(const char *in, unsigned int size)
{
  size_t i = 0;
  JOB *job = (JOB*)_ALLOC(sizeof(JOB));
  if (!job) return 0;

  job->data = (char*)_ALLOC(size);
  if (!job->data) return 0;

  for (i = 0; i < size; i++)
    job->data[i] = in[i];
  job->size = size;
  job->seq_num = d_in_seq_number;

  mt_add(d_ctx, (data_function)decompress_thread, (void*)job);
  return 1;
}

void set_d_in_finish(int finish)
{
  if (finish && !finished)
  {
    finished++;
  }
}

// Returns true if the input queue is empty.
UINT64 check_next_out(void)
{
  UINT64 ret;
  MT_MUTEX_LOCK(&d_data_out.mutex);
  ret = !finished ? GetListCount(&d_data_out.handle) : 0;
  MT_MUTEX_UNLOCK(&d_data_out.mutex);
  return ret;
}

// Returns true if the next output block is ready.
int have_next_out(void)
{
  int count;
  MT_MUTEX_LOCK(&d_data_out.mutex);
  count = FindSeqNum(d_out_seq_number, &d_data_out.handle);
  MT_MUTEX_UNLOCK(&d_data_out.mutex);
  return count;
}

char *d_get_next_out(char *out, int *ret_value)
{
  char *out_used = NULL;
  if (!have_next_out()) return NULL;
  MT_MUTEX_LOCK(&d_data_out.mutex);
  if (FindSeqNum(d_out_seq_number, &d_data_out.handle))
    out_used = GetData(out, d_out_seq_number, ret_value, &d_data_out.handle);
  MT_MUTEX_UNLOCK(&d_data_out.mutex);
  return out_used;
}

unsigned int d_get_next_out_size(void)
{
  unsigned int size = 0;
  MT_MUTEX_LOCK(&d_data_out.mutex);
  if (FindSeqNum(d_out_seq_number, &d_data_out.handle))
    size = GetDataLen(d_out_seq_number, &d_data_out.handle);
  MT_MUTEX_UNLOCK(&d_data_out.mutex);
  return size;
}

unsigned int decompress_thread(void *arg)
{
  JOB *job = (JOB*)arg;
  z_stream *inflate_stream;
  int ret = Z_OK;
  char *out = NULL;
  const unsigned int out_avail = MTW_BLOCK_BUF_SIZE;
  unsigned int out_len = 1;

  inflate_stream = (z_stream*)_ALLOC(sizeof(z_stream));
  out = (char*)_ALLOC(out_avail);
  if (inflate_stream && out)
  {
    inflateReset(inflate_stream);
    inflate_stream->next_in = (unsigned char*)job->data;
    inflate_stream->avail_in = job->size;

    inflate_stream->next_out = (unsigned char*)out;
    inflate_stream->avail_out = out_avail;

    while (inflate_stream->avail_in && ret == 0)
    {
      ret = inflate(inflate_stream);
    }
    out_len = (unsigned int)((char*)inflate_stream->next_out - out);
    if (ret < 0)
      out_len = 1;
  }
  else
  {
    ret = MTW_NOT_ENOUGH_MEM;
  }
  MT_MUTEX_LOCK(&d_data_out.mutex);
  AddData(out, out_len, job->seq_num, ret, &d_data_out.handle);
  MT_MUTEX_UNLOCK(&d_data_out.mutex);

  inflateEnd(inflate_stream);
  if (job->data) _FREE(job->data);
  if (out) _FREE(out);
  if (inflate_stream) _FREE(inflate_stream);
  if (job) _FREE(job);
  return 0;
}

int dmtwInit(mtw_decstream *s, unsigned int num_threads, int is_verbose)
{
  int i, error = 0;

  if (init) dmtwEnd(s);
  for (i = 0; i < sizeof(s->MTW_State); i++)
    *((char *)&s->MTW_State + i) = 0;

  d_in_seq_number = 0;
  d_out_seq_number = 0;
  d_in_finish = 0;
  d_out_finish = 0;
  d_num_threads = num_threads;
  finished = 0;

  error |= MT_MUTEX_INIT(&d_data_out.mutex, NULL);
  MT_MUTEX_LOCK(&d_data_out.mutex);
  InitVarList(&d_data_out.handle);
  MT_MUTEX_UNLOCK(&d_data_out.mutex);

  d_ctx = mt_create(num_threads);
  error |= !d_ctx;

  s->MTW_State.pInBuf = (char *)_ALLOC(MTW_BLOCK_BUF_SIZE);
  s->MTW_State.pDecBuf = (char *)_ALLOC(MTW_BLOCK_DATA_SIZE);
  error |= !s->MTW_State.pInBuf;
  error |= !s->MTW_State.pDecBuf;

  init = 1;
  return !error;
}

void dmtwEnd(mtw_decstream *s)
{
  if (!init) return;

  d_in_seq_number = d_out_seq_number = 0;
  MT_MUTEX_LOCK(&d_data_out.mutex);
  CleanVarList(&d_data_out.handle);
  MT_MUTEX_UNLOCK(&d_data_out.mutex);
  MT_MUTEX_DESTROY(&d_data_out.mutex);
  if (d_ctx) mt_free(d_ctx), d_ctx = NULL;

  if (s->MTW_State.pDecBuf) _FREE(s->MTW_State.pDecBuf), s->MTW_State.pDecBuf = NULL;
  if (s->MTW_State.pInBuf) _FREE(s->MTW_State.pInBuf), s->MTW_State.pInBuf = NULL;
  init = 0;
}

int dmtwDecode(mtw_decstream *s)
{
  for (;;)
  {
    switch (s->MTW_State.stage)
    {
    case MTWD_START:
    case MTWD_GETLENGTHFIRST:
      if (s->avail_in < 1)
      {
        s->MTW_State.stage = MTWD_PROCESSSTATE;
        break;
      }
      s->MTW_State.blockLength = 0;
      s->MTW_State.blockLength = (unsigned char)*s->next_in++, --s->avail_in;
      s->MTW_State.stage = MTWD_GETLENGTHSECOND;
    case MTWD_GETLENGTHSECOND:
      if (s->avail_in < 1)
        return MTW_OK;
      s->MTW_State.blockLength |= (unsigned short)(*s->next_in++ << 8) & 0xFF00, --s->avail_in;
      s->MTW_State.stage = MTWD_GETLENGTHTHIRD;
    case MTWD_GETLENGTHTHIRD:
      if (s->avail_in < 1)
        return MTW_OK;
      s->MTW_State.blockLength |= (unsigned int)(*s->next_in++ << 16) & 0x00FF0000, --s->avail_in;
      s->MTW_State.stage = MTWD_COPYIN;
    case MTWD_COPYIN:
      if (s->MTW_State.blockLength == 0 && !s->avail_in)
      {
        s->MTW_State.stage = MTWD_PROCESSSTATE; // aka input stream end
        d_in_finish++;
        break;
      }
      s->MTW_State.stage = s->MTW_State.blockLength < s->avail_in ? MTWD_COPYINFULL : MTWD_COPYINPARTIAL;
      break;
    case MTWD_COPYINFULL:
      for (s->MTW_State.inCount = 0; s->MTW_State.inCount < s->MTW_State.blockLength; s->MTW_State.inCount++)
        *(s->MTW_State.pInBuf + s->MTW_State.inCount) = *s->next_in++, --s->avail_in;
      s->MTW_State.stage = MTWD_DECOMPRESSBLOCK;
      break;
    case MTWD_COPYINPARTIAL:
      {
        unsigned int i, avail = s->MTW_State.blockLength - s->MTW_State.inCount < s->avail_in ? s->MTW_State.blockLength - s->MTW_State.inCount : s->avail_in;
        for (i = 0; i < avail; i++)
          *(s->MTW_State.pInBuf + s->MTW_State.inCount + i) = *s->next_in++, --s->avail_in;
        s->MTW_State.inCount += i;
        if (s->MTW_State.inCount < s->MTW_State.blockLength) return MTW_OK;
        s->MTW_State.stage = MTWD_DECOMPRESSBLOCK;
        break;
      }
    case MTWD_DECOMPRESSBLOCK:
      {
        if (set_next_in((char*)s->MTW_State.pInBuf, s->MTW_State.inCount))
        {
          s->MTW_State.inCount = 0;
          d_in_seq_number++;
          s->MTW_State.stage = MTWD_PROCESSSTATE;
          break;
        }
      }
    break;
    case MTWD_PROCESSSTATE:
      {
        // flush the output buffer first if we have one
        if (s->MTW_State.pDecSeeker)
        {
          if (s->MTW_State.pDecSeeker != s->MTW_State.pDecBufEnd)
          {
            if (s->avail_out)
            {
              s->MTW_State.stage = MTWD_HAVEOUT;
              break;
            }
            else
              return MTW_OK;
          }
          else
          {
            s->MTW_State.pDecSeeker = NULL;
            break;
          }
        }
        else // otherwise continue the processing loop
        {
          if (/*check_next_out() &&*/ have_next_out() && !d_out_finish)
          {
            s->MTW_State.stage = MTWD_HAVEDECOMPRESSED;
            break;
          }
          // check for stream end
          if (d_in_finish && !check_next_out() && d_in_seq_number == d_out_seq_number)
          {
            d_out_finish++;
            s->MTW_State.stage = MTWD_STREAMEND;
            break;
          }
          if (!d_in_finish && check_next_out() < d_num_threads)
          {
            if (s->avail_in)
            {
              s->MTW_State.stage = MTWD_START;
              break;
            }
            else
              return MTW_OK;
          }
          // continue waiting
          break;
        }
      }
    break;
    case MTWD_HAVEDECOMPRESSED:
      {
        int ret;
        s->MTW_State.pDecBufEnd = d_get_next_out(s->MTW_State.pDecBuf, &ret);
        if (s->MTW_State.pDecBufEnd != NULL && ret >= 0)
        {
          s->MTW_State.pDecSeeker = s->MTW_State.pDecBuf;
          d_out_seq_number++;
          s->MTW_State.stage = MTWD_PROCESSSTATE;
          break;
        }
        else
        {
          s->MTW_State.stage = MTWD_STREAMEND;
          dmtwEnd(s);
          return MTW_DATA_ERROR;
        }
      }
    break;
    case MTWD_HAVEOUT:
      {
        size_t i, outbytes = s->avail_out > (unsigned int)(s->MTW_State.pDecBufEnd - s->MTW_State.pDecSeeker) ? (unsigned int)(s->MTW_State.pDecBufEnd - s->MTW_State.pDecSeeker) : s->avail_out;

        for (i = 0; i < outbytes; i++)
          *s->next_out++ = *s->MTW_State.pDecSeeker++, s->avail_out--;

        s->MTW_State.stage = MTWD_PROCESSSTATE;
      }
    break;
    case MTWD_STREAMEND:
      dmtwEnd(s);
      return MTW_STREAM_END;
    }
  }
  return MTW_OK;
}
