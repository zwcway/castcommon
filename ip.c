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


#include <stddef.h>
#include <memory.h>
#include <stdio.h>
#include <unistd.h>

#if WIN32

#include <ws2tcpip.h>
#include <iphlpapi.h>

#else

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#endif


#include "ip.h"
#include "speaker_struct.h"
#include "log.h"
#include "utils.h"
#include "error.h"
#include "common.h"

#define BUFFER_SIZE 4096

static char s[INET6_ADDRSTRLEN];

LOG_TAG_DECLR("common");

int addr_stoa(addr_t *addr, const char *ip) {
  int ret;
  if (NULL == addr) return -1;

  memset(addr, 0, sizeof(addr_t));

  ret = inet_pton(AF_INET, ip, &(addr->ipv4.s_addr));
  if (ret != 0) {
    addr->type = AF_INET;
    return ret;
  }

  ret = inet_pton(AF_INET6, ip, &addr->ipv6);
  if (ret != 0) {
    addr->type = AF_INET6;
    return ret;
  }

  return OK;
}

in_addr_t ip_addr(const char *ip) {
  return inet_addr(ip);
}

int is_multicast_addr(const char *ip) {
  addr_t addr;
  uint32_t a;

  if (0 == addr_stoa(&addr, ip)) {
    return 0;
  }

  if (addr.type == AF_INET) {
    a = ntohl(addr.ipv4.s_addr);

    // mask 239.0.0.1/8
    return (a >> 24) == 0xEF;
  }

  // mask FF02::1/16
#if WIN32
  return ntohs(addr.ipv6.u.Word[0]) == 0xFF02;
#else
  return ntohs(addr.ipv6.s6_addr16[0]) == 0xFF02;
#endif
}

int set_sockaddr(struct sockaddr_storage *dst, const addr_t *src, uint32_t port) {
  struct sockaddr_in *v4 = (struct sockaddr_in *) dst;
  struct sockaddr_in6 *v6 = (struct sockaddr_in6 *) dst;

  if (dst == NULL || src == NULL) {
    return 0;
  }

  if (src->type == AF_INET) {
    v4->sin_family = AF_INET;
    v4->sin_addr.s_addr = src->ipv4.s_addr;
    v4->sin_port = htons(port);

    return sizeof(struct sockaddr_in);
  } else if (src->type == AF_INET6) {
    v6->sin6_family = AF_INET6;
    v6->sin6_addr = src->ipv6;
    v6->sin6_port = htons(port);

    return sizeof(struct sockaddr_in6);
  }

  return 0;
}


void get_sockaddr(addr_t *dst, const struct sockaddr_storage *src) {
  memset(dst, 0, sizeof(addr_t));

  if (src == NULL) {
    return;
  }

  dst->type = src->ss_family;

  if (src->ss_family == AF_INET) {
    dst->ipv4.s_addr = ((struct sockaddr_in *) src)->sin_addr.s_addr;
  } else if (src->ss_family == AF_INET6) {
    dst->ipv6 = ((struct sockaddr_in6 *) src)->sin6_addr;
  }
}

uint16_t sockaddr_port(const struct sockaddr_storage *addr) {
  if (addr == NULL) {
    return 0;
  }
  if (addr->ss_family == AF_INET) {
    return ntohs(((struct sockaddr_in *) addr)->sin_port);
  } else if (addr->ss_family == AF_INET6) {
    return ntohs(((struct sockaddr_in6 *) addr)->sin6_port);
  }
  return 0;
}

const char *sockaddr_ntop(const struct sockaddr_storage *addr) {
  memset(s, 0, sizeof(s));
  if (addr == NULL) {
    return s;
  }
  if (addr->ss_family == AF_INET) {
    inet_ntop(AF_INET, &((struct sockaddr_in *) addr)->sin_addr, s, INET_ADDRSTRLEN);
  } else if (addr->ss_family == AF_INET6) {
    inet_ntop(AF_INET6, &((struct sockaddr_in6 *) addr)->sin6_addr, s, INET6_ADDRSTRLEN);
  }

  return s;
}

