/*
 * fileform.c
 * 
 * This file is a part of NSIS.
 * 
 * Copyright (C) 1999-2021 Nullsoft and Contributors
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

#include "../Platform.h"
#include "fileform.h"
#include "util.h"
#include "state.h"
#include "resource.h"
#include "lang.h"
#include "ui.h"
#include "exec.h"
#include "../crc32.h"
#include "../tchar.h"

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
#ifdef NSIS_COMPRESS_USE_ZLIB
#include "../zlib/ZLIB.H"
#endif

#ifdef NSIS_COMPRESS_USE_LZ4
#include "../lz4/lz4decompress.h"
#define z_stream lz4_decstream
#define inflateInit(x) lz4Init(x)
#define inflateReset(x) lz4Init(x)
#define inflate(x) lz4Decode(x)
#define Z_OK LZ4_OK
#define Z_STREAM_END LZ4_STREAM_END
#endif

#ifdef NSIS_COMPRESS_USE_LZMA
#include "../7zip/LZMADecode.h"
#define z_stream lzma_stream
#define inflateInit(x) lzmaInit(x)
#define inflateReset(x) lzmaInit(x)
#define inflate(x) lzmaDecode(x)
#define Z_OK LZMA_OK
#define Z_STREAM_END LZMA_STREAM_END
#endif

#ifdef NSIS_COMPRESS_USE_BZIP2
#include "../bzip2/bzlib.h"

#define z_stream DState
#define inflateInit(x) BZ2_bzDecompressInit(x)
#define inflateReset(x) BZ2_bzDecompressInit(x)

#define inflate(x) BZ2_bzDecompress(x)
#define Z_OK BZ_OK
#define Z_STREAM_END BZ_STREAM_END
#endif//NSIS_COMPRESS_USE_BZIP2
#endif//NSIS_CONFIG_COMPRESSION_SUPPORT

struct block_header g_blocks[BLOCKS_NUM];
header *g_header;
int g_flags;
UINT g_filehdrsize;
int g_is_uninstaller;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
int g_has_external_file;
int g_is_stub_installer;
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
#ifdef NSIS_CONFIG_CRC_SUPPORT
int g_do_crc = 0;
#endif //NSIS_CONFIG_CRC_SUPPORT

HANDLE g_db_hFile=INVALID_HANDLE_VALUE;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
HANDLE g_dbex_hFile=INVALID_HANDLE_VALUE;
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT

#if defined(NSIS_CONFIG_COMPRESSION_SUPPORT) && defined(NSIS_COMPRESS_WHOLE)
HANDLE dbd_hFile=INVALID_HANDLE_VALUE;
static INT64 dbd_size;
static UINT32 dbd_pos, dbd_srcpos, dbd_fulllen;
#endif//NSIS_COMPRESS_WHOLE

static UINT32 m_length;
static UINT32 m_pos;

#define _calc_percent() (MulDiv(min(m_pos>>1,m_length>>1),100,m_length>>1))
#ifdef NSIS_COMPRESS_WHOLE
static int NSISCALL calc_percent()
{
  return _calc_percent();
}
#else
static int NSISCALL calc_percent()
{
  return MulDiv(min(m_pos>>1,m_length>>1),100,m_length>>1);
}
#endif

#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
#if defined(NSIS_CONFIG_CRC_SUPPORT) || defined(NSIS_COMPRESS_WHOLE)
INT_PTR CALLBACK verProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_INITDIALOG)
  {
    SetTimer(hwndDlg,1,250,NULL);
    uMsg = WM_TIMER;
  }
  if (uMsg == WM_TIMER)
  {
    TCHAR bt[64];
    int percent=calc_percent();
#ifdef NSIS_COMPRESS_WHOLE
    TCHAR *msg=g_header?_LANG_UNPACKING:_LANG_VERIFYINGINST;
#else
    TCHAR *msg=_LANG_VERIFYINGINST;
#endif
    wsprintf(bt,msg,percent);
    my_SetWindowText(hwndDlg,bt);
    my_SetDialogItemText(hwndDlg,IDC_STR,bt);
  }
  return FALSE;
}

DWORD verify_time;

void handle_ver_dlg(BOOL kill)
{
  static HWND hwnd;

  if (kill)
  {
    if (hwnd) DestroyWindow(hwnd);
    hwnd = NULL;

    return;
  }

  if (hwnd)
  {
    MessageLoop(0);
  }
  else if (GetTickCount() > verify_time)
  {
#ifdef NSIS_COMPRESS_WHOLE
    if (g_hwnd)
    {
      if (g_exec_flags.status_update & 1)
      {
        TCHAR bt[64];
        wsprintf(bt, _T("... %d%%"), calc_percent());
        update_status_text(0, bt);
      }
    }
    else
#endif
    {
      hwnd = CreateDialog(
        g_hInstance,
        MAKEINTRESOURCE(IDD_VERIFY),
        0,
        verProc
      );
      ShowWindow(hwnd, SW_SHOW);
    }
  }
}

#endif//NSIS_CONFIG_CRC_SUPPORT || NSIS_COMPRESS_WHOLE
#endif//NSIS_CONFIG_VISIBLE_SUPPORT

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
static z_stream g_inflate_stream;
#endif

const TCHAR * NSISCALL loadHeaders(int cl_flags)
{
  UINT32 length_of_all_following_data, left;
#ifdef NSIS_CONFIG_CRC_SUPPORT
  crc32_t crc = 0, crc_header;
#endif//NSIS_CONFIG_CRC_SUPPORT

  void *data;
  firstheader h;
  header *header;

  HANDLE db_hFile;

#ifdef C_ASSERT
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
{C_ASSERT(sizeof(firstheader) == sizeof(int) * 9);}
#else
{C_ASSERT(sizeof(firstheader) == sizeof(int) * 7);}
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
{C_ASSERT(sizeof(struct block_header) == sizeof(UINT_PTR) + sizeof(int));}
{C_ASSERT(LASIF_FITCTLW >> LASIS_FITCTLW == 1);}
{C_ASSERT(LASIF_LR_LOADFROMFILE == LR_LOADFROMFILE);}
#endif

#ifdef NSIS_CONFIG_CRC_SUPPORT
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
  verify_time = GetTickCount() + 1000;
#endif
#endif//NSIS_CONFIG_CRC_SUPPORT

  GetModuleFileName(NULL, state_exe_path, NSIS_MAX_STRLEN);

  g_db_hFile = db_hFile = myOpenFile(state_exe_path, GENERIC_READ, OPEN_EXISTING);
  if (db_hFile == INVALID_HANDLE_VALUE)
  {
    return _LANG_CANTOPENSELF;
  }

  mystrcpy(state_exe_directory, state_exe_path);
  mystrcpy(state_exe_file, trimslashtoend(state_exe_directory));

  left = m_length = GetFileSize(db_hFile,NULL);
  while (left > 0)
  {
    static char temp[32768];
    DWORD l = min(left, (g_filehdrsize ? 32768UL : 512UL));
    if (!ReadSelfFile(temp, l))
    {
#if defined(NSIS_CONFIG_CRC_SUPPORT) && defined(NSIS_CONFIG_VISIBLE_SUPPORT)
      handle_ver_dlg(TRUE);
#endif//NSIS_CONFIG_CRC_SUPPORT
      return _LANG_INVALIDCRC;
    }

    if (!g_filehdrsize)
    {
      mini_memcpy(&h, temp, sizeof(firstheader));
      if (
           (h.flags & (~FH_FLAGS_MASK)) == 0 &&
           h.siginfo == FH_SIG &&
           h.nsinst[2] == FH_INT3 &&
           h.nsinst[1] == FH_INT2 &&
           h.nsinst[0] == FH_INT1
         )
      {
        g_filehdrsize = m_pos;

#if defined(NSIS_CONFIG_CRC_SUPPORT) || defined(NSIS_CONFIG_SILENT_SUPPORT)
        cl_flags |= h.flags;
#endif

#ifdef NSIS_CONFIG_SILENT_SUPPORT
        g_exec_flags.silent |= cl_flags & FH_FLAGS_SILENT;
#endif

        // force signed to unsigned conversion
        length_of_all_following_data = (UINT32)h.length_of_all_following_data;
        if (length_of_all_following_data > left)
          return _LANG_INVALIDCRC;

#ifdef NSIS_CONFIG_CRC_SUPPORT
        if ((cl_flags & FH_FLAGS_FORCE_CRC) == 0)
        {
          if (cl_flags & FH_FLAGS_NO_CRC)
            break;
        }

        g_do_crc++;
        l = sizeof(firstheader);
#ifndef NSIS_CONFIG_CRC_ANAL
        left = l;
        // end crc checking at crc :) this means you can tack stuff on the end and it'll still work.
#else //!NSIS_CONFIG_CRC_ANAL
        left -= (length_of_all_following_data - l);
#endif//NSIS_CONFIG_CRC_ANAL

        // set the file position, because we are skipping the main header and datablock
        m_pos += (length_of_all_following_data - l - sizeof(crc32_t));
        // really only needed for crc_anal mode
        SetSelfFilePointer64(m_pos + l - sizeof(crc32_t));

        // this is in case the end part is < 512 bytes.
        if (l > left) l = left;

#else//!NSIS_CONFIG_CRC_SUPPORT
        // no crc support, no need to keep on reading
        break;
#endif//!NSIS_CONFIG_CRC_SUPPORT
      }
    }
#ifdef NSIS_CONFIG_CRC_SUPPORT

#ifdef NSIS_CONFIG_VISIBLE_SUPPORT

#ifdef NSIS_CONFIG_SILENT_SUPPORT
    else if ((cl_flags & FH_FLAGS_SILENT) == 0)
#endif//NSIS_CONFIG_SILENT_SUPPORT
    {
      handle_ver_dlg(FALSE);
    }
#endif//NSIS_CONFIG_VISIBLE_SUPPORT

#ifndef NSIS_CONFIG_CRC_ANAL
    if (left < m_length)
#endif//NSIS_CONFIG_CRC_ANAL
      crc = CRC32(crc, (unsigned char*)temp, l);

#endif//NSIS_CONFIG_CRC_SUPPORT
    m_pos += l;
    left -= l;
  }
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (h.flags & FH_FLAGS_UNINSTALL)
    g_is_uninstaller++;
#endif
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  if (h.flags & FH_FLAGS_HAS_EXTERNAL_FILE)
    g_has_external_file++;

  if (h.flags & FH_FLAGS_IS_STUB_INSTALLER)
    g_is_stub_installer++;

  if (g_has_external_file && !g_is_uninstaller)
  {
    TCHAR path_ext[MAX_PATH];

    mystrcpy(path_ext, state_exe_path);
    mystrcpy(path_ext, trimextension(path_ext));
    wsprintf(path_ext, _T("%s.nsisbin"), path_ext);
    g_dbex_hFile = myOpenFile(path_ext, GENERIC_READ, OPEN_EXISTING);
    if (g_dbex_hFile == INVALID_HANDLE_VALUE && !g_is_stub_installer)
      return _LANG_INVALIDCRC;
  }
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT

#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
#ifdef NSIS_CONFIG_CRC_SUPPORT
  handle_ver_dlg(TRUE);
#endif//NSIS_CONFIG_CRC_SUPPORT
#endif//NSIS_CONFIG_VISIBLE_SUPPORT
  if (!g_filehdrsize)
    return _LANG_INVALIDCRC;

#ifdef NSIS_CONFIG_CRC_SUPPORT
  if (g_do_crc)
  {
    crc32_t fcrc;
    SetSelfFilePointer64(m_pos - sizeof(crc32_t));
    if (!ReadSelfFile(&crc_header, sizeof(crc32_t)))
      return _LANG_INVALIDCRC;

    crc = CRC32(crc, (unsigned char*)&crc_header, sizeof(crc32_t));
    if (!ReadSelfFile(&fcrc, sizeof(crc32_t)) || crc != fcrc)
      return _LANG_INVALIDCRC;
  }
#endif//NSIS_CONFIG_CRC_SUPPORT

  data = (void *)GlobalAlloc(GPTR,h.length_of_header);

#ifdef NSIS_COMPRESS_WHOLE
  inflateReset(&g_inflate_stream);

  {
    TCHAR fno[MAX_PATH];
    my_GetTempFileName(fno, state_temp_dir);
    dbd_hFile=CreateFile(fno,GENERIC_WRITE|GENERIC_READ,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,NULL);
    if (dbd_hFile == INVALID_HANDLE_VALUE)
      return _LANG_ERRORWRITINGTEMP;
  }
  dbd_srcpos = SetSelfFilePointer(g_filehdrsize + sizeof(firstheader));
#ifdef NSIS_CONFIG_CRC_SUPPORT
  dbd_fulllen = dbd_srcpos - sizeof(h) + h.length_of_all_following_data - ((h.flags & FH_FLAGS_NO_CRC) ? 0 : sizeof(crc32_t)*2);
#else
  dbd_fulllen = dbd_srcpos - sizeof(h) + h.length_of_all_following_data;
#endif//NSIS_CONFIG_CRC_SUPPORT
#else
  SetSelfFilePointer(g_filehdrsize + sizeof(firstheader));
#endif//NSIS_COMPRESS_WHOLE

  if (GetCompressedDataFromExeHeadToMemory(-1, data, h.length_of_header, crc_header) != h.length_of_header)
    return _LANG_INVALIDCRC;

  header = g_header = data;

  g_flags = header->flags;

  // set offsets to real memory offsets rather than installer's header offset
  left = BLOCKS_NUM;
  while (left--)
  {
#ifdef DEBUG
    if ((UINT_PTR) h.length_of_header < header->blocks[left].offset)
      return _LANG_GENERIC_ERROR; // Should never happen
#endif
    header->blocks[left].offset += (UINT_PTR) data;
  }

#ifdef NSIS_COMPRESS_WHOLE
  header->blocks[NB_DATA].offset = dbd_pos;
#else
  header->blocks[NB_DATA].offset = (UINT_PTR)((UINT64)SetFilePointer64(db_hFile, 0, FILE_CURRENT));
#endif

  mini_memcpy(&g_blocks, &header->blocks, sizeof(g_blocks));

  return 0;
}

#define IBUFSIZE 16384
#define OBUFSIZE 32768

// returns -3 if compression error/eof/etc

#if !defined(NSIS_COMPRESS_WHOLE) || !defined(NSIS_CONFIG_COMPRESSION_SUPPORT)

int myReadFileData(LPVOID buffer, const int len, const int exehead_only)
{
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  if (g_has_external_file && !exehead_only)
  {
    if (!ReadExternalFile(buffer, len)) return 0;
  }
  else
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  {
    if (!ReadSelfFile(buffer, len)) return 0;
  }
  return 1;
}

void mySetFilePointer(INT64 offset, int exehead_only)
{
  if (offset < 0) return;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  if (g_has_external_file && !exehead_only)
  {
    SetExternalFilePointer(offset);
  }
  else
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  {
    offset += g_blocks[NB_DATA].offset;
    SetSelfFilePointer64(offset);
  }
}

// Decompress data.
FIRST_INT_TYPE NSISCALL _dodecomp(int exehead_only, INT64 offset, HANDLE hFileOut, unsigned char *outbuf, int outbuflen, crc32_t ex_crc)
{
  static char inbuffer[IBUFSIZE+OBUFSIZE];
  char *outbuffer;
  int outbuffer_len=outbuf?outbuflen:OBUFSIZE;
  FIRST_INT_TYPE retval=0;
  FIRST_INT_TYPE input_len;
#ifdef NSIS_CONFIG_CRC_SUPPORT
  crc32_t crc = 0;
#endif

  outbuffer = outbuf?(char*)outbuf:(inbuffer+IBUFSIZE);

  mySetFilePointer(offset, exehead_only);
  if (!myReadFileData((LPVOID)&input_len, FIRST_INT_SIZEOF, exehead_only)) return -3;
#ifdef NSIS_CONFIG_CRC_SUPPORT
  if (g_do_crc) crc = CRC32(crc, (unsigned char*)&input_len, FIRST_INT_SIZEOF);
#endif

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
  if (input_len & FIRST_INT_FLAG) // compressed
  {
    TCHAR progress[64];
    FIRST_INT_TYPE input_len_total;
    DWORD ltc = GetTickCount(), tc;

    inflateReset(&g_inflate_stream);
    input_len_total = input_len &= FIRST_INT_MASK; // take off top bit.

    while (input_len > 0)
    {
      int l=(int)min(input_len,(FIRST_INT_TYPE)IBUFSIZE);
      int err;

      if (!myReadFileData((LPVOID)inbuffer,l,exehead_only)) return -3;
#ifdef NSIS_CONFIG_CRC_SUPPORT
      if (g_do_crc) crc = CRC32(crc, (unsigned char*)inbuffer, l);
#endif
      g_inflate_stream.next_in = (unsigned char*) inbuffer;
      g_inflate_stream.avail_in = l;
      input_len-=l;

      for (;;)
      {
        int u;

        g_inflate_stream.next_out = (unsigned char*) outbuffer;
        g_inflate_stream.avail_out = (unsigned int)outbuffer_len;

        err=inflate(&g_inflate_stream);

        if (err<0) return -4;

        u=BUGBUG64TRUNCATE(int, (size_t)((char*)g_inflate_stream.next_out - outbuffer));

        tc=GetTickCount();
        if (g_exec_flags.status_update & 1 && (tc - ltc > 200 || !input_len))
        {
          int ret;
          UINT32 this_pos, this_len;
          ULARGE_INTEGER file_pos, file_len;
          
          file_len.QuadPart = input_len_total;
          file_pos.QuadPart = input_len_total - input_len;
              
          if (file_len.HighPart)
          { 
            // 16MB chunks, which gives a 64PB limit. Should be big enough for now :).
            this_len = (UINT32)(file_len.HighPart << 8);
            this_len |= (UINT32)(file_len.LowPart >> 24);
            this_pos = (UINT32)(file_pos.HighPart << 8);
            this_pos |= (UINT32)(file_pos.LowPart >> 24);
          }
          else
          {
            this_len = file_len.LowPart;
            this_pos = file_pos.LowPart;
          }
          ret = MulDiv(min(this_pos>>1,this_len>>1),100,this_len>>1);
          wsprintf(progress, _T("... %d%%"), ret);
          update_status_text(0, progress);
          ltc=tc;
        }

        // if there's no output, more input is needed
        if (!u)
          break;

        if (!outbuf)
        {
          if (!myWriteFile(hFileOut,outbuffer,u)) return -2;
          retval+=u;
        }
        else
        {
          retval+=u;
          outbuffer_len-=u;
          outbuffer=(char*)g_inflate_stream.next_out;
        }
        if (err==Z_STREAM_END)
        {
#ifdef NSIS_CONFIG_CRC_SUPPORT
          if (g_do_crc && crc != ex_crc) return -3;
#endif
          return retval;
        }
      }
    }
  }
  else
#endif//NSIS_CONFIG_COMPRESSION_SUPPORT
  {
    if (!outbuf)
    {
      while (input_len > 0)
      {
        DWORD l=(DWORD)min(input_len,(FIRST_INT_TYPE)outbuffer_len);
        if (!myReadFileData((LPVOID)inbuffer,l,exehead_only)) return -3;
#ifdef NSIS_CONFIG_CRC_SUPPORT
        if (g_do_crc) crc = CRC32(crc, (unsigned char*)inbuffer, l);
#endif
        if (!myWriteFile(hFileOut,inbuffer,l)) return -2;
        retval+=l;
        input_len-=l;
      }
    }
    else
    {
      int l=(int)min(input_len,(FIRST_INT_TYPE)outbuflen);
      if (!myReadFileData((LPVOID)outbuf,l,exehead_only)) return -3;
#ifdef NSIS_CONFIG_CRC_SUPPORT
      if (g_do_crc) crc = CRC32(crc, outbuf, l);
#endif
      retval=l;
    }
  }
#ifdef NSIS_CONFIG_CRC_SUPPORT
  if (g_do_crc && crc != ex_crc) return -3;
#endif
  return retval;
}
#else//NSIS_COMPRESS_WHOLE

static char _inbuffer[IBUFSIZE];
static char _outbuffer[OBUFSIZE];
extern UINT32 m_length;
extern UINT32 m_pos;
extern INT_PTR CALLBACK verProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
static int NSISCALL __ensuredata(INT64 amount)
{
  INT64 needed=amount-(dbd_size-dbd_pos);
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
  verify_time=GetTickCount()+500;
#endif
  if (needed>0)
  {
    SetSelfFilePointer64(dbd_srcpos);
    SetFilePointer64(dbd_hFile,dbd_size,FILE_BEGIN);
    m_length=(UINT32)needed;
    m_pos=0;
    for (;;)
    {
      int err;
      UINT32 l=min(IBUFSIZE,dbd_fulllen-dbd_srcpos);
      if (!ReadSelfFile((LPVOID)_inbuffer,l)) return -1;
      dbd_srcpos+=l;
      g_inflate_stream.next_in=(unsigned char*)_inbuffer;
      g_inflate_stream.avail_in=l;
      do
      {
        DWORD r;
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
        if (g_header)
#ifdef NSIS_CONFIG_SILENT_SUPPORT
          if (!g_exec_flags.silent)
#endif
          {
            m_pos=m_length-(UINT32)(amount-(dbd_size-dbd_pos));

            handle_ver_dlg(FALSE);
          }
#endif//NSIS_CONFIG_VISIBLE_SUPPORT
        g_inflate_stream.next_out=(unsigned char*)_outbuffer;
        g_inflate_stream.avail_out=OBUFSIZE;
        err=inflate(&g_inflate_stream);
        if (err<0)
        {
          return -3;
        }
        r=BUGBUG64TRUNCATE(DWORD,(UINT_PTR)g_inflate_stream.next_out)-BUGBUG64TRUNCATE(DWORD,(UINT_PTR)_outbuffer);
        if (r)
        {
          if (!myWriteFile(dbd_hFile,_outbuffer,r))
          {
            return -2;
          }
          dbd_size+=r;
        }
        else if (g_inflate_stream.avail_in || !l) return -3;
        else break;
      }
      while (g_inflate_stream.avail_in);
      if (amount-(dbd_size-dbd_pos) <= 0) break;
    }
    SetFilePointer64(dbd_hFile,dbd_pos,FILE_BEGIN);
  }
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
  handle_ver_dlg(TRUE);
#endif//NSIS_CONFIG_VISIBLE_SUPPORT
  return 0;
}

FIRST_INT_TYPE NSISCALL _dodecomp(int exehead_only, INT64 offset, HANDLE hFileOut, unsigned char *outbuf, int outbuflen, crc32_t ex_crc)
{
  crc32_t crc = 0;
  DWORD r;
  FIRST_INT_TYPE input_len;
  FIRST_INT_TYPE retval;
  if (offset>=0)
  {
    UINT_PTR datofs=(UINT_PTR)(g_blocks[NB_DATA].offset+(UINT64)offset);
#if (NSIS_MAX_EXEDATASIZE+0) > 0x7fffffffUL
#error "SetFilePointer is documented to only support signed 32-bit offsets in lDistanceToMove"
#endif
    dbd_pos=(UINT32)datofs;
    SetFilePointer64(dbd_hFile,datofs,FILE_BEGIN);
  }
  retval=__ensuredata(FIRST_INT_SIZEOF);
  if (retval<0) return retval;

  if (!myReadFile(dbd_hFile,(LPVOID)&input_len,FIRST_INT_SIZEOF)) return -3;
#ifdef NSIS_CONFIG_CRC_SUPPORT
  if (g_do_crc) crc = CRC32(crc, (unsigned char*)&input_len, FIRST_INT_SIZEOF);
#endif
  dbd_pos+=FIRST_INT_SIZEOF;

  retval=__ensuredata(input_len);
  if (retval < 0) return retval;

  if (!outbuf)
  {
    while (input_len > 0)
    {
      DWORD l=(DWORD)min(input_len,(FIRST_INT_TYPE)IBUFSIZE);
      if (!myReadFile(dbd_hFile,(LPVOID)_inbuffer,r=l)) return -3;
#ifdef NSIS_CONFIG_CRC_SUPPORT
      if (g_do_crc) crc = CRC32(crc, (unsigned char*)_inbuffer, r);
#endif
      if (!myWriteFile(hFileOut,_inbuffer,r)) return -2;
      retval+=r;
      input_len-=r;
      dbd_pos+=r;
    }
  }
  else
  {
    if (!ReadFile(dbd_hFile,(LPVOID)outbuf,(DWORD)min(input_len,(FIRST_INT_TYPE)outbuflen),&r,NULL)) return -3;
#ifdef NSIS_CONFIG_CRC_SUPPORT
    if (g_do_crc) crc = CRC32(crc, outbuf, r);
#endif
    retval=r;
    dbd_pos+=r;
  }
#ifdef NSIS_CONFIG_CRC_SUPPORT
  if (g_do_crc && crc != ex_crc) return -3;
#endif
  return retval;
}
#endif//NSIS_COMPRESS_WHOLE

BOOL NSISCALL ReadSelfFile(LPVOID lpBuffer, DWORD nNumberOfBytesToRead)
{
  return myReadFile(g_db_hFile,lpBuffer,nNumberOfBytesToRead);
}

INT64 NSISCALL SetFilePointer64(HANDLE handle, INT64 lDistanceToMove, DWORD flags)
{
  LARGE_INTEGER liDistanceToMove;
  liDistanceToMove.QuadPart = lDistanceToMove;
  liDistanceToMove.LowPart = SetFilePointer(handle, liDistanceToMove.LowPart, &liDistanceToMove.HighPart, flags);
  if (liDistanceToMove.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != ERROR_SUCCESS)
  {
    liDistanceToMove.QuadPart = -1;
  }
  return liDistanceToMove.QuadPart;
}

DWORD NSISCALL SetSelfFilePointer(LONG lDistanceToMove)
{
  LARGE_INTEGER liDistanceToMove;
  liDistanceToMove.QuadPart = SetFilePointer64(g_db_hFile, lDistanceToMove, FILE_BEGIN);
  return liDistanceToMove.LowPart;
}
INT64 NSISCALL SetSelfFilePointer64(INT64 lDistanceToMove)
{
  return SetFilePointer64(g_db_hFile, lDistanceToMove, FILE_BEGIN);
}

#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
BOOL NSISCALL ReadExternalFile(LPVOID lpBuffer, DWORD nNumberOfBytesToRead)
{
  return myReadFile(g_dbex_hFile, lpBuffer, nNumberOfBytesToRead);
}

INT64 NSISCALL SetExternalFilePointer(INT64 lDistanceToMove)
{
  return SetFilePointer64(g_dbex_hFile, lDistanceToMove, FILE_BEGIN);
}
#endif
