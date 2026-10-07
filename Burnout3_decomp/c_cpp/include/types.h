#ifndef TYPES_H
#define TYPES_H

/* Fixed-size integer types for the EE (CodeWarrior: int and long are 32-bit, long long is 64-bit). */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;

#ifndef NULL
#define NULL 0
#endif

#endif
