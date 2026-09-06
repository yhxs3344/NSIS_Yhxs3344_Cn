/*
 * mtwjob.c
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

#include "../Platform.h"
#include "mtwcommon.h"
#include "mtwthreads.h"
#include "mtwjob.h"

struct JOBITEM
{
  struct JOBITEM *next;
  struct JOBITEM *prev;

  char *data;          /* next byte */
  unsigned int size;   /* number of bytes available at 'data' */
  UINT64 seq_num;      /* where in the sequence this block belongs */
  int ret_value;       /* codec return value */
};

struct JOBLIST
{
  struct JOBITEM head;
  struct JOBITEM tail;
  MT_MUTEX_T mutex;
};

struct JOBLIST job_space;

void InitJobList(struct JOBLIST *p_list)
{
  p_list->head.next = &p_list->tail;
  p_list->head.prev = NULL;

  p_list->tail.next = NULL;
  p_list->tail.prev = &p_list->head;
}

unsigned int GetListCount(struct JOBLIST *p_list)
{
  struct JOBITEM *item = p_list->head.next;
  unsigned int count = 0;

  while (item != &p_list->tail)
  {
    count++;
    item = item->next;
  }
  return count;
}

struct JOBITEM *FindSeqItem(UINT64 seq_num, struct JOBLIST *p_list)
{
  struct JOBITEM *item = p_list->head.next;

  while (item != &p_list->tail)
  {
    if (item->seq_num == seq_num)
      break;
    item = item->next;
  }
  return (item == &p_list->tail || !item) ? NULL : item;
}

int AddJobData(const char *data, unsigned int len, UINT64 seq_num, int ret_value, struct JOBLIST *p_list)
{
  struct JOBITEM *pThisData;
  unsigned int i;

  if (!data || !len || !p_list) return 0;

  pThisData = (struct JOBITEM*)_ALLOC(sizeof(struct JOBITEM));
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

void RemoveJobData(struct JOBITEM *p_item)
{
  if (p_item)
  {
    if (p_item->data) _FREE(p_item->data);

    p_item->prev->next = p_item->next;
    p_item->next->prev = p_item->prev;

    _FREE(p_item), p_item = NULL;
  }
}

void CleanJobList(struct JOBLIST *p_list)
{
  struct JOBITEM *count = p_list->head.next;

  if (count == NULL) return;

  while (count != &p_list->tail)
  {
    RemoveJobData(count);
    count = p_list->head.next;
  }
}

char *GetData(char *out, UINT64 seq_num, int *ret_value, struct JOBLIST *p_list)
{
  size_t len, i;
  struct JOBITEM *item = FindSeqItem(seq_num, p_list);

  if (!out || !item) return NULL;
  if (ret_value) *ret_value = item->ret_value;
  len = item->size;

  for (i = 0; i < len; i++)
    out[i] = item->data[i];

  RemoveJobData(item);
  return &out[i];
}

int InitJobContext(void)
{
  InitJobList(&job_space);
  return !MT_MUTEX_INIT(&job_space.mutex, NULL); /* mutex init returns 1 on error */
}

void FreeJobContext(void)
{
  CleanJobList(&job_space);
  MT_MUTEX_DESTROY(&job_space.mutex);
}

// Returns the number of blocks that are not empty.
unsigned int JobCountNextOut(void)
{
  unsigned int count;
  MT_MUTEX_LOCK(&job_space.mutex);
  count = GetListCount(&job_space);
  MT_MUTEX_UNLOCK(&job_space.mutex);
  return count;
}

// Returns true if the next sequence number block is ready.
int JobHaveNextOut(UINT64 seq_num)
{
  int count;
  MT_MUTEX_LOCK(&job_space.mutex);
  count = !!FindSeqItem(seq_num, &job_space);
  MT_MUTEX_UNLOCK(&job_space.mutex);
  return count;
}

PMTWJOB MakeJob(const char* data, unsigned int in_size, unsigned int out_size, UINT64 seq_num, void* optional)
{
  unsigned int i = 0;
  PMTWJOB job = _ALLOC(sizeof(MTWJOB));
  if (!job) return NULL;

  job->input = _ALLOC(in_size);
  job->output = _ALLOC(out_size);
  if (!job->input || !job->output) return NULL;

  for (i = 0; i < in_size; i++)
    job->input[i] = data[i];
  job->input_size = in_size;
  job->output_size = out_size;
  job->seq_num = seq_num;
  job->optional = optional;

  return job;
}

int FinishJob(const PMTWJOB job)
{
  int ret;
  MT_MUTEX_LOCK(&job_space.mutex);
  ret = AddJobData(job->output, job->output_size, job->seq_num, job->ret_value, &job_space);
  _FREE(job->input);
  _FREE(job->output);
  MT_MUTEX_UNLOCK(&job_space.mutex);
  return ret;
}

// Gets data from the output queue. Returns used position in the buffer.
char *JobGetNextOut(char *out, UINT64 seq_num, int *ret_value)
{
  char *out_end = NULL;
  struct JOBITEM *chunk;

  if (!out) return NULL;
  MT_MUTEX_LOCK(&job_space.mutex);
  chunk = FindSeqItem(seq_num, &job_space);
  if (chunk == NULL)
  {
    MT_MUTEX_UNLOCK(&job_space.mutex);
    return 0;
  }
  out_end = GetData(out, seq_num, ret_value, &job_space);
  MT_MUTEX_UNLOCK(&job_space.mutex);
  return out_end;
}
