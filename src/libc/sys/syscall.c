#include <assert.h>
#include <errno.h>
#include <pthread_np.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/thr.h>

#include "../time.h"
#include "../../shim.h"

#ifdef __i386__
#define LINUX_WRITE             4
#define LINUX_OPEN              5
#define LINUX_GETPID           20
#define LINUX_CAPGET          184
#define LINUX_GETTID          224
#define LINUX_FUTEX           240
#define LINUX_CLOCK_GETTIME   265
#define LINUX_TGKILL          270
#define LINUX_GET_ROBUST_LIST 312
#define LINUX_PIPE2           331
#define LINUX_GETRANDOM       355
#define LINUX_MEMFD_CREATE    356
#endif

#ifdef __x86_64__
#define LINUX_WRITE             1
#define LINUX_OPEN              2
#define LINUX_MMAP              9
#define LINUX_GETPID           39
#define LINUX_CAPGET          125
#define LINUX_GETTID          186
#define LINUX_FUTEX           202
#define LINUX_CLOCK_GETTIME   228
#define LINUX_TGKILL          234
#define LINUX_GET_ROBUST_LIST 274
#define LINUX_PIPE2           293
#define LINUX_GETRANDOM       318
#define LINUX_MEMFD_CREATE    319
#endif

void* shim_mmap_impl(void*, size_t, int, int, int, linux_off_t);
int   shim_open_impl(const char*, int, va_list);

long shim_syscall_impl(long number, va_list args) {

  if (number == LINUX_WRITE) {

    int    fd     = va_arg(args, int);
    void*  buf    = va_arg(args, void*);
    size_t nbytes = va_arg(args, size_t);

    LOG("%s: write(%d, %p, %zu)", __func__, fd, buf, nbytes);

    int n = write(fd, buf, nbytes);
    LOG("%s: write -> %d", __func__, n);

    return n;
  }

  if (number == LINUX_OPEN) {

    char*  path  = va_arg(args, void*);
    int    flags = va_arg(args, int);

    LOG("%s: open(\"%s\", 0x%x, ...)", __func__, path, flags);

    int fd = shim_open_impl(path, flags, args);
    LOG("%s: open -> %d", __func__, fd);

    return fd;
  }

#ifdef __x86_64__
  if (number == LINUX_MMAP) {
    void*       addr     = va_arg(args, void*);
    size_t      len      = va_arg(args, size_t);
    int         prot     = va_arg(args, int);
    int         flags    = va_arg(args, int);
    int         fd       = va_arg(args, int);
    linux_off_t pgoffset = va_arg(args, linux_off_t);

    LOG("%s: mmap(%p, %zu, %d, %d, %d, %ld)", __func__, addr, len, prot, flags, fd, pgoffset);

    void* p = shim_mmap_impl(addr, len, prot, flags, fd, pgoffset);
    LOG("%s: mmap -> %p", __func__, p);

    return (uintptr_t)p;
  }
#endif

  if (number == LINUX_GETPID) {

    LOG("%s: getpid()", __func__);

    pid_t pid = getpid();
    LOG("%s: getpid -> %d", __func__, pid);

    return pid;
  }

  if (number == LINUX_GETTID) {

    LOG("%s: gettid()", __func__);

    int tid = pthread_getthreadid_np();
    LOG("%s: gettid -> %d", __func__, tid);

    return tid;
  }

  if (number == LINUX_CAPGET) {
    LOG("%s: capget(...)", __func__);
    errno = native_to_linux_errno(EPERM);
    return -1;
  }

  if (number == LINUX_FUTEX) {
    errno = native_to_linux_errno(ENOSYS);
    return -1;
  }

  if (number == LINUX_CLOCK_GETTIME) {

    linux_clockid_t clock_id = va_arg(args, linux_clockid_t);
    linux_timespec* tp       = va_arg(args, linux_timespec*);

    LOG("%s: clock_gettime(%d, %p)", __func__, clock_id, tp);

    int err = shim_clock_gettime_impl(clock_id, tp);
    LOG("%s: clock_gettime -> %d", __func__, err);

    return err;
  }

  if (number == LINUX_TGKILL) {

    pid_t tgid = va_arg(args, pid_t);
    pid_t tid  = va_arg(args, pid_t);
    int   sig  = va_arg(args, int);

    LOG("%s: tgkill(%d, %d, %d)", __func__, tgid, tid, sig);

    assert(tgid == getpid());
    assert(sig  == 0);

    int err = thr_kill(tid, sig);
    LOG("%s: tgkill -> %d", __func__, err);

    return err;
  }

  if (number == LINUX_GET_ROBUST_LIST) {

    typedef void robust_list_head;

    int get_robust_list(int, robust_list_head**, size_t*);

    int                pid        = va_arg(args, int);
    robust_list_head** list_head  = va_arg(args, robust_list_head**);
    size_t*            struct_len = va_arg(args, size_t*);

#if DEBUG
    LOG("%s: get_robust_list(%d, %p, %p)\n", __func__, pid, list_head, struct_len);
#else
    fprintf(stderr, "%s [get_robust_list]: nothing to see here, move along\n", __func__);
#endif

    int err = get_robust_list(pid, list_head, struct_len);
    LOG("%s: get_robust_list -> %d", __func__, err);

    return err;
  }

  if (number == LINUX_PIPE2) {

    int shim_pipe2_impl(int[2], int);

    int* fds   = va_arg(args, int*);
    int  flags = va_arg(args, int);

    LOG("%s: pipe2(%p, %d)", __func__, fds, flags);

    int err = shim_pipe2_impl(fds, flags);
    LOG("%s: pipe2 -> %d ({%d, %d})", __func__, err, fds[0], fds[1]);

    return err;
  }

  if (number == LINUX_GETRANDOM) {
    errno = native_to_linux_errno(ENOSYS);
    return -1;
  }

  if (number == LINUX_MEMFD_CREATE) {
#if __FreeBSD_version >= 1300139
    char* name  = va_arg(args, char*);
    int   flags = va_arg(args, int);

    LOG("%s: memfd_create(\"%s\", 0x%x)", __func__, name, flags);

    assert((flags & (MFD_CLOEXEC | MFD_ALLOW_SEALING)) == flags);

    int err = memfd_create(name, flags);

    LOG("%s: memfd_create -> %d", __func__, err);

    return err;
#else
    errno = native_to_linux_errno(ENOSYS);
    return -1;
#endif
  }

  UNIMPLEMENTED_ARGS("%ld, ...", number);
}

SHIM_WRAP(syscall);
