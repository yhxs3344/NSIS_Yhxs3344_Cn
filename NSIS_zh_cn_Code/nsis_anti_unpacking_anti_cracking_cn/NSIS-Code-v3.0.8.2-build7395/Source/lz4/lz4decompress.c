/*
   LZ4 - Fast LZ compression algorithm
   Copyright (C) 2011-2015, Yann Collet.

   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   - LZ4 source repository : https://github.com/Cyan4973/lz4
   - LZ4 public forum : https://groups.google.com/forum/#!forum/lz4c
*/

/*
 * lz4decompress.c
 * 
 * This file is a part of the LZ4 compression module for NSIS.
 * 
 * Copyright (C) 2016-2017 Jason Ross (JasonFriday13)
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

/**************************************
*  Includes
**************************************/
#include "lz4decompress.h"


/**************************************
*  Memory routines
**************************************/
#include "lz4common.h"


/**************************************
*  Compiler Options
**************************************/
#ifdef FORCE_INLINE
#  undef FORCE_INLINE
#endif
#ifdef _MSC_VER    /* Visual Studio */
#  define FORCE_INLINE static __forceinline
#  if !(_MSC_FULL_VER <= 140040310)
#    include <intrin.h>
#  endif
#  pragma warning(disable : 4127)        /* disable: C4127: conditional expression is constant */
#  pragma warning(disable : 4293)        /* disable: C4293: too large shift (32-bits) */
#else
#  if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)   /* C99 */
#    if defined(__GNUC__) || defined(__clang__)
#      define FORCE_INLINE static inline __attribute__((always_inline))
#    else
#      define FORCE_INLINE static inline
#    endif
#  else
#    define FORCE_INLINE static
#  endif   /* __STDC_VERSION__ */
#endif  /* _MSC_VER */

enum {
  LZ4D_START = 0,
  LZ4D_GETLENGTHFIRST,
  LZ4D_GETLENGTHSECOND,
  LZ4D_GETLENGTHTHIRD,
  LZ4D_COPYIN,
  LZ4D_COPYINFULL,
  LZ4D_COPYINPARTIAL,
  LZ4D_DECOMPRESSBLOCK,
  LZ4D_FLUSHOUT,
  LZ4D_STREAMEND,
} LZ4D_STAGE;

#if 0
enum
{
  LZ4_START = 0,
  LZ4_LENGTH,
  LZ4_TOKEN,
  LZ4_TOKENRUN,
  LZ4_LITS,
  LZ4_MOFFSET,
  LZ4_MLEN,
  LZ4_MLENRUN,
  LZ4_DICT,
  LZ4_DICTCOPY,
  LZ4_DICTAPPEND,
  LZ4_COPYREP,
  LZ4_LASTLITS,
};
#endif

void lz4Init(lz4_decstream *s)
{
  int i;
      
  for (i = 0; i < sizeof(s->LZ4_State); i++)
    *((char *)&s->LZ4_State + i) = 0;
}

