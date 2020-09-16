#include "textrender.h"

SR_FontAtlas SR_MakeFontAtlas(
    SR_Canvas *font,
    unsigned short charwidth,
    unsigned short charheight)
{
    SR_FontAtlas temp = {
        .font       = font,
        .charwidth  = charwidth,
        .charheight = charheight
    };

    return temp;
}