

#define	I82586_MEMZ	(64 * 1024)

#define	I82586_SCP_ADDR	(I82586_MEMZ - sizeof(scp_t))

#define	ADDR_LEN	6
#define	I82586NULL	0xFFFF

#define	toff(t,p,f) 	(unsigned short)((void *)(&((t *)((void *)0 + (p)))->f) - (void *)0)


typedef struct scp_t	scp_t;
struct scp_t
{
	unsigned short	scp_sysbus;	
#define		SCP_SY_16BBUS	(0x0 << 0)	
#define		SCP_SY_8BBUS	(0x1 << 0)	
	unsigned short	scp_junk[2];	
	unsigned short	scp_iscpl;	
	unsigned short	scp_iscph;	
};


typedef struct iscp_t	iscp_t;
struct iscp_t
{
	unsigned short	iscp_busy;	
					
	unsigned short	iscp_offset;	
	unsigned short	iscp_basel;	
	unsigned short	iscp_baseh;	
};


typedef struct scb_t	scb_t;
struct scb_t
{
	unsigned short	scb_status;	
#define		SCB_ST_INT	(0xF << 12)	
#define		SCB_ST_CX	(0x1 << 15)	
#define		SCB_ST_FR	(0x1 << 14)	
#define		SCB_ST_CNA	(0x1 << 13)	
#define		SCB_ST_RNR	(0x1 << 12)	
#define		SCB_ST_JUNK0	(0x1 << 11)	
#define		SCB_ST_CUS	(0x7 <<  8)	
#define			SCB_ST_CUS_IDLE	(0 << 8)	
#define			SCB_ST_CUS_SUSP	(1 << 8)	
#define			SCB_ST_CUS_ACTV	(2 << 8)	
#define		SCB_ST_JUNK1	(0x1 <<  7)	
#define		SCB_ST_RUS	(0x7 <<  4)	
#define			SCB_ST_RUS_IDLE	(0 << 4)	
#define			SCB_ST_RUS_SUSP	(1 << 4)	
#define			SCB_ST_RUS_NRES	(2 << 4)	
#define			SCB_ST_RUS_RDY	(4 << 4)	
	unsigned short	scb_command;	
#define		SCB_CMD_ACK_CX	(0x1 << 15)	
#define		SCB_CMD_ACK_FR	(0x1 << 14)	
#define		SCB_CMD_ACK_CNA	(0x1 << 13)	
#define		SCB_CMD_ACK_RNR	(0x1 << 12)	
#define		SCB_CMD_JUNKX	(0x1 << 11)	
#define		SCB_CMD_CUC	(0x7 <<  8)	
#define			SCB_CMD_CUC_NOP	(0 << 8)	
#define			SCB_CMD_CUC_GO	(1 << 8)	
#define			SCB_CMD_CUC_RES	(2 << 8)	
#define			SCB_CMD_CUC_SUS	(3 << 8)	
#define			SCB_CMD_CUC_ABT	(4 << 8)	
#define		SCB_CMD_RESET	(0x1 <<  7)	
#define		SCB_CMD_RUC	(0x7 <<  4)	
#define			SCB_CMD_RUC_NOP	(0 << 4)	
#define			SCB_CMD_RUC_GO	(1 << 4)	
#define			SCB_CMD_RUC_RES	(2 << 4)	
#define			SCB_CMD_RUC_SUS	(3 << 4)	
#define			SCB_CMD_RUC_ABT	(4 << 4)	
	unsigned short	scb_cbl_offset;	
					
	unsigned short	scb_rfa_offset;	
					
					
	unsigned short	scb_crcerrs;	
					
	unsigned short	scb_alnerrs;	
					
	unsigned short	scb_rscerrs;	
	unsigned short	scb_ovrnerrs;	
};

#define	scboff(p,f) 	toff(scb_t, p, f)


typedef enum acmd_e	acmd_e;
enum acmd_e
{
	acmd_nop	= 0,	
	acmd_ia_setup	= 1,	
				
	acmd_configure	= 2,	
	acmd_mc_setup	= 3,	
				
	acmd_transmit	= 4,	
	acmd_tdr	= 5,	
				
	acmd_dump	= 6,	
	acmd_diagnose	= 7,	
};


