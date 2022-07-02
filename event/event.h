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

#ifndef EVENT_H
#define EVENT_H

#include <stdint-gcc.h>
#include "../connection.h"
#include "../block_queue.h"

enum event_type_e {
    EVENT_TYPE_SELECT
};

enum event_protocol_e {
    EVENT_PROTOCOL_UDP
};

typedef int (*event_add_connection_fn)(connection_t *c);

typedef int (*event_del_connection_fn)(const connection_t *c);

typedef int (*event_start_process_fn)();

typedef int (*event_stop_process_fn)();

typedef const queue_t *(*event_get_queue_fn)();

typedef queue_t *(*event_init_fn)();

typedef int (*event_deinit_fn)();

typedef struct event_s {
    event_init_fn init;
    event_deinit_fn deinit;

    event_add_connection_fn add_connection;
    event_del_connection_fn del_connection;

    event_get_queue_fn get_queue;

    event_start_process_fn start_process;
    event_stop_process_fn stop_process;
} event_t;

typedef struct protocol_s {
    queue_t *(*init)(const queue_t *event_queue, uint32_t buf_size, uint32_t qlen);

    int (*deinit)();

    int (*send_data)(const void *data, size_t size);
} protocol_t;


int event_init(enum event_type_e type, enum event_protocol_e protocol, uint32_t buf_size, uint32_t qlen);

int event_deinit();

int event_start();

int event_stop();

int event_add(connection_t *c);

int event_del(const connection_t *c);

#endif //EVENT_H
