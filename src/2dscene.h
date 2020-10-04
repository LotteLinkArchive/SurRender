#ifndef SUR2D_HEADER_FILE
#define SUR2D_HEADER_FILE
#include "glbl.h"
#include "canvas.h"

typedef struct {
	/* Mandatory details (identity tag, type specification, user data, draw position)
	 * `tag` and `type` are 32-bit hashes.
	 */
	U32    tag;
	U32    type;
	X0    *udata;
	R32x3  pos;

	/* This is the atlas used to get the correct tile texture for this 2D object. */
	SR_AtlasCanvas atlas;
	
	/* The position of the tile in the atlas to use for this 2D object. */
	U16 tilex;
	U16 tiley;

	/* The visual width and height of the 2D object. The object's texture will be tiled to this size. */
	U32 width;
	U32 height;
} SR_2DObject;
#endif
