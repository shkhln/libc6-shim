#include <spawn.h>
#include "../shim.h"
#include "signal.h"

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
