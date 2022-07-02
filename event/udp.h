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


#ifndef UDP_H
#define UDP_H

#include "../ip.h"
#include "event.h"
#include "protocol.h"

extern protocol_t protocol_udp_;

queue_t *udp_init(const queue_t *event_queue, uint32_t buf_size, uint32_t qlen);

int udp_deinit();

const queue_t *udp_get_queue();

#endif //UDP_H
