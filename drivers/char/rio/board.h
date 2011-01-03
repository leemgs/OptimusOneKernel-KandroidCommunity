

#ifndef	__rio_board_h__
#define	__rio_board_h__



#define	DP_SRAM1_SIZE	0x7C00
#define	DP_SRAM2_SIZE	0x0200
#define	DP_SRAM3_SIZE	0x7000
#define	DP_SCRATCH_SIZE	0x1000
#define	DP_PARMMAP_ADDR	0x01FE	
#define	DP_STARTUP_ADDR	0x01F8	


struct s_Ctrl {
	u8 DpCtl;		
	u8 Dp_Unused2_[127];
	u8 DpIntSet;		
	u8 Dp_Unused3_[127];
	u8 DpTpuReset;	
	u8 Dp_Unused4_[127];
	u8 DpIntReset;	
	u8 Dp_Unused5_[127];
};


struct s_Prom {
	u16 DpSlxCode[2];
	u16 DpRev;
	u16 Dp_Unused6_;
	u16 DpUniq[4];
	u16 DpJahre;
	u16 DpWoche;
	u16 DpHwFeature[5];
	u16 DpOemId;
	u16 DpSiggy[16];
};


union u_CtrlProm {		
	struct s_Ctrl DpCtrl;
	struct s_Prom DpProm;
};


struct s_ParmMapS {		
	u8 Dp_Unused8_[DP_PARMMAP_ADDR];
	u16 DpParmMapAd;
};

struct s_StartUpS {
	u8 Dp_Unused9_[DP_STARTUP_ADDR];
	u8 Dp_LongJump[0x4];
	u8 Dp_Unused10_[2];
	u8 Dp_ShortJump[0x2];
};

union u_Sram2ParmMap {		
	u8 DpSramMem[DP_SRAM2_SIZE];
	struct s_ParmMapS DpParmMapS;
	struct s_StartUpS DpStartUpS;
};


struct DpRam {
	u8 DpSram1[DP_SRAM1_SIZE];	
	union u_CtrlProm DpCtrlProm;	
	union u_Sram2ParmMap DpSram2ParmMap;	
	u8 DpScratch[DP_SCRATCH_SIZE];	
	u8 DpSram3[DP_SRAM3_SIZE];	
};

#define	DpControl	DpCtrlProm.DpCtrl.DpCtl
#define	DpSetInt	DpCtrlProm.DpCtrl.DpIntSet
#define	DpResetTpu	DpCtrlProm.DpCtrl.DpTpuReset
#define	DpResetInt	DpCtrlProm.DpCtrl.DpIntReset

#define	DpSlx		DpCtrlProm.DpProm.DpSlxCode
#define	DpRevision	DpCtrlProm.DpProm.DpRev
#define	DpUnique	DpCtrlProm.DpProm.DpUniq
#define	DpYear		DpCtrlProm.DpProm.DpJahre
#define	DpWeek		DpCtrlProm.DpProm.DpWoche
#define	DpSignature	DpCtrlProm.DpProm.DpSiggy

#define	DpParmMapR	DpSram2ParmMap.DpParmMapS.DpParmMapAd
#define	DpSram2		DpSram2ParmMap.DpSramMem

#endif
