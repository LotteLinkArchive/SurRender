#ifndef SURCV_HEADER_FILE
#define SURCV_HEADER_FILE
#include "glbl.h"
#include "colours.h"

// Must be (a power of 2) - 1
// The larger the size, the larger the modulo LUT overhead
#ifndef SR_MAX_CANVAS_SIZE
#define SR_MAX_CANVAS_SIZE 4095
#endif

/* This is a canvas, which contains a width and height in pixels, an aspect
 * ratio and a pointer to an array of pixel values.
 */
typedef struct SR_Canvas {
	// General public properties
	U16 width;
	U16 height;
	R32 ratio;

	// Pointer to an array of pixels
	SR_RGBAPixel *pixels;
	
	// Coordinates to clip off, starting from 0, 0 (allow full canvas)
	U16 xclip;
	U16 yclip;

	/* Internal canvas properties - FORMAT:
	 * 0 b 0 0 0 0 0 0 0 0
	 *   X X | | | | | \- Canvas is a reference to another canvas' pixels
	 *       | | | | \--- Canvas is indestructible
	 *       | | | \----- Canvas is important             [UNIMPLEMENTED]
	 *       | | \------- Canvas is a memory-mapped file  [UNIMPLEMENTED]
	 *       | \--------- Canvas Rsize is a power of two
	 *       \----------- Canvas Csize is a power of two
	 */
	U8 hflags;

	// The "Real data" width and height, used for preventing segfaults
	U16 rwidth;
	U16 rheight;

	// The clipping width and height, used for ignoring segments
	U16 cwidth;
	U16 cheight;
} SR_Canvas;

/* An SR_OffsetCanvas is just a regular canvas, but with additional offset
 * data which you will probably need in order to "place" the canvas
 * correctly using merge functions. Essentially, take the coordinates you
 * originally intended to paste the canvas at, and apply the offset values
 * to those coordinates.
 */
typedef struct SR_OffsetCanvas {
	I32 offset_x;
	I32 offset_y;
	SR_Canvas canvas;
} SR_OffsetCanvas;

// Bounding box
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

// All supported scaling modes for canvases
enum SR_ScaleModes {
	SR_SCALE_NEARESTN,
	SR_SCALE_BILINEAR
};

// Returns an appropriate HFLAG if tex is power of 2
#define SR_CPow2FDtc(w, h, flag) \
((((w) & ((w) - 1)) || ((h) & ((h) - 1))) ? 0 : (flag))

// Generate the modulo LUT for a canvas. This should not be used by normal
// people.
X0 SR_GenCanvLUT(SR_Canvas *canvas);

/* Make a canvas larger or smaller. Preserves the contents, but not
 * accurately. May ruin the current contents of the canvas.
 */
U1 SR_ResizeCanvas(
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

/* A canvas may contain garbage data when initially created. This will
 * zero fill it for you, if needed.
 */
X0 SR_ZeroFill(SR_Canvas *canvas);

// Create a new canvas of the given size
SR_Canvas SR_NewCanvas(U16 width, U16 height);

// Get the height and width of a canvas
#define SR_CanvasGetWidth(canvas) ((canvas)->width)
#define SR_CanvasGetHeight(canvas) ((canvas)->height)

/* Calculate the "real" size (in memory) of a canvas - not really
 * recommended to use this yourself.
 */
#define SR_CanvasCalcSize(canvas) ((U32)(\
	(U32)((canvas)->rwidth)  *\
	(U32)((canvas)->rheight) *\
	sizeof(SR_RGBAPixel)\
))

// Modulo LUT
#define SR_MXCS_P1 SR_MAX_CANVAS_SIZE + 1
__extension__ U16 modlut[SR_MXCS_P1][SR_MXCS_P1] = {};
__extension__ U1 modlut_complete[SR_MXCS_P1] = {};

X0 SR_FillModLUT(U16 moperand)
{
	if (modlut_complete[moperand]) goto sr_fmlutexit;

	modlut_complete[moperand] = true;
	for (U16 x = 0; x < SR_MXCS_P1; x++) modlut[moperand][x] = x % moperand;
	
sr_fmlutexit:
	return;
}
#undef SR_MXCS_P1

/* Calculate the "real" position of a pixel in the canvas - not really
 * recommended to use this yourself.
 */
inline  U32 SR_CanvasCalcPosition(
	SR_Canvas *canvas,
	U32 x,
	U32 y)
{
	x = modlut[canvas->cwidth ][x & SR_MAX_CANVAS_SIZE] + canvas->xclip;
	x = modlut[canvas->rwidth ][x & SR_MAX_CANVAS_SIZE];
	y = modlut[canvas->cheight][y & SR_MAX_CANVAS_SIZE] + canvas->yclip;
	y = modlut[canvas->rheight][y & SR_MAX_CANVAS_SIZE];

	return (canvas->rwidth * y) + x;
}

