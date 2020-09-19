/* Standard, short-hand fixed-width data types for C
 * Inspired by Terry Davis' HolyC types
 */

#ifndef STANDTYPE_HEADER_FILE
#define STANDTYPE_HEADER_FILE

#include <stdint.h>
#include <stdbool.h> // C99
#include <xmmintrin.h>
#include <pmmintrin.h>

// Integer maximums
#define U8_MAX  UINT8_MAX
#define I8_MIN  INT8_MIN
#define I8_MAX  INT8_MAX
#define U16_MAX UINT16_MAX
#define I16_MIN INT16_MIN
#define I16_MAX INT16_MAX
#define U32_MAX UINT32_MAX
#define I32_MIN INT32_MIN
#define I32_MAX INT32_MAX
#define U64_MAX UINT64_MAX
#define I64_MIN INT64_MIN
#define I64_MAX INT64_MAX

// Void type
typedef     void X0;

// Integer typedefs
typedef uint8_t  U8;
typedef  int8_t  I8;
typedef uint16_t U16;
typedef  int16_t I16;
typedef uint32_t U32;
typedef  int32_t I32;
typedef uint64_t U64;
typedef  int64_t I64;

// Real typedefs
typedef    float R32; // May not be exactly 32 or 64 bits depending on arch
typedef   double R64;

// Natural types (Spoiler: they're all 32-bit)
typedef      I32 INAT;
typedef      U32 UNAT;
typedef      R32 RNAT;

// Boolean types
typedef     bool U1;
#define BOOLIFY(a) ((a)?(true):(false)) // May not be needed with _Bool

// Useful vector types
typedef R64        R64VEC_4D     __attribute__ ((vector_size (32)));
typedef R64VEC_4D  R64VEC_3D; // Only power of 2 vectors are allowed
typedef R64        R64VEC_2D     __attribute__ ((vector_size (16)));

typedef R32        R32VEC_4D     __attribute__ ((vector_size (16)));
typedef R32VEC_4D  R32VEC_3D;
typedef R32        R32VEC_2D     __attribute__ ((vector_size (8 )));

typedef I32        I32VEC_4D     __attribute__ ((vector_size (16)));
typedef I32VEC_4D  I32VEC_3D;
typedef I32        I32VEC_2D     __attribute__ ((vector_size (8 )));

typedef U32        U32VEC_4D     __attribute__ ((vector_size (16)));
typedef U32VEC_4D  U32VEC_3D;
typedef U32        U32VEC_2D     __attribute__ ((vector_size (8 )));

// These specific ones are useful for colour manipulation
typedef U8         U8x8          __attribute__ ((vector_size (8 )));
typedef U8         U8x4          __attribute__ ((vector_size (4 )));
typedef U16        U16x8         __attribute__ ((vector_size (16)));
typedef U16        U16x4         __attribute__ ((vector_size (8 )));

// Sensible aliases
typedef R64VEC_4D  R64x4;
typedef R32VEC_4D  R32x4;
typedef I32VEC_4D  I32x4;
typedef U32VEC_4D  U32x4;

typedef R64VEC_3D  R64x3;
typedef R32VEC_3D  R32x3;
typedef I32VEC_3D  I32x3;
typedef U32VEC_3D  U32x3;

typedef R64VEC_2D  R64x2;
typedef R32VEC_2D  R32x2;
typedef I32VEC_2D  I32x2;
typedef U32VEC_2D  U32x2;

// Useful macros
#define MIN(a,b) \
({ typeof (a) _a = (a); \
    typeof (b) _b = (b); \
    _a < _b ? _a : _b; })
#define MAX(a,b) \
({ typeof (a) _a = (a); \
    typeof (b) _b = (b); \
    _a > _b ? _a : _b; })

#define forever for (;;)

// Fast unsigned power functions
#define UXPowGen(type, name)                                    \
inline __attribute__((always_inline)) type name(type x, type y) \
    { y = MAX(1, y); while ((y--) - 1) x *= x; return x; }

UXPowGen(U16, U16Pow)
UXPowGen(U32, U32Pow)
UXPowGen(U64, U64Pow)

#undef UXPowGen

#endif
