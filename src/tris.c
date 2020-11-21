#include "tris.h"
#include "canvas.h"
#include <omp.h>

/* This is a private, inlined function. Only the array triangle fill needs to be public. */
FORCED_STATIC_INLINE X0 Trifill(
	SR_Canvas *canvas,
	SR_ScreenTriangle tri,
	SR_Canvas *zbuf)
{
	/* TODO: don't do this */
	#define t0 tri.vx[0]
	#define t1 tri.vx[1]
	#define t2 tri.vx[2]
	
	/* Vertex sort by y, t0 at the top, t1 in the middle and t2 on the bottom */
	if (t0.y > t1.y) SWAP(t0, t1);
	if (t0.y > t2.y) SWAP(t0, t2);
	if (t1.y > t2.y) SWAP(t1, t2);
	
	U16 t_height = t2.y - t0.y;
	
	I32 normal_x = ((I32)t1.y - (I32)t0.y) * (t2.z - t0.z);
	normal_x -= (t1.z - t0.z) * ((I32)t2.y - (I32)t0.y);
	I32 normal_y = (t1.z - t0.z) * ((I32)t2.x - (I32)t0.x);
	normal_y -= ((I32)t1.x - (I32)t0.x) * (t2.z - t0.z);
	I32 normal_z = ((I32)t1.x - (I32)t0.x) * ((I32)t2.y - (I32)t0.y);
	normal_z -= ((I32)t1.y - (I32)t0.y) * ((I32)t2.x - (I32)t0.x);

	for (U16 yy = 0; yy < t_height; yy++)
	{
		/* TODO: EXPLAIN THIS, DETAILED COMMENTS
		 * TODO: CLEARER VARIABLE NAMES
		 * that's about it.
		 */
		U8  s_half   = (yy > t1.y - t0.y || t1.y == t0.y);
		U16 s_height = s_half ? t2.y - t1.y : t1.y - t0.y;

		R32 aa = (R32)yy / t_height;
		R32 bb = (R32)(yy - (s_half ? t1.y - t0.y : 0)) / s_height;
		
		U16 ax = t0.x + (t2.x - t0.x) * aa;
		U16 bx = s_half ? t1.x + (t2.x - t1.x) * bb : t0.x + (t1.x - t0.x) * bb;

		if (ax > bx) SWAP(ax, bx);
		
		/* Correct the Y value for data height, clipping height and Y clipping distance */
		U16 ycorrected = SR_AxisPositionCRCTRM(canvas->rheight, canvas->cheight, yy + t0.y, canvas->yclip);

		#pragma omp simd
		for (U16 xx = ax; xx < bx; xx++)
		{
			/* Correct the X value for data width, cipping width and X clipping distance,
			 * then turn the corrected X and Y values into an array index for the pixel array.
			 */
			U32 gindex = SR_CombnAxisPosCalcXY(canvas, SR_AxisPositionCRCTRM(
				canvas->rwidth, canvas->cwidth, xx, canvas->xclip), ycorrected);

			/* Calculcate the Z position of this pixel. */
			U32 zz = (U32)(t0.z) - (U32)(
				(normal_x * ((I32)xx - (I32)t0.x) + normal_y * (I32)(yy)) / normal_z);

			if (zbuf->pixels[gindex].whole <= zz) {
				/* Update the Z buffer */
				zbuf->pixels[gindex].whole = zz;

				/* Set the colour of the destination pixel. */
				canvas->pixels[gindex] = tri.colour;
			}
		}
	}

	#undef t0
	#undef t1
	#undef t2
}

X0 SR_RenderTris(
	SR_Canvas *canvas,
	SR_ScreenTriangle *trilist,
	U32 list_length)
{
	SR_Canvas z_buffer = SR_RefCanvDepth(canvas, 0, 0,
		canvas->width, canvas->height, false);
	for (U32 i = 0; i < list_length; i++) Trifill(canvas, trilist[i], &z_buffer);
	SR_DestroyCanvas(&z_buffer);
}