// Check if a pixel is out of bounds
#define SR_CanvasCheckOutOfBounds(canvas, x, y)   \
(((((canvas)->xclip) + (x)) >= (canvas)->width || \
(((canvas)->yclip) + (y)) >= (canvas)->height) ? true : false)

// Set the value of a pixel in the canvas
inline  X0 SR_CanvasSetPixel(
	SR_Canvas *canvas,
	U16 x,
	U16 y,
	SR_RGBAPixel pixel)
{
	canvas->pixels[SR_CanvasCalcPosition(canvas, x, y)] = pixel;
}

// Get a pixel in the canvas
inline  SR_RGBAPixel SR_CanvasGetPixel(
	SR_Canvas *canvas,
	U16 x,
	U16 y)
{
	return canvas->pixels[SR_CanvasCalcPosition(canvas, x, y)];
}

// Check if a pixel is non-zero, hopefully
#define SR_CanvasPixelCNZ(canvas, x, y) \
(SR_CanvasGetPixel((canvas), (x), (y)).whole != 0)

/* Destroy the in-memory representation of the canvas
 * (Must create a new canvas or resize the current one in order to access)
 */
X0 SR_DestroyCanvas(SR_Canvas *canvas);

/* Check if the canvas has been successfully allocated. You must ALWAYS
 * check if a canvas is valid before you use it.
 */
#define SR_CanvasIsValid(canvas) (BOOLIFY((canvas)->pixels))

/* Malloc a new canvas of given size and start copying every pixel from the
 * specified old canvas to the new one, starting at the given position.
 * This also allows you to create cropped versions of a canvas! :)
 * Note: This does not destroy the old canvas. If you don't need it anymore
 * don't forget to destroy it, or it will remain allocated.
 */
SR_Canvas SR_CopyCanvas(
	SR_Canvas *canvas,
	U16 copy_start_x,
	U16 copy_start_y,
	U16 new_width,
	U16 new_height);

/* Takes pretty much the same arguments as SR_CopyCanvas, except it doesn't
 * malloc a new canvas - it returns essentially a segment of the original
 * canvas instead. This doesn't allocate any memory, but it relies on the
 * host canvas still existing in memory.
 * 
 * If you destroy the host canvas, the reference canvas becomes a dangling
 * pointer. If you destroy the reference canvas, the host canvas becomes
 * a dangling pointer. See the problem here?
 * 
 * The allow_destroy_host allows you to destroy the reference canvas, which
 * actually just destroys the host canvas, but will turn any references to
 * the host canvas into a dangling pointer, so be careful.
 */
SR_Canvas SR_RefCanv(
	SR_Canvas *src,
	U16 xclip,
	U16 yclip,
	U16 width,
	U16 height,
	U1 allow_destroy_host);

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

/* Scales the source canvas into the destination canvas. Bad things will
 * happen if the source and destination point to the same canvas.
 * The new width and height is the width and height of the destination
 * canvas.
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

/* Returns a canvas with the input canvas's content skewed
 * set mode for vertical shearing, else turn off for horizontal
 * 
 * Will malloc a new canvas!
 */
SR_OffsetCanvas SR_CanvasShear(
	SR_Canvas *src,
	I32 skew_amount,
	U1 mode);

/* Returns a canvas that is hecking rotated, hopefully.
 * Expects an angle in degrees - will rotate clockwise.
 *
 * Enable safety padding in order to prevent rotated images potentially
 * going off-scanvas.
 * 
 * When safety padding is enabled, offset_x and offset_y will be non-zero
 * (these are parameters of the SR_OffsetCanvas struct returned). Use
 * these offsets to properly determine where the rotated canvas should be
 * placed on-screen.
 * 
 * Will malloc a new canvas!
 * 
 * NEW: Autocrop option will automatically crop a padded rotated canvas
 * in order to remove extra space. Useful for feeding rotated canvases
 * into the rotation function.
 */
SR_OffsetCanvas SR_CanvasRotate(
	SR_Canvas *src,
	R32 degrees,
	U1 safety_padding,
	U1 autocrop);

/* Flip the target canvas - does not malloc, works in-place.
 * Enable vertical to flip vertically. Leave disabled for horizontal.
 */
X0 SR_InplaceFlip(SR_Canvas *src, U1 vertical);

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
#endif
