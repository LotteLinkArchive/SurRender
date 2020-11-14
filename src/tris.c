#include "glbl.h"
#include "canvas.h"
#include "colours.h"
#include "tris.h"

X0 Trifill_Slow(
	SR_Canvas *canvas,
	SR_ScreenTriangle tri)
{
	U16 max_x = MAX(tri.v0.x, MAX(tri.v1.x, tri.v2.x));
	U16 min_x = MIN(tri.v0.x, MAX(tri.v1.x, tri.v2.x));
	U16 max_y = MAX(tri.v0.y, MAX(tri.v1.y, tri.v2.y));
	U16 min_y = MIN(tri.v0.y, MAX(tri.v1.y, tri.v2.y));
	
	U16 vs1x = tri.v1.x - tri.v0.x;
	U16 vs1y = tri.v1.y - tri.v0.y;
	U16 vs2x = tri.v2.x - tri.v0.x;
	U16 vs2y = tri.v2.y - tri.v0.y;
	
	I64 bx = (I64)(tri.v1.x) - (I64)(tri.v0.x);
	I64 by = (I64)(tri.v1.y) - (I64)(tri.v0.y);
	I64 bz = (I64)(tri.v1.z) - (I64)(tri.v0.z);
	
	I64 cx = (I64)(tri.v2.x) - (I64)(tri.v0.x);
	I64 cy = (I64)(tri.v2.y) - (I64)(tri.v0.y);
	I64 cz = (I64)(tri.v2.z) - (I64)(tri.v0.z);
	
	I64 csx = by * cz - bz * cy;
	I64 csy = bz * cx - bx * cz;
	I64 csz = bx * cy - by * cx;
	
	U16 xx, yy;
	for (xx = min_x; x <= max_x; x++)
	for (yy = min_y; y <= min_y; y++)
	{
		U16 qx = xx - tri.v0.x;
		U16 qy = yy - tri.v0.y;
		
		float c = (float)(vs1x * vs2y - vs1y * vs2x);
		float s = (float)(qx * vs2y - qy * vs2x) / c;
		float t = (float)(vs1x * qy - vs1y * qx) / c;
		
		if ((s>=0) && (t>=0) && ((s+t)<=1)) {
			U32 zz = tri.v0.z - (U32)(
				(
					(csx * (I64)(xx - tri.v0.x)) +
					(csy * (I64)(yy - tri.v0.y))
				) / csz
			)
			
			SR_CanvasSetPixel(canvas, xx, yy, tri.colour);
		}
	}
}

X0 Trifill(
	SR_Canvas *canvas,
	SR_ScreenTriangle tri)
{
	SR_ScreenVertex t0 = tri.v0;
	SR_ScreenVertex t1 = tri.v1;
	SR_ScreenVertex t2 = tri.v2;
	
	if (t0.y==t1.y && t0.y==t2.y) return;

	if (t0.y > t1.y) SWAPVERTEX(t0, t1);
	if (t0.y > t2.y) SWAPVERTEX(t0, t2);
	if (t1.y > t2.y) SWAPVERTEX(t1, t2);
	
	U16 t_height = t2.y - t0.y;
	
	for (U16 yy = 0; yy < t_height; yy++)
	{
		U8 s_half = (yy > t1.y - t0.y || t1.y == t0.y);
		U16 s_height = s_half ? t2.y - t1.y : t1.y - t0.y;
		float aa = (float)i/t_height;
		float bb = (float)(i-(s_half ? t1.y - t0.y : 0))/s_height;
		
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
	for (U32 i = 0; i < list_length; i++)
	{
		Trifill_Slow(canvas, trilist[i]);
	}
}
