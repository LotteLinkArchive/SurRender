#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdlib.h>
#include "src/surrender.h"
#include "libs/holyh/src/holy.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

/* Demo specification */
#define SR_PCANVAS (convstate->primary_canvas)
#include <demos.h>
#ifndef SR_DEMO_PROG
#define SR_DEMO_INIT
#define SR_DEMO_LOOP
#define SR_DEMO_CLRF
#endif

/* Lock priorities */
typedef struct prio_lock {
	pthread_cond_t cond;
	pthread_mutex_t cv_mutex; /* Condition variable mutex */
	pthread_mutex_t cs_mutex; /* Critical section mutex */
	unsigned long high_waiters;
} prio_lock_t;

#define PRIO_LOCK_INITIALIZER { PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER }

void prio_lock_low(prio_lock_t *prio_lock)
{
	pthread_mutex_lock(&prio_lock->cv_mutex);
	while (prio_lock->high_waiters || pthread_mutex_trylock(&prio_lock->cs_mutex))
	{
		pthread_cond_wait(&prio_lock->cond, &prio_lock->cv_mutex);
	}
	pthread_mutex_unlock(&prio_lock->cv_mutex);
}

void prio_unlock_low(prio_lock_t *prio_lock)
{
	pthread_mutex_unlock(&prio_lock->cs_mutex);

	pthread_mutex_lock(&prio_lock->cv_mutex);
	if (!prio_lock->high_waiters)
		pthread_cond_signal(&prio_lock->cond);
	pthread_mutex_unlock(&prio_lock->cv_mutex);
}

void prio_lock_high(prio_lock_t *prio_lock)
{
	pthread_mutex_lock(&prio_lock->cv_mutex);
	prio_lock->high_waiters++;
	pthread_mutex_unlock(&prio_lock->cv_mutex);

	pthread_mutex_lock(&prio_lock->cs_mutex);
}

void prio_unlock_high(prio_lock_t *prio_lock)
{
	pthread_mutex_unlock(&prio_lock->cs_mutex);

	pthread_mutex_lock(&prio_lock->cv_mutex);
	prio_lock->high_waiters--;
	if (!prio_lock->high_waiters)
		pthread_cond_signal(&prio_lock->cond);
	pthread_mutex_unlock(&prio_lock->cv_mutex);
}

