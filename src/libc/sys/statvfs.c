#include <sys/statvfs.h>
#include "../../shim.h"

#ifdef __i386__
struct linux_statvfs64 {
  uint32_t f_bsize;
  uint32_t f_frsize;
  uint64_t f_blocks;
  uint64_t f_bfree;
  uint64_t f_bavail;
  uint64_t f_files;
  uint64_t f_ffree;
  uint64_t f_favail;
  uint32_t f_fsid;
  uint8_t _pad1[4];
  uint32_t f_flag;
  uint32_t f_namemax;
  uint8_t _pad2[24];
};
#endif

#ifdef __x86_64__
struct linux_statvfs64 {
  uint64_t f_bsize;
  uint64_t f_frsize;
  uint64_t f_blocks;
  uint64_t f_bfree;
  uint64_t f_bavail;
  uint64_t f_files;
  uint64_t f_ffree;
  uint64_t f_favail;
  uint64_t f_fsid;
  uint64_t f_flag;
  uint64_t f_namemax;
  uint8_t _pad[24];
};
#endif

typedef struct linux_statvfs64 linux_statvfs64;

#define LINUX_ST_RDONLY 1
#define LINUX_ST_NOSUID 2

static int shim_statvfs64_impl(const char* restrict path, linux_statvfs64* restrict linux_buf) {

  struct statvfs buf;
  int err = statvfs(path, &buf);
  if (err == 0) {
    linux_buf->f_bsize  = buf.f_bsize;
    linux_buf->f_frsize = buf.f_frsize;

    linux_buf->f_blocks = buf.f_blocks;
    linux_buf->f_bfree  = buf.f_bfree;
    linux_buf->f_bavail = buf.f_bavail;

    linux_buf->f_files  = buf.f_files;
    linux_buf->f_ffree  = buf.f_ffree;
    linux_buf->f_favail = buf.f_favail;

    linux_buf->f_fsid = 0;

    linux_buf->f_flag = 0;
    if (buf.f_flag & ST_RDONLY) {
      linux_buf->f_flag |= LINUX_ST_RDONLY;
    }
    if (buf.f_flag & ST_NOSUID) {
      linux_buf->f_flag |= LINUX_ST_NOSUID;
    }

    linux_buf->f_namemax = buf.f_namemax;
  }

  return err;
}

SHIM_WRAP(statvfs64);
