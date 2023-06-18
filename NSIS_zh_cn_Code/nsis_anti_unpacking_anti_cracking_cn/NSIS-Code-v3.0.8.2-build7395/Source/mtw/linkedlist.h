/*
 * linkedlist.h
 *
 * Double linked list that stores data.
 *
 * Copyright (C) 2021-2023 Jason Ross (JasonFriday13)
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "../Platform.h"

struct VARITEM
{
  struct VARITEM *next;
  struct VARITEM *prev;

  char *data;          /* next byte */
  unsigned int size;   /* number of bytes available at 'data' */
  UINT64 seq_num;      /* where in the sequence this block belongs */
  int ret_value;       /* codec return value */
};

struct VARLIST
{
  struct VARITEM head;
  struct VARITEM tail;
};

void InitVarList(struct VARLIST *p_list);
void CleanVarList(struct VARLIST *p_list);
UINT64 GetListCount(struct VARLIST *p_list);
int AddData(const char *data, unsigned int len, UINT64 seq_num, int ret_value, struct VARLIST *p_list);
int FindSeqNum(UINT64 seq_num, struct VARLIST *p_list);
unsigned int GetDataLen(UINT64 seq_num, struct VARLIST *p_list);
char *GetData(char *out, UINT64 seq_num, int *ret_value, struct VARLIST *p_list);
// only used by the threads.
UINT64 GetFirstItemSeqNum(struct VARLIST *p_list);
unsigned int GetFirstItemLen(struct VARLIST *p_list);
char *GetFirstItemData(char *out, int *ret_value, struct VARLIST *p_list);

#ifdef __cplusplus
}
#endif

#endif
