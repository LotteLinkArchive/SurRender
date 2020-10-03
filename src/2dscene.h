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

	/* Pointer to an array of pointers to canvases. Each index in the array leads to a pointer to a frame to use
	 * for animating the 2D object. Index 0 is a pointer to the pointer for the first frame.
	 */
	SR_Canvas **sprites;

	/* Index in the sprite canvas array to use (0 for static sprites) */
	SX drwindex;
} SR_2DObject;
#endif
