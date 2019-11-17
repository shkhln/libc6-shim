#include <stdint.h>

#ifdef __i386__

struct linux_dirent {
  uint32_t  d_ino;
  uint32_t  d_off; //TODO: lseek?
  uint16_t  d_reclen;
  uint8_t   d_type;
  char      d_name[256];
};

struct linux_dirent64 {
  uint64_t d_ino;
  uint64_t d_off;
  uint16_t d_reclen;
  uint8_t  d_type;
  char d_name[256];
};

#endif

#ifdef __x86_64__

struct linux_dirent {
  uint64_t  d_ino;
  uint64_t  d_off;
  uint16_t  d_reclen;
  uint8_t   d_type;
  char      d_name[256];
};

struct linux_dirent64 {
  uint64_t  d_ino;
  uint64_t  d_off;
  uint16_t  d_reclen;
  uint8_t   d_type;
  char      d_name[256];
};

#endif

typedef struct linux_dirent   linux_dirent;
typedef struct linux_dirent64 linux_dirent64;
