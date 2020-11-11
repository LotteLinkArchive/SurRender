#ifndef SURBLVEC_HEADER_FILE
#define SURBLVEC_HEADER_FILE
#include "colours.h"
#include "remote_simde/x86/avx512.h"
#include "remote_simde/x86/avx2.h"

typedef union {
	simde__m256i vec;
	U8  aU8x32[32];
	U16 aU16x16[16];
	U32 aU32x8[8];
	U64 aU64x4[4];
} pixbuf_t;

typedef union {
	simde__m512i vec;
	U8  aU8x64[64];
	U16 aU16x32[32];
	U32 aU32x16[16];
	U64 aU64x8[8];
} bigpixbuf_t;

/* Internal blending functions */

pixbuf_t SR_PixbufBlend(
	pixbuf_t srcAbuf,
	pixbuf_t srcBbuf,
	U8 alpha_modifier,
	I8 mode);
	
SR_RGBAPixel SR_RGBABlender(
	SR_RGBAPixel pixel_base,
	SR_RGBAPixel pixel_top,
	U8 alpha_modifier,
	I8 mode);

#endif
