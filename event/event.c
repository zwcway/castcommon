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

#include <unistd.h>
#include "event.h"
#include "../error.h"
#include "select.h"
#include "udp.h"
#include "receive.h"
#include "../log.h"

event_t *event_ = NULL;
protocol_t *protocol_ = NULL;

LOG_TAG_DECLR("event");

int event_init(enum event_type_e type, enum event_protocol_e protocol, uint32_t buf_size, uint32_t qlen) {
  switch (type) {
    case EVENT_TYPE_SELECT:
      event_ = &event_select_;
      break;
    default:
      return ERROR_ARG;
  }

  switch (protocol) {
    case EVENT_PROTOCOL_UDP:
      protocol_ = &protocol_udp_;
      break;
    default:
      return ERROR_ARG;
  }

  int ret = receive_init(protocol_->init(event_->init(), buf_size, qlen));

  return ret;
}

int event_deinit() {
  receive_deinit();
  protocol_->deinit();
  event_->deinit();

  return OK;
}

int event_start() {
  int ret = 0;
  if (event_) ret = event_->start_process();

  if (ret < 0) {
    sleep(1);
  }
  return OK;
}

int event_stop() {
  if (event_) event_->stop_process();

  return OK;
}

int event_add(connection_t *c) {
  if (NULL == c || !c->read_fd || !c->family) {
    LOGE("connection error");
    return ERROR_ARG;
  }
  if (event_) return event_->add_connection(c);

  return OK;
}

int event_del(const connection_t *c) {
  if (event_) return event_->del_connection(c);

  return OK;
}

