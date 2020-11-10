#include "canvmerge.h"
#include "blendvec.h"
#include <string.h>

pixbuf_t fstatelkp[17] = {
	{.sU32x16 = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  7,  0,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,  0,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  0,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13,  0,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  0}},
	{.sU32x16 = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15}}
};

#define D32 0x00000000
#define A32 0xFFFFFFFF
pixbuf_t fstatelkp2[17] = {
	{.sU32x16 = {D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, D32, D32, D32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, D32, D32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, A32, D32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, A32, A32, D32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, D32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, D32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, D32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, D32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, D32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, D32}},
	{.sU32x16 = {A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32, A32}}
};
#undef D32
#undef A32

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
	U1 precheck;

	/* CLUMPS represents the amount of pixels that can be stored in an AVX-512-compatible vector */
	#define CLUMPS (sizeof(pixbuf_t) / sizeof(SR_RGBAPixel))

	/* emax represents the total number of clumps in each row of the source canvas */
	emax = ((src_canvas->width + CLUMPS) - 1) / CLUMPS;

	/* fsub represnts the number of extra pixels in each clumped row, e.g a 125 pixel row with 16-pixel clumps
	 * would have 3 extra pixels that should not be overwritten */
	fsub = (emax * CLUMPS) - src_canvas->width;

	#define MBLEND destbuf = SR_PixbufBlend(srcAbuf, srcBbuf, alpha_modifier, mode);
	#define VMOVE(dest, src) memcpy(dest, src, sizeof(pixbuf_t));

	for (x = 0; x < emax; x++) {
		/* We can calculate the X position stuff here instead of per-clump in order to prevent any extra
		 * pointless calculations */
		fstate  = x + 1 == emax ? CLUMPS - fsub : CLUMPS;
		precheck = fstate != CLUMPS;
		for (z = 0; z < fstate; z++) {
			srcposx = (x * CLUMPS) + z;

			/* Create the X axis coordinate mappings for the source and destination canvases.
			 * This step is important because the X positions may not be continuous (it may
			 * loop back around to the start of the canvas if you reach the edge, for example)
			 */
			isxmap.aU32x16[z] = SR_AxisPositionCRCTRM(
				src_canvas->rwidth, src_canvas->cwidth, srcposx, src_canvas->xclip);
			idxmap.aU32x16[z] = SR_AxisPositionCRCTRM(
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
			isxtmap.sU32x16 = (
				SR_CombnAxisPosCalcXY(src_canvas, isxmap.sU32x16, isy) - fstatelkp[fstate].sU32x16
			) & fstatelkp2[fstate].sU32x16;
			idxtmap.sU32x16 = (
				SR_CombnAxisPosCalcXY(dest_canvas, idxmap.sU32x16, idy) - fstatelkp[fstate].sU32x16
			) & fstatelkp2[fstate].sU32x16;

			/* We can check if all of the memory regions are contiguous before we write to them, as we
			 * can save a significant amount of iteration and memory accesses if they are contiguous.
			 */
			sxycchk = cxycchk = 0;
			#define SXYOR(zix) sxycchk |= isxtmap.sU32x16[zix]; cxycchk |= idxtmap.sU32x16[zix];
			SXYOR( 0) SXYOR( 1) SXYOR( 2) SXYOR( 3) SXYOR( 4) SXYOR( 5) SXYOR( 6) SXYOR( 7)
			SXYOR( 8) SXYOR( 9) SXYOR(10) SXYOR(11) SXYOR(12) SXYOR(13) SXYOR(14) SXYOR(15)
			#undef SXYOR

			/* Perform the final stage of the continuity check */
			if (cxycchk != idxtmap.sU32x16[0] || sxycchk != isxtmap.sU32x16[0] || precheck) {
				/* If we know the addresses aren't continuous, which is usually unlikely, but
				 * can happen, then we can just iterate over each pixel in "safe mode" */
				isxtmap.sU32x16 += fstatelkp[fstate].sU32x16;
				idxtmap.sU32x16 += fstatelkp[fstate].sU32x16;

				for (z = 0; z < fstate; z++) {
					srcAbuf.aU32x16[z] = src_canvas->pixels[isxtmap.aU32x16[z]].whole;
					srcBbuf.aU32x16[z] = dest_canvas->pixels[idxtmap.aU32x16[z]].whole;
				}

				MBLEND

				for (z = 0; z < fstate; z++)
					dest_canvas->pixels[idxtmap.aU32x16[z]].whole = destbuf.aU32x16[z];
			} else {
				/* If the addresses ARE continuous, we can move up to 512 bits in a single
				 * cycle and manipulate them simultaneously, then write them back all in one
				 * go too. Fast!
				 */
				VMOVE(&srcAbuf, &src_canvas->pixels[sxycchk]);
				VMOVE(&srcBbuf, &dest_canvas->pixels[cxycchk]);

				MBLEND

				VMOVE(&dest_canvas->pixels[cxycchk], &destbuf);
			}
		}
	}

	#undef MBLEND
	#undef CLUMPS
	#undef VMOVE

	return;
}
