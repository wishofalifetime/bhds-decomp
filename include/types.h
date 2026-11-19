#ifndef BHDS_TYPES_H
#define BHDS_TYPES_H

/* The build defines exactly one of VERSION_JP, VERSION_US, VERSION_EU. */
#if !defined(VERSION_JP) && !defined(VERSION_US) && !defined(VERSION_EU)
#error "Define VERSION_JP, VERSION_US or VERSION_EU"
#endif

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned long       u32;
typedef unsigned long long  u64;
typedef signed char         s8;
typedef signed short        s16;
typedef signed long         s32;
typedef signed long long    s64;

typedef volatile u8         vu8;
typedef volatile u16        vu16;
typedef volatile u32        vu32;

typedef int                 BOOL;
#define TRUE  1
#define FALSE 0

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef void (*VoidFunc)(void);

#endif /* BHDS_TYPES_H */
