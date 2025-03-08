#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#define LINUX_PF_UNSPEC  0
#define LINUX_PF_UNIX    1
#define LINUX_PF_INET    2
#define LINUX_PF_INET6  10
#define LINUX_PF_PACKET 17

#define LINUX_AF_UNSPEC LINUX_PF_UNSPEC
#define LINUX_AF_UNIX   LINUX_PF_UNIX
#define LINUX_AF_INET   LINUX_PF_INET
#define LINUX_AF_INET6  LINUX_PF_INET6
#define LINUX_AF_PACKET LINUX_PF_PACKET

#define LINUX_SOL_IP      0
#define LINUX_SOL_SOCKET  1
#define LINUX_SOL_TCP     6
#define LINUX_SOL_UDP    17
#define LINUX_SOL_IPV6   41

#define LINUX_SCM_RIGHTS 1

#define LINUX_SOCK_STREAM          1
#define LINUX_SOCK_DGRAM           2
#define LINUX_SOCK_RAW             3
#define LINUX_SOCK_RDM             4
#define LINUX_SOCK_SEQPACKET       5
#define LINUX_SOCK_NONBLOCK  0x00800
#define LINUX_SOCK_CLOEXEC   0x80000

#define KNOWN_LINUX_SOCKET_TYPES ( \
  LINUX_SOCK_STREAM    |           \
  LINUX_SOCK_DGRAM     |           \
  LINUX_SOCK_RAW       |           \
  LINUX_SOCK_RDM       |           \
  LINUX_SOCK_SEQPACKET |           \
  LINUX_SOCK_NONBLOCK  |           \
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
#define LINUX_SO_PASSCRED   16
#define LINUX_SO_RCVLOWAT   18
#define LINUX_SO_SNDLOWAT   19
#define LINUX_SO_RCVTIMEO   20
#define LINUX_SO_SNDTIMEO   21
#define LINUX_SO_TIMESTAMP  29
#define LINUX_SO_ACCEPTCONN 30
#define LINUX_SO_PROTOCOL   38

#define LINUX_IP_TOS            1
#define LINUX_IP_MTU_DISCOVER  10
#define LINUX_IP_RECVTOS       13

#define LINUX_IPV6_V6ONLY      26

#define LINUX_TCP_NODELAY       1
#define LINUX_TCP_KEEPIDLE      4
#define LINUX_TCP_KEEPINTVL     5
#define LINUX_TCP_USER_TIMEOUT 18

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

typedef struct in_addr  linux_in_addr;
typedef struct in6_addr linux_in6_addr;
typedef struct iovec    linux_iovec;

struct linux_sockaddr {
  uint16_t sa_family;
  char     sa_data[14];
};

struct linux_sockaddr_in {
  uint16_t      sin_family;
  uint16_t      sin_port;
  linux_in_addr sin_addr;
  uint8_t       sin_zero[8];
};

struct linux_sockaddr_in6 {
   uint16_t       sin6_family;
   uint16_t       sin6_port;
   uint32_t       sin6_flowinfo;
   linux_in6_addr sin6_addr;
   uint32_t       sin6_scope_id;
};

struct linux_sockaddr_un {
  uint16_t sun_family;
  char     sun_path[108];
};

struct linux_msghdr {
  void*        msg_name;
  socklen_t    msg_namelen;
  linux_iovec* msg_iov;
  size_t       msg_iovlen;
  void*        msg_control;
  size_t       msg_controllen;
  int          msg_flags;
};

struct linux_cmsghdr {
  size_t cmsg_len;
  int    cmsg_level;
  int    cmsg_type;
  // unsigned char cmsg_data[];
};

typedef struct linux_cmsghdr      linux_cmsghdr;
typedef struct linux_msghdr       linux_msghdr;
typedef struct linux_sockaddr     linux_sockaddr;
typedef struct linux_sockaddr_in  linux_sockaddr_in;
typedef struct linux_sockaddr_in6 linux_sockaddr_in6;
typedef struct linux_sockaddr_un  linux_sockaddr_un;

int linux_to_native_sock_type(int linux_type);
int native_to_linux_sock_type(int linux_type);

void linux_to_native_sockaddr_in(struct sockaddr_in* dest, const linux_sockaddr_in* src);
void native_to_linux_sockaddr_in(linux_sockaddr_in* dest, const struct sockaddr_in* src);

void linux_to_native_sockaddr_in6(struct sockaddr_in6* dest, const linux_sockaddr_in6* src);
void native_to_linux_sockaddr_in6(linux_sockaddr_in6* dest, const struct sockaddr_in6* src);

void linux_to_native_sockaddr_un(struct sockaddr_un* dest, const linux_sockaddr_un* src);
void native_to_linux_sockaddr_un(linux_sockaddr_un* dest, const struct sockaddr_un* src);
