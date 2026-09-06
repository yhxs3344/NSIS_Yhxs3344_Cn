/*
 * mt_bzip2_common.h
 *
 * This file is a part of the multithread wrapper for NSIS.
 * 
 * Copyright (C) 2021-2023, 2026 Jason Ross (JasonFriday13)
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __MTBZIP2COMMON_H__
#define __MTBZIP2COMMON_H__

#if defined (__cplusplus)
extern "C" {
#endif

#define MT_BZIP2_BLOCK_HEADER_SIZE  3
#define MT_BZIP2_BLOCK_DATA_SIZE    (NSIS_COMPRESS_BZIP2_LEVEL * 100000) // 100k <-> 900k MB
#define MT_BZIP2_BLOCK_SIZE         (MT_BZIP2_BLOCK_HEADER_SIZE + MT_BZIP2_BLOCK_DATA_SIZE)
#define MT_BZIP2_BLOCK_BUF_SIZE     (MT_BZIP2_BLOCK_DATA_SIZE + 1024 + (MT_BZIP2_BLOCK_DATA_SIZE / 10))

#if defined (__cplusplus)
}
#endif

#endif
