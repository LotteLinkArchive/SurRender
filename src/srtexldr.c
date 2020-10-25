#include "glbl.h"
#include "srtexldr.h"
#include "srtex.h"
#include "canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* SRT missing texture */
static U8 missingtex[] = {
	0x3F, 0x83, 0xE5, 0xEA, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x10,
	0x00, 0x00, 0x00, 0x04, 0x0C, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x83,
	0xE5, 0xEA, 0x02, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF};

STATUS SR_TexBlobToCanvas(
	X0 *blob,
	SX size_boundary,
	SR_Canvas *target,
	U1 indestructible,
	X0 *free_base_address)
{
	/* Zero-fill all of the target canvas values (Initialize everything to zero) */
	memset(target, 0, sizeof(SR_Canvas));

	/* Prepare to read the header */
	struct SRTHeader *head = blob;

	/* Perform careful boundary checking before accessing anything to try to prevent a buffer overflow */
	if (size_boundary < SRT_HEADER_WIDTH)                                          return SR_BUFFER_OVERFLOW;
	if (head->magicno != SRT_MAGIC_NUMBER)                                         return SR_INVALID_HEADER;
	if ((head->height * head->width * head->stbi_type) != head->data_length)       return SR_INVALID_HEADER;

	/* Header is clean by now */
	SX dangerzone = SRT_HEADER_WIDTH + head->offset;

	/* Check canvas eligbility and data body bounds */
	if (head->data_length > (size_boundary - dangerzone))                          return SR_BUFFER_OVERFLOW;
	if (head->stbi_type != 4)                                                      return SR_INVALID_TEXTURE;
	if ((head->width > SR_MAX_CANVAS_SIZE) || (head->height > SR_MAX_CANVAS_SIZE)) return SR_INVALID_SIZE;
	if ((head->width < 1) || (head->height < 1))                                   return SR_INVALID_SIZE;

	/* Define the boundary between header and image content */
	X0 *body = (CHR *)blob + dangerzone;

	/* Verify the image body checksum */
	if (head->checksum != fnv1b16((U8 *)body, head->data_length))                  return SR_INTEGRITY_ERROR;

	/* With all of the error-checking out of the way, we can now manipulate the target canvas */
	target->pixels = body;
	target->b_addr = free_base_address ? free_base_address : blob;
	target->width  = target->rwidth    = target->cwidth    = head->width;
	target->height = target->rheight   = target->cheight   = head->height;
	target->ratio  = (R32)head->width  / head->height;

	SR_GenCanvLUT(target);

	if (indestructible) target->hflags |= 0x02;

	/* Return the OK state */
	return SR_NO_ERROR;
}

STATUS SR_TexFDToCanvas(I32 fd, SX size, OX offset, SR_Canvas *target)
{
	/* Zero-fill all of the target canvas values (Initialize everything to zero) */
	memset(target, 0, sizeof(SR_Canvas));

	/* Invalidate invalid sizes */
	if (size < SRT_HEADER_WIDTH) return SR_INVALID_HEADER;

	/* Map the file to memory */
	X0 *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, offset);

	/* If the map failed, return now. */
	if (!mapped) return SR_MAP_FAILURE;

	/* Try to create a canvas out of it */
	STATUS decstat = SR_TexBlobToCanvas(mapped, size, target, false, NULL);

	/* If it failed, remove the mapping and exit, returning the TexBlobToCanvas code */
	if (decstat != SR_NO_ERROR) {
		munmap(mapped, size);
		return decstat;
	}

	/* Set the mmap flag on the canvas */
	target->hflags |= 0x08;
	target->munmap_size = size;

	return SR_NO_ERROR;
}

STATUS SR_TexFileToCanvas(CHR *filename, SR_Canvas *target)
{
	/* Zero-fill all of the target canvas values (Initialize everything to zero) */
	memset(target, 0, sizeof(SR_Canvas));

	/* Open the file */
	FILE *fp = fopen(filename, "r");
	if (!fp) return SR_FILE_ERROR;

	/* Get the size of the file by seeking to the end and then back to the beginning again */
	fseek(fp, 0, SEEK_END);
	SX size_boundary = ftell(fp);
	rewind(fp);

	STATUS decstat = SR_TexFDToCanvas(fileno(fp), size_boundary, 0, target);
	fclose(fp);

	return decstat;
}

SR_Canvas SR_TexFileCanvSoftFail(CHR *filename)
{
	/* Soft-failing functions like this should absolutely not be used. Please use SR_TexFileToCanvas instead. */

	SR_Canvas target = {};

	STATUS isostat = SR_TexFileToCanvas(filename, &target);
	if (isostat != SR_NO_ERROR) {
		printf("WARNING: Soft-fail texture map ignored exception %u! (%s)\n", (U32)isostat, filename);

		isostat = SR_TexBlobToCanvas(&missingtex, sizeof(missingtex), &target, true, NULL);
	}

	if (isostat != SR_NO_ERROR) {
		printf("FATAL ERROR: Could not map soft-fail texture to missing texture! (%u)\n", (U32)isostat);
		abort();
	}

	return target;
}
