
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/math64.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>





#define OP_READ_CONTINUOUS	0xE8
#define OP_READ_PAGE		0xD2


#define OP_READ_STATUS		0xD7	


#define OP_READ_BUFFER1		0xD4	
#define OP_READ_BUFFER2		0xD6	
#define OP_WRITE_BUFFER1	0x84	
#define OP_WRITE_BUFFER2	0x87	


#define OP_ERASE_PAGE		0x81
#define OP_ERASE_BLOCK		0x50


#define OP_TRANSFER_BUF1	0x53
#define OP_TRANSFER_BUF2	0x55
#define OP_MREAD_BUFFER1	0xD4
#define OP_MREAD_BUFFER2	0xD6
#define OP_MWERASE_BUFFER1	0x83
#define OP_MWERASE_BUFFER2	0x86
#define OP_MWRITE_BUFFER1	0x88	
#define OP_MWRITE_BUFFER2	0x89	


#define OP_PROGRAM_VIA_BUF1	0x82
#define OP_PROGRAM_VIA_BUF2	0x85


#define OP_COMPARE_BUF1		0x60
#define OP_COMPARE_BUF2		0x61


#define OP_REWRITE_VIA_BUF1	0x58
#define OP_REWRITE_VIA_BUF2	0x59


#define OP_READ_ID		0x9F
#define OP_READ_SECURITY	0x77
#define OP_WRITE_SECURITY_REVC	0x9A
#define OP_WRITE_SECURITY	0x9B	


struct dataflash {
	uint8_t			command[4];
	char			name[24];

	unsigned		partitioned:1;

	unsigned short		page_offset;	
	unsigned int		page_size;	

	struct mutex		lock;
	struct spi_device	*spi;

	struct mtd_info		mtd;
};




static inline int dataflash_status(struct spi_device *spi)
{
	
	return spi_w8r8(spi, OP_READ_STATUS);
}


static int dataflash_waitready(struct spi_device *spi)
{
	int	status;

	for (;;) {
		status = dataflash_status(spi);
		if (status < 0) {
			DEBUG(MTD_DEBUG_LEVEL1, "%s: status %d?\n",
					dev_name(&spi->dev), status);
			status = 0;
		}

		if (status & (1 << 7))	
			return status;

		msleep(3);
	}
}




static int dataflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct dataflash	*priv = (struct dataflash *)mtd->priv;
	struct spi_device	*spi = priv->spi;
	struct spi_transfer	x = { .tx_dma = 0, };
	struct spi_message	msg;
	unsigned		blocksize = priv->page_size << 3;
	uint8_t			*command;
	uint32_t		rem;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: erase addr=0x%llx len 0x%llx\n",
	      dev_name(&spi->dev), (long long)instr->addr,
	      (long long)instr->len);

	
	if (instr->addr + instr->len > mtd->size)
		return -EINVAL;
	div_u64_rem(instr->len, priv->page_size, &rem);
	if (rem)
		return -EINVAL;
	div_u64_rem(instr->addr, priv->page_size, &rem);
	if (rem)
		return -EINVAL;

	spi_message_init(&msg);

	x.tx_buf = command = priv->command;
	x.len = 4;
	spi_message_add_tail(&x, &msg);

	mutex_lock(&priv->lock);
	while (instr->len > 0) {
		unsigned int	pageaddr;
		int		status;
		int		do_block;

		
		pageaddr = div_u64(instr->addr, priv->page_size);
		do_block = (pageaddr & 0x7) == 0 && instr->len >= blocksize;
		pageaddr = pageaddr << priv->page_offset;

		command[0] = do_block ? OP_ERASE_BLOCK : OP_ERASE_PAGE;
		command[1] = (uint8_t)(pageaddr >> 16);
		command[2] = (uint8_t)(pageaddr >> 8);
		command[3] = 0;

		DEBUG(MTD_DEBUG_LEVEL3, "ERASE %s: (%x) %x %x %x [%i]\n",
			do_block ? "block" : "page",
			command[0], command[1], command[2], command[3],
			pageaddr);

		status = spi_sync(spi, &msg);
		(void) dataflash_waitready(spi);

		if (status < 0) {
			printk(KERN_ERR "%s: erase %x, err %d\n",
				dev_name(&spi->dev), pageaddr, status);
			
			continue;
		}

		if (do_block) {
			instr->addr += blocksize;
			instr->len -= blocksize;
		} else {
			instr->addr += priv->page_size;
			instr->len -= priv->page_size;
		}
	}
	mutex_unlock(&priv->lock);

	
	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}


