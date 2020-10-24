#include "libs/holyh/src/holy.h"
#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb/stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/srtex.h"

I32 main(I32 argc, CHR *argv[])
{
	CHR error[256];
	I32 status = 0;
	CHR *idata;
	#define I2SRERR(statno, errmsg) { status = statno; strcpy(error, errmsg); goto cexit; }

	if (argc < 3)
		I2SRERR(1, "argc_check: too few arguments.");
	if (access(argv[1], F_OK | R_OK) == -1)
		I2SRERR(2, "access(): could not check F[ind]_OK and R[ead]_OK on input file");

	struct SRTHeader newsrt;
	idata = (CHR *)stbi_load(argv[1], &newsrt.width, &newsrt.height, &newsrt.stbi_type, 4);	
	if (!idata)
		I2SRERR(3, "stbi_load(): invalid input file format or unable to read input file");

	newsrt.data_length = newsrt.width * newsrt.height * newsrt.stbi_type;
	FILE *fp = fopen(argv[2], "w");

	if (!fp) 
		I2SRERR(4, "fopen(): unable to open output file with write mode");

	fwrite(&newsrt, sizeof(newsrt), 1, fp);
	fwrite(idata, 1, newsrt.data_length, fp);
	fclose(fp);

cexit:
	if (!status) goto cexitntx;
	printf(	"Software failure. (Guru Meditation #%08X.%08X, \"%s\")\n\n"
		"Usage: %s [INPUT_IMAGE_PATH](.jpg/.jpeg/.png/.bmp/...) [OUTPUT_TEXTURE](.srt)\n"
		"Example: %s catpicture.png cattexture.srt\n"
		, status, (U32)&error, error, argv[0], argv[0]);
cexitntx:
	if (idata) free(idata);

	return status;

	#undef I2SRERR
}