int lz4Decode(lz4_decstream *s)
{
  for (;;)
  {
    switch (s->LZ4_State.stage)
    {
    case LZ4D_START:
    case LZ4D_GETLENGTHFIRST:
      if (s->avail_in < 1)
      {
        s->LZ4_State.stage = LZ4D_FLUSHOUT;
        break;
      }
      s->LZ4_State.blockLength = 0;
      s->LZ4_State.blockLength = (unsigned char)*s->next_in++, --s->avail_in;
      s->LZ4_State.stage = LZ4D_GETLENGTHSECOND;
    case LZ4D_GETLENGTHSECOND:
      if (s->avail_in < 1)
        return LZ4_OK;
      s->LZ4_State.blockLength |= (unsigned short)(*s->next_in++ << 8) & 0xFF00, --s->avail_in;
#if 0
      s->LZ4_State.stage = LZ4D_GETLENGTHTHIRD;
    case LZ4D_GETLENGTHTHIRD:
      if (s->avail_in < 1)
        return LZ4_OK;
      s->LZ4_State.blockLength |= (unsigned int)(*s->next_in++ << 16) & 0xFF0000, --s->avail_in;
#endif
      s->LZ4_State.stage = LZ4D_COPYIN;
    case LZ4D_COPYIN:
      if (s->LZ4_State.blockLength == 0 && !s->avail_in)
      {
        s->LZ4_State.stage = LZ4D_STREAMEND;
        break;
      }
      s->LZ4_State.stage = s->LZ4_State.blockLength < s->avail_in ? LZ4D_COPYINFULL : LZ4D_COPYINPARTIAL;
      break;
    case LZ4D_COPYINFULL:
      for (s->LZ4_State.inCount = 0; s->LZ4_State.inCount < s->LZ4_State.blockLength; s->LZ4_State.inCount++)
        *(s->LZ4_State.pInBuf + s->LZ4_State.inCount) = *s->next_in++, --s->avail_in;
      s->LZ4_State.stage = LZ4D_DECOMPRESSBLOCK;
      break;
    case LZ4D_COPYINPARTIAL:
      {
        unsigned int i, avail = s->LZ4_State.blockLength - s->LZ4_State.inCount < s->avail_in ? s->LZ4_State.blockLength - s->LZ4_State.inCount : s->avail_in;

        for (i = 0; i < avail; i++)
          *(s->LZ4_State.pInBuf + s->LZ4_State.inCount + i) = *s->next_in++, --s->avail_in;
        s->LZ4_State.inCount += i;
        if (s->LZ4_State.inCount < s->LZ4_State.blockLength) return LZ4_OK;
        s->LZ4_State.stage = LZ4D_DECOMPRESSBLOCK;
      }
    case LZ4D_DECOMPRESSBLOCK:
#ifdef _LZ4_BLOCKONLY
      s->LZ4_State.decBytes = LZ4_decompress_safe(s->LZ4_State.pInBuf, s->LZ4_State.pDecBuf, s->LZ4_State.inCount, LZ4_BLOCK_DATA_SIZE);
      if (s->LZ4_State.decBytes < 0)
        return LZ4_DATA_ERROR;
      s->LZ4_State.inCount = 0;
      s->LZ4_State.decCount = s->LZ4_State.decBytes;
      s->LZ4_State.stage = LZ4D_FLUSHOUT;
#endif

#ifdef _LZ4_STREAMDICT
      {
        int i;
        s->LZ4_State.decBytes = LZ4_decompress_safe_continue(&s->LZ4_State.lz4StreamDecode, s->LZ4_State.pInBuf, s->LZ4_State.pDecBuf, s->LZ4_State.inCount, LZ4_BLOCK_DATA_SIZE);
        if (s->LZ4_State.decBytes < 0)
          return LZ4_DATA_ERROR;

        for (i = 0; i < LZ4_BLOCK_DATA_SIZE - s->LZ4_State.decBytes; ++i)
          s->LZ4_State.pDictSave[i] = s->LZ4_State.pDictSave[i + LZ4_BLOCK_DATA_SIZE - s->LZ4_State.decBytes];

        for (i = 0; i < s->LZ4_State.decBytes; ++i)
          s->LZ4_State.pDictSave[i + LZ4_BLOCK_DATA_SIZE - s->LZ4_State.decBytes] = s->LZ4_State.pDecBuf[i];

        LZ4_setStreamDecode(&s->LZ4_State.lz4StreamDecode, s->LZ4_State.pDictSave, LZ4_BLOCK_DATA_SIZE);
        s->LZ4_State.inCount = 0;
        s->LZ4_State.decCount = s->LZ4_State.decBytes;
        s->LZ4_State.stage = LZ4D_FLUSHOUT;
      }
#endif
    case LZ4D_FLUSHOUT:
      {
        unsigned int i;

        if (s->avail_out && s->LZ4_State.decCount)
        {
          if (s->LZ4_State.decCount > s->LZ4_State.bufCount)
          {
            unsigned int copyLimit = s->LZ4_State.decCount - s->LZ4_State.bufCount > s->avail_out ? s->avail_out : s->LZ4_State.decCount - s->LZ4_State.bufCount;
            for (i = 0; i < copyLimit; i++)
              *s->next_out++ = *(s->LZ4_State.pDecBuf + s->LZ4_State.bufCount + i), --s->avail_out;
            s->LZ4_State.bufCount += i;
            return LZ4_OK;
          }
          else
          {
            s->LZ4_State.decCount = 0;
            s->LZ4_State.bufCount = 0;
            break;
          }
        }
        else
        {
          s->LZ4_State.decCount = 0;
          s->LZ4_State.bufCount = 0;
          s->LZ4_State.inCount = 0;            
          s->LZ4_State.blockLength = 0;
          if (s->avail_in)
            s->LZ4_State.stage = LZ4D_START;
          else
            return LZ4_OK;
        }
      }
      break;
    case LZ4D_STREAMEND:
      return LZ4_STREAM_END;
    }
  }
  return LZ4_OK;
}


