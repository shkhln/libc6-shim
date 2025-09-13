#include <ifaddrs.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <net/if.h>
#include <net/if_dl.h>
#include "sys/socket.h"
#include "../shim.h"

typedef struct linux_ifaddrs linux_ifaddrs;

struct linux_ifaddrs {
  linux_ifaddrs*  ifa_next;
  char*           ifa_name;
  unsigned int    ifa_flags;
  linux_sockaddr* ifa_addr;
  linux_sockaddr* ifa_netmask;
  linux_sockaddr* ifa_dstaddr;
  void*           ifa_data;
};

struct linux_sockaddr_ll {
  unsigned short sll_family;
  unsigned short sll_protocol;
  int            sll_ifindex;
  unsigned short sll_hatype;
  unsigned char  sll_pkttype;
  unsigned char  sll_halen;
  unsigned char  sll_addr[8];
};

typedef struct linux_sockaddr_ll linux_sockaddr_ll;

static void native_to_linux_sockaddr_dl(struct linux_sockaddr_ll* dest, const struct sockaddr_dl* src) {
  dest->sll_family   = LINUX_AF_PACKET;
  dest->sll_protocol = 0; // ?
  dest->sll_ifindex  = src->sdl_index;
  dest->sll_hatype   = 0;
  dest->sll_pkttype  = 0;
  dest->sll_halen    = 0;
  memset(dest->sll_addr, 0, sizeof(dest->sll_addr));
}

#define LINUX_IFF_UP          0x0001
#define LINUX_IFF_BROADCAST   0x0002
#define LINUX_IFF_DEBUG       0x0004
#define LINUX_IFF_LOOPBACK    0x0008
#define LINUX_IFF_POINTOPOINT 0x0010
//~ #define LINUX_IFF_NOTRAILERS  0x0020
#define LINUX_IFF_RUNNING     0x0040
#define LINUX_IFF_NOARP       0x0080
#define LINUX_IFF_PROMISC     0x0100
#define LINUX_IFF_ALLMULTI    0x0200
#define LINUX_IFF_MULTICAST   0x1000

static int native_to_linux_if_flags(int flags) {

  int linux_flags = 0;

  if (flags & IFF_UP)          linux_flags |= LINUX_IFF_UP;
  if (flags & IFF_BROADCAST)   linux_flags |= LINUX_IFF_BROADCAST;
  if (flags & IFF_DEBUG)       linux_flags |= LINUX_IFF_DEBUG;
  if (flags & IFF_LOOPBACK)    linux_flags |= LINUX_IFF_LOOPBACK;
  if (flags & IFF_POINTOPOINT) linux_flags |= LINUX_IFF_POINTOPOINT;
  if (flags & IFF_RUNNING)     linux_flags |= LINUX_IFF_RUNNING;
  if (flags & IFF_NOARP)       linux_flags |= LINUX_IFF_NOARP;
  if (flags & IFF_PROMISC)     linux_flags |= LINUX_IFF_PROMISC;
  if (flags & IFF_ALLMULTI)    linux_flags |= LINUX_IFF_ALLMULTI;
  if (flags & IFF_MULTICAST)   linux_flags |= LINUX_IFF_MULTICAST;

  return linux_flags;
}

static struct linux_ifaddrs* copy_ifaddrs(struct ifaddrs* ifa) {

  linux_ifaddrs* linux_ifa = malloc(sizeof(linux_ifaddrs));

  linux_ifa->ifa_name  = strdup(ifa->ifa_name);
  linux_ifa->ifa_flags = native_to_linux_if_flags(ifa->ifa_flags);

  if (ifa->ifa_addr != NULL) {
    switch (ifa->ifa_addr->sa_family) {
      case AF_LINK:
        linux_ifa->ifa_addr = malloc(sizeof(linux_sockaddr_ll));
        native_to_linux_sockaddr_dl((linux_sockaddr_ll*)linux_ifa->ifa_addr, (struct sockaddr_dl*)ifa->ifa_addr);
        break;
      case AF_INET:
        linux_ifa->ifa_addr = malloc(sizeof(linux_sockaddr_in));
        native_to_linux_sockaddr_in((linux_sockaddr_in*)linux_ifa->ifa_addr, (struct sockaddr_in*)ifa->ifa_addr);
        break;
      case AF_INET6:
        linux_ifa->ifa_addr = malloc(sizeof(linux_sockaddr_in6));
        native_to_linux_sockaddr_in6((linux_sockaddr_in6*)linux_ifa->ifa_addr, (struct sockaddr_in6*)ifa->ifa_addr);
        break;
      default:
        assert(0);
    }
  } else {
    linux_ifa->ifa_addr = NULL;
  }

  if (ifa->ifa_netmask != NULL) {
    switch (ifa->ifa_netmask->sa_family) {
      case AF_INET:
        linux_ifa->ifa_netmask = malloc(sizeof(linux_sockaddr_in));
        native_to_linux_sockaddr_in((linux_sockaddr_in*)linux_ifa->ifa_netmask, (struct sockaddr_in*)ifa->ifa_netmask);
        break;
      case AF_INET6:
        linux_ifa->ifa_netmask = malloc(sizeof(linux_sockaddr_in6));
        native_to_linux_sockaddr_in6((linux_sockaddr_in6*)linux_ifa->ifa_netmask, (struct sockaddr_in6*)ifa->ifa_netmask);
        break;
      default:
        assert(0);
    }
  } else {
    linux_ifa->ifa_netmask = NULL;
  }

  linux_ifa->ifa_dstaddr = NULL; // ?
  linux_ifa->ifa_data    = NULL;

  return linux_ifa;
}

static int shim_getifaddrs_impl(linux_ifaddrs** res) {

  struct ifaddrs* list_head;
  int err = getifaddrs(&list_head);
  if (err == 0) {
    struct ifaddrs* ifa       = list_head;
    linux_ifaddrs*  linux_ifa = copy_ifaddrs(ifa);
    *res = linux_ifa;
    ifa  = ifa->ifa_next;

    while (ifa != NULL) {
      linux_ifa->ifa_next = copy_ifaddrs(ifa);
      linux_ifa = linux_ifa->ifa_next;
      ifa = ifa->ifa_next;
    }

    linux_ifa->ifa_next = NULL;
    freeifaddrs(list_head);
  }

  return err;
}

static void shim_freeifaddrs_impl(linux_ifaddrs* ifa) {
  while (ifa != NULL) {
    linux_ifaddrs* next = ifa->ifa_next;

    free(ifa->ifa_name);

    if (ifa->ifa_addr    != NULL) free(ifa->ifa_addr);
    if (ifa->ifa_netmask != NULL) free(ifa->ifa_netmask);
    if (ifa->ifa_dstaddr != NULL) free(ifa->ifa_dstaddr);

    free(ifa);

    ifa = next;
  }
}

SHIM_WRAP(getifaddrs);
SHIM_WRAP(freeifaddrs);
