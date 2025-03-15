#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <pthread_np.h>
#include <signal.h>
#include <spawn.h>
#include "../shim.h"
#include "../libc/sched.h"
#include "../libc/signal.h"
#include "../libc/time.h"
#include "pthread.h"

#define NATIVE_WHATEVER_ATTRS(name, max_attrs) \
                                                                                                            \
  static pthread_mutex_t name ## _attributes_mutex = PTHREAD_MUTEX_INITIALIZER;                             \
                                                                                                            \
  static pthread_ ## name ## attr_t name ## _attributes[max_attrs] = { NULL };                              \
                                                                                                            \
  static uint32_t name ## _attributes_index = 0;                                                            \
                                                                                                            \
  static int init_native_ ## name ## attr(linux_pthread_ ## name ## attr_t* attr) {                         \
                                                                                                            \
    assert(attr != NULL);                                                                                   \
                                                                                                            \
    assert(pthread_mutex_lock(& name ## _attributes_mutex) == 0);                                           \
                                                                                                            \
    for (int i = 0; i < max_attrs; i++) {                                                                   \
      uint32_t idx = name ## _attributes_index;                                                             \
      if (name ## _attributes[idx] == NULL) {                                                               \
        int err = pthread_ ## name ## attr_init(& name ## _attributes[idx]);                                \
        if (err == 0) {                                                                                     \
          *attr = idx + 1;                                                                                  \
        }                                                                                                   \
        assert(pthread_mutex_unlock(& name ## _attributes_mutex) == 0);                                     \
        return err;                                                                                         \
      }                                                                                                     \
      name ## _attributes_index = (idx + 1) % max_attrs;                                                    \
    }                                                                                                       \
                                                                                                            \
    assert(0);                                                                                              \
  }                                                                                                         \
                                                                                                            \
  static pthread_ ## name ## attr_t* find_native_ ## name ## attr(linux_pthread_ ## name ## attr_t* attr) { \
                                                                                                            \
    if (attr == NULL) {                                                                                     \
      return NULL;                                                                                          \
    }                                                                                                       \
                                                                                                            \
    assert(*attr >= 1 && *attr <= max_attrs);                                                               \
                                                                                                            \
    return & name ## _attributes[*attr - 1];                                                                \
  }                                                                                                         \
                                                                                                            \
  static int destroy_native_ ## name ## attr(linux_pthread_ ## name ## attr_t* attr) {                      \
                                                                                                            \
    assert(attr != NULL);                                                                                   \
    assert(*attr >= 1 && *attr <= max_attrs);                                                               \
                                                                                                            \
    int err = pthread_ ## name ## attr_destroy(& name ## _attributes[*attr - 1]);                           \
    if (err == 0) {                                                                                         \
      assert(pthread_mutex_lock(& name ## _attributes_mutex) == 0);                                         \
      name ## _attributes[*attr - 1] = NULL;                                                                \
      assert(pthread_mutex_unlock(& name ## _attributes_mutex) == 0);                                       \
      *attr = 0;                                                                                            \
    }                                                                                                       \
                                                                                                            \
    return err;                                                                                             \
  }

NATIVE_WHATEVER_ATTRS(barrier,  10);
NATIVE_WHATEVER_ATTRS(cond,    100);
NATIVE_WHATEVER_ATTRS(mutex,   300);

static int shim_pthread_join_impl(pthread_t thread, void** value_ptr) {
  int err = pthread_join(thread, value_ptr);
  if (err == 0) {
    if (value_ptr != NULL && *value_ptr == PTHREAD_CANCELED) {
      *value_ptr = LINUX_PTHREAD_CANCELED;
    }
  }
  return err;
}

static int shim_pthread_timedjoin_np_impl(pthread_t thread, void** value_ptr, const linux_timespec* abstime) {
  int err = pthread_timedjoin_np(thread, value_ptr, abstime);
  if (err == 0) {
    if (value_ptr != NULL && *value_ptr == PTHREAD_CANCELED) {
      *value_ptr = LINUX_PTHREAD_CANCELED;
    }
  }
  return native_to_linux_errno(err);
}

SHIM_WRAP(pthread_join);
SHIM_WRAP(pthread_timedjoin_np);

//TODO: impl
static int shim_pthread_getaffinity_np_impl(pthread_t thread, size_t cpusetsize, /*cpu_set_t* cpuset*/ void* cpuset) {
  return native_to_linux_errno(EPERM);
}

static int shim_pthread_setaffinity_np_impl(pthread_t thread, size_t cpusetsize, /*const cpu_set_t* cpuset*/ void* cpuset) {
  UNIMPLEMENTED();
}

static int shim_pthread_getname_np_impl(pthread_t tid, char* name, size_t len) {
  UNIMPLEMENTED();
}

static int shim_pthread_setname_np_impl(pthread_t tid, const char* name) {
  pthread_set_name_np(tid, name);
  return 0;
}

static int shim_pthread_kill_impl(pthread_t thread, int linux_sig) {
  int sig = linux_to_freebsd_signo(linux_sig);
  assert(sig != -1);
  return pthread_kill(thread, sig);
}

SHIM_WRAP(pthread_getaffinity_np);
SHIM_WRAP(pthread_setaffinity_np);
SHIM_WRAP(pthread_getname_np);
SHIM_WRAP(pthread_setname_np);
//SHIM_WRAP(pthread_key_create);
SHIM_WRAP(pthread_kill);

static int shim_pthread_mutexattr_getkind_np_impl(const linux_pthread_mutexattr_t* attr, int* kind) {
  UNIMPLEMENTED();
}

static int shim_pthread_mutexattr_settype_impl(linux_pthread_mutexattr_t* attr, int linux_kind);

static int shim_pthread_mutexattr_setkind_np_impl(linux_pthread_mutexattr_t* attr, int linux_kind) {
  return shim_pthread_mutexattr_settype_impl(attr, linux_kind);
}

static int shim_pthread_mutexattr_getprioceiling_impl(const linux_pthread_mutexattr_t* restrict attr, int* restrict prioceiling) {
  UNIMPLEMENTED();
}

static int shim_pthread_mutexattr_setprioceiling_impl(linux_pthread_mutexattr_t* attr, int prioceiling) {
  UNIMPLEMENTED();
}

static int shim_pthread_mutexattr_setpshared_impl(linux_pthread_mutexattr_t* attr, int pshared) {
  return pthread_mutexattr_setpshared(find_native_mutexattr(attr), pshared);
}

static int shim_pthread_mutexattr_getrobust_impl(const linux_pthread_mutexattr_t* attr, int* robustness) {
  UNIMPLEMENTED();
}

static int shim_pthread_mutexattr_setrobust_impl(const linux_pthread_mutexattr_t* attr, int robustness) {
  UNIMPLEMENTED();
}

static int shim_pthread_mutexattr_gettype_impl(const linux_pthread_mutexattr_t* attr, int* kind) {
  UNIMPLEMENTED();
}

static int linux_to_native_mutex_kind(int linux_kind) {
  switch (linux_kind) {
    case LINUX_PTHREAD_MUTEX_NORMAL:      return PTHREAD_MUTEX_NORMAL;
    case LINUX_PTHREAD_MUTEX_RECURSIVE:   return PTHREAD_MUTEX_RECURSIVE;
    case LINUX_PTHREAD_MUTEX_ERRORCHECK:  return PTHREAD_MUTEX_ERRORCHECK;
    case LINUX_PTHREAD_MUTEX_ADAPTIVE_NP: return PTHREAD_MUTEX_ADAPTIVE_NP;
    default:
      assert(0);
  }
}

static int shim_pthread_mutexattr_settype_impl(linux_pthread_mutexattr_t* attr, int linux_kind) {
  return pthread_mutexattr_settype(find_native_mutexattr(attr), linux_to_native_mutex_kind(linux_kind));
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

#define NATIVE_MUTEX_T(shim_mutex) &(shim_mutex->_wrapped_mutex)

static int shim_pthread_mutex_init_impl(linux_pthread_mutex_t* mutex, const linux_pthread_mutexattr_t* attr) {
  return pthread_mutex_init(NATIVE_MUTEX_T(mutex), find_native_mutexattr(attr));
}

static pthread_mutex_t mutex_initialization_mutex = PTHREAD_MUTEX_INITIALIZER;

static void init_mutex_if_necessary(linux_pthread_mutex_t* mutex) {

  if (mutex->_wrapped_mutex == NULL && mutex->linux_kind != LINUX_PTHREAD_MUTEX_NORMAL) {

    int lock_err = pthread_mutex_lock(&mutex_initialization_mutex);
    assert(lock_err == 0);

    if (mutex->_wrapped_mutex == NULL) {

      int type = linux_to_native_mutex_kind(mutex->linux_kind);
      LOG("%s: init %p (type = %d -> %d)", __func__, mutex, mutex->linux_kind, type);

      pthread_mutexattr_t attr;
      pthread_mutexattr_init(&attr);
      pthread_mutexattr_settype(&attr, type);

      pthread_mutex_init(NATIVE_MUTEX_T(mutex), &attr);
    }

    int unlock_err = pthread_mutex_unlock(&mutex_initialization_mutex);
    assert(unlock_err == 0);
  }
}

static int shim_pthread_mutex_lock_impl(linux_pthread_mutex_t* mutex) {
  init_mutex_if_necessary(mutex);
  return pthread_mutex_lock(NATIVE_MUTEX_T(mutex));
}

SHIM_WRAP(pthread_mutex_init);
SHIM_WRAP(pthread_mutex_lock);

static int shim_pthread_cond_timedwait_impl(pthread_cond_t* cond, linux_pthread_mutex_t* mutex, const linux_timespec* abstime) {
  init_mutex_if_necessary(mutex);
  return native_to_linux_errno(pthread_cond_timedwait(cond, NATIVE_MUTEX_T(mutex), abstime));
}

static int shim_pthread_cond_wait_impl(pthread_cond_t* cond, linux_pthread_mutex_t* mutex) {
  init_mutex_if_necessary(mutex);
  return pthread_cond_wait(cond, NATIVE_MUTEX_T(mutex));
}

static int shim_pthread_mutex_consistent_impl(linux_pthread_mutex_t* mutex) {
  assert(mutex->_wrapped_mutex != 0);
  return pthread_mutex_consistent(NATIVE_MUTEX_T(mutex));
}

static int shim_pthread_mutex_timedlock_impl(linux_pthread_mutex_t* mutex, const linux_timespec* abs_timeout) {
  init_mutex_if_necessary(mutex);
  return native_to_linux_errno(pthread_mutex_timedlock(NATIVE_MUTEX_T(mutex), abs_timeout));
}

static int shim_pthread_mutex_trylock_impl(linux_pthread_mutex_t* mutex) {
  init_mutex_if_necessary(mutex);
  return pthread_mutex_trylock(NATIVE_MUTEX_T(mutex));
}

SHIM_WRAP(pthread_cond_timedwait);
SHIM_WRAP(pthread_cond_wait);
SHIM_WRAP(pthread_mutex_consistent);
SHIM_WRAP(pthread_mutex_timedlock);
SHIM_WRAP(pthread_mutex_trylock);

static int shim_pthread_mutex_destroy_impl(linux_pthread_mutex_t* mutex) {
  assert(mutex->_wrapped_mutex != 0);
  return pthread_mutex_destroy(NATIVE_MUTEX_T(mutex));
}

static int shim_pthread_mutex_unlock_impl(linux_pthread_mutex_t* mutex) {
  assert(mutex->_wrapped_mutex != 0);
  return pthread_mutex_unlock(NATIVE_MUTEX_T(mutex));
}

SHIM_WRAP(pthread_mutex_destroy);
SHIM_WRAP(pthread_mutex_unlock);

static int shim_pthread_rwlock_timedrdlock_impl(pthread_rwlock_t* rwlock, const linux_timespec* abs_timeout) {
  return native_to_linux_errno(pthread_rwlock_timedrdlock(rwlock, abs_timeout));
}

static int shim_pthread_rwlock_timedwrlock_impl(pthread_rwlock_t* rwlock, const linux_timespec* abs_timeout) {
  return native_to_linux_errno(pthread_rwlock_timedwrlock(rwlock, abs_timeout));
}

SHIM_WRAP(pthread_rwlock_timedrdlock);
SHIM_WRAP(pthread_rwlock_timedwrlock);

static int shim_pthread_getattr_np_impl(pthread_t thread, pthread_attr_t* attr) {
  pthread_attr_init(attr);
  return pthread_attr_get_np(thread, attr);
}

SHIM_WRAP(pthread_getattr_np);

static int shim_pthread_mutexattr_init_impl(linux_pthread_mutexattr_t* attr) {
  return init_native_mutexattr(attr);
}

SHIM_WRAP(pthread_mutexattr_init);

static int shim_pthread_mutexattr_destroy_impl(linux_pthread_mutexattr_t* attr) {
  return destroy_native_mutexattr(attr);
}

SHIM_WRAP(pthread_mutexattr_destroy);

static int shim_pthread_mutexattr_getprotocol_impl(linux_pthread_mutexattr_t* attr, int* protocol) {
  return pthread_mutexattr_getprotocol(find_native_mutexattr(attr), protocol);
}

static int shim_pthread_mutexattr_setprotocol_impl(linux_pthread_mutexattr_t* attr, int protocol) {
  return pthread_mutexattr_setprotocol(find_native_mutexattr(attr), protocol);
}

SHIM_WRAP(pthread_mutexattr_getprotocol);
SHIM_WRAP(pthread_mutexattr_setprotocol);

static int shim_pthread_barrierattr_init_impl(linux_pthread_barrierattr_t* attr) {
  return init_native_barrierattr(attr);
}

static int shim_pthread_barrierattr_destroy_impl(linux_pthread_barrierattr_t* attr) {
  return destroy_native_barrierattr(attr);
}

SHIM_WRAP(pthread_barrierattr_init);
SHIM_WRAP(pthread_barrierattr_destroy);

static int shim_pthread_barrierattr_getpshared_impl(const linux_pthread_barrierattr_t* attr, int* pshared) {
  return pthread_barrierattr_getpshared(find_native_barrierattr(attr), pshared);
}

static int shim_pthread_barrierattr_setpshared_impl(linux_pthread_barrierattr_t* attr, int pshared) {
  return pthread_barrierattr_setpshared(find_native_barrierattr(attr), pshared);
}

SHIM_WRAP(pthread_barrierattr_getpshared);
SHIM_WRAP(pthread_barrierattr_setpshared);

static int shim_pthread_barrier_init_impl(pthread_barrier_t* barrier, const linux_pthread_barrierattr_t* attr, unsigned count) {
  return pthread_barrier_init(barrier, find_native_barrierattr(attr), count);
}

SHIM_WRAP(pthread_barrier_init);

static int shim_pthread_condattr_init_impl(linux_pthread_condattr_t* attr) {
  return init_native_condattr(attr);
}

static int shim_pthread_condattr_destroy_impl(linux_pthread_condattr_t* attr) {
  return destroy_native_condattr(attr);
}

SHIM_WRAP(pthread_condattr_init);
SHIM_WRAP(pthread_condattr_destroy);

static int shim_pthread_condattr_getclock_impl(linux_pthread_condattr_t* restrict attr, clockid_t* restrict clock_id) {
  return pthread_condattr_getclock(find_native_condattr(attr), clock_id);
}

static int shim_pthread_condattr_setclock_impl(linux_pthread_condattr_t* attr, clockid_t clock_id) {
  return pthread_condattr_setclock(find_native_condattr(attr), clock_id);
}

SHIM_WRAP(pthread_condattr_getclock);
SHIM_WRAP(pthread_condattr_setclock);

static int shim_pthread_condattr_getpshared_impl(linux_pthread_condattr_t* restrict attr, int* restrict pshared) {
  return pthread_condattr_getpshared(find_native_condattr(attr), pshared);
}

static int shim_pthread_condattr_setpshared_impl(linux_pthread_condattr_t* attr, int pshared) {
  return pthread_condattr_setpshared(find_native_condattr(attr), pshared);
}

SHIM_WRAP(pthread_condattr_setpshared);
SHIM_WRAP(pthread_condattr_getpshared);

static int shim_pthread_cond_init_impl(pthread_cond_t* cond, const linux_pthread_condattr_t* attr) {
  return pthread_cond_init(cond, find_native_condattr(attr));
}

SHIM_WRAP(pthread_cond_init);

static int shim_pthread_attr_getinheritsched_impl(const pthread_attr_t* attr, int* linux_inheritsched) {

  int inheritsched;

  int err = pthread_attr_getinheritsched(attr, &inheritsched);
  if (err == 0) {
    switch (inheritsched) {
      case PTHREAD_INHERIT_SCHED:
        *linux_inheritsched = LINUX_PTHREAD_INHERIT_SCHED;
        break;
      case PTHREAD_EXPLICIT_SCHED:
        *linux_inheritsched = LINUX_PTHREAD_EXPLICIT_SCHED;
        break;
      default:
        assert(0);
    }
  }

  return err;
}

static int shim_pthread_attr_setinheritsched_impl(pthread_attr_t* attr, int linux_inheritsched) {

  int inheritsched;

  switch (linux_inheritsched) {
    case LINUX_PTHREAD_INHERIT_SCHED:
      inheritsched = PTHREAD_INHERIT_SCHED;
      break;
    case LINUX_PTHREAD_EXPLICIT_SCHED:
      inheritsched = PTHREAD_EXPLICIT_SCHED;
      break;
    default:
      assert(0);
  }

  return pthread_attr_setinheritsched(attr, inheritsched);
}

SHIM_WRAP(pthread_attr_getinheritsched);
SHIM_WRAP(pthread_attr_setinheritsched);

static int shim_pthread_attr_getschedpolicy_impl(const pthread_attr_t* attr, int* linux_policy) {
  int policy;
  int err = pthread_attr_getschedpolicy(attr, &policy);
  if (err == 0) {
    *linux_policy = native_to_linux_sched_policy(policy);
  }
  return err;
}

static int shim_pthread_attr_setschedpolicy_impl(pthread_attr_t* attr, int linux_policy) {
  return pthread_attr_setschedpolicy(attr, linux_to_native_sched_policy(linux_policy));
}

SHIM_WRAP(pthread_attr_getschedpolicy);
SHIM_WRAP(pthread_attr_setschedpolicy);

static int shim_pthread_getschedparam_impl(pthread_t thread, int* linux_policy, linux_sched_param* param) {
  int policy;
  int err = pthread_getschedparam(thread, &policy, param);
  if (err == 0) {
    *linux_policy = native_to_linux_sched_policy(policy);
  }
  return err;
}

static int shim_pthread_setschedparam_impl(pthread_t thread, int linux_policy, const linux_sched_param* param) {
  return pthread_setschedparam(thread, linux_to_native_sched_policy(linux_policy), param);
}

SHIM_WRAP(pthread_getschedparam);
SHIM_WRAP(pthread_setschedparam);

static int shim_pthread_attr_getscope_impl(const pthread_attr_t* attr, int* linux_scope) {

  int scope;

  int err = pthread_attr_getscope(attr, &scope);
  if (err == 0) {
    switch(scope) {
      case PTHREAD_SCOPE_SYSTEM:
        *linux_scope = LINUX_PTHREAD_SCOPE_SYSTEM;
        break;
      case PTHREAD_SCOPE_PROCESS:
        *linux_scope = LINUX_PTHREAD_SCOPE_PROCESS;
        break;
      default:
        assert(0);
    }
  }

  return err;
}

static int shim_pthread_attr_setscope_impl(pthread_attr_t* attr, int linux_scope) {

  int scope;

  switch(linux_scope) {
    case LINUX_PTHREAD_SCOPE_SYSTEM:
      scope = PTHREAD_SCOPE_SYSTEM;
      break;
    case LINUX_PTHREAD_SCOPE_PROCESS:
      scope = PTHREAD_SCOPE_PROCESS;
      break;
    default:
      assert(0);
  }

  return pthread_attr_setscope(attr, scope);
}

SHIM_WRAP(pthread_attr_getscope);
SHIM_WRAP(pthread_attr_setscope);

#define MAX_ONCES 250

static pthread_mutex_t onces_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_once_t onces[MAX_ONCES] = { PTHREAD_ONCE_INIT };
static uint32_t onces_index = 0;

static int shim_pthread_once_impl(linux_pthread_once_t* linux_once, void (*routine)(void)) {

  assert(pthread_mutex_lock(&onces_mutex) == 0);

  if (*linux_once == 0) {
    assert(onces_index < MAX_ONCES);
    *linux_once = onces_index;
    onces_index++;
  }

  assert(pthread_mutex_unlock(&onces_mutex) == 0);

  return pthread_once(&onces[*linux_once], routine);
}

SHIM_WRAP(pthread_once);

static int shim_pthread_spin_destroy_impl(pthread_spinlock_t* lock) {
  UNIMPLEMENTED();
}

static int shim_pthread_spin_init_impl(pthread_spinlock_t* lock, int pshared) {
  UNIMPLEMENTED();
}

static int shim_pthread_spin_lock_impl(pthread_spinlock_t* lock) {
  UNIMPLEMENTED();
}

static int shim_pthread_spin_trylock_impl(pthread_spinlock_t* lock) {
  UNIMPLEMENTED();
}

static int shim_pthread_spin_unlock_impl(pthread_spinlock_t* lock) {
  UNIMPLEMENTED();
}

SHIM_WRAP(pthread_spin_destroy);
SHIM_WRAP(pthread_spin_init);
SHIM_WRAP(pthread_spin_lock);
SHIM_WRAP(pthread_spin_trylock);
SHIM_WRAP(pthread_spin_unlock);

#ifdef __i386__
#include <stdlib.h>

struct wrapper_args {
  void* (*start_routine)(void*);
  void* arg;
};

// helps with stack alignment crashes in steamclient.so
static void* pthread_create_start_routine_wrapper(void* arg) {
  LOG_ENTRY("%p", arg);
  struct wrapper_args* wargs = (struct wrapper_args*)arg;
  LOG("start_routine = %p, arg = %p", wargs->start_routine, wargs->arg);
  void* ret = wargs->start_routine(wargs->arg);
  free(wargs);
  LOG_EXIT("%p", ret);
  return ret;
}

static int shim_pthread_create_impl(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg) {
  struct wrapper_args* wargs = malloc(sizeof(struct wrapper_args));
  wargs->start_routine = start_routine;
  wargs->arg = arg;
  return pthread_create(thread, attr, pthread_create_start_routine_wrapper, wargs);
}

SHIM_WRAP(pthread_create);
#endif

static void shim___pthread_register_cancel_impl(void* buf) {
  // do nothing
}

static void shim___pthread_unregister_cancel_impl(void* buf) {
  // do nothing
}

SHIM_WRAP(__pthread_register_cancel);
SHIM_WRAP(__pthread_unregister_cancel);

static int shim_pthread_sigmask_impl(int linux_how, const linux_sigset_t* restrict set, linux_sigset_t* restrict oset) {
  switch (linux_how) {
    case LINUX_SIG_BLOCK:   return pthread_sigmask(SIG_BLOCK,   (sigset_t*)set, (sigset_t*)oset);
    case LINUX_SIG_UNBLOCK: return pthread_sigmask(SIG_UNBLOCK, (sigset_t*)set, (sigset_t*)oset);
    case LINUX_SIG_SETMASK: return pthread_sigmask(SIG_SETMASK, (sigset_t*)set, (sigset_t*)oset);
    default:
      UNIMPLEMENTED_ARGS("%d", linux_how);
  }
}

SHIM_WRAP(pthread_sigmask);

static int shim_posix_spawnattr_getsigdefault_impl(const posix_spawnattr_t* restrict attr, linux_sigset_t* restrict sigdefault) {
  return posix_spawnattr_getsigdefault(attr, (sigset_t*)sigdefault);
}

static int shim_posix_spawnattr_getsigmask_impl(const posix_spawnattr_t* restrict attr, linux_sigset_t* restrict sigmask) {
  return posix_spawnattr_getsigmask(attr, (sigset_t*)sigmask);
}

static int shim_posix_spawnattr_setsigdefault_impl(posix_spawnattr_t* attr, const linux_sigset_t* restrict sigdefault) {
  return posix_spawnattr_setsigdefault(attr, (sigset_t*)sigdefault);
}

static int shim_posix_spawnattr_setsigmask_impl(posix_spawnattr_t* attr, const linux_sigset_t* restrict sigmask) {
  return posix_spawnattr_setsigmask(attr, (sigset_t*)sigmask);
}

SHIM_WRAP(posix_spawnattr_getsigdefault);
SHIM_WRAP(posix_spawnattr_getsigmask);
SHIM_WRAP(posix_spawnattr_setsigdefault);
SHIM_WRAP(posix_spawnattr_setsigmask);

// FMOD init workaround (https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=240043#c26)
static int shim_pthread_attr_setschedparam_impl(pthread_attr_t* attr, const linux_sched_param* param) {
  return 0;
}

SHIM_WRAP(pthread_attr_setschedparam);
