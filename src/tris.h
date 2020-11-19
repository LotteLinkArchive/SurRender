#ifndef SURTRI_HEADER_FILE
#define SURTRI_HEADER_FILE
#include "glbl.h"
#include "canvas.h"
#include "colours.h"

// screen vertex struct
typedef struct {
	U16 x;
	U16 y;
	U32 z;
} SR_ScreenVertex;

// vertex swapping macro
#define SWAPVERTEX(x, y) do { SR_ScreenVertex TEMP = x; y = x; x = TEMP; } while (0)

// screen triangle struct
typedef struct {
	SR_ScreenVertex v0;
	SR_ScreenVertex v1;
	SR_ScreenVertex v2;
	SR_RGBAPixel colour;
} SR_ScreenTriangle;

// triangle rasterizer, takes a triangle and rasterizes to a specified canvas
X0 Trifill(
	SR_Canvas *canvas,
	SR_ScreenTriangle tri);

// iterates over an array of tris and draws them all
X0 SR_RenderTris(
	SR_Canvas *canvas,
	SR_ScreenTriangle *trilist,
	U32 list_length);
#endif