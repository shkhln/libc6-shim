#include <stdint.h>

#ifdef __i386__

struct linux_statfs {
  uint32_t f_type;
  uint32_t f_bsize;
  uint32_t f_blocks;
  uint32_t f_bfree;
  uint32_t f_bavail;
  uint32_t f_files;
  uint32_t f_ffree;
  uint64_t f_fsid;
  uint32_t f_namelen;
  uint32_t f_frsize;
  uint32_t f_flags;
  uint32_t f_spare[4];
};

struct linux_statfs64 {
  uint32_t f_type;
  uint32_t f_bsize;
  uint64_t f_blocks;
  uint64_t f_bfree;
  uint64_t f_bavail;
  uint64_t f_files;
  uint64_t f_ffree;
  uint64_t f_fsid;
  uint32_t f_namelen;
  uint32_t f_frsize;
  uint32_t f_flags;
  uint32_t f_spare[4];
};

typedef struct linux_statfs   linux_statfs;
typedef struct linux_statfs64 linux_statfs64;

#endif

#ifdef __x86_64__

struct linux_statfs {
  uint64_t f_type;
  uint64_t f_bsize;
  uint64_t f_blocks;
  uint64_t f_bfree;
  uint64_t f_bavail;
  uint64_t f_files;
  uint64_t f_ffree;
  uint64_t f_fsid;
  uint64_t f_namelen;
  uint64_t f_frsize;
  uint64_t f_flags;
  uint64_t f_spare[4];
};

typedef struct linux_statfs linux_statfs;
typedef struct linux_statfs linux_statfs64;

#endif
