/*
 * growbuf64.h
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
 * Unicode support by Jim Park -- 08/22/2007
 */

#ifndef __GROWBUF64_H_
#define __GROWBUF64_H_

#include "Platform.h"

/**
 * IGrowBuf64 is the interface to a buffer that grows as you
 * add to the buffer.
 */
class IGrowBuf64
{
  public:
    typedef INT64 size_type;
		static size_type getlimit() { return (size_type)0x7fffffffffffffff; }
    virtual ~IGrowBuf64() {}

    /**
     * Add data to the buffer.
     * @param data Pointer to the data to be stored.
     * @param len Size of the data in bytes.
     * @return the previous logical size in bytes before the addition.
     */
    virtual size_type add(const void *data, int len)=0;

    /**
     * Resizes the buffer to hold the number of bytes specified.
     * @param newlen the desired logical size of the buffer.
     */
    virtual void resize(size_type newlen)=0;

    /**
     * Get the length of the logical buffer in bytes.
     * @return the length in bytes
     */
    virtual size_type getlen() const=0;

    /**
     * Get the buffer itself.
     * @return Void pointer to the buffer.
     */
    virtual void *get() const=0;
};

/**
 * GrowBuf64 implements IGrowBuf64 and grows in 32K chunks.
 */
class GrowBuf64 : public IGrowBuf64
{
  private: // don't copy instances
    GrowBuf64(const GrowBuf64&);
    void operator=(const GrowBuf64&);

  public:
    GrowBuf64();
    virtual ~GrowBuf64();

    /**
     * Set whether to zero out buffer
     * @param zero A boolean value.
     */
    void set_zeroing(bool zero);

    /**
     * Add data to the buffer.
     * @param data Pointer to the data to be stored.
     * @param len Size of the data in bytes.
     * @return the previous logical size in bytes before the addition.
     */
    size_type add(const void *data, int len);

    /**
     * Resizes the buffer to hold the number of bytes specified.
     * Setting the newlen to 0 will cause the buffer to be at most
     * 2*m_bs bytes long.  (It will free the buffer if > 2*m_bs.)
     * @param newlen the desired logical size of the buffer.
     */
    void resize(size_type newlen);

    /**
     * Get the length of the logical buffer in bytes.
     * @return the length in bytes
     */
    size_type getlen() const;

    /**
     * Get the buffer itself.
     * @return Void pointer to the buffer.
     */
    void *get() const;

    void swap(GrowBuf64&other);

  private:
    void *m_s;    /* the storage buffer */
    size_type m_alloc;  /* allocated bytes */
    size_type m_used;   /* how many bytes of the buffer is used? */
    bool m_zero;   /* should storage be zeroed out? */

  protected:
    unsigned short m_bs;     // byte-size to grow by
};

/**
 * TinyGrowBuf64 is a derived class that grows the buffer
 * in tiny increments.
 */
class TinyGrowBuf64 : public GrowBuf64 {
  public:
    TinyGrowBuf64() : GrowBuf64() { m_bs=1024; }
};

#endif

