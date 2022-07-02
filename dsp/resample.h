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


#include <stdint-gcc.h>
#include "../audio.h"


#ifndef DSP_RESAMPLE_H
#define DSP_RESAMPLE_H

int upsample_441_480(uint8_t *dst, const uint8_t *src, audio_rate_t from, audio_rate_t to);

#endif //DSP_RESAMPLE_H