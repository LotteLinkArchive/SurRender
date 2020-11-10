#ifndef SURUMMAN_HEADER_FILE
#define SURUMMAN_HEADER_FILE
#include "env.h"
#include "glbl.h"

#ifdef ENV_ADV_UNIX
#define UMM_METHOD_MMAN
#else
#define UMM_METHOD_MALLOC
#endif

/* See manpage mmap(2) */
X0 *ummap(X0 *addr, SX length, INAT prot, INAT flags, INAT fd, OX offset);
INAT umunmap(X0 *addr, SX length);

#ifdef UMM_METHOD_MMAN
#include <sys/mman.h>
#endif

#ifdef UMM_METHOD_MALLOC
enum fake_mman_enum_prot {
	PROT_EXEC  = 1,
	PROT_READ  = 2,
	PROT_WRITE = 4,
	PROT_NONE  = 8
};

enum fake_mman_enum_flags {
	MAP_SHARED          = 1,
	MAP_SHARED_VALIDATE = 2,
	MAP_PRIVATE         = 4,
	MAP_32BIT           = 8,
	MAP_ANONYMOUS       = 16,
	MAP_DENYWRITE       = 32,
	MAP_EXECUTABLE      = 64,
	MAP_FILE            = 128,
	MAP_FIXED           = 256,
	MAP_FIXED_NOREPLACE = 512,
	MAP_GROWSDOWN       = 1024,
	MAP_HUGETLB         = 2048,
	MAP_HUGE_2MB        = 4096,
	MAP_HUDE_1GB        = 8192,
	MAP_LOCKED          = 16384,
	MAP_NONBLOCK        = 32768,
	MAP_NORESERVE       = 65536,
	MAP_POPULATE        = 131072,
	MAP_STACK           = 262144,
	MAP_SYNC            = 524288,
	MAP_UNINITIALIZED   = 1048576
};

#define MAP_ANON MAP_ANONYMOUS
#endif

#endif
