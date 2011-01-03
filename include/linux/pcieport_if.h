

#ifndef _PCIEPORT_IF_H_
#define _PCIEPORT_IF_H_


#define PCIE_RC_PORT			4	
#define PCIE_SW_UPSTREAM_PORT		5	
#define PCIE_SW_DOWNSTREAM_PORT		6	
#define PCIE_ANY_PORT			7


#define PCIE_PORT_SERVICE_PME_SHIFT	0	
#define PCIE_PORT_SERVICE_PME		(1 << PCIE_PORT_SERVICE_PME_SHIFT)
#define PCIE_PORT_SERVICE_AER_SHIFT	1	
#define PCIE_PORT_SERVICE_AER		(1 << PCIE_PORT_SERVICE_AER_SHIFT)
#define PCIE_PORT_SERVICE_HP_SHIFT	2	
#define PCIE_PORT_SERVICE_HP		(1 << PCIE_PORT_SERVICE_HP_SHIFT)
#define PCIE_PORT_SERVICE_VC_SHIFT	3	
#define PCIE_PORT_SERVICE_VC		(1 << PCIE_PORT_SERVICE_VC_SHIFT)


#define PCIE_PORT_NO_IRQ		(-1)
#define PCIE_PORT_INTx_MODE		0
#define PCIE_PORT_MSI_MODE		1
#define PCIE_PORT_MSIX_MODE		2

struct pcie_port_data {
	int port_type;		
	int port_irq_mode;	
};

struct pcie_device {
	int 		irq;	    
	struct pci_dev *port;	    
	u32		service;    
	void		*priv_data; 
	struct device	device;     
};
#define to_pcie_device(d) container_of(d, struct pcie_device, device)

static inline void set_service_data(struct pcie_device *dev, void *data)
{
	dev->priv_data = data;
}

static inline void* get_service_data(struct pcie_device *dev)
{
	return dev->priv_data;
}

struct pcie_port_service_driver {
	const char *name;
	int (*probe) (struct pcie_device *dev);
	void (*remove) (struct pcie_device *dev);
	int (*suspend) (struct pcie_device *dev);
	int (*resume) (struct pcie_device *dev);

	
	struct pci_error_handlers *err_handler;

	
	pci_ers_result_t (*reset_link) (struct pci_dev *dev);

	int port_type;  
	u32 service;    

	struct device_driver driver;
};
#define to_service_driver(d) \
	container_of(d, struct pcie_port_service_driver, driver)

extern int pcie_port_service_register(struct pcie_port_service_driver *new);
extern void pcie_port_service_unregister(struct pcie_port_service_driver *new);

#endif 
