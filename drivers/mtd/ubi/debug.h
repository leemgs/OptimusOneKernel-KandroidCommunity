

#ifndef __UBI_DEBUG_H__
#define __UBI_DEBUG_H__

#ifdef CONFIG_MTD_UBI_DEBUG
#include <linux/random.h>

#define dbg_err(fmt, ...) ubi_err(fmt, ##__VA_ARGS__)

#define ubi_assert(expr)  do {                                               \
	if (unlikely(!(expr))) {                                             \
		printk(KERN_CRIT "UBI assert failed in %s at %u (pid %d)\n", \
		       __func__, __LINE__, current->pid);                    \
		ubi_dbg_dump_stack();                                        \
	}                                                                    \
} while (0)

#define dbg_msg(fmt, ...)                                    \
	printk(KERN_DEBUG "UBI DBG (pid %d): %s: " fmt "\n", \
	       current->pid, __func__, ##__VA_ARGS__)

#define ubi_dbg_dump_stack() dump_stack()

struct ubi_ec_hdr;
struct ubi_vid_hdr;
struct ubi_volume;
struct ubi_vtbl_record;
struct ubi_scan_volume;
struct ubi_scan_leb;
struct ubi_mkvol_req;

void ubi_dbg_dump_ec_hdr(const struct ubi_ec_hdr *ec_hdr);
void ubi_dbg_dump_vid_hdr(const struct ubi_vid_hdr *vid_hdr);
void ubi_dbg_dump_vol_info(const struct ubi_volume *vol);
void ubi_dbg_dump_vtbl_record(const struct ubi_vtbl_record *r, int idx);
void ubi_dbg_dump_sv(const struct ubi_scan_volume *sv);
void ubi_dbg_dump_seb(const struct ubi_scan_leb *seb, int type);
void ubi_dbg_dump_mkvol_req(const struct ubi_mkvol_req *req);
void ubi_dbg_dump_flash(struct ubi_device *ubi, int pnum, int offset, int len);

#ifdef CONFIG_MTD_UBI_DEBUG_MSG

#define dbg_gen(fmt, ...) dbg_msg(fmt, ##__VA_ARGS__)
#else
#define dbg_gen(fmt, ...) ({})
#endif

#ifdef CONFIG_MTD_UBI_DEBUG_MSG_EBA

#define dbg_eba(fmt, ...) dbg_msg(fmt, ##__VA_ARGS__)
#else
#define dbg_eba(fmt, ...) ({})
#endif

#ifdef CONFIG_MTD_UBI_DEBUG_MSG_WL

#define dbg_wl(fmt, ...) dbg_msg(fmt, ##__VA_ARGS__)
#else
#define dbg_wl(fmt, ...) ({})
#endif

#ifdef CONFIG_MTD_UBI_DEBUG_MSG_IO

#define dbg_io(fmt, ...) dbg_msg(fmt, ##__VA_ARGS__)
#else
#define dbg_io(fmt, ...) ({})
#endif

#ifdef CONFIG_MTD_UBI_DEBUG_MSG_BLD

#define dbg_bld(fmt, ...) dbg_msg(fmt, ##__VA_ARGS__)
#define UBI_IO_DEBUG 1
#else
#define dbg_bld(fmt, ...) ({})
#define UBI_IO_DEBUG 0
#endif

#ifdef CONFIG_MTD_UBI_DEBUG_PARANOID
int ubi_dbg_check_all_ff(struct ubi_device *ubi, int pnum, int offset, int len);
#else
#define ubi_dbg_check_all_ff(ubi, pnum, offset, len) 0
#endif

#ifdef CONFIG_MTD_UBI_DEBUG_DISABLE_BGT
#define DBG_DISABLE_BGT 1
#else
#define DBG_DISABLE_BGT 0
#endif

#ifdef CONFIG_MTD_UBI_DEBUG_EMULATE_BITFLIPS

static inline int ubi_dbg_is_bitflip(void)
{
	return !(random32() % 200);
}
#else
#define ubi_dbg_is_bitflip() 0
#endif

#ifdef CONFIG_MTD_UBI_DEBUG_EMULATE_WRITE_FAILURES

static inline int ubi_dbg_is_write_failure(void)
{
	return !(random32() % 500);
}
#else
#define ubi_dbg_is_write_failure() 0
#endif

#ifdef CONFIG_MTD_UBI_DEBUG_EMULATE_ERASE_FAILURES

static inline int ubi_dbg_is_erase_failure(void)
{
		return !(random32() % 400);
}
#else
#define ubi_dbg_is_erase_failure() 0
#endif

#else

#define ubi_assert(expr)                 ({})
#define dbg_err(fmt, ...)                ({})
#define dbg_msg(fmt, ...)                ({})
#define dbg_gen(fmt, ...)                ({})
#define dbg_eba(fmt, ...)                ({})
#define dbg_wl(fmt, ...)                 ({})
#define dbg_io(fmt, ...)                 ({})
#define dbg_bld(fmt, ...)                ({})
#define ubi_dbg_dump_stack()             ({})
#define ubi_dbg_dump_ec_hdr(ec_hdr)      ({})
#define ubi_dbg_dump_vid_hdr(vid_hdr)    ({})
#define ubi_dbg_dump_vol_info(vol)       ({})
#define ubi_dbg_dump_vtbl_record(r, idx) ({})
#define ubi_dbg_dump_sv(sv)              ({})
#define ubi_dbg_dump_seb(seb, type)      ({})
#define ubi_dbg_dump_mkvol_req(req)      ({})
#define ubi_dbg_dump_flash(ubi, pnum, offset, len) ({})

#define UBI_IO_DEBUG               0
#define DBG_DISABLE_BGT            0
#define ubi_dbg_is_bitflip()       0
#define ubi_dbg_is_write_failure() 0
#define ubi_dbg_is_erase_failure() 0
#define ubi_dbg_check_all_ff(ubi, pnum, offset, len) 0

#endif 
#endif 
