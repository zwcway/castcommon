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


#include <stddef.h>
#include <pthread.h>
#include "../log.h"
#include "../common.h"
#include "../error.h"
#include "receive.h"


static pthread_t cb_thread;

static const queue_t *block_queue;

LOG_TAG_DECLR("event");

static void *thread_cost(void *arg)
{
  recv_data_t *d;
  queue_ret_t ret;
  uint32_t len;

  while (!exit_thread_flag) {
    ret = queue_pop((queue_t *) block_queue, (void **) &d, &len, NULL);
    if (QUEUE_OK == ret && d->conn->read_cb) {
      d->conn->read_cb(d->conn, &d->src, d->src_len, (uint8_t *) d + RECVDATA_SIZE, d->len);
    } else {
      LOGE("pop receive queue %d", ret);
    }
  }
  pthread_exit(NULL);
}

int receive_init(const queue_t *queue) {
  if (NULL == queue) return ERROR_ARG;

  block_queue = queue;

  if (0 > pthread_create(&cb_thread, NULL, thread_cost, NULL)) {
    LOGE("pthread create error: %m");
    return ERROR_THREAD;
  }
  pthread_detach(cb_thread);

  return OK;
}

int receive_deinit()
{
  LOGT("receive deinit");

  pthread_cancel(cb_thread);

  return OK;
}