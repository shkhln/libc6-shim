#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "../../shim.h"

#ifdef __i386__

struct linux_timespec {
  uint32_t tv_sec;
  uint32_t tv_nsec;
};

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
  uint32_t              st_size ;
  uint32_t              st_blksize;
  uint32_t              st_blocks;
  struct linux_timespec st_atim;
  struct linux_timespec st_mtim;
  struct linux_timespec st_ctim;
  uint8_t               _pad3[8];
};

#endif

#ifdef __x86_64__

struct linux_timespec {
  uint64_t tv_sec;
  uint64_t tv_nsec;
};

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
  struct linux_timespec st_atim;
  struct linux_timespec st_mtim;
  struct linux_timespec st_ctim;
  uint8_t               _pad2[24];
};

#endif

enum DEV_FILE_TYPE {
  DEV_NV,
  DEV_NV_CTL,
  DEV_NV_MODESET,
  DEV_OTHER
};

enum DEV_FILE_TYPE path_to_dev_type(const char* path) {

  if (strcmp(path, "/dev/nvidia0") == 0) {
    return DEV_NV;
  }

  if (strcmp(path, "/dev/nvidiactl") == 0) {
    return DEV_NV_CTL;
  }

  if (strcmp(path, "/dev/nvidia-modeset") == 0) {
    return DEV_NV_MODESET;
  }

  return DEV_OTHER;
}

uint64_t linux_makedev(uint32_t major, uint32_t minor) {
  return
    (((uint64_t)(major & 0x00000fffu)) <<  8) |
    (((uint64_t)(major & 0xfffff000u)) << 32) |
    (((uint64_t)(minor & 0x000000ffu)) <<  0) |
    (((uint64_t)(minor & 0xffffff00u)) << 12);
}

void copy_stat_buf(struct linux_stat* dst, struct stat* src) {

  memset(dst, 0, sizeof(struct linux_stat));

  dst->st_dev          = src->st_dev;
  dst->st_ino          = src->st_ino;
  dst->st_nlink        = src->st_nlink;
  dst->st_mode         = src->st_mode;
  dst->st_uid          = src->st_uid;
  dst->st_gid          = src->st_gid;
  dst->st_rdev         = src->st_rdev;
  dst->st_size         = src->st_size;
  dst->st_blksize      = src->st_blksize;
  dst->st_blocks       = src->st_blocks;
  dst->st_atim.tv_sec  = src->st_atim.tv_sec;
  dst->st_atim.tv_nsec = src->st_atim.tv_nsec;
  dst->st_mtim.tv_sec  = src->st_mtim.tv_sec;
  dst->st_mtim.tv_nsec = src->st_mtim.tv_nsec;
  dst->st_ctim.tv_sec  = src->st_ctim.tv_sec;
  dst->st_ctim.tv_nsec = src->st_ctim.tv_nsec;
}

int shim___xstat_impl(int ver, const char* path, struct linux_stat* stat_buf) {

  struct stat sb;

  int err = stat(path, &sb);
  if (err == 0) {

    copy_stat_buf(stat_buf, &sb);

    switch (path_to_dev_type(path)) {

      case DEV_NV:
        stat_buf->st_rdev = linux_makedev(195, 0);
        break;

      case DEV_NV_CTL:
        stat_buf->st_rdev = linux_makedev(195, 255);
        break;

      case DEV_NV_MODESET:
        stat_buf->st_rdev = linux_makedev(195, 254);
        break;

      default: ; // do nothing
    }
  }

  return err;
}

int shim___fxstat_impl(int ver, int fd, struct linux_stat* stat_buf) {

  struct stat sb;

  int err = fstat(fd, &sb);
  if (err == 0) {
    copy_stat_buf(stat_buf, &sb);
  }

  return err;
}

int shim_chmod_impl(const char* path, mode_t mode) {
  assert(!str_starts_with(path, "/dev/"));
  return chmod(path, mode);
}

int shim___xmknod_impl(int ver, const char* path, mode_t mode, dev_t* dev) {
  UNIMPLEMENTED();
}