typedef struct ach_t	ach_t;
struct ach_t
{
	unsigned short	ac_status;		
#define		AC_SFLD_C	(0x1 << 15)	
#define		AC_SFLD_B	(0x1 << 14)	
#define		AC_SFLD_OK	(0x1 << 13)	
#define		AC_SFLD_A	(0x1 << 12)	
#define		AC_SFLD_FAIL	(0x1 << 11)	
#define		AC_SFLD_S10	(0x1 << 10)	
						
#define		AC_SFLD_S9	(0x1 <<  9)	
						
#define		AC_SFLD_S8	(0x1 <<  8)	
						
#define		AC_SFLD_S7	(0x1 <<  7)	
						
#define		AC_SFLD_S6	(0x1 <<  6)	
						
#define		AC_SFLD_S5	(0x1 <<  5)	
						
#define		AC_SFLD_MAXCOL	(0xF <<  0)	
	unsigned short	ac_command;		
#define		AC_CFLD_EL	(0x1 << 15)	
#define		AC_CFLD_S	(0x1 << 14)	
#define		AC_CFLD_I	(0x1 << 13)	
#define		AC_CFLD_CMD	(0x7 <<  0)	
	unsigned short	ac_link;		
};

#define	acoff(p,f) 	toff(ach_t, p, f)


typedef struct ac_nop_t	ac_nop_t;
struct ac_nop_t
{
	ach_t	nop_h;
};


typedef struct ac_ias_t	ac_ias_t;
struct ac_ias_t
{
	ach_t		ias_h;
	unsigned char	ias_addr[ADDR_LEN]; 
};


typedef struct ac_cfg_t	ac_cfg_t;
struct ac_cfg_t
{
	ach_t		cfg_h;
	unsigned char	cfg_byte_cnt;	
#define	AC_CFG_BYTE_CNT(v)	(((v) & 0xF) << 0)
	unsigned char	cfg_fifolim;	
#define	AC_CFG_FIFOLIM(v)	(((v) & 0xF) << 0)
	unsigned char	cfg_byte8;
#define	AC_CFG_SAV_BF(v) 	(((v) & 0x1) << 7)	
#define	AC_CFG_SRDY(v) 		(((v) & 0x1) << 6)	
							
	unsigned char	cfg_byte9;
#define	AC_CFG_ELPBCK(v)	(((v) & 0x1) << 7)	
#define	AC_CFG_ILPBCK(v)	(((v) & 0x1) << 6)	
#define	AC_CFG_PRELEN(v)	(((v) & 0x3) << 4)	
#define		AC_CFG_PLEN_2		0		
#define		AC_CFG_PLEN_4		1		
#define		AC_CFG_PLEN_8		2		
#define		AC_CFG_PLEN_16		3		
#define	AC_CFG_ALOC(v)		(((v) & 0x1) << 3)	
							
#define	AC_CFG_ADDRLEN(v)	(((v) & 0x7) << 0)	
	unsigned char	cfg_byte10;
#define	AC_CFG_BOFMET(v)	(((v) & 0x1) << 7)	
							
#define	AC_CFG_ACR(v)		(((v) & 0x7) << 4)	
#define	AC_CFG_LINPRIO(v)	(((v) & 0x7) << 0)	
	unsigned char	cfg_ifs;	
	unsigned char	cfg_slotl;	
	unsigned char	cfg_byte13;
#define	AC_CFG_RETRYNUM(v)	(((v) & 0xF) << 4)	
#define	AC_CFG_SLTTMHI(v)	(((v) & 0x7) << 0)	
	unsigned char	cfg_byte14;
#define	AC_CFG_FLGPAD(v)	(((v) & 0x1) << 7)	
#define	AC_CFG_BTSTF(v)		(((v) & 0x1) << 6)	
#define	AC_CFG_CRC16(v)		(((v) & 0x1) << 5)	
#define	AC_CFG_NCRC(v)		(((v) & 0x1) << 4)	
#define	AC_CFG_TNCRS(v)		(((v) & 0x1) << 3)	
#define	AC_CFG_MANCH(v)		(((v) & 0x1) << 2)	
#define	AC_CFG_BCDIS(v)		(((v) & 0x1) << 1)	
#define	AC_CFG_PRM(v)		(((v) & 0x1) << 0)	
	unsigned char	cfg_byte15;
#define	AC_CFG_ICDS(v)		(((v) & 0x1) << 7)	
							
#define	AC_CFG_CDTF(v)		(((v) & 0x7) << 4)	
							
#define	AC_CFG_ICSS(v)		(((v) & 0x1) << 3)	
							
#define	AC_CFG_CSTF(v)		(((v) & 0x7) << 0)	
							
