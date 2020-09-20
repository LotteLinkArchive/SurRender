#include <SDL2/SDL.h>
#include "../src/surrender.h"
#include <complex.h>
#include <time.h>
#include <stdlib.h>

/* This is the code we use to test if things are working. It may look kinda
 * ugly. Please excuse the mess, we change this literally all the time and why
 * bother keeping it pretty if you're going to end up destroying the whole
 * thing at random?
 */

#if defined(__i386__)

static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

#elif defined(__x86_64__)

static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtscp" : "=a"(lo), "=d"(hi) :: "%ecx");
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

#endif

int main(int argc, char *argv[])
{
    SR_Canvas canvy;
    SDL_Window *win;
    SDL_Event ev;
    int status;

    float aspect_ratio;

    /* needed for blitting */
    SDL_Surface *wsurf, *canvysurf;
    SDL_Rect destrect;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    /* edit: this is the current way of testing whether
       canvas allocation has failed or not for now */
    canvy = SR_NewCanvas(640, 480);

    SR_Canvas afont = SR_ImageFileToCanvas("./demo/images/AFONT.PNG");

    if (!(win = SDL_CreateWindow(
        "SurRender your mysteries to zoidberg!",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        canvy.width,
        canvy.height,
        SDL_WINDOW_RESIZABLE))) {
        status = 2;
        goto sdl_quit;
    }

    if (!SR_CanvasIsValid(&canvy) || !SR_CanvasIsValid(&afont)) {
        status = 3;
        goto sdl_destroywin;
    }

    SR_DrawRect(
        &canvy, SR_CreateRGBA(255, 255, 255, 255), 0, 0,
        canvy.width, canvy.height);
    SR_FontAtlas afonta = SR_MakeFontAtlas(&afont, 5, 10);

#ifdef FLAG_PIX_TIX
    unsigned int times = 65535;
    unsigned long long i = rdtsc();
    while (--times)
        SR_CanvasSetPixel(&canvy, times, times, SR_CreateRGBA(0, 0, 0, 255));
    printf("Set Pixel Ticks: %llu\n", (rdtsc() - i) >> 16);
#endif

    /* Define variables for test here */
#ifdef FLAG_DOKI
    SR_Canvas ball = SR_ImageFileToCanvas("./demo/images/TILEROTTEX.BMP");
    SR_Canvas logo = SR_ImageFileToCanvas("./demo/images/DDLC.BMP");
    SR_Canvas monkas = SR_ImageFileToCanvas("./demo/images/MENU_HELL.BMP");
#endif

#if defined(FLAG_PUCK) || defined(FLAG_SQUISH)
	float speeen = 0.0;
#endif

#ifdef FLAG_PUCK
    SR_Canvas imagetest = SR_ImageFileToCanvas("./demo/images/PUCK.BMP");
    SR_OffsetCanvas rotcanvas;
#endif
    
#ifdef FLAG_ROT
    //look i'm not sure the 90 deg rots are hecking properly
    //yes i could theoretically just rotate the same tempbox
    //canvas three times but testing so HUIWNHRVYIh opsaiejhgr eas0ry
    SR_Canvas boxes = SR_NewCanvas(128, 128);
    SR_ZeroFill(&boxes);
    char filename[] = "TEST_SM.BMP";
    SR_Canvas tempbox0 = SR_ImageFileToCanvas(filename);
    SR_Canvas tempbox1 = SR_ImageFileToCanvas(filename);
    tempbox1 = SR_CanvasRotate(&tempbox1, 90, 0, 0).canvas;
    SR_Canvas tempbox2 = SR_ImageFileToCanvas(filename);
    tempbox2 = SR_CanvasRotate(&tempbox2, 180, 0, 0).canvas;
    SR_Canvas tempbox3 = SR_ImageFileToCanvas(filename);
    tempbox3 = SR_CanvasRotate(&tempbox3, 270, 0, 0).canvas;
    SR_MergeCanvasIntoCanvas(&boxes, &tempbox0,  2,  2, 255, SR_BLEND_REPLACE);
    SR_MergeCanvasIntoCanvas(&boxes, &tempbox1, 42,  2, 255, SR_BLEND_REPLACE);
    SR_MergeCanvasIntoCanvas(&boxes, &tempbox2, 82,  2, 255, SR_BLEND_REPLACE);
    SR_MergeCanvasIntoCanvas(&boxes, &tempbox3,  2, 42, 255, SR_BLEND_REPLACE);
    SR_DestroyCanvas(&tempbox0);
    SR_DestroyCanvas(&tempbox1);
    SR_DestroyCanvas(&tempbox2);
    SR_DestroyCanvas(&tempbox3);
#endif

#ifdef FLAG_SQUISH
    SR_Canvas pokesquish = SR_ImageFileToCanvas("./demo/images/GOODRA.BMP");
    SR_OffsetCanvas squish;
#endif

#ifdef FLAG_ATLAS
    SR_Canvas brick_tileset = SR_ImageFileToCanvas("./demo/images/BRICKS.BMP");
    SR_Canvas brick_tileset_res = SR_NewCanvas(192, 192);
    SR_CanvasScale(
        &brick_tileset, &brick_tileset_res, SR_SCALE_NEARESTN);
    SR_DestroyCanvas(&brick_tileset);
    brick_tileset = brick_tileset_res;
#endif

    if (!(canvysurf = SDL_CreateRGBSurfaceFrom(
        canvy.pixels,
        canvy.width,
        canvy.height,
        32,
        canvy.width * sizeof(SR_RGBAPixel),
        0x000000FF,
        0x0000FF00,
        0x00FF0000,
        0xFF000000))) {
        status = 4;
        goto sr_destroycanvas;
    }

    /* this rectangle will be recomputed
       every time the window is resized */
    destrect.x = 0, destrect.y = 0,
    destrect.w = canvy.width, destrect.h = canvy.height;

    if (!(wsurf = SDL_GetWindowSurface(win))) {
        status = 5;
        goto sdl_freesurf;
    }

event_loop:
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            status = 0;
            goto sdl_freesurf;
        }

        if (ev.type == SDL_WINDOWEVENT)
        if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            if (!(wsurf = SDL_GetWindowSurface(win))) {
                status = 5;
                goto sdl_freesurf;
            }

            if ((float)ev.window.data1 / ev.window.data2 >= canvy.ratio)
                aspect_ratio = (float)ev.window.data2 / canvy.height;
            else
                aspect_ratio = (float)ev.window.data1 / canvy.width;

            destrect.w = canvy.width  * aspect_ratio,
            destrect.h = canvy.height * aspect_ratio;

            destrect.x = (ev.window.data1 - destrect.w) * 0.5f,
            destrect.y = (ev.window.data2 - destrect.h) * 0.5f;
        }
    }

    /* Event loop test code here */
