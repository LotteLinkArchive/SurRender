#include "glbl.h"
#include "canvas.h"
#include "colours.h"
#include "errors.h"
#include "umman.h"
#include "srtex.h"
#include <math.h>
#include <string.h>

#define SR_MXCS_P1 SR_MAX_CANVAS_SIZE + 1
__extension__ U16 modlut[SR_MXCS_P1][SR_MXCS_P1] = {};
__extension__ U1  modlut_complete   [SR_MXCS_P1] = {};
#undef SR_MXCS_P1

X0 SR_FillModLUT(U16 moperand)
{
	if (modlut_complete[moperand]) goto sr_fmlutexit;

	modlut_complete[moperand] = true;
	for (U16 x = 0; x < (SR_MAX_CANVAS_SIZE + 1); x++) modlut[moperand][x] = x % moperand;
	
sr_fmlutexit:
	return;
}

X0 SR_GenCanvLUT(SR_Canvas *canvas)
{
	canvas->hflags |= SR_CPow2FDtc(canvas->rwidth, canvas->rheight, 0x10);
	canvas->hflags |= SR_CPow2FDtc(canvas->cwidth, canvas->cheight, 0x20);

	SR_FillModLUT(canvas->cwidth );
	SR_FillModLUT(canvas->cheight);
	SR_FillModLUT(canvas->rwidth );
	SR_FillModLUT(canvas->rheight);
}

STATUS SR_ResizeCanvas(
	SR_Canvas *canvas,
	U16 width,
	U16 height)
{
	/* It is impossible to create a 0-width/0-height canvas. */
	if (	!width  ||
		!height ||
		canvas->pixels ||
		canvas->hflags & 0x0B ||
		canvas->b_addr) return SR_SPECIAL_TYPE_DENIED;

	/* @direct */
	canvas->width  = canvas->rwidth  = canvas->cwidth  = width;
	canvas->height = canvas->rheight = canvas->cheight = height;

	SR_GenCanvLUT(canvas);

	/* Not strictly neccessary, but rodger put it here anyway, so whatever. */
	canvas->ratio = (R32)width / height;

	/* FYI: We can actually use realloc here if we assume that canvas->pixels
	 * contains either a valid allocation or none at all (represented by NULL)
	 * That way, we can simplify the whole process, as realloc works just like
	 * malloc does when you feed it a null pointer. Magic!
	 */
	canvas->pixels = realloc(canvas->pixels,
		SRT_WIDTH_ROUNDUP((U32)canvas->rwidth) * (U32)canvas->rheight * sizeof(SR_RGBAPixel));

	/* Return the allocation state. */
	return canvas->pixels ? SR_NO_ERROR : SR_MALLOC_FAILURE;
}

X0 SR_TileTo(
	SR_Canvas *canvas,
	U16 width,
	U16 height)
{
	/* @direct */
	canvas->width = width;
	canvas->height = height;
}

X0 SR_ZeroFill(SR_Canvas *canvas)
{
	if (!canvas->pixels) return;

	if (	canvas->xclip   != 0 ||
		canvas->yclip   != 0 ||
		canvas->cwidth  != canvas->rwidth  ||
		canvas->cheight != canvas->rheight ) {
		U16 x, y;
		for (x = 0; x < canvas->width; x++)
		for (y = 0; y < canvas->height; y++)
			SR_CanvasSetPixel(canvas, x, y, SR_CreateRGBA(0, 0, 0, 0));

		return;
	}

	/* Fill the canvas with zeros, resulting in RGBA(0, 0, 0, 0).
	 * To fill with something like RGBA(0, 0, 0, 255), see shapes.h.
	 */
	memset(canvas->pixels, 0, SR_CanvasCalcSize(canvas));
}

STATUS SR_NewCanvas(SR_Canvas *target, U16 width, U16 height)
{
	memset(target, 0, sizeof(SR_Canvas));

	/* As long as we set pixels to NULL, ResizeCanvas can be used here too. */
	return SR_ResizeCanvas(target, width, height);
}

