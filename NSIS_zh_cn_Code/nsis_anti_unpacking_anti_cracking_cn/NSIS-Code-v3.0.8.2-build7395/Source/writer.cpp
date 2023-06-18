/*
 * writer.cpp
 * 
 * This file is a part of NSIS.
 * 
 * Copyright (C) 1999-2022 Nullsoft and Contributors
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 *
 * Unicode support by Jim Park -- 08/13/2007
 */

#include "exehead/config.h"
#include "writer.h"
#include "growbuf.h"
#include "growbuf64.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include "tchar.h"

void writer_sink::write_byte(const unsigned char b)
{
  write_data(&b, (UINT32)1);
}

void writer_sink::write_short(const short s)
{
  short fs = FIX_ENDIAN_INT16(s);
  write_data(&fs, sizeof(short));
}

void writer_sink::write_int(const int i)
{
  int fi = FIX_ENDIAN_INT32(i);
  write_data(&fi, sizeof(int));
}
void writer_sink::write_int64(const INT64 i)
{
  INT64 fi = FIX_ENDIAN_INT64(i);
  write_data(&fi, sizeof(INT64));
}

void writer_sink::write_int_array(const int i[], const size_t len)
{
  for (size_t l = 0; l < len; l++)
  {
    write_int(i[l]);
  }
}

// size in this case is the length of the string to write.
void writer_sink::write_string(const TCHAR *s, size_t size)
{
#ifdef _UNICODE
  if (m_ti.is_unicode())
  {
    bool strEnd = false;
    TCHAR ch = L'\0';
    for (; size ; size--)
    {
      if (!strEnd)
      {
        ch = *s++;
        if (ch == _T('\0'))
          strEnd = true;
      }
      write_short(ch);
    }
  }
  else
  {
    char *wb = new char[size];
    memset(wb, 0, size);
    WideCharToMultiByte(CP_ACP, 0, s, -1, wb, (int)size, NULL, NULL);
    write_data(wb, (UINT32)size);
    delete [] wb;
 }
#else
  //TODO: Why does this allocate memory? It could just write the string and a manual zero character?
  char *wb = new char[size];
  memset(wb, 0, size);
  strncpy(wb, s, size);
  write_data(wb, size);
  delete [] wb;
#endif
}

void writer_sink::write_growbuf(const IGrowBuf *b)
{
  write_data(b->get(), (UINT32)b->getlen());
}

void writer_sink::write_growbuf64(const IGrowBuf64 *b)
{
  write_data(b->get(), (UINT32)b->getlen());
}

namespace hlp {
  template<class T> static inline bool issigned() { return T(-1) < T(0); }
  template<class T> static inline bool issigned(const T&t) { return issigned<T>(); }
}

void growbuf_writer_sink::write_data(const void *data, const UINT32 size)
{
  m_buf->add(data, (sink_type::size_type)size);
}

void growbuf64_writer_sink::write_data(const void *data, const UINT32 size)
{
  m_buf->add(data, size);
}

/*void growbuf64_writer_sink::write_data(const void *data, const UINT64 size)
{
  m_buf->add(data, (size_t)size);
}
*/
void file_writer_sink::write_data(const void *data, const UINT32 size)
{
  if (fwrite(data, 1, size, m_fp) != size)
  {
    throw std::runtime_error("error writing");
  }
}

#ifdef NSIS_CONFIG_CRC_SUPPORT
#include "crc32.h"

void crc_writer_sink::write_data(const void *data, const UINT32 size)
{
  *m_crc = CRC32(*m_crc, (const unsigned char *) data, size);
}
#endif
