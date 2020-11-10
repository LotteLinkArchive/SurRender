#ifndef SURCNVBL_HEADER_FILE
#define SURCNVBL_HEADER_FILE

#include "canvas.h"

/* Allows you to blend/merge a source canvas on to a destination canvas.
 * Can be pasted at a given offset (paste_start_x and paste_start_y)
 * Uses alpha modifier and mode values just like SR_RGBABlender. Usually
 * you'll just want a modifier of 255 and mode SR_BLEND_ADDITIVE.
 * 
 * Note that if the base canvas is completely transparent (which will be
 * the case if you create a completely new canvas) then you will want to
 * use SR_BLEND_REPLACE, as other blend modes will re-use the alpha value
 * of the base canvas in order to perform correct blending.
 */
X0 SR_MergeCanvasIntoCanvas(
	SR_Canvas *dest_canvas,
	SR_Canvas *src_canvas,
	U16 paste_start_x,
	U16 paste_start_y,
	U8 alpha_modifier,
	I8 mode);

#endif
