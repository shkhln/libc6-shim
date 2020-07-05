#include <fcntl.h>

#define LINUX_F_GETFD  1
#define LINUX_F_SETFD  2
#define LINUX_F_GETFL  3
#define LINUX_F_SETFL  4
#define LINUX_F_SETOWN 6

#define LINUX_O_RDONLY    0x00000
#define LINUX_O_WRONLY    0x00001
#define LINUX_O_RDWR      0x00002
#define LINUX_O_CREAT     0x00040
#define LINUX_O_EXCL      0x00080
#define LINUX_O_TRUNC     0x00200
#define LINUX_O_NONBLOCK  0x00800
#define LINUX_O_DIRECTORY 0x10000
#define LINUX_O_CLOEXEC   0x80000
#define LINUX_O_TMPFILE   (0x400000 | LINUX_O_DIRECTORY)

#define KNOWN_LINUX_OPEN_FLAGS ( \
 LINUX_O_RDONLY    |             \
 LINUX_O_WRONLY    |             \
 LINUX_O_RDWR      |             \
 LINUX_O_CREAT     |             \
 LINUX_O_EXCL      |             \
 LINUX_O_TRUNC     |             \
 LINUX_O_NONBLOCK  |             \
 LINUX_O_DIRECTORY |             \
 LINUX_O_CLOEXEC   |             \
 LINUX_O_TMPFILE                 \
)
