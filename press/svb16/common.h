#ifndef COMMON_H
#define COMMON_H

#if __cplusplus >= 201703L
#define SVB16_IF_CONSTEXPR if constexpr
#else
#define SVB16_IF_CONSTEXPR if
#endif

#define SVB_RESTRICT __restrict

#if defined(__x86_64__) || defined(_M_AMD64)  // x64
#define SVB16_X64
#elif defined(__arm__) || defined(__aarch64__)
#define SVB16_ARM
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#endif /* COMMON_H */