#ifdef FLAG_DOKI
    static int mod = 0;
    SR_Canvas temp = SR_RefCanv(
        &ball, mod, mod, canvy.width, canvy.height, false);
    SR_MergeCanvasIntoCanvas(
        &canvy, &temp,
        0, 0,
        255, SR_BLEND_REPLACE);
    mod++;

    SR_MergeCanvasIntoCanvas(
        &canvy, &logo,
        32, 32,
        255, SR_BLEND_ADDITIVE);

    SR_MergeCanvasIntoCanvas(
        &canvy, &monkas,
        canvy.width - monkas.width, canvy.height - monkas.height,
        255, SR_BLEND_ADDITIVE);
#endif

#ifdef FLAG_FRACTAL
    static float minX = -2.0;
    static float maxX = 1.0;
    static float pos = 0;
    float yScale = (((float)maxX-minX) *
        ((float)canvy.height / canvy.width) * canvy.ratio);

    uint8_t shades[] = {
        0, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210,
        225, 240, 255};

    for (unsigned short x = 0; x < canvy.width; x++)
    for (unsigned short y = 0; y < canvy.height; y++) {
        float complex c = (
            ((float)minX + x * (float)(maxX - minX) / (float)canvy.width) +
            (y * yScale / canvy.height - yScale / 2.0) * I);

        c -= pos * 2; c /= (pos + 1);
        float complex z = c;

        size_t zC;
        for (zC = 0; zC < sizeof(shades); zC++) {
            if (abs(z) > 2) break;

            z = z * z + c;
        }

        SR_CanvasSetPixel(&canvy, x, y, SR_CreateRGBA(
            shades[zC],
            shades[zC],
            shades[zC],
            255));
    }

    pos += 1;
#endif

#ifdef FLAG_RAND_LINES
    SR_DrawLine(
        &canvy,
        SR_RGBABlender(
            SR_CreateRGBA(rand(), rand(), rand(), 255),
            SR_CreateRGBA(rand(), rand(), rand(), 255),
            255,
            SR_BLEND_OVERLAY
        ), 
        rand() % (canvy.width),
        rand() % (canvy.height),
        rand() % (canvy.width),
        rand() % (canvy.height)
    );
#endif

