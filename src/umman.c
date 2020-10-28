#include "umman.h"

#ifdef UMM_METHOD_MMAN
#include <sys/mman.h>
#elif defined(UMM_METHOD_MALLOC)
#include <string.h>
#include <stdio.h>
#endif

X0 *ummap(X0 *addr, SX length, INAT prot, INAT flags, INAT fd, OX offset)
{
	#ifdef UMM_METHOD_MMAN
	return mmap(addr, length, prot, flags, fd, offset);
	#elif defined(UMM_METHOD_MALLOC)
	X0 *vspace = realloc(addr, length);
	if (!vspace) return NULL;

	FILE *fp = fdopen(fd, "r");
	if (!fp) goto ummap_free_error;

	if (offset != 0) if (fseek(fp, offset, SEEK_SET) != 0) goto ummap_fclose_error;

	/* May read less than length bytes! */
	fread(vspace, 1, length, fp);

	return vspace;
ummap_fclose_error:
	fclose(fp);
ummap_free_error:
	free(vspace);
	return NULL;
	#endif
}

INAT umunmap(X0 *addr, SX length)
{
	#ifdef UMM_METHOD_MMAN
	return munmap(addr, length);
	#elif defined(UMM_METHOD_MALLOC)
	/* Literally do nothing. Windows users deserve memory leaks.
	 * (If anyone would like to introduce a better solution, feel free.)
	 */
	return 0;
	#endif
}