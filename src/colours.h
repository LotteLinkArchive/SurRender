#ifndef SURCL_HEADER_FILE
#define SURCL_HEADER_FILE
#include "glbl.h"

// Colours must be stored in little endian format
#pragma scalar_storage_order little-endian

typedef struct SR_RGBPixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} SR_RGBPixel;

typedef struct SR_RGBAPixel {
    SR_RGBPixel rgb;
    uint8_t alpha;
} SR_RGBAPixel;

enum SR_BlendingModes {
    // XOR all RGB values
    SR_BLEND_XOR,
    // Add RGB values together with clamping
    SR_BLEND_ADDITIVE,
    // Rounded overlay approach (fastest)
    SR_BLEND_OVERLAY,
    // Replace base alpha with inverted top alpha
    SR_BLEND_INVERT_DROP,
    // Replace base alpha with top alpha
    SR_BLEND_DROP,
    // Replace entire top pixel with bottom pixel
    SR_BLEND_REPLACE,
    // Directly XOR the RGB channels without mutating the alpha
    SR_BLEND_DIRECT_XOR,
    // Directly XOR EVERYTHING (RGBA) without mutating the alpha
    SR_BLEND_DIRECT_XOR_ALL
};

// Construct an RGB colour value.
inline __attribute__((always_inline)) SR_RGBPixel SR_CreateRGB(
    register uint8_t red,
    register uint8_t green,
    register uint8_t blue)
{
    SR_RGBPixel temp = {red, green, blue}; return temp;
}

// Create an RGBA colour value.
inline __attribute__((always_inline)) SR_RGBAPixel SR_CreateRGBA(
    register uint8_t red,
    register uint8_t green,
    register uint8_t blue,
    register uint8_t alpha)
{
    SR_RGBAPixel temp = {{red, green, blue}, alpha};
    return temp;
}

// Conversion
#define SR_RGBAtoRGB(pixel) (pixel.rgb)

inline __attribute__((always_inline)) SR_RGBAPixel SR_RGBtoRGBA(
    SR_RGBPixel pixel,
    uint8_t alpha)
{
    SR_RGBAPixel temp;
    temp.rgb = pixel;
    temp.alpha = alpha;
    return temp;
}

#ifndef SURCL_PREVENT_TYPE_PUNNING
    inline __attribute__((always_inline)) uint32_t SR_RGBAtoWhole(
        SR_RGBAPixel pix) { return *(uint32_t *) &pix; }

    inline __attribute__((always_inline)) SR_RGBAPixel SR_WholetoRGBA(
        uint32_t pix) { return *(SR_RGBAPixel *) &pix; }
#else
    #define SR_RGBAtoWhole(pix) (  \
        ((pix).rgb.red        ) |  \
        ((pix).rgb.green <<  8) |  \
        ((pix).rgb.blue  << 16) |  \
        ((pix).alpha     << 24)    \
    )

    inline __attribute__((always_inline)) SR_RGBAPixel SR_WholetoRGBA(
        uint32_t pix)
    {
        SR_RGBAPixel temp;
        temp.rgb.red   = (pix & 0x000000FF)      ;
        temp.rgb.green = (pix & 0x0000FF00) >>  8;
        temp.rgb.blue  = (pix & 0x00FF0000) >> 16;
        temp.alpha     = (pix & 0xFF000000) >> 24;
        return temp;
    }
#endif

