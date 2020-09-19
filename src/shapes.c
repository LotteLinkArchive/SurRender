#include "glbl.h"
#include "canvas.h"
#include "colours.h"
#include "shapes.h"

//copy pasted from:
//https://gist.github.com/bert/1085538#file-plot_line-c-L9
X0 SR_DrawLine(
    SR_Canvas *canvas,
    SR_RGBAPixel colour,
    I32 x0, I32 y0, 
    I32 x1, I32 y1)
{
    I32 dx, dy, err, sx, sy, e2;
    dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
    dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
    err = dx + dy;

    for (;;) {
        SR_CanvasSetPixel(canvas, x0, y0, colour);

        if (x0 == x1 && y0 == y1) break;

        e2 = err << 1;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

X0 SR_DrawTriOutline(
    SR_Canvas *canvas,
    SR_RGBAPixel colour,
    I32 x0, I32 y0,
    I32 x1, I32 y1,
    I32 x2, I32 y2)
{
    SR_DrawLine(canvas, colour, x0, y0, x1, y1);
    SR_DrawLine(canvas, colour, x1, y1, x2, y2);
    SR_DrawLine(canvas, colour, x2, y2, x0, y0);
}

X0 SR_DrawTri(
    SR_Canvas *canvas,
    SR_RGBAPixel colour,
    I32 x0, I32 y0,
    I32 x1, I32 y1,
    I32 x2, I32 y2)
{
	U16  min_x, min_y, max_x, max_y;
    min_x = MIN(x0, MIN(x1, x2));
	min_y = MIN(y0, MIN(y1, y2));
	max_x = MAX(x0, MAX(x1, x2));
	max_y = MAX(y0, MAX(y1, y2));
    
    I32 v0x, v0y, v1x, v1y;
    v0x = x1 - x0;
    v0y = y1 - y0;
    v1x = x2 - x0;
    v1y = y2 - y0;
    
    R32 vcross = v0x * v1y - v0y * v1x;
    
    for (U16 x = min_x; x <= max_x; x++)
        for (U16 y = min_y; y <= max_y; y++) {
            I32 vqx = x - x0;
            I32 vqy = y - y0;
            
            R32 s = (R32)(vqx * v1y - vqy * v1x);
            s /= vcross;
            R32 t = (R32)(v0x * vqy - v0y * vqx);
            t /= vcross;
            
            if ((s >= 0) && (t >= 0) && (s + t <= 1))
				SR_CanvasSetPixel(canvas, x, y, colour);
        }
}

X0 SR_DrawRectOutline(
    SR_Canvas *canvas,
    SR_RGBAPixel colour,
    U16 x,
    U16 y,
    U16 w,
    U16 h)
{
    w += x;
    h += y;
    U16 x1 = MIN(canvas->width - 1, w);
    U16 y1 = MIN(canvas->height - 1, h);
    
    for (U16 xi = x; xi < x1; xi++) {
        SR_CanvasSetPixel(canvas, xi, y, colour);
        SR_CanvasSetPixel(canvas, xi, h, colour);
    }
    
    for (U16 yi = y + 1; yi < y1; yi++) {
        SR_CanvasSetPixel(canvas, x, yi, colour);
        SR_CanvasSetPixel(canvas, w, yi, colour);
    }
}

X0 SR_DrawRect(
    SR_Canvas *canvas,
    SR_RGBAPixel colour,
    U16 x,
    U16 y,
    U16 w,
    U16 h)
{
    U16 x1 = MIN(canvas->width - 1, w + x);
    U16 y1 = MIN(canvas->height - 1, h + y);

    for (U16 yi = y; yi < y1; yi++)
        for (U16 xi = x; xi < x1; xi++)
            SR_CanvasSetPixel(canvas, xi, yi, colour);
}

X0 SR_DrawCircOutline(
    SR_Canvas *canvas,
    SR_RGBAPixel colour,
    U16 x,
    U16 y,
    U16 r)
{
    I32 xs = r;
    I32 ys = 0;
    I32 er = 0;
    
    while (xs >= ys) {
        SR_CanvasSetPixel(canvas, x + xs, y + ys, colour);
        SR_CanvasSetPixel(canvas, x + ys, y + xs, colour);
        SR_CanvasSetPixel(canvas, x - ys, y + xs, colour);
        SR_CanvasSetPixel(canvas, x - xs, y + ys, colour);
        SR_CanvasSetPixel(canvas, x - xs, y - ys, colour);
        SR_CanvasSetPixel(canvas, x - ys, y - xs, colour);
        SR_CanvasSetPixel(canvas, x + ys, y - xs, colour);
        SR_CanvasSetPixel(canvas, x + xs, y - ys, colour);
        
        if (er <= 0) { ys++; er += (ys << 1) + 1; }
        if (er >  0) { xs--; er -= (xs << 1) + 1; }
    }
}

X0 SR_DrawCirc(
    SR_Canvas *canvas,
    SR_RGBAPixel colour,
    I32 x,
    I32 y,
    U32 r)
{
    U16 min_x, min_y, max_x, max_y;
    min_x = MAX(0, x - r);
    min_y = MAX(0, y - r);
    max_x = MIN(canvas->width - 1, x + r);
    max_y = MIN(canvas->height - 1, y + r);
    r *= r;
    
    for (U16 xx = min_x; xx <= max_x; xx++)
        for (U16 yy = min_y; yy <= max_y; yy++) {
            I32 xp, yp;
            xp = xx - x;
            xp *= xp;
            yp = yy - y;
            yp *= yp;

            if (xp + yp <= r) SR_CanvasSetPixel(canvas, xx, yy, colour);
        }
}
