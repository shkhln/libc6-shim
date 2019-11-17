#include <stdint.h>

struct linux_utsname {
  char sysname[65];
  char nodename[65];
  char release[65];
  char version[65];
  char machine[65];
  char domainname[65];
};

typedef struct linux_utsname linux_utsname;
