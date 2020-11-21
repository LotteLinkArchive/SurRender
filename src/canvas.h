#ifndef SURCV_HEADER_FILE
#define SURCV_HEADER_FILE
#include "glbl.h"
#include "colours.h"
#include "errors.h"
#include <math.h>

/* Must be (a power of 2) - 1. The larger the size, the larger the modulo LUT overhead! */
#ifndef SR_MAX_CANVAS_SIZE
#define SR_MAX_CANVAS_SIZE 4095
#endif

/* This is a canvas, which contains a width and height in pixels, an aspect
 * ratio and a pointer to an array of pixel values.
 */
typedef struct SR_Canvas {
	/* General public properties */
	U16 width;
	U16 height;
	R32 ratio;

	/* Pointer to an array of pixels */
	SR_RGBAPixel *pixels;
	
	/* Coordinates to clip off, starting from 0, 0 (allow full canvas) */
	U16 xclip;
	U16 yclip;

	/* Internal canvas properties - FORMAT:
	 * 0 b 0 0 0 0 0 0 0 0
	 *     X | | | | | | \- Canvas is a reference to another canvas' pixels
	 *       | | | | | \--- Canvas is indestructible
	 *       | | | | \----- Canvas is important             [UNIMPLEMENTED]
	 *       | | | \------- Canvas is a memory-mapped file
	 *       | | \--------- Canvas Rsize is a power of two
	 *       | \----------- Canvas Csize is a power of two
	 *       \------------- Canvas needs an equivalently sized depth buffer
	 */
	U8 hflags;

	/* The "Real data" width and height, used for preventing segfaults due to invalid access positions */
	U16 rwidth;
	U16 rheight;

	/* The clipping width and height, used for ignoring segments of the source data, useful for reference
	 * canvases
	 */
	U16 cwidth;
	U16 cheight;

	/* The amount of references a canvas has. A canvas cannot be destroyed if its reference count is above 0. */
	U32 references;
	X0 *refsrc; /* Keep track of the referenced source so we can decrement the reference counter on it. */

	/* Base address to free. */
	X0 *b_addr;
	
	/* Size to feed to munmap */
	SX munmap_size;

	/* If needed, a reference to a depth buffer canvas */
	SR_RGBAPixel *depth_buffer;
} SR_Canvas;

/* An Atlas Canvas is a canvas supplied with a tile width and tile height to make it easier to retrieve tiles in a
 * texture atlas.
 */
typedef struct SR_AtlasCanvas {
	U16 twidth;
	U16 theight;
	SR_Canvas *canvas;
} SR_AtlasCanvas;

/* Bounding box */
typedef union {
	struct {
		U16 sx;
		U16 sy;
		U16 ex;
		U16 ey;
	} named;
	U64 whole;
	U16x4 parts;
} SR_BBox;

/* All supported scaling modes for canvases */
enum SR_ScaleModes {
	SR_SCALE_NEARESTN,
	SR_SCALE_BILINEAR
};

enum SR_NewCanvasFlags {
	SR_CANVAS_IMPORTANT = 0x04,
	SR_CANVAS_DEPTH     = 0x40,
	SR_CANVAS_NODESTROY = 0x02
};

/* Returns an appropriate HFLAG if tex is power of 2 */
#define SR_CPow2FDtc(w, h, flag) \
((((w) & ((w) - 1)) || ((h) & ((h) - 1))) ? 0 : (flag))

/* Generate the modulo LUT for a canvas. This should not be used by normal people. */
X0 SR_GenCanvLUT(SR_Canvas *canvas);

/* Make a canvas larger or smaller. Preserves the contents, but not
 * accurately. May ruin the current contents of the canvas.
 */
STATUS SR_ResizeCanvas(
	SR_Canvas *canvas,
	U16 width,
	U16 height);

/* Change the visual width/height of a canvas without modifying the real
 * width and height, allowing you to tile a texture or something without
 * consuming any memory or doing any difficult operations.
 */
X0 SR_TileTo(
	SR_Canvas *canvas,
	U16 width,
	U16 height);

/* A canvas may contain garbage data when initially created. This will zero fill it for you, if needed. */
X0 SR_ZeroFill(SR_Canvas *canvas);

