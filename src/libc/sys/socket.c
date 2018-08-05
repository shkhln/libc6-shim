#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "../../shim.h"

struct linux_sockaddr {
  uint16_t sa_family;
  char     sa_data[14];
};

int shim_bind_impl(int s, const struct linux_sockaddr* linux_addr, socklen_t addrlen) {

  struct sockaddr_un addr;

  assert(sizeof(addr) >= addrlen);

  assert(linux_addr->sa_family == 1);
  assert(linux_addr->sa_data[0] == 0 && str_starts_with(&linux_addr->sa_data[1], "nvidia"));

  //memset(&addr, 0, sizeof(addr));

  addr.sun_len    = 0;
  addr.sun_family = linux_addr->sa_family;

  snprintf(addr.sun_path, sizeof(addr.sun_path), "/var/run/%s", &linux_addr->sa_data[1]);

  int err = bind(s, (struct sockaddr*)&addr, sizeof(addr));
  if (err == 0) {
    unlink(addr.sun_path);
  } else {
    perror(__func__);
  }

  return err;
}

int shim_bind(int s, const struct linux_sockaddr *addr, socklen_t addrlen) {
  LOG_ARGS("%d, %p, %u", s, addr, addrlen);
  int err = shim_bind_impl(s, addr, addrlen);
  LOG_RES("%d", err);
  return err;
}

SYM_EXPORT(shim_bind, bind);