#ifndef LZ4_COMMONDEFS_ONLY

//#define LZ4_STATEMACHINE

#ifdef LZ4_STATEMACHINE

FORCE_INLINE void flushOut(lz4_decstream *s)
{
  while (s->avail_out && s->st.outPos)
  {
    *s->next_out++ = s->st.decBuf[s->st.bufCount-s->st.outPos];
    s->st.outPos--;
    s->avail_out--;
  }
}

FORCE_INLINE int outByte(lz4_decstream *s, char out)
{
//  printf("s->st.bufCount=|%d|\n", s->st.bufCount);
//  if (s->st.bufCount >= 65535) s->st.bufCount = 65535;
  if (s->st.outPos) flushOut(s);
  if (s->st.bufCount >= 65535)
  {
    unsigned int i;

    for (i = 0; i < 65535; i++)
      s->st.decBuf[i] = s->st.decBuf[i+1];

    s->st.decBuf[i] = out;
  }
  else
  {
    s->st.decBuf[s->st.bufCount] = out;
    s->st.bufCount++; // = s->st.bufCount >= 65535 ? 65535 : s->st.bufCount + 1;
  }
  s->st.outPos++;
  s->st.matchPos++;

  return 1;
}

FORCE_INLINE int loadByte(lz4_decstream *s)
{
  if (!s->avail_in) return 0;
  s->st.shortBuf = *s->next_in++;
  s->avail_in--;

  return 1;
}

FORCE_INLINE int loadShort(lz4_decstream *s)
{
  if (s->avail_in < 2) return 0;
  s->st.shortBuf = *s->next_in++;
  s->st.shortBuf = *s->next_in++ << 8;
  s->avail_in -= 2;

  return 1;
}

FORCE_INLINE char getByte(lz4_decstream *s)
{
  return (char)s->st.shortBuf;
}

FORCE_INLINE short getShort(lz4_decstream *s)
{
  return s->st.shortBuf;
}

FORCE_INLINE char getMatchByte(lz4_decstream *s)
{
  s->st.matchPos--;
  return s->st.decBuf[s->st.matchPos];
}

void trace(int stage)
{
  //printf("Token number=|%d|\n", stage);
}

/*******************************
*  Decompression functions
*******************************/
/*
 * This generic decompression function cover all use cases.
 * It shall be instantiated several times, using different sets of directives
 * Note that it is essential this generic function is really inlined,
 * in order to remove useless branches during compilation optimization.
 */
