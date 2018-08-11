#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "../../shim.h"

#define LINUX_SOL_SOCKET 1

#define LINUX_SCM_RIGHTS 1

struct linux_sockaddr {
  uint16_t sa_family;
  char     sa_data[14];
};

struct linux_msghdr {
  void*         msg_name;
  socklen_t     msg_namelen;
  struct iovec* msg_iov;
  size_t        msg_iovlen;
  void*         msg_control;
  size_t        msg_controllen;
  int           msg_flags;
};

struct linux_cmsghdr {
  size_t cmsg_len;
  int    cmsg_level;
  int    cmsg_type;
  // unsigned char cmsg_data[];
};

void linux_to_native_sockaddr(struct sockaddr_un* dest, const struct linux_sockaddr* src, socklen_t addrlen) {

  assert(sizeof(*dest) >= addrlen);

  //memset(&addr, 0, sizeof(addr));

  dest->sun_len    = 0;
  dest->sun_family = src->sa_family;

  if (src->sa_data[0] == 0 /* abstract socket address */) {
    snprintf(dest->sun_path, sizeof(dest->sun_path), "/var/run/%s", &src->sa_data[1]);
  } else {
    strncpy(dest->sun_path, src->sa_data, sizeof(dest->sun_path));
  }
}

int shim_accept_impl(int s, struct sockaddr* restrict addr, socklen_t* restrict addrlen) {
  UNIMPLEMENTED();
}

int shim_bind_impl(int s, const struct linux_sockaddr* linux_addr, socklen_t addrlen) {

  assert(linux_addr->sa_family == 1);
  assert(linux_addr->sa_data[0] == 0 && str_starts_with(&linux_addr->sa_data[1], "nvidia"));

  struct sockaddr_un addr;

  linux_to_native_sockaddr(&addr, linux_addr, addrlen);

  int err = bind(s, (struct sockaddr*)&addr, sizeof(addr));
  if (err == 0) {
    unlink(addr.sun_path);
  } else {
    perror(__func__);
  }

  return err;
}

int shim_connect_impl(int s, const struct linux_sockaddr* linux_name, socklen_t namelen) {

  assert(linux_name->sa_family == 1);
  //assert(str_starts_with(&linux_name->sa_data[0], "/var/run/nvidia"));

  return -1; // ?

  /*struct sockaddr_un name;
  linux_to_native_sockaddr(&name, linux_name, namelen);

  return connect(s, (struct sockaddr*)&name, sizeof(name));*/
}

int shim_getsockname_impl(int s, struct sockaddr* restrict name, socklen_t* restrict namelen) {
  UNIMPLEMENTED();
}

void linux_to_native_msghdr(struct msghdr* dest, struct cmsghdr* cdest, int cdestlen, const struct linux_msghdr* src) {

  dest->msg_name       = src->msg_name;
  dest->msg_namelen    = src->msg_namelen;
  dest->msg_iov        = src->msg_iov;
  dest->msg_iovlen     = src->msg_iovlen;
  dest->msg_controllen = src->msg_controllen;
  dest->msg_flags      = src->msg_flags;

  if (src->msg_controllen > 0) {

    assert(cdest != NULL);
    assert(cdestlen >= src->msg_controllen);

    struct linux_cmsghdr* csrc = (struct linux_cmsghdr*)src->msg_control;

    assert(csrc->cmsg_level == LINUX_SOL_SOCKET);
    assert(csrc->cmsg_type  == LINUX_SCM_RIGHTS);

    cdest->cmsg_len   = csrc->cmsg_len;
    cdest->cmsg_level = SOL_SOCKET;
    cdest->cmsg_type  = SCM_RIGHTS;

    memcpy((char*)cdest + sizeof(struct cmsghdr), (char*)csrc + sizeof(struct linux_cmsghdr), src->msg_controllen - sizeof(struct linux_cmsghdr));

    dest->msg_control = (struct cmsghdr*)cdest;
  } else {
    dest->msg_control = src->msg_control;
  }
}

void native_to_linux_msghdr(struct linux_msghdr* dest, const struct msghdr* src) {

  dest->msg_name       = src->msg_name;
  dest->msg_namelen    = src->msg_namelen;
  dest->msg_iov        = src->msg_iov;
  dest->msg_iovlen     = src->msg_iovlen;
  dest->msg_controllen = src->msg_controllen;
  dest->msg_flags      = src->msg_flags;

  if (src->msg_controllen > 0) {
    assert(0);
  } else {
    dest->msg_control = dest->msg_control;
  }
}

ssize_t shim_sendmsg_impl(int s, const struct linux_msghdr* linux_msg, int flags) {
  struct msghdr msg;
  char control[32];
  linux_to_native_msghdr(&msg, (struct cmsghdr*)control, sizeof(control), linux_msg);
  return sendmsg(s, &msg, flags);
}

ssize_t shim_recvmsg_impl(int s, struct linux_msghdr* linux_msg, int flags) {
  struct msghdr msg;
  int err = recvmsg(s, &msg, flags);
  if (err != -1) {
    native_to_linux_msghdr(linux_msg, &msg);
  }
  return err;
}
