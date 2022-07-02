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


#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint-gcc.h>
#include <sys/socket.h>


typedef struct connection_s connection_t;

typedef int (*read_fn)(connection_t *__restrict c,
                       const struct sockaddr_storage *__restrict src,
                       socklen_t src_len,
                       const void *__restrict data,
                       uint32_t len);

typedef int (*write_fn)();

typedef void (*recv_refuse)();

#define DEFAULT_CONNECTION_UDP_INIT {0, 0, 0, 1, NULL, NULL};


struct connection_s
{
    int read_fd;
    uint32_t index;
    unsigned tcp;
    unsigned readed;
    read_fn read_cb;
    recv_refuse refuse_cb;
};

#endif //CONNECTION_H
