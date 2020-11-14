#include "canvmerge.h"
#include "colours.h"
#include "remote_simde/x86/avx2.h"
#include <string.h>

typedef union {
	simde__m256i vec;
	simde__m128i vec128s[2];
	U8  aU8x32[32];
	U16 aU16x16[16];
	U32 aU32x8[8];
	U64 aU64x4[4];

	U32 pix0;

	U32x8 vevec_epu32;
} pixbuf_t;

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

pixbuf_t fstatelkp[9] = {
	{.aU32x8 = { 0,  0,  0,  0,  0,  0,  0,  0}},
	{.aU32x8 = { 0,  0,  0,  0,  0,  0,  0,  0}},
	{.aU32x8 = { 0,  1,  0,  0,  0,  0,  0,  0}},
	{.aU32x8 = { 0,  1,  2,  0,  0,  0,  0,  0}},
	{.aU32x8 = { 0,  1,  2,  3,  0,  0,  0,  0}},
	{.aU32x8 = { 0,  1,  2,  3,  4,  0,  0,  0}},
	{.aU32x8 = { 0,  1,  2,  3,  4,  5,  0,  0}},
	{.aU32x8 = { 0,  1,  2,  3,  4,  5,  6,  0}},
	{.aU32x8 = { 0,  1,  2,  3,  4,  5,  6,  7}}
};

pixbuf_t fstatelkp2[9] = {
	{.aU32x8 = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},
	{.aU32x8 = {0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},
	{.aU32x8 = {0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},
	{.aU32x8 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},
	{.aU32x8 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000}},
	{.aU32x8 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000}},
	{.aU32x8 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000}},
	{.aU32x8 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000}},
	{.aU32x8 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}}
};

#define ZEROVEC simde_mm256_setzero_si256()

__extension__ static inline __attribute__((always_inline)) simde__m256i sex_mm256_bimulhi_epu8(
	simde__m256i a,
	simde__m256i b)
{
	union i256buf {
		simde__m256i w256;
		simde__m128i h128[2];
		simde__m64   q64[4];
	} ab = {.w256 = a}, bb = {.w256 = b};
	simde__m256i ax1, ax2, bx1, bx2;
	simde__m64 temp_m64;

	ax1 = simde_mm256_cvtepu8_epi16(ab.h128[0]);	ax2 = simde_mm256_cvtepu8_epi16(ab.h128[1]);
	bx1 = simde_mm256_cvtepu8_epi16(bb.h128[0]);	bx2 = simde_mm256_cvtepu8_epi16(bb.h128[1]);
	ax1 = simde_mm256_mullo_epi16(ax1, bx1);	ax2 = simde_mm256_mullo_epi16(ax2, bx2);
	bx1 = simde_mm256_set1_epi16(0x00FF);
	ax1 = simde_mm256_add_epi16(ax1, bx1);		ax2 = simde_mm256_add_epi16(ax2, bx1);
	ax1 = simde_mm256_srli_epi16(ax1, 8);		ax2 = simde_mm256_srli_epi16(ax2, 8);

	ab.w256 = simde_mm256_packus_epi16(ax1, ax2);
	/* __m64 : ax1 ax2 ax1 ax2 */
	temp_m64  = ab.q64[1];
	ab.q64[1] = ab.q64[2];
	ab.q64[2] = temp_m64;
	/* __m64 : ax1 ax1 ax2 ax2 */

	return ab.w256;
}

