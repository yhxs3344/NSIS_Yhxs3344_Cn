/*
 * cmtw.cpp
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

#include "Platform.h"
#include "mtw/mtwcommon.h"
#include "mtw/mtwthreads.h"
#include "mtw/linkedlist.h"
#include "cmtw.h"
#include "czlib.h"
#include "cbzip2.h"
#include "clzma.h"
#include "clz4.h"

DATA_HANDLE data_out;

UINT64 in_seq_number, out_seq_number;
bool finished, init = false;

char *out_buf;       /* compressed buffer for output */
char *out_seeker;    /* keeps track of where the start of unflushed data is */
char *out_end;       /* end pointer of buffer, so we know when we are finished */

MT_THREAD_CTX *c_ctx;

unsigned int compress_thread(void *arg);

typedef struct {
  int compressor;
  unsigned int num_threads;
  int level;
  int dict_size;
} CINFO;
CINFO compressor_info;

// Returns true if the output queue length is less than the number of threads.
bool check_next_out(void)
{
  MT_MUTEX_LOCK(&data_out.mutex);
  bool ret = !finished ? GetListCount(&data_out.handle) < compressor_info.num_threads : false;
  MT_MUTEX_UNLOCK(&data_out.mutex);
  return ret;
}

// Returns true if the output queue is empty.
bool check_next_out_empty(void)
{
  MT_MUTEX_LOCK(&data_out.mutex);
  bool ret = !finished ? !GetListCount(&data_out.handle) : false;
  MT_MUTEX_UNLOCK(&data_out.mutex);
  return ret;
}

// Returns true if the next sequence number compressed block is ready.
bool have_next_out(void)
{
  MT_MUTEX_LOCK(&data_out.mutex);
  bool count = !!FindSeqNum(out_seq_number, &data_out.handle);
  MT_MUTEX_UNLOCK(&data_out.mutex);
  return count;
}

typedef struct 
{
  char *data;
  unsigned int size;
  UINT64 seq_num;
  CINFO comp_info; 
} JOB;

bool set_next_in(const char *in, unsigned int size)
{
  size_t i = 0;
  JOB *job = (JOB*)malloc(sizeof(JOB));
  if (!job) return false;

  job->data = (char*)malloc(size);
  if (!job->data) return false;

  for (i = 0; i < size; i++)
    job->data[i] = in[i];
  job->size = size;
  job->seq_num = in_seq_number;
  job->comp_info.compressor = compressor_info.compressor;
  job->comp_info.dict_size = compressor_info.dict_size;
  job->comp_info.level = compressor_info.level;
  job->comp_info.num_threads = compressor_info.num_threads;

  mt_add(c_ctx, (data_function)compress_thread, (void*)job);
  return true;
}

void set_in_finish(bool finish)
{
  if (finish && !finished)
  {
    finished = true;
  }
}

char *get_next_out(char *out, int *ret_value)
{
  char *out_end = NULL;
  if (!have_next_out()) return NULL;
  MT_MUTEX_LOCK(&data_out.mutex);
  if (FindSeqNum(out_seq_number, &data_out.handle))
    out_end = GetData(out, out_seq_number, ret_value, &data_out.handle);
  MT_MUTEX_UNLOCK(&data_out.mutex);
  return out_end;
}

unsigned int get_next_out_size(void)
{
  unsigned int size = 0;
  MT_MUTEX_LOCK(&data_out.mutex);
  if (FindSeqNum(out_seq_number, &data_out.handle))
    size = GetDataLen(out_seq_number, &data_out.handle);
  MT_MUTEX_UNLOCK(&data_out.mutex);
  return size;
}

unsigned int compress_thread(void *arg)
{
  JOB *job = (JOB*)arg;
  char *out;
  unsigned int out_len = 1;
  const unsigned int out_avail = MTW_BLOCK_BUF_SIZE;
  ICompressor *compressor;
 
  switch (job->comp_info.compressor)
  {
  case CMTW::BZIP2:
    compressor = new CBzip2;
  break;
  case CMTW::LZ4:
    compressor = new CLZ4;
  break;
  case CMTW::LZMA:
    compressor = new CLZMA;
  break;
  case CMTW::ZLIB:
  default:
    compressor = new CZlib;
  }
  out = (char*)malloc(out_avail);
  int ret;
  if (out && compressor)
  {
    compressor->Init(job->comp_info.level, job->comp_info.dict_size);
    compressor->SetNextIn(job->data, job->size);
    compressor->SetNextOut(out, out_avail);
    ret = compressor->Compress(C_FINISH);
    out_len = (unsigned int)(compressor->GetNextOut() - out);
    if (ret < 0)
      out_len = 1;
    compressor->End();
  }
  else
  {
    ret = MTW_NOT_ENOUGH_MEM;
  }
  MT_MUTEX_LOCK(&data_out.mutex);
  AddData(out, out_len, job->seq_num, ret, &data_out.handle);
  MT_MUTEX_UNLOCK(&data_out.mutex);

  if (compressor) delete compressor;
  if (out) free(out);
  if (job->data) free(job->data);
  if (job) free(job), job = NULL;
  return 0;
}

void CMTW::SetCompressor(int which_compressor)
{
  compressor_info.compressor = which_compressor;
}

