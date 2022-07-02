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


#include "audio.h"


int rate_name(audio_rate_t rate)
{
  switch (rate) {
  case RATE_44100:
    return 44100;
  case RATE_48000:
    return 48000;
  case RATE_96000:
    return 96000;
  case RATE_192000:
    return 192000;
  case RATE_384000:
    return 384000;
  default:
    return 0;
  }
}


int bits_name(audio_bits_t bits)
{
  switch (bits) {
  case BIT_16:
    return 16;
  case BIT_20:
    return 20;
  case BIT_24:
    return 24;
  case BIT_32:
  case BIT_32_FLOAT:
    return 32;
  default:
    return 0;
  }
}

char *channel_name(audio_channel_t channel)
{
  switch (channel) {
  case CHANNEL_FRONT_LEFT:
    return "Front Left";
  case CHANNEL_FRONT_RIGHT:
    return "Front Right";
  case CHANNEL_FRONT_CENTER:
    return "Front Center";
  case CHANNEL_LOW_FREQUENCY:
    return "LFE / Subwoofer";
  case CHANNEL_BACK_LEFT:
    return "Rear Left";
  case CHANNEL_BACK_RIGHT:
    return "Rear Right";
  case CHANNEL_FRONT_LEFT_OF_CENTER:
    return "Front Left Of Center";
  case CHANNEL_FRONT_RIGHT_OF_CENTER:
    return "Front Right Of Center";
  case CHANNEL_BACK_CENTER:
    return "Rear Center";
  case CHANNEL_SIDE_LEFT:
    return "Side Left";
  case CHANNEL_SIDE_RIGHT:
    return "Side Right";
  case CHANNEL_TOP_CENTER:
    return "Top Center";
  case CHANNEL_TOP_FRONT_LEFT:
    return "Top Front Left";
  case CHANNEL_TOP_FRONT_CENTER:
    return "Top Front Center";
  case CHANNEL_TOP_FRONT_RIGHT:
    return "Top Front Right";
  case CHANNEL_TOP_BACK_LEFT:
    return "Top Rear Left";
  case CHANNEL_TOP_BACK_CENTER:
    return "Top Rear Center";
  case CHANNEL_TOP_BACK_RIGHT:
    return "Top Rear Right";
  default:
    return "Unknown";
  }
}
