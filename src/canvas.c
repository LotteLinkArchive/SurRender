#include "glbl.h"
#include "canvas.h"
#include "colours.h"

X0 SR_FillModLUT(U16 moperand)
{
    if (modlut_complete[moperand]) goto sr_fmlutexit;
    modlut_complete[moperand] = true;
    for (U16 x = 0; x < (SR_MAX_CANVAS_SIZE + 1); x++)
        modlut[moperand][x] = x % moperand;
sr_fmlutexit:
    return;
}

X0 SR_GenCanvLUT(SR_Canvas *canvas)
{
    canvas->hflags  |= SR_CPow2FDtc(
        canvas->rwidth, canvas->rheight, 0x10);
    canvas->hflags  |= SR_CPow2FDtc(
        canvas->cwidth, canvas->cheight, 0x20);

    SR_FillModLUT(canvas->cwidth );
    SR_FillModLUT(canvas->cheight);
    SR_FillModLUT(canvas->rwidth );
    SR_FillModLUT(canvas->rheight);
}

U1 SR_ResizeCanvas(
    SR_Canvas *canvas,
    U16 width,
    U16 height)
{
    // It is impossible to create a 0-width/0-height canvas.
    if (!width  ||
        !height ||
        canvas->pixels ||
        canvas->hflags & 0x0B) return false;

    // @direct
    canvas->width = width;
    canvas->height = height;

    canvas->rwidth = width;
    canvas->rheight = height;

    canvas->cwidth = width;
    canvas->cheight = height;
    SR_GenCanvLUT(canvas);

    // Not strictly neccessary, but rodger put it here anyway, so whatever.
    canvas->ratio = (R32)width / height;

    /* FYI: We can actually use realloc here if we assume that canvas->pixels
     * contains either a valid allocation or none at all (represented by NULL)
     * That way, we can simplify the whole process, as realloc works just like
     * malloc does when you feed it a null pointer. Magic!
     */
    canvas->pixels = realloc(
        canvas->pixels,
        (U32)(
            (U32)width *
            (U32)height *
            sizeof(SR_RGBAPixel)));

    // Return the allocation state.
    return BOOLIFY(canvas->pixels);
}

X0 SR_TileTo(
    SR_Canvas *canvas,
    U16 width,
    U16 height)
{
    // @direct
    canvas->width = width;
    canvas->height = height;
}

X0 SR_ZeroFill(SR_Canvas *canvas)
{
    if (!canvas->pixels) return;

    if (canvas->xclip   != 0 ||
        canvas->yclip   != 0 ||
        canvas->cwidth  != canvas->rwidth  ||
        canvas->cheight != canvas->rheight ) {
        U16 x, y;
        for (x = 0; x < canvas->width; x++)
        for (y = 0; y < canvas->height; y++)
            SR_CanvasSetPixel(canvas, x, y, SR_CreateRGBA(0, 0, 0, 0));

        return;
    }

    // Fill the canvas with zeros, resulting in RGBA(0, 0, 0, 0).
    // To fill with something like RGBA(0, 0, 0, 255), see shapes.h.
    memset(canvas->pixels, 0, SR_CanvasCalcSize(canvas));
}

SR_Canvas SR_NewCanvas(U16 width, U16 height)
{
    SR_Canvas temp = {};
    temp.pixels = NULL; // Just for realloc's sake. Can NULL be non-zero?

    // As long as we set pixels to NULL, ResizeCanvas can be used here too.
    SR_ResizeCanvas(&temp, width, height);

    return temp;
}

// SR_DestroyCanvas is super important for any mallocated canvases. Use it.
X0 SR_DestroyCanvas(SR_Canvas *canvas)
{
	U1 hfstate = !(canvas->hflags & 0x02);

    if (canvas->pixels  && hfstate)
        free(canvas->pixels);

    canvas->pixels = NULL;
}

SR_Canvas SR_CopyCanvas(
    SR_Canvas *canvas,
    U16 copy_start_x,
    U16 copy_start_y,
    U16 new_width,
    U16 new_height)
{
    // Create the destination canvas
    SR_Canvas new = SR_NewCanvas(new_width, new_height);

    // If it isn't valid, just return the metadata and pray it doesn't get used
    if (!new.pixels) goto srcc_finish;

    if (copy_start_x == 0 &&
        copy_start_y == 0 &&
        new.width    == canvas->width  &&
        new.height   == canvas->height &&
        !canvas->xclip &&
        !canvas->yclip ) {
        // Super fast memcpy when possible, hopefully.
        memcpy(new.pixels, canvas->pixels, SR_CanvasCalcSize(&new));

        goto srcc_finish; // Just jump to finish here, saves ident level
    }

    // Slower copying, but not much slower - used for cropping/panning
    U16 x, y;
    for (x = 0; x < new.width; x++)
    for (y = 0; y < new.height; y++)
        SR_CanvasSetPixel(&new, x, y, SR_CanvasGetPixel(
            canvas, x + copy_start_x, y + copy_start_y));

srcc_finish:
    return new;
}