static int dataflash_read(struct mtd_info *mtd, loff_t from, size_t len,
			       size_t *retlen, u_char *buf)
{
	struct dataflash	*priv = (struct dataflash *)mtd->priv;
	struct spi_transfer	x[2] = { { .tx_dma = 0, }, };
	struct spi_message	msg;
	unsigned int		addr;
	uint8_t			*command;
	int			status;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: read 0x%x..0x%x\n",
		dev_name(&priv->spi->dev), (unsigned)from, (unsigned)(from + len));

	*retlen = 0;

	
	if (!len)
		return 0;
	if (from + len > mtd->size)
		return -EINVAL;

	
	addr = (((unsigned)from / priv->page_size) << priv->page_offset)
		+ ((unsigned)from % priv->page_size);

	command = priv->command;

	DEBUG(MTD_DEBUG_LEVEL3, "READ: (%x) %x %x %x\n",
		command[0], command[1], command[2], command[3]);

	spi_message_init(&msg);

	x[0].tx_buf = command;
	x[0].len = 8;
	spi_message_add_tail(&x[0], &msg);

	x[1].rx_buf = buf;
	x[1].len = len;
	spi_message_add_tail(&x[1], &msg);

	mutex_lock(&priv->lock);

	
	command[0] = OP_READ_CONTINUOUS;
	command[1] = (uint8_t)(addr >> 16);
	command[2] = (uint8_t)(addr >> 8);
	command[3] = (uint8_t)(addr >> 0);
	

	status = spi_sync(priv->spi, &msg);
	mutex_unlock(&priv->lock);

	if (status >= 0) {
		*retlen = msg.actual_length - 8;
		status = 0;
	} else
		DEBUG(MTD_DEBUG_LEVEL1, "%s: read %x..%x --> %d\n",
			dev_name(&priv->spi->dev),
			(unsigned)from, (unsigned)(from + len),
			status);
	return status;
}


