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


#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "speaker_struct.h"
#include "utils.h"
#include "log.h"


speaker_list_t speakers_list_ch[SPEAKER_LINE_MAX][CHANNEL_MAX] = {0};

speaker_flat_list_t speakers_list_flat = {
  .len = 0,
  .max=SPEAKER_MAX_COUNT,
  .speakers = NULL
};

LOG_TAG_DECLR("sp");

void init_speakerlist(speaker_line_t line) {
  speakers_list_flat.speakers = xmalloc(sizeof(speaker_t) * (speakers_list_flat.max));

  for (int i = 0; i < CHANNEL_MAX; ++i) {
    speakers_list_ch[line][i].max = SPEAKER_GROW_STEP;
    speakers_list_ch[line][i].speakers = xmalloc(sizeof(speaker_t *) * (speakers_list_ch[line][i].max));
  }
}

int grow_speakerlist(speaker_list_t *list) {
  if (list->len >= list->max) {// 扩容
    if (list->max >= SPEAKER_MAX_COUNT) {
      return -1;
    }
    list->max += SPEAKER_GROW_STEP;
    list->speakers = realloc(list->speakers, sizeof(speaker_t *) * (list->max));
  }
  return 0;
}

speaker_list_t *get_speaker_list(speaker_line_t line, audio_channel_t ch) {
  if (line < 0 || line >= SPEAKER_LINE_MAX) return NULL;
  if (ch >= CHANNEL_MAX || ch <= 0) return NULL;

  return SPEAKERLIST_GET(line, ch);
}

speaker_t *find_speaker_by_addr(const struct sockaddr *addr, socklen_t len) {
  speaker_t *s;

  if (addr != NULL && addr->sa_family == AF_INET) {
    for (int i = 0; i < speakers_list_flat.len; ++i) {
      s = &speakers_list_flat.speakers[i];
      if (s->ip.ipv4.s_addr == ((struct sockaddr_in *) addr)->sin_addr.s_addr) {
        return s;
      }
    }
  } else if (addr != NULL && addr->sa_family == AF_INET6) {
    for (int i = 0; i < speakers_list_flat.len; ++i) {
      s = &speakers_list_flat.speakers[i];
      if (memcmp(&s->ip.ipv6, &((struct sockaddr_in6 *) addr)->sin6_addr, len) == 0) {
        return s;
      }
    }
  }

  return NULL;
}

speaker_t **find_speaker_by_sp(const speaker_list_t *list, const speaker_t *sp) {
  for (int i = 0; i < list->len; ++i) {
    if (sp == list->speakers[i]) {
      return &list->speakers[i];
    }
  }

  return NULL;
}

speaker_t *find_speaker_by_mac(const mac_address_t *mac) {
  speaker_t *s;

  for (int i = 0; i < speakers_list_flat.len; ++i) {
    s = &speakers_list_flat.speakers[i];
    if (memcmp(&s->mac, mac, sizeof(mac_address_t)) == 0) {
      return s;
    }
  }

  return NULL;
}

speaker_t *find_speaker_by_ip(const addr_t *ip) {
  speaker_t *s;

  for (int i = 0; i < speakers_list_flat.len; ++i) {
    s = &speakers_list_flat.speakers[i];
    if (memcmp(&s->ip, ip, sizeof(addr_t)) == 0) {
      return s;
    }
  }

  return NULL;
}

speaker_t *find_speaker_by_id(uint32_t id) {
  speaker_t *s;

  for (int i = 0; i < speakers_list_flat.len; ++i) {
    s = &speakers_list_flat.speakers[i];
    if (s->id == id) {
      return s;
    }
  }

  return NULL;
}

void speaker_check_online(speaker_t *sp) {
  if (sp == NULL) return;
  if (sp->dport) {
    sp->state = SPEAKER_STAT_ONLINE;
    LOGI("speaker(%u,%s:%d) is online", sp->id, addr_ntop(&sp->ip), sp->dport);
  } else {
    LOGI("speaker(%u,%s:%d) is offline", sp->id, addr_ntop(&sp->ip), sp->dport);
    sp->state = SPEAKER_STAT_OFFLINE;
  }
  sp->time = 10;
}

