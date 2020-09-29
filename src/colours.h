#ifndef SURCL_HEADER_FILE
#define SURCL_HEADER_FILE
#include "glbl.h"

typedef union {
	struct {
		U8 red;
		U8 green;
		U8 blue;
		U8 alpha;
	} chn;
	U32 whole;
	U8x4 splitvec;
} SR_RGBAPixel;

typedef union {
	U64 whole;
	struct {
		U32 left;
		U32 right;
	} uparts;
	struct {
		SR_RGBAPixel left;
		SR_RGBAPixel right;
	} srparts;
	U8x8 splitvec;
} SR_RGBADoublePixel;

enum SR_BlendingModes {
	/* Add RGB values together with clamping and multiplication */
	SR_BLEND_ADDITIVE,
	/* Like additive blending, but doesn't change base alpha and doesn't
	 * multiply values. Can overflow. Use it to paint colour onto black.
	 */
	SR_BLEND_ADDITIVE_PAINT,
	/* XOR all RGB values after multiplying them */
	SR_BLEND_XOR,
	/* Directly XOR the RGB channels without mutating the alpha */
	SR_BLEND_DIRECT_XOR,
	/* Rounded overlay approach (fastest) */
	SR_BLEND_OVERLAY,
	/* Replace base alpha with inverted top alpha */
	SR_BLEND_INVERT_DROP,
	/* Replace base alpha with top alpha */
	SR_BLEND_DROP,
	/* Replace entire top pixel with bottom pixel */
	SR_BLEND_REPLACE,
	/* Replace entire top pixel with button pixel but use the alpha modifier of RGBABlender mutiplied by the
	 * alpha of the top pixel.
	 * 
	 * This is very useful for rendering fonts into a temporary canvas quickly.
	 */
	SR_BLEND_REPLACE_WALPHA_MOD,
	/* Directly XOR EVERYTHING (RGBA) without mutating the alpha */
	SR_BLEND_DIRECT_XOR_ALL,
	/* Depending on the alpha value of the top layer, invert the base colours */
	SR_BLEND_INVERTED_DRAW,
	/* Keep the bottom's alpha but use the top's RGB values */
	SR_BLEND_PAINT
};

/* Create an RGBA colour value. */
inline	__attribute__((always_inline)) SR_RGBAPixel SR_CreateRGBA(
	U8 red,
	U8 green,
	U8 blue,
	U8 alpha)
{
	SR_RGBAPixel temp  = {
		.chn.red   = red,
		.chn.green = green,
		.chn.blue  = blue,
		.chn.alpha = alpha
	};
	return temp;
}

/* Blend RGBA values */
/* Use mode provided by SR_BlendingModes */
/* Usually, you'll want to set alpha_modifier to 255. */
inline	__attribute__((always_inline)) SR_RGBAPixel SR_RGBABlender(
	SR_RGBAPixel pixel_base,
	SR_RGBAPixel pixel_top,
	U8 alpha_modifier,
	I8 mode)
{
	SR_RGBAPixel final;

	U32 alpha_mul;
	SR_RGBADoublePixel buffer, merge;
	merge.uparts.right = pixel_top.whole;
	merge.uparts.left  = pixel_base.whole;

	#define PREALPHA alpha_mul = (((U16)merge.srparts.right.chn.alpha * alpha_modifier) >> 8);

	#define PREMULTIPLY \
	PREALPHA \
	buffer.whole = (0xFF00000000000000 | (0x0001010100010101 * alpha_mul)) ^ 0x00000000FFFFFFFF; \
	merge.splitvec = hcl_vector_convert((( \
		hcl_vector_convert(buffer.splitvec, U16x8)  * \
		hcl_vector_convert(merge.splitvec , U16x8)) + 255) >> 8, U8x8);

	switch (mode) {
	case SR_BLEND_ADDITIVE:
		PREMULTIPLY
	case SR_BLEND_ADDITIVE_PAINT:
		final.whole  = merge.uparts.left & 0xFF000000;
		merge.whole &= 0x00FFFFFF00FFFFFF;
		final.whole |= merge.uparts.left + merge.uparts.right;
		
		break;
	case SR_BLEND_XOR:
		PREMULTIPLY
	case SR_BLEND_DIRECT_XOR:
		final.whole = merge.uparts.left ^ (merge.uparts.right & 0x00FFFFFF);

		break;
	case SR_BLEND_OVERLAY:
		PREALPHA
		buffer.uparts.left  = (merge.uparts.right & 0x00FFFFFF) | (merge.uparts.left & 0xFF000000);
		buffer.uparts.right = merge.uparts.left;
		final.whole = alpha_mul >= 1 ? buffer.uparts.left : buffer.uparts.right;

		break;
	case SR_BLEND_INVERT_DROP:
		merge.uparts.right = ~merge.uparts.right;
	case SR_BLEND_DROP:
		final.whole = (merge.uparts.left & 0x00FFFFFF) | (merge.uparts.right & 0xFF000000);

		break;
	case SR_BLEND_REPLACE:
		final.whole = merge.uparts.right;

		break;
	case SR_BLEND_REPLACE_WALPHA_MOD:
		PREALPHA
		final.whole = (merge.uparts.right & 0x00FFFFFF) | (alpha_mul << 24);

		break;
	case SR_BLEND_DIRECT_XOR_ALL:
		final.whole = merge.uparts.left ^ merge.uparts.right;

		break;
	case SR_BLEND_INVERTED_DRAW:
		PREALPHA
		final.whole = merge.uparts.left - (alpha_mul * 0x00010101);

		break;
	case SR_BLEND_PAINT:
		final.whole = (merge.uparts.left & 0xFF000000) | (merge.uparts.right & 0x00FFFFFF);

		break;
	}

	#undef PREMULTIPLY
	#undef PREALPHA

	return final;
}
#endif