/* SR_DestroyCanvas is super important for any mallocated canvases. Use it. */
STATUS SR_DestroyCanvas(SR_Canvas *canvas)
{
	/* Just in case we need to free anything else */
	if      ( canvas->hflags      & 0x02) return SR_CANVAS_CONSTANT;
	else if ( canvas->references > 0x00) return SR_NONZERO_REFCOUNT;
	else if (!canvas->pixels           ) return SR_NULL_CANVAS;
	else if ( canvas->hflags      & 0x01) {
		if (canvas->refsrc) ((SR_Canvas *)canvas->refsrc)->references--;
		canvas->pixels = canvas->b_addr = NULL;

		return SR_NO_ERROR;
	}

	X0 *freeadr = canvas->b_addr ? canvas->b_addr : canvas->pixels;

	if (canvas->hflags & 0x08) {
		if (umunmap(freeadr, canvas->munmap_size) != 0) return SR_MUNMAP_FAILURE;
		canvas->hflags ^= 0x08;
	} else {
		free(freeadr);
	}

	canvas->pixels = canvas->b_addr = NULL;

	return SR_NO_ERROR;
}

X0 SR_CopyCanvas(
	SR_Canvas *canvas,
	SR_Canvas *new,
	U16 copy_start_x,
	U16 copy_start_y)
{
	U16 x, y;
	for (x = 0; x < new->width ; x++)
	for (y = 0; y < new->height; y++)
		SR_CanvasSetPixel(new, x, y, SR_CanvasGetPixel(canvas, x + copy_start_x, y + copy_start_y));
}

SR_Canvas SR_RefCanv(
	SR_Canvas *src,
	U16 xclip,
	U16 yclip,
	U16 width,
	U16 height,
	U1  absorb_host)
{
	if ((src->hflags & 0x01) && src->refsrc) {
		xclip += src->xclip;
		yclip += src->yclip;

		width  = MIN(src->cwidth , width );
		height = MIN(src->cheight, height);

		src = (SR_Canvas *)src->refsrc;
	}

	/* @direct */
	SR_Canvas temp = {
		.hflags   = absorb_host ? 0x00 : 0x01,
		.width   = width,
		.height  = height,
		.ratio   = (R32)width / height,
		.pixels  = src->pixels,
		.rwidth  = src->rwidth,
		.rheight = src->rheight,
		.xclip   = xclip,
		.yclip   = yclip,
		.cwidth  = MIN(src->cwidth , width ),
		.cheight = MIN(src->cheight, height),
		.refsrc  = (void *)src
	};

	if (absorb_host) src->references++;

	SR_GenCanvLUT(&temp);

	return temp;
}

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

static X0 SR_BilinearCanvasScale(
	SR_Canvas *src,
	SR_Canvas *dest)
{
	if (!dest->pixels) return;

	#define getByte(value, n) (value >> (n * 8) & 0xFF)
	#define lerp(s, e, t) ((s) + ((e) - (s)) * (t))
	#define blerp(c00, c10, c01, c11, tx, ty) \
	(lerp(lerp((c00), (c10), (tx)), lerp((c01), (c11), (tx)), (ty)))

	U32 x, y;
	for (x = 0, y = 0; y < dest->height; x++) {
		if (x > dest->width) { x = 0; y++; }

		R32 gx = x / (R32)(dest->width ) * (SR_CanvasGetWidth (src) - 1);
		R32 gy = y / (R32)(dest->height) * (SR_CanvasGetHeight(src) - 1);
		I32 gxi = (int)gx;
		I32 gyi = (int)gy;

		/* TODO: Clean this up, preferably stop using SR_RGBAtoWhole, it's slow */

		U32 c00 = SR_CanvasGetPixel(src, gxi    , gyi    ).whole;
		U32 c10 = SR_CanvasGetPixel(src, gxi + 1, gyi    ).whole;
		U32 c01 = SR_CanvasGetPixel(src, gxi    , gyi + 1).whole;
		U32 c11 = SR_CanvasGetPixel(src, gxi + 1, gyi + 1).whole;

		U32 result = 0;
		for (U8 i = 0; i < 4; i++)
			result |= (U8)blerp(
				(R32)getByte(c00, i), (R32)getByte(c10, i), (R32)getByte(c01, i), (R32)getByte(c11, i),
				(R32)gx - gxi, (R32)gy - gyi
			) << (8 * i);

		SR_RGBAPixel final = {
			.whole = result
		};

		SR_CanvasSetPixel(dest, x, y, final);
	}

	#undef getByte
	#undef lerp
	#undef blerp
}

