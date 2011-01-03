

#ifndef __BFA_MODULES_PRIV_H__
#define __BFA_MODULES_PRIV_H__

#include "bfa_uf_priv.h"
#include "bfa_port_priv.h"
#include "bfa_rport_priv.h"
#include "bfa_fcxp_priv.h"
#include "bfa_lps_priv.h"
#include "bfa_fcpim_priv.h"
#include <cee/bfa_cee.h>
#include <port/bfa_port.h>


struct bfa_modules_s {
	struct bfa_pport_s	pport;	
	struct bfa_fcxp_mod_s fcxp_mod; 
	struct bfa_lps_mod_s lps_mod;   
	struct bfa_uf_mod_s uf_mod;	
	struct bfa_rport_mod_s rport_mod; 
	struct bfa_fcpim_mod_s fcpim_mod; 
	struct bfa_sgpg_mod_s sgpg_mod; 
	struct bfa_cee_s cee;   	
	struct bfa_port_s port;		
};

#endif 
