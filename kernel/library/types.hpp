#ifndef __LIBRARY__TYPES_HPP_
#define __LIBRARY__TYPES_HPP_

typedef signed char i8;
typedef unsigned char u8;
typedef signed short int i16;
typedef unsigned short int u16;
typedef signed int i32;
typedef unsigned int u32;
typedef signed long int i64;
typedef unsigned long int u64;

typedef unsigned long uptr;

#ifdef __SIZE_TYPE__
typedef __SIZE_TYPE__ size_t;
#else
typedef u64 size_t;
#endif

#endif