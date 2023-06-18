/*
 * cmtw.h
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

#ifndef __MTCOMPRESS_H__
#define __MTCOMPRESS_H__

class CMTW
{
public:
  enum {
    ZLIB = 0,
    BZIP2,
    LZMA,
    LZ4,
  };
  
  virtual ~CMTW() {}

  void SetCompressor(int which_compressor);
  int GetCompressor();
  int Init(int level, unsigned int dict_size, unsigned int threads);
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
};

#endif