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

/* Private vector types, particularly used for alpha blending */
typedef union {
	U8x64  sU8x64;
	U16x32 sU16x32;
	U64x8  sU64x8;
	U32x16 sU32x16;
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
	if      ( canvas->hflags     & 0x02) return SR_CANVAS_CONSTANT;
	else if ( canvas->references > 0x00) return SR_NONZERO_REFCOUNT;
	else if (!canvas->pixels           ) return SR_NULL_CANVAS;
	else if ( canvas->hflags     & 0x01) {
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
		.hflags  = absorb_host ? 0x00 : 0x01,
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

static inline __attribute__((always_inline)) pixbuf_t SR_PixbufBlend(
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
	case SR_BLEND_DIRECT_XOR:
		destbuf.sU32x16 = srcBbuf.sU32x16 ^ (srcAbuf.sU32x16 & 0x00FFFFFF);

		break;
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
	pixbuf_t srcAbuf, srcBbuf, destbuf;

	/* CLUMPS represents the amount of pixels that can be stored in an AVX-512-compatible vector */
	#define CLUMPS (sizeof(pixbuf_t) / sizeof(SR_RGBAPixel))

	/* emax represents the total number of clumps in each row of the source canvas */
	emax = ((src_canvas->width + CLUMPS) - 1) / CLUMPS;

	/* fsub represnts the number of extra pixels in each clumped row, e.g a 125 pixel row with 16-pixel clumps
	 * would have 3 extra pixels that should not be overwritten */
	fsub = (emax * CLUMPS) - src_canvas->width;

	U16 isxmap[CLUMPS], idxmap[CLUMPS];
	U32 cxybmap[CLUMPS];

	for (x = 0; x < emax; x++) {
		/* We can calculate the X position stuff here instead of per-clump in order to prevent any extra
		 * pointless calculations */
		fstate  = x + 1 == emax ? CLUMPS - fsub : CLUMPS;
		for (z = 0; z < fstate; z++) {
			srcposx = (x * CLUMPS) + z;

			isxmap[z] = SR_AxisPositionCRCTRM(
				src_canvas->rwidth, src_canvas->cwidth, srcposx, src_canvas->xclip);
			idxmap[z] = SR_AxisPositionCRCTRM(
				dest_canvas->rwidth, dest_canvas->cwidth, srcposx + paste_start_x, dest_canvas->xclip);
		}

		for (y = 0; y < src_canvas->height; y++) {
			/* We already have the X position, so we don't need to calculate it. We CAN calculate the Y
			 * positions now, however. */
			isy = SR_AxisPositionCRCTRM(
				src_canvas->rheight, src_canvas->cheight , y, src_canvas->yclip);
			idy = SR_AxisPositionCRCTRM(
				dest_canvas->rheight, dest_canvas->cheight, y + paste_start_y, dest_canvas->yclip);

			/* Copy the top layer and bottom layer pixel clumps into a malleable buffer. */
			for (z = 0; z < fstate; z++) {
				srcAbuf.sU32x16[z] = src_canvas->pixels [
					SR_CombnAxisPosCalcXY(src_canvas, isxmap[z], isy)].whole;
				cxybmap[z] = SR_CombnAxisPosCalcXY(dest_canvas, idxmap[z], idy);
				srcBbuf.sU32x16[z] = dest_canvas->pixels[cxybmap[z]].whole;
			}
			
			destbuf = SR_PixbufBlend(srcAbuf, srcBbuf, alpha_modifier, mode);

			for (z = 0; z < fstate; z++) dest_canvas->pixels[cxybmap[z]].whole = destbuf.sU32x16[z];
		}
	}

	#undef CLUMPS

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

SR_OffsetCanvas SR_CanvasShear(
	SR_Canvas *src,
	I32 skew_amount,
	U1 mode)
{
	U16 w, h, mcenter;
	R32 skew;

	w = src->width;
	h = src->height;
	mcenter = mode ? w >> 1 : h >> 1;
	skew = (R32)skew_amount / (R32)mcenter;
	skew_amount = abs(skew_amount);

	__extension__ SR_OffsetCanvas final = {};

	if (mode) SR_NewCanvas(&final.canvas, w, h + (skew_amount << 1)); /* @warn: couldfail */
	else      SR_NewCanvas(&final.canvas, w + (skew_amount << 1), h); /* @warn: couldfail */
	SR_ZeroFill(&(final.canvas));

	final.offset_x = mode ? 0 : -skew_amount;
	final.offset_y = mode ? -skew_amount : 0;

	U16 x, y;
	I32 mshift;

	#define TMP_SKEWTRFS(ol, olc, il, ilc, ils0, ils1)\
	for (ol = 0; olc; ol++) {\
		mshift = skew_amount + (ol - mcenter) * skew;\
		for (il = 0; ilc; il++) {\
			SR_RGBAPixel pixel = SR_CanvasGetPixel(src, x, y);\
			SR_CanvasSetPixel(&(final.canvas), ils0, ils1, pixel);\
		}\
	}

	if (mode) TMP_SKEWTRFS(x, x < w, y, y < h, x, y + mshift)
	else      TMP_SKEWTRFS(y, y < h, x, x < w, x + mshift, y)
	#undef TMP_SKEWTRFS

	return final;
}

SR_OffsetCanvas SR_CanvasRotate(
	SR_Canvas *src,
	R32 degrees,
	U1 safety_padding,
	U1 autocrop)
{
	/* Declare everything here */
	SR_Canvas temp;
	U16 w, h, boundary, xC, yC, nx, ny;
	I32 x, y, nxM, nyM, half_w, half_h;
	R32 the_sin, the_cos;
	SR_RGBAPixel pixel, pixbuf;
	__extension__ SR_OffsetCanvas final = {};

	/* There's no point in considering unique values above 359. 360 -> 0 */
	degrees = fmod(degrees, 360);

	/* For simplicity's sake */
	w = src->width;
	h = src->height;

	final.offset_x = 0;
	final.offset_y = 0;

	if (safety_padding) {
		/* Create additional padding in case rotated data goes off-canvas */
		boundary = MAX(w, h) << 1; /* Double the largest side length */
		SR_NewCanvas(&final.canvas, boundary, boundary); /* @warn: couldfail */
		final.offset_x = -(int)(boundary >> 2);
		final.offset_y = -(int)(boundary >> 2);
	} else {
		SR_NewCanvas(&final.canvas, w, h); /* @warn: couldfail */
	}
	/* Prevent garbage data seeping in */
	SR_ZeroFill(&final.canvas);

	/* Rotation not 0, 90, 180 or 270 degrees? Use inaccurate method instead */
	if (fmod(degrees, 90) != .0) goto srcvrot_mismatch;

	/* Trying to rotate 0 degrees? Just copy the canvas, I guess. */
	if (!((U16)degrees % 360)) {
		SR_MergeCanvasIntoCanvas(
			&final.canvas,
			src,
			-final.offset_x, /* Still need to use the offset incase padding */
			-final.offset_y,
			255,
			SR_BLEND_REPLACE); /* Fastest and safest, also no background alpha */

		goto srcvrot_finished; /* Jump to the finishing/cleanup line */
	}

	/* TODO: Mismatching width and height causes accurate rotation bug.
	 * We should probably fix this, but for now it's fine to use inaccurate
	 * rotation.
	 */
	if (w != h) goto srcvrot_mismatch;

	/* This is the accurate rotation system, but it only works on degrees where x % 90 == 0 */
	for (xC = 0; xC < w; xC++)
	for (yC = 0; yC < h; yC++) {
		pixbuf = SR_CanvasGetPixel(src, xC, yC);
		nx = 0, ny = 0;
		switch (((U16)degrees) % 360) {
		case 90:
			nx = (h - 1) - yC;
			ny = xC;
			break;
		case 180:
			nx = (w - 1) - xC;
			ny = (h - 1) - yC;
			break;
		case 270:
			nx = yC;
			ny = (w - 1) - xC;
			break;
		}

		SR_CanvasSetPixel(
			&final.canvas,
			nx - final.offset_x, /* Correct for offset */
			ny - final.offset_y,
			pixbuf);
	}

	goto srcvrot_finished;

srcvrot_mismatch:
	/* Convert to radians and then modulo by 2pi */
	degrees = fmod(degrees * 0.017453292519943295, 6.28318530718);

	the_sin = -sin(degrees);
	the_cos = cos(degrees);
	half_w = w >> 1;
	half_h = h >> 1;

	for (x = -half_w; x < half_w; x++)
	for (y = -half_h; y < half_h; y++) {
		nxM = (x * the_cos + y * the_sin + half_w) - final.offset_x;
		nyM = (y * the_cos - x * the_sin + half_h) - final.offset_y;
		pixel = SR_CanvasGetPixel(src, x + half_w, y + half_h);

		/* Set target AND a single nearby pixel to de-alias */
		SR_CanvasSetPixel(&final.canvas, nxM	, nyM	, pixel);
		SR_CanvasSetPixel(&final.canvas, nxM - 1, nyM	, pixel);
	}

srcvrot_finished:
	if (autocrop) {
		/* If autocropping is enabled, auto-crop padded images. This is slow,
		 * but speeds up merging a fair bit. Use if you only intend to rotate once.
		 */
		SR_BBox bbox = SR_NZBoundingBox(&final.canvas);
		if (bbox.whole) {
			temp = SR_RefCanv(
				&final.canvas,
				bbox.named.sx,
				bbox.named.sy,
				(bbox.named.ex - bbox.named.sx) + 1,
				(bbox.named.ey - bbox.named.sy) + 1,
				true);

			final.canvas = temp;

			final.offset_x += bbox.named.sx;
			final.offset_y += bbox.named.sy;
		}
	}

	return final;
}

X0 SR_InplaceFlip(SR_Canvas *src, U1 vertical)
{
	/* Flipping canvases honestly doesn't need a new canvas to be allocated,
	 * so we can do it in-place just fine for extra speed and less memory usage.
	 */
	U16 x, y, wmax, hmax, xdest, ydest;
	SR_RGBAPixel temp, pixel;

	wmax = vertical ? src->width : src->width >> 1;
	hmax = vertical ? src->height >> 1 : src->height;

	for (x = 0; x < wmax; x++)
	for (y = 0; y < hmax; y++) {
		xdest = vertical ? x : (src->width  - 1) - x;
		ydest = vertical ? (src->height - 1) - y : y;

		temp  = SR_CanvasGetPixel(src, xdest, ydest);
		pixel = SR_CanvasGetPixel(src, x, y);
		SR_CanvasSetPixel(src, xdest, ydest, pixel);
		SR_CanvasSetPixel(src, x, y, temp);
	}
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
