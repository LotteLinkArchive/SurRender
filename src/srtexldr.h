#ifndef SURTXLDR_HEADER_FILE
#define SURTXLDR_HEADER_FILE
#include "glbl.h"
#include "canvas.h"
#include "srtex.h"
#include "errors.h"

/* Loads an in-memory SRT texture into a canvas.
 * An important point: The new canvas merely *references* the SRT blob. That means that if you free() the region of
 * memory represented by the blob argument, you will put the target canvas at risk. If you need to free that region,
 * you should malloc a new one and memcpy the blob to it before you run this.
 * 
 * blob -> Address of the SRT texture, including the header and body
 * 
 * size_boundary -> The maximum amount of memory to read. If the SRT header states it is larger than this amount, an
 * OOB error will be triggered.
 * 
 * target -> An empty canvas structure. This function will populate it with enough values to make it useful.
 * 
 * indestructible -> Whether or not it should be possible to destroy this canvas.
 * 
 * free_base_address -> The address to free when SR_DestroyCanvas is called. If set to NULL, it will free the blob
 * address.
 */
STATUS SR_TexBlobToCanvas(
	X0 *blob,
	SX size_boundary,
	SR_Canvas *target,
	U1 indestructible,
	X0 *free_base_address);

/* Maps a portion of a file to the target canvas. Takes in a *valid* file descriptor (undefined behaviour if invalid).
 * 
 * Size determines how many bytes in the range should be considered as part of the texture. Offset determines how many
 * bytes should be between the start of the texture and the start of the file given by the file descriptor.
 */
STATUS SR_TexFDToCanvas(I32 fd, SX size, OX offset, SR_Canvas *target);

/* Opens a texture file and maps it to the target canvas.
 *
 * This one is pretty simple. It does what it says on the tin. POSIX only.
 */
STATUS SR_TexFileToCanvas(CHR *filename, SR_Canvas *target);

/* Soft-failing version of SR_TexFileToCanvas(). On failure, tries to replace texture with a hard-coded missing texture.
 * 
 * Please don't use this. It's only intended as a drop-in replacement for the deprecated SR_ImageFileToCanvas function.
 */
SR_Canvas SR_TexFileCanvSoftFail(CHR *filename);

#endif