static int dataflash_write(struct mtd_info *mtd, loff_t to, size_t len,
				size_t * retlen, const u_char * buf)
{
	struct dataflash	*priv = (struct dataflash *)mtd->priv;
	struct spi_device	*spi = priv->spi;
	struct spi_transfer	x[2] = { { .tx_dma = 0, }, };
	struct spi_message	msg;
	unsigned int		pageaddr, addr, offset, writelen;
	size_t			remaining = len;
	u_char			*writebuf = (u_char *) buf;
	int			status = -EINVAL;
	uint8_t			*command;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: write 0x%x..0x%x\n",
		dev_name(&spi->dev), (unsigned)to, (unsigned)(to + len));

	*retlen = 0;

	
	if (!len)
		return 0;
	if ((to + len) > mtd->size)
		return -EINVAL;

	spi_message_init(&msg);

	x[0].tx_buf = command = priv->command;
	x[0].len = 4;
	spi_message_add_tail(&x[0], &msg);

	pageaddr = ((unsigned)to / priv->page_size);
	offset = ((unsigned)to % priv->page_size);
	if (offset + len > priv->page_size)
		writelen = priv->page_size - offset;
	else
		writelen = len;

	mutex_lock(&priv->lock);
	while (remaining > 0) {
		DEBUG(MTD_DEBUG_LEVEL3, "write @ %i:%i len=%i\n",
			pageaddr, offset, writelen);

		

		addr = pageaddr << priv->page_offset;

		
		if (writelen != priv->page_size) {
			command[0] = OP_TRANSFER_BUF1;
			command[1] = (addr & 0x00FF0000) >> 16;
			command[2] = (addr & 0x0000FF00) >> 8;
			command[3] = 0;

			DEBUG(MTD_DEBUG_LEVEL3, "TRANSFER: (%x) %x %x %x\n",
				command[0], command[1], command[2], command[3]);

			status = spi_sync(spi, &msg);
			if (status < 0)
				DEBUG(MTD_DEBUG_LEVEL1, "%s: xfer %u -> %d \n",
					dev_name(&spi->dev), addr, status);

			(void) dataflash_waitready(priv->spi);
		}

		
		addr += offset;
		command[0] = OP_PROGRAM_VIA_BUF1;
		command[1] = (addr & 0x00FF0000) >> 16;
		command[2] = (addr & 0x0000FF00) >> 8;
		command[3] = (addr & 0x000000FF);

		DEBUG(MTD_DEBUG_LEVEL3, "PROGRAM: (%x) %x %x %x\n",
			command[0], command[1], command[2], command[3]);

		x[1].tx_buf = writebuf;
		x[1].len = writelen;
		spi_message_add_tail(x + 1, &msg);
		status = spi_sync(spi, &msg);
		spi_transfer_del(x + 1);
		if (status < 0)
			DEBUG(MTD_DEBUG_LEVEL1, "%s: pgm %u/%u -> %d \n",
				dev_name(&spi->dev), addr, writelen, status);

		(void) dataflash_waitready(priv->spi);


#ifdef CONFIG_MTD_DATAFLASH_WRITE_VERIFY

		
		addr = pageaddr << priv->page_offset;
		command[0] = OP_COMPARE_BUF1;
		command[1] = (addr & 0x00FF0000) >> 16;
		command[2] = (addr & 0x0000FF00) >> 8;
		command[3] = 0;

		DEBUG(MTD_DEBUG_LEVEL3, "COMPARE: (%x) %x %x %x\n",
			command[0], command[1], command[2], command[3]);

		status = spi_sync(spi, &msg);
		if (status < 0)
			DEBUG(MTD_DEBUG_LEVEL1, "%s: compare %u -> %d \n",
				dev_name(&spi->dev), addr, status);

		status = dataflash_waitready(priv->spi);

		
		if (status & (1 << 6)) {
			printk(KERN_ERR "%s: compare page %u, err %d\n",
				dev_name(&spi->dev), pageaddr, status);
			remaining = 0;
			status = -EIO;
			break;
		} else
			status = 0;

#endif	

		remaining = remaining - writelen;
		pageaddr++;
		offset = 0;
		writebuf += writelen;
		*retlen += writelen;

		if (remaining > priv->page_size)
			writelen = priv->page_size;
		else
			writelen = remaining;
	}
	mutex_unlock(&priv->lock);

	return status;
}



#ifdef CONFIG_MTD_DATAFLASH_OTP

static int dataflash_get_otp_info(struct mtd_info *mtd,
		struct otp_info *info, size_t len)
{
	
	info->start = 0;
	info->length = 64;
	info->locked = 1;
	return sizeof(*info);
}

static ssize_t otp_read(struct spi_device *spi, unsigned base,
		uint8_t *buf, loff_t off, size_t len)
{
	struct spi_message	m;
	size_t			l;
	uint8_t			*scratch;
	struct spi_transfer	t;
	int			status;

	if (off > 64)
		return -EINVAL;

	if ((off + len) > 64)
		len = 64 - off;
	if (len == 0)
		return len;

	spi_message_init(&m);

	l = 4 + base + off + len;
	scratch = kzalloc(l, GFP_KERNEL);
	if (!scratch)
		return -ENOMEM;

	
	scratch[0] = OP_READ_SECURITY;

	memset(&t, 0, sizeof t);
	t.tx_buf = scratch;
	t.rx_buf = scratch;
	t.len = l;
	spi_message_add_tail(&t, &m);

	dataflash_waitready(spi);

	status = spi_sync(spi, &m);
	if (status >= 0) {
		memcpy(buf, scratch + 4 + base + off, len);
		status = len;
	}

	kfree(scratch);
	return status;
}

