/*
 * mt_lz4_decompress.c
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

#include "../Platform.h"
#include "../lz4/lz4decompress.h"
#include "mtwcommon.h"
#include "mtwthreads.h"
#include "mtwjob.h"
#include "mt_lz4_common.h"
#include "mt_lz4_decompress.h"

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

unsigned int decompress_thread(void *arg)
{
  PMTWJOB job = (PMTWJOB)arg;
  lz4_decstream *inflate_stream = (lz4_decstream*)job->optional;

  lz4Init(inflate_stream);
  inflate_stream->next_in = (unsigned char*)job->input;
  inflate_stream->avail_in = job->input_size;
  inflate_stream->next_out = (unsigned char*)job->output;
  inflate_stream->avail_out = MT_LZ4_BLOCK_DATA_SIZE;
  job->ret_value = lz4Decode(inflate_stream);
  job->output_size = (unsigned int)((char*)inflate_stream->next_out - job->output);

  FinishJob(job);
  _FREE(inflate_stream);
  _FREE(job);
  return 0;
}

int add_threaded_job(mtlz4_decstream *s)
{
  void *inflate_stream = _ALLOC(sizeof(lz4_decstream));
  PMTWJOB job;
  if (!inflate_stream) return 0;
  job = MakeJob(s->mt_lz4_state.pInBuf, s->mt_lz4_state.inCount, MT_LZ4_BLOCK_SIZE, s->mt_lz4_state.d_in_seq_number, inflate_stream);
  if (!job) return 0;

  mt_add(s->mt_lz4_state.d_ctx, (data_function)decompress_thread, (void*)job);
  return 1;
}

int dmtlz4Init(mtlz4_decstream *s, unsigned int num_threads)
{
  int i, error = 0;

  if (s->mt_lz4_state.init) dmtlz4End(s);
  for (i = 0; i < sizeof(s->mt_lz4_state); i++)
    *((char *)&s->mt_lz4_state + i) = 0;

  s->mt_lz4_state.d_in_seq_number = s->mt_lz4_state.d_out_seq_number = 0;
  s->mt_lz4_state.d_in_finish = 0;
  s->mt_lz4_state.d_num_threads = num_threads;

  s->mt_lz4_state.d_ctx = mt_create(num_threads);
  error |= !s->mt_lz4_state.d_ctx;

  s->mt_lz4_state.pInBuf = (char *)_ALLOC(MT_LZ4_BLOCK_BUF_SIZE);
  s->mt_lz4_state.pDecBuf = (char *)_ALLOC(MT_LZ4_BLOCK_DATA_SIZE);
  error |= !s->mt_lz4_state.pInBuf;
  error |= !s->mt_lz4_state.pDecBuf;
  error |= !InitJobContext();

  s->mt_lz4_state.init = !error;
  return s->mt_lz4_state.init;
}

void dmtlz4End(mtlz4_decstream *s)
{
  if (!s->mt_lz4_state.init) return;

  s->mt_lz4_state.d_in_seq_number = s->mt_lz4_state.d_out_seq_number = 0;
  FreeJobContext();
  if (s->mt_lz4_state.d_ctx) mt_free(s->mt_lz4_state.d_ctx), s->mt_lz4_state.d_ctx = NULL;

  if (s->mt_lz4_state.pDecBuf) _FREE(s->mt_lz4_state.pDecBuf), s->mt_lz4_state.pDecBuf = NULL;
  if (s->mt_lz4_state.pInBuf) _FREE(s->mt_lz4_state.pInBuf), s->mt_lz4_state.pInBuf = NULL;
  s->mt_lz4_state.init = 0;
}

int dmtlz4Decode(mtlz4_decstream *s)
{
  for (;;)
  {
    switch (s->mt_lz4_state.stage)
    {
    case MTWD_START:
    case MTWD_GETLENGTHFIRST:
      if (s->avail_in < 1)
      {
        s->mt_lz4_state.stage = MTWD_PROCESSSTATE;
        break;
      }
      s->mt_lz4_state.blockLength = 0;
      s->mt_lz4_state.blockLength = (unsigned char)*s->next_in++, --s->avail_in;
      s->mt_lz4_state.stage = MTWD_GETLENGTHSECOND;
    case MTWD_GETLENGTHSECOND:
      if (s->avail_in < 1)
        return LZ4_OK;
      s->mt_lz4_state.blockLength |= (unsigned short)(*s->next_in++ << 8) & 0xFF00, --s->avail_in;
      s->mt_lz4_state.stage = MTWD_GETLENGTHTHIRD;
    case MTWD_GETLENGTHTHIRD:
      if (s->avail_in < 1)
        return LZ4_OK;
      s->mt_lz4_state.blockLength |= (unsigned int)(*s->next_in++ << 16) & 0x00FF0000, --s->avail_in;
      s->mt_lz4_state.stage = MTWD_COPYIN;
    case MTWD_COPYIN:
      if (s->mt_lz4_state.blockLength == 0 && !s->avail_in)
      {
        s->mt_lz4_state.stage = MTWD_PROCESSSTATE; // aka input stream end
        s->mt_lz4_state.d_in_finish++;
        break;
      }
      s->mt_lz4_state.stage = s->mt_lz4_state.blockLength < s->avail_in ? MTWD_COPYINFULL : MTWD_COPYINPARTIAL;
      break;
    case MTWD_COPYINFULL:
      for (s->mt_lz4_state.inCount = 0; s->mt_lz4_state.inCount < s->mt_lz4_state.blockLength; s->mt_lz4_state.inCount++)
        *(s->mt_lz4_state.pInBuf + s->mt_lz4_state.inCount) = *s->next_in++, --s->avail_in;
      s->mt_lz4_state.stage = MTWD_DECOMPRESSBLOCK;
      break;
    case MTWD_COPYINPARTIAL:
      {
        unsigned int i, avail = s->mt_lz4_state.blockLength - s->mt_lz4_state.inCount < s->avail_in ? s->mt_lz4_state.blockLength - s->mt_lz4_state.inCount : s->avail_in;
        for (i = 0; i < avail; i++)
          *(s->mt_lz4_state.pInBuf + s->mt_lz4_state.inCount + i) = *s->next_in++, --s->avail_in;
        s->mt_lz4_state.inCount += i;
        if (s->mt_lz4_state.inCount < s->mt_lz4_state.blockLength) return LZ4_OK;
        s->mt_lz4_state.stage = MTWD_DECOMPRESSBLOCK;
        break;
      }
    case MTWD_DECOMPRESSBLOCK:
      {
        if (add_threaded_job(s))
        {
          s->mt_lz4_state.inCount = 0;
          s->mt_lz4_state.d_in_seq_number++;
          s->mt_lz4_state.stage = MTWD_PROCESSSTATE;
          break;
        }
        else
        {
          s->mt_lz4_state.stage = MTWD_STREAMEND;
          dmtlz4End(s);
          return LZ4_NOT_ENOUGH_MEM;
        }
      }
    break;
    case MTWD_PROCESSSTATE:
      {
        // flush the output buffer first if we have one
        if (s->mt_lz4_state.pDecSeeker)
        {
          if (s->mt_lz4_state.pDecSeeker != s->mt_lz4_state.pDecBufEnd)
          {
            if (s->avail_out)
            {
              s->mt_lz4_state.stage = MTWD_HAVEOUT;
              break;
            }
            else
              return LZ4_OK;
          }
          else
          {
            s->mt_lz4_state.pDecSeeker = NULL;
            break;
          }
        }
        else // otherwise continue the processing loop
        {
          if (JobHaveNextOut(s->mt_lz4_state.d_out_seq_number))
          {
            s->mt_lz4_state.stage = MTWD_HAVEDECOMPRESSED;
            break;
          }
          // check for stream end
          if (s->mt_lz4_state.d_in_finish && !JobCountNextOut() && s->mt_lz4_state.d_in_seq_number == s->mt_lz4_state.d_out_seq_number)
          {
            dmtlz4End(s);
            s->mt_lz4_state.stage = MTWD_STREAMEND;
            break;
          }
          // add more data if we have room
          if (!s->mt_lz4_state.d_in_finish && JobCountNextOut() < s->mt_lz4_state.d_num_threads)
          {
            if (s->avail_in)
            {
              s->mt_lz4_state.stage = MTWD_START;
              break;
            }
            else
              return LZ4_OK;
          }
          // continue waiting
          break;
        }
      }
    break;
    case MTWD_HAVEDECOMPRESSED:
      {
        int ret;
        s->mt_lz4_state.pDecBufEnd = JobGetNextOut(s->mt_lz4_state.pDecBuf, s->mt_lz4_state.d_out_seq_number, &ret);
        if (s->mt_lz4_state.pDecBufEnd != NULL && ret >= 0)
        {
          s->mt_lz4_state.pDecSeeker = s->mt_lz4_state.pDecBuf;
          s->mt_lz4_state.d_out_seq_number++;
          s->mt_lz4_state.stage = MTWD_PROCESSSTATE;
          break;
        }
        else
        {
          s->mt_lz4_state.stage = MTWD_STREAMEND;
          dmtlz4End(s);
          return LZ4_DATA_ERROR;
        }
      }
    break;
    case MTWD_HAVEOUT:
      {
        size_t i, outbytes = s->avail_out > (unsigned int)(s->mt_lz4_state.pDecBufEnd - s->mt_lz4_state.pDecSeeker) ? (unsigned int)(s->mt_lz4_state.pDecBufEnd - s->mt_lz4_state.pDecSeeker) : s->avail_out;

        for (i = 0; i < outbytes; i++)
          *s->next_out++ = *s->mt_lz4_state.pDecSeeker++, s->avail_out--;

        s->mt_lz4_state.stage = MTWD_PROCESSSTATE;
      }
    break;
    case MTWD_STREAMEND:
      return LZ4_STREAM_END;
    }
  }
  return LZ4_OK;
}
