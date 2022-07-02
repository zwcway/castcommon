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


#ifndef CASTSPEAKER_H
#define CASTSPEAKER_H

#include <stdint.h>
#include "log.h"
#include "utils.h"


#define SOCKET_QUIT_MAGIC       "quit"
#define SOCKET_QUIT_MAGIC_SIZE  4

#define DECL_THREAD_CMD()         static int wakeup_fd[2] = {0}; static char _buf[SOCKET_QUIT_MAGIC_SIZE] = {0};
#define INIT_THREAD_CMD(name)     if (pipe(wakeup_fd) < 0) { LOGF( name" pipe: %m"); exit(1); }

#define WRITE_EXIT_THREAD_CMD()   if (wakeup_fd[0]) write(wakeup_fd[1], SOCKET_QUIT_MAGIC, SOCKET_QUIT_MAGIC_SIZE)
#define DEINIT_THREAD_CMD()       do{close(wakeup_fd[0]); close(wakeup_fd[1]); wakeup_fd[0]=0; wakeup_fd[1]=0;}while(0)

#define CHK_EXIT_THREAD()           if (exit_thread_flag) return -1
#define CHK_RECV_EXIT_THREAD(buf)    if (exit_thread_flag || (n == SOCKET_QUIT_MAGIC_SIZE && \
memcmp((buf), SOCKET_QUIT_MAGIC, SOCKET_QUIT_MAGIC_SIZE) == 0)) \
goto exit_thread

#define CHK_CMD_EXIT_THREAD()      CHK_EXIT_THREAD(); \
if (FD_ISSET(wakeup_fd[0], &readfds))  \
if (read(wakeup_fd[0], (void *)(_buf), SOCKET_QUIT_MAGIC_SIZE)) \
return -1;

typedef enum run_mode_t
{
    RUN_MODE_SERVER,
    RUN_MODE_SERVER_DAEMON,
    RUN_MODE_CLIENT,
    RUN_MODE_COMMAND,
} run_mode_t;

extern int exit_thread_flag;

#endif
