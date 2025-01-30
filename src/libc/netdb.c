#include <stdlib.h>
#include <string.h>
#include "../shim.h"
#include "sys/socket.h"
#include "netdb.h"

static int* shim___h_errno_location_impl() {
  return &h_errno;
}

SHIM_WRAP(__h_errno_location);

typedef struct linux_addrinfo linux_addrinfo;

struct linux_addrinfo {
  int              ai_flags;
  int              ai_family;
  int              ai_socktype;
  int              ai_protocol;
  socklen_t        ai_addrlen;
  linux_sockaddr*  ai_addr;
  char*            ai_canonname;
  linux_addrinfo*  ai_next;
};

#define LINUX_AI_PASSIVE      0x01
#define LINUX_AI_CANONNAME    0x02
#define LINUX_AI_NUMERICHOST  0x04
#define LINUX_AI_V4MAPPED     0x08
#define LINUX_AI_ALL          0x10
#define LINUX_AI_ADDRCONFIG   0x20

#define KNOWN_LINUX_AI_FLAGS ( \
  LINUX_AI_PASSIVE     |       \
  LINUX_AI_CANONNAME   |       \
  LINUX_AI_NUMERICHOST |       \
  LINUX_AI_V4MAPPED    |       \
  LINUX_AI_ALL         |       \
  LINUX_AI_ADDRCONFIG          \
)

static int linux_to_native_ai_flags(int linux_flags) {

  assert((linux_flags & ~KNOWN_LINUX_AI_FLAGS) == 0);

  int flags = 0;

  if (linux_flags & LINUX_AI_PASSIVE)     flags |= AI_PASSIVE;
  if (linux_flags & LINUX_AI_CANONNAME)   flags |= AI_CANONNAME;
  if (linux_flags & LINUX_AI_NUMERICHOST) flags |= AI_NUMERICHOST;
  if (linux_flags & LINUX_AI_V4MAPPED)    flags |= AI_V4MAPPED;
  if (linux_flags & LINUX_AI_ALL)         flags |= AI_ALL;
  if (linux_flags & LINUX_AI_ADDRCONFIG)  flags |= AI_ADDRCONFIG;

  return flags;
}

static linux_addrinfo* copy_addrinfo(struct addrinfo* info) {

  linux_addrinfo* linux_info = malloc(sizeof(linux_addrinfo));

  linux_info->ai_flags     = 0;
  linux_info->ai_socktype  = native_to_linux_sock_type(info->ai_socktype);
  linux_info->ai_protocol  = info->ai_protocol;
  linux_info->ai_canonname = info->ai_canonname != NULL ? strdup(info->ai_canonname) : NULL;

  switch (info->ai_family) {
    case PF_INET:
      linux_info->ai_family    = LINUX_PF_INET;
      linux_info->ai_addrlen   = sizeof(linux_sockaddr_in);
      linux_info->ai_addr      = malloc(sizeof(linux_sockaddr_in));
      native_to_linux_sockaddr_in((linux_sockaddr_in*)linux_info->ai_addr, (struct sockaddr_in*)info->ai_addr);
      break;
    case PF_INET6:
      linux_info->ai_family    = LINUX_PF_INET6;
      linux_info->ai_addrlen   = sizeof(linux_sockaddr_in6);
      linux_info->ai_addr      = malloc(sizeof(linux_sockaddr_in6));
      native_to_linux_sockaddr_in6((linux_sockaddr_in6*)linux_info->ai_addr, (struct sockaddr_in6*)info->ai_addr);
      break;
    default:
      assert(0);
  }

  return linux_info;
}

