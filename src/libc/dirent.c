#include <assert.h>
#include <dirent.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include "../shim.h"
#include "dirent.h"

struct shim_directory {
  DIR* dir;
  SLIST_HEAD(shim_dir_entry_list, shim_dir_entry) head;
  pthread_mutex_t mutex;
};

struct shim_dir_entry {
  SLIST_ENTRY(shim_dir_entry) entries;
  struct linux_dirent*   linux_entry;
  struct linux_dirent64* linux_entry64;
};

static struct shim_directory* create_shim_dir(DIR* dir) {

  struct shim_directory* shim_dir = malloc(sizeof(struct shim_directory));

  shim_dir->dir = dir;

  SLIST_INIT(&shim_dir->head);

  int err = pthread_mutex_init(&shim_dir->mutex, NULL);
  assert(err == 0);

  return shim_dir;
}

static void destroy_shim_dir(struct shim_directory* shim_dir) {

  pthread_mutex_lock(&shim_dir->mutex);

  struct shim_dir_entry *shim_entry, *temp;
  SLIST_FOREACH_SAFE(shim_entry, &shim_dir->head, entries, temp) {

    SLIST_REMOVE(&shim_dir->head, shim_entry, shim_dir_entry, entries);

    if (shim_entry->linux_entry != NULL) {
      free(shim_entry->linux_entry);
    }

    if (shim_entry->linux_entry64 != NULL) {
      free(shim_entry->linux_entry64);
    }

    free(shim_entry);
  }

  pthread_mutex_unlock(&shim_dir->mutex);

  int err = pthread_mutex_destroy(&shim_dir->mutex);
  assert(err == 0);

  free(shim_dir);
}

static struct linux_dirent* insert_entry(struct shim_directory* shim_dir, struct dirent* entry) {

  struct linux_dirent* linux_entry = malloc(sizeof(struct linux_dirent));

  linux_entry->d_ino    = entry->d_ino;
  linux_entry->d_off    = entry->d_off;
  linux_entry->d_reclen = entry->d_reclen;
  linux_entry->d_type   = entry->d_type;

  strlcpy(linux_entry->d_name, entry->d_name, sizeof(linux_entry->d_name));

  struct shim_dir_entry* shim_entry = malloc(sizeof(struct shim_dir_entry));
  shim_entry->linux_entry   = linux_entry;
  shim_entry->linux_entry64 = NULL;

  pthread_mutex_lock(&shim_dir->mutex);

  SLIST_INSERT_HEAD(&shim_dir->head, shim_entry, entries);

  pthread_mutex_unlock(&shim_dir->mutex);

  return linux_entry;
}

static struct linux_dirent64* insert_entry64(struct shim_directory* shim_dir, struct dirent* entry) {

  struct linux_dirent64* linux_entry64 = malloc(sizeof(struct linux_dirent64));

  linux_entry64->d_ino    = entry->d_ino;
  linux_entry64->d_off    = entry->d_off;
  linux_entry64->d_reclen = entry->d_reclen;
  linux_entry64->d_type   = entry->d_type;

  strlcpy(linux_entry64->d_name, entry->d_name, sizeof(linux_entry64->d_name));

  struct shim_dir_entry* shim_entry = malloc(sizeof(struct shim_dir_entry));
  shim_entry->linux_entry   = NULL;
  shim_entry->linux_entry64 = linux_entry64;

  pthread_mutex_lock(&shim_dir->mutex);

  SLIST_INSERT_HEAD(&shim_dir->head, shim_entry, entries);

  pthread_mutex_unlock(&shim_dir->mutex);

  return linux_entry64;
}

struct shim_directory* shim_fdopendir_impl(int fd) {
  DIR* dir = fdopendir(fd);
  return dir != NULL ? create_shim_dir(dir) : NULL;
}

struct shim_directory* shim_opendir_impl(const char* filename) {
  DIR* dir = opendir(filename);
  return dir != NULL ? create_shim_dir(dir) : NULL;
}

struct linux_dirent* shim_readdir_impl(struct shim_directory* shim_dir) {
  struct dirent* entry = readdir(shim_dir->dir);
  return entry != NULL ? insert_entry(shim_dir, entry) : NULL;
}

struct linux_dirent64* shim_readdir64_impl(struct shim_directory* shim_dir) {
  struct dirent* entry = readdir(shim_dir->dir);
  return entry != NULL ? insert_entry64(shim_dir, entry) : NULL;
}

int shim_closedir_impl(struct shim_directory* shim_dir) {
  int err = closedir(shim_dir->dir);
  destroy_shim_dir(shim_dir);
  return err;
}

int shim_dirfd_impl(struct shim_directory* shim_dir) {
  return dirfd(shim_dir->dir);
}

void shim_rewinddir_impl(struct shim_directory* shim_dir) {
  rewinddir(shim_dir->dir);
}

void shim_seekdir_impl(struct shim_directory* shim_dir, long loc) {
  seekdir(shim_dir->dir, loc);
}

long shim_telldir_impl(struct shim_directory* shim_dir) {
  return telldir(shim_dir->dir);
}

int shim_alphasort_impl(const struct linux_dirent** d1, const struct linux_dirent** d2) {
  UNIMPLEMENTED();
}

int shim_readdir_r_impl(struct shim_directory* shim_dir, struct linux_dirent* linux_entry, struct linux_dirent** linux_result) {
  UNIMPLEMENTED();
}

int shim_scandir_impl(
  const char* dirname,
  struct linux_dirent*** namelist,
  int (*select)(const struct linux_dirent*),
  int (*compar)(const struct linux_dirent**, const struct linux_dirent**)
) {
  UNIMPLEMENTED();
}

typedef struct shim_directory linux_DIR;

SHIM_WRAP(alphasort);
SHIM_WRAP(closedir);
SHIM_WRAP(dirfd);
SHIM_WRAP(fdopendir);
SHIM_WRAP(opendir);
SHIM_WRAP(readdir);
SHIM_WRAP(readdir64);
SHIM_WRAP(readdir_r);
SHIM_WRAP(rewinddir);
SHIM_WRAP(scandir);
SHIM_WRAP(seekdir);
SHIM_WRAP(telldir);
