#include <string.h>
#include <sys/utsname.h>
#include "../../shim.h"
#include "utsname.h"

static int shim_uname_impl(struct linux_utsname* linux_name) {

  struct utsname name;

  int err = uname(&name);
  if (err == 0) {
    memset(linux_name, 0, sizeof(struct linux_utsname));

    strlcpy(linux_name->machine,
#ifdef __x86_64__
      "x86_64"
#elif defined(__i386__)
      access("/libexec/ld-elf32.so.1", F_OK) == 0 ? "x86_64" : name.machine
#else
      name.machine
#endif
      , 65);

    strlcpy(linux_name->sysname,    name.sysname,  65);
    strlcpy(linux_name->nodename,   name.nodename, 65);
    strlcpy(linux_name->release,    name.release,  65);
    strlcpy(linux_name->version,    name.version,  65);
    strlcpy(linux_name->domainname, "",            65);
  }

  return err;
}

SHIM_WRAP(uname);
