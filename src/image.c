#include "glbl.h"
#include "image.h"
#include "canvas.h"
#include "colours.h"

/* Including in the header prevents SurRender
 * from being included multiple times
 */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

U8 * LD_Blob_STBI(X0 *data, SX length, I32 *x, I32 *y, I32 *n)
{
	return stbi_load_from_memory(data, length, x, y, n, 4);
}

SR_Canvas LD_STBICanv(U8 *image, I32 *x, I32 *y)
{
	// @direct
	SR_Canvas temp = {
		.pixels   = (SR_RGBAPixel *) image,
		.width    = *x,
		.height   = *y,
		.rwidth   = *x,
		.rheight  = *y,
		.cwidth   = *x,
		.cheight  = *y,
		.ratio    = (R32)*x / *y,
	};

	if (!temp.pixels) goto ldstbicanv_missing;
	else goto ldstbicanv_perfect;
ldstbicanv_missing:
	temp = SR_NewCanvas(2, 2);
	if (!temp.pixels) goto ldstbicanv_fin;

	SR_CanvasSetPixel(&temp, 0, 0, SR_CreateRGBA(255, 0  , 255, 255));
	SR_CanvasSetPixel(&temp, 0, 1, SR_CreateRGBA(0  , 0  , 0  , 255));
	SR_CanvasSetPixel(&temp, 1, 0, SR_CreateRGBA(0  , 0  , 0  , 255));
	SR_CanvasSetPixel(&temp, 1, 1, SR_CreateRGBA(255, 0  , 255, 255));
ldstbicanv_perfect:
	if (!UINTP2CHK(temp.rwidth) || !UINTP2CHK(temp.rheight)) {
		SR_Canvas temp_old = temp;
		temp = SR_NewCanvas(u32rup2(temp_old.rwidth), u32rup2(temp_old.rheight));
		if (!temp.pixels) goto ldstbicanv_fin;
		SR_ZeroFill(&temp);

		for (U32 cgy = 0; cgy < temp_old.rheight; cgy++) {
			U32 dispN = (temp.rwidth * cgy);
			U32 dispO = (temp_old.rwidth * cgy);

			memcpy(temp.pixels + dispN, temp_old.pixels + dispO, temp_old.rwidth * sizeof(SR_RGBAPixel));
		}
	}
ldstbicanv_fin:
	SR_GenCanvLUT(&temp);

	return temp;
}

SR_Canvas SR_ImageMemToCanvas(X0 *data, SX length)
{
	I32 x, y, n = 0;
	return LD_STBICanv(LD_Blob_STBI(data, length, &x, &y, &n), &x, &y);
}

SR_Canvas SR_ImageFileToCanvas(CHR *filename)
{
	I32 x, y, n = 0;
	return LD_STBICanv(stbi_load(filename, &x, &y, &n, 4), &x, &y);
}
