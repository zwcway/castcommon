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


#ifndef AUDIO_H
#define AUDIO_H

#include <stdint-gcc.h>


#define CHANNEL_DEFAULT (CHANNEL_FRONT_CENTER)

typedef enum audio_bits_e
{
    BIT_NONE = 0,
    BIT_16,
    BIT_20,
    BIT_24,
    BIT_32,
    BIT_32_FLOAT,
    BIT_MAX,
} audio_bits_t;

typedef enum audio_rate_e
{
    RATE_NONE = 0,
    RATE_44100,
    RATE_48000,
    RATE_96000,
    RATE_192000,
    RATE_384000,
    RATE_MAX,
} audio_rate_t;

typedef enum audio_channel_e
{
    CHANNEL_NONE = 0,
    CHANNEL_FRONT_LEFT,
    CHANNEL_FRONT_RIGHT,
    CHANNEL_FRONT_CENTER,
    CHANNEL_LOW_FREQUENCY,
    CHANNEL_BACK_LEFT,
    CHANNEL_BACK_RIGHT,
    CHANNEL_FRONT_LEFT_OF_CENTER,
    CHANNEL_FRONT_RIGHT_OF_CENTER,
    CHANNEL_BACK_CENTER,
    CHANNEL_SIDE_LEFT,
    CHANNEL_SIDE_RIGHT,
    CHANNEL_TOP_CENTER,
    CHANNEL_TOP_FRONT_LEFT,
    CHANNEL_TOP_FRONT_CENTER,
    CHANNEL_TOP_FRONT_RIGHT,
    CHANNEL_TOP_BACK_LEFT,
    CHANNEL_TOP_BACK_CENTER,
    CHANNEL_TOP_BACK_RIGHT,
    CHANNEL_MAX,
} audio_channel_t;


typedef uint16_t audio_rate_mask_t;
typedef uint8_t audio_bits_mask_t;
typedef uint32_t audio_channel_mask_t;

#define RATEMASK_SIZE (16)
#define BITSMASK_SIZE (8)

#define MASK_ARR_PACK(mask, arr, size) do { \
  (mask) = 0;                             \
  for (int i = 0; i < (size); ++i) \
    if ((arr)[i] > 0)        \
      (mask) |= 1 << ((arr)[i]);            \
} while(0)

#define MASK_ISSET(mask, v)   (((mask) & (1 << ((v)))) > 0)

typedef struct channel_list_s {
    uint32_t len;
    audio_channel_t list[CHANNEL_MAX - 1];
} channel_list_t;

typedef struct audio_format_s {
    uint32_t channels;
    uint32_t samples_per_sec;
    uint32_t bits_per_sample;
} audio_format_t;


int bits_name(audio_bits_t bits);

int rate_name(audio_rate_t rate);

char *channel_name(audio_channel_t channel);

#endif //AUDIO_H
