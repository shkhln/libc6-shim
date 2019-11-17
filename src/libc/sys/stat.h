#include <stdint.h>
#include "../time.h"

#ifdef __i386__

struct linux_stat {
  uint64_t              st_dev;
  uint8_t               _pad1[4];
  uint32_t              st_ino;
  uint32_t              st_mode;
  uint32_t              st_nlink;
  uint32_t              st_uid;
  uint32_t              st_gid;
  uint64_t              st_rdev;
  uint8_t               _pad2[4];
  uint32_t              st_size;
  uint32_t              st_blksize;
  uint32_t              st_blocks;
  linux_timespec st_atim;
  linux_timespec st_mtim;
  linux_timespec st_ctim;
  uint8_t               _pad3[8];
};

#endif

#ifdef __x86_64__

struct linux_stat {
  uint64_t              st_dev;
  uint64_t              st_ino;
  uint64_t              st_nlink;
  uint32_t              st_mode;
  uint32_t              st_uid;
  uint32_t              st_gid;
  uint8_t               _pad1[4];
  uint64_t              st_rdev;
  uint64_t              st_size;
  uint64_t              st_blksize;
  uint64_t              st_blocks;
  linux_timespec st_atim;
  linux_timespec st_mtim;
  linux_timespec st_ctim;
  uint8_t               _pad2[24];
};

#endif

typedef struct linux_stat linux_stat;