static int shim_getaddrinfo_impl(const char* hostname, const char* servname, const linux_addrinfo* linux_hints, linux_addrinfo** res) {

  struct addrinfo hints;
  if (linux_hints != NULL) {
    switch (linux_hints->ai_family) {
      case LINUX_AF_UNSPEC: hints.ai_family = AF_UNSPEC; break;
      case LINUX_AF_INET:   hints.ai_family = AF_INET;   break;
      case LINUX_AF_INET6:  hints.ai_family = AF_INET6;  break;
      default:
        assert(0);
    }
    hints.ai_socktype = linux_to_native_sock_type(linux_hints->ai_socktype);
    hints.ai_protocol = linux_hints->ai_protocol;
    hints.ai_flags    = linux_to_native_ai_flags(linux_hints->ai_flags);
  } else {
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    hints.ai_flags    = 0;
  }
  hints.ai_addrlen   = 0;
  hints.ai_addr      = NULL;
  hints.ai_canonname = NULL;
  hints.ai_next      = NULL;

  struct addrinfo* list_head;
  int err = getaddrinfo(hostname, servname, &hints, &list_head);
  if (err == 0) {
    linux_addrinfo* linux_info = (*res = copy_addrinfo(list_head));
    for (struct addrinfo* info = list_head->ai_next; info != NULL; info = info->ai_next) {
      linux_info->ai_next = copy_addrinfo(info);
      linux_info = linux_info->ai_next;
    }
    linux_info->ai_next = NULL;
    freeaddrinfo(list_head);
  }

  return err;
}

static void shim_freeaddrinfo_impl(linux_addrinfo* ai) {
  while (ai != NULL) {
    linux_addrinfo* next = ai->ai_next;
    if (ai->ai_canonname != NULL) {
      free(ai->ai_canonname);
    }
    free(ai->ai_addr);
    free(ai);
    ai = next;
  }
}

SHIM_WRAP(getaddrinfo);
SHIM_WRAP(freeaddrinfo);

#define LINUX_NI_NUMERICHOST  1
#define LINUX_NI_NUMERICSERV  2
#define LINUX_NI_NOFQDN       4
#define LINUX_NI_NAMEREQD     8
#define LINUX_NI_DGRAM       16

#define KNOWN_LINUX_NI_FLAGS ( \
  LINUX_NI_NUMERICHOST |       \
  LINUX_NI_NUMERICSERV |       \
  LINUX_NI_NOFQDN      |       \
  LINUX_NI_NAMEREQD    |       \
  LINUX_NI_DGRAM               \
)

static int linux_to_native_ni_flags(int linux_flags) {

  assert((linux_flags & ~KNOWN_LINUX_NI_FLAGS) == 0);

  int flags = 0;

  if (linux_flags & LINUX_NI_NUMERICHOST) flags |= NI_NUMERICHOST;
  if (linux_flags & LINUX_NI_NUMERICSERV) flags |= NI_NUMERICSERV;
  if (linux_flags & LINUX_NI_NOFQDN)      flags |= NI_NOFQDN;
  if (linux_flags & LINUX_NI_NAMEREQD)    flags |= NI_NAMEREQD;
  if (linux_flags & LINUX_NI_DGRAM)       flags |= NI_DGRAM;

  return flags;
}

static int shim_getnameinfo_impl(const linux_sockaddr* linux_addr, socklen_t linux_addrlen, char* host, size_t hostlen, char* serv, size_t servlen, int linux_flags) {

  switch (linux_addr->sa_family) {
    case LINUX_AF_UNIX:
      {
        struct sockaddr_un addr;
        assert(linux_addrlen <= sizeof(struct linux_sockaddr_un));
        linux_to_native_sockaddr_un(&addr, (linux_sockaddr_un*)linux_addr);
        return getnameinfo((struct sockaddr*)&addr, sizeof(addr), host, hostlen, serv, servlen, linux_to_native_ni_flags(linux_flags));
      }
      break;
    case LINUX_AF_INET:
      {
        struct sockaddr_in addr;
        assert(linux_addrlen == sizeof(struct linux_sockaddr_in));
        linux_to_native_sockaddr_in(&addr, (linux_sockaddr_in*)linux_addr);
        return getnameinfo((struct sockaddr*)&addr, sizeof(addr), host, hostlen, serv, servlen, linux_to_native_ni_flags(linux_flags));
      }
      break;
    case LINUX_AF_INET6:
      {
        struct sockaddr_in6 addr;
        assert(linux_addrlen == sizeof(struct linux_sockaddr_in6));
        linux_to_native_sockaddr_in6(&addr, (linux_sockaddr_in6*)linux_addr);
        return getnameinfo((struct sockaddr*)&addr, sizeof(addr), host, hostlen, serv, servlen, linux_to_native_ni_flags(linux_flags));
      }
      break;
    default:
      assert(0);
  }
}

SHIM_WRAP(getnameinfo);
