#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "../../shim.h"
#include "socket.h"

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

_Static_assert(LINUX_SOCK_STREAM    == SOCK_STREAM,    "");
_Static_assert(LINUX_SOCK_DGRAM     == SOCK_DGRAM,     "");
_Static_assert(LINUX_SOCK_RAW       == SOCK_RAW,       "");
_Static_assert(LINUX_SOCK_RDM       == SOCK_RDM,       "");
_Static_assert(LINUX_SOCK_SEQPACKET == SOCK_SEQPACKET, "");

int linux_to_native_sock_type(int linux_type) {

  assert((linux_type & KNOWN_LINUX_SOCKET_TYPES) == linux_type);

  int type = linux_type & 0xFF;

  if (linux_type & LINUX_SOCK_NONBLOCK) type |= SOCK_NONBLOCK;
  if (linux_type & LINUX_SOCK_CLOEXEC)  type |= SOCK_CLOEXEC;

  return type;
}

int native_to_linux_sock_type(int type) {

  assert((type & (SOCK_STREAM | SOCK_DGRAM | SOCK_RAW | SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC)) == type);

  _Static_assert(SOCK_NONBLOCK > 0xFF, "");
  _Static_assert(SOCK_CLOEXEC  > 0xFF, "");

  int linux_type = type & 0xFF;

  if (type & SOCK_NONBLOCK) linux_type |= LINUX_SOCK_NONBLOCK;
  if (type & SOCK_CLOEXEC)  linux_type |= LINUX_SOCK_CLOEXEC;

  return linux_type;
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

void linux_to_native_sockaddr_in(struct sockaddr_in* dest, const linux_sockaddr_in* src) {

  dest->sin_len    = 0;
  dest->sin_family = PF_INET;
  dest->sin_port   = src->sin_port;
  dest->sin_addr   = src->sin_addr;

  memcpy(dest->sin_zero, src->sin_zero, sizeof(dest->sin_zero));
}

void linux_to_native_sockaddr_in6(struct sockaddr_in6* dest, const linux_sockaddr_in6* src) {
  dest->sin6_len      = 0;
  dest->sin6_family   = PF_INET6;
  dest->sin6_port     = src->sin6_port;
  dest->sin6_flowinfo = src->sin6_flowinfo;
  memcpy(dest->sin6_addr.s6_addr, src->sin6_addr.s6_addr, sizeof(dest->sin6_addr.s6_addr));
  dest->sin6_scope_id = src->sin6_scope_id;
}

void linux_to_native_sockaddr_un(struct sockaddr_un* dest, const linux_sockaddr_un* src) {

  dest->sun_len    = 0;
  dest->sun_family = PF_UNIX;

  if (src->sun_path[0] == 0 /* abstract socket address */) {
    snprintf(dest->sun_path, sizeof(dest->sun_path), "/var/run/%s", &src->sun_path[1]);
  } else {
    size_t nbytes = strlcpy(dest->sun_path, src->sun_path, sizeof(dest->sun_path));
    assert(nbytes < sizeof(dest->sun_path));
  }
}

void native_to_linux_sockaddr_in(linux_sockaddr_in* dest, const struct sockaddr_in* src) {
  dest->sin_family = LINUX_PF_INET;
  dest->sin_port   = src->sin_port;
  dest->sin_addr   = src->sin_addr;
  memcpy(dest->sin_zero, src->sin_zero, sizeof(dest->sin_zero));
}

void native_to_linux_sockaddr_in6(linux_sockaddr_in6* dest, const struct sockaddr_in6* src) {
  dest->sin6_family   = LINUX_PF_INET6;
  dest->sin6_port     = src->sin6_port;
  dest->sin6_flowinfo = src->sin6_flowinfo;
  memcpy(dest->sin6_addr.s6_addr, src->sin6_addr.s6_addr, sizeof(dest->sin6_addr.s6_addr));
  dest->sin6_scope_id = src->sin6_scope_id;
}

void native_to_linux_sockaddr_un(linux_sockaddr_un* dest, const struct sockaddr_un* src) {
  dest->sun_family = LINUX_PF_UNIX;
  size_t nbytes = strlcpy(dest->sun_path, src->sun_path, sizeof(dest->sun_path));
  assert(nbytes < sizeof(dest->sun_path));
}

static int linux_to_native_domain(int domain) {
  switch (domain) {
    case LINUX_PF_UNIX:  return PF_UNIX;
    case LINUX_PF_INET:  return PF_INET;
    case LINUX_PF_INET6: return PF_INET6;
    default:
      assert(0);
  }
}

int shim_socket_impl(int domain, int type, int protocol) {
  return socket(linux_to_native_domain(domain), linux_to_native_sock_type(type), protocol);
}

int shim_socketpair_impl(int domain, int type, int protocol, int* sv) {
  return socketpair(linux_to_native_domain(domain), linux_to_native_sock_type(type), protocol, sv);
}

int shim_bind_impl(int s, const linux_sockaddr* linux_addr, socklen_t addrlen) {

  switch (linux_addr->sa_family) {

    case LINUX_PF_UNIX:
      {
        struct sockaddr_un addr;
        assert(addrlen <= sizeof(struct linux_sockaddr_un));
        linux_to_native_sockaddr_un(&addr, (linux_sockaddr_un*)linux_addr);

        int err = bind(s, (struct sockaddr*)&addr, sizeof(addr));
        if (err == 0) {
          // unlink(addr.sun_path); ?
        }

        return err;
      }

    case LINUX_PF_INET:
      {
        struct sockaddr_in addr;
        assert(addrlen == sizeof(struct linux_sockaddr_in));
        linux_to_native_sockaddr_in(&addr, (linux_sockaddr_in*)linux_addr);
        return bind(s, (struct sockaddr*)&addr, sizeof(addr));
      }

    case LINUX_PF_INET6:
      {
        struct sockaddr_in6 addr;
        assert(addrlen == sizeof(struct linux_sockaddr_in6));
        linux_to_native_sockaddr_in6(&addr, (linux_sockaddr_in6*)linux_addr);
        return bind(s, (struct sockaddr*)&addr, sizeof(addr));
      }

    default:
      assert(0);
  }
}

int shim_connect_impl(int s, const linux_sockaddr* linux_name, socklen_t namelen) {

  switch (linux_name->sa_family) {

    case LINUX_PF_UNIX:
      {
        struct sockaddr_un addr;
        assert(namelen <= sizeof(struct linux_sockaddr_un));
        linux_to_native_sockaddr_un(&addr, (linux_sockaddr_un*)linux_name);
        LOG("%s: path = %s", __func__, addr.sun_path);
        return connect(s, (struct sockaddr*)&addr, sizeof(addr));
      }

    case LINUX_PF_INET:
      {
        struct sockaddr_in addr;
        assert(namelen == sizeof(struct linux_sockaddr_in));
        linux_to_native_sockaddr_in(&addr, (linux_sockaddr_in*)linux_name);
        return connect(s, (struct sockaddr*)&addr, sizeof(addr));
      }

    case LINUX_PF_INET6:
      {
        struct sockaddr_in6 addr;
        assert(namelen == sizeof(struct linux_sockaddr_in6));
        linux_to_native_sockaddr_in6(&addr, (linux_sockaddr_in6*)linux_name);
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
#elif defined(__i386__)
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
#elif defined(__i386__)
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

ssize_t shim_recv_impl(int s, void* buf, size_t len, int linux_flags) {
  ssize_t nbytes = recv(s, buf, len, linux_to_native_msg_flags(linux_flags));
  if (nbytes == -1) {
    errno = native_to_linux_errno(errno);
  }
  return nbytes;
}

ssize_t shim_send_impl(int s, const void* msg, size_t len, int linux_flags) {
  ssize_t nbytes = send(s, msg, len, linux_to_native_msg_flags(linux_flags));
  if (nbytes == -1) {
    errno = native_to_linux_errno(errno);
  }
  return nbytes;
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

  ssize_t nbytes = recvmsg(s, &msg, linux_to_native_msg_flags(linux_flags));
  if (nbytes != -1) {
    native_to_linux_msghdr(linux_msg, &msg);
  } else {
    errno = native_to_linux_errno(errno);
  }

  return nbytes;
}

ssize_t shim_sendmsg_impl(int s, const struct linux_msghdr* linux_msg, int linux_flags) {

  struct msghdr msg;
  uint8_t buf[linux_msg->msg_controllen];

  msg.msg_control    = &buf;
  msg.msg_controllen = sizeof(buf);

  linux_to_native_msghdr(&msg, linux_msg);

  ssize_t nbytes = sendmsg(s, &msg, linux_to_native_msg_flags(linux_flags));
  if (nbytes == -1) {
    errno = native_to_linux_errno(errno);
  }

  return nbytes;
}

ssize_t shim_recvfrom_impl(int s, void* buf, size_t len, int linux_flags, linux_sockaddr* restrict linux_from, socklen_t* restrict linux_fromlen) {

  ssize_t nbytes;
  if (linux_from != NULL) {

    uint8_t   from[110]; // ?
    socklen_t fromlen = sizeof(from);

    nbytes = recvfrom(s, buf, len, linux_to_native_msg_flags(linux_flags), (struct sockaddr*)&from, &fromlen);
    if (nbytes != -1) {
      switch (((struct sockaddr*)&from)->sa_family) {
        case PF_UNIX:
          assert(*linux_fromlen >= sizeof(struct linux_sockaddr_un));
          native_to_linux_sockaddr_un((linux_sockaddr_un*)linux_from, (struct sockaddr_un*)&from);
          break;
        case PF_INET:
          assert(*linux_fromlen >= sizeof(struct linux_sockaddr_in));
          native_to_linux_sockaddr_in((linux_sockaddr_in*)linux_from, (struct sockaddr_in*)&from);
          break;
        case PF_INET6:
          assert(*linux_fromlen >= sizeof(struct linux_sockaddr_in6));
          native_to_linux_sockaddr_in6((linux_sockaddr_in6*)linux_from, (struct sockaddr_in6*)&from);
          break;
        default:
          assert(0);
      }
    }

  } else {
    nbytes = recvfrom(s, buf, len, linux_to_native_msg_flags(linux_flags), NULL, linux_fromlen);
  }

  if (nbytes == -1) {
    errno = native_to_linux_errno(errno);
  }

  return nbytes;
}

ssize_t shim_sendto_impl(int s, const void* msg, size_t len, int linux_flags, const linux_sockaddr* linux_to, socklen_t tolen) {

  ssize_t nbytes;
  switch (linux_to->sa_family) {

    case LINUX_PF_UNIX:
    {
      struct sockaddr_un to;
      assert(tolen <= sizeof(struct linux_sockaddr_un));
      linux_to_native_sockaddr_un(&to, (linux_sockaddr_un*)linux_to);
      nbytes = sendto(s, msg, len, linux_to_native_msg_flags(linux_flags), (struct sockaddr*)&to, sizeof(to));
    }
    break;

    case LINUX_PF_INET:
    {
      struct sockaddr_in to;
      assert(tolen == sizeof(struct linux_sockaddr_in));
      linux_to_native_sockaddr_in(&to, (linux_sockaddr_in*)linux_to);
      nbytes = sendto(s, msg, len, linux_to_native_msg_flags(linux_flags), (struct sockaddr*)&to, sizeof(to));
    }
    break;

    case LINUX_PF_INET6:
    {
      struct sockaddr_in6 to;
      assert(tolen == sizeof(struct linux_sockaddr_in6));
      linux_to_native_sockaddr_in6(&to, (linux_sockaddr_in6*)linux_to);
      nbytes = sendto(s, msg, len, linux_to_native_msg_flags(linux_flags), (struct sockaddr*)&to, sizeof(to));
    }
    break;

    default:
      assert(0);
  }

  if (nbytes == -1) {
    errno = native_to_linux_errno(errno);
  }

  return nbytes;
}

SHIM_WRAP(bind);
SHIM_WRAP(connect);
SHIM_WRAP(recv);
SHIM_WRAP(send);
SHIM_WRAP(recvmsg);
SHIM_WRAP(sendmsg);
SHIM_WRAP(recvfrom);
SHIM_WRAP(sendto);
SHIM_WRAP(socket);
SHIM_WRAP(socketpair);

ssize_t shim___recv_chk_impl(int fd, void* buf, size_t len, size_t buflen, int flags) {
  assert(len <= buflen);
  return recv(fd, buf, len, flags);
}

SHIM_WRAP(__recv_chk);

static int linux_to_native_so_opt(int optname) {
  switch (optname) {
    case LINUX_SO_BROADCAST: return SO_BROADCAST;
    case LINUX_SO_SNDBUF:    return SO_SNDBUF;
    case LINUX_SO_RCVBUF:    return SO_RCVBUF;
    case LINUX_SO_KEEPALIVE: return SO_KEEPALIVE;
    default:
      assert(0);
  }
}

static int linux_to_native_tcp_opt(int optname) {
  switch (optname) {
    case LINUX_TCP_NODELAY:      return TCP_NODELAY;
    case LINUX_TCP_USER_TIMEOUT: return -1; // ?
    default:
      assert(0);
  }
}

static int linux_to_native_ip6_opt(int optname) {
  switch (optname) {
    case LINUX_IPV6_V6ONLY: return IPV6_V6ONLY;
    default:
      assert(0);
  }
}

int shim_getsockopt_impl(int s, int linux_level, int linux_optname, void* restrict optval, socklen_t* restrict optlen) {
  switch (linux_level) {
    case LINUX_SOL_SOCKET: return getsockopt(s, SOL_SOCKET,  linux_to_native_so_opt (linux_optname), optval, optlen);
    case LINUX_SOL_TCP:    return getsockopt(s, IPPROTO_TCP, linux_to_native_tcp_opt(linux_optname), optval, optlen);
    case LINUX_SOL_IPV6:   return getsockopt(s, IPPROTO_TCP, linux_to_native_ip6_opt(linux_optname), optval, optlen);
    default:
      assert(0);
  }
}

int shim_setsockopt_impl(int s, int linux_level, int linux_optname, const void* optval, socklen_t optlen) {
  int err;
  switch (linux_level) {
    case LINUX_SOL_SOCKET:
      if (linux_optname == LINUX_SO_SNDBUF && optval && *((int*)optval) == 0) {
        err = 0; // ?
      } else {
        err = setsockopt(s, SOL_SOCKET,  linux_to_native_so_opt(linux_optname), optval, optlen);
      }
      break;
    case LINUX_SOL_TCP:
      err = setsockopt(s, IPPROTO_TCP, linux_to_native_tcp_opt(linux_optname), optval, optlen);
      break;
    case LINUX_SOL_IPV6:
      err = setsockopt(s, IPPROTO_TCP, linux_to_native_ip6_opt(linux_optname), optval, optlen);
      break;
    default:
      assert(0);
  }
  return err;
}

SHIM_WRAP(getsockopt);
SHIM_WRAP(setsockopt);

int shim_getsockname_impl(int s, linux_sockaddr* restrict linux_name, socklen_t* restrict linux_namelen) {

  uint8_t   name[110]; // ?
  socklen_t namelen = sizeof(name);

  int err = getsockname(s, (struct sockaddr*)&name, &namelen);
  if (err != -1) {
    switch (((struct sockaddr*)&name)->sa_family) {
      case PF_UNIX:
        assert(*linux_namelen >= sizeof(struct linux_sockaddr_un));
        native_to_linux_sockaddr_un((linux_sockaddr_un*)linux_name, (struct sockaddr_un*)&name);
        break;
      case PF_INET:
        assert(*linux_namelen >= sizeof(struct linux_sockaddr_in));
        native_to_linux_sockaddr_in((linux_sockaddr_in*)linux_name, (struct sockaddr_in*)&name);
        break;
      case PF_INET6:
        assert(*linux_namelen >= sizeof(struct linux_sockaddr_in6));
        native_to_linux_sockaddr_in6((linux_sockaddr_in6*)linux_name, (struct sockaddr_in6*)&name);
        break;
      default:
        assert(0);
    }
  }

  return err;
}

SHIM_WRAP(getsockname);

const char* shim_inet_ntop_impl(int af, const void* restrict src, char* restrict dst, socklen_t size) {
  assert(af == LINUX_PF_INET || af == LINUX_PF_INET6);
  return inet_ntop(linux_to_native_domain(af), src, dst, size);
}

int shim_inet_pton_impl(int af, const char* restrict src, void* restrict dst) {
  assert(af == LINUX_PF_INET || af == LINUX_PF_INET6);
  return inet_pton(linux_to_native_domain(af), src, dst);
}

#ifdef SHIM_SCAN
#undef inet_ntop
#undef inet_pton
SHIM_WRAP(inet_ntop);
SHIM_WRAP(inet_pton);
#else
SHIM_WRAPPER_inet_ntop
SHIM_WRAPPER_inet_pton
#endif
