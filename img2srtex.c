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
	#define I2SRERR(statno) { status = statno; goto cexit; }

	I32 status = 0;
	CHR *idata;
	
	if (argc < 3)
		I2SRERR(1);
	if (access(argv[1], F_OK | R_OK) == -1)
		I2SRERR(2);

	struct SRTHeader newsrt;
	idata = (CHR *)stbi_load(argv[1], &newsrt.width, &newsrt.height, &newsrt.stbi_type, 4);	
	if (!idata)
		I2SRERR(3);

	newsrt.data_length = newsrt.width * newsrt.height * newsrt.stbi_type;
	FILE *fp = fopen(argv[2], "w");

	if (!fp) 
		I2SRERR(4);

	fwrite(&newsrt, sizeof(newsrt), 1, fp);
	fwrite(idata, 1, newsrt.data_length, fp);
	fclose(fp);

cexit:
	if (!status) goto cexitntx;
	printf(	"Software failure. (Guru Meditation #%08X)\n\n"
		"Usage: %s [INPUT_IMAGE_PATH](.jpg/.jpeg/.png/.bmp/...) [OUTPUT_TEXTURE](.srt)\n"
		"Example: %s catpicture.png cattexture.srt\n"
		, status, argv[0], argv[0]);
cexitntx:
	if (idata) free(idata);

	return status;

	#undef I2SRERR
}