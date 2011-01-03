#ifndef LINUX_MSI_H
#define LINUX_MSI_H

#include <linux/list.h>

struct msi_msg {
	u32	address_lo;	
	u32	address_hi;	
	u32	data;		
};


struct irq_desc;
extern void mask_msi_irq(unsigned int irq);
extern void unmask_msi_irq(unsigned int irq);
extern void read_msi_msg_desc(struct irq_desc *desc, struct msi_msg *msg);
extern void write_msi_msg_desc(struct irq_desc *desc, struct msi_msg *msg);
extern void read_msi_msg(unsigned int irq, struct msi_msg *msg);
extern void write_msi_msg(unsigned int irq, struct msi_msg *msg);

struct msi_desc {
	struct {
		__u8	is_msix	: 1;
		__u8	multiple: 3;	
		__u8	maskbit	: 1; 	
		__u8	is_64	: 1;	
		__u8	pos;	 	
		__u16	entry_nr;    	
		unsigned default_irq;	
	} msi_attrib;

	u32 masked;			
	unsigned int irq;
	struct list_head list;

	union {
		void __iomem *mask_base;
		u8 mask_pos;
	};
	struct pci_dev *dev;

	
	struct msi_msg msg;
};


int arch_setup_msi_irq(struct pci_dev *dev, struct msi_desc *desc);
void arch_teardown_msi_irq(unsigned int irq);
extern int arch_setup_msi_irqs(struct pci_dev *dev, int nvec, int type);
extern void arch_teardown_msi_irqs(struct pci_dev *dev);
extern int arch_msi_check_device(struct pci_dev* dev, int nvec, int type);


#endif 
