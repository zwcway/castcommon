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


typedef enum control_command_e {
    SPCMD_SAMPLE = 1,
    SPCMD_CHUNK,
    /**
     * speaker response the data port
     */
    SPCMD_UNKNOWN_SP = 0xFF,
} control_command_t;

typedef struct control_package_s
{
    control_command_t cmd: 8;
    uint32_t spid;
    union
    {
        struct
        {
            audio_bits_t bits: 8;
            audio_rate_t rate: 8;
            audio_channel_t channel: 8;
            uint8_t reseved: 8;
        } sample;
        struct {
            uint16_t size: 16;
            uint16_t reseved: 16;
        } chunk;
    };
} control_package_t;

typedef struct control_resp_s {
    control_command_t cmd: 8;
    uint32_t spid;
} control_resp_t;


#define CONTROL_PACKAGE_SIZE \
    sizeof(control_package_t)

/**
 *  header size
 */
#define CONTROL_RESP_SIZE     \
    sizeof(control_resp_t)

#define CONTROL_IS_CMD(buf, c)        \
    (((control_resp_t *)(buf))->cmd == (c))


#endif //PACKAGE_CONTROL_H
