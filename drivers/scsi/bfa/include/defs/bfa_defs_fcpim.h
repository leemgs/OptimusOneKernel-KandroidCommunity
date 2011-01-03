
#ifndef __BFA_DEFS_FCPIM_H__
#define __BFA_DEFS_FCPIM_H__

struct bfa_fcpim_stats_s {
	u32        total_ios;	
	u32        qresumes;	
	u32        no_iotags;	
	u32        io_aborts;	
	u32        no_tskims;	
	u32        iocomp_ok;	
	u32        iocomp_underrun;	
	u32        iocomp_overrun;	
	u32        iocomp_aborted;	
	u32        iocomp_timedout;	
	u32        iocom_nexus_abort;	
	u32        iocom_proto_err;	
	u32        iocom_dif_err;	
	u32        iocom_tm_abort;	
	u32        iocom_sqer_needed;	
	u32        iocom_res_free;	
	u32        iocomp_scsierr;	
	u32        iocom_hostabrts;	
	u32        iocom_utags;	
	u32        io_cleanups;	
	u32        io_tmaborts;	
	u32        rsvd;
};
#endif 
