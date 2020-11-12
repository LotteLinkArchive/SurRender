#ifndef SURBLVEC_HEADER_FILE
#define SURBLVEC_HEADER_FILE
#include "colours.h"
#include "remote_holy.h"
#include "remote_simde/x86/avx2.h"
#include "remote_simde/x86/avx512.h"

typedef union {
	simde__m256i vec;
	simde__m128i vec128s[2];
	U8  aU8x32[32];
	U16 aU16x16[16];
	U32 aU32x8[8];
	U64 aU64x4[4];

	U32 pix0;

	U32x8 vevec_epu32;
} pixbuf_t;

typedef union {
	simde__m512i avx512[2];
	simde__m256i avx2[4];
	simde__m128i avx[8];
	U32 aU32x32[32];
	U32x32 vevec_epu32;
} p32_local_t;

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
