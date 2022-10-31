#ifndef STRUCT_TYPEDEF_H
#define STRUCT_TYPEDEF_H

typedef signed char int8_t;
typedef signed short int int16_t;

#ifndef _INT32_T_DECLARED
typedef signed int int32_t;
#define _INT32_T_DECLARED
#endif

typedef signed long long int64_t;

/* exact-width unsigned integer types */
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

#ifndef _UINT32_T_DECLARED
typedef unsigned int uint32_t;
#define _UINT32_T_DECLARED
#endif

typedef unsigned long long uint64_t;
typedef unsigned char bool_t;
typedef float fp32;
typedef double fp64;

#endif



