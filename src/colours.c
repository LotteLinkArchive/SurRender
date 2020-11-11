#include "colours.h"

SR_RGBAPixel SR_CreateRGBA(
	U8 red,
	U8 green,
	U8 blue,
	U8 alpha)
{
	SR_RGBAPixel temp  = {
		.chn.red   = red,
		.chn.green = green,
		.chn.blue  = blue,
		.chn.alpha = alpha
	};
	return temp;
}
