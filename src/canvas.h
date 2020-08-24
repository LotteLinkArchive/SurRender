#ifndef SURCV_HEADER_FILE
#define SURCV_HEADER_FILE
    #include "glbl.h"
    #include "colours.h"

    // Force little endian, hopefully
    #pragma scalar_storage_order little-endian

    /* This is a canvas, which contains a width and height in pixels, an aspect
     * ratio and a pointer to an array of pixel values.
     */
    typedef struct SR_Canvas {
        // General public properties
        unsigned short width;
        unsigned short height;
        float ratio;

        // Pointer to an array of pixels
        SR_RGBAPixel *pixels;
        
        // Coordinates to clip off, starting from 0, 0 (allow full canvas)
        unsigned short xclip;
        unsigned short yclip;

        /* Internal canvas properties - FORMAT:
         * 0 b 0 0 0 0 0 0 0 0
         *     X X X X | | | \- Canvas is a reference to another canvas' pixels
         *             | | \--- Canvas is indestructible
         *             | \----- Canvas is important             [UNIMPLEMENTED]
         *             \------- Canvas is a memory-mapped file  [UNIMPLEMENTED]
         */
        uint8_t hflags;
    } SR_Canvas;

    /* This is a texture atlas, essentially an array of 2D canvases. 
     * See the functions below to learn how to create one for yourself.
     */
    typedef struct SR_Atlas {
        SR_Canvas *src;
        unsigned char columns;
        unsigned char rows;
        unsigned short tile_width;
        unsigned short tile_height;
        SR_Canvas *canvies;
    } SR_Atlas;

    /* An SR_OffsetCanvas is just a regular canvas, but with additional offset
     * data which you will probably need in order to "place" the canvas
     * correctly using merge functions. Essentially, take the coordinates you
     * originally intended to paste the canvas at, and apply the offset values
     * to those coordinates.
     */
    typedef struct SR_OffsetCanvas {
        int offset_x;
        int offset_y;
        SR_Canvas canvas;
    } SR_OffsetCanvas;

    // All supported scaling modes for canvases
    enum SR_ScaleModes {
        SR_SCALE_BILINEAR
    };

    /* Make a canvas larger or smaller. Preserves the contents, but not
     * accurately. May ruin the current contents of the canvas.
     */
    bool SR_ResizeCanvas(
        SR_Canvas *canvas,
        unsigned short width,
        unsigned short height);

    /* A canvas may contain garbage data when initially created. This will
     * zero fill it for you, if needed.
     */
    void SR_ZeroFill(SR_Canvas *canvas);

    // Create a new canvas of the given size
    SR_Canvas SR_NewCanvas(unsigned short width, unsigned short height);

    // Get the width of a canvas
    unsigned short SR_CanvasGetWidth(SR_Canvas *canvas);

    // Get the height of a canvas
    unsigned short SR_CanvasGetHeight(SR_Canvas *canvas);

    /* Calculate the "real" size (in memory) of a canvas - not really
     * recommended to use this yourself.
     */
    #define SR_CanvasCalcSize(canvas) ((unsigned int)( \
        (unsigned int)((canvas)->width)  *             \
        (unsigned int)((canvas)->height) *             \
        sizeof(SR_RGBAPixel)                           \
    ))

    /* Calculate the "real" position of a pixel in the canvas - not really
     * recommended to use this yourself.
     */
    #define SR_CanvasCalcPosition(canvas, x, y) \
    ((((unsigned int)((canvas)->width)) *       \
    (((canvas)->yclip) + (y))) + (((canvas)->xclip) + (x)))

    // Check if a pixel is out of bounds
    #define SR_CanvasCheckOutOfBounds(canvas, x, y)   \
    (((((canvas)->xclip) + (x)) >= (canvas)->width || \
    (((canvas)->yclip) + (y)) >= (canvas)->height) ? true : false)

    // Set the value of a pixel in the canvas
    inline __attribute__((always_inline)) void SR_CanvasSetPixel(
        register SR_Canvas *canvas,
        register unsigned short x,
        register unsigned short y,
        SR_RGBAPixel pixel)
    {
        if (!canvas->pixels) return;

        canvas->pixels[SR_CanvasCalcPosition(
            canvas, x % canvas->width, y % canvas->height)] = pixel;
    }

    // Get a pixel in the canvas
    inline __attribute__((always_inline)) SR_RGBAPixel SR_CanvasGetPixel(
        SR_Canvas *canvas,
        register unsigned short x,
        register unsigned short y)
    {
        if (!canvas->pixels) { return SR_CreateRGBA(255, 0, 0, 255); }

        return canvas->pixels[SR_CanvasCalcPosition(
            canvas, x % canvas->width, y % canvas->height)];
    }

    // Check if a pixel is non-zero, hopefully
    #define SR_CanvasPixelCNZ(canvas, x, y) \
    (SR_RGBAtoWhole(SR_CanvasGetPixel((canvas), (x), (y))) != 0)

    /* Destroy the in-memory representation of the canvas
     * (Must create a new canvas or resize the current one in order to access)
     */
    void SR_DestroyCanvas(SR_Canvas *canvas);

    // Check if the canvas has been successfully allocated
    bool SR_CanvasIsValid(SR_Canvas *canvas);

    /* Malloc a new canvas of given size and start copying every pixel from the
     * specified old canvas to the new one, starting at the given position.
     * This also allows you to create cropped versions of a canvas! :)
     * Note: This does not destroy the old canvas. If you don't need it anymore
     * don't forget to destroy it, or it will remain allocated.
     */
    SR_Canvas SR_CopyCanvas(
        register SR_Canvas *canvas,
        register unsigned short copy_start_x,
        register unsigned short copy_start_y,
        unsigned short new_width,
        unsigned short new_height);

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
        unsigned short xclip,
        unsigned short yclip,
        unsigned short width,
        unsigned short height,
        bool allow_destroy_host);

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
    void SR_MergeCanvasIntoCanvas(
        register SR_Canvas *dest_canvas,
        register SR_Canvas *src_canvas,
        register unsigned short paste_start_x,
        register unsigned short paste_start_y,
        register uint8_t alpha_modifier,
        register char mode);

    /* Return a scaled up version of a canvas using SR_ScaleModes for the mode.
     * Will malloc a new canvas! You will need to destroy the old one if
     * needed.
     */
    SR_Canvas SR_CanvasScale(
        SR_Canvas *src,
        unsigned short newWidth,
        unsigned short newHeight,
        char mode);
    
    /* Returns a pointer to a static array containing 4 unsigned shorts.
     * The first 2 values are the x, y coordinates of the top left of the
     * bounding box. The last 2 values are the x, y coordinates of the bottom
     * right of the bounding box.
     * 
     * If the canvas is empty, returns a null pointer.
     * 
     * The bounding box is created when a non-zero value is found.
     * 
     * Note that this is a particularly slow operation.
     */
    unsigned short * SR_NZBoundingBox(SR_Canvas *src);

    /* Returns a canvas with the input canvas's content skewed
     * set mode for vertical shearing, else turn off for horizontal
     * 
     * Will malloc a new canvas!
     */
    SR_OffsetCanvas SR_CanvasShear(
        SR_Canvas *src,
        int skew_amount,
        bool mode);
    
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
        float degrees,
        bool safety_padding,
        bool autocrop);

    /* Flip the target canvas - does not malloc, works in-place.
     * Enable vertical to flip vertically. Leave disabled for horizontal.
     */
    void SR_InplaceFlip(SR_Canvas *src, bool vertical);

    /* Convert a source canvas (e.g an image loaded from disk) to a texture
     * atlas of a given tile width/height.
     * 
     * Mallocs a new atlas, but **absorbs** the source canvas. In other words,
     * DO NOT DESTROY THE SOURCE CANVAS. Call SR_DestroyAtlas on the ATLAS
     * intead, which will destroy the source canvas for you in a controlled
     * environment. Ya feel me?
     */
    SR_Atlas SR_CanvToAltas(
        SR_Canvas *src,
        unsigned short tile_w,
        unsigned short tile_h);

    /* Return a canvas stored at the given column and row in the texture atlas.
     * Col/row is modulo'd by column count and row count, so that's why you can
     * never end up with an out of bounds texture.
     */
    SR_Canvas * SR_GetAtlasCanv(
        SR_Atlas *atlas,
        unsigned char col,
        unsigned char row);

    /* Destroy the texture atlas, including all of the textures stored within
     * it (because it destroys the source canvas unless told to do otherwise)
     */
    void SR_DestroyAtlas(SR_Atlas *atlas, bool keep_source);

    // Abstraction macros, if you really want to use them
    #define SR_GetAtlasRows(atlas) (atlas->rows)
    #define SR_GetAtlasColumns(atlas) (atlas->columns)
#endif
