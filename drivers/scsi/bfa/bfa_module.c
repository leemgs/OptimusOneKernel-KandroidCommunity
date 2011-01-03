
#include <bfa.h>
#include <defs/bfa_defs_pci.h>
#include <cs/bfa_debug.h>
#include <bfa_iocfc.h>


struct bfa_module_s *hal_mods[] = {
	&hal_mod_sgpg,
	&hal_mod_pport,
	&hal_mod_fcxp,
	&hal_mod_lps,
	&hal_mod_uf,
	&hal_mod_rport,
	&hal_mod_fcpim,
#ifdef BFA_CFG_PBIND
	&hal_mod_pbind,
#endif
	NULL
};


bfa_isr_func_t  bfa_isrs[BFI_MC_MAX] = {
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_pport_isr,		
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_uf_isr,		
	bfa_fcxp_isr,		
	bfa_lps_isr,		
	bfa_rport_isr,		
	bfa_itnim_isr,		
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_ioim_isr,		
	bfa_ioim_good_comp_isr,	
	bfa_tskim_isr,		
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
	bfa_isr_unhandled,	
};


bfa_ioc_mbox_mcfunc_t  bfa_mbox_isrs[BFI_MC_MAX] = {
	NULL,
	NULL,			
	NULL,			
	NULL,		
	NULL,			
	NULL,			
	bfa_iocfc_isr,		
	NULL,
};