const char *addr_ntop(const addr_t *addr) {
  static char s[INET6_ADDRSTRLEN];

  memset(s, 0, sizeof(s));
  if (addr == NULL) {
    return s;
  }
  if (addr->type == AF_INET) {
    inet_ntop(AF_INET, (void *) &addr->ipv4, s, INET_ADDRSTRLEN);
  } else if (addr->type == AF_INET6) {
    inet_ntop(AF_INET6, (void *) &addr->ipv6, s, INET6_ADDRSTRLEN);
  }

  return s;
}

const char *mac_ntop(const mac_address_t *mac) {
  sprintf(s, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac->mac[0], mac->mac[1], mac->mac[2], mac->mac[3], mac->mac[4], mac->mac[5]);
  return s;
}

#if WIN32

int list_interfaces(sa_family_t sf, interface_t *ifl, uint8_t max_len) {
  ULONG outBufLen = 0;
  DWORD dwRetVal = 0;
  PIP_ADAPTER_ADDRESSES pAdapterAddrs;

  int l = 0;
  if (NULL == ifl) return 0;

  pAdapterAddrs = (PIP_ADAPTER_ADDRESSES) malloc(sizeof(IP_ADAPTER_ADDRESSES));

  // retry up to 5 times, to get the adapter address needed
  for (int i = 0; i < 5 && (dwRetVal == ERROR_BUFFER_OVERFLOW || dwRetVal == NO_ERROR); ++i) {
    dwRetVal = GetAdaptersAddresses(sf, GAA_FLAG_INCLUDE_ALL_COMPARTMENTS, NULL, pAdapterAddrs, &outBufLen);
    if (dwRetVal == NO_ERROR) {
      break;
    } else if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
      free(pAdapterAddrs);
      pAdapterAddrs = (PIP_ADAPTER_ADDRESSES) malloc(outBufLen);
    } else {
      pAdapterAddrs = 0;
      break;
    }
  }
  if (dwRetVal == NO_ERROR) {
    PIP_ADAPTER_ADDRESSES pAdapterAddr = pAdapterAddrs;
    while (pAdapterAddr) {
      strcpy(ifl[l].name, pAdapterAddr->AdapterName);
      if (sf == AF_INET) {
        ifl[l].ifindex = (int) pAdapterAddr->IfIndex;
      } else {
        ifl[l].ifindex = (int) pAdapterAddr->Ipv6IfIndex;
      }
      ifl[l].mtu = (int) pAdapterAddr->Mtu;
      memcpy(&ifl[l].mac, pAdapterAddr->PhysicalAddress, pAdapterAddr->PhysicalAddressLength);

      PIP_ADAPTER_UNICAST_ADDRESS_LH pIpAddress = pAdapterAddr->FirstUnicastAddress;
      if (pIpAddress != 0 && l < max_len) {
        ifl[l].ip.type = pIpAddress->Address.lpSockaddr->sa_family;

        if (pIpAddress->Address.lpSockaddr->sa_family == AF_INET) {
          ifl[l].ip.ipv4 = ((struct sockaddr_in *) pIpAddress->Address.lpSockaddr)->sin_addr;
        } else {
          ifl[l].ip.ipv6 = ((struct sockaddr_in6 *) pIpAddress->Address.lpSockaddr)->sin6_addr;
        }
//        pIpAddress = pIpAddress->Next;
      }
      if (l++ >= max_len - 1) {
        break;
      }
      pAdapterAddr = pAdapterAddr->Next;
    }
  }
  free(pAdapterAddrs);
  return l;
}

#else

int list_interfaces(sa_family_t sf, interface_t *ifl, uint8_t max_len) {
  struct ifaddrs *ifaddr = NULL, *ifEntry = NULL;
  int i, l;

  if (NULL == ifl) return 0;

  if (getifaddrs(&ifaddr) != 0) {
    LOGE("getifaddrs error: %m");
    return -1;
  }

  for (ifEntry = ifaddr, l = 0; ifEntry != NULL && l < max_len; ifEntry = ifEntry->ifa_next) {
    if (ifEntry->ifa_addr->sa_data == NULL || ifEntry->ifa_name == NULL) continue;
    if (ifEntry->ifa_addr->sa_family != sf) continue;

    strcpy(ifl[l].name, ifEntry->ifa_name);
    ifl[l].ip.type = sf;

    ifl[l].ifindex = (int) if_nametoindex(ifEntry->ifa_name);

    if (sf == AF_INET)
      ifl[l].ip.ipv4 = ((struct sockaddr_in *) ifEntry->ifa_addr)->sin_addr;
    else
      ifl[l].ip.ipv6 = ((struct sockaddr_in6 *) ifEntry->ifa_addr)->sin6_addr;

    l++;
  }
  freeifaddrs(ifaddr);

  return l;
}

