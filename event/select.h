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


#ifndef SELECT_H
#define SELECT_H

#include <stddef.h>
#include <stdint-gcc.h>
#include "../connection.h"
#include "../block_queue.h"
#include "event.h"


#define SELECT_QUEUE_SZIE 100

queue_t *select_init();

int select_deinit();

int select_add_connection(connection_t *c);

int select_del_connection(const connection_t *c);

int select_process();

int select_stop_process();

const queue_t *select_get_queue();

extern event_t event_select_;

#endif
