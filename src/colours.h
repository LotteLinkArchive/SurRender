#ifndef SURCL_HEADER_FILE
#define SURCL_HEADER_FILE
#include "glbl.h"

typedef union {
	__extension__ struct {
		U8 red;
		U8 green;
		U8 blue;
		U8 alpha;
	} __attribute__ ((packed)) chn;
	U32 whole;
	U8x4 splitvec;
} SR_RGBAPixel;

typedef union {
	U64 whole;
	__extension__ struct {
		U32 left;
		U32 right;
	} __attribute__ ((packed)) uparts;
	__extension__ struct {
		SR_RGBAPixel left;
		SR_RGBAPixel right;
	} __attribute__ ((packed)) srparts;
	U8x8 splitvec;
} SR_RGBADoublePixel;

typedef union {
	U8x64  sU8x64;
	U16x32 sU16x32;
	U64x8  sU64x8;
	U32x16 sU32x16;
	U32 aU32x16[16];
} pixbuf_t;

typedef union {
	U8x128 sU8x128;
	U16x64 sU16x64;
	U32x32 sU32x32;
} bigpixbuf_t;

typedef union {
	U16x128 sU16x128;
	__extension__ struct {
		U16x64 c1;
		U16x64 c2;
	} __attribute__ ((packed)) sU16x64x2;
} largepixbuf_t;

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

inline __attribute__((always_inline)) pixbuf_t SR_PixbufBlend(
	pixbuf_t srcAbuf,
	pixbuf_t srcBbuf,
	U8 alpha_modifier,
	I8 mode)
{
	largepixbuf_t blendbufA, blendbufB;
	pixbuf_t destbuf;

	#define PREALPHA\
	destbuf.sU32x16 = (((((srcAbuf.sU32x16 & 0xFF000000) >> 24) * alpha_modifier) + 0xFF) >> 8);

	#define PREALPHA_MID destbuf.sU32x16 *= 0x00010101;

	#define PREMULTIPLY\
	PREALPHA\
	PREALPHA_MID\
	blendbufA.sU16x64x2.c1 = hcl_vector_convert( srcAbuf.sU8x64, U16x64);\
	blendbufA.sU16x64x2.c2 = hcl_vector_convert( srcBbuf.sU8x64, U16x64);\
	blendbufB.sU16x64x2.c1 = hcl_vector_convert( destbuf.sU8x64, U16x64);\
	blendbufB.sU16x64x2.c2 = hcl_vector_convert(~destbuf.sU8x64, U16x64);\
	blendbufA.sU16x128     = ((blendbufA.sU16x128 * blendbufB.sU16x128) + 0xFF) >> 8;\
	srcAbuf.sU8x64         = hcl_vector_convert(blendbufA.sU16x64x2.c1, U8x64);\
	srcBbuf.sU8x64         = hcl_vector_convert(blendbufA.sU16x64x2.c2, U8x64);

	switch (mode) {
	case SR_BLEND_XOR:
		PREMULTIPLY
		/* fallthrough */
	case SR_BLEND_DIRECT_XOR:
		destbuf.sU32x16 = srcBbuf.sU32x16 ^ (srcAbuf.sU32x16 & 0x00FFFFFF);

		break;
	default:
	case SR_BLEND_OVERLAY:
		PREALPHA
		destbuf.sU32x16  = (destbuf.sU32x16 / 0xFF) * 0x00010101;
		srcAbuf.sU8x64  *= destbuf.sU8x64;
		destbuf.sU32x16 ^= 0x01010101;
		srcBbuf.sU8x64  *= destbuf.sU8x64;
		destbuf.sU8x64   = srcAbuf.sU8x64 | srcBbuf.sU8x64;

		break;
	case SR_BLEND_ADDITIVE:
		PREMULTIPLY
		/* fallthrough */
	case SR_BLEND_ADDITIVE_PAINT:
		destbuf.sU32x16 = srcBbuf.sU32x16 & 0xFF000000;

		srcAbuf.sU32x16 &= 0x00FFFFFF;
		srcBbuf.sU32x16 &= 0x00FFFFFF;
		destbuf.sU8x64  |= srcAbuf.sU8x64 + srcBbuf.sU8x64;

		break;
	case SR_BLEND_REPLACE:
		destbuf = srcAbuf;

		break;
	case SR_BLEND_INVERT_DROP:
		srcAbuf.sU64x8 = ~srcAbuf.sU64x8;
		/* fallthrough */
	case SR_BLEND_DROP:
		destbuf.sU32x16 = (srcBbuf.sU32x16 & 0x00FFFFFF) | (srcAbuf.sU32x16 & 0xFF000000);

		break;
	case SR_BLEND_REPLACE_WALPHA_MOD:
		PREALPHA
		destbuf.sU32x16 = (srcAbuf.sU32x16 & 0x00FFFFFF) | (destbuf.sU32x16 << 24);

		break;
	case SR_BLEND_DIRECT_XOR_ALL:
		destbuf.sU64x8 = srcAbuf.sU64x8 ^ srcBbuf.sU64x8;

		break;
	case SR_BLEND_INVERTED_DRAW:
		PREALPHA
		PREALPHA_MID
		destbuf.sU8x64 = srcBbuf.sU8x64 - destbuf.sU8x64;

		break;
	case SR_BLEND_PAINT:
		destbuf.sU32x16 = (srcBbuf.sU32x16 & 0xFF000000) | (srcAbuf.sU32x16 & 0x00FFFFFF);

		break;
	}

	#undef PREMULTIPLY
	#undef PREALPHA_MID
	#undef PREALPHA

	return destbuf;
}

inline __attribute__((always_inline)) SR_RGBAPixel SR_RGBABlender(
	SR_RGBAPixel pixel_base,
	SR_RGBAPixel pixel_top,
	U8 alpha_modifier,
	I8 mode)
{
	pixbuf_t top, bottom, rfinal;

	top.sU32x16[0]    = pixel_top.whole;
	bottom.sU32x16[0] = pixel_base.whole;

	rfinal = SR_PixbufBlend(top, bottom, alpha_modifier, mode);

	SR_RGBAPixel rspfinal = { .whole = rfinal.sU32x16[0] };

	return rspfinal;
}

#endif
