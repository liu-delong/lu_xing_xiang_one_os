#ifndef __MODEL_DEF_H__
#define __MODEL_DEF_H__

#if defined(__CC_ARM) || defined(__CLANG_ARM) /* ARM Compiler 							*/
#include <stdarg.h>
#define SECTION(x) __attribute__((section(x)))
#define OS_UNUSED __attribute__((unused))
#define OS_USED __attribute__((used))
#define ALIGN(n) __attribute__((aligned(n)))

#define OS_WEAK __attribute__((weak))
#define os_inline static __inline

#elif defined(__GNUC__) /* GNU GCC Compiler */
#ifdef OS_USING_NEWLIB
#include <stdarg.h>
#else
/* the version of GNU GCC must be greater than 4.x */
typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)
#endif

#define SECTION(x) __attribute__((section(x)))
#define OS_UNUSED __attribute__((unused))
#define OS_USED __attribute__((used))
#define ALIGN(n) __attribute__((aligned(n)))
#define OS_WEAK __attribute__((weak))
#define os_inline static __inline

#elif defined(_WIN32)

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define SECTION(x) __attribute__((section(x)))
#define OS_UNUSED __attribute__((unused))
#define OS_USED __attribute__((used))
#define ALIGN(n) __attribute__((aligned(n)))
#define OS_WEAK __attribute__((weak))
#define os_inline static __inline

#endif

#endif
