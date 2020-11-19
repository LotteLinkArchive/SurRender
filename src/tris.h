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

// screen triangle struct
typedef struct {
	SR_ScreenVertex vx[3];
	SR_RGBAPixel colour;
} SR_ScreenTriangle;

// iterates over an array of tris and draws them all
X0 SR_RenderTris(
	SR_Canvas *canvas,
	SR_ScreenTriangle *trilist,
	U32 list_length);
#endif