#endif

#if WIN32

int get_default_interface(sa_family_t sf, char *name) {
}

#else

int get_default_interface(sa_family_t sf, char *name) {
  int received_bytes = 0, msg_len = 0, route_attribute_len = 0;
  int sock = -1, msgseq = 0;
  struct nlmsghdr *nlh, *nlmsg;
  struct rtmsg *route_entry;
  // This struct contain route attributes (route type)
  struct rtattr *route_attribute;
  char iface[IF_NAMESIZE];
  char msgbuf[BUFFER_SIZE], buffer[BUFFER_SIZE];
  char *ptr = buffer;
  struct timeval tv;

  if (NULL == name) return -1;

  if ((sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
    perror("socket failed");
    return -1;
  }

  memset(msgbuf, 0, sizeof(msgbuf));
  memset(iface, 0, sizeof(iface));
  memset(buffer, 0, sizeof(buffer));

  /* point the header and the msg structure pointers into the buffer */
  nlmsg = (struct nlmsghdr *) msgbuf;

  /* Fill in the nlmsg header*/
  nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  nlmsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .
  nlmsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
  nlmsg->nlmsg_seq = msgseq++; // Sequence of the message packet.
  nlmsg->nlmsg_pid = getpid(); // PID of process sending the request.

  /* 1 Sec Timeout to avoid stall */
  tv.tv_sec = 1;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv, sizeof(struct timeval));
  /* send msg */
  if (send(sock, nlmsg, nlmsg->nlmsg_len, 0) < 0) {
    perror("send failed");
    return -1;
  }

  /* receive response */
  do {
    received_bytes = recv(sock, ptr, sizeof(buffer) - msg_len, 0);
    if (received_bytes < 0) {
      perror("Error in recv");
      return -1;
    }

    nlh = (struct nlmsghdr *) ptr;

    /* Check if the header is valid */
    if ((NLMSG_OK(nlmsg, received_bytes) == 0) ||
        (nlmsg->nlmsg_type == NLMSG_ERROR)) {
      perror("Error in received packet");
      return -1;
    }

    /* If we received all data break */
    if (nlh->nlmsg_type == NLMSG_DONE)
      break;
    else {
      ptr += received_bytes;
      msg_len += received_bytes;
    }

    /* Break if its not a multi part message */
    if ((nlmsg->nlmsg_flags & NLM_F_MULTI) == 0)
      break;
  } while ((nlmsg->nlmsg_seq != msgseq) || (nlmsg->nlmsg_pid != getpid()));

  /* parse response */
  for (; NLMSG_OK(nlh, received_bytes); nlh = NLMSG_NEXT(nlh, received_bytes)) {
    /* Get the route data */
    route_entry = (struct rtmsg *) NLMSG_DATA(nlh);

    /* We are just interested in main routing table */
    if (route_entry->rtm_table != RT_TABLE_MAIN)
      continue;

    route_attribute = (struct rtattr *) RTM_RTA(route_entry);
    route_attribute_len = RTM_PAYLOAD(nlh);

    /* Loop through all attributes */
    for (; RTA_OK(route_attribute, route_attribute_len);
           route_attribute = RTA_NEXT(route_attribute, route_attribute_len)) {
      switch (route_attribute->rta_type) {
        case RTA_OIF:
          if_indextoname(*(int *) RTA_DATA(route_attribute), iface);
          break;
        case RTA_GATEWAY:
//            if (sf == AF_INET) {
//              inet_ntop(sf, RTA_DATA(route_attribute), gateway_address, INET_ADDRSTRLEN);
//            } else {
//              inet_ntop(sf, RTA_DATA(route_attribute), gateway_address, INET6_ADDRSTRLEN);
//            }
          break;
        default:
          break;
      }
    }

    if ((*iface)) {
      memcpy(name, iface, IF_NAMESIZE);
      break;
    }
  }

  closesocket(sock);

  return *iface ? 1 : 0;
}

