#include "tris.h"

// TODO: make me healthier
X0 Trifill(
	SR_Canvas *canvas,
	SR_ScreenTriangle tri)
{
	SR_ScreenVertex t0 = tri.v0;
	SR_ScreenVertex t1 = tri.v1;
	SR_ScreenVertex t2 = tri.v2;
	
	// exit for nonsense triangles
	if (t0.y==t1.y && t0.y==t2.y) return;
	
	// vertex sort by y, t0 at the top, t1 in the middle and t2 on the bottom
	if (t0.y > t1.y) SWAPVERTEX(t0, t1);
	if (t0.y > t2.y) SWAPVERTEX(t0, t2);
	if (t1.y > t2.y) SWAPVERTEX(t1, t2);
	
	U16 t_height = t2.y - t0.y;
	
	for (U16 yy = 0; yy < t_height; yy++)
	{
		U8 s_half = (yy > t1.y - t0.y || t1.y == t0.y);
		U16 s_height = s_half ? t2.y - t1.y : t1.y - t0.y;
		float aa = (float)yy/t_height;
		float bb = (float)(yy-(s_half ? t1.y - t0.y : 0))/s_height;
		
		U16 ax = t0.x + (t2.x - t0.x) * aa;
		U16 bx = s_half ? t1.x + (t2.x - t1.x) * bb : t0.x + (t1.x - t0.x) * bb;
		U16 a2x;
		U16 b2x;
		
		if (ax > bx) {
			a2x = bx;
			b2x = ax;
		} else {
			a2x = ax;
			b2x = bx;
		}
		
		// draw a scanline
		for (U16 xx = a2x; xx <= b2x; xx++)
		{
			// TODO: compare z value to depth buffer and such
			SR_CanvasSetPixel(canvas, xx, t0.y + yy, tri.colour);
		}
	}
}

X0 SR_RenderTris(
	SR_Canvas *canvas,
	SR_ScreenTriangle *trilist,
	U32 list_length)
{
	// just iterate over a list of tris and draw them
	for (U32 i = 0; i < list_length; i++)
	{
		Trifill(canvas, trilist[i]);
	}
}
