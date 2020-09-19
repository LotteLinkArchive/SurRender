/* Standardization of C types and addition of a few useful macros
 * Inspired by Terry Davis' HolyC
 * 
 * May he rest in peace.
 */

#ifndef HOLY_HEADER_FILE
#define HOLY_HEADER_FILE

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

// Vector definition macro
#define DEF_VECTYPE(name, element_type, elements) \
typedef element_type name __attribute__ \
((vector_size (sizeof(element_type) * elements)));
// TODO: Automatically round up to power of two for vector size

// Useful vector types
DEF_VECTYPE(R64VEC_4D, R64, 4)
DEF_VECTYPE(R64VEC_2D, R64, 2)
typedef R64VEC_4D  R64VEC_3D; // Only power of 2 vectors are allowed
DEF_VECTYPE(R32VEC_4D, R32, 4)
DEF_VECTYPE(R32VEC_2D, R32, 2)
typedef R32VEC_4D  R32VEC_3D;
DEF_VECTYPE(U32VEC_4D, U32, 4)
DEF_VECTYPE(U32VEC_2D, U32, 2)
typedef U32VEC_4D  U32VEC_3D;
DEF_VECTYPE(I32VEC_4D, I32, 4)
DEF_VECTYPE(I32VEC_2D, I32, 2)
typedef I32VEC_4D  I32VEC_3D;

// These specific ones are useful for colour manipulation
DEF_VECTYPE(U8x8, U8, 8)
DEF_VECTYPE(U8x4, U8, 4)
DEF_VECTYPE(U16x8, U16, 8)
DEF_VECTYPE(U16x4, U16, 4)

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
// TODO: Do this the other way round (e.g R64VEC_4D points to R64x4)

// Vector macros
#define hcl_vector_convert __builtin_convertvector
/* See https://github.com/simd-everywhere/simde
 * This is a much better library for doing vector stuff anywhere, but the good
 * news is that you ought to be able to do most things without it assuming
 * you're on GCC or Clang. (E.g vector shuffling can be done simply by
 * defining a new vector containing elements from other vectors, and the
 * compiler ought to optimize it automatically. This should work anywhere, but
 * it'll just look kinda ugly.)
 * 
 * In the future, this header may be expanded to create its own simple,
 * standardized vector interface.
 */

// Useful macros
#define MIN(a,b)         \
({ typeof (a) _a = (a);  \
    typeof (b) _b = (b); \
    _a < _b ? _a : _b; })
#define MAX(a,b)         \
({ typeof (a) _a = (a);  \
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
