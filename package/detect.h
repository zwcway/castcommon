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

typedef enum detect_connect_e {
    DETECT_SERVER_CONNECTED = 1,
    DETECT_SERVER_DISCONECTED
} detect_connect_t;

/**
 * speaker广播头
 */
typedef struct detect_request_s {
    uint8_t ver: 4;
    detect_connect_t connected: 4;
    addr_t addr;
    speaker_id_t id;
    mac_address_t mac;
    audio_rate_mask_t rate_mask;
    audio_bits_mask_t bits_mask;
    uint16_t data_port;
} detect_request_t;

#if PACKAGE_PACKED
#define DETECT_PACKAGE_SIZE(sf)  ((sf) == AF_INET ? 20 : 32)

// ┌───────┬──────┬────────────┬────────────┬──────────┬──────────┬─────────┬────────────┬────────────┬────────────┐
// │ name  │ ver  │ connected  │ addr.type  │ addr.ip  │ id       │ mac     │ rate_mask  │ bits_mask  │ data_port  │
// ├───────┼──────┼────────────┼────────────┼──────────┼──────────┼─────────┼────────────┼────────────┼────────────┤
// │ size  │ 4    │ 1          │ 1          │ 32/128   │ 32       │ 48      │ 16         │ 8          │ 16         │
// │ ipv4  │ 0-3  │ 4          │ 5          │ 8-39     │ 40-71    │ 72-119  │ 120-135    │ 136-143    │ 144-159    │
// │ ipv6  │ 0-3  │ 4          │ 5          │ 8-135    │ 136-167  │ 168-215 │ 216-231    │ 232-239    │ 240-255    │
// └───────┴──────┴────────────┴────────────┴──────────┴──────────┴─────────┴────────────┴────────────┴────────────┘

#define DETECT_PACKAGE_ENCODE(sf, d, p) do { \
    uint8_t *ptr = (uint8_t *)(d);           \
    \
    *((uint8_t*)(ptr)) = (((p)->ver & 0x0F) << 4) | ((!!(p)->connected) << 3) | (((p)->addr.type == AF_INET6) << 2); \
    ptr += 1;                                \
    \
    memcpy(ptr, &(p)->addr.ipv6, (sf) == AF_INET ? 4 : 16);\
    ptr += (sf) == AF_INET ? 4 : 16;         \
    \
    (uint32_t*)ptr[0] = (p)->id;             \
    ptr += 4;                                \
    \
    memcpy(ptr, &(p)->mac.mac, 6);           \
    ptr += 6;                                \
    \
    (uint16_t *)ptr[0] = (p)->rate_mask;     \
    ptr += 2;                                \
    \
    ptr[0] = (p)->bits_mask;                 \
    ptr += 1;                                \
    \
    (uint16_t*)ptr[0] = (p)->data_port;      \
    ptr += 2;                                \
    } while(0)

#define DETECT_PACKAGE_DECODE(sf, p, d) do { \
    uint8_t *ptr = (uint8_t *)(d);           \
    \
    (p)->ver = (ptr[0] >> 4) & 0x0F;         \
    (p)->connected = (ptr[0] >> 3) & 0x01;   \
    (p)->addr.type = ((ptr[0] >> 2) & 0x01) ? AF_INET6 : AF_INET;   \
    ptr += 1;                                \
    \
    memcpy(&(p)->addr.ipv6, ptr, (sf) == AF_INET ? 4 : 16);\
    ptr += (sf) == AF_INET ? 4 : 16;         \
    \
    (p)->id = (uint16_t*)ptr[0];             \
    ptr += 4;                                \
    \
    memcpy(&(p)->mac.mac, ptr, 6);           \
    ptr += 6;                                \
    \
    (p)->rate_mask = (uint16_t *)ptr[0];     \
    ptr += 2;                                \
    \
    (p)->bits_mask = ptr[0];                 \
    ptr += 1;                                \
    \
    (p)->data_port = (uint16_t*)ptr[0];      \
    ptr += 2;                                \
    } while(0)

#else
#define DETECT_REQUEST_SIZE(sf)  sizeof(detect_request_t)

#define DETECT_REQUEST_ENCODE(sf, d, p) do { \
    memcpy(d, p, DETECT_REQUEST_SIZE(sf));                                  \
    } while(0)

#define DETECT_REQUEST_DECODE(sf, p, d) do { \
    memcpy(p, d, DETECT_REQUEST_SIZE(sf));                                  \
    } while(0)
#endif

typedef enum detect_type_e {
    DETECT_TYPE_FIRST_RUN = 1,
    DETECT_TYPE_EXIT
} detect_type_t;

/**
 * multicast response from server for speaker
 */
typedef struct detect_response_s {
    uint8_t ver: 4;
    detect_type_t type: 4;
    addr_t addr;
} detect_response_t;

#endif //PACKAGE_DETECT_H
