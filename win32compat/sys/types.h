#ifndef __TYPES_H
#define __TYPES_H

/* 7.18.1.1  Exact-width integer types */
#ifdef _MSC_VER
typedef signed __int8       int8_t;
typedef unsigned __int8   u_int8_t;
typedef unsigned __int8   u_char;
typedef __int16             int16_t;
typedef unsigned __int16  u_int16_t;
typedef __int32             int32_t;
typedef unsigned __int32  u_int32_t;
typedef  __int64            int64_t;
typedef unsigned  __int64 u_int64_t;
#else
typedef signed char          int8_t;
typedef unsigned char      u_int8_t;
typedef unsigned char      u_char;
typedef short                int16_t;
typedef unsigned short     u_int16_t;
typedef int                  int32_t;
typedef unsigned int       u_int32_t;
typedef long long            int64_t;
typedef unsigned  int64_t  u_int64_t;
#endif

typedef	long     time_t;
typedef long     _ssize_t;
typedef _ssize_t ssize_t;
typedef  u_int64_t off64_t;
typedef  u_int64_t fpos64_t;

#endif