__extension__ static inline __attribute__((always_inline)) pixbuf_t SR_PixbufBlend(
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
	srcAbuf.vec = sex_mm256_bimulhi_epu8(srcAbuf.vec, destbuf.vec);\
	srcBbuf.vec = sex_mm256_bimulhi_epu8(srcBbuf.vec, simde_mm256_xor_si256(destbuf.vec, consdat[5].vec));
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
		destbuf.vec = simde_mm256_cmpgt_epi32(destbuf.vec, ZEROVEC);
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
		destbuf.vec = simde_mm256_or_si256 (destbuf.vec, simde_mm256_adds_epu8(srcAbuf.vec, srcBbuf.vec));
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

X0 SR_MergeCanvasIntoCanvas(
	SR_Canvas *dest_canvas,
	SR_Canvas *src_canvas,
	U16 paste_start_x,
	U16 paste_start_y,
	U8 alpha_modifier,
	I8 mode)
{
	U16 x, y, z, srcposx, emax, fsub, fstate, isy, idy;
	U32 sxycchk, cxycchk, sxycchk0, cxycchk0;
	pixbuf_t srcAbuf, srcBbuf, destbuf, isxmap, idxmap, isxtmap, idxtmap;

	/* CLUMPS represents the amount of pixels that can be stored in a large vector */
	#define CLUMPS (sizeof(pixbuf_t) / sizeof(SR_RGBAPixel))

	/* emax represents the total number of clumps in each row of the source canvas */
	emax = ((src_canvas->width + CLUMPS) - 1) / CLUMPS;

	/* fsub represnts the number of extra pixels in each clumped row, e.g a 125 pixel row with 16-pixel clumps
	 * would have 3 extra pixels that should not be overwritten */
	fsub = (emax * CLUMPS) - src_canvas->width;

	#define MBLEND destbuf = SR_PixbufBlend(srcAbuf, srcBbuf, alpha_modifier, mode);

	for (x = 0; x < emax; x++) {
		/* We can calculate the X position stuff here instead of per-clump in order to prevent any extra
		 * pointless calculations */
		fstate  = x + 1 == emax ? CLUMPS - fsub : CLUMPS;
		for (z = 0; z < fstate; z++) {
			srcposx = (x * CLUMPS) + z;

			/* Create the X axis coordinate mappings for the source and destination canvases.
			 * This step is important because the X positions may not be continuous (it may
			 * loop back around to the start of the canvas if you reach the edge, for example)
			 */
			isxmap.aU32x8[z] = SR_AxisPositionCRCTRM(
				src_canvas->rwidth, src_canvas->cwidth, srcposx, src_canvas->xclip);
			idxmap.aU32x8[z] = SR_AxisPositionCRCTRM(
				dest_canvas->rwidth, dest_canvas->cwidth, srcposx + paste_start_x, dest_canvas->xclip);
		}

		for (y = 0; y < src_canvas->height; y++) {
			/* We already have the X position, so we don't need to calculate it. We CAN calculate the Y
			 * positions now, however. */
			isy = SR_AxisPositionCRCTRM(
				src_canvas->rheight, src_canvas->cheight , y, src_canvas->yclip);
			idy = SR_AxisPositionCRCTRM(
				dest_canvas->rheight, dest_canvas->cheight, y + paste_start_y, dest_canvas->yclip);

			/* Create the pixel index map using the row (Y) position and the contents of the X position
			 * map.
			 */
			isxtmap.vec = simde_mm256_and_si256(simde_mm256_sub_epi32((
				simde_mm256_add_epi32(isxmap.vec, simde_mm256_set1_epi32(
					(U32)src_canvas->rwidth * isy))),
				fstatelkp[fstate].vec),
				fstatelkp2[fstate].vec);

			idxtmap.vec = simde_mm256_and_si256(simde_mm256_sub_epi32((
				simde_mm256_add_epi32(idxmap.vec, simde_mm256_set1_epi32(
					(U32)dest_canvas->rwidth * idy))),
				fstatelkp[fstate].vec),
				fstatelkp2[fstate].vec);

			/* We can check if all of the memory regions are contiguous before we write to them, as we
			 * can save a significant amount of iteration and memory accesses if they are contiguous.
			 */
			sxycchk = sxycchk0 = simde_mm256_extract_epi32(isxtmap.vec, 0);
			cxycchk = cxycchk0 = simde_mm256_extract_epi32(idxtmap.vec, 0);
			#define SXYOR(zix)\
			sxycchk |= simde_mm256_extract_epi32(isxtmap.vec, zix);\
			cxycchk |= simde_mm256_extract_epi32(idxtmap.vec, zix);
			SXYOR( 1) SXYOR( 2) SXYOR( 3) SXYOR( 4) SXYOR( 5) SXYOR( 6) SXYOR( 7)
			#undef SXYOR

			/* Perform the final stage of the continuity check */
			if (cxycchk == cxycchk0 && sxycchk == sxycchk0) {
				/* If the addresses ARE continuous, we can move up to 256 bits in a single
				 * cycle and manipulate them simultaneously, then write them back all in one
				 * go too. Fast!
				 */
				srcAbuf.vec = simde_mm256_maskload_epi32(
					(void *)&src_canvas->pixels[sxycchk], fstatelkp2[fstate].vec);

				srcBbuf.vec = simde_mm256_maskload_epi32(
					(void *)&dest_canvas->pixels[cxycchk], fstatelkp2[fstate].vec);

				MBLEND

				simde_mm256_maskstore_epi32(
					(void *)&dest_canvas->pixels[cxycchk], fstatelkp2[fstate].vec, destbuf.vec);
			} else {
				/* If we know the addresses aren't continuous, which is usually unlikely, but
				 * can happen, then we can just iterate over each pixel in "safe mode" */
				isxtmap.vec = simde_mm256_add_epi32(isxtmap.vec, fstatelkp[fstate].vec);
				idxtmap.vec = simde_mm256_add_epi32(idxtmap.vec, fstatelkp[fstate].vec);

				for (z = 0; z < fstate; z++) {
					srcAbuf.aU32x8[z] = src_canvas->pixels[isxtmap.aU32x8[z]].whole;
					srcBbuf.aU32x8[z] = dest_canvas->pixels[idxtmap.aU32x8[z]].whole;
				}

				MBLEND

				for (z = 0; z < fstate; z++)
					dest_canvas->pixels[idxtmap.aU32x8[z]].whole = destbuf.aU32x8[z];
			}
		}
	}

	#undef MBLEND
	#undef CLUMPS
	#undef VMOVE

	return;
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
