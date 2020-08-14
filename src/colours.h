#ifndef SURCL_HEADER_FILE
#define SURCL_HEADER_FILE
    #include "glbl.h"

    typedef struct SR_RGBPixel {
	    uint8_t red;
        uint8_t green;
        uint8_t blue;
	} SR_RGBPixel;

    typedef struct SR_RGBAPixel {
	    SR_RGBPixel rgb;
        uint8_t alpha;
	} SR_RGBAPixel;

    // Construct an RGB colour value.
    SR_RGBPixel SR_CreateRGB(uint8_t red, uint8_t green, uint8_t blue);

    // Create an RGBA colour value.
    SR_RGBAPixel SR_CreateRGBA(
        uint8_t red,
        uint8_t green,
        uint8_t blue,
        uint8_t alpha);

    // Conversion
    SR_RGBPixel SR_RGBAtoRGB(SR_RGBAPixel pixel);
    SR_RGBAPixel SR_RGBtoRGBA(SR_RGBPixel pixel, uint8_t alpha);
#endif