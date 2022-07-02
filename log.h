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


#ifndef LOG_H
#define LOG_H


#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>


#define LOG_VERSION "0.1.0"

extern const char *LOG_DEFAULT_NAME;

typedef struct
{
    va_list ap;
    const char *fmt;
    const char *file;
    struct tm *time;
    const char *tag;
    void *udata;
    int line;
    int level;
} log_Event;

typedef void (*log_LogFn)(log_Event *ev);

typedef void (*log_LockFn)(bool lock, void *udata);

enum
{
    LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL
};

#define log_trace(...) log_log(LOG_DEFAULT_NAME, LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEFAULT_NAME, LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_DEFAULT_NAME, LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_DEFAULT_NAME, LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_DEFAULT_NAME, LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_DEFAULT_NAME, LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define logt(name, ...) log_log(name, LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define logd(name, ...) log_log(name, LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define logi(name, ...)  log_log(name, LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define logw(name, ...)  log_log(name, LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define loge(name, ...) log_log(name, LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define logf(name, ...) log_log(name, LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)


#define LOG_TAG_DECLR(name)  static const char *TAG = name

#define LOGT(...) log_log(TAG, LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOGD(...) log_log(TAG, LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOGI(...)  log_log(TAG, LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOGW(...)  log_log(TAG, LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOGE(...) log_log(TAG, LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOGF(...) log_log(TAG, LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

const char *log_level_string(int level);

void log_set_lock(log_LockFn fn, void *udata);

void log_set_level(int level);

int log_set_level_from_string(const char *level);

void log_set_quiet(bool enable);

int log_add_callback(log_LogFn fn, void *udata, int level);

int log_add_fp(FILE *fp, int level);

void log_add_filter(const char *name, int level);

void log_log(const char *tag, int level, const char *file, int line, const char *fmt, ...);

#endif