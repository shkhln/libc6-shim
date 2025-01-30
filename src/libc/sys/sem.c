#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "../../shim.h"

#define LINUX_IPC_RMID 0
#define LINUX_IPC_SET  1
#define LINUX_IPC_STAT 2
#define LINUX_GETPID  11
#define LINUX_GETVAL  12
#define LINUX_GETALL  13
#define LINUX_GETNCNT 14
#define LINUX_GETZCNT 15
#define LINUX_SETVAL  16
#define LINUX_SETALL  17

static int linux_to_native_sem_cmd(int cmd) {
  switch (cmd) {
    case LINUX_IPC_RMID: return IPC_RMID;
    //~ case LINUX_IPC_SET:  return IPC_SET;
    //~ case LINUX_IPC_STAT: return IPC_STAT;
    case LINUX_GETPID:   return GETPID;
    //~ case LINUX_GETVAL:   return GETVAL;
    //~ case LINUX_GETALL:   return GETALL;
    case LINUX_GETNCNT:  return GETNCNT;
    case LINUX_GETZCNT:  return GETZCNT;
    //~ case LINUX_SETVAL:   return SETVAL;
    //~ case LINUX_SETALL:   return SETALL;
    default:
      assert(0);
  }
}

static int shim_semctl_impl(int semid, int semnum, int cmd, va_list args) {

  if (cmd == LINUX_SETVAL) {
    return semctl(semid, semnum, SETVAL, va_arg(args, int));
  }

  return semctl(semid, semnum, linux_to_native_sem_cmd(cmd));
}

SHIM_WRAP(semctl);

typedef struct sembuf linux_sembuf;

// IPC_NOWAIT and SEM_UNDO have same values on Linux and FreeBSD
static int shim_semop_impl(int semid, linux_sembuf* array, size_t nops) {
  int err = semop(semid, array, nops);
  if (err == -1) {
    errno = native_to_linux_errno(errno);
  }
  return err;
}

SHIM_WRAP(semop);
