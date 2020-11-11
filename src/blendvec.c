#include "blendvec.h"

/* TODO:
 * - Fix blue issue
 * - Add the premultiply section
 * - Optimize
 * - Replace memcpies
 */

typedef union {
	simde__m128i vec;
	U32 aU32x4[4];
} countbuf_t;

countbuf_t counts[3] = {
	{.aU32x4 = {24, 0, 0, 0}},
	{.aU32x4 = { 7, 0, 0, 0}},
	{.aU32x4 = { 8, 0, 0, 0}},
};

pixbuf_t SR_PixbufBlend(
	pixbuf_t srcAbuf,
	pixbuf_t srcBbuf,
	U8 alpha_modifier,
	I8 mode)
{
	bigpixbuf_t blendbufA, blendbufB;
	pixbuf_t destbuf;

	#define PREALPHA destbuf.vec = simde_mm256_srl_epi32(simde_mm256_add_epi32(simde_mm256_mullo_epi32(\
		simde_mm256_srl_epi32(simde_mm256_and_si256(srcAbuf.vec, simde_mm256_set1_epi32(0xFF000000)),\
		counts[0].vec), simde_mm256_set1_epi32(alpha_modifier)), simde_mm256_set1_epi32(0xFF)),\
		counts[2].vec);
	#define PREALPHA_MID destbuf.vec = simde_mm256_mullo_epi32(destbuf.vec, simde_mm256_set1_epi32(0x00010101));
	#define PREMULTIPLY 

	switch (mode) {
	case SR_BLEND_XOR:
		PREMULTIPLY
		/* fallthrough */
	case SR_BLEND_DIRECT_XOR:
		destbuf.vec = simde_mm256_xor_si256(
			srcBbuf.vec, simde_mm256_and_si256(srcAbuf.vec, simde_mm256_set1_epi32(0x00FFFFFF)));

		break;
	default:
	case SR_BLEND_OVERLAY:
		PREALPHA
		destbuf.vec = simde_mm256_mullo_epi32(simde_mm256_srl_epi32(
			destbuf.vec, counts[1].vec), simde_mm256_set1_epi32(0x00010101));
		srcAbuf.vec = simde_mm256_mullo_epi32(srcAbuf.vec, destbuf.vec);
		srcBbuf.vec = simde_mm256_mullo_epi32(
			simde_mm256_xor_si256(destbuf.vec, simde_mm256_set1_epi32(0x01010101)), srcBbuf.vec);
		destbuf.vec = simde_mm256_or_si256(srcAbuf.vec, srcBbuf.vec);

		break;
	case SR_BLEND_ADDITIVE:
		PREMULTIPLY
		/* fallthrough */
	case SR_BLEND_ADDITIVE_PAINT:
		destbuf.vec = simde_mm256_and_si256(srcBbuf.vec, simde_mm256_set1_epi32(0xFF000000));
		srcAbuf.vec = simde_mm256_and_si256(srcAbuf.vec, simde_mm256_set1_epi32(0x00FFFFFF));
		srcBbuf.vec = simde_mm256_and_si256(srcBbuf.vec, simde_mm256_set1_epi32(0x00FFFFFF));
		destbuf.vec = simde_mm256_or_si256 (destbuf.vec, simde_mm256_add_epi8(srcAbuf.vec, srcBbuf.vec));

		break;
	case SR_BLEND_REPLACE:
		destbuf.vec = srcAbuf.vec;

		break;
	case SR_BLEND_INVERT_DROP:
		srcAbuf.vec = simde_mm256_xor_si256(srcAbuf.vec, simde_mm256_set1_epi32(0xFFFFFFFF));
		/* fallthrough */
	case SR_BLEND_DROP:
		destbuf.vec = simde_mm256_or_si256(
			simde_mm256_and_si256(srcBbuf.vec, simde_mm256_set1_epi32(0x00FFFFFF)),
			simde_mm256_and_si256(srcAbuf.vec, simde_mm256_set1_epi32(0xFF000000)));

		break;
	case SR_BLEND_REPLACE_WALPHA_MOD:
		PREALPHA
		destbuf.vec = simde_mm256_or_si256(
			simde_mm256_and_si256(srcAbuf.vec, simde_mm256_set1_epi32(0x00FFFFFF)),
			simde_mm256_sll_epi32(destbuf.vec, counts[0].vec));

		break;
	case SR_BLEND_DIRECT_XOR_ALL:
		destbuf.vec = simde_mm256_xor_si256(srcAbuf.vec, srcBbuf.vec);

		break;
	case SR_BLEND_INVERTED_DRAW:
		PREALPHA
		PREALPHA_MID

		destbuf.vec = simde_mm256_sub_epi8(srcBbuf.vec, destbuf.vec);

		break;
	case SR_BLEND_PAINT:
		destbuf.vec = simde_mm256_or_si256(
			simde_mm256_and_si256(srcBbuf.vec, simde_mm256_set1_epi32(0xFF000000)),
			simde_mm256_and_si256(srcAbuf.vec, simde_mm256_set1_epi32(0x00FFFFFF)));

		break;
	}

	return destbuf;
}

SR_RGBAPixel SR_RGBABlender(
	SR_RGBAPixel pixel_base,
	SR_RGBAPixel pixel_top,
	U8 alpha_modifier,
	I8 mode)
{
	pixbuf_t top, bottom, rfinal;

	top.aU32x8[0]    = pixel_top.whole;
	bottom.aU32x8[0] = pixel_base.whole;

	rfinal = SR_PixbufBlend(top, bottom, alpha_modifier, mode);

	SR_RGBAPixel rspfinal = { .whole = rfinal.aU32x8[0] };

	return rspfinal;
}