void pack_list_by_ch__(speaker_list_t *list) {
  speaker_t *sp, *ns = NULL;
  int d = 0;
  for (int i = 0, j = 0; i < list->len; ++i) {
    sp = list->speakers[i];
    if (sp == NULL || sp->state == SPEAKER_STAT_DELETED) {
      for (j = i + 1; j < list->len; ++j) {
        ns = list->speakers[j];
        if (ns == NULL || ns->state == SPEAKER_STAT_DELETED) {
          continue;
        }
        ns = NULL;
        j++;
      }
      if (j == list->len - 1) {
        break;
      }
      if (j < list->len && ns != NULL) {
        *sp = *ns;
        if (sp != NULL) sp->idx = i;
        ns->state = SPEAKER_STAT_DELETED;
        d++;
      }
    } else {
      d++;
    }
  }
  memset(&list->speakers[0] + d, 0, sizeof(speaker_t) * (list->len - d - 1));
  list->len = d + 1;
}

void pack_list(speaker_line_t line) {
  if (line < 0 || line >= SPEAKER_LINE_MAX) {
    return;
  }
  for (int i = 0; i < CHANNEL_MAX; ++i) {
    pack_list_by_ch__(&speakers_list_ch[line][i]);
  }
}

speaker_t *add_speaker(speaker_id_t id, speaker_line_t line, audio_channel_t channel) {
  speaker_t *sp = find_speaker_by_id(id);
  speaker_list_t *list;

  if (sp != NULL) {

    if (line != sp->line || channel != sp->channel) {

    }

    return sp;
  }

  if (speakers_list_flat.len >= speakers_list_flat.max) {
    LOGW("memory full. can not add speaker");
    return NULL;
  }
  sp = &speakers_list_flat.speakers[speakers_list_flat.len];

  sp->idx = speakers_list_flat.len;
  sp->state = SPEAKER_STAT_OFFLINE;
  sp->dport = 0;
  sp->line = line;
  sp->channel = channel;

  speakers_list_flat.len++;

  list = &speakers_list_ch[line][channel];
  if (find_speaker_by_sp(list, sp) == NULL) {
    if (grow_speakerlist(list) < 0) {
      speakers_list_flat.len--;
      LOGW("grow fail. can not add speaker");
      return NULL;
    }
  }
  list->speakers[list->len++] = sp;

  return sp;
}

int edit_speaker() {

}

void change_channel(speaker_t *sp, speaker_line_t new_line, audio_channel_t new_ch) {
  speaker_t **tmp;
  speaker_line_t old_line;
  audio_channel_t old_ch;

  if (sp == NULL || (sp->channel == new_ch && sp->line == new_line)) return;

  old_line = sp->line;
  old_ch = sp->channel;

  sp->line = new_line;
  sp->channel = new_ch;
  sp->state = SPEAKER_STAT_DELETED;

  if ((tmp = find_speaker_by_sp(SPEAKERLIST_GET(old_line, old_ch), sp)) != NULL) {
    // remove sp from old list
    *tmp = NULL;
  }
  SPEAKERLIST_ADD(new_line, new_ch, sp);

  pack_list_by_ch__(SPEAKERLIST_GET(old_line, old_ch));
}

void change_channel_by_id(uint32_t id, speaker_line_t line, audio_channel_t new_ch) {
  speaker_t *sp = find_speaker_by_id(id);
  change_channel(sp, line, new_ch);
}

int speaker_init() {
  init_speakerlist(DEFAULT_LINE);
  return 0;
}


int speaker_deinit() {
  LOGD("speaker deinit");
  for (int i = 0; i < SPEAKER_LINE_MAX; ++i) {
    for (int j = 0; j < CHANNEL_MAX; ++j) {
      if (speakers_list_ch[i][j].speakers) {
        free(speakers_list_ch[i][j].speakers);
        speakers_list_ch[i][j].speakers = 0;
      }
    }
  }

  if (speakers_list_flat.speakers) {
    free(speakers_list_flat.speakers);
    speakers_list_flat.speakers = 0;
  }

  return 0;
}
