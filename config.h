#include "build.config.h"

#ifdef __mips64
#define PCICONFIDX_ST (GT64120_ISD_ST+0xcf8)
#define PCICONFIDX_LB (PCICONFIDX_ST+4-1)
#define PCICONFDAT_ST (GT64120_ISD_ST+0xcf8)
#define PCICONFDAT_LB (PCICONFDAT_ST+4-1)
#endif
