#include <assert.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Just enough stuff to keep Steam from complaining.
 */

#ifdef __i386__
#define FUTEX_OFFSET -20
#endif

#ifdef __x86_64__
#define FUTEX_OFFSET -32
#endif

struct robust_list_head {
  void* list;
  long  futex_offset;
  void* list_op_pending;
};

struct fake_pthread {
  void*  robust_prev;
  struct robust_list_head list;
};

static __thread struct fake_pthread pthread = {};

long get_robust_list(int pid, struct robust_list_head** list_head, size_t* struct_len) {

  if (!(list_head && struct_len))
    return -1;

  assert(pid == 0);

  pthread.robust_prev          = &pthread.list;
  pthread.list.list            = &pthread.list;
  pthread.list.futex_offset    = FUTEX_OFFSET;
  pthread.list.list_op_pending = NULL;

  *list_head  = &pthread.list;
  *struct_len = sizeof(struct robust_list_head);

  return 0;
}
