/*
 * mt_lz4_common.h
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

#ifndef __MTLZ4COMMON_H__
#define __MTLZ4COMMON_H__

#if defined (__cplusplus)
extern "C" {
#endif

#define MT_LZ4_BLOCK_HEADER_SIZE  3
#define MT_LZ4_BLOCK_DATA_SIZE    (1 << 20) // 1MB
#define MT_LZ4_BLOCK_SIZE         (MT_LZ4_BLOCK_HEADER_SIZE + MT_LZ4_BLOCK_DATA_SIZE)
#define MT_LZ4_BLOCK_BUF_SIZE     (MT_LZ4_BLOCK_DATA_SIZE + 1024 + (MT_LZ4_BLOCK_DATA_SIZE / 10))

#if defined (__cplusplus)
}
#endif

#endif
