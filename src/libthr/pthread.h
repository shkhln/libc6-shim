#include <pthread.h>

struct shim_pthread_mutex {
  pthread_mutex_t _wrapped_mutex;
  uint32_t        _pad[2];
  uint32_t        linux_kind;
  pthread_mutex_t _init_mutex;
};

#ifdef __i386__
  _Static_assert(sizeof(pthread_mutex_t) == 4, "");
  _Static_assert(sizeof(struct shim_pthread_mutex) <= 24 /* sizeof(pthread_mutex_t) on glibc/Linux */, "");
#endif

#ifdef __x86_64__
  _Static_assert(sizeof(pthread_mutex_t) == 8, "");
  _Static_assert(sizeof(struct shim_pthread_mutex) <= 40 /* sizeof(pthread_mutex_t) on glibc/Linux */, "");
#endif

typedef struct shim_pthread_mutex linux_pthread_mutex_t;