FORCE_INLINE int LZ4_decompress_generic(lz4_decstream *s)
{
    /* Local Variables */
//    const BYTE* ip = (const BYTE*) s->next_in;
//    const BYTE* const iend = ip + s->avail_in;

//    BYTE* op = (BYTE*) s->next_out;
//    BYTE* const oend = op + s->avail_out;
//    BYTE* cpy;
//    BYTE* oexit = op + targetOutputSize;
//    const BYTE* const lowLimit = lowPrefix - dictSize;

//    const BYTE* const dictEnd = (const BYTE*)dictStart + dictSize;
    const size_t dec32table[] = {4, 1, 2, 1, 4, 4, 4, 4};
    const size_t dec64table[] = {0, 0, 0, (size_t)-1, 0, 1, 2, 3};

//    const int safeDecode = (endOnInput==endOnInputSize);
//    const int checkOffset = ((safeDecode) && (dictSize < (int)(64 KB)));


    /* Special cases */
//    if ((partialDecoding) && (oexit> oend-MFLIMIT)) oexit = oend-MFLIMIT;                         /* targetOutputSize too high => decode everything */
//    if ((endOnInput) && (unlikely(outputSize==0))) return ((inputSize==1) && (*ip==0)) ? 0 : -1;  /* Empty output buffer */
//    if ((!endOnInput) && (unlikely(outputSize==0))) return (*ip==0?1:-1);

    if (!s->avail_in) return LZ4_STREAM_END;


    /* Main Loop */
    while (s->avail_in && s->avail_out)
    {
        //printf("s->st.bufCount=|%d|\n", s->st.bufCount);
        flushOut(s);
        switch (s->st.stage)
        {
        case LZ4_START:
        case LZ4_LENGTH:
            if (!loadShort(s)) return LZ4_OK;
            s->st.length = getShort(s);
            s->st.stage = LZ4_TOKEN;
            break;

        case LZ4_TOKEN:
//            unsigned token;
//            size_t length;
//            const BYTE* match;
            trace(s->st.stage);

            /* get literal length */
            if (!loadByte(s)) return LZ4_OK;
            s->st.token = getByte(s);
            s->st.litLen = s->st.token >> ML_BITS;
            s->st.stage = (s->st.litLen == RUN_MASK) ? LZ4_TOKENRUN : LZ4_LITS;
            break;

        case LZ4_TOKENRUN:
            {
                unsigned char size;
                trace(s->st.stage);
                do
                {
                    if (!loadByte(s)) return LZ4_OK;
                    size = getByte(s);
                    s->st.litLen += size;
                }
                while (size == 255);
//                while (likely((endOnInput)?ip<iend-RUN_MASK:1) && (s==255));
//                if ((safeDecode) && unlikely((size_t)(op+length)<(size_t)(op))) goto _output_error;   /* overflow detection */
//                if ((safeDecode) && unlikely((size_t)(ip+length)<(size_t)(ip))) goto _output_error;   /* overflow detection */
                s->st.stage = LZ4_LITS;
            }
        case LZ4_LITS:
            trace(s->st.stage);

            /* copy literals */
//            cpy = op+length;
            while (s->st.litLen)
            {
                if (!s->avail_out) return LZ4_OK;
                if (!loadByte(s)) return LZ4_OK;
                outByte(s, getByte(s));
                s->st.litLen--;
//              LZ4_wildCopy(op, ip, cpy);
//              ip += length; op = cpy;
            }
            s->st.stage = LZ4_MOFFSET;

        case LZ4_MOFFSET:
            trace(s->st.stage);

            /* get offset */
            if (s->avail_in < 2) return LZ4_OK;
            s->st.matchPos = LZ4_readLE16(s->next_in);
            s->next_in += 2;
            s->avail_in -= 2;
            if (s->st.matchPos > 65535) return -7;
            s->st.stage = LZ4_MLEN;
//            if ((checkOffset) && (unlikely(match < lowLimit))) goto _output_error;   /* Error : offset outside destination buffer */

        case LZ4_MLEN:
            trace(s->st.stage);

            /* get matchlength */
            s->st.matchLen = s->st.token & ML_MASK;
            s->st.stage = (s->st.matchLen == RUN_MASK) ? LZ4_MLENRUN : LZ4_DICT;
            s->st.matchLen += MINMATCH;
            break;

        case LZ4_MLENRUN:
            {
                unsigned char size;
                trace(s->st.stage);
                do
                {
                    if (!loadByte(s)) return LZ4_OK;
                    size = getByte(s);
                    s->st.matchLen += size;
                }
                while (size == 255);
//            if ((safeDecode) && unlikely((size_t)(op+length)<(size_t)op)) goto _output_error;   /* overflow detection */
                s->st.stage = LZ4_DICT;
            }
        case LZ4_DICT:
            trace(s->st.stage);

            if (s->st.matchPos < s->avail_in)
            {
                s->st.stage = LZ4_COPYREP;
                break;
            }
            if ((int)(s->st.bufCount - s->st.matchPos + s->st.matchLen) < s->st.bufCount)
            {
                s->st.stage = LZ4_DICTCOPY;
            }
            else
//              if (s->st.matchPos + s->st.matchLen >= 0)
            {
                s->st.stage = LZ4_DICTAPPEND;
            }
            break;

        case LZ4_DICTCOPY:
            trace(s->st.stage);

            /* match can be copied as a single segment from external dictionary */
//            match = dictEnd - (lowPrefix-match);
//            MEM_MOVE(op, match, length); op += length;
            while (s->st.matchLen)
            {
                if (!outByte(s, getMatchByte(s))) return LZ4_OK;
//                s->st.matchPos--;
                s->st.matchLen--;
            }
            s->st.stage = LZ4_COPYREP;
            break;

        case LZ4_DICTAPPEND:
            trace(s->st.stage);

            while (s->st.matchLen /*s->avail_out && s->avail_in*/)
            {
                if (s->st.matchPos > 0)
                {
                    if (!outByte(s, getMatchByte(s))) return LZ4_OK;
                }
                else
                {
                    if (!loadByte(s)) return LZ4_OK;
                    if (!outByte(s, getByte(s))) return LZ4_OK;
                }
                s->st.matchLen--;
            }
            s->st.stage = LZ4_COPYREP;
            break;


#if 0
        /* check external dictionary */
        if ((dict==usingExtDict) && (match < lowPrefix))
        {
            if (unlikely(op+length > oend-LASTLITERALS)) goto _output_error;   /* doesn't respect parsing restriction */

            if (length <= (size_t)(lowPrefix-match))
            {
                /* match can be copied as a single segment from external dictionary */
                match = dictEnd - (lowPrefix-match);
                MEM_MOVE(op, match, length); op += length;
            }
            else
            {
                /* match encompass external dictionary and current segment */
                size_t copySize = (size_t)(lowPrefix-match);
                MEM_CPY(op, dictEnd - copySize, copySize);
                op += copySize;
                copySize = length - copySize;
                if (copySize > (size_t)(op-lowPrefix))   /* overlap within current segment */
                {
                    BYTE* const endOfMatch = op + copySize;
                    const BYTE* copyFrom = lowPrefix;
                    while (op < endOfMatch) *op++ = *copyFrom++;
                }
                else
                {
                    MEM_CPY(op, lowPrefix, copySize);
                    op += copySize;
                }
            }
            continue;
        }
#endif
        case LZ4_COPYREP:
            trace(s->st.stage);

            /* copy repeated sequence */
//            cpy = op + length;
            if (s->avail_out < 8) return LZ4_OK;
            if (s->st.matchLen)
            {
                if (unlikely((s->st.bufCount-s->st.matchPos)<8))
                {
                    const size_t dec64 = dec64table[s->st.bufCount-s->st.matchPos];
                    outByte(s, s->st.decBuf[s->st.bufCount-s->st.matchPos--]);
                    outByte(s, s->st.decBuf[s->st.bufCount-s->st.matchPos--]);
                    outByte(s, s->st.decBuf[s->st.bufCount-s->st.matchPos--]);
                    outByte(s, s->st.decBuf[s->st.bufCount-s->st.matchPos--]);
                    s->st.matchLen -= dec32table[s->st.bufCount-s->st.matchLen];
                    outByte(s, s->st.decBuf[s->st.bufCount-s->st.matchPos--]);
                    outByte(s, s->st.decBuf[s->st.bufCount-s->st.matchPos--]);
                    outByte(s, s->st.decBuf[s->st.bufCount-s->st.matchPos--]);
                    outByte(s, s->st.decBuf[s->st.bufCount-s->st.matchPos--]);
                    s->st.matchLen += dec64;
                }
                else
                {
                    unsigned int i;

                    for (i = 0; i < 8; i++)
                        outByte(s, s->st.decBuf[s->st.bufCount-(s->st.matchPos-i)]);
                    s->st.matchPos -= 8;
                    s->st.matchLen -= 8;
                }
            }
            s->st.stage = LZ4_LASTLITS;
            s->st.lastLit = 5;

        case LZ4_LASTLITS:
            trace(s->st.stage);

//            if (s->avail_out < 12) return LZ4_OK;

            while (s->st.lastLit)
            {
                if (!s->avail_out) return LZ4_OK;
//                if (!loadByte(s)) return LZ4_OK;
                outByte(s, s->st.decBuf[s->st.bufCount-s->st.matchPos]);
                s->st.matchPos--;
//                s->avail_out--;
                s->st.lastLit--;
//              LZ4_wildCopy(op, ip, cpy);
//              ip += length; op = cpy;
            }
            s->st.stage = LZ4_START;
            break;

#if 0
            if (unlikely(cpy>oend-12))
            {
                if (cpy > oend-LASTLITERALS) goto _output_error;    /* Error : last LASTLITERALS bytes must be literals */
                if (op < oend-8)
                {
                    LZ4_wildCopy(op, match, oend-8);
                    match += (oend-8) - op;
                    op = oend-8;
                }
                while (op<cpy) *op++ = *match++;
            }
            else
                LZ4_wildCopy(op, match, cpy);
            op=cpy;   /* correction */
#endif
        }
    }
    flushOut(s);

    return LZ4_OK;

#if 0
    /* end of decoding */
    if (endOnInput)
       return (int) (((char*)op)-dest);     /* Nb of output bytes decoded */
    else
       return (int) (((const char*)ip)-source);   /* Nb of input bytes read */

    /* Overflow error detected */
_output_error:
    return (int) (-(((const char*)ip)-source))-1;
#endif
}

