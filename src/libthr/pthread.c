#include <assert.h>
#include <pthread.h>
#include <pthread_np.h>
#include "../shim.h"

int shim_pthread_setname_np(pthread_t tid, const char* name) {
  LOG_ARGS("%p, %s",  tid, name);
  pthread_set_name_np(tid, name);
  return 0;
}

int shim_pthread_getname_np(pthread_t tid, char* name, size_t len) {
  UNIMPLEMENTED();
}

int shim_register_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void), void* dso_handle) {
  LOG_ARGS("%p, %p, %p, %p", prepare, parent, child, dso_handle);
  int err = pthread_atfork(prepare, parent, child);
  LOG_RES("%d", err);
  return err;
}

int shim_pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared) {
  LOG_ARGS("%p, %d",  attr, pshared);
  int err = pthread_mutexattr_setpshared(attr, pshared);
  LOG_RES("%d", err);
  return err;
}

SYM_EXPORT(shim_pthread_mutexattr_setpshared, pthread_mutexattr_setpshared);
