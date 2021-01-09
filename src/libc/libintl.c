#include "../shim.h"

char* shim_gettext_impl(const char* msgid) {
  return msgid;
}

SHIM_WRAP(gettext);
