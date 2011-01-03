

#include <linux/module.h>
#include <linux/kernel.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/ss.h>
#include <pcmcia/cs.h>
#include "cs_internal.h"


int pcmcia_validate_mem(struct pcmcia_socket *s)
{
	if (s->resource_ops->validate_mem)
		return s->resource_ops->validate_mem(s);
	
	return 0;
}
EXPORT_SYMBOL(pcmcia_validate_mem);

int pcmcia_adjust_io_region(struct resource *res, unsigned long r_start,
		     unsigned long r_end, struct pcmcia_socket *s)
{
	if (s->resource_ops->adjust_io_region)
		return s->resource_ops->adjust_io_region(res, r_start, r_end, s);
	return -ENOMEM;
}
EXPORT_SYMBOL(pcmcia_adjust_io_region);

struct resource *pcmcia_find_io_region(unsigned long base, int num,
		   unsigned long align, struct pcmcia_socket *s)
{
	if (s->resource_ops->find_io)
		return s->resource_ops->find_io(base, num, align, s);
	return NULL;
}
EXPORT_SYMBOL(pcmcia_find_io_region);

struct resource *pcmcia_find_mem_region(u_long base, u_long num, u_long align,
				 int low, struct pcmcia_socket *s)
{
	if (s->resource_ops->find_mem)
		return s->resource_ops->find_mem(base, num, align, low, s);
	return NULL;
}
EXPORT_SYMBOL(pcmcia_find_mem_region);

void release_resource_db(struct pcmcia_socket *s)
{
	if (s->resource_ops->exit)
		s->resource_ops->exit(s);
}


static int static_init(struct pcmcia_socket *s)
{
	unsigned long flags;

	

	spin_lock_irqsave(&s->lock, flags);
	s->resource_setup_done = 1;
	spin_unlock_irqrestore(&s->lock, flags);

	return 0;
}


struct pccard_resource_ops pccard_static_ops = {
	.validate_mem = NULL,
	.adjust_io_region = NULL,
	.find_io = NULL,
	.find_mem = NULL,
	.add_io = NULL,
	.add_mem = NULL,
	.init = static_init,
	.exit = NULL,
};
EXPORT_SYMBOL(pccard_static_ops);


#ifdef CONFIG_PCCARD_IODYN

static struct resource *
make_resource(unsigned long b, unsigned long n, int flags, char *name)
{
	struct resource *res = kzalloc(sizeof(*res), GFP_KERNEL);

	if (res) {
		res->name = name;
		res->start = b;
		res->end = b + n - 1;
		res->flags = flags;
	}
	return res;
}

struct pcmcia_align_data {
	unsigned long	mask;
	unsigned long	offset;
};

static void pcmcia_align(void *align_data, struct resource *res,
			unsigned long size, unsigned long align)
{
	struct pcmcia_align_data *data = align_data;
	unsigned long start;

	start = (res->start & ~data->mask) + data->offset;
	if (start < res->start)
		start += data->mask + 1;
	res->start = start;

#ifdef CONFIG_X86
        if (res->flags & IORESOURCE_IO) {
                if (start & 0x300) {
                        start = (start + 0x3ff) & ~0x3ff;
                        res->start = start;
                }
        }
#endif

#ifdef CONFIG_M68K
        if (res->flags & IORESOURCE_IO) {
		if ((res->start + size - 1) >= 1024)
			res->start = res->end;
	}
#endif
}


static int iodyn_adjust_io_region(struct resource *res, unsigned long r_start,
				      unsigned long r_end, struct pcmcia_socket *s)
{
	return adjust_resource(res, r_start, r_end - r_start + 1);
}


static struct resource *iodyn_find_io_region(unsigned long base, int num,
		unsigned long align, struct pcmcia_socket *s)
{
	struct resource *res = make_resource(0, num, IORESOURCE_IO,
					     dev_name(&s->dev));
	struct pcmcia_align_data data;
	unsigned long min = base;
	int ret;

	if (align == 0)
		align = 0x10000;

	data.mask = align - 1;
	data.offset = base & data.mask;

#ifdef CONFIG_PCI
	if (s->cb_dev) {
		ret = pci_bus_alloc_resource(s->cb_dev->bus, res, num, 1,
					     min, 0, pcmcia_align, &data);
	} else
#endif
		ret = allocate_resource(&ioport_resource, res, num, min, ~0UL,
					1, pcmcia_align, &data);

	if (ret != 0) {
		kfree(res);
		res = NULL;
	}
	return res;
}

struct pccard_resource_ops pccard_iodyn_ops = {
	.validate_mem = NULL,
	.adjust_io_region = iodyn_adjust_io_region,
	.find_io = iodyn_find_io_region,
	.find_mem = NULL,
	.add_io = NULL,
	.add_mem = NULL,
	.init = static_init,
	.exit = NULL,
};
EXPORT_SYMBOL(pccard_iodyn_ops);

#endif 