/* Time value time difference */
float timedifference_msec(struct timeval t0, struct timeval t1)
{
	return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

/* Allows for basic communication of important data between the demo thread and the main thread */
typedef struct {
	INAT threads_created;
	INAT demo_status;
	SR_Canvas primary_canvas;
	prio_lock_t *sr_render_mutex;
} demo_thread_state_t;

/* This is the thread all SurRender calculations are performed on for accurate performance measurements */
X0 *DemoThread(X0 *state)
{
	demo_thread_state_t *convstate = (demo_thread_state_t *)state;
	printf("SurRender performing calculations on Thread ID %d\n", convstate->threads_created);

	#ifndef SR_DEMO_NO_FPS_COUNTER
	U64 frames;
	time_t laf, cur;

	frames = 0;
	laf = cur = time(NULL);
	#endif

	prio_lock_low(convstate->sr_render_mutex);
	SR_DEMO_INIT
	prio_unlock_low(convstate->sr_render_mutex);

	while (convstate->demo_status != 255) {
		prio_lock_low(convstate->sr_render_mutex);
		SR_DEMO_LOOP
		prio_unlock_low(convstate->sr_render_mutex);

		#ifndef SR_DEMO_NO_FPS_COUNTER
		if ((cur = time(NULL)) >= laf + 2) {
			printf("FPS: %llu AT %lld\n", frames >> 1, (I64)cur);

			laf = cur;
			frames = 0;
		} else {
			frames++;
		}
		#endif
	}

	prio_lock_low(convstate->sr_render_mutex);
	SR_DEMO_CLRF
	prio_unlock_low(convstate->sr_render_mutex);

	if (convstate->demo_status == 0)
		convstate->demo_status = 1;
	pthread_exit(NULL);
}

/* Global demo state */
static demo_thread_state_t state;

/* This is the thread where all of the SDL2 work is done (the main thread), because SDL2 is slow */
INAT main(X0)
{
	/* Per frame timing */
	struct timeval t0, t1;
	float elapsed;

	/* Create the initial canvas surface for use with SDL2, as well as declaring the window surface */
	SDL_Surface *sdl_srcanv_surf, *sdl_window_surf;

	/* Destination rectangle for blitting - changes whenever the window is resized */
	SDL_Rect destrect;
	SDL_Window *win;
	SDL_Event ev;

	float sdl_sr_aspect_ratio;

	/* Return status code (0 = success) */
	INAT status = 0;

	void *buffercanvas;

	/* Information about the SurRender calculation thread */
	pthread_t SR_Thread;

	/* Render mutex initialization */
	prio_lock_t mmutex = PRIO_LOCK_INITIALIZER;

	/* Initialize the primary canvas along with the global state */
	state.primary_canvas  = SR_NewCanvas(640, 480);
	state.sr_render_mutex = &mmutex;
	state.threads_created = 0;
	state.demo_status     = 0;

	/* Fail if the primary canvas is not valid */
	if (!SR_CanvasIsValid(&state.primary_canvas)) {
		status = 1;
		goto srdm_main_thread_exit;
	}

	if (!(buffercanvas = malloc(
		  state.primary_canvas.rwidth 
		* state.primary_canvas.rheight
		* sizeof(SR_RGBAPixel)))) {
		status = 2;
		goto sr_destroycanvas;
	}

	/* Finally, if the canvas is valid, we can start doing SurRender stuff safely. Start the thread! */
	if (pthread_create(&SR_Thread, NULL, DemoThread, (X0 *)&state)) {
		status = 3;
		goto sr_destroybufcanv;
	}

	state.threads_created++;

	/* Begin SDL initialization */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		status = 4;
		goto sr_destroythrd;
	}

	/* Create the primary SDL window */
	if (!(win = SDL_CreateWindow(
		"SurRender Demonstration and Testing Program",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		state.primary_canvas.width,
		state.primary_canvas.height,
		SDL_WINDOW_RESIZABLE))) {
		status = 5;
		goto sdl_quit;
	}

	if (!(sdl_srcanv_surf = SDL_CreateRGBSurfaceFrom(
		buffercanvas, /* Use the buffer canvas */
		state.primary_canvas.width,
		state.primary_canvas.height,
		32, /* Color Depth */
		state.primary_canvas.width * sizeof(SR_RGBAPixel),
		0x000000FF, /* Channel masks (RGBA in this case) */
		0x0000FF00,
		0x00FF0000,
		0xFF000000))) {
		status = 6;
		goto sdl_destroywin;
	}

	/* RODGER: Clear the destination rectangle */
	destrect.x = 0;
	destrect.y = 0;
	destrect.w = state.primary_canvas.width;
	destrect.h = state.primary_canvas.height;

	#define REFRESH_SDL_WINDOW_SURF(status_a, status_b)\
	/* RODGER: Refresh the window surface handle  */\
	if (!(sdl_window_surf = SDL_GetWindowSurface(win))) {\
		status = status_a;\
		goto sdl_freesurf;\
	}\
	\
	/* Clear the window surface */\
	if (SDL_FillRect(sdl_window_surf, NULL, SDL_MapRGB(sdl_window_surf->format, 0, 0, 0)) < 0) {\
		status = status_b;\
		goto sdl_freesurf;\
	}

	REFRESH_SDL_WINDOW_SURF(7, 8)
event_loop:
	gettimeofday(&t0, 0);
	while (SDL_PollEvent(&ev)) {
		if (ev.type == SDL_QUIT) {
			status = 0;
			goto sdl_freesurf;
		}

		if (ev.type == SDL_WINDOWEVENT)
		if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			REFRESH_SDL_WINDOW_SURF(9, 10)

			/* Perform fitting whenever the resolution changes */
			if ((float)ev.window.data1 / ev.window.data2 >= state.primary_canvas.ratio)
				sdl_sr_aspect_ratio = (float)ev.window.data2 / state.primary_canvas.height;
			else
				sdl_sr_aspect_ratio = (float)ev.window.data1 / state.primary_canvas.width;

			destrect.w = state.primary_canvas.width  * sdl_sr_aspect_ratio;
			destrect.h = state.primary_canvas.height * sdl_sr_aspect_ratio;

			destrect.x = (ev.window.data1 - destrect.w) * 0.5f;
			destrect.y = (ev.window.data2 - destrect.h) * 0.5f;
		}
	}

	/* Blit between canvas surface and window surface (SLOW) */
	prio_lock_high(state.sr_render_mutex);
		memcpy(buffercanvas, state.primary_canvas.pixels,
			state.primary_canvas.rwidth * state.primary_canvas.rheight * sizeof(SR_RGBAPixel));
	prio_unlock_high(state.sr_render_mutex);

	SDL_BlitScaled(sdl_srcanv_surf, NULL, sdl_window_surf, &destrect);
	SDL_UpdateWindowSurface(win);

	if (state.demo_status != 0) {
		status = state.demo_status - 1;
		goto sdl_freesurf;
	}
	gettimeofday(&t1, 0);

	/* Wait (60 FPS target) */
	SDL_Delay(abs((int)(17.0f - timedifference_msec(t0, t1))));

	/* Repeat event loop */
	goto event_loop;

sdl_freesurf:
	SDL_FreeSurface(sdl_srcanv_surf);

sdl_destroywin:
	SDL_DestroyWindow(win);

sdl_quit:
	SDL_Quit();

sr_destroythrd:
	/* Nicely ask the SurRender thread to exit */
	if (state.demo_status == 0)
		state.demo_status = 255;

	pthread_join(SR_Thread, NULL);

sr_destroybufcanv:
	free(buffercanvas);

sr_destroycanvas:
	SR_DestroyCanvas(&state.primary_canvas);

srdm_main_thread_exit:
	pthread_exit(NULL);
	return status;
}
