#include "blendvec.h"

typedef union {
	simde__m128i vec;
	U32 aU32x4[4];
	U32 count;
} countbuf_t;

const pixbuf_t consdat[8] = {
	{.aU32x8 = {0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000}},
	{.aU32x8 = {0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF}},
	{.aU32x8 = {0x00010101, 0x00010101, 0x00010101, 0x00010101, 0x00010101, 0x00010101, 0x00010101, 0x00010101}},
	{.aU32x8 = {0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF}},
	{.aU32x8 = {0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101}},
	{.aU32x8 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},
	{.aU32x8 = {0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001}},
	{.aU32x8 = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}}};

pixbuf_t SR_PixbufBlend(
	pixbuf_t srcAbuf,
	pixbuf_t srcBbuf,
	U8 alpha_modifier,
	I8 mode)
{
	pixbuf_t destbuf = destbuf;

	/* Feed Assumptions:
	 * srcAbuf = 0xFF37A4B6 (top)
	 * srcBbuf = 0xFF888888 (base)
	 */

	#define PREALPHA destbuf.vec = simde_mm256_srli_epi32(simde_mm256_add_epi32(simde_mm256_mullo_epi32(\
		simde_mm256_srli_epi32(simde_mm256_and_si256(srcAbuf.vec, consdat[0].vec), 24),\
		simde_mm256_set1_epi32(alpha_modifier)), consdat[1].vec), 8);
	/* destbuf = 0x000000FF */
	#define PREALPHA_MID destbuf.vec = simde_mm256_mullo_epi32(destbuf.vec, consdat[2].vec);
	/* destbuf = 0x00FFFFFF */

	#define PREMULTIPLY PREALPHA PREALPHA_MID\
	srcAbuf.vec = simde_mm256_and_si256(srcAbuf.vec, destbuf.vec);\
	srcBbuf.vec = simde_mm256_and_si256(srcBbuf.vec, simde_mm256_xor_si256(destbuf.vec, consdat[5].vec));
	/* TODO: Premultiply properly rather than using AND as a cheap workaround */

	switch (mode) {
	case SR_BLEND_XOR:
		PREMULTIPLY
		/* fallthrough */
	case SR_BLEND_DIRECT_XOR:
		destbuf.vec = simde_mm256_xor_si256(srcBbuf.vec, simde_mm256_and_si256(srcAbuf.vec, consdat[3].vec));

		break;
	case SR_BLEND_OVERLAY:
		PREALPHA
		destbuf.vec = simde_mm256_cmpgt_epi32(destbuf.vec, simde_mm256_setzero_si256());
		srcBbuf.vec = simde_mm256_and_si256(srcBbuf.vec,
			simde_mm256_or_si256(simde_mm256_xor_si256(destbuf.vec, consdat[5].vec), consdat[0].vec));
		destbuf.vec = simde_mm256_or_si256(simde_mm256_and_si256(
			simde_mm256_and_si256(srcAbuf.vec, consdat[3].vec), destbuf.vec), srcBbuf.vec);

		break;
	case SR_BLEND_ADDITIVE:
		PREMULTIPLY
		/* fallthrough */
	case SR_BLEND_ADDITIVE_PAINT:
		destbuf.vec = simde_mm256_and_si256(srcBbuf.vec, consdat[0].vec);
		srcAbuf.vec = simde_mm256_and_si256(srcAbuf.vec, consdat[3].vec);
		srcBbuf.vec = simde_mm256_and_si256(srcBbuf.vec, consdat[3].vec);
		destbuf.vec = simde_mm256_or_si256 (destbuf.vec, simde_mm256_adds_epi8(srcAbuf.vec, srcBbuf.vec));
		/* TODO: If premultiplication is accurate, adds can be replaced with just add here. */

		break;
	case SR_BLEND_REPLACE:
		destbuf.vec = srcAbuf.vec;

		break;
	case SR_BLEND_INVERT_DROP:
		srcAbuf.vec = simde_mm256_xor_si256(srcAbuf.vec, consdat[5].vec);
		/* fallthrough */
	case SR_BLEND_DROP:
		destbuf.vec = simde_mm256_or_si256(
			simde_mm256_and_si256(srcBbuf.vec, consdat[3].vec),
			simde_mm256_and_si256(srcAbuf.vec, consdat[0].vec));

		break;
	case SR_BLEND_REPLACE_WALPHA_MOD:
		PREALPHA
		destbuf.vec = simde_mm256_or_si256(
			simde_mm256_and_si256(srcAbuf.vec, consdat[3].vec),
			simde_mm256_slli_epi32(destbuf.vec, 24));

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
			simde_mm256_and_si256(srcBbuf.vec, consdat[0].vec),
			simde_mm256_and_si256(srcAbuf.vec, consdat[3].vec));

		break;
	}

	return destbuf;
}

/* Backwards compatibility function used in text rendering - Avoid using this */
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