SR_Canvas SR_RefCanv(
    SR_Canvas *src,
    U16 xclip,
    U16 yclip,
    U16 width,
    U16 height,
    U1 allow_destroy_host)
{
    // @direct
    SR_Canvas temp = {
        .hflags  = 0x01,
        .width   = width,
        .height  = height,
        .ratio   = (R32)width / height,
        .pixels  = src->pixels,
        .rwidth  = src->rwidth,
        .rheight = src->rheight,
        .xclip   = xclip,
        .yclip   = yclip,
        .cwidth  = MIN(src->cwidth , width ),
        .cheight = MIN(src->cheight, height)
    };

    if (!allow_destroy_host) temp.hflags |= 0x02;

    SR_GenCanvLUT(&temp);

    return temp;
}

X0 SR_MergeCanvasIntoCanvas(
    SR_Canvas *dest_canvas,
    SR_Canvas *src_canvas,
    U16 paste_start_x,
    U16 paste_start_y,
    U8 alpha_modifier,
    I8 mode)
{
    U16 x, y;
    for (x = 0; x < src_canvas->width; x++)
    for (y = 0; y < src_canvas->height; y++) {
        // Uses the function for blending individual RGBA values.
        SR_CanvasSetPixel(
            dest_canvas,
            x + paste_start_x,
            y + paste_start_y,
            SR_RGBABlender(
                SR_CanvasGetPixel(
                    dest_canvas,
                    x + paste_start_x,
                    y + paste_start_y),
                SR_CanvasGetPixel(src_canvas, x, y),
                alpha_modifier,
                mode));
    }
}

// Private
#define getByte(value, n) (value >> (n * 8) & 0xFF)
#define lerp(s, e, t) ((s) + ((e) - (s)) * (t))
#define blerp(c00, c10, c01, c11, tx, ty) \
(lerp(lerp((c00), (c10), (tx)), lerp((c01), (c11), (tx)), (ty)))

// Private
X0 SR_BilinearCanvasScale(
    SR_Canvas *src,
    SR_Canvas *dest)
{
    if (!dest->pixels) return;

    U32 x, y;
    for (x = 0, y = 0; y < dest->height; x++) {
        if (x > dest->width) { x = 0; y++; }

        R32 gx = x / (R32)(dest->width ) * (SR_CanvasGetWidth (src) - 1);
        R32 gy = y / (R32)(dest->height) * (SR_CanvasGetHeight(src) - 1);
        I32 gxi = (int)gx;
        I32 gyi = (int)gy;

        // TODO: Clean this up, preferably stop using SR_RGBAtoWhole, it's slow

        U32 c00 = SR_CanvasGetPixel(src, gxi    , gyi    ).whole;
        U32 c10 = SR_CanvasGetPixel(src, gxi + 1, gyi    ).whole;
        U32 c01 = SR_CanvasGetPixel(src, gxi    , gyi + 1).whole;
        U32 c11 = SR_CanvasGetPixel(src, gxi + 1, gyi + 1).whole;

        U32 result = 0;
        for (U8 i = 0; i < 4; i++)
            result |= (U8)blerp(
                (R32)getByte(c00, i),
                (R32)getByte(c10, i),
                (R32)getByte(c01, i),
                (R32)getByte(c11, i),
                (R32)gx - gxi,
                (R32)gy - gyi
            ) << (8 * i);

        SR_RGBAPixel final = {
            .whole = result
        };

        SR_CanvasSetPixel(dest, x, y, final);
    }
}

// Private
X0 SR_NearestNeighborCanvasScale(
    SR_Canvas *src,
    SR_Canvas *dest)
{
    if (!dest->pixels) return;

    R32 x_factor = (R32)src->width  / (R32)dest->width;
    R32 y_factor = (R32)src->height / (R32)dest->height;
    
    U16 x, y;
    for (x = 0; x < dest->width; x++)
    for (y = 0; y < dest->height; y++) {
        SR_RGBAPixel sample = SR_CanvasGetPixel(
            src, x * x_factor, y * y_factor);
        SR_CanvasSetPixel(dest, x, y, sample);
    }
}

X0 SR_CanvasScale(
    SR_Canvas *src,
    SR_Canvas *dest,
    I8 mode)
{
    switch (mode) {
    default:
    case SR_SCALE_NEARESTN:
        SR_NearestNeighborCanvasScale(src, dest);
        
        break;
    case SR_SCALE_BILINEAR:
        SR_BilinearCanvasScale(src, dest);

        break;
    }
}

