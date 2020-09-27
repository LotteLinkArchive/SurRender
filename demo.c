#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdlib.h>
#include "src/surrender.h"
#include "libs/holyh/src/holy.h"
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Demo specification
#define SR_PCANVAS (convstate->primary_canvas)
#include <demos.h>
#ifndef SR_DEMO_PROG
#define SR_DEMO_INIT
#define SR_DEMO_LOOP
#define SR_DEMO_CLRF
#endif

// Allows for basic communication of important data between the demo thread and the main thread
typedef struct {
	INAT threads_created;
	INAT demo_status;
	SR_Canvas primary_canvas;
	pthread_mutex_t sr_render_mutex;
} demo_thread_state_t;

// This is the thread all SurRender calculations are performed on for accurate performance measurements
X0 *DemoThread(X0 *state) {
	demo_thread_state_t *convstate = (demo_thread_state_t *)state;
	printf("SurRender performing calculations on Thread ID %d\n", convstate->threads_created);

	pthread_mutex_lock(&convstate->sr_render_mutex);
	SR_DEMO_INIT
	pthread_mutex_unlock(&convstate->sr_render_mutex);
sr_event_loop:
	// Check if the main thread has just exited, and clean up if it has
	if (convstate->demo_status == 0xFF) goto sr_finish_loop;

	pthread_mutex_lock(&convstate->sr_render_mutex);
	SR_DEMO_LOOP
	pthread_mutex_unlock(&convstate->sr_render_mutex);

	#ifndef SR_DEMO_NO_FPS_COUNTER
	static U64 frames = 0;
	static time_t laf = 0;
	static time_t cur = 0;

	frames++;
	cur = time(NULL);
	if (((cur & 1) == 0) && (laf != cur)) {
		printf("FPS: %llu AT %lld\n", frames >> 1, (I64)cur);

		laf = cur;
		frames = 0;
	}
	#else
	// Required to convince GCC not to optimize out this area of code. FFS.
	asm volatile("" : : : "memory");
	#endif

	// Repeat the event loop
	goto sr_event_loop;
sr_finish_loop:
	pthread_mutex_lock(&convstate->sr_render_mutex);
	SR_DEMO_CLRF
	pthread_mutex_unlock(&convstate->sr_render_mutex);

	if (convstate->demo_status == 0x00)
		convstate->demo_status = 0x01;
	pthread_exit(NULL);
}

// Global demo state
static demo_thread_state_t state = {
	.threads_created = 0,
	.demo_status     = 0
};