#else


/*******************************
*  Decompression functions
*******************************/
/*
 * This generic decompression function cover all use cases.
 * It shall be instantiated several times, using different sets of directives
 * Note that it is essential this generic function is really inlined,
 * in order to remove useless branches during compilation optimization.
 */
FORCE_INLINE int LZ4_decompress_generic(
                 const char* const source,
                 char* const dest,
                 int inputSize,
                 int outputSize,         /* If endOnInput==endOnInputSize, this value is the max size of Output Buffer. */

                 int endOnInput,         /* endOnOutputSize, endOnInputSize */
                 int partialDecoding,    /* full, partial */
                 int targetOutputSize,   /* only used if partialDecoding==partial */
                 int dict,               /* noDict, withPrefix64k, usingExtDict */
                 const BYTE* const lowPrefix,  /* == dest if dict == noDict */
                 const BYTE* const dictStart,  /* only if dict==usingExtDict */
                 const size_t dictSize         /* note : = 0 if noDict */
                 )
{
    /* Local Variables */
    const BYTE* ip = (const BYTE*) source;
    const BYTE* const iend = ip + inputSize;

    BYTE* op = (BYTE*) dest;
    BYTE* const oend = op + outputSize;
    BYTE* cpy;
    BYTE* oexit = op + targetOutputSize;
    const BYTE* const lowLimit = lowPrefix - dictSize;

    const BYTE* const dictEnd = (const BYTE*)dictStart + dictSize;
    const size_t dec32table[] = {4, 1, 2, 1, 4, 4, 4, 4};
    const size_t dec64table[] = {0, 0, 0, (size_t)-1, 0, 1, 2, 3};

    const int safeDecode = (endOnInput==endOnInputSize);
    const int checkOffset = ((safeDecode) && (dictSize < (int)(64 KB)));


    /* Special cases */
    if ((partialDecoding) && (oexit> oend-MFLIMIT)) oexit = oend-MFLIMIT;                         /* targetOutputSize too high => decode everything */
    if ((endOnInput) && (unlikely(outputSize==0))) return ((inputSize==1) && (*ip==0)) ? 0 : -1;  /* Empty output buffer */
    if ((!endOnInput) && (unlikely(outputSize==0))) return (*ip==0?1:-1);


    /* Main Loop */
    while (1)
    {
        unsigned token;
        size_t length;
        const BYTE* match;

        /* get literal length */
        token = *ip++;
        if ((length=(token>>ML_BITS)) == RUN_MASK)
        {
            unsigned s;
            do
            {
                s = *ip++;
                length += s;
            }
            while (likely((endOnInput)?ip<iend-RUN_MASK:1) && (s==255));
            if ((safeDecode) && unlikely((size_t)(op+length)<(size_t)(op))) goto _output_error;   /* overflow detection */
            if ((safeDecode) && unlikely((size_t)(ip+length)<(size_t)(ip))) goto _output_error;   /* overflow detection */
        }

        /* copy literals */
        cpy = op+length;
        if (((endOnInput) && ((cpy>(partialDecoding?oexit:oend-MFLIMIT)) || (ip+length>iend-(2+1+LASTLITERALS))) )
            || ((!endOnInput) && (cpy>oend-COPYLENGTH)))
        {
            if (partialDecoding)
            {
                if (cpy > oend) goto _output_error;                           /* Error : write attempt beyond end of output buffer */
                if ((endOnInput) && (ip+length > iend)) goto _output_error;   /* Error : read attempt beyond end of input buffer */
            }
            else
            {
                if ((!endOnInput) && (cpy != oend)) goto _output_error;       /* Error : block decoding must stop exactly there */
                if ((endOnInput) && ((ip+length != iend) || (cpy > oend))) goto _output_error;   /* Error : input must be consumed */
            }
            MEM_CPY(op, ip, length);
            ip += length;
            op += length;
            break;     /* Necessarily EOF, due to parsing restrictions */
        }
        LZ4_wildCopy(op, ip, cpy);
        ip += length; op = cpy;

        /* get offset */
        match = cpy - LZ4_readLE16(ip); ip+=2;
        if ((checkOffset) && (unlikely(match < lowLimit))) goto _output_error;   /* Error : offset outside destination buffer */

        /* get matchlength */
        length = token & ML_MASK;
        if (length == ML_MASK)
        {
            unsigned s;
            do
            {
                if ((endOnInput) && (ip > iend-LASTLITERALS)) goto _output_error;
                s = *ip++;
                length += s;
            } while (s==255);
            if ((safeDecode) && unlikely((size_t)(op+length)<(size_t)op)) goto _output_error;   /* overflow detection */
        }
        length += MINMATCH;

        /* check external dictionary */
        if ((dict==usingExtDict) && (match < lowPrefix))
        {
            if (unlikely(op+length > oend-LASTLITERALS)) goto _output_error;   /* doesn't respect parsing restriction */

            if (length <= (size_t)(lowPrefix-match))
            {
                /* match can be copied as a single segment from external dictionary */
                match = dictEnd - (lowPrefix-match);
                MEM_MOVE(op, match, length); op += length;
            }
            else
            {
                /* match encompass external dictionary and current segment */
                size_t copySize = (size_t)(lowPrefix-match);
                MEM_CPY(op, dictEnd - copySize, copySize);
                op += copySize;
                copySize = length - copySize;
                if (copySize > (size_t)(op-lowPrefix))   /* overlap within current segment */
                {
                    BYTE* const endOfMatch = op + copySize;
                    const BYTE* copyFrom = lowPrefix;
                    while (op < endOfMatch) *op++ = *copyFrom++;
                }
                else
                {
                    MEM_CPY(op, lowPrefix, copySize);
                    op += copySize;
                }
            }
            continue;
        }

        /* copy repeated sequence */
        cpy = op + length;
        if (unlikely((op-match)<8))
        {
            const size_t dec64 = dec64table[op-match];
            op[0] = match[0];
            op[1] = match[1];
            op[2] = match[2];
            op[3] = match[3];
            match += dec32table[op-match];
            LZ4_copy4(op+4, match);
            op += 8; match -= dec64;
        } else { LZ4_copy8(op, match); op+=8; match+=8; }

        if (unlikely(cpy>oend-12))
        {
            if (cpy > oend-LASTLITERALS) goto _output_error;    /* Error : last LASTLITERALS bytes must be literals */
            if (op < oend-8)
            {
                LZ4_wildCopy(op, match, oend-8);
                match += (oend-8) - op;
                op = oend-8;
            }
            while (op<cpy) *op++ = *match++;
        }
        else
            LZ4_wildCopy(op, match, cpy);
        op=cpy;   /* correction */
    }

    /* end of decoding */
    if (endOnInput)
       return (int) (((char*)op)-dest);     /* Nb of output bytes decoded */
    else
       return (int) (((const char*)ip)-source);   /* Nb of input bytes read */

    /* Overflow error detected */
_output_error:
    return (int) (-(((const char*)ip)-source))-1;
}

