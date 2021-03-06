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
    uint8_t alpha_modifier,
    char mode)
{
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
    uint32_t final;
    __asm__ (
    "movb  %%sil, %%al ;"  // Move mode to spare register D
    "andb  $0xFE, %%al ;"  // AND with 0xFE to check non-mul methods
    "testb %%al , %%al ;"  // Check if zero (is mul required?)
    "jnz   1f;"            // Mul is not required, jump to table

    /* 0xFF   * 0x0100010001000100 = 0xFF00FF00FF00FF00
     * 0xFF   * 0x0101010101010101 = 0xFFFFFFFFFFFFFFFF
     * 0xFF   * 0x0001000100010001 = 0x00FF00FF00FF00FF
     * 0xFFFF * 0x0001000100010001 = 0xFFFFFFFFFFFFFFFF
     */

    // MMX setup
    "movq      $0x0000000100010001, %%r8;"
    "movq      $0x000000FF00FF00FF, %%r9;"

    // Generate alpha_mul and alpha_mul_neg
    "movl  %%ebx, %%eax;" // v---v
    "shrl  $24  , %%eax;" // Get top alpha value into %al
    "mulb  %%dl ;"        // Multiply the top alpha by the alpha modifier
    "shrw  $8   , %%ax ;" // Store only the high bits (shift right by 8)
    "mulq  %%r8;"         // Multiply by %r8 in order to repeat 3 times (RGB)

    "movq  %%rax, %%xmm2;" // Move into %xmm2 for future use
    "movq  %%r9 , %%rdx ;" // Move full-sub/full-add mask into %rfx
    "subq  %%rax, %%rdx ;" // Invert %rax to get alpha_mul_neg
    "movq  %%rdx, %%xmm3;" // Move the new inverted alpha_mul_neg into %xmm3

    "movd      %%ebx , %%xmm0;" // Move top  into %xmm0
    "movd      %%ecx , %%xmm1;" // Move base into %xmm1
    "pxor      %%xmm4, %%xmm4;" // Clear %xmm4
    "punpcklbw %%xmm4, %%xmm0;" // Interleave lower bytes of %xmm4 and %xmm0
    "punpcklbw %%xmm4, %%xmm1;" // Interleave lower bytes of %xmm4 and %xmm1
    "pmullw    %%xmm2, %%xmm0;" // Multiply %xmm0 by %xmm2 (alpha_mul)
    "pmullw    %%xmm3, %%xmm1;" // Multiply %xmm1 by %xmm3 (alpha_mul_neg)

    "punpcklqdq %%xmm0, %%xmm1;"
    "movq       %%r9  , %%xmm0;" // Move round-mask (r9) into %xmm0
    "paddw      %%xmm0, %%xmm1;" // TODO: Missing round-up for high bytes!!!
    "psrlw      $8    , %%xmm1;"
    "packuswb   %%xmm0, %%xmm1;"
    "movq       %%xmm1, %%rax ;"

    "andl      $0xFF000000, %%ebx;" // Erase top  RGB but not A
    "andl      $0xFF000000, %%ecx;" // Erase base RGB but not A
    "orl       %%eax, %%ebx;" // OR resultant RGB into base
    "shrq      $32  , %%rax;" // Shift right to get top's resultant RGB
    "orl       %%eax, %%ecx;" // OR resultant RGB into top
"1:;"
    "leaq   14f(%%rip), %%rdx;"
    "movslq (%%rdx,%%rsi,4), %%rax;"
    "addq   %%rdx, %%rax;"
    "jmp    *%%rax;"
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
    "movl  %%ecx, %%eax;"
    "andl  $0x00FFFFFF, %%ebx;"
    "addl  %%ebx, %%eax;"
    "jmp   5f;"
"8:;"
    "movl  %%ebx, %%eax;"
    "andl  $0xFF000000, %%eax;"
    "testl %%eax, %%eax;"
    "je    9f;"
    "andl  $0x00FFFFFF, %%ebx;"
    "andl  $0xFF000000, %%ecx;"
    "movl  %%ebx, %%eax;"
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
"5:;"
    : "+a" (final)
    : "b" (SR_RGBAtoWhole(pixel_top )),
      "c" (SR_RGBAtoWhole(pixel_base)),
      "d" (alpha_modifier),
      "S" (mode)
    : "cc"   , "%r9"  , "%xmm0", "%xmm1",
      "%xmm2", "%xmm3", "%xmm4", "%r8"  );
#else
    register uint32_t final, pixel_base_whole, pixel_top_whole = 0;
    uint16_t alpha_mul, alpha_mul_neg;

    if (mode != SR_BLEND_ADDITIVE && mode != SR_BLEND_XOR) goto srbl_nomul;

    alpha_mul     = (pixel_top.alpha * alpha_modifier) >> 8;
    alpha_mul_neg = 255 - alpha_mul;

    pixel_top.rgb.red   = ((pixel_top.rgb.red   * alpha_mul) + 255) >> 8;
    pixel_top.rgb.green = ((pixel_top.rgb.green * alpha_mul) + 255) >> 8;
    pixel_top.rgb.blue  = ((pixel_top.rgb.blue  * alpha_mul) + 255) >> 8;

    pixel_base.rgb.red   = ((pixel_base.rgb.red   * alpha_mul_neg) + 255) >> 8;
    pixel_base.rgb.green = ((pixel_base.rgb.green * alpha_mul_neg) + 255) >> 8;
    pixel_base.rgb.blue  = ((pixel_base.rgb.blue  * alpha_mul_neg) + 255) >> 8;

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