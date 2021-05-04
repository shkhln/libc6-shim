#include "shim.h"

FILE* shim_stdin  = NULL;
FILE* shim_stdout = NULL;
FILE* shim_stderr = NULL;

SHIM_EXPORT(stdin);
SHIM_EXPORT(stdout);
SHIM_EXPORT(stderr);

#ifdef __i386__

FILE* shim__IO_stdin_  = NULL;
FILE* shim__IO_stdout_ = NULL;
FILE* shim__IO_stderr_ = NULL;

SHIM_EXPORT(_IO_stdin_);
SHIM_EXPORT(_IO_stdout_);
SHIM_EXPORT(_IO_stderr_);

#endif

FILE* shim__IO_2_1_stdin_  = NULL;
FILE* shim__IO_2_1_stdout_ = NULL;
FILE* shim__IO_2_1_stderr_ = NULL;

SHIM_EXPORT(_IO_2_1_stdin_);
SHIM_EXPORT(_IO_2_1_stdout_);
SHIM_EXPORT(_IO_2_1_stderr_);

char** shim_environ = NULL;

SHIM_EXPORT(environ);

char** shim___environ = NULL;
char** shim__environ  = NULL;

SHIM_EXPORT(__environ);
SHIM_EXPORT(_environ);

char* shim___progname = "???";
char* shim___progname_full = "???";
char* shim_program_invocation_name = "???";
char* shim_program_invocation_short_name = "???";

SHIM_EXPORT(__progname);
SHIM_EXPORT(__progname_full);
SHIM_EXPORT(program_invocation_name);
SHIM_EXPORT(program_invocation_short_name);

char* shim_optarg = NULL;
int   shim_optind = 1;
int   shim_optopt = 0;
int   shim_opterr = 1;

SHIM_EXPORT(optarg);
SHIM_EXPORT(optind);
SHIM_EXPORT(optopt);
SHIM_EXPORT(opterr);

#include <netinet/in.h>

const struct in6_addr shim_in6addr_any      = IN6ADDR_ANY_INIT;
const struct in6_addr shim_in6addr_loopback = IN6ADDR_LOOPBACK_INIT;

SHIM_EXPORT(in6addr_any);
SHIM_EXPORT(in6addr_loopback);
