/*
 * mmap.h
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

#ifndef __MMAP_H_
#define __MMAP_H_

#include "Platform.h"
#include "growbuf64.h"
#include "exehead/fileform.h"
#include "tchar.h"

#ifndef _WIN32
#include <cstdio> // for FILE*
#include <fstream> // (some systems have FILE* in here)
#endif

typedef union _DB_OFFSET {
  struct {
    UINT32 low_part;
    INT32 high_part;
  };
  INT64 quad_part;
} DB_OFFSET, *PDB_OFFSET;

class IMMap
{
  public:
    typedef DWORD maxfilesizetype; // TODO: Change to UINT64
    static inline UINT64 getmaxoffset() { return INT_MAX; } // NSIS offset limit is 31 bits right now

    virtual void resize(IGrowBuf64::size_type newlen)=0;
    virtual IGrowBuf64::size_type getsize() const=0;
    virtual void *get(IGrowBuf64::size_type offset, IGrowBuf64::size_type size) const=0;
    virtual void *get(IGrowBuf64::size_type offset, IGrowBuf64::size_type *size) const=0;
    virtual void *getmore(IGrowBuf64::size_type offset, IGrowBuf64::size_type size) const=0;
    virtual void release()=0;
    virtual void release(void *view, size_t size)=0;
    virtual void clear()=0;
    virtual void setro(BOOL bRO)=0;
    virtual void flush(size_t num)=0;
    virtual ~IMMap() {}

    virtual bool write_to_external_file(FILE*file, UINT64 size = ~(UINT64)0);
};

class MMapFile : public IMMap
{
  private: // don't copy instances
    MMapFile(const MMapFile&);
    void operator=(const MMapFile&);

  public:
    static DWORD getmaxfilesize();

    MMapFile();
    virtual ~MMapFile();

    /**
     * Closes the memory map and the file handle.
     */
    void clear();

    /**
     * Set read-only.
     * @param bRO Boolean value to set read-only.
     */
    void setro(BOOL bRO);

    /**
     * Creates the memory mapping object of the file with a mapping size.
     *
     * @param hFile The handle to the opened file.
     * @param size The size of the memory mapped object. You cannot set
     * this value to zero like with CreateFileMapping() because it will
     * immediately return.  Most likely, you want to set it to the size
     * of the file unless you want to only map a part of the file on
     * purpose.
     * @return Returns 1 on success, 0 on failure.
     */
#ifdef _WIN32
    bool setfile(HANDLE hFile, UINT64 size);
private:
    int internalsetfile(HANDLE hFile, DWORD dwSize);
#else
    bool setfile(FILE*hFile, UINT64 size);
private:
    int internalsetfile(int hFile, DWORD dwSize);
#endif

public:
    UINT64 setfile(const TCHAR*fpath);

#ifdef _WIN32
    static HANDLE openfilehelper(const TCHAR*fpath, UINT64 &size);
#else
    static FILE* openfilehelper(const TCHAR*fpath, UINT64 &size);
