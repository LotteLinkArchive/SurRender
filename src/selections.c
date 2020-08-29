#include "glbl.h"
#include "canvas.h"
#include "colours.h"
#include "selections.h"

SR_Select SR_NewSelect(unsigned short width, unsigned short height)
{
    SR_Select temp = {
        .width = width,
        .height = height,
        .bitfield = calloc((width * height) >> 3, sizeof(uint8_t))
    };
    
    return temp;
}

void SR_DestroySelect(SR_Select *selection)
{
    if (!selection->bitfield) return;
    free(selection->bitfield);
    selection->bitfield = NULL;
}

void SR_EatBreakfast(
    SR_Canvas *canvas,
    SR_Select *selection,
    unsigned short offset_x,
    unsigned short offset_y)
{
    unsigned short max_x = MIN(canvas->width, selection->width + offset_x);
    unsigned short max_y = MIN(canvas->height, selection->height + offset_y);
    unsigned short x, y;
    for (x = offset_x; x < max_x; x++)
    for (y = offset_y; y < max_y; y++) {
        if (SR_SelectGetPoint(selection, x, y) {
            SR_RGBAPixel rand_col = SR_CreateRGBA(rand(), rand(), rand(), 255);
            SR_CanvasSetPixel(canvas, x, y, rand_col);
        }
    }
}