/* Create a new canvas of the given size. (Will initially be filled with garbage data, use SR_ZeroFill to correct this)
 * 
 * target -> Must be a blank, unused SR_Canvas variable. You can create it like this...
 *  * SR_Canvas mycanvas = {};
 * (or by using memset)
 *
 * flags -> see SR_NewCanvasFlags
 */
STATUS SR_NewCanvas(SR_Canvas *target, U16 width, U16 height, U8 flags);

/* Get the height and width of a canvas */
#define SR_CanvasGetWidth(canvas) ((canvas)->width)
#define SR_CanvasGetHeight(canvas) ((canvas)->height)

/* Calculate the "real" size (in memory) of a canvas - not really recommended to use this yourself. */
#define SR_CanvasCalcSize(canvas) ((U32)(\
	(U32)((canvas)->rwidth)  *\
	(U32)((canvas)->rheight) *\
	sizeof(SR_RGBAPixel)\
))

/* Calculate the "real" position of a pixel in the canvas - not really recommended to use this yourself. */
U32 SR_CanvasCalcPosition(
	SR_Canvas *canvas,
	U32 x,
	U32 y);

/* Check if a pixel is out of bounds */
#define SR_CanvasCheckOutOfBounds(canvas, x, y)   \
(((((canvas)->xclip) + (x)) >= (canvas)->width || \
(((canvas)->yclip) + (y)) >= (canvas)->height) ? true : false)

/* Modulo LUT */
#define SR_MXCS_P1 SR_MAX_CANVAS_SIZE + 1
extern U16 modlut[SR_MXCS_P1][SR_MXCS_P1];
extern U1  modlut_complete   [SR_MXCS_P1];
#undef SR_MXCS_P1

/* Calculate the in-memory (or 1 dimensional) position of a pixel in the canvas based on its X and Y coordinates. */
#define SR_AxisPositionCRCTRM(a, b, c, d) (modlut[(a)][\
	(modlut[(b)][(c) & SR_MAX_CANVAS_SIZE] + (d)) & SR_MAX_CANVAS_SIZE])
#define SR_CombnAxisPosCalcXY(canvas, x, y) ((U32)x) + (U32)((canvas)->rwidth * (U32)(y))
#define SR_CanvasCalcPosition(canvas, x, y) SR_CombnAxisPosCalcXY((canvas), \
	SR_AxisPositionCRCTRM((canvas)->rwidth, (canvas)->cwidth, (x), (canvas)->xclip), \
	SR_AxisPositionCRCTRM((canvas)->rheight, (canvas)->cheight, (y), (canvas)->yclip))

/* Calculate a uniform position on the canvas with a float between the range 0 and 1.
 *
 * E.g for a 256x128 canvas...
 *
 * X 1.0  Y 1.0  -> X 256 Y 128
 * X 0.0  Y 0.0  -> X 0   Y 0
 * X 0.13 Y 0.82 -> X 33  Y 105
 */
#define SR_CanvasUniformPos(canvas, x, y) SR_CanvasCalcPosition((canvas),\
	(U16)round((float)(x) * (float)((canvas)->width)),\
	(U16)round((float)(y) * (float)((canvas)->height)))

/* Set the value of a pixel in the canvas */
#define SR_CanvasSetPixel(canvas, x, y, pixel) (canvas)->pixels[SR_CanvasCalcPosition((canvas), (U16)(x), (U16)(y))] =\
(pixel)

/* Get a pixel in the canvas */
#define SR_CanvasGetPixel(canvas, x, y) ((canvas)->pixels[SR_CanvasCalcPosition((canvas), (U16)(x), (U16)(y))])

/* Check if a pixel is non-zero, hopefully */
#define SR_CanvasPixelCNZ(canvas, x, y) \
(SR_CanvasGetPixel((canvas), (x), (y)).chn.alpha != 0)

/* Destroy the in-memory representation of the canvas
 * (Must create a new canvas or resize the current one in order to access)
 * 
 * See errors.h for STATUS codes. SR_DestroyCanvaas errors are typically
 * non-fatal (aka the program will not crash) but may be responsible for a
 * memory leak (you should report them as warnings, not errors)
 */
STATUS SR_DestroyCanvas(SR_Canvas *canvas);

/* Check if the canvas has been successfully allocated. You must ALWAYS
 * check if a canvas is valid before you use it.
 */
