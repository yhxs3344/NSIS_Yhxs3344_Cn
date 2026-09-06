/*
 * mt_lz4_compress.cpp
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
#include "mtwcommon.h"
#include "mtwjob.h"
#include "../clz4.h"
#include "../clz4mt.h"
#include "mt_lz4_common.h"

unsigned int lz4_compress_thread(void *arg)
{
  PMTWJOB job = (PMTWJOB)arg;
  ICompressor *compressor = (CLZ4*)job->optional;
  
  compressor->Init(job->comp_info.level, job->comp_info.dict_size, 0);
  compressor->SetNextIn(job->input, job->input_size);
  compressor->SetNextOut(job->output, job->output_size);
  job->ret_value = compressor->Compress(C_FINISH);
  job->output_size = (unsigned int)(compressor->GetNextOut() - job->output);
  compressor->End();

  FinishJob(job);
  delete compressor;
  _FREE(job);
  return 0;
}

// Adds data to the input queue. Returns false if allocation fails.
int CLZ4MT::add_threaded_job(const char *in, unsigned int size)
{
  ICompressor *compressor = new CLZ4;

  if (!compressor) return 0;

  PMTWJOB job = MakeJob(in, size, MT_LZ4_BLOCK_BUF_SIZE, in_seq_number, (void*)compressor);
  if (!job) return 0;

  job->comp_info.compressor = compressor_info.compressor;
  job->comp_info.dict_size = MT_LZ4_BLOCK_BUF_SIZE;  // Slight speedup if the dictionary is slightly bigger than the block size
  job->comp_info.level = compressor_info.level;
  job->comp_info.num_threads = compressor_info.num_threads;

  // Gives a slight performance increase on sizes less than the block size
  if (size < MT_LZ4_BLOCK_DATA_SIZE)
    lz4_compress_thread((void*)job);
  else
  {
    if (!c_ctx)
    {
      c_ctx = mt_create(compressor_info.num_threads);
      if (!c_ctx) return 0;
    }
    mt_add(c_ctx, (data_function)lz4_compress_thread, (void*)job);
  }
  return 1;
}

CLZ4MT::CLZ4MT()
{
  init = 0;
  out_buf = NULL;
}

CLZ4MT::~CLZ4MT()
{
  if (out_buf) _FREE(out_buf), out_buf = NULL;
}

void CLZ4MT::SetCompressor(int which_compressor)
{
  compressor_info.compressor = which_compressor;
}

int CLZ4MT::GetCompressor()
{
  return compressor_info.compressor;
}

int CLZ4MT::Init(int level, unsigned int dict_size, unsigned int num_threads)
{
  int error = 0;
  if (init) End();

  compressor_info.level = level;
  compressor_info.dict_size = dict_size;
  compressor_info.num_threads = num_threads;

  c_ctx = NULL;
  in_seq_number = out_seq_number = 1;
  in_finish = out_finish = false;
  finished = false;
  out_seeker = out_end = NULL;
  if (!out_buf)
  {
    out_buf = (char *)_ALLOC(MT_LZ4_BLOCK_BUF_SIZE);
    error |= !out_buf;
  }
  else
  {
    for (int i = 0; i < MT_LZ4_BLOCK_BUF_SIZE; ++i)
      out_buf[i] = 0;
  }
  error |= !InitJobContext();

  init = !error;
  return error ? LZ4_NOT_ENOUGH_MEM : LZ4_OK;
}

int CLZ4MT::End()
{
  if (init)
  {
    if (c_ctx) mt_free(c_ctx), c_ctx = NULL;
    out_seeker = out_end = NULL;
    FreeJobContext();
    init = false;
  }
  return LZ4_OK;
}

int CLZ4MT::Compress(bool finish)
{
  if (finished) return LZ4_STREAM_END;

  while (!(in_finish && out_finish))
  {
    if (out_seeker)
    {
      unsigned int shift = 0, header_bytes = out_seeker == out_buf ? MT_LZ4_BLOCK_HEADER_SIZE : 0;
      int i, outbytes = (avail_out > (unsigned int)(out_end - out_seeker) + header_bytes) ? (unsigned int)(out_end - out_seeker) : avail_out - header_bytes;

      if (outbytes < 1)
        return LZ4_OK;

      if (header_bytes)
      {
        unsigned int size = (unsigned int)(out_end - out_buf);
        for (i = 0; i < MT_LZ4_BLOCK_HEADER_SIZE; i++, shift += 8)
          *next_out++ = (unsigned char)(size >> shift), avail_out--;
      }

      for (i = 0; i < outbytes; i++)
        *next_out++ = *out_seeker++, avail_out--;

      if (out_seeker == out_end)
        out_seeker = NULL, out_seq_number++;

      if (!avail_out)
        return LZ4_OK;
    }
    else
    {
      // out condition here
      if (JobHaveNextOut(out_seq_number) && !out_finish)
      {
        int ret_value;
        out_end = JobGetNextOut(out_buf, out_seq_number, &ret_value);
        if (!out_end || ret_value < 0)
        {
          out_seeker = out_end = NULL;
          return (ret_value < 0) ? LZ4_DATA_ERROR : LZ4_NOT_ENOUGH_MEM;
        }
        if (out_end - out_buf)
          out_seeker = out_buf;
      }
      else
      {
        // in condition here

        // Helps control uncompressible data from running away with the memory usage
        // in the output buffer, by limiting the input data.
        if (JobCountNextOut() < compressor_info.num_threads && !in_finish)
        {
          if (!avail_in)
            in_finish = true;
          else
          {
            unsigned int inputBytes = avail_in < MT_LZ4_BLOCK_DATA_SIZE ? avail_in : MT_LZ4_BLOCK_DATA_SIZE;
            if (add_threaded_job(next_in, inputBytes))
            {
              next_in += inputBytes;
              avail_in -= inputBytes;
              in_seq_number++;
            }
            else
              return LZ4_NOT_ENOUGH_MEM;
          }
        }
        if (!JobCountNextOut() && in_seq_number == out_seq_number)
          out_finish = true;
      }
    }
  }
  if (in_finish && out_finish && finish)
  {
    int i;
    finished = true;
    if (avail_out < MT_LZ4_BLOCK_HEADER_SIZE) return LZ4_DATA_ERROR;
    for (i = 0; i < MT_LZ4_BLOCK_HEADER_SIZE; ++i)
      *next_out++ = 0, avail_out--;
  }
  in_finish = out_finish = false;
  return LZ4_OK;
}

void CLZ4MT::SetNextIn(char *in, unsigned int size)
{
  next_in = in;
  avail_in = size;
}

void CLZ4MT::SetNextOut(char *out, unsigned int size)
{
  next_out = out;
  avail_out = size;
}

char* CLZ4MT::GetNextOut()
{
  return next_out;
}

unsigned int CLZ4MT::GetAvailIn()
{
  return avail_in;
}
unsigned int CLZ4MT::GetAvailOut()
{
  return avail_out;
}

const TCHAR* CLZ4MT::GetName()
{
  return _T("lz4");
}

const TCHAR* CLZ4MT::GetErrStr(int err)
{
  switch (err)
  {
  case LZ4_DATA_ERROR:
    return _T("data error");
  case LZ4_NOT_ENOUGH_MEM:
    return _T("not enough memory, try reducing thread count");
  default:
    return _T("unknown error");
  }
}
