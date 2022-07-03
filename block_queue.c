/**
    This file is part of castspeaker
    Copyright (C) 2022-2028  zwcway

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <malloc.h>
#include <pthread.h>
#include <memory.h>
#include <sys/time.h>
#include <string.h>
#include "block_queue.h"
#include "log.h"


LOG_TAG_DECLR("queue");

queue_t *queue_create(const char *name, uint32_t data_size, uint32_t length, queue_flag_t flag)
{
  queue_t *q;

  if (data_size * length == 0) {
    LOGE("param error");
    return NULL;
  }

  q = malloc(sizeof(queue_t));
  if (NULL == q) {
    LOGE("malloc error: %m");
    return NULL;
  }

  q->f = q->r = 0;
  q->len = length;
  q->flag = flag;
  q->size = data_size;
  q->data = calloc(data_size, length);

  q->lock.inited = 0;

  if (NULL == q->data) {
    LOGE("calloc error: %m");
    free(q);
    return NULL;
  }

  if (name != NULL) strcpy(q->name, name);
  else strcpy(q->name, "default queue");

  return q;
}

queue_ret_t queue_destory(queue_t *q) {
  if (NULL == q) return QUEUE_NOT_EXIST;

  if (!q->data || !q->lock.inited) return QUEUE_OK;

  q->lock.inited = 0;

  pthread_mutex_destroy(&q->lock.mutex);

  pthread_cond_destroy(&q->lock.push_cond);
  pthread_cond_destroy(&q->lock.pop_cond);

  if (NULL != q->data) {
    free(q->data);
    q->data = 0;
  }
  free(q);

  return QUEUE_OK;
}

int queue_lock_init(queue_t *q)
{
  if (q->lock.inited) return 0;

  if (pthread_cond_init(&q->lock.push_cond, NULL)) {
    LOGE("thread push condition create error: %m");
    return -1;
  }
  if (pthread_cond_init(&q->lock.pop_cond, NULL)) {
    LOGE("thread pop condition create error: %m");
    return -1;
  }

  if (pthread_mutex_init(&q->lock.mutex, NULL)) {
    LOGE("thread lock.mutex create error: %m");
    return -1;
  }

  q->lock.inited = 1;

  return 0;
}

queue_ret_t queue_is_full(queue_t *q)
{
  if (NULL == q) return QUEUE_NOT_EXIST;

  if (q->f == q->r - 1) return QUEUE_FULL;

  if (q->f == q->r + q->len - 1) return QUEUE_FULL;

  return QUEUE_OK;
}

queue_ret_t queue_is_empty(queue_t *q)
{
  if (NULL == q) return QUEUE_NOT_EXIST;

  if (q->f == q->r) return QUEUE_EMPTY;

  return QUEUE_OK;
}

uint32_t queue_len(queue_t *q)
{
  return q->f >= q->r ? (q->f - q->r) : q->len - 1 - (q->r - q->f);
}


queue_ret_t queue_push(queue_t *q, void *data, struct timespec *t, push_fn cb)
{
  if (NULL == q) return QUEUE_NOT_EXIST;

  queue_lock_init(q);
  pthread_mutex_lock(&q->lock.mutex);

  if (QUEUE_IS_FULL(q)) {
    LOGD("queue(%s) is full, block it", q->name);

    if ((q->flag & QUEUE_BLOCK) == 0) {
      pthread_mutex_unlock(&q->lock.mutex);
      return QUEUE_FULL;
    }

    if (NULL == t) {

      LOGD("queue(%s) infinite blocking", q->name);

      while (QUEUE_IS_FULL(q)) {
        pthread_cond_wait(&q->lock.push_cond, &q->lock.mutex);
      }

    } else {
      struct timespec tsp;
      struct timeval now;
      gettimeofday(&now, NULL);
      tsp.tv_sec = now.tv_sec + t->tv_sec;
      tsp.tv_nsec = now.tv_usec * 1000 + t->tv_nsec;

      LOGD("queue(%s) block %d sec", q->name, t->tv_sec);

      if (0 != pthread_cond_timedwait(&q->lock.push_cond, &q->lock.mutex, &tsp)) {
        LOGD("queue(%s) block timeout", q->name);
        pthread_mutex_unlock(&q->lock.mutex);
        return QUEUE_TIMEOUT;
      }
    }

  } // end if (QUEUE_IS_FULL(q))

  if (cb == NULL) {
    memcpy((q->data + q->f * q->size), data, q->size);
  } else if (cb(data, q, q->data + q->f * q->size, q->size) < 0) {
    LOGE("queue(%s) pushed cb %d fail, length %d. cancel.", q->name, q->size, queue_len(q));
    pthread_mutex_unlock(&q->lock.mutex);
    return QUEUE_OK;
  }

  if (++q->f >= q->len) {
    q->f = 0;
  }

  LOGD("queue(%s) pushed %d sucess, length %d", q->name, q->size, queue_len(q));

  pthread_cond_signal(&q->lock.pop_cond);
  pthread_mutex_unlock(&q->lock.mutex);

  return QUEUE_OK;
}

queue_ret_t queue_pop(queue_t *q, void **data, uint32_t *len, struct timespec *timeout)
{
  if (NULL == q) {
    LOGW("queue not exists while pop");
    return QUEUE_NOT_EXIST;
  }
  if (NULL == data) {
    LOGW("queue(%s) pop data error", q->name);
    return QUEUE_PARAM_ERROR;
  }

  queue_lock_init(q);

  pthread_mutex_lock(&q->lock.mutex);

  if (QUEUE_IS_EMPTY(q)) {
    LOGD("queue(%s) is empty, block it", q->name);

    if ((q->flag & QUEUE_BLOCK) == 0) {
      pthread_mutex_unlock(&q->lock.mutex);
      return QUEUE_EMPTY;
    }

    if (NULL == timeout) {
      LOGD("queue(%s) infinite blocking", q->name);
      while (QUEUE_IS_EMPTY(q)) {
        pthread_cond_wait(&q->lock.pop_cond, &q->lock.mutex);
      }
    } else {
      struct timespec tsp;
      struct timeval now;
      gettimeofday(&now, NULL);
      tsp.tv_sec = now.tv_sec + timeout->tv_sec;
      tsp.tv_nsec = now.tv_usec * 1000 + timeout->tv_nsec;

      LOGD("queue(%s) block %d sec", q->name, timeout->tv_sec);

      if (0 != pthread_cond_timedwait(&q->lock.pop_cond, &q->lock.mutex, &tsp)) {
        LOGD("queue(%s) block timeout", q->name);
        pthread_mutex_unlock(&q->lock.mutex);
        return QUEUE_TIMEOUT;
      }
    }

  } // end if (QUEUE_IS_FULL(q))

  *data = ((void **) (q->data + q->r * q->size));

  if (++q->r >= q->len) {
    q->r = 0;
  }
  if (NULL != len) *len = q->size;

  LOGD("queue(%s) poped %d sucess, queue length %d", q->name, q->size, queue_len(q));

  pthread_cond_signal(&q->lock.push_cond);
  pthread_mutex_unlock(&q->lock.mutex);

  return QUEUE_OK;
}