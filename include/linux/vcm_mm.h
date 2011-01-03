






#define VCM_DEV_ATTR_NON_SH 	(0x00)
#define VCM_DEV_ATTR_SH		(0x04)


#define VCM_DEV_ATTR_NONCACHED		(0x00)
#define VCM_DEV_ATTR_CACHED_WB_WA	(0x01)
#define VCM_DEV_ATTR_CACHED_WB_NWA	(0x02)
#define VCM_DEV_ATTR_CACHED_WT		(0x03)


#define VCM_DEV_DEFAULT_ATTR	(VCM_DEV_ATTR_SH | VCM_DEV_ATTR_NONCACHED)


int set_arm7_pte_attr(unsigned long pt_base, unsigned long va,
		     unsigned long len,	unsigned int attr);



int cpu_set_attr(unsigned long va, unsigned long len, unsigned int attr);



int vcm_setup_tex_classes(void);
