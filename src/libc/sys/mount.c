#include <assert.h>
#include <sys/param.h>
#include <sys/mount.h>

#include "../../shim.h"

#ifdef __i386__

struct linux_statfs {
  uint32_t f_type;
  uint32_t f_bsize;
  uint32_t f_blocks;
  uint32_t f_bfree;
  uint32_t f_bavail;
  uint32_t f_files;
  uint32_t f_ffree;
  uint64_t f_fsid;
  uint32_t f_namelen;
  uint32_t f_frsize;
  uint32_t f_flags;
  uint32_t f_spare[4];
};

#endif

#ifdef __x86_64__

struct linux_statfs {
  uint64_t f_type;
  uint64_t f_bsize;
  uint64_t f_blocks;
  uint64_t f_bfree;
  uint64_t f_bavail;
  uint64_t f_files;
  uint64_t f_ffree;
  uint64_t f_fsid;
  uint64_t f_namelen;
  uint64_t f_frsize;
  uint64_t f_flags;
  uint64_t f_spare[4];
};

#endif

int shim_statfs_impl(const char* path, struct linux_statfs* linux_buf) {

  struct statfs buf;

  int err = statfs(path, &buf);
  if (err == 0) {
    memset(linux_buf, 0, sizeof(struct linux_statfs));

    linux_buf->f_type    = buf.f_type;
    linux_buf->f_bsize   = buf.f_bsize;
    linux_buf->f_blocks  = buf.f_blocks;
    linux_buf->f_bfree   = buf.f_bfree;
    linux_buf->f_bavail  = buf.f_bavail;
    linux_buf->f_files   = buf.f_files;
    linux_buf->f_ffree   = buf.f_ffree;
    linux_buf->f_fsid    = 0; // buf.f_fsid;
    linux_buf->f_namelen = buf.f_namemax;
    linux_buf->f_frsize  = 0;
    linux_buf->f_flags   = 0; // ?
  }

  return err;
}
