#include "glbl.h"
#include "canvas.h"
#include "colours.h"

typedef struct {
	U16 x;
	U16 y;
	U32 z;
} SR_ScreenVertex;

typedef struct {
	SR_ScreenVertex v0;
	SR_ScreenVertex v1;
	SR_ScreenVertex v2;
	SR_RGBAPixel colour;
} SR_ScreenTriangle;

X0 Trifill_Slow(
	SR_Canvas *canvas,
	SR_ScreenTriangle tri);

X0 SR_RenderTris(
	SR_Canvas *canvas,
	SR_ScreenTriangle *trilist,
	U32 list_length);
