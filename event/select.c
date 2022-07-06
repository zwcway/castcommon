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

#if WIN32

#include "winsock2.h"

#else
#include <sys/select.h>
#endif

#include "../log.h"
#include "../block_queue.h"
#include "../common.h"
#include <unistd.h>
#include <error.h>
#include "../error.h"
#include "select.h"


static fd_set readfds;
static fd_set writefds;

static connection_t *conns[1024];
static uint32_t conn_cnt = 0;

static queue_t *queue = NULL;
DECL_THREAD_CMD();

static const char *TAG = "event";

int select_add_connection(connection_t *c)
{
  conns[conn_cnt] = (connection_t *) c;
  c->index = conn_cnt;
  conn_cnt++;

  return OK;
}

int select_del_connection(const connection_t *c)
{
  connection_t *tmp;

  if (conns[c->index] != c || conn_cnt <= 0) {
    return OK;
  }

  if (c->index < --conn_cnt) {
    tmp = conns[conn_cnt];
    conns[c->index] = tmp;
    tmp->index = c->index;
  }

  return OK;
}

int select_stop_process()
{
  DEINIT_THREAD_CMD();

  return OK;
}

int select_process() {
  int ready, n, rn;
  socket_t max_fd = -1;
  connection_t *c;
  static struct timeval tv = {0};

  FD_ZERO(&readfds);
  FD_ZERO(&writefds);

  ADD_THREAD_CMD(&readfds);

  for (int i = 0; i < conn_cnt; ++i) {
    c = conns[i];
    FD_SET(c->read_fd, &readfds);

    if (max_fd < c->read_fd) {
      max_fd = c->read_fd;
    }
  }

  tv.tv_sec = 3;
  tv.tv_usec = 0;

#if WIN32
  ready = select(0, &readfds, NULL, NULL, &tv);
#else
  ready = select(max_fd + 1, &readfds, NULL, NULL, &tv);
#endif

  if (ready < 0) {
    LOGE("select error: %m");
    return -1;
  }

  if (ready == 0) {
    LOGT("select timeout");
    return -2;
  }

  n = rn = 0;
  for (int i = 0; i < conn_cnt; ++i) {
    c = conns[i];
    if (FD_ISSET(c->read_fd, &readfds)) {
      if (c->readed) {
        c->readed = 0;
        queue_normal_push(queue, &c, NULL);
      } else {
        rn++;
      }
      n++;
    }
    CHK_CMD_EXIT_THREAD(&readfds);
  }

  if (ready != n) {
    LOGE("select ready != connection: %d != %d", ready, n);
    return -1;
  }
  if (n != rn) {
    usleep(10);
  }

  return OK;
}

const queue_t *select_get_queue()
{
  return queue;
}

queue_t *select_init() {
  conn_cnt = 0;

  if (queue != NULL) return 0;

  INIT_THREAD_CMD("select");

  queue = queue_create("select main", sizeof(connection_t *), SELECT_QUEUE_SZIE, QUEUE_BLOCK);
  if (NULL == queue) {
    return NULL;
  }

  return queue;
}

int select_deinit()
{
  LOGT("select deinit");

  conn_cnt = 0;

  queue_destory(queue);
  select_stop_process();

  return 0;
}


event_t event_select_ = {
  select_init,
  select_deinit,

  select_add_connection,
  select_del_connection,

  select_get_queue,

  select_process,
  select_stop_process,
};