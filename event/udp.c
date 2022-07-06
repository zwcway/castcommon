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
#include <errno.h>
#include <string.h>
#include "../log.h"
#include "../common.h"
#include "../connection.h"
#include "../block_queue.h"
#include "udp.h"
#include "event.h"


static pthread_t recv_thread;

static queue_t *block_queue;
static queue_t *recv_queue;

static struct thread_arg_s {
    uint32_t buf_size;
    uint32_t len;
} thread_arg;

LOG_TAG_DECLR("event");

int send_data(const void *data, size_t size) {

}

static int push_recv(void *arg, queue_t *q, void *data, uint32_t size) {
  connection_t *c = *((connection_t **) arg);
  recv_data_t *ud = (recv_data_t *) data;

  ud->conn = c;
  ud->src_len = SOCKADDR_SIZE(c->family);

  LOGT("recvfrom start");
  ssize_t s = recvfrom(c->read_fd,
                       data + RECVDATA_SIZE, size - RECVDATA_SIZE,
                       0,
                       (struct sockaddr *) &ud->src, &ud->src_len);

  if (0 > s) {
    LOGE("recvfrom error: %m");
    if (errno == ECONNREFUSED) {
      LOGW("server(%s:%d) unreachable :%m. quit", sockaddr_ntop(&ud->src), sockaddr_port(&ud->src));
      if (c->refuse_cb) c->refuse_cb();
    }

    return -1;
  }

  c->readed = 1;
  ud->len = s;

  if (0 == s) {
    LOGW("recvfrom received 0");
    return 0;
  }


  return 0;
}

void *thread_cost(void *arg)
{
  connection_t **c;
  queue_ret_t ret;
  struct thread_arg_s *cfg = (struct thread_arg_s *) arg;


  while (!exit_thread_flag) {
    ret = queue_pop(block_queue, (void **) &c, NULL, NULL);
    if (QUEUE_OK == ret) {
      queue_callback_push(recv_queue, push_recv, c, NULL);
    } else {
      LOGE("pop event queue %d", ret);
    }
  }
  pthread_exit(NULL);
}

const queue_t *udp_get_queue()
{
  return recv_queue;
}


queue_t *udp_init(const queue_t *event_queue, uint32_t buf_size, uint32_t qlen) {
  if (NULL == event_queue) {
    return NULL;
  }

  block_queue = (queue_t *) event_queue;

  recv_queue = queue_create("udp main", buf_size + RECVDATA_SIZE, qlen, QUEUE_BLOCK);
  if (NULL == recv_queue) {
    return NULL;
  }

  thread_arg.buf_size = buf_size;
  thread_arg.len = qlen;

  if (0 > pthread_create(&recv_thread, NULL, thread_cost, &thread_arg)) {
    queue_destory(recv_queue);
    LOGE("pthread create error: %m");
    return NULL;
  }
  pthread_detach(recv_thread);

  return recv_queue;
}

int udp_deinit() {
  LOGT("udp deinit");

  if (recv_thread) {
    pthread_cancel(recv_thread);
    recv_thread = 0;
  }

  if (recv_queue) {
    queue_destory(recv_queue);
    recv_queue = 0;
  }

  return 0;
}

protocol_t protocol_udp_ = {
  .init = udp_init,
  .deinit = udp_deinit,

  .send_data = send_data
};