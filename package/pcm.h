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


#ifndef PACKAGE_PCM_H
#define PACKAGE_PCM_H

#include "../audio.h"


typedef enum header_compress_s
{
    COMPRESS_NONE = 0,
} header_compress_t;

typedef struct header_sample_s {
    audio_rate_t rate: 4;                  /* 采样率 */
    audio_bits_t bits: 4;                  /* 采样位深 */
    audio_channel_mask_t channel_mask;     /* 总声道 */
    audio_channel_t channel: 8;            /* 当前声道 */
} __attribute__((packed)) header_sample_t;

typedef struct channel_header_s {
//    uint8_t crc :8;                         /* CRC 校验*/
    uint8_t ver: 4;                         /* 版本号 */
    header_compress_t compress: 4;          /* 压缩方式 */
    header_sample_t sample;                 /* pcm format */
    uint16_t seq;                           /* 序号 */
    uint16_t len;                           /* 长度 */
}__attribute__((packed)) channel_header_t;

typedef struct channel_resp_t
{
    int32_t maigc;
} channel_resp_t;

#define CHANNEL_HEADER_SIZE (sizeof(channel_header_t))
#define PACKAGE_HEADER_SIZE        (sizeof(channel_header_t))

#endif //PACKAGE_PCM_H
