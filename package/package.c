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
#include "control.h"
#include "detect.h"
#include "pcm.h"

void control_header_encode(void *pack, const control_header_t *ctl) {
  uint8_t *ptr = (uint8_t *) (pack);

  ptr[0] = BIT_4TO8(ctl->ver, ctl->cmd);
  ptr += 1;
  ((uint32_t *) ptr)[0] = ctl->spid;
  ptr += 4;

}

void control_header_decode(control_header_t *ctl, const void *pack) {
  uint8_t *ptr = (uint8_t *) (pack);

  BIT_8TO4(ctl->ver, ctl->cmd, ptr[0]);
  ptr += 1;
  ctl->spid = ((uint32_t *) ptr)[0];
  ptr += 4;
}


void control_package_encode(void *pack, const control_package_t *ctl) {
  uint8_t *ptr = (uint8_t *) (pack);

  control_header_encode(pack, &ctl->header);

  ptr += sizeof(control_header_t);

  switch (ctl->header.cmd) {
    case SPCMD_SAMPLE:
      ptr[0] = BIT_4TO8(ctl->sample.bits, ctl->sample.rate);
      ptr[1] = ctl->sample.channel;
      break;
    case SPCMD_CHUNK:
      ((uint16_t *) ptr)[0] = ctl->chunk.size;
    default:
      break;
  }
}

void control_package_decode(control_package_t *ctl, const void *pack) {
  const uint8_t *ptr = (const uint8_t *) (pack);

  control_header_decode(&ctl->header, pack);
  ptr += sizeof(control_header_t);

  switch (ctl->header.cmd) {
    case SPCMD_SAMPLE:
      BIT_8TO4(ctl->sample.bits, ctl->sample.rate, ptr[0]);
      ctl->sample.channel = ptr[2];
      break;
    case SPCMD_CHUNK:
      ctl->chunk.size = ((uint16_t *) ptr)[0];
    default:
      break;
  }
}

void control_time_encode(void *pack, const control_time_t *ctl) {
  uint8_t *ptr = (uint8_t *) (pack);

  control_header_encode(pack, &ctl->header);
  ptr += sizeof(control_header_t);

  *(int32_t *) ptr = ctl->time;
}

void control_time_decode(control_time_t *ctl, const void *pack) {
  const uint8_t *ptr = (const uint8_t *) (pack);

  control_header_decode(&ctl->header, pack);
  ptr += sizeof(control_header_t);

  ctl->time = ((int32_t *) ptr)[0];
}

bool control_is_cmd(const void *buf, control_command_t c) {
  control_header_t h;
  control_header_decode(&h, buf);
  return h.cmd == c;
}

void spk_detect_request_encode(sa_family_t sf, void *pack, const spk_detect_request_t *req) {
  uint8_t *ptr = (uint8_t *) (pack);

  *((uint8_t *) (ptr)) =
      ((req->ver & 0x0F) << 4) | ((req->connected != 0) << 3) | ((req->addr.type == AF_INET6) << 2);
  ptr += 1;

  memcpy(ptr, &req->addr.ipv6, (sf) == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr));
  ptr += (sf) == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr);

  ((uint32_t *) ptr)[0] = req->id;
  ptr += 4;

  memcpy(ptr, &req->mac.mac, 6);
  ptr += 6;

  ((uint16_t *) ptr)[0] = req->rate_mask;
  ptr += 2;

  ptr[0] = req->bits_mask;
  ptr += 1;

  ((uint16_t *) ptr)[0] = req->data_port;
  ptr += 2;
}

void spk_detect_request_decode(sa_family_t sf, spk_detect_request_t *req, const void *pack) {
  uint8_t *ptr = (uint8_t *) (pack);

  req->ver = (ptr[0] >> 4) & 0x0F;
  req->connected = (ptr[0] >> 3) & 0x01;
  req->addr.type = ((ptr[0] >> 2) & 0x01) ? AF_INET6 : AF_INET;
  ptr += 1;

  memcpy(&req->addr.ipv6, ptr, (sf) == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr));
  ptr += (sf) == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr);

  req->id = ((uint16_t *) ptr)[0];
  ptr += 4;

  memcpy(&req->mac.mac, ptr, 6);
  ptr += 6;

  req->rate_mask = ((uint16_t *) ptr)[0];
  ptr += 2;

  req->bits_mask = ptr[0];
  ptr += 1;

  req->data_port = ((uint16_t *) ptr)[0];
  ptr += 2;
}

void spk_detect_response_encode(sa_family_t sf, void *pack, const spk_detect_response_t *res) {
  uint8_t *ptr = (uint8_t *) (pack);

  ptr[0] = BIT_4TO8(res->ver, res->type);
  ptr += 1;

  memcpy(ptr, &res->addr.ipv6, (sf) == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr));
  ptr += (sf) == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr);

}

void spk_detect_response_decode(sa_family_t sf, spk_detect_response_t *res, const void *pack) {
  uint8_t *ptr = (uint8_t *) (pack);

  BIT_8TO4(res->ver, res->type, ptr[0]);
  ptr += 1;

  memcpy(&res->addr.ipv6, ptr, (sf) == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr));
  res->addr.type = sf;
  ptr += (sf) == AF_INET ? sizeof(struct in_addr) : sizeof(struct in6_addr);

}

/**
 * s
 * @param pack
 * @param hd
 */
void pcm_header_encode(void *pack, const pcm_header_t *hd) {
  uint8_t *ptr = (uint8_t *) pack;

  ptr[0] = BIT_4TO8(hd->ver, hd->compress);
  ptr += 1;

  ptr[0] = BIT_4TO8(hd->sample.bits, hd->sample.rate);
  ptr[1] = hd->sample.channel;
  ptr += 2;

  ((uint16_t *) ptr)[0] = hd->seq;
  ptr += 2;

  ((uint16_t *) ptr)[0] = hd->len;
  ptr += 2;

}

void pcm_header_decode(pcm_header_t *hd, const void *pack) {
  uint8_t *ptr = (uint8_t *) pack;

  BIT_8TO4(hd->ver, hd->compress, ptr[0]);
  ptr += 1;

  BIT_8TO4(hd->sample.bits, hd->sample.rate, ptr[0]);
  hd->sample.channel = ptr[1];
  ptr += 2;

  hd->seq = ((uint16_t *) ptr)[0];
  ptr += 2;

  hd->time = ((uint32_t *) ptr)[0];
  ptr += 4;

  hd->len = ((uint16_t *) ptr)[0];
  ptr += 2;

}