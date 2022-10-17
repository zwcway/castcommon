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


#ifndef PACKAGE_CONTROL_H
#define PACKAGE_CONTROL_H


#include "../audio.h"
#include "../common.h"


typedef enum control_command_e {
    SPCMD_SAMPLE = 1,
    SPCMD_CHUNK,
    SPCMD_TIME,
    /**
     * speaker response the data port
     */
    SPCMD_UNKNOWN_SP = 0x0F,
} control_command_t;

typedef struct control_header_s {
    uint8_t ver;
    control_command_t cmd;
    uint32_t spid;
} control_header_t;

typedef struct control_package_s {
    control_header_t header;
    union {
        struct {
            audio_bits_t bits: 4;
            audio_rate_t rate: 4;
            audio_channel_t channel: 8;
            uint16_t reseved: 16;
        } sample;

        struct {
            uint16_t size: 16;
            uint16_t reseved: 16;
        } chunk;
        struct {
            uint32_t server;
            uint16_t offset;
        } time;
    };
} control_package_t;

typedef struct control_time_s {
    control_header_t header;
    uint64_t time;
} control_time_t;

#define CONTROL_PACKAGE_SIZE  (9)
#define CONTROL_TIME_SIZE  (6)

void control_header_encode(void *pack, const control_header_t *ctl);

void control_header_decode(control_header_t *ctl, const void *pack);

void control_package_encode(void *pack, const control_package_t *ctl);

void control_package_decode(control_package_t *ctl, const void *pack);

void control_time_encode(void *pack, const control_time_t *ctl);

void control_time_decode(control_time_t *ctl, const void *pack);

/**
 *  header size
 */
#define CONTROL_RESP_SIZE     \
    sizeof(control_header_t)

bool control_is_cmd(const void *buf, control_command_t c) {
  control_header_t h;
  control_header_decode(&h, buf);
  return h.cmd == c;
}


#endif //PACKAGE_CONTROL_H