#define SR_CanvasIsValid(canvas) (BOOLIFY((canvas)->pixels))

/* Fills the whole "new" canvas with the contents of the original canvas.
 *
 * Copying from the original canvas starts at (copy_start_x, copy_start_y)
 */
X0 SR_CopyCanvas(
	SR_Canvas *canvas,
	SR_Canvas *dest,
	U16 copy_start_x,
	U16 copy_start_y);

/* Takes pretty much the same arguments as SR_CopyCanvas, except it doesn't
 * malloc a new canvas - it returns essentially a segment of the original
 * canvas instead. This doesn't allocate any memory, but it relies on the
 * host canvas still existing in memory.
 * 
 * When you create a reference canvas, it increments the reference count of
 * the source canvas, which prevents it from being destroyed. You must
 * destroy all reference canvases when you are done using them, otherwise
 * your source canvas will become indestructible. The source canvas can
 * only be destroyed when it is referenced by zero reference canvases.
 * 
 * The act of destroying a reference canvas will decrement the reference
 * counter of the source canvas automatically.
 * 
 * If you attempt to create a reference to a reference canvas, this
 * function will automatically resolve to the original canvas and
 * increment the reference count of the original accordingly. That way,
 * you won't need to destroy your reference canvases in any particular
 * order.
 * 
 * If absorb_host is enabled, reference counting is disabled and the result
 * of SR_RefCanv is nolonger a reference canvas but rather a normal canvas
 * that just points to the same pixels as another one. In that case, you
 * can safely discard the source canvas without destroying it, as long as
 * you destroy the absorbed version at some point.
 */
SR_Canvas SR_RefCanv(
	SR_Canvas *src,
	U16 xclip,
	U16 yclip,
	U16 width,
	U16 height,
	U1  absorb_host);

/* Scales the source canvas into the destination canvas. Bad things will
 * happen if the source and destination point to the same canvas.
 * The new width and height is the width and height of the destination
 * canvas.
 * 
 * BOTH SOURCE AND DESTINATION MUST BE A VALID CANVAS. IF THIS IS NOT THE
 * CASE, EXPECT UNDEFINED BEHAVIOUR.
 */
X0 SR_CanvasScale(
	SR_Canvas *src,
	SR_Canvas *dest,
	I8 mode);

/* Returns a SR_BBox representing the bounding box of a canvas.
 * The first 2 values are the x, y coordinates of the top left of the
 * bounding box. The last 2 values are the x, y coordinates of the bottom
 * right of the bounding box.
 * 
 * If the canvas is empty, returns all zeros.
 * 
 * The bounding box is created when a non-zero value is found.
 * 
 * Note that this is a particularly slow operation.
 */
SR_BBox SR_NZBoundingBox(SR_Canvas *src);

/* Creates a reference canvas based on a given tile inside another canvas.
 * The col/row argument is modulo'd by the amount of columns and rows in
 * the host canvas, so it's impossible to go out of bounds.
 * 
 * Note that destroying a reference canvas' host canvas will cause the
 * reference canvas to contain a dangling pointer, causing a segfault if
 * you attempt to access it. Only destroy the host canvas when you are done
 * using all of the references and you are absolutely sure that the
 * reference canvas will never be accessed.
 */
SR_Canvas SR_RefCanvTile(
	SR_Canvas *atlas,
	U16 tile_w,
	U16 tile_h,
	U16 col,
	U16 row);

/* Get a reference canvas which points to the depth buffer of the source
 * canvas. Has the same limitations as SR_RefCanv. You MUST destroy this
 * reference after you're done using it with SR_DestroyCanvas, or the
 * source canvas will become indestructible (you will create a memory
 * leak).
 *
 * Do NOT call this on a canvas that has no depth buffer.
 */
SR_Canvas SR_RefCanvDepth(
	SR_Canvas *src,
	U16 xclip,
	U16 yclip,
	U16 width,
	U16 height,
	U1  absorb_host);

/* Clear the whole memory range for the depth buffer of a canvas. Includes
 * any clipped regions.
 *
 * Fill value could be any 8-bit value, either 0xFF and 0x00 are fine.
 */
void SR_EraseCanvDepth(SR_Canvas *target, U8 fill);
#endif
