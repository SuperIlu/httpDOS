/* usuals.h -- useful typedefs */
#ifndef USUALS_H
#define USUALS_H

#include <limits.h>

#if UCHAR_MAX == 0xFF
typedef unsigned char byte;	/* 8-bit byte */
#else
#error No 8-bit type found
#endif

#if UINT_MAX == 0xFFFFFFFF
typedef unsigned int word32;	/* 32-bit word */
#elif ULONG_MAX == 0xFFFFFFFF
typedef unsigned long word32;
#else
#error No 32-bit type found
#endif

#endif /* USUALS_H */
