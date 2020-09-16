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
        SR_Canvas *font; /* Must be a valid canvas */
        unsigned short charwidth; /* Used as tile width/height */
        unsigned short charheight;
        SR_RGBAPixel colour;
        unsigned short rescalewidth;
        unsigned short rescaleheight;
        unsigned char  rescalemode;
        unsigned short hpadding;
        unsigned short vpadding;
    } SR_FontAtlas;

    SR_FontAtlas SR_MakeFontAtlas(
        SR_Canvas *font,
        unsigned short charwidth,
        unsigned short charheight);

    void SR_PrintToAtlas(
        SR_FontAtlas *font,
        SR_Canvas *dest,
        uint16_t *text,
        size_t length,
        unsigned short x,
        unsigned short y,
        unsigned short breakpoint);
#endif
