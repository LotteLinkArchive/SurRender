#include "../libs/holyh/src/holy.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#include "../libs/stb/stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../src/srtex.h"

I32 main(I32 argc, CHR *argv[])
{
	CHR error[256];
	I32 status = 0;
	X0 *idata = NULL;
	#define I2SRERR(statno, errmsg) { status = statno; strcpy(error, errmsg); goto cexit; }

	if (argc < 3)
		I2SRERR(1, "argc_check: too few arguments.");
	if (access(argv[1], F_OK | R_OK) == -1)
		I2SRERR(2, "access(): could not check F[ind]_OK and R[ead]_OK on input file");

	printf("stbi_load(): decoding \"%s\"\n", argv[1]);

	struct SRTHeader newsrt = SRT_INIT;
	I32 x, y, z;
	idata = (CHR *)stbi_load(argv[1], &x, &y, &z, 4);
	newsrt.width = (U32)x;
	newsrt.height = (U32)y;
	newsrt.Bpp = 4; /* Always 4 channels */
	if (!idata)
		I2SRERR(3, "stbi_load(): invalid input file format or unable to read input file");

	newsrt.data_length = SRT_WIDTH_ROUNDUP(newsrt.width) * newsrt.height * (U32)newsrt.Bpp;
	idata = realloc(idata, newsrt.data_length);
	if (!idata) I2SRERR(4, "unable to allocate extra memory");

	printf("stbi_load(): decoded %u bytes as type %u\n", newsrt.data_length, (U32)newsrt.Bpp);

	newsrt.checksum = fnv1b16((U8 *)idata, newsrt.data_length);
	printf("fnv1b16(): data checksum is %04X\n", newsrt.checksum);

	newsrt.offset = MIN(255, u32rup2(sizeof(newsrt)) - sizeof(newsrt));
	printf("u32rup2(): decided to add %u bytes of padding\n", (U32)newsrt.offset);

	FILE *fp = fopen(argv[2], "w");

	if (!fp) 
		I2SRERR(4, "fopen(): unable to open output file with write mode");

	printf("fopen(): opened \"%s\" for writing\n", argv[2]);

	fwrite(&newsrt, sizeof(newsrt), 1, fp);
	fwrite(&status, 1, newsrt.offset, fp);
	fwrite(idata, 1, newsrt.data_length, fp);

	U32 written = (U32)ftell(fp);
	printf("fwrite(): wrote %u bytes (%u header, %u padding, %u body)\n",
		written, (U32)sizeof(newsrt), (U32)newsrt.offset, (U32)newsrt.data_length);
	fclose(fp);

cexit:
	if (!status) goto cexitntx;
	printf(	"Software failure. (Guru Meditation #%02X.%016" PRIXPTR ", \"%s\")\n\n"
		"Usage: %s [INPUT_IMAGE_PATH](.jpg/.jpeg/.png/.bmp/...) [OUTPUT_TEXTURE](.srt/.srtx/...)\n"
		"Example: %s catpicture.png cattexture.srt\n"
		, status, (UPTR)&error, error, argv[0], argv[0]);
cexitntx:
	if (idata) free(idata);

	return status;

	#undef I2SRERR
}
