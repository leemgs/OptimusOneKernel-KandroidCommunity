

#ifndef _LINUX_VIRTIO_PCI_H
#define _LINUX_VIRTIO_PCI_H

#include <linux/virtio_config.h>


#define VIRTIO_PCI_HOST_FEATURES	0


#define VIRTIO_PCI_GUEST_FEATURES	4


#define VIRTIO_PCI_QUEUE_PFN		8


#define VIRTIO_PCI_QUEUE_NUM		12


#define VIRTIO_PCI_QUEUE_SEL		14


#define VIRTIO_PCI_QUEUE_NOTIFY		16


#define VIRTIO_PCI_STATUS		18


#define VIRTIO_PCI_ISR			19


#define VIRTIO_PCI_ISR_CONFIG		0x2



#define VIRTIO_MSI_CONFIG_VECTOR        20

#define VIRTIO_MSI_QUEUE_VECTOR         22

#define VIRTIO_MSI_NO_VECTOR            0xffff


#define VIRTIO_PCI_CONFIG(dev)		((dev)->msix_enabled ? 24 : 20)


#define VIRTIO_PCI_ABI_VERSION		0


#define VIRTIO_PCI_QUEUE_ADDR_SHIFT	12


#define VIRTIO_PCI_VRING_ALIGN		4096
#endif
