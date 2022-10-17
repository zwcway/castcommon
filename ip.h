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


#ifndef IP_H
#define IP_H

#if WIN32

#include <ws2tcpip.h>
#include <netioapi.h>

typedef int sa_family_t;
typedef u_long in_addr_t;
#else
#include <netinet/in.h>
#include <net/if.h>
#endif

#include <stdint-gcc.h>

typedef struct addr_s {
    sa_family_t type;
    union {
        struct in_addr ipv4;
        struct in6_addr ipv6;
    };
} addr_t;

typedef union mac_address_u {
    uint8_t mac[6];
} mac_address_t;

typedef struct interface_s {
    int ifindex;
    addr_t ip;
    mac_address_t mac;
    int mtu;
    char name[IF_NAMESIZE];
} interface_t;

#define EQUAL_SOCK_ADDR(s, a) \
    ((s)->ss_family == AF_INET ? \
    ((struct sockaddr_in *) (s))->sin_addr.s_addr == (a)->ipv4.s_addr : \
    memcmp(&((struct sockaddr_in6 *) (s))->sin6_addr, &(a)->ipv6, sizeof(struct in6_addr)) == 0)

#define SOCKADDR_SIZE(family) \
    ( (family) == AF_INET ? \
      sizeof(struct sockaddr_in) : \
      ( (family) == AF_INET6 ? sizeof(struct sockaddr_in6) : 0) )


int is_multicast_addr(const char *ip);

int addr_stoa(addr_t *addr, const char *ip);

int is_addr_valid(const addr_t *addr);

void addr_reset(addr_t *addr);

in_addr_t ip_addr(const char *ip);

int set_sockaddr(struct sockaddr_storage *__restrict dst, const addr_t *__restrict src, uint32_t port);

void get_sockaddr(addr_t *__restrict dst, const struct sockaddr_storage *__restrict src);

uint16_t sockaddr_port(const struct sockaddr_storage *addr);

const char *sockaddr_ntop(const struct sockaddr_storage *__restrict addr);

const char *addr_ntop(const addr_t *__restrict addr);

const char *mac_ntop(const mac_address_t *mac);

int list_interfaces(sa_family_t sf, interface_t *ifl, uint8_t max_len);

int get_default_interface(sa_family_t sf, char *name);

int get_interface(sa_family_t sf, interface_t *__restrict ift, const char *__restrict name);

#endif //IP_H
