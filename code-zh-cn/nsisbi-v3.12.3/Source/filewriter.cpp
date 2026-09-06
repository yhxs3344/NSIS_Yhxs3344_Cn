/*
 * filewriter.cpp
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

#include "Platform.h"
#include <cstdio>
#include "filewriter.h"
#include "util.h"

#ifdef _WIN32
#  include <io.h>
#  define _myseek64 _lseeki64
#  define _myfileno _fileno
#else // !_WIN32
#  include <unistd.h>
#  include <fcntl.h> // for open(2)
#  define _myseek64 lseek // needs _FILE_OFFSET_BITS=64 on 32 bit systems
#  define _myfileno fileno
#endif // ~_WIN32

#define seek64_helper(f, o, w) _myseek64((_myfileno(f)), (o), (w))
#define tell64_helper(f) seek64_helper((f), 0, SEEK_CUR)

int myfgetsetpos(FILE *stream)
{
  fpos_t pos;
  return ((fgetpos(stream, &pos) == 0) && (fsetpos(stream, &pos) == 0));
}

INT64 myftell64(FILE *stream)
{
  return (myfgetsetpos(stream)) ? tell64_helper(stream) : -1;
}
int myfseek64(FILE *stream, INT64 offset, int origin)
{
  return (myfgetsetpos(stream)) ? ((seek64_helper(stream, offset, origin) == (off_t)-1) ? -1 : 0) : -1;
}

FileWriter::FileWriter()
{
  fp = 0;
}

FileWriter::~FileWriter()
{
  Fclose();
}

FILE* FileWriter::GetHandle()
{
  return fp;
}

bool FileWriter::Fopen(TCHAR *filename)
{
  fp = FOPEN(filename, ("w+b"));
  return !!fp;
}

void FileWriter::Fclose()
{
  if (fp) Fflush(), fclose(fp), fp = 0;
}

long FileWriter::Ftell()
{
  return ftell(fp);
}

INT64 FileWriter::Ftell64()
{
  return myftell64(fp);
}

int FileWriter::Fseek(int pos, int origin)
{
  return fseek(fp, pos, origin);
}

int FileWriter::Fseek64(INT64 pos, int origin)
{
  return myfseek64(fp, pos, origin);
}

size_t FileWriter::Fread(void *ptr, size_t size, size_t nmemb)
{
  return fread(ptr, size, nmemb, fp);
}

size_t FileWriter::Fwrite(const void *ptr, size_t size, size_t nmemb)
{
  return fwrite(ptr, size, nmemb, fp);
}

void FileWriter::Fflush()
{
  fflush(fp);
}

FileWriterExternal::FileWriterExternal()
{
  fp = 0;
  dir = 0;
  out_size = 0;
  current_size = 0;
  file_count = 1;
  written = 0;
  total = 0;
}

FileWriterExternal::~FileWriterExternal()
{
  if (dir) free(dir), dir = 0;
  Fclose();
}

void FileWriterExternal::SetSize(UINT64 size)
{
  out_size = size;
}

bool FileWriterExternal::Fopen_next(void)
{
  TCHAR tempname[260];
  wsprintf(tempname, _T("%") NPRIs _T("%") NPRIs _T("setup%u.bin"), dir, PLATFORM_PATH_SEPARATOR_STR, file_count);
  fp = FOPEN(tempname, ("w+b"));
  return !!fp;
}

bool FileWriterExternal::Fopen(const TCHAR *in_dir)
{
  dir = _tcsdup(in_dir);
  return Fopen_next();
}

void FileWriterExternal::Fclose()
{
  if (fp)
  {
    Fflush();
    fclose(fp);
    fp = 0;
  }
}

INT64 FileWriterExternal::Ftell64()
{
  return total;
}

unsigned int FileWriterExternal::Fcount()
{
  return file_count;
}

size_t FileWriterExternal::Fwrite(const void *ptr, size_t size)
{
  unsigned char *buf = (unsigned char*)ptr;
  size_t written = 0;

  if (!out_size)
  {
    written = fwrite(ptr, 1, size, fp);
    total += written;
    return written;
  }

  while (size)
  {
    UINT64 calc_size64 = (current_size + size) > out_size ? out_size - current_size : size;
    size_t calc_size = calc_size64 > 0xFFFFFFFF ? 0xFFFFFFFF : (size_t)calc_size64;

    if (!current_size && total)
      Fopen_next();
    size_t write = fwrite((void*)buf, 1, calc_size, fp);
    if (write != calc_size)
      return written + write;
    written += write;
    total += calc_size;
    buf += calc_size;
    current_size += calc_size;
    size -= calc_size;
    if (current_size == out_size)
    {
      file_count++;
      current_size = 0;
      Fclose();
    }
  }
  return written;
}

void FileWriterExternal::Fflush()
{
  fflush(fp);
}