#endif



#if 1
int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize, endOnInputSize, full, 0, noDict, (BYTE*)dest, NULL, 0);
}

int LZ4_decompress_safe_partial(const char* source, char* dest, int compressedSize, int targetOutputSize, int maxDecompressedSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize, endOnInputSize, partial, targetOutputSize, noDict, (BYTE*)dest, NULL, 0);
}

int LZ4_decompress_fast(const char* source, char* dest, int originalSize)
{
    return LZ4_decompress_generic(source, dest, 0, originalSize, endOnOutputSize, full, 0, withPrefix64k, (BYTE*)(dest - 64 KB), NULL, 64 KB);
}

#endif
/* streaming decompression functions */

typedef struct
{
    const BYTE* externalDict;
    size_t extDictSize;
    const BYTE* prefixEnd;
    size_t prefixSize;
} LZ4_streamDecode_t_internal;

/*
 * If you prefer dynamic allocation methods,
 * LZ4_createStreamDecode()
 * provides a pointer (void*) towards an initialized LZ4_streamDecode_t structure.
 */
LZ4_streamDecode_t* LZ4_createStreamDecode(void)
{
    LZ4_streamDecode_t* lz4s = (LZ4_streamDecode_t*) ALLOCATOR(1, sizeof(LZ4_streamDecode_t));
    return lz4s;
}

