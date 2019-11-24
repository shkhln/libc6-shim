#include <assert.h>
#include <pthread.h>
#include <pthread_np.h>
#include "../shim.h"

enum linux_pthread_mutextype {
  LINUX_PTHREAD_MUTEX_NORMAL     = 0,
  LINUX_PTHREAD_MUTEX_RECURSIVE  = 1,
  LINUX_PTHREAD_MUTEX_ERRORCHECK = 2
};

int shim_pthread_getaffinity_np_impl(pthread_t thread, size_t cpusetsize, /*cpu_set_t* cpuset*/ void* cpuset) {
  UNIMPLEMENTED();
}

int shim_pthread_setaffinity_np_impl(pthread_t thread, size_t cpusetsize, /*const cpu_set_t* cpuset*/ void* cpuset) {
  UNIMPLEMENTED();
}

int shim_pthread_getname_np(pthread_t tid, char* name, size_t len) {
  UNIMPLEMENTED();
}

int shim_pthread_setname_np(pthread_t tid, const char* name) {
  LOG_ARGS("%p, %s",  tid, name);
  pthread_set_name_np(tid, name);
  return 0;
}

int shim_pthread_kill_impl(pthread_t thread, int sig) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_getprioceiling_impl(const pthread_mutexattr_t* restrict attr, int* restrict prioceiling) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_setprioceiling_impl(pthread_mutexattr_t* attr, int prioceiling) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_getrobust_impl(const pthread_mutexattr_t* attr, int* robustness) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_setrobust_impl(const pthread_mutexattr_t* attr, int robustness) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_gettype(const pthread_mutexattr_t* attr, int* kind) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_getkind_np(const pthread_mutexattr_t*attr, int* kind) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_settype_impl(pthread_mutexattr_t* attr, int linux_kind) {

  int kind = -1;

  switch (linux_kind) {

    case LINUX_PTHREAD_MUTEX_NORMAL:
      kind = PTHREAD_MUTEX_NORMAL;
      break;

    case LINUX_PTHREAD_MUTEX_RECURSIVE:
      kind = PTHREAD_MUTEX_RECURSIVE;
      break;

    case LINUX_PTHREAD_MUTEX_ERRORCHECK:
      kind = PTHREAD_MUTEX_ERRORCHECK;
      break;

    default:
      assert(0);
  }

  return pthread_mutexattr_settype(attr, kind);
}

int shim_pthread_mutexattr_setkind_np_impl(pthread_mutexattr_t* attr, int linux_kind) {
  return shim_pthread_mutexattr_settype_impl(attr, linux_kind);
}

int shim_pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared) {
  LOG_ARGS("%p, %d",  attr, pshared);
  int err = pthread_mutexattr_setpshared(attr, pshared);
  LOG_RES("%d", err);
  return err;
}

int shim___pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) {
  LOG_ARGS("%p, %p", key, destructor);
  int err = pthread_key_create(key, destructor);
  LOG_RES("%d", err);
  return err;
}
