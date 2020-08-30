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

void SR_SelectLine(
    SR_Select *selection, char mode,
    int x0, int y0,
    int x1, int y1)
{
    int dx, dy, err, sx, sy, e2;
    dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
    dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
    err = dx + dy;

    for (;;) {
        SR_SelectSetPoint(selection, x0, y0, mode);

        if (x0 == x1 && y0 == y1) break;

        e2 = err << 1;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}
