
#ifndef _VPSS_H
#define _VPSS_H


enum vpss_ccdc_source_sel {
	VPSS_CCDCIN,
	VPSS_HSSIIN
};


enum vpss_clock_sel {
	
	VPSS_CCDC_CLOCK,
	VPSS_IPIPE_CLOCK,
	VPSS_H3A_CLOCK,
	VPSS_CFALD_CLOCK,
	
	VPSS_VENC_CLOCK_SEL,
	VPSS_VPBE_CLOCK,
};


int vpss_select_ccdc_source(enum vpss_ccdc_source_sel src_sel);

int vpss_enable_clock(enum vpss_clock_sel clock_sel, int en);


enum vpss_wbl_sel {
	VPSS_PCR_AEW_WBL_0 = 16,
	VPSS_PCR_AF_WBL_0,
	VPSS_PCR_RSZ4_WBL_0,
	VPSS_PCR_RSZ3_WBL_0,
	VPSS_PCR_RSZ2_WBL_0,
	VPSS_PCR_RSZ1_WBL_0,
	VPSS_PCR_PREV_WBL_0,
	VPSS_PCR_CCDC_WBL_O,
};
int vpss_clear_wbl_overflow(enum vpss_wbl_sel wbl_sel);
#endif
