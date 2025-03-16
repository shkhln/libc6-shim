#include <glob.h>
#include "../shim.h"
#include "glob.h"

static int linux_to_native_glob_flags(int linux_flags) {

  assert((linux_flags & ~KNOWN_LINUX_GLOB_FLAGS) == 0);

  int flags = 0;

  if (linux_flags & LINUX_GLOB_ERR)         flags |= GLOB_ERR;
  if (linux_flags & LINUX_GLOB_MARK)        flags |= GLOB_MARK;
  if (linux_flags & LINUX_GLOB_NOSORT)      flags |= GLOB_NOSORT;
  if (linux_flags & LINUX_GLOB_DOOFFS)      flags |= GLOB_DOOFFS;
  if (linux_flags & LINUX_GLOB_NOCHECK)     flags |= GLOB_NOCHECK;
  if (linux_flags & LINUX_GLOB_APPEND)      flags |= GLOB_APPEND;
  if (linux_flags & LINUX_GLOB_NOESCAPE)    flags |= GLOB_NOESCAPE;
  // LINUX_GLOB_PERIOD
  if (linux_flags & LINUX_GLOB_MAGCHAR)     flags |= GLOB_MAGCHAR;
  if (linux_flags & LINUX_GLOB_ALTDIRFUNC)  flags |= GLOB_ALTDIRFUNC;
  if (linux_flags & LINUX_GLOB_BRACE)       flags |= GLOB_BRACE;
  if (linux_flags & LINUX_GLOB_NOMAGIC)     flags |= GLOB_NOMAGIC;
  if (linux_flags & LINUX_GLOB_TILDE)       flags |= GLOB_TILDE;
  // LINUX_GLOB_ONLYDIR
  // LINUX_GLOB_TILDE_CHECK

  return flags;
}

static int shim_glob_impl(const char* restrict pattern, int linux_flags, int (*errfunc)(const char*, int), linux_glob_t* restrict pglob) {

  assert((linux_flags & LINUX_GLOB_ALTDIRFUNC) == 0); // not implemented
  assert(errfunc == NULL);                            // ditto

  glob_t out;
  out.gl_offs = pglob->gl_offs;

  int err = glob(pattern, linux_to_native_glob_flags(linux_flags), NULL, &out);

  pglob->gl_pathc = out.gl_pathc;
  pglob->gl_pathv = out.gl_pathv;
  pglob->gl_offs  = out.gl_offs;
  if (out.gl_flags & GLOB_MAGCHAR) {
    pglob->gl_flags = linux_flags | LINUX_GLOB_MAGCHAR;
  } else {
    pglob->gl_flags = linux_flags & ~LINUX_GLOB_MAGCHAR;
  }

  switch (err) {
    case 0:            return 0;
    case GLOB_NOSPACE: return LINUX_GLOB_NOSPACE;
    case GLOB_ABORTED: return LINUX_GLOB_ABORTED;
    case GLOB_NOMATCH: return LINUX_GLOB_NOMATCH;
    default:
      PANIC("Unknown glob err value: %d", err);
  }
}

static void shim_globfree_impl(linux_glob_t* pglob) {
  glob_t g;
  g.gl_pathc  = pglob->gl_pathc;
  g.gl_pathv  = pglob->gl_pathv;
  g.gl_matchc = 0;
  g.gl_offs   = pglob->gl_offs;
  g.gl_flags  = linux_to_native_glob_flags(pglob->gl_flags); // ?
  globfree(&g);
}

static int shim_glob64_impl(const char* restrict pattern, int linux_flags, int (*errfunc)(const char*, int), linux_glob64_t* restrict pglob) {
  return shim_glob_impl(pattern, linux_flags, errfunc, pglob);
}

static void shim_globfree64_impl(linux_glob64_t* pglob) {
  shim_globfree_impl(pglob);
}

SHIM_WRAP(glob);
SHIM_WRAP(glob64);
SHIM_WRAP(globfree);
SHIM_WRAP(globfree64);
