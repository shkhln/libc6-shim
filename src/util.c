#include <errno.h>
#include <string.h>
#include "shim.h"

bool str_starts_with(const char* str, const char* substr) {
  return strncmp(str, substr, strlen(substr)) == 0;
}

#define LINUX_EAGAIN     11
#define LINUX_ETIMEDOUT 110

int native_to_linux_errno(int error) {
  switch (error) {
    case EAGAIN:    return LINUX_EAGAIN;
    case ETIMEDOUT: return LINUX_ETIMEDOUT;
    //TODO: anything else?
    default:
      return error;
  }
}