int CMTW::GetCompressor()
{
  return compressor_info.compressor;
}

bool in_finish = false, out_finish = false;

int CMTW::Init(int level, unsigned int dict_size, unsigned int num_threads)
{
  int error = 0;
  if (init) End();

  compressor_info.level = level;
  compressor_info.dict_size = dict_size;
  compressor_info.num_threads = num_threads;

  in_seq_number = out_seq_number = 1;
  in_finish = out_finish = false;
  finished = false;

  out_buf = out_seeker = out_end = NULL;
  out_buf = (char*)malloc(MTW_BLOCK_BUF_SIZE);
  error |= !out_buf;
  error |= MT_MUTEX_INIT(&data_out.mutex, NULL);

  MT_MUTEX_LOCK(&data_out.mutex);
  InitVarList(&data_out.handle);
  MT_MUTEX_UNLOCK(&data_out.mutex);

  c_ctx = mt_create(compressor_info.num_threads);
  error |= !c_ctx;

  return error ? MTW_NOT_ENOUGH_MEM : init = true, MTW_OK;
}

int CMTW::End()
{
  if (init)
  {
    if (c_ctx)
    {
      mt_free(c_ctx);
      c_ctx = NULL;
    }

    if (out_buf)
    {
      free(out_buf);
      out_buf = out_seeker = out_end = NULL;
    }

    MT_MUTEX_LOCK(&data_out.mutex);
    CleanVarList(&data_out.handle);
    MT_MUTEX_UNLOCK(&data_out.mutex);

    MT_MUTEX_DESTROY(&data_out.mutex);
    init = false;
  }
  return MTW_OK;
}

int CMTW::Compress(bool finish)
{
  if (finished) return MTW_OK;

  while (!(in_finish && out_finish))
  {
    if (out_seeker)
    {
      unsigned int shift = 0, header_bytes = out_seeker == out_buf ? MTW_BLOCK_HEADER_SIZE : 0;
      int i, outbytes = (avail_out > (unsigned int)(out_end - out_seeker) + header_bytes) ? (unsigned int)(out_end - out_seeker) : avail_out - header_bytes;

      if (outbytes < 1)
        return MTW_OK;

      if (header_bytes)
      {
        unsigned int size = (unsigned int)(out_end - out_buf);
        for (i = 0; i < MTW_BLOCK_HEADER_SIZE; i++, shift += 8)
          *next_out++ = (unsigned char)(size >> shift), avail_out--;
      }

      for (i = 0; i < outbytes; i++)
        *next_out++ = *out_seeker++, avail_out--;

      if (out_seeker == out_end)
        out_seeker = NULL, out_seq_number++;

      if (!avail_out)
        return MTW_OK;
    }
    else
    {
      // out condition here
      if (!check_next_out_empty() && have_next_out() && !out_finish)
      {
        int ret_value;
        out_end = get_next_out(out_buf, &ret_value);
        if (!out_end || ret_value < 0)
        {
          free(out_buf);
          out_buf = out_seeker = out_end = NULL;
          return (ret_value < 0) ? MTW_DATA_ERROR : MTW_NOT_ENOUGH_MEM;
        }
        if (out_end - out_buf)
          out_seeker = out_buf;
      }
      else
      {
        // in condition here

        // Helps control uncompressible data from running away with the memory usage
        // in the output buffer, by limiting the input data.
        if (check_next_out() && !in_finish)
        {
          if (!avail_in)
            in_finish = true;
          else
          {
            unsigned int inputBytes = avail_in < MTW_BLOCK_DATA_SIZE ? avail_in : MTW_BLOCK_DATA_SIZE;
            if (set_next_in(next_in, inputBytes))
            {
              next_in += inputBytes;
              avail_in -= inputBytes;
              in_seq_number++;
            }
          }
        }
        if (check_next_out_empty() && in_seq_number == out_seq_number)
          out_finish = true;
      }
    }
  }
  if (in_finish && out_finish && finish)
  {
    set_in_finish(finish);
    int i;

    if (avail_out < MTW_BLOCK_HEADER_SIZE) return MTW_DATA_ERROR;
    for (i = 0; i < MTW_BLOCK_HEADER_SIZE; ++i)
      *next_out++ = 0, avail_out--;
  }
  in_finish = out_finish = false;
  return MTW_OK;
}

void CMTW::SetNextIn(char *in, unsigned int size)
{
  next_in = in;
  avail_in = size;
}

void CMTW::SetNextOut(char *out, unsigned int size)
{
  next_out = out;
  avail_out = size;
}

char* CMTW::GetNextOut()
{
  return next_out;
}

unsigned int CMTW::GetAvailIn()
{
  return avail_in;
}
unsigned int CMTW::GetAvailOut()
{
  return avail_out;
}

const TCHAR* CMTW::GetName()
{
  switch (compressor_info.compressor)
  {
  case CMTW::ZLIB:
    return _T("zlib");
  case CMTW::BZIP2:
    return _T("bzip2");
  case CMTW::LZMA:
    return _T("lzma");
  case CMTW::LZ4:
    return _T("lz4");
  default:
    return _T("unknown");
  }
}

const TCHAR* CMTW::GetErrStr(int err)
{
  return _T("error");
}
