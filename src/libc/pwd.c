#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include "../shim.h"

struct linux_passwd {
  char* pw_name;
  char* pw_passwd;
  uid_t pw_uid;
  gid_t pw_gid;
  char* pw_gecos;
  char* pw_dir;
  char* pw_shell;
};

typedef struct linux_passwd linux_passwd;

static linux_passwd lp;

static linux_passwd* shim_getpwuid_impl(uid_t uid) {

  struct passwd* p = getpwuid(uid);
  if (p == NULL) {
    return NULL;
  }

  lp.pw_name   = p->pw_name;
  lp.pw_passwd = p->pw_passwd;
  lp.pw_uid    = p->pw_uid;
  lp.pw_gid    = p->pw_gid;
  lp.pw_gecos  = p->pw_gecos;
  lp.pw_dir    = p->pw_dir;
  lp.pw_shell  = p->pw_shell;

  return &lp;
}

SHIM_WRAP(getpwuid);