// This is the thread where all of the SDL2 work is done (the main thread), because SDL2 is slow
INAT main(X0)
{
	// Return status code (0 = success)
	INAT status = 0;

	// Information about the SurRender calculation thread
	pthread_t SR_Thread;

	// Render mutex initialization
	if (pthread_mutex_init(&state.sr_render_mutex, NULL)) {
		status = 0x33;
		goto srdm_main_thread_exit;
	}

	// Initialize the primary canvas in the global state
	state.primary_canvas = SR_NewCanvas(640, 480);

	// Fail if the primary canvas is not valid
	if (!SR_CanvasIsValid(&state.primary_canvas)) {
		status = 0xFF;
		goto pt_destroy_mutex;
	}

	void *buffercanvas = malloc(state.primary_canvas.rwidth * state.primary_canvas.rheight * sizeof(SR_RGBAPixel));
	if (!buffercanvas) {
		status = 0x33;
		goto sr_destroycanvas;
	}

	// Finally, if the canvas is valid, we can start doing SurRender stuff safely. Start the thread!
	if (pthread_create(&SR_Thread, NULL, DemoThread, (X0 *)&state)) {
		status = 0xAD;
		goto sr_destroybufcanv;
	} else state.threads_created++;

	// Begin SDL initialization
	SDL_Window *win;
	SDL_Event ev;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		status = 0x01;
		goto sr_destroythrd;
	}

	// Create the primary SDL window
	if (!(win = SDL_CreateWindow(
		"SurRender Demonstration and Testing Program",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		state.primary_canvas.width,
		state.primary_canvas.height,
		SDL_WINDOW_RESIZABLE))) {
		status = 0x02;
		goto sdl_quit;
	}

	// Create the initial canvas surface for use with SDL2, as well as declaring the window surface
	SDL_Surface *sdl_srcanv_surf, *sdl_window_surf;
	float sdl_sr_aspect_ratio;

	if (!(sdl_srcanv_surf = SDL_CreateRGBSurfaceFrom(
		buffercanvas, // Use the buffer canvas
		state.primary_canvas.width,
		state.primary_canvas.height,
		32, // C. Depth
		state.primary_canvas.width * sizeof(SR_RGBAPixel),
		0x000000FF, // Channel masks
		0x0000FF00,
		0x00FF0000,
		0xFF000000))) {
		status = 0x04;
		goto sdl_destroywin;
	}

	// Destination rectangle for blitting - changes whenever the window is resized
	SDL_Rect destrect;
	destrect.x = 0, destrect.y = 0,
	destrect.w = state.primary_canvas.width, destrect.h = state.primary_canvas.height;

	// Get the window surface for blitting
	if (!(sdl_window_surf = SDL_GetWindowSurface(win))) {
		status = 0x05;
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
			// Perform fitting whenever the resolution changes
			if (!(sdl_window_surf = SDL_GetWindowSurface(win))) {
				status = 5;
				goto sdl_freesurf;
			}

			if ((float)ev.window.data1 / ev.window.data2 >= state.primary_canvas.ratio)
				sdl_sr_aspect_ratio = (float)ev.window.data2 / state.primary_canvas.height;
			else
				sdl_sr_aspect_ratio = (float)ev.window.data1 / state.primary_canvas.width;

			destrect.w = state.primary_canvas.width  * sdl_sr_aspect_ratio,
			destrect.h = state.primary_canvas.height * sdl_sr_aspect_ratio;

			destrect.x = (ev.window.data1 - destrect.w) * 0.5f,
			destrect.y = (ev.window.data2 - destrect.h) * 0.5f;
		}
	}

	// Fill the window surface with zeros
	if (SDL_FillRect(sdl_window_surf, NULL, SDL_MapRGB(sdl_window_surf->format, 0, 0, 0)) < 0) {
		status = 0x06;
		goto sdl_freesurf;
	}

	// Blit between canvas surface and window surface (SLOW)
	pthread_mutex_lock(&state.sr_render_mutex);
	memcpy(buffercanvas, state.primary_canvas.pixels,
		state.primary_canvas.rwidth * state.primary_canvas.rheight * sizeof(SR_RGBAPixel));
	pthread_mutex_unlock(&state.sr_render_mutex);
	if (SDL_BlitScaled(sdl_srcanv_surf, NULL, sdl_window_surf, &destrect) < 0) {
		status = 0x07;
		goto sdl_freesurf;
	}

	// Update the window surface
	if (SDL_UpdateWindowSurface(win) < 0) {
		status = 0x08;
		goto sdl_freesurf;
	}

	if (state.demo_status != 0x00) {
		status = state.demo_status - 1;
		goto sdl_freesurf;
	}

	// Wait (60 FPS target)
	SDL_Delay(17);

	// Repeat event loop
	goto event_loop;
sdl_freesurf:
	SDL_FreeSurface(sdl_srcanv_surf);
sdl_destroywin:
	SDL_DestroyWindow(win);
sdl_quit:
	SDL_Quit();
sr_destroythrd:
	// Nicely ask the SurRender thread to exit
	if (state.demo_status == 0x00)
		state.demo_status = 0xFF;

	pthread_join(SR_Thread, NULL);
sr_destroybufcanv:
	free(buffercanvas);
sr_destroycanvas:
	SR_DestroyCanvas(&state.primary_canvas);
pt_destroy_mutex:
	pthread_mutex_destroy(&state.sr_render_mutex);
srdm_main_thread_exit:
	pthread_exit(NULL);
	return status;
}