static X0 SR_NearestNeighborCanvasScale(
	SR_Canvas *src,
	SR_Canvas *dest)
{
	if (!dest->pixels) return;

	R32 x_factor = (R32)src->width  / (R32)dest->width;
	R32 y_factor = (R32)src->height / (R32)dest->height;
	
	U16 x, y;
	for (x = 0; x < dest->width ; x++)
	for (y = 0; y < dest->height; y++) {
		SR_RGBAPixel sample = SR_CanvasGetPixel(src, x * x_factor, y * y_factor);
		SR_CanvasSetPixel(dest, x, y, sample);
	}
}

X0 SR_CanvasScale(
	SR_Canvas *src,
	SR_Canvas *dest,
	I8 mode)
{
	switch (mode) {
	default:
	case SR_SCALE_NEARESTN:
		SR_NearestNeighborCanvasScale(src, dest);
		
		break;
	case SR_SCALE_BILINEAR:
		SR_BilinearCanvasScale(src, dest);

		break;
	}
}

SR_BBox SR_NZBoundingBox(SR_Canvas *src)
{
	/* TODO: Test this for bugs
	 * TODO: Find some way to clean up the repetition here
	 */

	/* Static declaration prevents a dangling pointer */
	__extension__ SR_BBox bbox = {};
	U16 xC, yC, firstX, firstY, lastX, lastY, x, y;

	for (y = 0; y < src->height; y++)
	for (x = 0; x < src->width ; x++)
		if (SR_CanvasPixelCNZ(src, x, y))
			{ firstX = x, firstY = y; goto srnzbbx_first_pixel_done; }
	
	goto srnzbbx_empty; /* No data found in image - commit die */
srnzbbx_first_pixel_done: /* Exit loop */
	for (y = src->height - 1; y > 0; y--)
	for (x = src->width  - 1; x > 0; x--)
		if (SR_CanvasPixelCNZ(src, x, y))
			{ lastX = x, lastY = y; goto srnzbbx_last_pixel_done; }

	goto srnzbbx_empty;
srnzbbx_last_pixel_done:
	for (xC = 0     ; xC <= firstX; xC++)
	for (yC = firstY; yC <= lastY ; yC++)
		if (SR_CanvasPixelCNZ(src, xC, yC)) {
			bbox.named.sx = xC; bbox.named.sy = firstY;
			goto srnzbbx_found_first;
		}

	goto srnzbbx_empty;
srnzbbx_found_first:
	for (xC = src->width - 1; xC >  lastX ; xC--)
	for (yC = lastY         ; yC >= firstY; yC--)
		if (SR_CanvasPixelCNZ(src, xC, yC)) {
			bbox.named.ex = xC; bbox.named.ey = lastY;
			goto srnzbbx_bounded;
		}

	goto srnzbbx_no_end_in_sight; /* No last point found - is this possible? */
srnzbbx_no_end_in_sight:
	bbox.named.ex = src->width - 1; bbox.named.ey = src->height - 1;
srnzbbx_bounded:
	return bbox; /* Return the box (er, I mean RETURN THE SLAB) */
srnzbbx_empty:
	/* We can return a 0-size box if we believe there is nothing here. */
	bbox.whole = 0;
	goto srnzbbx_bounded;
}

SR_Canvas SR_RefCanvTile(
	SR_Canvas *atlas,
	U16 tile_w,
	U16 tile_h,
	U16 col,
	U16 row)
{
	U16 columns, rows;
	columns = ceilf((R32)atlas->width  / tile_w);
	rows    = ceilf((R32)atlas->height / tile_h);

	return SR_RefCanv(atlas,
		(col % columns) * tile_w,
		(row % rows   ) * tile_h,
		tile_w, tile_h, false);
}
