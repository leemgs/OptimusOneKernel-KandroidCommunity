
#ifndef _VCD_DDL_UTILS_H_
#define _VCD_DDL_UTILS_H_

#include "vcd_ddl_core.h"
#include "vcd_ddl.h"

#define DDL_INLINE

#define DDL_ALIGN_SIZE(n_size, n_guard_bytes, n_align_mask) \
  (((u32)(n_size) + n_guard_bytes) & n_align_mask)

#define DDL_MALLOC(x)  kmalloc(x, GFP_KERNEL)
#define DDL_FREE(x)   { if ((x)) kfree((x)); (x) = NULL; }

void ddl_pmem_alloc(struct ddl_buf_addr_type *, u32, u32);

void ddl_pmem_free(struct ddl_buf_addr_type);

void ddl_get_core_start_time(u8 codec_type);

void ddl_calc_core_time(u8 codec_type);

void ddl_reset_time_variables(u8 codec_type);

#define DDL_ASSERT(x)
#define DDL_MEMSET(src, value, len) memset((src), (value), (len))
#define DDL_MEMCPY(dest, src, len)  memcpy((dest), (src), (len))

#define DDL_ADDR_IS_ALIGNED(addr, align_bytes) \
(!((u32)(addr) & ((align_bytes) - 1)))

#define VIDC_DDL_QCIF_MBS 99
#define VIDC_DDL_CIF_MBS  396
#define VIDC_DDL_QVGA_MBS 300
#define VIDC_DDL_VGA_MBS  1200
#define VIDC_DDL_WVGA_MBS 1500
#define VIDC_DDL_720P_MBS 3600

#endif
