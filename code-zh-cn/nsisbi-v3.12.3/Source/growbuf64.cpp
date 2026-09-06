/*
 * growbuf64.cpp
 * 
 * This file is a part of NSIS.
 * 
 * Copyright (C) 1999-2018 Nullsoft and Contributors
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 * 
 * Unicode support and Doxygen comments by Jim Park -- 07/31/2007
 */

#include "Platform.h"
#include "growbuf64.h"

#include <cstdlib> // for malloc/free
#include <cstring> // for memcpy
#include <cstdio> // for f*
#include <algorithm> // for std::min
#include "tchar.h"
#include "util.h"


using namespace std;

// Default constructor
GrowBuf64::GrowBuf64() { m_alloc=m_used=0, m_zero=false, m_s=NULL, m_bs=32768; }

// Destructor
GrowBuf64::~GrowBuf64() { free(m_s); }

void GrowBuf64::set_zeroing(bool zero) { m_zero=zero; }

GrowBuf64::size_type GrowBuf64::add(const void *data, int len)
{
  if (len<=0) return 0; // BUGBUG: Why is this returning 0? It should return m_used?
  resize(m_used+len);
  memcpy((BYTE*)m_s+m_used-len,data,len);
  return m_used-len;
}

void GrowBuf64::resize(GrowBuf64::size_type newlen)
{
  const size_type orgalloc=m_alloc;
  const size_type orgused=m_used;

  m_used=newlen;
  if (newlen > m_alloc)
  {
    void *newstor;

    // Jim Park: Not sure why we don't just add m_bs.  Multiplying by 2
    // makes m_bs meaningless after a few resizes.  So TinyGrowBuf
    // isn't very tiny.
    m_alloc = newlen*2 + m_bs;
    newstor = realloc(m_s, (size_t)m_alloc);
    if (!newstor)
    {
      extern int g_display_errors;
      if (g_display_errors)
      {
        PrintColorFmtMsg_ERR(_T("\nack! realloc(%d) failed, trying malloc(%d)!\n"),m_alloc,newlen);
      }
      m_alloc=newlen; // try to malloc the minimum needed
      newstor=malloc((size_t)m_alloc);
      if (!newstor)
      {
        extern void quit();
        if (g_display_errors)
        {
          PrintColorFmtMsg_ERR(_T("\nInternal compiler error #12345: GrowBuf realloc/malloc(%d) failed.\n"),m_alloc);
        }
        quit();
      }
      memcpy(newstor,m_s,(size_t)min(newlen,orgalloc));
      free(m_s);
    }
    m_s=newstor;
  }

  // Zero out the new buffer area
  if (m_zero && m_used > orgused)
    memset((BYTE*)m_s + orgused, 0, (size_t)(m_used - orgused));

  if (!m_used && m_alloc > 2*m_bs) // only free if you resize to 0 and we're > 64k or
                                   // 2K in the case of TinyGrowBuf
  {
    m_alloc=0;
    free(m_s);
    m_s=NULL;
  }
}

GrowBuf64::size_type GrowBuf64::getlen() const { return m_used; }
void *GrowBuf64::get() const { return m_s; }

void GrowBuf64::swap(GrowBuf64&other)
{
  std::swap(m_s, other.m_s);
  std::swap(m_alloc, other.m_alloc);
  std::swap(m_used, other.m_used);
  std::swap(m_zero, other.m_zero);
  std::swap(m_bs, other.m_bs);
}