// Blend RGBA values
// Use mode provided by SR_BlendingModes
// Usually, you'll want to set alpha_modifier to 255.
inline __attribute__((always_inline)) SR_RGBAPixel SR_RGBABlender(
    SR_RGBAPixel pixel_base,
    SR_RGBAPixel pixel_top,
    register uint8_t alpha_modifier,
    register char mode)
{
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
    uint32_t final;
    __asm__ (
    "movb  %%dl , %%dil;"  // Move mode to spare register D
    "andb  $0xFE, %%dil;"  // AND with 0xFE to check non-mul methods
    "testb %%dil, %%dil;"  // Check if zero (is mul required?)
    "jnz   1f;"            // Mul is not required, jump to table

    // Generate alpha_mul and alpha_mul_neg
    "roll  $8   , %%ebx;"  // Replace top red with top alpha
    "movb  %%bl , %%al;"   // Move it to the accumulator
    "rorl  $8   , %%ebx;"  // Rollback the top pixel state
    "mulb  %%sil;"         // alpha modifier * top alpha -> AH and AL
    "shrw  $8   , %%ax;"   // Shift AH and AL to the right by 8 bits
    "movb  %%al , %%sil;"  // Move alpha_mul to alpha_modifier, not needed
    "movb  $0xFF, %%ah;"
    "subb  %%al , %%ah;"   // Turn accumulator into alpha_mul_neg
    "shrw  $8   , %%ax;"
    "movb  %%al , %%dil;"  // Move alpha_mul_neg into register D

    // Actual multiplication
    "orl   $0xFF000000, %%eax;"
"2:;"
    "movb  %%bl , %%al;"
    "mulb  %%sil;"
    "movb  %%ah , %%bl;"   // Run pixel_top.rgb.red through the alpha_mul
    "movb  %%bh , %%al;"
    "mulb  %%sil;"
    "movb  %%ah , %%bh;"   // Run pixel_top.rgb.green through the alpha_mul
    "rorl  $8   , %%ebx;"  // Rotate right to shift blue into green (BH)
    "movb  %%bh , %%al;"
    "mulb  %%sil;"
    "movb  %%ah , %%bh;"   // Run pixel_top.rgb.blue through the alpha_mul
    "roll  $8   , %%ebx;"  // Rotate left to shift green back into blue

    "andl  $0xFF000000, %%eax;"
    "cmpl  $0xFF000000, %%eax;"
    "xchg  %%sil, %%dil;"  // Swap SIL/DIL and EBX/ECX to handle pixel_base
    "xchg  %%ebx, %%ecx;"
    "je    3f;"            // Go back and do pixel_base now
    "jmp   1f;"            // Or, if it's done, finish up and start blending
"3:;"
    "andl  $0x00FFFFFF, %%eax;"
    "jmp   2b;"            // Prevent next loop and do the last iteration.
"1:;"
    "andl  $0   , %%eax;"  // Clear EAX
    "leaq  14f(%%rip), %%rdi;"
    "movslq (%%rdi,%%rdx,4), %%r9;"
    "addq  %%rdi, %%r9;"
    "jmp   *%%r9;"
"14:;"
    ".long 6f   - 14b;"    // SR_BLEND_XOR
    ".long 7f   - 14b;"    // SR_BLEND_ADDITIVE
    ".long 8f   - 14b;"    // SR_BLEND_OVERLAY
    ".long 10f  - 14b;"    // SR_BLEND_INVERT_DROP
    ".long 11f  - 14b;"    // SR_BLEND_DROP
    ".long 12f  - 14b;"    // SR_BLEND_REPLACE
    ".long 6f   - 14b;"    // SR_BLEND_DIRECT_XOR
    ".long 13f  - 14b;"    // SR_BLEND_DIRECT_XOR_ALL
"6:;"
    "andl  $0x00FFFFFF, %%ebx;"
    "xorl  %%ecx, %%ebx;"
    "movl  %%ebx, %%eax;"
    "jmp   5f;"
"7:;"
    "roll  $8   , %%ecx;"
    "movb  %%cl , %%al;"
    "shll  $24  , %%eax;"
    "rorl  $8   , %%ecx;"
    "andl  $0x00FFFFFF, %%ebx;"
    "andl  $0x00FFFFFF, %%ecx;"
    "addl  %%ebx, %%ecx;"
    "orl   %%ecx, %%eax;"
    "jmp   5f;"
"8:;"
    "cmpb  $0   , %%sil;"
    "je    9f;"
    "andl  $0x00FFFFFF, %%ebx;"
    "andl  $0xFF000000, %%ecx;"
    "orl   %%ebx, %%eax;"
    "orl   %%ecx, %%eax;"
    "jmp   5f;"
"9:;"
    "movl  %%ecx, %%eax;"
    "jmp   5f;"
"10:;"
    "movl  %%ecx, %%eax;"
    "andl  $0x00FFFFFF, %%eax;"
    "notl  %%ebx;"
    "andl  $0xFF000000, %%ebx;"
    "orl   %%ebx, %%eax;"
    "jmp   5f;"
"11:;"
    "movl  %%ecx, %%eax;"
    "andl  $0x00FFFFFF, %%eax;"
    "andl  $0xFF000000, %%ebx;"
    "orl   %%ebx, %%eax;"
    "jmp   5f;"
"12:;"
    "movl  %%ebx, %%eax;"
    "jmp   5f;"
"13:;"
    "xorl  %%ebx, %%ecx;"
    "movl  %%ecx, %%eax;"
    "jmp   5f;"
"5:;"
    : "=a" (final)
    : "b" (SR_RGBAtoWhole(pixel_top )),
        "c" (SR_RGBAtoWhole(pixel_base)),
        "d" (mode),
        "S" (alpha_modifier)
    : "%edi", "cc", "%r8" );
#else
    register uint32_t final, pixel_base_whole, pixel_top_whole = 0;
    uint16_t alpha_mul, alpha_mul_neg;

    if (mode != SR_BLEND_ADDITIVE && mode != SR_BLEND_XOR) goto srbl_nomul;

    alpha_mul     = (pixel_top.alpha * alpha_modifier) >> 8;
    alpha_mul_neg = 255 - alpha_mul;

    pixel_top.rgb.red   = (pixel_top.rgb.red   * alpha_mul) >> 8;
    pixel_top.rgb.green = (pixel_top.rgb.green * alpha_mul) >> 8;
    pixel_top.rgb.blue  = (pixel_top.rgb.blue  * alpha_mul) >> 8;

    pixel_base.rgb.red   = (pixel_base.rgb.red   * alpha_mul_neg) >> 8;
    pixel_base.rgb.green = (pixel_base.rgb.green * alpha_mul_neg) >> 8;
    pixel_base.rgb.blue  = (pixel_base.rgb.blue  * alpha_mul_neg) >> 8;

srbl_nomul:
    pixel_base_whole = SR_RGBAtoWhole(pixel_base);
    pixel_top_whole  = SR_RGBAtoWhole(pixel_top );

    switch (mode) {
    case SR_BLEND_DIRECT_XOR: // Mul skipped by goto srbl_nomul
    case SR_BLEND_XOR: // Mul version
        final = pixel_base_whole ^ (pixel_top_whole & 0x00FFFFFF);

        break;
    case SR_BLEND_ADDITIVE:
        final = pixel_base_whole & 0xFF000000;
        final |= (
            (pixel_base_whole & 0x00FFFFFF) +
            (pixel_top_whole  & 0x00FFFFFF));

        break;
    case SR_BLEND_OVERLAY:
        if (((uint16_t)alpha_modifier * pixel_top.alpha) > 16129)
            final = ((pixel_top_whole & 0x00FFFFFF) |
                (pixel_base_whole & 0xFF000000));
        else final = pixel_base_whole;

        break;
    case SR_BLEND_INVERT_DROP:
        final  = (pixel_base_whole & 0x00FFFFFF);
        final |= (255 - ((pixel_top_whole & 0xFF000000) >> 24)) << 24;

        break;
    case SR_BLEND_DROP:
        final = ((pixel_base_whole & 0x00FFFFFF) |
            (pixel_top_whole & 0xFF000000));

        break;
    case SR_BLEND_REPLACE:
        final = pixel_top_whole;

        break;
    case SR_BLEND_DIRECT_XOR_ALL:
        final = pixel_base_whole ^ pixel_top_whole;
        break;
    default:
        fprintf(stderr, "Invalid blending mode!\n");
        exit(EXIT_FAILURE);

        break;
    }

#endif
    return SR_WholetoRGBA(final);
}
#endif