#ifndef SURSLE_HEADER_FILE
#define SURSLE_HEADER_FILE
#include "glbl.h"
#include "canvas.h"
#include "colours.h"

#define SR_SelectIsValid(selection) BOOLIFY(selection->bitfield);

typedef struct SR_Select {
	/* Width and height, in bits */
	U16 width;
	U16 height;
	
	/* Pointer to an array of bytes */
	U8 *bitfield;
} SR_Select;

/* Create a new selecty box of given dimensions */
SR_Select SR_NewSelect(U16 width, U16 height);

/* Destroy + free a selecty box */
X0 SR_DestroySelect(SR_Select *selection);

/* Different ways to modify a selection with SR_SelectSetPoint */
enum SR_SelectModes {
	SR_SMODE_SET,
	SR_SMODE_RESET,
	SR_SMODE_XOR
};

/* Modify a point in a selection */
X0 SR_SelectSetPoint(
	SR_Select *selection,
	U16 x,
	U16 y,
	I8 mode);

/* checque if a bit is sett */
U1 SR_SelectGetPoint(
	SR_Select *selection,
	U16 x,
	U16 y);

/* select yonder line */
X0 SR_SelectLine(
	SR_Select *selection, I8 mode,
	I32 x0, I32 y0,
	I32 x1, I32 y1);

/* select yonder triangle */
X0 SR_SelectTri(
	SR_Select *selection, I8 mode,
	I32 x0, I32 y0, 
	I32 x1, I32 y1,
	I32 x2, I32 y2);

/* select yonder rectangle */
X0 SR_SelectRect(
	SR_Select *selection, I8 mode,
	I32 x, I32 y,
	I32 w, I32 h);

/* select yonder circle */
X0 SR_SelectCirc(
	SR_Select *selection, I8 mode,
	I32 x, I32 y,
	I32 r);
#endif
