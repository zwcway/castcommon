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


#include "crc.h"


uint8_t crc8_check(const uint8_t *buffer, uint32_t length)
{
  uint32_t i;
  uint8_t crc = CRC_INIT;

  while (length--) {
    crc ^= *buffer++;

    for (i = 0; i < 8; i++) {
      if (crc & 0x80) {
        crc <<= 1;
        crc ^= CRC_POLY;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc;
}