	unsigned short	cfg_min_frm_len;
#define	AC_CFG_MNFRM(v)		(((v) & 0xFF) << 0)	
};


typedef struct ac_mcs_t	ac_mcs_t;
struct ac_mcs_t
{
	ach_t		mcs_h;
	unsigned short	mcs_cnt;	
#if 0
	unsigned char	mcs_data[ADDR_LEN]; 
	...
#endif
};

#define I82586_MAX_MULTICAST_ADDRESSES	128	


typedef struct ac_tx_t	ac_tx_t;
struct ac_tx_t
{
	ach_t		tx_h;
	unsigned short	tx_tbd_offset;	
#if	0
Linux packets are passed down with the destination MAC address
and length/type field already prepended to the data,
so we do not need to insert it.  Consistent with this
we must also set the AC_CFG_ALOC(..) flag during the
ac_cfg_t action command.
	unsigned char	tx_addr[ADDR_LEN]; 
	unsigned short	tx_length;	
#endif	
};


typedef struct ac_tdr_t	ac_tdr_t;
struct ac_tdr_t
{
	ach_t		tdr_h;
	unsigned short	tdr_result;	
#define		AC_TDR_LNK_OK	(0x1 << 15)	
#define		AC_TDR_XCVR_PRB	(0x1 << 14)	
#define		AC_TDR_ET_OPN	(0x1 << 13)	
#define		AC_TDR_ET_SRT	(0x1 << 12)	
#define		AC_TDR_TIME	(0x7FF << 0)	
						
						
};


typedef struct ac_dmp_t	ac_dmp_t;
struct ac_dmp_t
{
	ach_t		dmp_h;
	unsigned short	dmp_offset;	
};


#define	DUMPBYTES	170


typedef struct ac_dgn_t	ac_dgn_t;
struct ac_dgn_t
{
	ach_t		dgn_h;
};


typedef struct tbd_t	tbd_t;
struct tbd_t
{
	unsigned short	tbd_status;		
#define		TBD_STATUS_EOF	(0x1 << 15)	
						
#define		TBD_STATUS_ACNT	(0x3FFF << 0)	
						
	unsigned short	tbd_next_bd_offset;	
	unsigned short	tbd_bufl;		
	unsigned short	tbd_bufh;		
};


typedef struct rbd_t	rbd_t;
struct rbd_t
{
	unsigned short	rbd_status;		
#define		RBD_STATUS_EOF	(0x1 << 15)	
						
#define		RBD_STATUS_F	(0x1 << 14)	
#define		RBD_STATUS_ACNT	(0x3FFF << 0)	
						
	unsigned short	rbd_next_rbd_offset;	
	unsigned short	rbd_bufl;		
	unsigned short	rbd_bufh;		
	unsigned short	rbd_el_size;		
#define		RBD_EL	(0x1 << 15)		
						
#define		RBD_SIZE (0x3FFF << 0)		
						
};

#define	rbdoff(p,f) 	toff(rbd_t, p, f)


typedef struct fd_t	fd_t;
struct fd_t
{
	unsigned short	fd_status;		
#define		FD_STATUS_C	(0x1 << 15)	
#define		FD_STATUS_B	(0x1 << 14)	
#define		FD_STATUS_OK	(0x1 << 13)	
#define		FD_STATUS_S11	(0x1 << 11)	
#define		FD_STATUS_S10	(0x1 << 10)	
#define		FD_STATUS_S9	(0x1 <<  9)	
#define		FD_STATUS_S8	(0x1 <<  8)	
#define		FD_STATUS_S7	(0x1 <<  7)	
#define		FD_STATUS_S6	(0x1 <<  6)	
	unsigned short	fd_command;		
#define		FD_COMMAND_EL	(0x1 << 15)	
#define		FD_COMMAND_S	(0x1 << 14)	
	unsigned short	fd_link_offset;		
	unsigned short	fd_rbd_offset;		
						
						
#if	0
I think the rest is unused since we
have set AC_CFG_ALOC(..).  However, just
in case, we leave the space.
#endif	
	unsigned char	fd_dest[ADDR_LEN];	
						
	unsigned char	fd_src[ADDR_LEN];	
						
	unsigned short	fd_length;		
						
};

#define	fdoff(p,f) 	toff(fd_t, p, f)


