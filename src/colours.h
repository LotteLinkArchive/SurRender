#ifndef SURCL_HEADER_FILE
#define SURCL_HEADER_FILE
#include "glbl.h"

// Colours must be stored in little endian format
#pragma scalar_storage_order little-endian

typedef uint8_t  SRu8x8  __attribute__ ((vector_size (8 )));
typedef uint8_t  SRu8x4  __attribute__ ((vector_size (4 )));
typedef uint16_t SRu16x8 __attribute__ ((vector_size (16)));
typedef uint16_t SRu16x4 __attribute__ ((vector_size (8 )));

typedef union {
    struct {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;
    } chn;
    uint32_t whole;
    SRu8x4 splitvec;
} SR_RGBAPixel;

typedef union {
    uint64_t whole;
    struct {
        uint32_t left;
        uint32_t right;
    } components;
    SRu8x8 splitvec;
} SR_RGBADoublePixel;

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
    SR_BLEND_DIRECT_XOR_ALL,
    // Like additive blending, but doesn't change base alpha and doesn't
    // multiply values. Can overflow. Use it to paint colour onto black.
    SR_BLEND_ADDITIVE_PAINT,
    // Depending on the alpha value of the top layer, invert the base colours
    SR_BLEND_INVERTED_DRAW
};

// Create an RGBA colour value.
inline __attribute__((always_inline)) SR_RGBAPixel SR_CreateRGBA(
    register uint8_t red,
    register uint8_t green,
    register uint8_t blue,
    register uint8_t alpha)
{
    SR_RGBAPixel temp = {
        .chn.red = red,
        .chn.green = green,
        .chn.blue = blue,
        .chn.alpha = alpha
    };
    return temp;
}

// Blend RGBA values
// Use mode provided by SR_BlendingModes
// Usually, you'll want to set alpha_modifier to 255.
inline __attribute__((always_inline)) SR_RGBAPixel SR_RGBABlender(
    SR_RGBAPixel pixel_base,
    SR_RGBAPixel pixel_top,
    uint8_t alpha_modifier,
    char mode)
{
    SR_RGBAPixel final;

    if (mode > SR_BLEND_ADDITIVE) goto srbl_nomul;

    uint8_t alpha_mul, alpha_mul_neg;
    alpha_mul     = ((uint16_t)pixel_top.chn.alpha * alpha_modifier) >> 8;
    alpha_mul_neg = ~alpha_mul;

    SRu16x8 buffer = {
        alpha_mul_neg, alpha_mul_neg, alpha_mul_neg, 255,
        alpha_mul,     alpha_mul,     alpha_mul,     255};
    SR_RGBADoublePixel merge = {
        .components.right = pixel_top.whole,
        .components.left  = pixel_base.whole};

    merge.splitvec = __builtin_convertvector(((
            buffer * __builtin_convertvector(merge.splitvec, SRu16x8)
        ) + 255) >> 8, SRu8x8);
    pixel_top.whole  = merge.components.right;
    pixel_base.whole = merge.components.left;

srbl_nomul:
    switch (mode) {
    case SR_BLEND_DIRECT_XOR: // Mul skipped by goto srbl_nomul
    case SR_BLEND_XOR: // Mul version
        final.whole = pixel_base.whole ^ (pixel_top.whole & 0x00FFFFFF);

        break;
    default:
    case SR_BLEND_ADDITIVE_PAINT:
    case SR_BLEND_ADDITIVE:
        final.whole = (pixel_base.whole & 0xFF000000) | (
                      (pixel_base.whole & 0x00FFFFFF) +
                      (pixel_top.whole  & 0x00FFFFFF));

        break;
    case SR_BLEND_OVERLAY:
        if (((uint16_t)alpha_modifier * pixel_top.chn.alpha) >= 255)
            final.whole  = (pixel_top.whole  & 0x00FFFFFF) |
                           (pixel_base.whole & 0xFF000000);
        else final.whole = pixel_base.whole;

        break;
    case SR_BLEND_INVERT_DROP:
        final.whole = (pixel_base.whole & 0x00FFFFFF) |
                     ~(pixel_top.whole  & 0xFF000000);

        break;
    case SR_BLEND_DROP:
        final.whole = (pixel_base.whole & 0x00FFFFFF) |
                      (pixel_top.whole  & 0xFF000000);

        break;
    case SR_BLEND_REPLACE:
        final.whole = pixel_top.whole;

        break;
    case SR_BLEND_DIRECT_XOR_ALL:
        final.whole = pixel_base.whole ^ pixel_top.whole;

        break;
    case SR_BLEND_INVERTED_DRAW:
        final.whole = pixel_base.whole - ((
            ((uint16_t)pixel_top.chn.alpha * alpha_modifier)
        >> 8) * 0x00010101);

        break;
    }
    return final;
}
#endif