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


#ifndef SCREAM_CRC_H
#define SCREAM_CRC_H

#include <stdbool.h>
#include <stdint-gcc.h>


#define CRC_POLY    0x07
#define CRC_INIT    0x00
#define CRC_XOROUT  0x00

uint8_t crc8_check(const uint8_t *buffer, uint32_t length);

#endif //SCREAM_CRC_H
