#ifndef SURCL_HEADER_FILE
#define SURCL_HEADER_FILE
#include "glbl.h"

typedef union {
	__extension__ struct {
		U8 red;
		U8 green;
		U8 blue;
		U8 alpha;
	} __attribute__ ((packed)) chn;
	U32 whole;
	U8x4 splitvec;
} SR_RGBAPixel;

typedef union {
	U64 whole;
	__extension__ struct {
		U32 left;
		U32 right;
	} __attribute__ ((packed)) uparts;
	__extension__ struct {
		SR_RGBAPixel left;
		SR_RGBAPixel right;
	} __attribute__ ((packed)) srparts;
	U8x8 splitvec;
} SR_RGBADoublePixel;

enum SR_BlendingModes {
	/* Add RGB values together with clamping and multiplication */
	SR_BLEND_ADDITIVE,
	/* Like additive blending, but doesn't change base alpha and doesn't
	 * multiply values. Can overflow. Use it to paint colour onto black.
	 */
	SR_BLEND_ADDITIVE_PAINT,
	/* XOR all RGB values after multiplying them */
	SR_BLEND_XOR,
	/* Directly XOR the RGB channels without mutating the alpha */
	SR_BLEND_DIRECT_XOR,
	/* Rounded overlay approach (fastest) */
	SR_BLEND_OVERLAY,
	/* Replace base alpha with inverted top alpha */
	SR_BLEND_INVERT_DROP,
	/* Replace base alpha with top alpha */
	SR_BLEND_DROP,
	/* Replace entire top pixel with bottom pixel */
	SR_BLEND_REPLACE,
	/* Replace entire top pixel with button pixel but use the alpha modifier of RGBABlender mutiplied by the
	 * alpha of the top pixel.
	 * 
	 * This is very useful for rendering fonts into a temporary canvas quickly.
	 */
	SR_BLEND_REPLACE_WALPHA_MOD,
	/* Directly XOR EVERYTHING (RGBA) without mutating the alpha */
	SR_BLEND_DIRECT_XOR_ALL,
	/* Depending on the alpha value of the top layer, invert the base colours */
	SR_BLEND_INVERTED_DRAW,
	/* Keep the bottom's alpha but use the top's RGB values */
	SR_BLEND_PAINT
};

/* Create an RGBA colour value. */
SR_RGBAPixel SR_CreateRGBA(
	U8 red,
	U8 green,
	U8 blue,
	U8 alpha);
	
#endif
