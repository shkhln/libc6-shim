#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "../../shim.h"
#include "stat.h"

void copy_stat_buf(linux_stat* dst, struct stat* src) {

  memset(dst, 0, sizeof(linux_stat));

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

void copy_stat_buf64(linux_stat64* dst, struct stat* src) {

  memset(dst, 0, sizeof(linux_stat64));

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

int shim___fxstat_impl(int ver, int fd, linux_stat* stat_buf) {

  struct stat sb;

  int err = fstat(fd, &sb);
  if (err == 0) {
    copy_stat_buf(stat_buf, &sb);
  }

  return err;
}

int shim___fxstat64_impl(int ver, int fd, linux_stat64* stat_buf) {

  struct stat sb;

  int err = fstat(fd, &sb);
  if (err == 0) {
    copy_stat_buf64(stat_buf, &sb);
  }

  return err;
}

int shim___lxstat64_impl(int ver, const char* path, linux_stat64* stat_buf) {

  struct stat sb;

  int err = lstat(path, &sb);
  if (err == 0) {
    copy_stat_buf64(stat_buf, &sb);
  }

  return err;
}

int shim___xmknod_impl(int ver, const char* path, mode_t mode, dev_t* dev) {
  UNIMPLEMENTED();
}

int shim___xstat_impl(int ver, const char* path, linux_stat* stat_buf) {

  struct stat sb;

  int err = stat(path, &sb);
  if (err == 0) {
    copy_stat_buf(stat_buf, &sb);
  }

  return err;
}

int shim___xstat64_impl(int ver, const char* path, linux_stat64* stat_buf) {

  struct stat sb;

  int err = stat(path, &sb);
  if (err == 0) {
    copy_stat_buf64(stat_buf, &sb);
  }

  return err;
}

int shim_chmod_impl(const char* path, mode_t mode) {
  assert(!str_starts_with(path, "/dev/"));
  return chmod(path, mode);
}

SHIM_WRAP(__fxstat);
SHIM_WRAP(__fxstat64);
SHIM_WRAP(__lxstat64);
SHIM_WRAP(__xmknod);
SHIM_WRAP(__xstat);
SHIM_WRAP(__xstat64);
SHIM_WRAP(chmod);
