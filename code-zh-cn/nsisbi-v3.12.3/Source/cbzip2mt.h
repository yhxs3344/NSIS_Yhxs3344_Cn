/*
 * cbzip2mt.h
 *
 * This file is a part of the multithread wrapper for NSIS.
 *
 * Copyright (C) 2026 Jason Ross (JasonFriday13)
 *
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __CBZIP2MT_H__
#define __CBZIP2MT_H__

#include "compressor.h"
#include "mtw/mtwcommon.h"
#include "mtw/mtwthreads.h"

class CBzip2MT : public ICompressor
{
public:
  CBzip2MT();
  virtual ~CBzip2MT();

  int Init(int level, unsigned int dict_size, unsigned int threads);
  virtual int Init(int level, unsigned int dict_size) { return Init(level, dict_size, 1); }
  virtual int End();
  virtual int Compress(bool finish);

  virtual void SetNextIn(char *in, unsigned int size);
  virtual void SetNextOut(char *out, unsigned int size);

  virtual char* GetNextOut();

  virtual unsigned int GetAvailIn();
  virtual unsigned int GetAvailOut();

  virtual const TCHAR* GetName();

  virtual const TCHAR* GetErrStr(int err);

private:
  char *next_in;          /* next input byte */
  unsigned int avail_in;  /* number of bytes available at next_in */

  char *next_out;         /* next output byte should be put there */
  unsigned int avail_out; /* remaining free space at next_out */

  int add_threaded_job(const char *in, unsigned int size);

  UINT64 in_seq_number, out_seq_number;
  bool finished, init;

  char *out_buf;          /* compressed buffer for output */
  char *out_seeker;       /* keeps track of where the start of unflushed data is */
  char *out_end;          /* end pointer of buffer, so we know when we are finished */

  MT_THREAD_CTX *c_ctx;
  CINFO compressor_info;
};

#endif
