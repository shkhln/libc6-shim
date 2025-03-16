#include <stddef.h>

#define LINUX_GLOB_ERR         0x0001
#define LINUX_GLOB_MARK        0x0002
#define LINUX_GLOB_NOSORT      0x0004
#define LINUX_GLOB_DOOFFS      0x0008
#define LINUX_GLOB_NOCHECK     0x0010
#define LINUX_GLOB_APPEND      0x0020
#define LINUX_GLOB_NOESCAPE    0x0040
#define LINUX_GLOB_PERIOD      0x0080
#define LINUX_GLOB_MAGCHAR     0x0100
#define LINUX_GLOB_ALTDIRFUNC  0x0200
#define LINUX_GLOB_BRACE       0x0400
#define LINUX_GLOB_NOMAGIC     0x0800
#define LINUX_GLOB_TILDE       0x1000
#define LINUX_GLOB_ONLYDIR     0x2000
#define LINUX_GLOB_TILDE_CHECK 0x4000

#define KNOWN_LINUX_GLOB_FLAGS ( \
  LINUX_GLOB_ERR               | \
  LINUX_GLOB_MARK              | \
  LINUX_GLOB_NOSORT            | \
  LINUX_GLOB_DOOFFS            | \
  LINUX_GLOB_NOCHECK           | \
  LINUX_GLOB_APPEND            | \
  LINUX_GLOB_NOESCAPE          | \
  /*LINUX_GLOB_PERIOD*/ 0      | \
  LINUX_GLOB_MAGCHAR           | \
  LINUX_GLOB_ALTDIRFUNC        | \
  LINUX_GLOB_BRACE             | \
  LINUX_GLOB_NOMAGIC           | \
  LINUX_GLOB_TILDE             | \
  /*LINUX_GLOB_ONLYDIR*/ 0     | \
  /*LINUX_GLOB_TILDE_CHECK*/ 0   \
)

#define LINUX_GLOB_NOSPACE 1
#define LINUX_GLOB_ABORTED 2
#define LINUX_GLOB_NOMATCH 3

struct shim_glob {
  size_t gl_pathc;
  char** gl_pathv;
  size_t gl_offs;
  int    gl_flags;
};

typedef struct shim_glob linux_glob_t;
typedef struct shim_glob linux_glob64_t;
