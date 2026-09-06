/*
 * filewriter.h
 * 
 * This file is a part of NSISBI.
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

#ifndef ___FILEWRITER__H___
#define ___FILEWRITER__H___

#include "Platform.h"

class FileWriter
{
public:
  FileWriter();
  ~FileWriter();
  FILE* GetHandle();
  bool Fopen(TCHAR *filename);
  void Fclose();
  long Ftell();
  INT64 Ftell64();
  int Fseek(int offset, int origin);
  int Fseek64(INT64 offset, int origin);
  size_t Fread(void *ptr, size_t size, size_t nmemb);
  size_t Fwrite(const void *ptr, size_t size, size_t nmemb);
  void Fflush();

private:
  FILE *fp;
};

class FileWriterExternal
{
public:
  FileWriterExternal();
  ~FileWriterExternal();
  void SetSize(UINT64 size);
  bool Fopen(const TCHAR *in_dir);
  void Fclose();
  INT64 Ftell64();
  unsigned int Fcount();
  size_t Fwrite(const void *ptr, size_t size);
  void Fflush();

private:
  bool Fopen_next(void);

  FILE *fp;
  TCHAR *dir;
  UINT64 out_size;
  UINT64 current_size;
  unsigned int file_count;
  UINT64 written;
  UINT64 total;
};

#endif//!___WRITER__H___
