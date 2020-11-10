#include "blendvec.h"

pixbuf_t SR_PixbufBlend(
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

SR_RGBAPixel SR_RGBABlender(
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