#ifdef FLAG_RAND_TRIS
    SR_DrawTri(
        &canvy,
        SR_CreateRGBA(rand(), rand(), rand(), 255),
        rand() % (canvy.width),
        rand() % (canvy.height),
        rand() % (canvy.width),
        rand() % (canvy.height),
        rand() % (canvy.width),
        rand() % (canvy.height)
    );
#endif

#ifdef FLAG_PUCK
    speeen += 1;
    rotcanvas = SR_CanvasRotate(&imagetest, speeen, 1, 1);
    unsigned short spx = 128 + rotcanvas.offset_x;
    unsigned short spy = 128 + rotcanvas.offset_y;
    SR_DrawRect(
        &canvy, SR_CreateRGBA(255, 255, 255, 255), 0, 0,
        canvy.width, canvy.height);
    SR_MergeCanvasIntoCanvas(
        &canvy, &(rotcanvas.canvas), spx, spy,
        //0, 0,
        255, SR_BLEND_ADDITIVE);
    SR_DrawRectOutline(
        &canvy, SR_CreateRGBA(255, 167, 15, 255),
        spx, spy, rotcanvas.canvas.width, rotcanvas.canvas.height);
    SR_DestroyCanvas(&(rotcanvas.canvas));
#endif
    
#ifdef FLAG_SQUISH
    static float the_squish = 0;
    the_squish += .03f;
    squish = SR_CanvasShear(
        &pokesquish,
        sin(the_squish) * 16,
        0);
    SR_MergeCanvasIntoCanvas(
        &canvy, &(squish.canvas),
        128 + squish.offset_x, 128 + squish.offset_y, 255, SR_BLEND_ADDITIVE);
    SR_DestroyCanvas(&(squish.canvas));
#endif

#ifdef FLAG_ROT
    SR_MergeCanvasIntoCanvas(&canvy, &boxes, 0, 0, 255, SR_BLEND_ADDITIVE);
#endif

#ifdef FLAG_ATLAS
    static uint16_t cheese_timer = 0;
    cheese_timer++;

    // get a texture from the atlas and tile it over the whole window
    SR_Canvas the = SR_RefCanvTile(&brick_tileset, 32, 32,
        (cheese_timer >> 3) & 3,
        (cheese_timer >> 5) % 6);
    SR_TileTo(&the, canvy.width, canvy.height);
    SR_MergeCanvasIntoCanvas(
        &canvy, &the,
        0, 0,
        255, SR_BLEND_REPLACE);

    // draw the atlas itself in the top left corner
    SR_MergeCanvasIntoCanvas(
         &canvy, &brick_tileset,
         24, 24,
         255, SR_BLEND_OVERLAY);
    
    // draw a box around the current texture
    SR_DrawRectOutline(
        &canvy, SR_CreateRGBA(255, 167, 15, 255),
        (((cheese_timer >> 3) & 3) << 5) + 23,
        (((cheese_timer >> 5) % 6) << 5) + 23,
        33, 33);

    static uint16_t hstri[] = u"This is the atlas demo!\n\nEnjoy!";
    afonta.colour = SR_CreateRGBA(255, 255, 255, 127);
    SR_PrintToCanvas(
        &afonta, &canvy, hstri, sizeof(hstri) / 2, 128, 128, 0,
        SR_BLEND_ADDITIVE);
#endif

    /* update the canvas here, the rest is
       actually blitting it to the window */
    
    static unsigned short frames = 0;
    static time_t laf = 0;
    static time_t cur = 0;
    frames++;
    cur = time(NULL);
    if (((cur & 1) == 0) && (laf != cur)) {
        printf("FPS: %u AT %lld\n", frames >> 1, (long long int)cur);
        laf = cur;
        frames = 0;
    }

    /* refresh the window */
    if (SDL_FillRect(wsurf, NULL, SDL_MapRGB(wsurf->format, 0, 0, 0)) < 0) {
        status = 6;
        goto sdl_freesurf;
    }

    if (SDL_BlitScaled(canvysurf, NULL, wsurf, &destrect) < 0) {
        status = 7;
        goto sdl_freesurf;
    }

    if (SDL_UpdateWindowSurface(win) < 0) {
        status = 8;
        goto sdl_freesurf;
    }

    //SDL_Delay(17);
    goto event_loop;

sdl_freesurf:
    SDL_FreeSurface(canvysurf);
sr_destroycanvas:
    SR_DestroyCanvas(&canvy);
sdl_destroywin:
    SDL_DestroyWindow(win);
sdl_quit:
    SDL_Quit();
    return status;
}
