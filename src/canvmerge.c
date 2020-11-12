#include "canvmerge.h"
#include "blendvec.h"
#include <string.h>

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

X0 SR_MergeCanvasIntoCanvas(
	SR_Canvas *dest_canvas,
	SR_Canvas *src_canvas,
	U16 paste_start_x,
	U16 paste_start_y,
	U8 alpha_modifier,
	I8 mode)
{
	/* Canvas Blending/Merging with AVX-512 x86_64 Processor Instructions
	 * ------------------------------------------------------------------
	 * 
	 * Z M M   R E G I S T E R   F O R M A T
	 * .... .... .... .... .... .... .... .... .... .... .... .... .... .... .... .... sU8x64
	 * |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ |/|/ sU16x32
	 * 0 1  2 3  4 5  6 7  8 9  1011 1213 1415 1617 1819 2021 2223 2425 2627 2829 3031
	 * |--/ |--/ |--/ |--/ |--/ |--/ |--/ |--/ |--/ |--/ |--/ |--/ |--/ |--/ |--/ |--/ sU32x16
	 * 0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15
	 * |-------/ |-------/ |-------/ |-------/ |-------/ |-------/ |-------/ |-------/ sU64x8
	 * 0         1         2         3         4         5         6         7
	 * 
	 * Intended to use AVX-512 to blend 16 pixels simultaneously. On some machines, it may have
	 * to fallback to using standard AVX or even previous SSE instructions.
	 */

	U16 x, y, z, srcposx, emax, fsub, fstate, isy, idy;
	U32 sxycchk, cxycchk;
	pixbuf_t srcAbuf, srcBbuf, destbuf, isxmap, idxmap, isxtmap, idxtmap;
	
	/* CLUMPS represents the amount of pixels that can be stored in an AVX-512-compatible vector */
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

			sxycchk = simde_mm256_extract_epi32(isxtmap.vec, 0);
			cxycchk = simde_mm256_extract_epi32(idxtmap.vec, 0);
			#define CONTIGCHK(imap, vc) simde_mm256_testc_si256(\
				simde_mm256_set1_epi32(vc), imap)

			/* Perform the final stage of the continuity check */
			if (CONTIGCHK(isxtmap.vec, sxycchk) && CONTIGCHK(idxtmap.vec, cxycchk)) {
				/* If the addresses ARE continuous, we can move up to 512 bits in a single
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
