#include <assert.h>
#include <pthread.h>
#include <pthread_np.h>
#include "../shim.h"
#include "../libc/time.h"
#include "pthread.h"

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

int shim_pthread_getname_np_impl(pthread_t tid, char* name, size_t len) {
  UNIMPLEMENTED();
}

int shim_pthread_setname_np_impl(pthread_t tid, const char* name) {
  pthread_set_name_np(tid, name);
  return 0;
}

int shim_pthread_kill_impl(pthread_t thread, int sig) {
  UNIMPLEMENTED();
}

SHIM_WRAP(pthread_getaffinity_np);
SHIM_WRAP(pthread_setaffinity_np);
SHIM_WRAP(pthread_getname_np);
SHIM_WRAP(pthread_setname_np);
//SHIM_WRAP(pthread_key_create);
SHIM_WRAP(pthread_kill);

int shim_pthread_mutexattr_getkind_np_impl(/*const pthread_mutexattr_t* attr, int* kind*/ pthread_mutexattr_t attr) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_settype_impl(pthread_mutexattr_t* attr, int linux_kind);

int shim_pthread_mutexattr_setkind_np_impl(pthread_mutexattr_t* attr, int linux_kind) {
  return shim_pthread_mutexattr_settype_impl(attr, linux_kind);
}

int shim_pthread_mutexattr_getprioceiling_impl(const pthread_mutexattr_t* restrict attr, int* restrict prioceiling) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_setprioceiling_impl(pthread_mutexattr_t* attr, int prioceiling) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_setpshared_impl(pthread_mutexattr_t* attr, int pshared) {
  return pthread_mutexattr_setpshared(attr, pshared);
}

int shim_pthread_mutexattr_getrobust_impl(const pthread_mutexattr_t* attr, int* robustness) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_setrobust_impl(const pthread_mutexattr_t* attr, int robustness) {
  UNIMPLEMENTED();
}

int shim_pthread_mutexattr_gettype_impl(const pthread_mutexattr_t* attr, int* kind) {
  UNIMPLEMENTED();
}

static int linux_to_native_mutex_kind(int linux_kind) {
  switch (linux_kind) {
    case LINUX_PTHREAD_MUTEX_NORMAL:     return PTHREAD_MUTEX_NORMAL;
    case LINUX_PTHREAD_MUTEX_RECURSIVE:  return PTHREAD_MUTEX_RECURSIVE;
    case LINUX_PTHREAD_MUTEX_ERRORCHECK: return PTHREAD_MUTEX_ERRORCHECK;
    default:
      assert(0);
  }
}

int shim_pthread_mutexattr_settype_impl(pthread_mutexattr_t* attr, int linux_kind) {
  return pthread_mutexattr_settype(attr, linux_to_native_mutex_kind(linux_kind));
}

SHIM_WRAP(pthread_mutexattr_getkind_np);
SHIM_WRAP(pthread_mutexattr_setkind_np);
SHIM_WRAP(pthread_mutexattr_getprioceiling);
SHIM_WRAP(pthread_mutexattr_setprioceiling);
SHIM_WRAP(pthread_mutexattr_setpshared);
SHIM_WRAP(pthread_mutexattr_getrobust);
SHIM_WRAP(pthread_mutexattr_setrobust);
SHIM_WRAP(pthread_mutexattr_gettype);
SHIM_WRAP(pthread_mutexattr_settype);

#define NATIVE_MUTEX_T(shim_mutex) &(shim_mutex->_native_mutex)

int shim_pthread_mutex_init_impl(linux_pthread_mutex_t* mutex, const pthread_mutexattr_t* attr) {
  return pthread_mutex_init(NATIVE_MUTEX_T(mutex), attr);
}

static void init_mutex_if_necessary(linux_pthread_mutex_t* mutex) {

  if (mutex->_native_mutex == 0 && mutex->linux_kind > 0) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, linux_to_native_mutex_kind(mutex->linux_kind));
    pthread_mutex_init(NATIVE_MUTEX_T(mutex), &attr);
  }
}

int shim_pthread_mutex_lock_impl(linux_pthread_mutex_t* mutex) {
  init_mutex_if_necessary(mutex);
  return pthread_mutex_lock(NATIVE_MUTEX_T(mutex));
}

SHIM_WRAP(pthread_mutex_init);
SHIM_WRAP(pthread_mutex_lock);

int shim_pthread_cond_timedwait_impl(pthread_cond_t* cond, linux_pthread_mutex_t* mutex, const linux_timespec* abstime) {
  assert(mutex->_native_mutex != 0);
  return pthread_cond_timedwait(cond, NATIVE_MUTEX_T(mutex), abstime);
}

int shim_pthread_cond_wait_impl(pthread_cond_t* cond, linux_pthread_mutex_t* mutex) {
  assert(mutex->_native_mutex != 0);
  return pthread_cond_wait(cond, NATIVE_MUTEX_T(mutex));
}

int shim_pthread_mutex_consistent_impl(linux_pthread_mutex_t* mutex) {
  assert(mutex->_native_mutex != 0);
  return pthread_mutex_consistent(NATIVE_MUTEX_T(mutex));
}

int shim_pthread_mutex_timedlock_impl(linux_pthread_mutex_t* mutex, const linux_timespec* abs_timeout) {
  assert(mutex->_native_mutex != 0);
  return pthread_mutex_timedlock(NATIVE_MUTEX_T(mutex), abs_timeout);
}

int shim_pthread_mutex_trylock_impl(linux_pthread_mutex_t* mutex) {
  assert(mutex->_native_mutex != 0);
  return pthread_mutex_trylock(NATIVE_MUTEX_T(mutex));
}

SHIM_WRAP(pthread_cond_timedwait);
SHIM_WRAP(pthread_cond_wait);
SHIM_WRAP(pthread_mutex_consistent);
SHIM_WRAP(pthread_mutex_timedlock);
SHIM_WRAP(pthread_mutex_trylock);

int shim_pthread_mutex_destroy_impl(linux_pthread_mutex_t* mutex) {
  assert(mutex->_native_mutex != 0);
  return pthread_mutex_destroy(NATIVE_MUTEX_T(mutex));
}

int shim_pthread_mutex_unlock_impl(linux_pthread_mutex_t* mutex) {
  assert(mutex->_native_mutex != 0);
  return pthread_mutex_unlock(NATIVE_MUTEX_T(mutex));
}

SHIM_WRAP(pthread_mutex_destroy);
SHIM_WRAP(pthread_mutex_unlock);
