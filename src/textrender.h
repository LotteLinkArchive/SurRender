#ifndef SURTR_HEADER_FILE
#define SURTR_HEADER_FILE
    #include "glbl.h"
    #include "canvas.h"
    #include "colours.h"

    /* The font renderer module renders UCS-2 formatted text into a canvas.
     * It requires a font canvas, which arranges characters into a 256x256
     * grid. The X position (0-255) corresponds to the first byte of each
     * character, and the Y position (0-255) corresponds to the last byte
     * of each character. Use little-endian UCS-2.
     */

    typedef struct SR_FontAtlas {
        /* The font canvas must be a valid canvas (check that beforehand).
         * Charwidth and charheight specifies the width and height of each
         * character on the canvas. The font canvas must be a 256x256 grid of
         * characters. For a 5x10 font, the resultant font canvas should be up
         * to 1280x2560.
         * 
         * ALL RGB VALUES MUST BE ZERO IN A FONT ATLAS. ONLY THE ALPHA VALUES
         * MUST VARY.
         * 
         * THE FONT CANVAS WILL BE MODIFIED!
         * 
         * FONT ATLASES ARE NOT THREADSAFE (IF ATTEMPTING TO RENDER WITH THE
         * SAME CANVAS/ATLAS SIMULTANEOUSLY)
         */
        SR_Canvas *font;
        unsigned short charwidth;
        unsigned short charheight;

        /* The colour to use when drawing each character */
        SR_RGBAPixel colour;

        /* The "new font size" and how it should be scaled, see canvas.h */
        unsigned short rescalewidth;
        unsigned short rescaleheight;
        unsigned char  rescalemode;

        /* The hpadding (between characters) and vpadding (between lines) */
        unsigned short hpadding;
        unsigned short vpadding;
        unsigned short tabspaces;
    } SR_FontAtlas;

    // Create a font atlas with default values for a given font map.
    SR_FontAtlas SR_MakeFontAtlas(
        SR_Canvas *font,
        unsigned short charwidth,
        unsigned short charheight);

    /* Draw text on a destination canvas using a given font object.
     * Text must be in UCS-2 format. If length is incorrect, expect undefined
     * behvaiour. X and Y represent the position to draw the text at.
     * Breakpoint is the amount of horizontal pixels that each line can consume
     * before it will automatically insert a line break. Set to 0 to turn this
     * off completely.
     * 
     * NOTE: `length` is NOT MEASURED IN BYTES, it is measured in CHARACTERS.
     * For example, a 64 byte string is 32 16-bit characters, thus a length of
     * 32 characters.
     */
    void SR_PrintToAtlas(
        SR_FontAtlas *font,
        SR_Canvas *dest,
        uint16_t *text,
        size_t length,
        unsigned short x,
        unsigned short y,
        unsigned short breakpoint,
        unsigned char blendmode);
#endif