static int dataflash_read_fact_otp(struct mtd_info *mtd,
		loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct dataflash	*priv = (struct dataflash *)mtd->priv;
	int			status;

	
	mutex_lock(&priv->lock);
	status = otp_read(priv->spi, 64, buf, from, len);
	mutex_unlock(&priv->lock);

	if (status < 0)
		return status;
	*retlen = status;
	return 0;
}

static int dataflash_read_user_otp(struct mtd_info *mtd,
		loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct dataflash	*priv = (struct dataflash *)mtd->priv;
	int			status;

	
	mutex_lock(&priv->lock);
	status = otp_read(priv->spi, 0, buf, from, len);
	mutex_unlock(&priv->lock);

	if (status < 0)
		return status;
	*retlen = status;
	return 0;
}

static int dataflash_write_user_otp(struct mtd_info *mtd,
		loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct spi_message	m;
	const size_t		l = 4 + 64;
	uint8_t			*scratch;
	struct spi_transfer	t;
	struct dataflash	*priv = (struct dataflash *)mtd->priv;
	int			status;

	if (len > 64)
		return -EINVAL;

	
	if ((from + len) > 64)
		return -EINVAL;

	
	scratch = kzalloc(l, GFP_KERNEL);
	if (!scratch)
		return -ENOMEM;
	scratch[0] = OP_WRITE_SECURITY;
	memcpy(scratch + 4 + from, buf, len);

	spi_message_init(&m);

	memset(&t, 0, sizeof t);
	t.tx_buf = scratch;
	t.len = l;
	spi_message_add_tail(&t, &m);

	
	mutex_lock(&priv->lock);
	dataflash_waitready(priv->spi);
	status = spi_sync(priv->spi, &m);
	mutex_unlock(&priv->lock);

	kfree(scratch);

	if (status >= 0) {
		status = 0;
		*retlen = len;
	}
	return status;
}

static char *otp_setup(struct mtd_info *device, char revision)
{
	device->get_fact_prot_info = dataflash_get_otp_info;
	device->read_fact_prot_reg = dataflash_read_fact_otp;
	device->get_user_prot_info = dataflash_get_otp_info;
	device->read_user_prot_reg = dataflash_read_user_otp;

	
	if (revision > 'c')
		device->write_user_prot_reg = dataflash_write_user_otp;

	return ", OTP";
}

#else

static char *otp_setup(struct mtd_info *device, char revision)
{
	return " (OTP)";
}

#endif