int LZ4_freeStreamDecode (LZ4_streamDecode_t* LZ4_stream)
{
    FREEMEM(LZ4_stream);
    return 0;
}

/*
 * LZ4_setStreamDecode
 * Use this function to instruct where to find the dictionary
 * This function is not necessary if previous data is still available where it was decoded.
 * Loading a size of 0 is allowed (same effect as no dictionary).
 * Return : 1 if OK, 0 if error
 */
int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize)
{
    LZ4_streamDecode_t_internal* lz4sd = (LZ4_streamDecode_t_internal*) LZ4_streamDecode;
    lz4sd->prefixSize = (size_t) dictSize;
    lz4sd->prefixEnd = (const BYTE*) dictionary + dictSize;
    lz4sd->externalDict = NULL;
    lz4sd->extDictSize  = 0;
    return 1;
}
#if 1
/*
*_continue() :
    These decoding functions allow decompression of multiple blocks in "streaming" mode.
    Previously decoded blocks must still be available at the memory position where they were decoded.
    If it's not possible, save the relevant part of decoded data into a safe buffer,
    and indicate where it stands using LZ4_setStreamDecode()
*/
int LZ4_decompress_safe_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int compressedSize, int maxOutputSize)
{
    LZ4_streamDecode_t_internal* lz4sd = (LZ4_streamDecode_t_internal*) LZ4_streamDecode;
    int result;

    if (lz4sd->prefixEnd == (BYTE*)dest)
    {
        result = LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                        endOnInputSize, full, 0,
                                        usingExtDict, lz4sd->prefixEnd - lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize += result;
        lz4sd->prefixEnd  += result;
    }
    else
    {
        lz4sd->extDictSize = lz4sd->prefixSize;
        lz4sd->externalDict = lz4sd->prefixEnd - lz4sd->extDictSize;
        result = LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize,
                                        endOnInputSize, full, 0,
                                        usingExtDict, (BYTE*)dest, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize = result;
        lz4sd->prefixEnd  = (BYTE*)dest + result;
    }

    return result;
}

