#include <assert.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../shim.h"

#ifdef __i386__

struct linux_dirent {
  uint32_t  d_ino;
  uint32_t  _pad1;
  uint16_t  d_reclen;
  uint8_t   d_type;
  char      d_name[256];
};

#endif

#ifdef __x86_64__

struct linux_dirent {
  uint64_t  d_ino;
  uint64_t  _pad1;
  uint16_t  d_reclen;
  uint8_t   d_type;
  char      d_name[256];
};

#endif

struct dir_dirent {
  DIR* dirp;
  struct linux_dirent* direntp;
};

#define DIRENT_POOL_SIZE 1000

static struct dir_dirent opened_entries[DIRENT_POOL_SIZE];

struct linux_dirent* shim_readdir_impl(DIR* dirp) {

  struct dirent* entry = readdir(dirp);

  if (entry == NULL)
    return NULL;

  struct linux_dirent* e2 = NULL;

  for (int i = 0; i < DIRENT_POOL_SIZE; i++) {

    if (opened_entries[i].dirp == NULL) {

      e2 = malloc(sizeof(struct linux_dirent));

      opened_entries[i].dirp    = dirp;
      opened_entries[i].direntp = e2;

      break;
    }
  }

  assert(e2 != NULL);

  e2->d_ino    = entry->d_ino;
  e2->d_reclen = entry->d_reclen;
  e2->d_type   = entry->d_type;

  strncpy(e2->d_name, entry->d_name, sizeof(e2->d_name));

  return e2;
}

int shim_readdir_r_impl(DIR* dirp, struct dirent* entry, struct dirent** result) {
  UNIMPLEMENTED();
}

int shim_closedir_impl(DIR* dirp) {

  for (int i = 0; i < DIRENT_POOL_SIZE; i++) {

    if (opened_entries[i].dirp != NULL) {

      free(opened_entries[i].direntp);

      opened_entries[i].dirp    = NULL;
      opened_entries[i].direntp = NULL;
    }
  }

  return closedir(dirp);
}

int shim_alphasort_impl(const struct dirent** d1, const struct dirent** d2) {
  UNIMPLEMENTED();
}
