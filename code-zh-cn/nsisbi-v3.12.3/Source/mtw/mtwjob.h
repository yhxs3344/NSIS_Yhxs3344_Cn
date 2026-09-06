/*
 * mtwjob.h
 *
 * Job manager that controls memory and access to said memory.
 *
 * Copyright (C) 2023 Jason Ross (JasonFriday13)
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __MTWJOB_H__
#define __MTWJOB_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "../Platform.h"

typedef struct
{
  char *input, *output;                     /* buffers that hold the data */
  unsigned int input_size, output_size;     /* number of bytes available in the buffers */
  UINT64 seq_num;                           /* where in the sequence this block belongs */
  int ret_value;                            /* codec return value */
  void *optional;                           /* optional extra data that might be useful */
#ifndef EXEHEAD
  CINFO comp_info;                          /* compressor info (unused for decompression) */
#endif
} MTWJOB, *PMTWJOB;

int InitJobContext(void);
void FreeJobContext(void);

PMTWJOB MakeJob(const char* data, unsigned int in_size, unsigned int out_size, UINT64 seq_num, void* optional);
int FinishJob(const PMTWJOB job);

unsigned int JobCountNextOut(void);
int JobHaveNextOut(UINT64 seq_num);
char *JobGetNextOut(char *out, UINT64 seq_num, int *ret_value);

#ifdef __cplusplus
}
#endif

#endif