static int __devinit
add_dataflash_otp(struct spi_device *spi, char *name,
		int nr_pages, int pagesize, int pageoffset, char revision)
{
	struct dataflash		*priv;
	struct mtd_info			*device;
	struct flash_platform_data	*pdata = spi->dev.platform_data;
	char				*otp_tag = "";

	priv = kzalloc(sizeof *priv, GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	mutex_init(&priv->lock);
	priv->spi = spi;
	priv->page_size = pagesize;
	priv->page_offset = pageoffset;

	
	sprintf(priv->name, "spi%d.%d-%s",
			spi->master->bus_num, spi->chip_select,
			name);

	device = &priv->mtd;
	device->name = (pdata && pdata->name) ? pdata->name : priv->name;
	device->size = nr_pages * pagesize;
	device->erasesize = pagesize;
	device->writesize = pagesize;
	device->owner = THIS_MODULE;
	device->type = MTD_DATAFLASH;
	device->flags = MTD_WRITEABLE;
	device->erase = dataflash_erase;
	device->read = dataflash_read;
	device->write = dataflash_write;
	device->priv = priv;

	device->dev.parent = &spi->dev;

	if (revision >= 'c')
		otp_tag = otp_setup(device, revision);

	dev_info(&spi->dev, "%s (%lld KBytes) pagesize %d bytes%s\n",
			name, (long long)((device->size + 1023) >> 10),
			pagesize, otp_tag);
	dev_set_drvdata(&spi->dev, priv);

	if (mtd_has_partitions()) {
		struct mtd_partition	*parts;
		int			nr_parts = 0;

		if (mtd_has_cmdlinepart()) {
			static const char *part_probes[]
					= { "cmdlinepart", NULL, };

			nr_parts = parse_mtd_partitions(device,
					part_probes, &parts, 0);
		}

		if (nr_parts <= 0 && pdata && pdata->parts) {
			parts = pdata->parts;
			nr_parts = pdata->nr_parts;
		}

		if (nr_parts > 0) {
			priv->partitioned = 1;
			return add_mtd_partitions(device, parts, nr_parts);
		}
	} else if (pdata && pdata->nr_parts)
		dev_warn(&spi->dev, "ignoring %d default partitions on %s\n",
				pdata->nr_parts, device->name);

	return add_mtd_device(device) == 1 ? -ENODEV : 0;
}

static inline int __devinit
add_dataflash(struct spi_device *spi, char *name,
		int nr_pages, int pagesize, int pageoffset)
{
	return add_dataflash_otp(spi, name, nr_pages, pagesize,
			pageoffset, 0);
}

struct flash_info {
	char		*name;

	
	uint32_t	jedec_id;

	
	unsigned	nr_pages;
	uint16_t	pagesize;
	uint16_t	pageoffset;

	uint16_t	flags;
#define SUP_POW2PS	0x0002		
#define IS_POW2PS	0x0001		
};

static struct flash_info __devinitdata dataflash_data [] = {

	
	{ "AT45DB011B",  0x1f2200, 512, 264, 9, SUP_POW2PS},
	{ "at45db011d",  0x1f2200, 512, 256, 8, SUP_POW2PS | IS_POW2PS},

	{ "AT45DB021B",  0x1f2300, 1024, 264, 9, SUP_POW2PS},
	{ "at45db021d",  0x1f2300, 1024, 256, 8, SUP_POW2PS | IS_POW2PS},

	{ "AT45DB041x",  0x1f2400, 2048, 264, 9, SUP_POW2PS},
	{ "at45db041d",  0x1f2400, 2048, 256, 8, SUP_POW2PS | IS_POW2PS},

	{ "AT45DB081B",  0x1f2500, 4096, 264, 9, SUP_POW2PS},
	{ "at45db081d",  0x1f2500, 4096, 256, 8, SUP_POW2PS | IS_POW2PS},

	{ "AT45DB161x",  0x1f2600, 4096, 528, 10, SUP_POW2PS},
	{ "at45db161d",  0x1f2600, 4096, 512, 9, SUP_POW2PS | IS_POW2PS},

	{ "AT45DB321x",  0x1f2700, 8192, 528, 10, 0},		

	{ "AT45DB321x",  0x1f2701, 8192, 528, 10, SUP_POW2PS},
	{ "at45db321d",  0x1f2701, 8192, 512, 9, SUP_POW2PS | IS_POW2PS},

	{ "AT45DB642x",  0x1f2800, 8192, 1056, 11, SUP_POW2PS},
	{ "at45db642d",  0x1f2800, 8192, 1024, 10, SUP_POW2PS | IS_POW2PS},
};

static struct flash_info *__devinit jedec_probe(struct spi_device *spi)
{
	int			tmp;
	uint8_t			code = OP_READ_ID;
	uint8_t			id[3];
	uint32_t		jedec;
	struct flash_info	*info;
	int status;

	
	tmp = spi_write_then_read(spi, &code, 1, id, 3);
	if (tmp < 0) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: error %d reading JEDEC ID\n",
			dev_name(&spi->dev), tmp);
		return ERR_PTR(tmp);
	}
	if (id[0] != 0x1f)
		return NULL;

	jedec = id[0];
	jedec = jedec << 8;
	jedec |= id[1];
	jedec = jedec << 8;
	jedec |= id[2];

	for (tmp = 0, info = dataflash_data;
			tmp < ARRAY_SIZE(dataflash_data);
			tmp++, info++) {
		if (info->jedec_id == jedec) {
			DEBUG(MTD_DEBUG_LEVEL1, "%s: OTP, sector protect%s\n",
				dev_name(&spi->dev),
				(info->flags & SUP_POW2PS)
					? ", binary pagesize" : ""
				);
			if (info->flags & SUP_POW2PS) {
				status = dataflash_status(spi);
				if (status < 0) {
					DEBUG(MTD_DEBUG_LEVEL1,
						"%s: status error %d\n",
						dev_name(&spi->dev), status);
					return ERR_PTR(status);
				}
				if (status & 0x1) {
					if (info->flags & IS_POW2PS)
						return info;
				} else {
					if (!(info->flags & IS_POW2PS))
						return info;
				}
			} else
				return info;
		}
	}

	
	dev_warn(&spi->dev, "JEDEC id %06x not handled\n", jedec);
	return ERR_PTR(-ENODEV);
}


