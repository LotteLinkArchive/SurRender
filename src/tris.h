#include "glbl.h"
#include "canvas.h"
#include "colours.h"

typedef union {
	struct {
		U16 x;
		U16 y;
		U32 z;
	} cmp; // "cmp" is shorthand for "components"
	U64 whole;
} SR_ScreenVertex;

typedef struct {
	SR_ScreenVertex v0;
	SR_ScreenVertex v1;
	SR_ScreenVertex v2;
} SR_ScreenTriangle;

X0 SR_RenderTris(
	SR_Canvas *canvas,
	SR_ScreenTriangle *trilist,
	U32 list_length);
