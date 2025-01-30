#include "../shim.h"
#include "termios.h"

static int shim_tcgetattr_impl(int fd, linux_termios* t) {
  return -1;
}

SHIM_WRAP(tcgetattr);
