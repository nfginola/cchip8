#ifndef _UTILS
#define _UTILS
#include "types.h"

#define DBG_PRINT 1

#if DBG_PRINT
#define d_printf(a) printf a
#else
#define d_printf(a) (void)0
#endif

void *read_bin_file(char *fname, u32 *out_size);
u64 time_in_ms();

#endif
