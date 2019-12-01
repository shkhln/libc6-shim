#include <assert.h>
#include <string.h>
#include <sys/param.h>
#include <sys/mount.h>

#include "../../shim.h"
#include "mount.h"

int shim_statfs_impl(const char* path, linux_statfs* linux_buf) {

  struct statfs buf;

  int err = statfs(path, &buf);
  if (err == 0) {
    memset(linux_buf, 0, sizeof(linux_statfs));

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

int shim_statfs64_impl(const char* path, linux_statfs64* linux_buf) {

  struct statfs buf;

  int err = statfs(path, &buf);
  if (err == 0) {
    memset(linux_buf, 0, sizeof(linux_statfs64));

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

SHIM_WRAP(statfs);
SHIM_WRAP(statfs64);
