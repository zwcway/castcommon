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


#ifndef SPEAKER_STRUCT_H
#define SPEAKER_STRUCT_H

#include <stdint-gcc.h>
#include "audio.h"
#include "ip.h"
#include "common.h"


#define DEFAULT_MULTICAST_GROUP "239.44.77.16"
#define DEFAULT_MULTICAST_GROUPV6 "FF02:2C:4D:FF::16"
#define DEFAULT_MULTICAST_PORT  4414
#define DEFAULT_RECEIVER_PORT   4417
#define SPEAKER_MAX_COUNT       255

#define SPEAKER_LINE_MAX        255
#define DEFAULT_LINE            0
#define DEFAULT_CHANNEL         CHANNEL_FRONT_LEFT
#define SPEAKER_GROW_STEP       10


typedef uint8_t speaker_line_t;

typedef enum speaker_mode_t
{
    SPEAKER_MODE_UNICAST,
    SPEAKER_MODE_MULTICAST,
} speaker_mode_t;

typedef enum speaker_state_t
{
    SPEAKER_STAT_NONE,
    SPEAKER_STAT_ONLINE,
    SPEAKER_STAT_OFFLINE,
    SPEAKER_STAT_DELETED,
} speaker_state_t;

typedef uint32_t speaker_id_t;

typedef struct speaker_statistic_t
{
    uint32_t queue; // i2s queue size(DMA size)
    uint32_t spend; // spend size for queue
    uint32_t drop;  // droped size
} speaker_statistic_t;

typedef struct speaker_t {
    uint8_t idx;
    speaker_id_t id;
    mac_address_t mac;
    addr_t ip;
    speaker_line_t line;
    uint8_t mode;
    uint16_t dport;
    uint8_t supported;
    struct {
        audio_rate_mask_t rate_mask;
        audio_bits_mask_t bits_mask;
        audio_channel_t channel;
    };
    struct {
        socket_t fd;
    };
    int timeout;
    uint64_t conn_time;
    speaker_state_t state;
    speaker_statistic_t statistic;
} speaker_t;

typedef struct speaker_list_t
{
    uint32_t max;
    uint32_t len;
    speaker_t **speakers;
} speaker_list_t;

typedef struct speaker_flat_list_t
{
    uint32_t max;
    uint32_t len;
    speaker_t *speakers;
} speaker_flat_list_t;

#define SPEAKER_COUNT() \
    (speakers_list_flat.len)

#define SPEAKER_FOREACH(v) \
    for (int _i = 0;       \
        (v) = &speakers_list_flat.speakers[_i], _i < speakers_list_flat.len; \
        ++_i)


#define SPEAKER_STRUCT_INIT_FROM_HEADER(sp, hd)  \
  do {                                              \
    (sp)->idx = 0;                                  \
    (sp)->id = (hd)->id;                            \
    (sp)->rate_mask = (hd)->rate_mask;              \
    (sp)->bits_mask = (hd)->bits_mask;              \
    (sp)->dport = (hd)->data_port;                  \
    (sp)->mac = (hd)->mac;                          \
    (sp)->ip = (hd)->addr;                          \
    (sp)->state = SPEAKER_STAT_OFFLINE;          \
    (sp)->fd = -1;                               \
  } while(0)

#define SPEAKER_ONLINE(sp)   ((sp)->state = SPEAKER_STAT_ONLINE)
#define SPEAKER_OFFLINE(sp)   ((sp)->state = SPEAKER_STAT_OFFLINE)
#define SPEAKER_IS_ONLINE(sp)   ((sp)->state == SPEAKER_STAT_ONLINE)
#define SPEAKER_SUPPORTED(sp)   ((sp)->supported == 1)

#define SPEAKERLIST_GET(line, ch)       (&speakers_list_ch[(line)][(ch)])
#define SPEAKERLIST_ADD(line, ch, sp)   SPEAKERLIST_GET(line, ch)->speakers[SPEAKERLIST_GET(line, ch)->len++] = (sp);

extern speaker_list_t speakers_list_ch[SPEAKER_LINE_MAX][CHANNEL_MAX];
extern speaker_flat_list_t speakers_list_flat;

int speaker_init();

int speaker_deinit();

speaker_list_t *get_speaker_list(speaker_line_t line, audio_channel_t ch);

speaker_t *find_speaker_by_id(uint32_t id);

speaker_t *find_speaker_by_mac(const mac_address_t *mac);

speaker_t *find_speaker_by_ip(const addr_t *ip);

speaker_t *find_speaker_by_addr(const struct sockaddr *addr, socklen_t len);

void speaker_check_online(speaker_t *sp);


speaker_t *add_speaker(speaker_id_t id, speaker_line_t line, audio_channel_t channel);

#endif //SPEAKER_STRUCT_H
