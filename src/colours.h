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

#ifdef SURCL_ALLOW_TYPE_PUNNING
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
    return SR_WholetoRGBA(final);
}
#endif