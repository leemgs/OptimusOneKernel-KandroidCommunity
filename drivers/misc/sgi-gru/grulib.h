

#ifndef __GRULIB_H__
#define __GRULIB_H__

#define GRU_BASENAME		"gru"
#define GRU_FULLNAME		"/dev/gru"
#define GRU_IOCTL_NUM 		 'G'


#define GRU_MAX_OPEN_CONTEXTS		32


#define GRU_CREATE_CONTEXT		_IOWR(GRU_IOCTL_NUM, 1, void *)


#define GRU_SET_CONTEXT_OPTION		_IOWR(GRU_IOCTL_NUM, 4, void *)


#define GRU_USER_GET_EXCEPTION_DETAIL	_IOWR(GRU_IOCTL_NUM, 6, void *)


#define GRU_USER_CALL_OS		_IOWR(GRU_IOCTL_NUM, 8, void *)


#define GRU_USER_UNLOAD_CONTEXT		_IOWR(GRU_IOCTL_NUM, 9, void *)


#define GRU_DUMP_CHIPLET_STATE		_IOWR(GRU_IOCTL_NUM, 11, void *)


#define GRU_GET_GSEG_STATISTICS		_IOWR(GRU_IOCTL_NUM, 12, void *)


#define GRU_USER_FLUSH_TLB		_IOWR(GRU_IOCTL_NUM, 50, void *)


#define GRU_GET_CONFIG_INFO		_IOWR(GRU_IOCTL_NUM, 51, void *)


#define GRU_KTEST			_IOWR(GRU_IOCTL_NUM, 52, void *)

#define CONTEXT_WINDOW_BYTES(th)        (GRU_GSEG_PAGESIZE * (th))
#define THREAD_POINTER(p, th)		(p + GRU_GSEG_PAGESIZE * (th))
#define GSEG_START(cb)			((void *)((unsigned long)(cb) & ~(GRU_GSEG_PAGESIZE - 1)))


struct gts_statistics {
	unsigned long	fmm_tlbdropin;
	unsigned long	upm_tlbdropin;
	unsigned long	context_stolen;
};

struct gru_get_gseg_statistics_req {
	unsigned long		gseg;
	struct gts_statistics	stats;
};


struct gru_create_context_req {
	unsigned long		gseg;
	unsigned int		data_segment_bytes;
	unsigned int		control_blocks;
	unsigned int		maximum_thread_count;
	unsigned int		options;
};


struct gru_unload_context_req {
	unsigned long	gseg;
};


enum {sco_gseg_owner, sco_cch_req_slice};
struct gru_set_context_option_req {
	unsigned long	gseg;
	int		op;
	unsigned long	val1;
};


struct gru_flush_tlb_req {
	unsigned long	gseg;
	unsigned long	vaddr;
	size_t		len;
};


enum {dcs_pid, dcs_gid};
struct gru_dump_chiplet_state_req {
	unsigned int	op;
	unsigned int	gid;
	int		ctxnum;
	char		data_opt;
	char		lock_cch;
	pid_t		pid;
	void		*buf;
	size_t		buflen;
	
	unsigned int	num_contexts;
};

#define GRU_DUMP_MAGIC	0x3474ab6c
struct gru_dump_context_header {
	unsigned int	magic;
	unsigned int	gid;
	unsigned char	ctxnum;
	unsigned char	cbrcnt;
	unsigned char	dsrcnt;
	pid_t		pid;
	unsigned long	vaddr;
	int		cch_locked;
	unsigned long	data[0];
};


struct gru_config_info {
	int		cpus;
	int		blades;
	int		nodes;
	int		chiplets;
	int		fill[16];
};

#endif 
