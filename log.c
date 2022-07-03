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


#include <string.h>
#include <stdlib.h>
#include "log.h"

#if WIN32

#include <windows.h>

#endif

#define MAX_CALLBACKS 32

#define MAX_FILTERS 32

const char *LOG_DEFAULT_NAME = "default";

static char time_buf[64];

struct Filter
{
    char name[16];
    int level;
};

typedef struct
{
    log_LogFn fn;
    void *udata;
    int level;
} Callback;

static struct
{
    void *udata;
    log_LockFn lock;
    int level;
    bool quiet;
    struct Filter filter[MAX_FILTERS];
    int filter_len;
    Callback callbacks[MAX_CALLBACKS];
} L;


static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif


 void stdout_callback(log_Event *ev)
 {
   time_buf[strftime(time_buf, sizeof(time_buf), "%H:%M:%S", ev->time)] = '\0';
#ifdef LOG_USE_COLOR
   fprintf(
     ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
     buf, level_colors[ev->level], level_strings[ev->level],
     ev->file, ev->line);
#else
   fprintf(
     ev->udata, "%s %-5s %s:%d %s: ",
    time_buf, level_strings[ev->level], ev->file, ev->line, ev->tag == LOG_DEFAULT_NAME ? "" : ev->tag);
#endif
  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}


static void file_callback(log_Event *ev)
{
  time_buf[strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
  fprintf(
    ev->udata, "%s %-5s %s:%d %s: ",
    time_buf, level_strings[ev->level], ev->file, ev->line, ev->tag == LOG_DEFAULT_NAME ? "" : ev->tag);
  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}


static void lock(void)
{
  if (L.lock) { L.lock(true, L.udata); }
}


static void unlock(void)
{
  if (L.lock) { L.lock(false, L.udata); }
}


const char *log_level_string(int level)
{
  return level_strings[level];
}


void log_set_lock(log_LockFn fn, void *udata)
{
  L.lock = fn;
  L.udata = udata;
}


void log_set_level(int level)
{
  L.level = level;
}

void log_add_filter(const char *name, int level)
{
  if (L.filter_len >= MAX_FILTERS) return;

  strncpy(L.filter[L.filter_len].name, name, 15);
  L.filter[L.filter_len].level = level;

  L.filter_len++;
}

int parse_level_string(const char *s)
{
  int loglevel = -1;
  char *end;

  for (int i = 0; i < 6; ++i) {
    if (strcasecmp(s, level_strings[i]) == 0)
      return LOG_TRACE;
  }

  loglevel = (int) strtol(s, &end, 10);
  if (end == s) return -1;
  if (loglevel >= 0) return loglevel;

  return -1;
}

/**
 * example:
 * 1.  level
 * 2.  name:level[,...]
 *
 * @param level
 * @return
 */
int log_set_level_from_string(const char *level)
{
  if (level == NULL) return -1;

  int loglevel = LOG_INFO, f = 0, i = 0;
  char *str = strdup(level);
  char *s = str, *t, *k;

  do {
    t = strchr(s, ',');
    if (NULL != t) *t = '\0';

    if (t == s) return -1;

    k = strchr(s, ':');
    if (k == NULL || s == k) {
      if ((loglevel = parse_level_string(s)) < 0) return -1;

      log_set_level(loglevel);
      return 0;
    }

    *k = '\0';
    strcpy(L.filter[f].name, s);
    s = k + 1;

    if ((loglevel = parse_level_string(s)) < 0) return -1;

    L.filter[f].level = loglevel;

    f++;

    if (NULL == t) break;
    s = t + 1;
  } while (*s != '\0' && f < MAX_FILTERS);

  free(str);

  return 0;
}


void log_set_quiet(bool enable)
{
  L.quiet = enable;
}


int log_add_callback(log_LogFn fn, void *udata, int level)
{
  for (int i = 0; i < MAX_CALLBACKS; i++) {
    if (!L.callbacks[i].fn) {
      L.callbacks[i] = (Callback) {fn, udata, level};
      return 0;
    }
  }
  return -1;
}


int log_add_fp(FILE *fp, int level)
{
  return log_add_callback(file_callback, fp, level);
}


static void init_event(log_Event *ev, void *udata) {
  if (!ev->time) {
    time_t t = time(NULL);
    ev->time = localtime(&t);
  }
  ev->udata = udata;

#if WIN32
  char *s;
  char *fmt = NULL;
  int offset, flen, len;
  if (NULL != (s = strstr(ev->fmt, "%m"))) {
    if (s == ev->fmt || *(s - 1) != '%') {
      // 替换 %m 为错误信息，多语言。
      offset = (int) (s - ev->fmt);
      wchar_t *err = NULL;
      FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, WSAGetLastError(),
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPWSTR) &err, 0, NULL);
      len = WideCharToMultiByte(CP_ACP, 0, err, -1, NULL, 0, NULL, NULL);
      flen = (int) strlen(ev->fmt) - 2 + len;

      // free 在 log_log 函数尾
      fmt = malloc(flen + 1);

      memset(fmt, 0, len + 1);
      strncpy(fmt, ev->fmt, offset);

      WideCharToMultiByte(CP_ACP, 0, err, -1, fmt + offset, len, NULL, NULL);

      // 强制不换行
      if (NULL != (s = strchr(fmt, '\r'))) *s = ' ';
      if (NULL != (s = strchr(fmt, '\n'))) *s = ' ';

      strcat(fmt, ev->fmt + offset + 2);
      LocalFree(err);
      ev->fmt = fmt;
    }
  }
#endif
}


static const char *simple_file(const char *file)
{
  char *pos = (char *) file, *s = (char *) file;
  while (*s != '\0') {
    if (*s == '/') pos = s + 1;
    s++;
  }
  return pos;
}

static int filter_log(const char *name, int level)
{
  if (L.quiet) return 0;

  for (int i = 0; i < L.filter_len; ++i) {
    if (strcasecmp(name, L.filter[i].name) == 0) {
      return level >= L.filter[i].level;
    }
  }

  if (level >= L.level) return 1;

  return 0;
}

void log_log(const char *tag, int level, const char *file, int line, const char *fmt, ...)
{
  log_Event ev = {
    .fmt   = fmt,
    .file  = simple_file(file),
    .line  = line,
    .level = level,
    .tag = tag,
  };

  lock();

  if (filter_log(tag, level)) {
    init_event(&ev, stderr);
    va_start(ev.ap, fmt);
    stdout_callback(&ev);
    va_end(ev.ap);
  }

  for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
    Callback *cb = &L.callbacks[i];
    if (level >= cb->level) {
      init_event(&ev, cb->udata);
      va_start(ev.ap, fmt);
      cb->fn(&ev);
      va_end(ev.ap);
    }
  }

  if (ev.fmt != fmt) {
    free((char *) ev.fmt);
  }
  unlock();
}