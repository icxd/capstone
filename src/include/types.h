#ifndef TYPES_H
#define TYPES_H

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef long isz;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned long usz;

#define null ((void *)0)

typedef enum { false = 0, true = 1 } bool;

#define ALIGNED(n) __attribute__((aligned(n)))
#define PACKED __attribute__((packed))
#define NORETURN __attribute__((noreturn))
#define WEAK __attribute__((weak))
#define ALWAYS_INLINE __attribute__((always_inline))

#endif // !TYPES_H