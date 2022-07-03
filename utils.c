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
#include <unistd.h>
#include <math.h>
#include "utils.h"
#include "speaker_struct.h"
#include "log.h"


LOG_TAG_DECLR("common");

static uint32_t chunk_quality(uint32_t per_sec) {
  double ms = per_sec / 1000.0;
  uint32_t d = 1;

  for (int i = 1; i <= 10; ++i) {
    if (FLOAT_IS_NUMBER(ms * i)) {
      if (ms * i > 1500) {
        break;
      }
      d = i;
    }
  }

  return (uint32_t) (ms * d);
}

static uint32_t chunk_speed(uint32_t per_src) {
  return per_src / 1000;
}

uint32_t samples_chunk(uint32_t per_sec, chunk_type_t type) {
  if (type == CHUNK_QUALITY) {
    return chunk_quality(per_sec);
  } else if (type == CHUNK_SPEED) {
    return chunk_speed(per_sec);
  }

  return 0;
}

int32_t is_uint32(const char *str) {
  int i = (int) strlen(str), n = 0;
  if (i <= 0) return -1;

  while (*str != '\0') {
    if (*str >= '0' && *str <= '9') {
      n += pow(10, --i) * (*str - '0');
    } else {
      return -1;
    }
    str++;
  }

  return n;
}