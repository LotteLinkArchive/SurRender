#ifndef SURERR_HEADER_FILE
#define SURERR_HEADER_FILE
#include "glbl.h"

enum SR_Error {
	SR_NO_ERROR,
	SR_BUFFER_OVERFLOW,
	SR_FILE_ERROR,
	SR_MAP_FAILURE,
	SR_INVALID_HEADER,
	SR_INVALID_SIZE,
	SR_INVALID_TEXTURE,
	SR_MALLOC_FAILURE,
	SR_NULL_CANVAS,
	SR_NONZERO_REFCOUNT,
	SR_CANVAS_CONSTANT,
	SR_INTEGRITY_ERROR,
	SR_SPECIAL_TYPE_DENIED
};

typedef U8 STATUS;

#endif