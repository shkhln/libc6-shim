#include <errno.h>
#include <semaphore.h>
#include "../shim.h"
#include "time.h"

int shim_sem_trywait_impl(sem_t* sem) {
  int err = sem_trywait(sem);
  if (err == -1) {
    errno = native_to_linux_errno(errno);
  }
  return err;
}

int shim_sem_timedwait_impl(sem_t* sem, const linux_timespec* abs_timeout) {
  int err = sem_timedwait(sem, abs_timeout);
  if (err == -1) {
    errno = native_to_linux_errno(errno);
  }
  return err;
}

SHIM_WRAP(sem_trywait);
SHIM_WRAP(sem_timedwait);

int shim_sem_getvalue_impl(sem_t* restrict sem, int* restrict sval) {
  UNIMPLEMENTED();
}

SHIM_WRAP(sem_getvalue);
