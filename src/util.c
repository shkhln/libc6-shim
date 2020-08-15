#include <errno.h>
#include <string.h>
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
