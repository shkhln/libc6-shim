#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "../../shim.h"

#define LINUX_SOL_IP      0
#define LINUX_SOL_SOCKET  1
#define LINUX_SOL_TCP     6
#define LINUX_SOL_UDP    17

#define LINUX_SCM_RIGHTS 1

#define LINUX_SOCK_STREAM   0x00001
#define LINUX_SOCK_DGRAM    0x00002
#define LINUX_SOCK_NONBLOCK 0x00800
#define LINUX_SOCK_CLOEXEC  0x80000

#define KNOWN_LINUX_SOCKET_TYPES ( \
  LINUX_SOCK_STREAM   |            \
  LINUX_SOCK_DGRAM    |            \
  LINUX_SOCK_NONBLOCK |            \
  LINUX_SOCK_CLOEXEC               \
)

#define LINUX_SO_DEBUG       1
#define LINUX_SO_REUSEADDR   2
#define LINUX_SO_TYPE        3
#define LINUX_SO_ERROR       4
#define LINUX_SO_DONTROUTE   5
#define LINUX_SO_BROADCAST   6
#define LINUX_SO_SNDBUF      7
#define LINUX_SO_RCVBUF      8
#define LINUX_SO_KEEPALIVE   9
#define LINUX_SO_OOBINLINE  10
#define LINUX_SO_LINGER     13
#define LINUX_SO_REUSEPORT  15
#define LINUX_SO_RCVLOWAT   18
#define LINUX_SO_SNDLOWAT   19
#define LINUX_SO_RCVTIMEO   20
#define LINUX_SO_SNDTIMEO   21
#define LINUX_SO_TIMESTAMP  29
#define LINUX_SO_ACCEPTCONN 30
#define LINUX_SO_PROTOCOL   38

#define LINUX_MSG_OOB          0x00000001
#define LINUX_MSG_PEEK         0x00000002
#define LINUX_MSG_DONTROUTE    0x00000004
#define LINUX_MSG_CTRUNC       0x00000008
#define LINUX_MSG_TRUNC        0x00000020
#define LINUX_MSG_DONTWAIT     0x00000040
#define LINUX_MSG_EOR          0x00000080
#define LINUX_MSG_WAITALL      0x00000100
#define LINUX_MSG_NOSIGNAL     0x00004000
#define LINUX_MSG_WAITFORONE   0x00010000
#define LINUX_MSG_CMSG_CLOEXEC 0x40000000

#define KNOWN_LINUX_MSG_FLAGS ( \
  LINUX_MSG_OOB          |      \
  LINUX_MSG_PEEK         |      \
  LINUX_MSG_DONTROUTE    |      \
  LINUX_MSG_CTRUNC       |      \
  LINUX_MSG_TRUNC        |      \
  LINUX_MSG_DONTWAIT     |      \
  LINUX_MSG_EOR          |      \
  LINUX_MSG_WAITALL      |      \
  LINUX_MSG_NOSIGNAL     |      \
  LINUX_MSG_WAITFORONE   |      \
  LINUX_MSG_CMSG_CLOEXEC        \
)

#define KNOWN_NATIVE_MSG_FLAGS ( \
  MSG_OOB          |             \
  MSG_PEEK         |             \
  MSG_DONTROUTE    |             \
  MSG_CTRUNC       |             \
  MSG_TRUNC        |             \
  MSG_DONTWAIT     |             \
  MSG_EOR          |             \
  MSG_EOF          |             \
  MSG_WAITALL      |             \
  MSG_NOSIGNAL     |             \
  MSG_WAITFORONE   |             \
  MSG_CMSG_CLOEXEC               \
)

struct linux_sockaddr {
  uint16_t sa_family;
  char     sa_data[14];
};

