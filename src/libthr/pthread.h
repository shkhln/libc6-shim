#include <pthread.h>

struct shim_pthread_mutex {
  pthread_mutex_t _wrapped_mutex;
  uint32_t        _pad[2];
  uint32_t        linux_kind;
  pthread_mutex_t _init_mutex;
};

typedef struct shim_pthread_mutex linux_pthread_mutex_t;

#ifdef __i386__
_Static_assert(sizeof(struct shim_pthread_mutex) <= 24 /* sizeof(pthread_mutex_t) on glibc/Linux */, "");
#endif

#ifdef __x86_64__
_Static_assert(sizeof(struct shim_pthread_mutex) <= 40 /* sizeof(pthread_mutex_t) on glibc/Linux */, "");
#endif

typedef uint32_t linux_pthread_barrierattr_t;
typedef uint32_t linux_pthread_condattr_t;
typedef uint32_t linux_pthread_mutexattr_t;

_Static_assert(sizeof(pthread_rwlockattr_t) <= 8 /* sizeof(pthread_rwlockattr_t) on glibc/Linux */, "");

typedef pthread_rwlockattr_t linux_pthread_rwlockattr_t;

enum linux_pthread_mutextype {
  LINUX_PTHREAD_MUTEX_NORMAL     = 0,
  LINUX_PTHREAD_MUTEX_RECURSIVE  = 1,
  LINUX_PTHREAD_MUTEX_ERRORCHECK = 2
};

enum linux_pthread_inheritsched {
  LINUX_PTHREAD_INHERIT_SCHED  = 0,
  LINUX_PTHREAD_EXPLICIT_SCHED = 1
};
