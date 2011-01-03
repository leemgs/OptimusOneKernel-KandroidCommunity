

#ifndef __HCP_PHYP_H__
#define __HCP_PHYP_H__



struct h_galpa {
	u64 fw_handle;
	
};


struct h_galpas {
	u32 pid;		
	struct h_galpa user;	
	struct h_galpa kernel;	
};

static inline u64 hipz_galpa_load(struct h_galpa galpa, u32 offset)
{
	u64 addr = galpa.fw_handle + offset;
	return *(volatile u64 __force *)addr;
}

static inline void hipz_galpa_store(struct h_galpa galpa, u32 offset, u64 value)
{
	u64 addr = galpa.fw_handle + offset;
	*(volatile u64 __force *)addr = value;
}

int hcp_galpas_ctor(struct h_galpas *galpas, int is_user,
		    u64 paddr_kernel, u64 paddr_user);

int hcp_galpas_dtor(struct h_galpas *galpas);

int hcall_map_page(u64 physaddr, u64 * mapaddr);

int hcall_unmap_page(u64 mapaddr);

#endif
