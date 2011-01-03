
#ifndef __IBM_NEWEMAC_ZMII_H
#define __IBM_NEWEMAC_ZMII_H


struct zmii_regs {
	u32 fer;		
	u32 ssr;		
	u32 smiirs;		
};


struct zmii_instance {
	struct zmii_regs __iomem	*base;

	
	struct mutex			lock;

	
	int				mode;

	
	int				users;

	
	u32				fer_save;

	
	struct of_device		*ofdev;
};

#ifdef CONFIG_IBM_NEW_EMAC_ZMII

extern int zmii_init(void);
extern void zmii_exit(void);
extern int zmii_attach(struct of_device *ofdev, int input, int *mode);
extern void zmii_detach(struct of_device *ofdev, int input);
extern void zmii_get_mdio(struct of_device *ofdev, int input);
extern void zmii_put_mdio(struct of_device *ofdev, int input);
extern void zmii_set_speed(struct of_device *ofdev, int input, int speed);
extern int zmii_get_regs_len(struct of_device *ocpdev);
extern void *zmii_dump_regs(struct of_device *ofdev, void *buf);

#else
# define zmii_init()		0
# define zmii_exit()		do { } while(0)
# define zmii_attach(x,y,z)	(-ENXIO)
# define zmii_detach(x,y)	do { } while(0)
# define zmii_get_mdio(x,y)	do { } while(0)
# define zmii_put_mdio(x,y)	do { } while(0)
# define zmii_set_speed(x,y,z)	do { } while(0)
# define zmii_get_regs_len(x)	0
# define zmii_dump_regs(x,buf)	(buf)
#endif				

#endif 
