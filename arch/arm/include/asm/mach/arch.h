

#ifndef __ASSEMBLY__

struct tag;
struct meminfo;
struct sys_timer;

struct machine_desc {
	
	unsigned int		nr;		
	unsigned int		phys_io;	
	unsigned int		io_pg_offst;	

	const char		*name;		
	unsigned long		boot_params;	

	unsigned int		video_start;	
	unsigned int		video_end;	

	unsigned int		reserve_lp0 :1;	
	unsigned int		reserve_lp1 :1;	
	unsigned int		reserve_lp2 :1;	
	unsigned int		soft_reboot :1;	
	void			(*fixup)(struct machine_desc *,
					 struct tag *, char **,
					 struct meminfo *);
	void			(*map_io)(void);
	void			(*init_irq)(void);
	struct sys_timer	*timer;		
	void			(*init_machine)(void);
};


#define MACHINE_START(_type,_name)			\
static const struct machine_desc __mach_desc_##_type	\
 __used							\
 __attribute__((__section__(".arch.info.init"))) = {	\
	.nr		= MACH_TYPE_##_type,		\
	.name		= _name,

#define MACHINE_END				\
};

#endif
