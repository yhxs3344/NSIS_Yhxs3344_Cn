/*
 * clz4.h
 * 
 * This file is a part of NSIS.
 * 
 * Copyright (C) 2016-2017, 2023 JasonFriday13
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __CLZ4_H__
#define __CLZ4_H__

#include "compressor.h"
#include "lz4/lz4common.h"
#include "lz4/lz4compress.h"

class CLZ4 : public ICompressor {
  public:
    virtual ~CLZ4() {}

    virtual int Init(int level, unsigned int dict_size, unsigned int threads) {
      return LZ4_Init(&stream, level, dict_size);
    }

    virtual int End() {
      return LZ4_End(&stream);
    }

    virtual int Compress(bool finish) {
      return LZ4_Compress(&stream, finish);
    }

    virtual void SetNextIn(char *in, unsigned int size) {
      stream.next_in = in;
      stream.avail_in = size;
    }

    virtual void SetNextOut(char *out, unsigned int size) {
      stream.next_out = out;
      stream.avail_out = size;
    }

    virtual char* GetNextOut() {
      return stream.next_out;
    }

    virtual unsigned int GetAvailIn() {
      return stream.avail_in;
    }

    virtual unsigned int GetAvailOut() {
      return stream.avail_out;
    }

    virtual const TCHAR* GetName() {
      return _T("lz4");
    }

    virtual const TCHAR* GetErrStr(int err) {
      switch (err)
      {
      case LZ4_DATA_ERROR:
        return _T("data error");
      case LZ4_NOT_ENOUGH_MEM:
        return _T("not enough memory");
      default:
        return _T("unknown error");
      }
    }

  private:
    lz4_compstream stream;
};

#endif
