#ifndef SURIM_HEADER_FILE
#define SURIM_HEADER_FILE
#include "glbl.h"
#include "canvas.h"
#include "colours.h"

// Load a Radix memory blob as an image and convert it into a canvas
SR_Canvas SR_ImageMemToCanvas(X0 *data, SX length);

// Load an image file into a new canvas (Uses stb_image.h)
SR_Canvas SR_ImageFileToCanvas(CHR *filename);

// WARNING: All created canvases must be freed as they are mallocated.
#endif