U16 * SR_NZBoundingBox(SR_Canvas *src)
{
    // TODO: Test this for bugs
    // TODO: Find some way to clean up the repetition here

    // Static declaration prevents a dangling pointer
    static U16 bbox[4] = {0, 0, 0, 0};
    U16 xC, yC, firstX, firstY, lastX, lastY, x, y;

    for (y = 0; y < src->height; y++)
    for (x = 0; x < src->width; x++)
        if (SR_CanvasPixelCNZ(src, x, y))
            { firstX = x, firstY = y; goto srnzbbx_first_pixel_done; }
    
    goto srnzbbx_empty; // No data found in image - commit die
srnzbbx_first_pixel_done: // Exit loop
    for (y = src->height - 1; y > 0; y--)
    for (x = src->width - 1; x > 0; x--)
        if (SR_CanvasPixelCNZ(src, x, y))
            { lastX = x, lastY = y; goto srnzbbx_last_pixel_done; }

    goto srnzbbx_empty;
srnzbbx_last_pixel_done:
    for (xC = 0; xC <= firstX; xC++)
    for (yC = firstY; yC <= lastY; yC++)
        if (SR_CanvasPixelCNZ(src, xC, yC)) {
            bbox[0] = xC; bbox[1] = firstY;
            goto srnzbbx_found_first;
        }

    goto srnzbbx_empty;
srnzbbx_found_first:
    for (xC = src->width - 1; xC > lastX; xC--)
    for (yC = lastY; yC >= firstY; yC--)
        if (SR_CanvasPixelCNZ(src, xC, yC)) {
            bbox[2] = xC; bbox[3] = lastY;
            goto srnzbbx_bounded;
        }

    goto srnzbbx_no_end_in_sight; // No last poI32 found - is this possible?
srnzbbx_no_end_in_sight:
    bbox[2] = src->width - 1; bbox[3] = src->height - 1;
srnzbbx_bounded:
    return bbox; // Return the box (er, I mean RETURN THE SLAB)
srnzbbx_empty:
    /* We can return a null pointer if we believe the canvas is empty, making
     * it easier to check the state of a canvas.
     */
    return NULL;
}

SR_OffsetCanvas SR_CanvasShear(
    SR_Canvas *src,
    I32 skew_amount,
    U1 mode)
{
    U16 w, h, mcenter;
    R32 skew;

    w = src->width;
    h = src->height;
    mcenter = mode ? w >> 1 : h >> 1;
    skew = (R32)skew_amount / (R32)mcenter;
    skew_amount = abs(skew_amount);

    SR_OffsetCanvas final;

    if (mode) final.canvas = SR_NewCanvas(w, h + (skew_amount << 1));
    else final.canvas = SR_NewCanvas(w + (skew_amount << 1), h);
    SR_ZeroFill(&(final.canvas));

    final.offset_x = mode ? 0 : -skew_amount;
    final.offset_y = mode ? -skew_amount : 0;

    I32 mshift;

    // TODO: Find some way to clean up the repetition here

    if (mode) {
        for (U16 x = 0; x < w; x++) {
            mshift = skew_amount + (x - mcenter) * skew;
            for (U16 y = 0; y < h; y++) {
                SR_RGBAPixel pixel = SR_CanvasGetPixel(src, x, y);
                SR_CanvasSetPixel(&(final.canvas), x, y + mshift, pixel);
            }
        }
	} else {
        for (U16 y = 0; y < h; y++) {
            mshift = skew_amount + (y - mcenter) * skew;
            for (U16 x = 0; x < w; x++) {
                SR_RGBAPixel pixel = SR_CanvasGetPixel(src, x, y);
                SR_CanvasSetPixel(&(final.canvas), x + mshift, y, pixel);
            }
        }
    }

    return final;
}