static int __devinit dataflash_probe(struct spi_device *spi)
{
	int status;
	struct flash_info	*info;

	
	info = jedec_probe(spi);
	if (IS_ERR(info))
		return PTR_ERR(info);
	if (info != NULL)
		return add_dataflash_otp(spi, info->name, info->nr_pages,
				info->pagesize, info->pageoffset,
				(info->flags & SUP_POW2PS) ? 'd' : 'c');

	
	status = dataflash_status(spi);
	if (status <= 0 || status == 0xff) {
		DEBUG(MTD_DEBUG_LEVEL1, "%s: status error %d\n",
				dev_name(&spi->dev), status);
		if (status == 0 || status == 0xff)
			status = -ENODEV;
		return status;
	}

	
	switch (status & 0x3c) {
	case 0x0c:	
		status = add_dataflash(spi, "AT45DB011B", 512, 264, 9);
		break;
	case 0x14:	
		status = add_dataflash(spi, "AT45DB021B", 1024, 264, 9);
		break;
	case 0x1c:	
		status = add_dataflash(spi, "AT45DB041x", 2048, 264, 9);
		break;
	case 0x24:	
		status = add_dataflash(spi, "AT45DB081B", 4096, 264, 9);
		break;
	case 0x2c:	
		status = add_dataflash(spi, "AT45DB161x", 4096, 528, 10);
		break;
	case 0x34:	
		status = add_dataflash(spi, "AT45DB321x", 8192, 528, 10);
		break;
	case 0x38:	
	case 0x3c:
		status = add_dataflash(spi, "AT45DB642x", 8192, 1056, 11);
		break;
	
	default:
		DEBUG(MTD_DEBUG_LEVEL1, "%s: unsupported device (%x)\n",
				dev_name(&spi->dev), status & 0x3c);
		status = -ENODEV;
	}

	if (status < 0)
		DEBUG(MTD_DEBUG_LEVEL1, "%s: add_dataflash --> %d\n",
				dev_name(&spi->dev), status);

	return status;
}

static int __devexit dataflash_remove(struct spi_device *spi)
{
	struct dataflash	*flash = dev_get_drvdata(&spi->dev);
	int			status;

	DEBUG(MTD_DEBUG_LEVEL1, "%s: remove\n", dev_name(&spi->dev));

	if (mtd_has_partitions() && flash->partitioned)
		status = del_mtd_partitions(&flash->mtd);
	else
		status = del_mtd_device(&flash->mtd);
	if (status == 0)
		kfree(flash);
	return status;
}

static struct spi_driver dataflash_driver = {
	.driver = {
		.name		= "mtd_dataflash",
		.bus		= &spi_bus_type,
		.owner		= THIS_MODULE,
	},

	.probe		= dataflash_probe,
	.remove		= __devexit_p(dataflash_remove),

	
};

static int __init dataflash_init(void)
{
	return spi_register_driver(&dataflash_driver);
}
module_init(dataflash_init);

static void __exit dataflash_exit(void)
{
	spi_unregister_driver(&dataflash_driver);
}
module_exit(dataflash_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrew Victor, David Brownell");
MODULE_DESCRIPTION("MTD DataFlash driver");
MODULE_ALIAS("spi:mtd_dataflash");
