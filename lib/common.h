#ifndef LIB_COMMON_H
#define LIB_COMMON_H
#include <cinttypes>
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef float f32;
typedef double f64;

#define THROW_IF(x, y) if (x) {throw y;}

#endif