struct linux_sockaddr_in {
  uint16_t       sin_family;
  uint16_t       sin_port;
  struct in_addr sin_addr;
  uint8_t        sin_zero[8];
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

static int linux_to_native_sock_level(int level) {
  switch (level) {
    case LINUX_SOL_SOCKET: return SOL_SOCKET;
    case LINUX_SOL_IP:     return IPPROTO_IP;
    case LINUX_SOL_TCP:    return IPPROTO_TCP;
    case LINUX_SOL_UDP:    return IPPROTO_UDP;
    default:
      assert(0);
  }
}

static int native_to_linux_sock_level(int level) {
  switch (level) {
    case SOL_SOCKET:  return LINUX_SOL_SOCKET;
    case IPPROTO_IP:  return LINUX_SOL_IP;
    case IPPROTO_TCP: return LINUX_SOL_TCP;
    case IPPROTO_UDP: return LINUX_SOL_UDP;
    default:
      assert(0);
  }
}

static int linux_to_native_sock_type(int linux_type) {

  assert((linux_type & KNOWN_LINUX_SOCKET_TYPES) == linux_type);

  int type = 0;

  if (linux_type & LINUX_SOCK_STREAM)   type |= SOCK_STREAM;
  if (linux_type & LINUX_SOCK_DGRAM)    type |= SOCK_DGRAM;
  if (linux_type & LINUX_SOCK_NONBLOCK) type |= SOCK_NONBLOCK;
  if (linux_type & LINUX_SOCK_CLOEXEC)  type |= SOCK_CLOEXEC;

  return type;
}

static int linux_to_native_msg_flags(int linux_flags) {

  assert((linux_flags & KNOWN_LINUX_MSG_FLAGS) == linux_flags);

  int flags = 0;

  if (linux_flags & LINUX_MSG_OOB)          flags |= MSG_OOB;
  if (linux_flags & LINUX_MSG_PEEK)         flags |= MSG_PEEK;
  if (linux_flags & LINUX_MSG_DONTROUTE)    flags |= MSG_DONTROUTE;
  if (linux_flags & LINUX_MSG_CTRUNC)       flags |= MSG_CTRUNC;
  if (linux_flags & LINUX_MSG_TRUNC)        flags |= MSG_TRUNC;
  if (linux_flags & LINUX_MSG_DONTWAIT)     flags |= MSG_DONTWAIT;
  if (linux_flags & LINUX_MSG_EOR)          flags |= MSG_EOR;
  if (linux_flags & LINUX_MSG_WAITALL)      flags |= MSG_WAITALL;
  if (linux_flags & LINUX_MSG_NOSIGNAL)     flags |= MSG_NOSIGNAL;
  if (linux_flags & LINUX_MSG_WAITFORONE)   flags |= MSG_WAITFORONE;
  if (linux_flags & LINUX_MSG_CMSG_CLOEXEC) flags |= MSG_CMSG_CLOEXEC;

  return flags;
}

static int native_to_linux_msg_flags(int flags) {

  assert((flags & KNOWN_NATIVE_MSG_FLAGS) == flags);

  int linux_flags = 0;

  if (flags & MSG_EOF) {
    assert(0);
  }

  if (flags & MSG_OOB)          linux_flags |= LINUX_MSG_OOB;
  if (flags & MSG_PEEK)         linux_flags |= LINUX_MSG_PEEK;
  if (flags & MSG_DONTROUTE)    linux_flags |= LINUX_MSG_DONTROUTE;
  if (flags & MSG_CTRUNC)       linux_flags |= LINUX_MSG_CTRUNC;
  if (flags & MSG_TRUNC)        linux_flags |= LINUX_MSG_TRUNC;
  if (flags & MSG_DONTWAIT)     linux_flags |= LINUX_MSG_DONTWAIT;
  if (flags & MSG_EOR)          linux_flags |= LINUX_MSG_EOR;
  if (flags & MSG_WAITALL)      linux_flags |= LINUX_MSG_WAITALL;
  if (flags & MSG_NOSIGNAL)     linux_flags |= LINUX_MSG_NOSIGNAL;
  if (flags & MSG_WAITFORONE)   linux_flags |= LINUX_MSG_WAITFORONE;
  if (flags & MSG_CMSG_CLOEXEC) linux_flags |= LINUX_MSG_CMSG_CLOEXEC;

  return linux_flags;
}

static void linux_to_native_sockaddr(struct sockaddr* dest, const struct linux_sockaddr* src, socklen_t addrlen) {

  switch (src->sa_family) {

    case PF_UNIX:
      {
        assert(addrlen <= sizeof(struct sockaddr_un));

        struct sockaddr_un* d = (struct sockaddr_un*)dest;
        memset(d, 0, sizeof(struct sockaddr_un));

        d->sun_len    = 0;
        d->sun_family = src->sa_family;

        if (src->sa_data[0] == 0 /* abstract socket address */) {
          snprintf(d->sun_path, sizeof(d->sun_path), "/var/run/%s", &src->sa_data[1]);
        } else {
          strlcpy(d->sun_path, src->sa_data, sizeof(d->sun_path));
        }
      }

      break;

    case PF_INET:
      {
        assert(addrlen <= sizeof(struct sockaddr_in));

        struct sockaddr_in* d = (struct sockaddr_in*)dest;
        memset(d, 0, sizeof(struct sockaddr_in));

        struct linux_sockaddr_in* s = (struct linux_sockaddr_in*)src;

        d->sin_len    = 0;
        d->sin_family = s->sin_family;
        d->sin_port   = s->sin_port;
        d->sin_addr   = s->sin_addr;

        memcpy(d->sin_zero, s->sin_zero, sizeof(d->sin_zero));
      }

      break;

    default:
      assert(0);
  }
}

int shim_socket_impl(int domain, int type, int protocol) {
  assert(domain == PF_UNIX || domain == PF_INET);
  return socket(domain, linux_to_native_sock_type(type), protocol);
}

int shim_bind_impl(int s, const struct linux_sockaddr* linux_addr, socklen_t addrlen) {

  switch (linux_addr->sa_family) {

    case PF_UNIX:
      {
        struct sockaddr_un addr;
        linux_to_native_sockaddr((struct sockaddr*)&addr, linux_addr, addrlen);

        int err = bind(s, (struct sockaddr*)&addr, sizeof(addr));
        if (err == 0) {
          // unlink(addr.sun_path); ?
        }

        return err;
      }

    case PF_INET:
      {
        struct sockaddr_in addr;
        linux_to_native_sockaddr((struct sockaddr*)&addr, linux_addr, addrlen);
        return bind(s, (struct sockaddr*)&addr, sizeof(addr));
      }

    default:
      assert(0);
  }
}

int shim_connect_impl(int s, const struct linux_sockaddr* linux_name, socklen_t namelen) {

  switch (linux_name->sa_family) {

    case PF_UNIX:
      {
        struct sockaddr_un addr;
        linux_to_native_sockaddr((struct sockaddr*)&addr, linux_name, namelen);
        LOG("%s: path = %s\n", __func__, addr.sun_path);
        return connect(s, (struct sockaddr*)&addr, sizeof(addr));
      }

    case PF_INET:
      {
        struct sockaddr_in addr;
        linux_to_native_sockaddr((struct sockaddr*)&addr, linux_name, namelen);
        return connect(s, (struct sockaddr*)&addr, sizeof(addr));
      }

    default:
      assert(0);
  }
}

static void linux_to_native_msghdr(struct msghdr* msg, const struct linux_msghdr* linux_msg) {

  msg->msg_name    = linux_msg->msg_name;
  msg->msg_namelen = linux_msg->msg_namelen;
  msg->msg_iov     = linux_msg->msg_iov;
  msg->msg_iovlen  = linux_msg->msg_iovlen;
  msg->msg_flags   = linux_to_native_msg_flags(linux_msg->msg_flags);

  if (linux_msg->msg_controllen > 0) {

    assert(msg->msg_controllen >= linux_msg->msg_controllen);
    msg->msg_controllen = linux_msg->msg_controllen;

    memset(msg->msg_control, 0, linux_msg->msg_controllen);

    struct linux_cmsghdr* linux_cmsg = (struct linux_cmsghdr*)CMSG_FIRSTHDR(linux_msg);
    while (linux_cmsg != NULL) {
      struct cmsghdr* cmsg = (struct cmsghdr*)((uint8_t*)msg->msg_control + ((uint64_t)linux_cmsg - (uint64_t)linux_msg->msg_control));

      assert(linux_cmsg->cmsg_type == LINUX_SCM_RIGHTS);

      cmsg->cmsg_len   = linux_cmsg->cmsg_len;
      cmsg->cmsg_level = linux_to_native_sock_level(linux_cmsg->cmsg_level);
      cmsg->cmsg_type  = SCM_RIGHTS;

#ifdef __x86_64__
      memcpy((uint8_t*)cmsg + 16, (uint8_t*)linux_cmsg + 16, linux_cmsg->cmsg_len - 16);
#elif  __i386__
      memcpy((uint8_t*)cmsg + 12, (uint8_t*)linux_cmsg + 12, linux_cmsg->cmsg_len - 12);
#else
  #error
#endif

      linux_cmsg = (struct linux_cmsghdr*)CMSG_NXTHDR(linux_msg, linux_cmsg);
    }
  } else {
    msg->msg_control    = NULL;
    msg->msg_controllen = 0;
  }
}

static void native_to_linux_msghdr(struct linux_msghdr* linux_msg, const struct msghdr* msg) {

  linux_msg->msg_name    = msg->msg_name;
  linux_msg->msg_namelen = msg->msg_namelen;
  linux_msg->msg_iov     = msg->msg_iov;
  linux_msg->msg_iovlen  = msg->msg_iovlen;
  linux_msg->msg_flags   = native_to_linux_msg_flags(msg->msg_flags);

  if (msg->msg_controllen > 0) {

    assert(linux_msg->msg_controllen >= msg->msg_controllen);
    linux_msg->msg_controllen = msg->msg_controllen;

    memset(linux_msg->msg_control, 0, msg->msg_controllen);

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(msg);
    while (cmsg != NULL) {
      struct linux_cmsghdr* linux_cmsg = (struct linux_cmsghdr*)((uint8_t*)linux_msg->msg_control + ((uint64_t)cmsg - (uint64_t)msg->msg_control));

      assert(cmsg->cmsg_type == SCM_RIGHTS);

      linux_cmsg->cmsg_len   = cmsg->cmsg_len;
      linux_cmsg->cmsg_level = native_to_linux_sock_level(cmsg->cmsg_level);
      linux_cmsg->cmsg_type  = LINUX_SCM_RIGHTS;

#ifdef __x86_64__
      memcpy((uint8_t*)linux_cmsg + 16, (uint8_t*)cmsg + 16, cmsg->cmsg_len - 16);
#elif  __i386__
      memcpy((uint8_t*)linux_cmsg + 12, (uint8_t*)cmsg + 12, cmsg->cmsg_len - 12);
#else
  #error
#endif

      cmsg = CMSG_NXTHDR(msg, cmsg);
    }

  } else {
    linux_msg->msg_control    = NULL;
    linux_msg->msg_controllen = 0;
  }
}

ssize_t shim_sendmsg_impl(int s, const struct linux_msghdr* linux_msg, int linux_flags) {

  struct msghdr msg;
  uint8_t buf[linux_msg->msg_controllen];

  msg.msg_control    = &buf;
  msg.msg_controllen = sizeof(buf);

  linux_to_native_msghdr(&msg, linux_msg);

  return sendmsg(s, &msg, linux_to_native_msg_flags(linux_flags));
}

ssize_t shim_recvmsg_impl(int s, struct linux_msghdr* linux_msg, int linux_flags) {

  struct msghdr msg;
  uint8_t buf[linux_msg->msg_controllen];

  msg.msg_name       = linux_msg->msg_name;
  msg.msg_namelen    = linux_msg->msg_namelen;
  msg.msg_iov        = linux_msg->msg_iov;
  msg.msg_iovlen     = linux_msg->msg_iovlen;
  msg.msg_control    = &buf;
  msg.msg_controllen = sizeof(buf);
  msg.msg_flags      = linux_to_native_msg_flags(linux_msg->msg_flags);

  int err = recvmsg(s, &msg, linux_to_native_msg_flags(linux_flags));
  if (err != -1) {
    native_to_linux_msghdr(linux_msg, &msg);
  }

  return err;
}
