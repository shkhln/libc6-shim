#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define __HEAD(head, ...) head
#define __TAIL(head, ...) __VA_ARGS__

#ifdef DEBUG

#define LOG(...)      fprintf(stderr, __VA_ARGS__)
#define LOG_ARGS(...) fprintf(stderr, "%s("    __HEAD(__VA_ARGS__) ")\n", __func__, __TAIL(__VA_ARGS__))
#define LOG_RES(...)  fprintf(stderr, "%s -> " __HEAD(__VA_ARGS__)  "\n", __func__, __TAIL(__VA_ARGS__))

#else

#define LOG(...)
#define LOG_ARGS(...)
#define LOG_RES(...)

#endif

#define UNIMPLEMENTED()         { fprintf(stderr, "%s is not implemented\n", __func__);                                               assert(0); }
#define UNIMPLEMENTED_ARGS(...) { fprintf(stderr, "%s(" __HEAD(__VA_ARGS__) ") is not implemented\n", __func__, __TAIL(__VA_ARGS__)); assert(0); }

bool str_starts_with(const char* str, const char* substr);

typedef off_t linux_off64_t;

#ifdef __i386__
typedef int32_t linux_off_t;
#endif

#ifdef __x86_64__
typedef int64_t linux_off_t;
#endif
