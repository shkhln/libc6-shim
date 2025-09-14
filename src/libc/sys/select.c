#include <signal.h>
#include <sys/select.h>
#include "../../shim.h"
#include "../signal.h"
#include "../time.h"

static int shim_pselect_impl(int nfds, fd_set* restrict readfds, fd_set* restrict writefds, fd_set* restrict exceptfds,
  const linux_timespec* restrict timeout, const linux_sigset_t* restrict newsigmask)
{
  return pselect(nfds, readfds, writefds, exceptfds, timeout, (sigset_t*)newsigmask);
}

SHIM_WRAP(pselect);

long shim___fdelt_chk_impl(long d) {
  assert(0 <= d && d < 1024);
  return d / (8 * sizeof(long));
}

SHIM_WRAP(__fdelt_chk);
