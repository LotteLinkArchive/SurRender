#include "tris.h"
#include "remote_holy.h"

/* This is a private, inlined function. Only the array triangle fill needs to be public. */
FORCED_STATIC_INLINE X0 Trifill(
	SR_Canvas *canvas,
	SR_ScreenTriangle tri)
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

		U16 a2x = ax;
		U16 b2x = bx;
		
		for (U16 xx = a2x; xx < b2x; xx++)
		{
			/* TODO: compare z value to depth buffer and such */
			SR_CanvasSetPixel(canvas, xx, t0.y + yy, tri.colour);
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
	for (U32 i = 0; i < list_length; i++) Trifill(canvas, trilist[i]);
}
