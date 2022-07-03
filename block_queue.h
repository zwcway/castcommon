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


#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <stdint.h>
#include <pthread.h>


typedef struct queue_s
{
    char name[16];
    int32_t f;
    int32_t r;
    struct
    {
        uint32_t inited;
        pthread_mutex_t mutex;
        pthread_cond_t push_cond;
        pthread_cond_t pop_cond;
    } lock;

    // data size
    uint32_t size;

    // queue length
    uint32_t len;

    uint32_t flag;
    void *data;
} queue_t;

typedef enum queue_ret_e
{
    QUEUE_TIMEOUT = -3,
    QUEUE_PARAM_ERROR = -2,
    QUEUE_NOT_EXIST = -1,
    QUEUE_OK = 0,
    QUEUE_EMPTY,
    QUEUE_FULL
} queue_ret_t;

typedef enum queue_flag_e
{
    QUEUE_UNBLOCK = 0,
    QUEUE_BLOCK = 1,
    QUEUE_PTR_DATA = 2,
    QUEUE_NEVER_TIMEOUT = 4,
} queue_flag_t;

#define QUEUE_IS_EMPTY(q) \
    ((q)->f == (q)->r)

#define QUEUE_IS_FULL(q) \
    ((q)->f == (q)->r - 1 || (q)->f == (q)->r + (q)->len - 1)

#define queue_create_ptr(length, flag)  \
    queue_create(sizeof(void*), (length), QUEUE_PTR_DATA | (flag))

#define queue_callback_push(q, cb, arg, timeout) \
    queue_push(q, arg, timeout, cb)

#define queue_normal_push(q, data, timeout) \
    queue_push(q, data, timeout, NULL)

typedef int (*push_fn)(void *__restrict arg, queue_t *__restrict q, void *__restrict data, uint32_t size);

queue_t *queue_create(const char *name, uint32_t data_size, uint32_t length, queue_flag_t flag);

queue_ret_t queue_destory(queue_t *q);

queue_ret_t queue_is_full(queue_t *q);

queue_ret_t queue_is_empty(queue_t *q);

queue_ret_t queue_push(queue_t *q, void *__restrict data, struct timespec *timeout, push_fn cb);

queue_ret_t queue_pop(queue_t *q, void **__restrict data, uint32_t *len, struct timespec *timeout);

#endif //BLOCK_QUEUE_H