#endif

#if WIN32

int get_interface(sa_family_t sf, interface_t *ift, const char *name) {
  int i, found;
  if (NULL == ift) return -1;
  if (NULL == name || strlen(name) == 0) {
    return -1;
  }

  if (strlen(name) >= IF_NAMESIZE) {
    LOGW("Too long interface name %s", name);
    return -1;
  }
    // ifindex
  else if ((i = is_uint32(name)) >= 0) {
    ift->ifindex = i;
  } else {
    strcpy(ift->name, name);

    // do not support ip address
    if (addr_stoa(&ift->ip, ift->name)) {
      return -1;
    }
  }

  interface_t list[16] = {0};

  int maxlen = list_interfaces(sf, list, 16);

  found = -1;
  for (i = 0; i < maxlen; ++i) {
    if (ift->ifindex) {
      if (ift->ifindex != list[i].ifindex)
        continue;
    } else if (strcmp(ift->name, list[i].name) != 0)
      continue;

    found = 1;
    *ift = list[i];
    break;
  }

  return found;
}

#else

int get_interface(sa_family_t sf, interface_t *ift, const char *name) {
  int sockfd;
  struct ifreq ifr = {0};
  struct ifaddrs *ifaddr = NULL, *ifEntry = NULL;
  int i, found = 0;

  if (NULL == ift) return -1;

  if (NULL == name || strlen(name) == 0) {
    return -1;
  }
  if (strlen(name) >= IF_NAMESIZE) {
    LOGW("Too long interface name %s", name);
    return -1;
  } else if ((i = is_uint32(name)) >= 0) {
    if (NULL == if_indextoname(i, ift->name)) {
      LOGW("get name by index error: %m");
      return -1;
    }
  } else {
    strcpy(ift->name, name);
  }

  // do not support ip address
  if (addr_stoa(&ift->ip, ift->name)) {
    return -1;
  }

  if (getifaddrs(&ifaddr) != 0) {
    LOGE("getifaddrs error: %m");
    return -1;
  }

  found = 0;
  for (ifEntry = ifaddr; ifEntry != NULL; ifEntry = ifEntry->ifa_next) {
    if (ifEntry->ifa_addr->sa_data == NULL || ifEntry->ifa_name == NULL) continue;
    if (ifEntry->ifa_addr->sa_family != sf) continue;
    if (strcmp(ifEntry->ifa_name, ift->name) != 0) continue;

    found = 1;

    ift->ip.type = sf;
    ift->ifindex = (int) if_nametoindex(ifEntry->ifa_name);

    if (sf == AF_INET)
      ift->ip.ipv4 = ((struct sockaddr_in *) ifEntry->ifa_addr)->sin_addr;
    else
      ift->ip.ipv6 = ((struct sockaddr_in6 *) ifEntry->ifa_addr)->sin6_addr;

    break;
  }

  if (!found) {
    freeifaddrs(ifaddr);
    LOGW("Get address V6 for interface '%s' failed: %m", name);
    return -1;
  }
  freeifaddrs(ifaddr);

  sockfd = socket(sf, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    LOGW("create socket error: %m");
    return -1;
  }
  bzero(&ift->mac, sizeof(mac_address_t));
  strcpy(ifr.ifr_name, ift->name);

  if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
    closesocket(sockfd);
    LOGW("Get IF flags Fail: %s", name);
    return -1;
  }
  if (ifr.ifr_flags & IFF_LOOPBACK) {
    closesocket(sockfd);
    LOGW("Can not count loopback");
    return -1;
  }
  if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
    closesocket(sockfd);
    LOGW("Get mac address error: %m");
    return -1;
  }
  memcpy(&ift->mac, ifr.ifr_hwaddr.sa_data, sizeof(ift->mac));

  if (ioctl(sockfd, SIOCGIFMTU, &ifr) != 0) {
    LOGW("Get MTU Fail: %s\n\n", name);
    closesocket(sockfd);
    return -1;
  }
  ift->mtu = ifr.ifr_mtu;

  closesocket(sockfd);
  return 0;
}

#endif