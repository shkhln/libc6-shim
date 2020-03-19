#include <fenv.h>
#include "../shim.h"

#define LINUX_FE_DFL_ENV ((const fenv_t*)-1)

int shim_fesetenv_impl(const fenv_t* envp) {
  if (envp == LINUX_FE_DFL_ENV) {
    return fesetenv(FE_DFL_ENV);
  } else {
    return fesetenv(envp);
  }
}

SHIM_WRAP(fesetenv);
