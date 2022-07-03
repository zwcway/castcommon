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


#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include "speaker_struct.h"


#define FLOAT_IS_NUMBER(i) ((i) - (int)(i) == 0)

typedef enum chunk_type_t
{
    CHUNK_QUALITY = 1,  // more delay
    CHUNK_SPEED,        // may drop some data
} chunk_type_t;

inline static void *xmalloc(size_t len)
{
  void *a = malloc(len);
  if (a == NULL) perror("malloc error"), exit(255);
  return a;
}

uint32_t samples_chunk(uint32_t per_sec, chunk_type_t type);

int32_t is_uint32(const char *str);

#endif
