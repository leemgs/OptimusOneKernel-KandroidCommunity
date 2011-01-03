
#ifndef DCA_H
#define DCA_H

#include <linux/pci.h>




void dca_register_notify(struct notifier_block *nb);
void dca_unregister_notify(struct notifier_block *nb);

#define DCA_PROVIDER_ADD     0x0001
#define DCA_PROVIDER_REMOVE  0x0002

struct dca_provider {
	struct list_head	node;
	struct dca_ops		*ops;
	struct device 		*cd;
	int			 id;
};

struct dca_domain {
	struct list_head	node;
	struct list_head	dca_providers;
	struct pci_bus		*pci_rc;
};

struct dca_ops {
	int	(*add_requester)    (struct dca_provider *, struct device *);
	int	(*remove_requester) (struct dca_provider *, struct device *);
	u8	(*get_tag)	    (struct dca_provider *, struct device *,
				     int cpu);
	int	(*dev_managed)      (struct dca_provider *, struct device *);
};

struct dca_provider *alloc_dca_provider(struct dca_ops *ops, int priv_size);
void free_dca_provider(struct dca_provider *dca);
int register_dca_provider(struct dca_provider *dca, struct device *dev);
void unregister_dca_provider(struct dca_provider *dca, struct device *dev);

static inline void *dca_priv(struct dca_provider *dca)
{
	return (void *)dca + sizeof(struct dca_provider);
}


#define DCA_GET_TAG_TWO_ARGS
int dca_add_requester(struct device *dev);
int dca_remove_requester(struct device *dev);
u8 dca_get_tag(int cpu);
u8 dca3_get_tag(struct device *dev, int cpu);


int __init dca_sysfs_init(void);
void __exit dca_sysfs_exit(void);
int dca_sysfs_add_provider(struct dca_provider *dca, struct device *dev);
void dca_sysfs_remove_provider(struct dca_provider *dca);
int dca_sysfs_add_req(struct dca_provider *dca, struct device *dev, int slot);
void dca_sysfs_remove_req(struct dca_provider *dca, int slot);

#endif 
