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


#ifndef WAVE_H
#define WAVE_H

#include <stdint-gcc.h>
#include "../audio.h"
#include "../utils.h"


typedef struct wave_format_t {
    uint16_t format_tag;
    uint16_t channels;
    uint32_t samples_per_sec;
    uint32_t avg_bytes_per_sec;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint16_t cbSize;
} __attribute__((packed)) wave_format_t;

struct wave_format_extensible_t
{
    wave_format_t format;
    uint16_t valid_bits_per_sample;
    uint32_t channel_mask;
    uint32_t sub_format[4];
};

struct wave_riff_t
{
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    uint32_t wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"
};

struct wave_fmt_t
{
    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    uint32_t fmt_chunk_size;
};

struct wave_fact_t
{
    char fact_header[4]; // Contains "fact"
    uint32_t fact_bytes;
};

typedef struct wave_data_t
{
    char data_header[4]; // Contains "data"
    uint32_t data_bytes;
} wave_data_t;

typedef struct wave_chunk_t
{
    uint32_t chunk;
    uint32_t time;
} wave_chunk_t;

uint32_t header_check(const uint8_t *wave, size_t length);

uint32_t get_filesize();

uint32_t get_datasize();

uint32_t get_duration();

uint32_t get_chunksize(wave_chunk_t *c, chunk_type_t type);

uint32_t get_bytes_per_sample();

int get_format(audio_format_t *format);

channel_list_t *get_channel_list();

#endif //WAVE_H
