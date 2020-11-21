#include "tris.h"

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
		
		for (U16 xx = ax; xx < bx; xx++)
		{
			/* interpolate z by plane equation */
			/* using I16 here, worried that vertices with large z wont fit */
			I16 normal_x = ((I16)t1.y - (I16)t0.y) * ((I16)t2.z - (I16)t0.z);
			normal_x -= ((I16)t1.z - (I16)t0.z) * ((I16)t2.y - (I16)t0.y);
			I16 normal_y = ((I16)t1.z - (I16)t0.z) * ((I16)t2.x - (I16)t0.x);
			normal_y -= ((I16)t1.x - (I16)t0.x) * ((I16)t2.z - (I16)t0.z);
			I16 normal_z = ((I16)t1.x - (I16)t0.x) * ((I16)t2.y - (I16)t0.y);
			normal_z -= ((I16)t1.y - (I16)t0.y) * ((I16)t2.x - (I16)t0.x);
			U32 zz = (U32)(t0.z);
			zz -= (U32)(
				(normal_x * ((I16)xx - (I16)t0.x) + normal_y * (I16)(yy)) /
				normal_z);
			if (SR_CanvasGetPixel(zbuf, xx, yy + t0.y).whole < zz) {
				SR_RGBAPixel new_z = SR_CreateRGBA(0,0,0,0);
				new_z.whole = zz;
				SR_CanvasSetPixel(zbuf, xx, yy + t0.y, new_z);
				SR_CanvasSetPixel(canvas, xx, t0.y + yy, tri.colour);
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
