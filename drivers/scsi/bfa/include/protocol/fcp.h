

#ifndef __FCPPROTO_H__
#define __FCPPROTO_H__

#include <protocol/scsi.h>

#pragma pack(1)

enum {
	FCP_RJT		= 0x01000000,	
	FCP_SRR_ACCEPT	= 0x02000000,	
	FCP_SRR		= 0x14000000,	
};


struct fc_srr_s{
	u32	ls_cmd;
	u32        ox_id:16;	
	u32        rx_id:16;	
	u32        ro;		
	u32        r_ctl:8;		
	u32        res:24;
};



#define FCP_CMND_CDB_LEN    16
#define FCP_CMND_LUN_LEN    8

struct fcp_cmnd_s{
	lun_t           lun;		
	u8         crn;		
#ifdef __BIGENDIAN
	u8         resvd:1,
			priority:4,	
			taskattr:3;	
#else
	u8         taskattr:3,	
			priority:4,	
			resvd:1;
#endif
	u8         tm_flags;	
#ifdef __BIGENDIAN
	u8         addl_cdb_len:6,	
			iodir:2;	
#else
	u8         iodir:2,	
			addl_cdb_len:6;	
#endif
	struct scsi_cdb_s      cdb;

	
	u32        fcp_dl;	
};

#define fcp_cmnd_cdb_len(_cmnd) ((_cmnd)->addl_cdb_len * 4 + FCP_CMND_CDB_LEN)
#define fcp_cmnd_fcpdl(_cmnd)	((&(_cmnd)->fcp_dl)[(_cmnd)->addl_cdb_len])


enum fcp_iodir{
	FCP_IODIR_NONE	= 0,
	FCP_IODIR_WRITE = 1,
	FCP_IODIR_READ	= 2,
	FCP_IODIR_RW	= 3,
};


enum {
	FCP_TASK_ATTR_SIMPLE	= 0,
	FCP_TASK_ATTR_HOQ	= 1,
	FCP_TASK_ATTR_ORDERED	= 2,
	FCP_TASK_ATTR_ACA	= 4,
	FCP_TASK_ATTR_UNTAGGED	= 5,	
};


#ifndef BIT
#define BIT(_x)	(1 << (_x))
#endif
enum fcp_tm_cmnd{
	FCP_TM_ABORT_TASK_SET	= BIT(1),
	FCP_TM_CLEAR_TASK_SET	= BIT(2),
	FCP_TM_LUN_RESET	= BIT(4),
	FCP_TM_TARGET_RESET	= BIT(5),	
	FCP_TM_CLEAR_ACA	= BIT(6),
};


struct fcp_xfer_rdy_s{
	u32        data_ro;
	u32        burst_len;
	u32        reserved;
};


enum fcp_residue{
	FCP_NO_RESIDUE = 0,	
	FCP_RESID_OVER = 1,	
	FCP_RESID_UNDER = 2,	
};

enum {
	FCP_RSPINFO_GOOD = 0,
	FCP_RSPINFO_DATALEN_MISMATCH = 1,
	FCP_RSPINFO_CMND_INVALID = 2,
	FCP_RSPINFO_ROLEN_MISMATCH = 3,
	FCP_RSPINFO_TM_NOT_SUPP = 4,
	FCP_RSPINFO_TM_FAILED = 5,
};

struct fcp_rspinfo_s{
	u32        res0:24;
	u32        rsp_code:8;	
	u32        res1;
};

struct fcp_resp_s{
	u32        reserved[2];	
	u16        reserved2;
#ifdef __BIGENDIAN
	u8         reserved3:3;
	u8         fcp_conf_req:1;	
	u8         resid_flags:2;	
	u8         sns_len_valid:1;
	u8         rsp_len_valid:1;
#else
	u8         rsp_len_valid:1;
	u8         sns_len_valid:1;
	u8         resid_flags:2;	
	u8         fcp_conf_req:1;	
	u8         reserved3:3;
#endif
	u8         scsi_status;	
	u32        residue;	
	u32        sns_len;	
	u32        rsp_len;	
};

#define fcp_snslen(__fcprsp)	((__fcprsp)->sns_len_valid ? 		\
					(__fcprsp)->sns_len : 0)
#define fcp_rsplen(__fcprsp)	((__fcprsp)->rsp_len_valid ? 		\
					(__fcprsp)->rsp_len : 0)
#define fcp_rspinfo(__fcprsp)	((struct fcp_rspinfo_s *)((__fcprsp) + 1))
#define fcp_snsinfo(__fcprsp)	(((u8 *)fcp_rspinfo(__fcprsp)) + 	\
						fcp_rsplen(__fcprsp))

struct fcp_cmnd_fr_s{
	struct fchs_s          fchs;
	struct fcp_cmnd_s      fcp;
};

#pragma pack()

#endif
