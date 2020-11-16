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


#define CACHEBYTES 128
typedef union {
	pixbuf_t pbfs[CACHEBYTES / sizeof(pixbuf_t)];
	U32 pixels[CACHEBYTES / sizeof(U32)];
} localbuf_t;

const static pixbuf_t consdat[8] = {
	/* General table of constants. */
	{.aU32x8 = {0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000}},
	{.aU32x8 = {0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF}},
	{.aU32x8 = {0x00010101, 0x00010101, 0x00010101, 0x00010101, 0x00010101, 0x00010101, 0x00010101, 0x00010101}},
	{.aU32x8 = {0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF}},
	{.aU32x8 = {0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101, 0x01010101}},
	{.aU32x8 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},
	{.aU32x8 = {0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001}},
	{.aU32x8 = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}}};

const static pixbuf_t fstatelkp[9] = {
	/* Addition and subtraction lookup table for the general continuity check */
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

const static pixbuf_t fstatelkp2[9] = {
	/* Masking lookup table to "mask out" elements in a vector depending on the fstate value without branching */
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
	/* This function is equivalent to the following operation:
	 * U8 a -> U16 a
	 * U8 b -> U16 b
	 * a = ((a * b) + 255) >> 8
	 * U16 a -> U8 a
	 * return a
	 * 
	 * This implementation is quite slow, because there isn't a singular AVX/AVX2 instruction that does it for us.
	 */
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
		destbuf.vec = simde_mm256_or_si256 (destbuf.vec, simde_mm256_add_epi8(srcAbuf.vec, srcBbuf.vec));

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
	#define CLTYPE localbuf_t
	#define OBTYPE pixbuf_t

	/* CLUMPS represents the amount of pixels that can be stored in a large vector */
	#define CLUMPS (sizeof(CLTYPE) / sizeof(SR_RGBAPixel))
	#define OBBUFS (sizeof(localbuf_t) / sizeof(pixbuf_t))
	#define FXLOOP(it) for (it = 0; it < OBBUFS; it++) 

	/* emax represents the total number of clumps in each row of the source canvas */
	U16 emax = ((src_canvas->width + CLUMPS) - 1) / CLUMPS;

	/* fsub represnts the number of extra pixels in each clumped row, e.g a 125 pixel row with 16-pixel clumps
	 * would have 3 extra pixels that should not be overwritten */
	U16 fsub = (emax * CLUMPS) - src_canvas->width;

	for (U16 x = 0; x < emax; x++) {
		/* We can calculate the X position stuff here instead of per-clump in order to prevent any extra
		 * pointless calculations */
		I16 z, obi, fstatew = x + 1 == emax ? CLUMPS - fsub : CLUMPS;
		U8 fstate[OBBUFS];
		FXLOOP(obi) fstate[obi] = MAX(
			fstatew - ((CLUMPS / OBBUFS) * obi) - MAX(fstatew - ((CLUMPS / OBBUFS) * (obi + 1)), 0), 0);

		CLTYPE isxmap, idxmap;
		for (z = 0; z < fstatew; z++) {
			U16 srcposx = (x * CLUMPS) + z;

			/* Create the X axis coordinate mappings for the source and destination canvases.
			 * This step is important because the X positions may not be continuous (it may
			 * loop back around to the start of the canvas if you reach the edge, for example)
			 */
			isxmap.pixels[z] = SR_AxisPositionCRCTRM(
				src_canvas->rwidth, src_canvas->cwidth, srcposx, src_canvas->xclip);
			idxmap.pixels[z] = SR_AxisPositionCRCTRM(
				dest_canvas->rwidth, dest_canvas->cwidth, srcposx + paste_start_x, dest_canvas->xclip);
		}

		for (U16 y = 0; y < src_canvas->height; y++) {
			/* We already have the X position, so we don't need to calculate it. We CAN calculate the Y
			 * positions now, however. */
			U16 isy = SR_AxisPositionCRCTRM(
				src_canvas->rheight, src_canvas->cheight , y, src_canvas->yclip);
			U16 idy = SR_AxisPositionCRCTRM(
				dest_canvas->rheight, dest_canvas->cheight, y + paste_start_y, dest_canvas->yclip);

			/* Create the pixel index map using the row (Y) position and the contents of the X position
			 * map.
			 */
			CLTYPE isxtmap, idxtmap;
			#define PXLMAP(ixt, ix, iy, rwidth, fst) ixt = simde_mm256_and_si256(simde_mm256_sub_epi32((\
				simde_mm256_add_epi32((ix), simde_mm256_set1_epi32(\
					(U32)(rwidth) * (iy)))),\
				fstatelkp[fst].vec),\
				fstatelkp2[fst].vec);

			FXLOOP(obi) PXLMAP(
				isxtmap.pbfs[obi].vec, isxmap.pbfs[obi].vec, isy, src_canvas->rwidth, fstate[obi])
			FXLOOP(obi) PXLMAP(
				idxtmap.pbfs[obi].vec, idxmap.pbfs[obi].vec, idy, dest_canvas->rwidth, fstate[obi])

			/* We can check if all of the memory regions are contiguous before we write to them, as we
			 * can save a significant amount of iteration and memory accesses if they are contiguous.
			 */
			#define CONTIGCHK(imap) simde_mm256_testc_si256(simde_mm256_broadcastss_ps(\
				simde_mm256_castps256_ps128(simde_mm256_castsi256_ps(imap))), imap)

			U1 contig = true;
			FXLOOP(obi) contig = contig && (
				CONTIGCHK(isxtmap.pbfs[obi].vec) && CONTIGCHK(idxtmap.pbfs[obi].vec));

			#define MBLEND(a, b) SR_PixbufBlend(a, b, alpha_modifier, mode)
			#define MBLENDA FXLOOP(obi) destbuf.pbfs[obi] = MBLEND(srcAbuf.pbfs[obi], srcBbuf.pbfs[obi]);

			/* Perform the final stage of the continuity check */
			CLTYPE srcAbuf, srcBbuf, destbuf;
			if (contig) {
				/* If the addresses ARE continuous, we can move up to 256 bits in a single
				 * cycle and manipulate them simultaneously, then write them back all in one
				 * go too. Fast!
				 */
				#define SRCADR (OBTYPE *)&src_canvas->pixels [isxtmap.pixels[0]]
				#define DSTADR (OBTYPE *)&dest_canvas->pixels[idxtmap.pixels[0]]
				#define LPLOAD(dbuf, addr, pbi) dbuf = simde_mm256_maskload_epi32(\
					(void *)(addr + pbi), fstatelkp2[fstate[pbi]].vec);
				FXLOOP(obi) LPLOAD(srcAbuf.pbfs[obi].vec, SRCADR, obi)
				FXLOOP(obi) LPLOAD(srcBbuf.pbfs[obi].vec, DSTADR, obi)
				#undef LPLOAD

				MBLENDA

				FXLOOP(obi) simde_mm256_maskstore_epi32(
					(void *)(DSTADR + obi), fstatelkp2[fstate[obi]].vec, destbuf.pbfs[obi].vec);

				#undef SRCADR
				#undef DSTADR
			} else {
				/* If we know the addresses aren't continuous, which is usually unlikely, but
				 * can happen, then we can just iterate over each pixel in "safe mode" */

				#define VECINCRS(v, fst) v = simde_mm256_add_epi32(v, fstatelkp[fst].vec)
				FXLOOP(obi) VECINCRS(isxtmap.pbfs[obi].vec, fstate[obi]);
				FXLOOP(obi) VECINCRS(idxtmap.pbfs[obi].vec, fstate[obi]);
				#undef VECINCRS

				for (z = 0; z < fstatew; z++) {
					srcAbuf.pixels[z] = src_canvas->pixels[isxtmap.pixels[z]].whole;
					srcBbuf.pixels[z] = dest_canvas->pixels[idxtmap.pixels[z]].whole;
				}

				MBLENDA

				for (z = 0; z < fstatew; z++)
					dest_canvas->pixels[idxtmap.pixels[z]].whole = destbuf.pixels[z];
			}

			#undef MBLEND
			#undef MBLENDA
		}
	}
	#undef CLUMPS
	#undef VMOVE
	#undef OBTYPE
	#undef CLTYPE
	#undef OBBUFS
	#undef FXLOOP

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
