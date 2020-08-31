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

void SR_SelectTri(
    SR_Select *selection, char mode,
    int x0, int y0, 
    int x1, int y1,
    int x2, int y2)
{
	unsigned short  min_x, min_y, max_x, max_y;
    min_x = MIN(x0, MIN(x1, x2));
	min_y = MIN(y0, MIN(y1, y2));
	max_x = MAX(x0, MAX(x1, x2));
	max_y = MAX(y0, MAX(y1, y2));
    
    int v0x, v0y, v1x, v1y;
    v0x = x1 - x0;
    v0y = y1 - y0;
    v1x = x2 - x0;
    v1y = y2 - y0;
    
    float vcross = v0x * v1y - v0y * v1x;
    
    for (unsigned short x = min_x; x <= max_x; x++)
    for (unsigned short y = min_y; y <= max_y; y++) {
        int vqx = x - x0;
        int vqy = y - y0;
        
        float s = (float)(vqx * v1y - vqy * v1x);
        s /= vcross;
        float t = (float)(v0x * vqy - v0y * vqx);
        t /= vcross;
        
        if ((s >= 0) && (t >= 0) && (s + t <= 1))
            SR_SelectSetPoint(selection, x, y, mode);
    }
}

void SR_SelectRect(
    SR_Select *selection, char mode,
    int x, int y,
    int w, int h)
{
    int max_x = MIN(x + w, selection->width - 1);
    int max_y = MIN(y + h, selection->height - 1);
    
    unsigned short xi, yi;
    for (xi = x; xi < max_x; xi++)
    for (yi = y; yi < max_y; yi++) {
        SR_SelectSetPoint(selection, x, y, mode);
    }
}
void SR_SelectCirc(
    SR_Select *selection, char mode,
    int x, int y,
    unsigned long r)
{
    unsigned short min_x, min_y, max_x, max_y;
    min_x = MAX(0, x - r);
    min_y = MAX(0, y - r);
    max_x = MIN(selection->width - 1, x + r);
    max_y = MIN(selection->height - 1, y + r);
    r *= r;
    
    unsigned short xi, yi;
    for (xi = min_x; xi <= max_x; xi++)
    for (yi = min_y; yi <= max_y; yi++) {
        int xp, yp;
        xp = xi - x;
        xp *= xp;
        yp = yi - y;
        yp *= yp;

        if (xp + yp <= r)
            SR_SelectSetPoint(selection, xi, yi, mode);
    }
}
