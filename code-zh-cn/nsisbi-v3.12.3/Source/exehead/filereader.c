/*
 * filereader.c
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

#include "config.h"

#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT

#include "../Platform.h"
#include "util.h"

HANDLE *file_handles;
unsigned int file_num, file_total;
INT64 file_offset, file_size;

void SetExternalFileSpecs(unsigned int num, unsigned int size)
{
  file_total = num ? num : 1;
  file_size = (UINT64)size << 20;
}

void CloseExternalFile(void)
{
  UINT64 i;
  file_num = 0;
  file_total = 0;
  file_offset = 0;
  file_size = 0;
  for (i = 0; i < file_total; i++)
  {
    if (file_handles)
      if (file_handles[i])
        CloseHandle(file_handles[i]), file_handles[i] = INVALID_HANDLE_VALUE;
  }
  if (file_handles) GlobalFree(file_handles), file_handles = NULL;
}

BOOL OpenExternalFile(const TCHAR* dir)
{
  unsigned int i;
  TCHAR filename[MAX_PATH];

  if (file_handles) CloseExternalFile();
  file_handles = GlobalAlloc(GPTR, sizeof(HANDLE *) * file_total);
  if (!file_handles) return FALSE;
  file_num = 0;
  file_offset = 0;
  for (i = 0; i < file_total; i++)
  {
    wsprintf(filename, _T("%s\\setup%u.bin"), dir, i + 1);
    file_handles[i] = myOpenFile(filename, GENERIC_READ, OPEN_EXISTING);
    if (file_handles[i] == INVALID_HANDLE_VALUE) return FALSE;
  }
  return TRUE;
}

BOOL NSISCALL ReadExternalFile(LPVOID lpBuffer, DWORD read_bytes)
{
  DWORD read_size;
  BOOL ret;
  char *buf = (char *)lpBuffer;

  while (read_bytes)
  {
    read_size = read_bytes;
    if (file_total > 1)
    {
      if (file_offset == file_size)
      {
        file_num++;
        file_offset = 0;
        if (-1 == SetFilePointer64(file_handles[file_num - 1], file_offset, FILE_BEGIN))
          return FALSE;
      }
      read_size = read_bytes < file_size - file_offset ? read_bytes : (DWORD)(file_size - file_offset);
    }
    ret = myReadFile(file_handles[file_num - 1], (LPVOID)buf, read_size);
    if (!ret) return FALSE;
    file_offset += read_size;
    buf += read_size;
    read_bytes -= read_size;
  }
  return TRUE;
}

INT64 NSISCALL SetExternalFilePointer(INT64 lDistanceToMove)
{
  file_num = 1;
  file_offset = lDistanceToMove;
  while (file_offset >= file_size && file_total > 1)
  {
    file_num++;
    file_offset -= file_size;
  }
  return SetFilePointer64(file_handles[file_num - 1], file_offset, FILE_BEGIN);
}
#endif

