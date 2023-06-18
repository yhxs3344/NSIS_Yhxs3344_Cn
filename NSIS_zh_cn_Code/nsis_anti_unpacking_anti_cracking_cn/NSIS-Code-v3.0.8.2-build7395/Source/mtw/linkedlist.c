/*
 * linkedlist.c
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

#include "../Platform.h"
#include "mtwcommon.h"
#include "linkedlist.h"

void InitVarList(struct VARLIST *p_list)
{
  p_list->head.next = &p_list->tail;
  p_list->head.prev = NULL;

  p_list->tail.next = NULL;
  p_list->tail.prev = &p_list->head;
}

UINT64 GetListCount(struct VARLIST *p_list)
{
  struct VARITEM *item = p_list->head.next;
  UINT64 count = 0;

  while (item != &p_list->tail)
  {
    count++;
    item = item->next;
  }
  return count;
}

struct VARITEM *FindSeqItem(UINT64 seq_num, struct VARLIST *p_list)
{
  struct VARITEM *count = p_list->head.next;

  while (count != &p_list->tail)
  {
    if (count->seq_num == seq_num) break;
    count = count->next;
  }
  return count == &p_list->tail || !count ? NULL : count;
}

int FindSeqNum(UINT64 seq_num, struct VARLIST *p_list)
{
  return NULL == FindSeqItem(seq_num, p_list) ? 0 : 1;
}

unsigned int GetDataLen(UINT64 seq_num, struct VARLIST *p_list)
{
  struct VARITEM *item = FindSeqItem(seq_num, p_list);
  return item ? item->size : 0;
}

UINT64 GetFirstItemSeqNum(struct VARLIST *p_list)
{
  struct VARITEM *count = p_list->head.next;
  return count != &p_list->tail ? count->seq_num : 0;
}

int AddData(const char *data, unsigned int len, UINT64 seq_num, int ret_value, struct VARLIST *p_list)
{
  struct VARITEM *pThisData;
  unsigned int i;

  if (!data || !len || !p_list) return 0;

  pThisData = (struct VARITEM*)_ALLOC(sizeof(struct VARITEM));
  if (!pThisData)
    return 0;

  pThisData->data = (char*)_ALLOC(len);
  if (!pThisData->data)
    return 0;

  pThisData->seq_num = seq_num;

  for (i = 0; i < len; i++)
    pThisData->data[i] = data[i];

  pThisData->size = len;
  pThisData->ret_value = ret_value;

  pThisData->next = &p_list->tail;
  pThisData->prev = p_list->tail.prev;
  p_list->tail.prev->next = pThisData;
  p_list->tail.prev = pThisData;

  return 1;
}

void RemoveData(struct VARITEM *p_item)
{
  if (p_item)
  {
    if (p_item->data) _FREE(p_item->data);

    p_item->prev->next = p_item->next;
    p_item->next->prev = p_item->prev;

    _FREE(p_item), p_item = NULL;
  }
}

void CleanVarList(struct VARLIST *p_list)
{
  struct VARITEM *count = p_list->head.next;

  if (count == NULL) return;

  while (count != &p_list->tail)
  {
    RemoveData(count);
    count = p_list->head.next;
  }
}

char *GetData(char *out, UINT64 seq_num, int *ret_value, struct VARLIST *p_list)
{
  size_t len, i;
  struct VARITEM *item = FindSeqItem(seq_num, p_list);

  if (!out || !item) return NULL;
  if (ret_value) *ret_value = item->ret_value;
  len = item->size;

  for (i = 0; i < len; i++)
    out[i] = item->data[i];

  RemoveData(item);
  return &out[i];
}

unsigned int GetFirstItemLen(struct VARLIST *p_list)
{
  struct VARITEM *count = p_list->head.next;

  if (count == &p_list->tail) return 0;

  return GetDataLen(count->seq_num, p_list);
}

char *GetFirstItemData(char *out, int *ret_value, struct VARLIST *p_list)
{
  struct VARITEM *count = p_list->head.next;

  if (count == &p_list->tail) return NULL;

  return GetData(out, count->seq_num, ret_value, p_list);
}