int LZ4_decompress_fast_continue (LZ4_streamDecode_t* LZ4_streamDecode, const char* source, char* dest, int originalSize)
{
    LZ4_streamDecode_t_internal* lz4sd = (LZ4_streamDecode_t_internal*) LZ4_streamDecode;
    int result;

    if (lz4sd->prefixEnd == (BYTE*)dest)
    {
        result = LZ4_decompress_generic(source, dest, 0, originalSize,
                                        endOnOutputSize, full, 0,
                                        usingExtDict, lz4sd->prefixEnd - lz4sd->prefixSize, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize += originalSize;
        lz4sd->prefixEnd  += originalSize;
    }
    else
    {
        lz4sd->extDictSize = lz4sd->prefixSize;
        lz4sd->externalDict = (BYTE*)dest - lz4sd->extDictSize;
        result = LZ4_decompress_generic(source, dest, 0, originalSize,
                                        endOnOutputSize, full, 0,
                                        usingExtDict, (BYTE*)dest, lz4sd->externalDict, lz4sd->extDictSize);
        if (result <= 0) return result;
        lz4sd->prefixSize = originalSize;
        lz4sd->prefixEnd  = (BYTE*)dest + originalSize;
    }

    return result;
}


/*
Advanced decoding functions :
*_usingDict() :
    These decoding functions work the same as "_continue" ones,
    the dictionary must be explicitly provided within parameters
*/

FORCE_INLINE int LZ4_decompress_usingDict_generic(const char* source, char* dest, int compressedSize, int maxOutputSize, int safe, const char* dictStart, int dictSize)
{
    if (dictSize==0)
        return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, noDict, (BYTE*)dest, NULL, 0);
    if (dictStart+dictSize == dest)
    {
        if (dictSize >= (int)(64 KB - 1))
            return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, withPrefix64k, (BYTE*)dest-64 KB, NULL, 0);
        return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, noDict, (BYTE*)dest-dictSize, NULL, 0);
    }
    return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, safe, full, 0, usingExtDict, (BYTE*)dest, (const BYTE*)dictStart, dictSize);
}

int LZ4_decompress_safe_usingDict(const char* source, char* dest, int compressedSize, int maxOutputSize, const char* dictStart, int dictSize)
{
    return LZ4_decompress_usingDict_generic(source, dest, compressedSize, maxOutputSize, 1, dictStart, dictSize);
}

int LZ4_decompress_fast_usingDict(const char* source, char* dest, int originalSize, const char* dictStart, int dictSize)
{
    return LZ4_decompress_usingDict_generic(source, dest, 0, originalSize, 0, dictStart, dictSize);
}

/* debug function */
int LZ4_decompress_safe_forceExtDict(const char* source, char* dest, int compressedSize, int maxOutputSize, const char* dictStart, int dictSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxOutputSize, endOnInputSize, full, 0, usingExtDict, (BYTE*)dest, (const BYTE*)dictStart, dictSize);
}
#endif

#endif   /* LZ4_COMMONDEFS_ONLY */