SR_OffsetCanvas SR_CanvasRotate(
    SR_Canvas *src,
    R32 degrees,
    U1 safety_padding,
    U1 autocrop)
{
    // Declare everything here
    SR_Canvas temp;
    U16 w, h, boundary, xC, yC, nx, ny;
    I32 x, y, nxM, nyM, half_w, half_h;
    R32 the_sin, the_cos;
    SR_RGBAPixel pixel, pixbuf;
    SR_OffsetCanvas final;

    // There's no poI32 in considering unique values above 359. 360 -> 0
    degrees = fmod(degrees, 360);

    // For simplicity's sake
    w = src->width;
    h = src->height;

    final.offset_x = 0;
    final.offset_y = 0;

    if (safety_padding) {
        // Create additional padding in case rotated data goes off-canvas
        boundary = MAX(w, h) << 1; // Double the largest side length
        final.canvas = SR_NewCanvas(boundary, boundary);
        final.offset_x = -(int)(boundary >> 2);
        final.offset_y = -(int)(boundary >> 2);
    } else {
        final.canvas = SR_NewCanvas(w, h);
    }
    // Prevent garbage data seeping in
    SR_ZeroFill(&final.canvas);

    // Rotation not 0, 90, 180 or 270 degrees? Use inaccurate method instead
    if (fmod(degrees, 90) != .0) goto srcvrot_mismatch;

    // Trying to rotate 0 degrees? Just copy the canvas, I guess.
    if (!((U16)degrees % 360)) {
        SR_MergeCanvasIntoCanvas(
            &final.canvas,
            src,
            -final.offset_x, // Still need to use the offset incase padding
            -final.offset_y,
            255,
            SR_BLEND_REPLACE); // Fastest and safest, also no background alpha

        goto srcvrot_finished; // Jump to the finishing/cleanup line
    }

    /* TODO: Mismatching width and height causes accurate rotation bug.
     * We should probably fix this, but for now it's fine to use inaccurate
     * rotation.
     */
    if (w != h) goto srcvrot_mismatch;

    // This is the accurate rotation system, but it only works on degrees
    // where x % 90 == 0
    for (xC = 0; xC < w; xC++)
    for (yC = 0; yC < h; yC++) {
        pixbuf = SR_CanvasGetPixel(src, xC, yC);
        nx = 0, ny = 0;
        switch (((U16)degrees) % 360) {
        case 90:
            nx = (h - 1) - yC;
            ny = xC;
            break;
        case 180:
            nx = (w - 1) - xC;
            ny = (h - 1) - yC;
            break;
        case 270:
            nx = yC;
            ny = (w - 1) - xC;
            break;
        }

        SR_CanvasSetPixel(
            &final.canvas,
            nx - final.offset_x, // Correct for offset
            ny - final.offset_y,
            pixbuf);
    }

    goto srcvrot_finished;

srcvrot_mismatch:
    // Convert to radians and then modulo by 2*pi
    degrees = fmod(degrees * 0.017453292519943295, 6.28318530718);

    the_sin = -sin(degrees);
    the_cos = cos(degrees);
    half_w = w >> 1;
    half_h = h >> 1;

    for (x = -half_w; x < half_w; x++)
    for (y = -half_h; y < half_h; y++) {
        nxM = (x * the_cos + y * the_sin + half_w) - final.offset_x;
        nyM = (y * the_cos - x * the_sin + half_h) - final.offset_y;
        pixel = SR_CanvasGetPixel(src, x + half_w, y + half_h);

        // Set target AND a single nearby pixel to de-alias
        SR_CanvasSetPixel(&final.canvas, nxM    , nyM    , pixel);
        SR_CanvasSetPixel(&final.canvas, nxM - 1, nyM    , pixel);
    }

srcvrot_finished:
    if (safety_padding && autocrop) {
        // If autocropping is enabled, auto-crop padded images. This is slow,
        // but speeds up merging a fair bit. Use if you only intend to rotate
        // once.
        U16 * bbox = SR_NZBoundingBox(&final.canvas);
        if (bbox) {
            temp = SR_RefCanv(
                &final.canvas,
                bbox[0],
                bbox[1],
                (bbox[2] - bbox[0]) + 1,
                (bbox[3] - bbox[1]) + 1,
                true);

            final.canvas = temp;

            final.offset_x += bbox[0];
            final.offset_y += bbox[1];
        }
    }

    return final;
}

X0 SR_InplaceFlip(SR_Canvas *src, U1 vertical)
{
    // Flipping canvases honestly doesn't need a new canvas to be allocated,
    // so we can do it in-place just fine for extra speed and less memory
    // usage.
    U16 x, y, wmax, hmax, xdest, ydest;
    SR_RGBAPixel temp, pixel;

    wmax = vertical ? src->width : src->width >> 1;
    hmax = vertical ? src->height >> 1 : src->height;

    for (x = 0; x < wmax; x++)
    for (y = 0; y < hmax; y++) {
        xdest = vertical ? x : (src->width  - 1) - x;
        ydest = vertical ? (src->height - 1) - y : y;

        temp  = SR_CanvasGetPixel(src, xdest, ydest);
        pixel = SR_CanvasGetPixel(src, x, y);
        SR_CanvasSetPixel(src, xdest, ydest, pixel);
        SR_CanvasSetPixel(src, x, y, temp);
    }
}

SR_Canvas SR_RefCanvTile(
    SR_Canvas *atlas,
    U16 tile_w,
    U16 tile_h,
    U16 col,
    U16 row)
{
    U16 columns, rows;
    columns = ceilf((R32)atlas->width / tile_w);
    rows = ceilf((R32)atlas->height / tile_h);

    return SR_RefCanv(atlas,
        (col % columns) * tile_w,
        (row % rows   ) * tile_h,
        tile_w, tile_h, false);
}
