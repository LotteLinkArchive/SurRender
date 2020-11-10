#ifndef SURBLVEC_HEADER_FILE
#define SURBLVEC_HEADER_FILE
#include "colours.h"
#include "remote_simde/x86/avx512.h"
#include "remote_simde/x86/avx2.h"

typedef union {
	U8x64  sU8x64;
	U16x32 sU16x32;
	U64x8  sU64x8;
	U32x16 sU32x16;
	U32 aU32x16[16];
} pixbuf_t;

typedef union {
	U8x128 sU8x128;
	U16x64 sU16x64;
	U32x32 sU32x32;
} bigpixbuf_t;

typedef union {
	U16x128 sU16x128;
	__extension__ struct {
		U16x64 c1;
		U16x64 c2;
	} __attribute__ ((packed)) sU16x64x2;
} largepixbuf_t;

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
