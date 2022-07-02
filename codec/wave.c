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
#include <stdio.h>
#include "wave.h"
#include "../utils.h"
#include "../log.h"


static struct wave_riff_t *riff = NULL;
static struct wave_fmt_t *fmt = NULL;
static struct wave_format_t *format = NULL;
static struct wave_data_t *data = NULL;


static channel_list_t channel_list = {0};

LOG_TAG_DECLR("wave");

void check_format(struct wave_format_t *f)
{
//  printf("format_tag: 0x%04X\n", f->format_tag);
  LOGI("channels: %d", f->channels);
  LOGI("samples_per_sec: %d", f->samples_per_sec);
  LOGI("avg_bytes_per_sec: %d", f->avg_bytes_per_sec);
//  printf("block_align: %d\n", f->block_align);
  LOGI("bits_per_sample: %d", f->bits_per_sample);
//  printf("cbSize: %d\n", f->cbSize);
}

void check_extensible(struct wave_format_extensible_t *f)
{
  check_format(&f->format);
//  printf("valid_bits_per_sample: %d\n", f->valid_bits_per_sample);
//    printf("samples_per_block: %d\n", f->samples_per_block);
//  printf("channel_mask: 0x%04X\n", f->channel_mask);
//  printf("sub_format: 0x%04X %04X %04X %04X\n", f->sub_format[0], f->sub_format[1], f->sub_format[2], f->sub_format[3]);
}

uint32_t header_check(const uint8_t *wave, const size_t length)
{
  uint8_t *ptr = (uint8_t *) wave;

  riff = NULL;
  fmt = NULL;
  format = NULL;
  data = NULL;

  riff = (struct wave_riff_t *) ptr;
  ptr += sizeof(struct wave_riff_t);
  if (strncasecmp(riff->riff_header, "RIFF", 4) != 0) {
    LOGW("invalid value of riff_header %s", riff->riff_header);
    return 0;
  }
  LOGI("wave_size: %d", riff->wav_size);
  if (strncasecmp(riff->wave_header, "WAVE", 4) != 0) {
    LOGW("invalid value of  wave_header %s", riff->wave_header);
    return 0;
  }

  fmt = (struct wave_fmt_t *) ptr;
  ptr += sizeof(struct wave_fmt_t);
  if (strncasecmp(fmt->fmt_header, "fmt ", 4) != 0) {
    LOGW("invalid value of  fmt_header %s", fmt->fmt_header);
    return 0;
  }
  format = (struct wave_format_t *) ptr;
  uint32_t channel_mask = 0;
  if (fmt->fmt_chunk_size == sizeof(struct wave_format_extensible_t)) {
    check_extensible((struct wave_format_extensible_t *) ptr);
    channel_mask = ((struct wave_format_extensible_t *) ptr)->channel_mask;
    ptr += sizeof(struct wave_format_extensible_t);
  } else if (fmt->fmt_chunk_size == sizeof(struct wave_format_t)) {
    if (format->channels == 2) {
      channel_mask = 0x03; // CHANNEL_FRONT_LEFT | CHANNEL_FRONT_RIGHT
    } else if (format->channels == 1) {
      channel_mask = 0x04; // CHANNEL_FRONT_CENTER
    }
  } else {
    LOGW("invalid fmt_chunk_size %d", fmt->fmt_chunk_size);
    return 0;
  }

  channel_list.len = 0;
  for (int i = 0; i < 32; ++i) {
    if (((channel_mask >> i) & 0x01) > 0) {
      channel_list.list[channel_list.len++] = (audio_channel_t) (i + 1);
    }
  }

  do {
    data = (struct wave_data_t *) ptr;
    if (strncasecmp(data->data_header, "data", 4) != 0) {
      ptr++;
      continue;
    }
    if (data->data_bytes < 0 || data->data_bytes >= riff->wav_size) {
      ptr++;
      continue;
    }
    if (ptr >= wave + length) {
      LOGW("can not found wave data section");
      return 0;
    }
    break;
  } while (ptr < wave + length);
  ptr += sizeof(struct wave_data_t);

  LOGI("data_size: %d", data->data_bytes);
  LOGI("header_size: %d", (uint32_t) (ptr - wave));
  return ptr - wave;
}

channel_list_t *get_channel_list()
{
  if (riff == NULL || fmt == NULL || format == NULL || data == NULL) {
    return 0;
  }

  return &channel_list;
}

int get_format(audio_format_t *f)
{
  if (riff == NULL || fmt == NULL || format == NULL || data == NULL || f == NULL) {
    return 1;
  }

  f->channels = format->channels;
  f->bits_per_sample = format->bits_per_sample;
  f->samples_per_sec = format->samples_per_sec;

  return 0;
}

uint32_t get_bytes_per_sample()
{
  if (riff == NULL || fmt == NULL || format == NULL || data == NULL) {
    return 0;
  }

  return format->bits_per_sample / 8;
}

uint32_t get_chunksize(wave_chunk_t *c, chunk_type_t type) {
  uint32_t chunk, time;
  uint32_t bytes_per_sec, d;
  if (riff == NULL || fmt == NULL || format == NULL || data == NULL) {
    return 0;
  }

  bytes_per_sec = format->samples_per_sec * get_bytes_per_sample();

  chunk = samples_chunk(bytes_per_sec, type);
  time = chunk * 1000000 / bytes_per_sec;
  chunk *= format->channels;

  if (c) {
    c->chunk = chunk;
    c->time = time;
  }

  LOGI("wave chunk size %d, time: %dus", chunk, time);
  return chunk;
}

uint32_t get_filesize()
{
  uint32_t size;
  if (riff == NULL || fmt == NULL || format == NULL || data == NULL) {
    return 0;
  }
  return riff->wav_size;
}

uint32_t get_datasize()
{
  uint32_t size;
  if (riff == NULL || fmt == NULL || format == NULL || data == NULL) {
    return 0;
  }
  return data->data_bytes;
}

uint32_t get_duration()
{
  uint32_t dur;
  if (riff == NULL || fmt == NULL || format == NULL || data == NULL) {
    return 0;
  }

  dur = (uint32_t) (data->data_bytes / format->avg_bytes_per_sec);
  LOGI("wave duration: %02d:%02d:%02d", dur / 3600, (dur % 3600) / 60, (dur % 60));

  return dur;
}

int parse_sample(const uint8_t *samples, const size_t length)
{
}