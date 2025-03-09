#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include "shim.h"

#ifdef DEBUG
__thread int what_was_that_error;
#endif

bool str_starts_with(const char* str, const char* substr) {
  return strncmp(str, substr, strlen(substr)) == 0;
}

#define LINUX_EAGAIN     11
#define LINUX_ENOSYS     38
#define LINUX_ETIMEDOUT 110

int native_to_linux_errno(int error) {
  switch (error) {
    case EAGAIN:    return LINUX_EAGAIN;
    case ENOSYS:    return LINUX_ENOSYS;
    case ETIMEDOUT: return LINUX_ETIMEDOUT;
    //TODO: anything else?
    default:
      return error;
  }
}

int linux_to_native_errno(int error) {
  switch (error) {
    case LINUX_EAGAIN:    return EAGAIN;
    case LINUX_ENOSYS:    return ENOSYS;
    case LINUX_ETIMEDOUT: return ETIMEDOUT;
    default:
      return error;
  }
}

static char** procfs_redirects = NULL;

__attribute__((constructor))
static void init_redirects() {

  int capacity = 5;
  procfs_redirects = malloc(sizeof(char*) * capacity);

  char linux_emul_path[MAXPATHLEN];
  size_t linux_emul_path_size = MAXPATHLEN;

  int err = sysctlbyname("compat.linux.emul_path", linux_emul_path, &linux_emul_path_size, NULL, 0);
  assert(err == 0);

  int i = 0;

  // CUDA init
  procfs_redirects[i++] = "/proc/self/maps";
  procfs_redirects[i++] = "/dev/null";

  // Steam
  //~ asprintf(&procfs_redirects[i++], "/proc/%d/status", getpid());
  //~ asprintf(&procfs_redirects[i++], "%s/proc/%d/status", linux_emul_path, getpid());

  procfs_redirects[i++] = "/proc/cpuinfo";
  asprintf(&procfs_redirects[i++], "%s/%s", linux_emul_path, "/proc/cpuinfo");

  //~ procfs_redirects[i++] = "/proc/net/route";
  //~ asprintf(&procfs_redirects[i++], "%s/%s", linux_emul_path, "/proc/net/route");

  //~ procfs_redirects[i++] = "/proc/version";
  //~ asprintf(&procfs_redirects[i++], "%s/%s", linux_emul_path, "/proc/version");

  procfs_redirects[i++] = NULL;
  assert(i <= capacity);
}

const char* redirect(const char* path) {

  if (strcmp("/dev/nvidia-uvm", path) == 0) {
    return "/dev/null";
  }

  if (str_starts_with(path, "/proc/")) {

    for (int i = 0; procfs_redirects[i] != NULL; i += 2) {
      if (strcmp(path, procfs_redirects[i]) == 0) {
        return procfs_redirects[i + 1];
      }
    }

    return NULL;
  }

  if (str_starts_with(path, "/sys/")) {
    return NULL;
  }

  // Steam
  if (strcmp(path, "/etc/ssl/certs/ca-certificates.crt") == 0) {
    return "/etc/ssl/cert.pem";
  }

  return path;
}

char* proc_self_exe_override = NULL;

__attribute__((constructor))
static void init_proc_self_exe_override() {
  proc_self_exe_override = getenv("SHIM_PROC_SELF_EXE");
}
