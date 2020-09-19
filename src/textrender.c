#include "textrender.h"

SR_FontAtlas SR_MakeFontAtlas(
    SR_Canvas *font,
    U16 charwidth,
    U16 charheight)
{
    SR_FontAtlas temp = {
        .font          = font,
        .charwidth     = charwidth,
        .charheight    = charheight,
        .colour        = SR_CreateRGBA(255, 255, 255, 255),
        .rescalewidth  = charwidth,
        .rescaleheight = charheight,
        .rescalemode   = SR_SCALE_NEARESTN,
        .hpadding      = 2,
        .vpadding      = 2,
        .tabspaces     = 4
    };

    return temp;
}

void SR_PrintToAtlas(
    SR_FontAtlas *font,
    SR_Canvas *dest,
    U16 *text,
    size_t length,
    U16 x,
    U16 y,
    U16 breakpoint,
    U8 blendmode)
{
    U16 rootx = x;

    while (--length) {
        U16 ucsc = *text++;

        if (ucsc == 0x0009) x += (font->rescalewidth * font->tabspaces) +
            (font->hpadding * font->tabspaces);

        if (ucsc == 0x000a ||
            (breakpoint != 0 &&
                ((x - rootx) > (breakpoint - font->rescalewidth)))) {
            y += font->rescaleheight + font->vpadding;
            x =  rootx;
        } 

        if (ucsc == 0x000a || ucsc == 0x0009) continue;
        if (ucsc == 0x0000) break;

        SR_Canvas character = SR_RefCanvTile(
            font->font,
            font->charwidth,
            font->charheight,
            ucsc & 255,
            ucsc >> 8);
            
        if ((SR_CanvasGetPixel(&character, 0, 0).whole & 0x00FFFFFF) !=
            (font->colour.whole & 0x00FFFFFF)) {
            register U16 xc, yc;
            for (xc = 0; xc < character.width; xc++)
            for (yc = 0; yc < character.height; yc++) {
                SR_CanvasSetPixel(&character, xc, yc, SR_RGBABlender(
                    SR_CanvasGetPixel(&character, xc, yc),
                    font->colour, 255, SR_BLEND_ADDITIVE_PAINT));
            }
        }

        if ((font->rescalewidth  != font->charwidth) ||
            (font->rescaleheight != font->charheight)) {
            SR_Canvas temp = SR_NewCanvas(
                font->rescalewidth, font->rescaleheight);

            if (!SR_CanvasIsValid(&temp)) break;

            character = SR_CopyCanvas(
                &character, 0, 0, font->charwidth, font->charheight);

            if (!SR_CanvasIsValid(&character)) break;

            SR_CanvasScale(&character, &temp, font->rescalemode);
            SR_DestroyCanvas(&character);

            character = temp;
        }

        SR_MergeCanvasIntoCanvas(
            dest, &character, x, y, font->colour.chn.alpha, blendmode);

        SR_DestroyCanvas(&character);

        x += font->rescalewidth + font->hpadding;
    }
}