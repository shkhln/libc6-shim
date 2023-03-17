#include <assert.h>
#include <string.h>
#include <sys/param.h>
#include <sys/mount.h>

#include "../../shim.h"
#include "mount.h"

static void copy_statfs_buf(linux_statfs* dst, struct statfs* src) {
  dst->f_type    = src->f_type;
  dst->f_bsize   = src->f_bsize;
  dst->f_blocks  = src->f_blocks;
  dst->f_bfree   = src->f_bfree;
  dst->f_bavail  = src->f_bavail;
  dst->f_files   = src->f_files;
  dst->f_ffree   = src->f_ffree;
  dst->f_fsid    = 0; // buf->f_fsid;
  dst->f_namelen = src->f_namemax;
  dst->f_frsize  = 0;
  dst->f_flags   = 0; // ?
}

static void copy_statfs64_buf(linux_statfs64* dst, struct statfs* src) {
  dst->f_type    = src->f_type;
  dst->f_bsize   = src->f_bsize;
  dst->f_blocks  = src->f_blocks;
  dst->f_bfree   = src->f_bfree;
  dst->f_bavail  = src->f_bavail;
  dst->f_files   = src->f_files;
  dst->f_ffree   = src->f_ffree;
  dst->f_fsid    = 0; // buf->f_fsid;
  dst->f_namelen = src->f_namemax;
  dst->f_frsize  = 0;
  dst->f_flags   = 0; // ?
}

int shim_fstatfs_impl(int fd, linux_statfs* linux_buf) {

  struct statfs buf;

  int err = fstatfs(fd, &buf);
  if (err == 0) {
    copy_statfs_buf(linux_buf, &buf);
  }

  return err;
}

int shim_fstatfs64_impl(int fd, linux_statfs64* linux_buf) {

  struct statfs buf;

  int err = fstatfs(fd, &buf);
  if (err == 0) {
    copy_statfs64_buf(linux_buf, &buf);
  }

  return err;
}

int shim_statfs_impl(const char* path, linux_statfs* linux_buf) {

  struct statfs buf;

  int err = statfs(path, &buf);
  if (err == 0) {
    copy_statfs_buf(linux_buf, &buf);
  }

  return err;
}

int shim_statfs64_impl(const char* path, linux_statfs64* linux_buf) {

  struct statfs buf;

  int err = statfs(path, &buf);
  if (err == 0) {
    copy_statfs64_buf(linux_buf, &buf);
  }

  return err;
}

#include <errno.h>
#include <sys/statvfs.h>
int shim_statvfs64_impl(const char* path, void* buf)
{
  return statvfs(path, buf);
}

int shim_fstatvfs64_impl(int fd, void* buf)
{
  return fstatvfs(fd, buf);
}

SHIM_WRAP(fstatfs);
SHIM_WRAP(fstatfs64);
SHIM_WRAP(fstatvfs64);
SHIM_WRAP(statfs);
SHIM_WRAP(statfs64);
SHIM_WRAP(statvfs64);
