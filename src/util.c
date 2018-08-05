#include <string.h>
#include "shim.h"

bool str_starts_with(const char* str, const char* substr) {
  return strncmp(str, substr, strlen(substr)) == 0;
}
