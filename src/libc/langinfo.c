#include <langinfo.h>
#include "../shim.h"
#include "locale.h"

char* shim_nl_langinfo_l_impl(nl_item item, linux_locale_t loc) {
  UNIMPLEMENTED();
}

SHIM_WRAP(nl_langinfo_l);
