#include "glbl.h"
#include "srtexldr.h"
#include "srtex.h"
#include "canvas.h"
#include "umman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SRT missing texture */
static U8 missingtex[160] = {
	0x3F, 0x83, 0xE5, 0xEA, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x04, 0x0C,
	0x0D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x83, 0xE5, 0xEA, 0x02, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x90, 0x08, 0x53, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xF1, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xBA, 0xA9, 0x2C, 0x14, 0x7F, 0x00, 0x00,
	0x80, 0xF8, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
	if (size_boundary < SRT_HEADER_WIDTH)                                                 return SR_BUFFER_OVERFLOW;
	if (head->magicno != SRT_MAGIC_NUMBER)                                                return SR_INVALID_HEADER;
	if ((head->height * SRT_WIDTH_ROUNDUP(head->width) * head->Bpp) != head->data_length) return SR_INVALID_HEADER;

	/* Header is clean by now */
	SX dangerzone = SRT_HEADER_WIDTH + head->offset;

	/* Check canvas eligbility and data body bounds */
	if (head->data_length > (size_boundary - dangerzone))                                 return SR_BUFFER_OVERFLOW;
	if (head->Bpp != 4)                                                                   return SR_INVALID_TEXTURE;
	if ((head->width > SR_MAX_CANVAS_SIZE) || (head->height > SR_MAX_CANVAS_SIZE))        return SR_INVALID_SIZE;
	if ((head->width < 1) || (head->height < 1))                                          return SR_INVALID_SIZE;

	/* Define the boundary between header and image content */
	X0 *body = (CHR *)blob + dangerzone;

	/* Verify the image body checksum */
	if (head->checksum != fnv1b16((U8 *)body, head->data_length))                         return SR_INTEGRITY_ERROR;

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

STATUS SR_TexFDToCanvas(INAT fd, SX size, OX offset, SR_Canvas *target)
{
	/* Zero-fill all of the target canvas values (Initialize everything to zero) */
	memset(target, 0, sizeof(SR_Canvas));

	/* Invalidate invalid sizes */
	if (size < SRT_HEADER_WIDTH) return SR_INVALID_HEADER;

	/* Map the file to memory */
	X0 *mapped = ummap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, offset);

	/* If the map failed, return now. */
	if (!mapped) return SR_MAP_FAILURE;

	/* Try to create a canvas out of it */
	STATUS decstat = SR_TexBlobToCanvas(mapped, size, target, false, NULL);

	/* If it failed, remove the mapping and exit, returning the TexBlobToCanvas code */
	if (decstat != SR_NO_ERROR) {
		umunmap(mapped, size);
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
	if (fseek(fp, 0, SEEK_END) != 0) {
		fclose(fp);
		return SR_FILE_ERROR;
	}
	SX size_boundary = ftell(fp);
	rewind(fp);

	STATUS decstat = SR_TexFDToCanvas(fileno(fp), size_boundary, 0, target);
	fclose(fp);

	return decstat;
}

SR_Canvas SR_TexFileCanvSoftFail(CHR *filename)
{
	/* Soft-failing functions like this should absolutely not be used. Please use SR_TexFileToCanvas instead. */

	SR_Canvas target;

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
