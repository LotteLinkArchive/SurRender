#ifndef SURSH_HEADER_FILE
#define SURSH_HEADER_FILE
	#include "glbl.h"
	#include "canvas.h"
	#include "colours.h"
	
	//line drawing function
	X0 SR_DrawLine(
		SR_Canvas *canvas,
		SR_RGBAPixel colour,
		I32 x0,
		I32 y0,
		I32 x1,
		I32 y1);
	
	//triangle drawing function
	X0 SR_DrawTriOutline(
		SR_Canvas *canvas,
		SR_RGBAPixel colour,
		I32 x0,
		I32 y0,
		I32 x1,
		I32 y1,
		I32 x2,
		I32 y2);

	X0 SR_DrawTri(
		SR_Canvas *canvas,
		SR_RGBAPixel colour,
		I32 x0,
		I32 y0, 
		I32 x1, 
		I32 y1,
		I32 x2,
		I32 y2);
	
	//get pain wrecked
	X0 SR_DrawRectOutline(
		SR_Canvas *canvas,
		SR_RGBAPixel colour,
		U16 x,
		U16 y,
		U16 w,
		U16 h);
	
	X0 SR_DrawRect(
		SR_Canvas *canvas,
		SR_RGBAPixel colour,
		U16 x,
		U16 y,
		U16 w,
		U16 h);
	
	//round
	X0 SR_DrawCircOutline(
		SR_Canvas *canvas,
		SR_RGBAPixel colour,
		U16 x,
		U16 y,
		U16 r);
	
	X0 SR_DrawCirc(
		SR_Canvas *canvas,
		SR_RGBAPixel colour,
		I32 x,
		I32 y,
		I32 r);
#endif
