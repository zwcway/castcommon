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


#ifndef PACKAGE_DETECT_H
#define PACKAGE_DETECT_H


#include "../audio.h"
#include "../speaker_struct.h"

typedef enum spk_detect_connect_e {
    DETECT_SERVER_CONNECTED = 1,
    DETECT_SERVER_DISCONECTED
} spk_detect_connect_t;

/**
 * speaker广播头
 */
typedef struct spk_detect_request_s {
    uint8_t ver: 4;
    spk_detect_connect_t connected: 4;
    addr_t addr;
    speaker_id_t id;
    mac_address_t mac;
    audio_rate_mask_t rate_mask;
    audio_bits_mask_t bits_mask;
    uint16_t data_port;
} spk_detect_request_t;

#define spk_detect_request_size(sf)  ((sf) == AF_INET ? 20 : 32)

/**
 *
 * @param sf
 * @param pack
 * @param req

┌───────┬──────┬────────────┬────────────┬──────────┬──────────┬─────────┬────────────┬────────────┬────────────┐
│ name  │ ver  │ connected  │ addr.type  │ addr.ip  │ id       │ mac     │ rate_mask  │ bits_mask  │ data_port  │
├───────┼──────┼────────────┼────────────┼──────────┼──────────┼─────────┼────────────┼────────────┼────────────┤
│ size  │ 4    │ 1          │ 1          │ 32/128   │ 32       │ 48      │ 16         │ 8          │ 16         │
│ ipv4  │ 0-3  │ 4          │ 5          │ 8-39     │ 40-71    │ 72-119  │ 120-135    │ 136-143    │ 144-159    │
│ ipv6  │ 0-3  │ 4          │ 5          │ 8-135    │ 136-167  │ 168-215 │ 216-231    │ 232-239    │ 240-255    │
└───────┴──────┴────────────┴────────────┴──────────┴──────────┴─────────┴────────────┴────────────┴────────────┘
*/
void spk_detect_request_encode(sa_family_t sf, void *pack, const spk_detect_request_t *req);

void spk_detect_request_decode(sa_family_t sf, spk_detect_request_t *req, const void *pack);

typedef enum detect_type_e {
    DETECT_TYPE_FIRST_RUN = 1,
    DETECT_TYPE_EXIT
} detect_type_t;

/**
 * multicast response from server for speaker
 */
typedef struct spk_detect_response_s {
    uint8_t ver: 4;
    detect_type_t type: 4;
    addr_t addr;
} spk_detect_response_t;

#define spk_detect_response_size(sf)  ((sf) == AF_INET ? 5 : 17)

void spk_detect_response_encode(sa_family_t sf, void *pack, const spk_detect_response_t *res);
void spk_detect_response_decode(sa_family_t sf, spk_detect_response_t *res, const void *pack);

#endif //PACKAGE_DETECT_H
