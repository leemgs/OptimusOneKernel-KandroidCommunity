

#ifndef _LINUX_SS_H
#define _LINUX_SS_H

#include <linux/device.h>
#include <linux/sched.h>	
#include <linux/mutex.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#ifdef CONFIG_CARDBUS
#include <linux/pci.h>
#endif


#define SS_WRPROT	0x0001
#define SS_CARDLOCK	0x0002
#define SS_EJECTION	0x0004
#define SS_INSERTION	0x0008
#define SS_BATDEAD	0x0010
#define SS_BATWARN	0x0020
#define SS_READY	0x0040
#define SS_DETECT	0x0080
#define SS_POWERON	0x0100
#define SS_GPI		0x0200
#define SS_STSCHG	0x0400
#define SS_CARDBUS	0x0800
#define SS_3VCARD	0x1000
#define SS_XVCARD	0x2000
#define SS_PENDING	0x4000
#define SS_ZVCARD	0x8000


#define SS_CAP_PAGE_REGS	0x0001
#define SS_CAP_VIRTUAL_BUS	0x0002
#define SS_CAP_MEM_ALIGN	0x0004
#define SS_CAP_STATIC_MAP	0x0008
#define SS_CAP_PCCARD		0x4000
#define SS_CAP_CARDBUS		0x8000


typedef struct socket_state_t {
	u_int	flags;
	u_int	csc_mask;
	u_char	Vcc, Vpp;
	u_char	io_irq;
} socket_state_t;

extern socket_state_t dead_socket;


#define SS_PWR_AUTO	0x0010
#define SS_IOCARD	0x0020
#define SS_RESET	0x0040
#define SS_DMA_MODE	0x0080
#define SS_SPKR_ENA	0x0100
#define SS_OUTPUT_ENA	0x0200


#define MAP_ACTIVE	0x01
#define MAP_16BIT	0x02
#define MAP_AUTOSZ	0x04
#define MAP_0WS		0x08
#define MAP_WRPROT	0x10
#define MAP_ATTRIB	0x20
#define MAP_USE_WAIT	0x40
#define MAP_PREFETCH	0x80


#define MAP_IOSPACE	0x20


#define HOOK_POWER_PRE	0x01
#define HOOK_POWER_POST	0x02

typedef struct pccard_io_map {
	u_char	map;
	u_char	flags;
	u_short	speed;
	phys_addr_t start, stop;
} pccard_io_map;

typedef struct pccard_mem_map {
	u_char		map;
	u_char		flags;
	u_short		speed;
	phys_addr_t	static_start;
	u_int		card_start;
	struct resource	*res;
} pccard_mem_map;

typedef struct io_window_t {
	u_int			InUse, Config;
	struct resource		*res;
} io_window_t;

#define WINDOW_MAGIC	0xB35C
typedef struct window_t {
	u_short			magic;
	u_short			index;
	struct pcmcia_device	*handle;
	struct pcmcia_socket 	*sock;
	pccard_mem_map		ctl;
} window_t;


#define MAX_IO_WIN 2


#define MAX_WIN 4



struct pcmcia_socket;
struct pccard_resource_ops;
struct config_t;
struct pcmcia_callback;
struct user_info_t;

struct pccard_operations {
	int (*init)(struct pcmcia_socket *s);
	int (*suspend)(struct pcmcia_socket *s);
	int (*get_status)(struct pcmcia_socket *s, u_int *value);
	int (*set_socket)(struct pcmcia_socket *s, socket_state_t *state);
	int (*set_io_map)(struct pcmcia_socket *s, struct pccard_io_map *io);
	int (*set_mem_map)(struct pcmcia_socket *s, struct pccard_mem_map *mem);
};

struct pcmcia_socket {
	struct module			*owner;
	spinlock_t			lock;
	socket_state_t			socket;
	u_int				state;
	u_short				functions;
	u_short				lock_count;
	pccard_mem_map			cis_mem;
	void __iomem 			*cis_virt;
	struct {
		u_int			AssignedIRQ;
		u_int			Config;
	} irq;
	io_window_t			io[MAX_IO_WIN];
	window_t			win[MAX_WIN];
	struct list_head		cis_cache;
	size_t				fake_cis_len;
	u8				*fake_cis;

	struct list_head		socket_list;
	struct completion		socket_released;

 	
	unsigned int			sock;		


	
	u_int				features;
	u_int				irq_mask;
	u_int				map_size;
	u_int				io_offset;
	u_char				pci_irq;
	struct pci_dev *		cb_dev;


	
	u8				resource_setup_done:1;

	
	u8				resource_setup_old:1;
	u8				resource_setup_new:1;

	u8				reserved:5;

	
	struct pccard_operations *	ops;
	struct pccard_resource_ops *	resource_ops;
	void *				resource_data;

	
	void 				(*zoom_video)(struct pcmcia_socket *,
						      int);

	
	int (*power_hook)(struct pcmcia_socket *sock, int operation);

	
#ifdef CONFIG_CARDBUS
	void (*tune_bridge)(struct pcmcia_socket *sock, struct pci_bus *bus);
#endif

	
	struct task_struct		*thread;
	struct completion		thread_done;
	unsigned int			thread_events;
	
	struct mutex			skt_mutex;
	
	spinlock_t			thread_lock;

	
	struct pcmcia_callback		*callback;

#if defined(CONFIG_PCMCIA) || defined(CONFIG_PCMCIA_MODULE)
	
	struct list_head		devices_list;

	
	u8				device_count;

	
	struct {
		
		u8			present:1;
		
		u8			busy:1;
		
		u8			dead:1;
		
		u8			device_add_pending:1;
		
		u8			mfc_pfc:1;

		u8			reserved:3;
	} pcmcia_state;


	
	struct work_struct		device_add;

#ifdef CONFIG_PCMCIA_IOCTL
	struct user_info_t		*user;
	wait_queue_head_t		queue;
#endif 
#endif 

	
#ifdef CONFIG_CARDBUS
	struct resource *		cb_cis_res;
	void __iomem			*cb_cis_virt;
#endif 

	
	struct device			dev;
	
	void				*driver_data;
	
	int				resume_status;
};



extern struct pccard_resource_ops pccard_static_ops;
extern struct pccard_resource_ops pccard_iodyn_ops;
extern struct pccard_resource_ops pccard_nonstatic_ops;


extern int pcmcia_socket_dev_suspend(struct device *dev);
extern void pcmcia_socket_dev_early_resume(struct device *dev);
extern void pcmcia_socket_dev_late_resume(struct device *dev);
extern int pcmcia_socket_dev_resume(struct device *dev);


extern void pcmcia_parse_events(struct pcmcia_socket *socket,
				unsigned int events);


extern int pcmcia_register_socket(struct pcmcia_socket *socket);
extern void pcmcia_unregister_socket(struct pcmcia_socket *socket);


#endif 