#endif

   /**
    * Resize the memory mapping of the file.  Used when the filesize has
    * changed.  When setfile has not been called previously, then it will
    * create a temporary file and use it to create a memory map.  This is
    * what's used by MMapBuf to create a Memory Mapped Buffer.
    * 
    * @param newsize The new size of the file.  Limited to 32-bits.
    */
    void resize(IGrowBuf64::size_type newsize);

    /**
     * Size of the memory map object.
     */
    IGrowBuf64::size_type getsize() const;

    /**
     * Set the memory map to a particular offset in the file and return the
     * memory mapped pointer to it.  Internally it may have to align to a
     * certain page size.
     * 
     * @param offset The offset from the beginning of the file.
     * @param size The size of the memory map window.
     */
    void *get(IGrowBuf64::size_type offset, IGrowBuf64::size_type size) const;

    /**
     * Set the memory map to a particular offset in the file and return the
     * memory mapped pointer to it.  Internally it may have to align to a
     * certain page size.
     * 
     * @param offset The offset from the beginning of the file.
     * @param sizep [in/out] The size of the memory map window.  (In non-Win32
     * systems, the new size is written back out.)
     */
    void *get(IGrowBuf64::size_type offset, IGrowBuf64::size_type *sizep) const;

    /**
     * This function sets memory map and just hands you the pointer and
     * it expects you to manage it.  So you need to call release(pView, size)
     * yourself or you will leak memory.
     *
     * Warning: This breaks encapsulation.  The user should probably just
     * create a new map.
     *
     * @param offset The offset from the beginning of the file.
     * @param size The size of the memory map window.
     */
    void *getmore(IGrowBuf64::size_type offset, IGrowBuf64::size_type size) const;

    /**
     * Releases the memory map currently being used.  Calls UnMapViewOfFile().
     */
    void release();

    /**
     * Releases the memory map pointed to by pView.  In Win32 systems
     * eventually calls UnmapViewOfFile().  Interestingly, the function
     * tries to align the pointer value back to the beginning of the
     * paged memory which is necessary because of the way get() works.
     *
     * This looks like it should only be used in conjunction with
     * getmore().  Otherwise, just call release().
     *
     * @param pView The pointer to somewhere in a MemMapped object.
     * @param size The size of the object.  Used only in non-Win32 systems.
     */
    void release(void *pView, size_t size);

    /**
     * Flushes the contents of the current memory map to disk.  Set size to 0
     * if you want to flush everything.
     *
     * @param num The number of bytes to flush.  0 for everything.
     */
    void flush(size_t num);

  private:
#ifdef _WIN32
    HANDLE m_hFile, m_hFileMap;
#else
    FILE *m_hFile;
    int m_hFileDesc;
    mutable IGrowBuf64::size_type m_iMappedSize;
#endif
    mutable void *m_pView;
    mutable IGrowBuf64::size_type m_iSize;
    BOOL m_bReadOnly;
    BOOL m_bTempHandle;

    static int m_iAllocationGranularity;
};

class MMapFake : public IMMap
{
  private: // don't copy instances
    MMapFake(const MMapFake&);
    void operator=(const MMapFake&);
  public:
    MMapFake();

    void set(const char *pMem, FIRST_INT_TYPE iSize);
    IGrowBuf64::size_type getsize() const;
    void *get(IGrowBuf64::size_type offset, IGrowBuf64::size_type size) const;
    void *get(IGrowBuf64::size_type offset, IGrowBuf64::size_type *size) const;
    void *getmore(IGrowBuf64::size_type offset, IGrowBuf64::size_type size) const;

    void resize(IGrowBuf64::size_type n);
    void release();
    void release(void *p, size_t size);
    void clear();
    void setro(BOOL b);
    void flush(size_t b);

  private:
    const char *m_pMem;
    IGrowBuf64::size_type m_iSize;
};

/**
 * A data structure that can be used to create a scratch file to do
 * work in. When it's smaller than 16mb, it's all in memory using the
 * GrowBuf class. But when it gets larger than 16mb, then it uses
 * the MMapFile class to create a memory map to a temporary file and
 * then uses it. This reduces memory overhead of the installer.
 *
 * This is sort of our virtual memory manager.
 */
class MMapBuf : public IGrowBuf64, public IMMap
{
  private: // don't copy instances
    MMapBuf(const MMapBuf&);
    void operator=(const MMapBuf&);
    size_type cur_limit;

  public:
    MMapBuf();
    virtual ~MMapBuf();

    void setlimit(size_type limit);
		size_type getlimit();

    IGrowBuf64::size_type add(const void *data, int len);
    void setro(BOOL bRO);
    void resize(IGrowBuf64::size_type newlen);
    IGrowBuf64::size_type getsize() const;
    IGrowBuf64::size_type getlen() const;
    void *get() const;
    void *get(IGrowBuf64::size_type offset, IGrowBuf64::size_type *sizep) const;
    void *get(IGrowBuf64::size_type offset, IGrowBuf64::size_type size) const;
    void *getmore(IGrowBuf64::size_type offset, IGrowBuf64::size_type size) const;
    void release();
    void release(void *pView, size_t size);
    void clear();
    void flush(size_t num);

  protected:
    static inline int getmodethreshold() { return 16 << 20; }

  private:
    GrowBuf64 m_gb;
    MMapFile m_fm;

    int m_gb_u;
    IGrowBuf64::size_type m_alloc, m_used;
};

#endif//__MMAP_H_

