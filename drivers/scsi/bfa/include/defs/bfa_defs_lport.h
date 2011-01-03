

#ifndef __BFA_DEFS_LPORT_H__
#define __BFA_DEFS_LPORT_H__

#include <defs/bfa_defs_types.h>
#include <defs/bfa_defs_port.h>


enum bfa_lport_aen_event {
	BFA_LPORT_AEN_NEW	= 1,	
	BFA_LPORT_AEN_DELETE	= 2,	
	BFA_LPORT_AEN_ONLINE	= 3,	
	BFA_LPORT_AEN_OFFLINE	= 4,	
	BFA_LPORT_AEN_DISCONNECT = 5,	
	BFA_LPORT_AEN_NEW_PROP	= 6,	
	BFA_LPORT_AEN_DELETE_PROP = 7,	
	BFA_LPORT_AEN_NEW_STANDARD = 8,	
	BFA_LPORT_AEN_DELETE_STANDARD = 9,  
	BFA_LPORT_AEN_NPIV_DUP_WWN = 10,    
	BFA_LPORT_AEN_NPIV_FABRIC_MAX = 11, 
	BFA_LPORT_AEN_NPIV_UNKNOWN = 12, 
};


struct bfa_lport_aen_data_s {
	u16        vf_id;	
	u16        rsvd;
	enum bfa_port_role roles;	
	wwn_t           ppwwn;	
	wwn_t           lpwwn;	
};

#endif 
