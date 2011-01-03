

#include "saa7164.h"


int saa7164_bus_setup(struct saa7164_dev *dev)
{
	tmComResBusInfo_t *b	= &dev->bus;

	mutex_init(&b->lock);

	b->Type			= TYPE_BUS_PCIe;
	b->m_wMaxReqSize	= SAA_DEVICE_MAXREQUESTSIZE;

	b->m_pdwSetRing		= (u8 *)(dev->bmmio +
		((u32)dev->busdesc.CommandRing));

	b->m_dwSizeSetRing	= SAA_DEVICE_BUFFERBLOCKSIZE;

	b->m_pdwGetRing		= (u8 *)(dev->bmmio +
		((u32)dev->busdesc.ResponseRing));

	b->m_dwSizeGetRing	= SAA_DEVICE_BUFFERBLOCKSIZE;

	b->m_pdwSetWritePos	= (u32 *)((u8 *)(dev->bmmio +
		((u32)dev->intfdesc.BARLocation) + (2 * sizeof(u64))));

	b->m_pdwSetReadPos	= (u32 *)((u8 *)b->m_pdwSetWritePos +
		1 * sizeof(u32));

	b->m_pdwGetWritePos	= (u32 *)((u8 *)b->m_pdwSetWritePos +
		2 * sizeof(u32));

	b->m_pdwGetReadPos	= (u32 *)((u8 *)b->m_pdwSetWritePos +
		3 * sizeof(u32));

	return 0;
}

void saa7164_bus_dump(struct saa7164_dev *dev)
{
	tmComResBusInfo_t *b = &dev->bus;

	dprintk(DBGLVL_BUS, "Dumping the bus structure:\n");
	dprintk(DBGLVL_BUS, " .type             = %d\n", b->Type);
	dprintk(DBGLVL_BUS, " .dev->bmmio       = 0x%p\n", dev->bmmio);
	dprintk(DBGLVL_BUS, " .m_wMaxReqSize    = 0x%x\n", b->m_wMaxReqSize);
	dprintk(DBGLVL_BUS, " .m_pdwSetRing     = 0x%p\n", b->m_pdwSetRing);
	dprintk(DBGLVL_BUS, " .m_dwSizeSetRing  = 0x%x\n", b->m_dwSizeSetRing);
	dprintk(DBGLVL_BUS, " .m_pdwGetRing     = 0x%p\n", b->m_pdwGetRing);
	dprintk(DBGLVL_BUS, " .m_dwSizeGetRing  = 0x%x\n", b->m_dwSizeGetRing);

	dprintk(DBGLVL_BUS, " .m_pdwSetWritePos = 0x%p (0x%08x)\n",
		b->m_pdwSetWritePos, *b->m_pdwSetWritePos);

	dprintk(DBGLVL_BUS, " .m_pdwSetReadPos  = 0x%p (0x%08x)\n",
		b->m_pdwSetReadPos, *b->m_pdwSetReadPos);

	dprintk(DBGLVL_BUS, " .m_pdwGetWritePos = 0x%p (0x%08x)\n",
		b->m_pdwGetWritePos, *b->m_pdwGetWritePos);

	dprintk(DBGLVL_BUS, " .m_pdwGetReadPos  = 0x%p (0x%08x)\n",
		b->m_pdwGetReadPos, *b->m_pdwGetReadPos);
}

void saa7164_bus_dumpmsg(struct saa7164_dev *dev, tmComResInfo_t* m, void *buf)
{
	dprintk(DBGLVL_BUS, "Dumping msg structure:\n");
	dprintk(DBGLVL_BUS, " .id               = %d\n",   m->id);
	dprintk(DBGLVL_BUS, " .flags            = 0x%x\n", m->flags);
	dprintk(DBGLVL_BUS, " .size             = 0x%x\n", m->size);
	dprintk(DBGLVL_BUS, " .command          = 0x%x\n", m->command);
	dprintk(DBGLVL_BUS, " .controlselector  = 0x%x\n", m->controlselector);
	dprintk(DBGLVL_BUS, " .seqno            = %d\n",   m->seqno);
	if (buf)
		dprintk(DBGLVL_BUS, " .buffer (ignored)\n");
}


int saa7164_bus_set(struct saa7164_dev *dev, tmComResInfo_t* msg, void *buf)
{
	tmComResBusInfo_t *bus = &dev->bus;
	u32 bytes_to_write, read_distance, timeout, curr_srp, curr_swp;
	u32 new_swp, space_rem;
	int ret = SAA_ERR_BAD_PARAMETER;

	if (!msg) {
		printk(KERN_ERR "%s() !msg\n", __func__);
		return SAA_ERR_BAD_PARAMETER;
	}

	dprintk(DBGLVL_BUS, "%s()\n", __func__);

	msg->size = cpu_to_le16(msg->size);
	msg->command = cpu_to_le16(msg->command);
	msg->controlselector = cpu_to_le16(msg->controlselector);

	if (msg->size > dev->bus.m_wMaxReqSize) {
		printk(KERN_ERR "%s() Exceeded dev->bus.m_wMaxReqSize\n",
			__func__);
		return SAA_ERR_BAD_PARAMETER;
	}

	if ((msg->size > 0) && (buf == 0)) {
		printk(KERN_ERR "%s() Missing message buffer\n", __func__);
		return SAA_ERR_BAD_PARAMETER;
	}

	
	mutex_lock(&bus->lock);

	bytes_to_write = sizeof(*msg) + msg->size;
	read_distance = 0;
	timeout = SAA_BUS_TIMEOUT;
	curr_srp = le32_to_cpu(*bus->m_pdwSetReadPos);
	curr_swp = le32_to_cpu(*bus->m_pdwSetWritePos);

	
	if (curr_srp > curr_swp)
		
		read_distance = curr_srp - curr_swp;
	else
		
		read_distance = (curr_srp + bus->m_dwSizeSetRing) - curr_swp;

	dprintk(DBGLVL_BUS, "%s() bytes_to_write = %d\n", __func__,
		bytes_to_write);

	dprintk(DBGLVL_BUS, "%s() read_distance = %d\n", __func__,
		read_distance);

	dprintk(DBGLVL_BUS, "%s() curr_srp = %x\n", __func__, curr_srp);
	dprintk(DBGLVL_BUS, "%s() curr_swp = %x\n", __func__, curr_swp);

	
	while (bytes_to_write >= read_distance) {

		if (timeout-- == 0) {
			printk(KERN_ERR "%s() bus timeout\n", __func__);
			ret = SAA_ERR_NO_RESOURCES;
			goto out;
		}

		
		
		mdelay(1);

		
		curr_srp = le32_to_cpu(*bus->m_pdwSetReadPos);

		
		if (curr_srp > curr_swp)
			
			read_distance = curr_srp - curr_swp;
		else
			
			read_distance = (curr_srp + bus->m_dwSizeSetRing) -
				curr_swp;

	}

	
	new_swp = curr_swp + bytes_to_write;

	dprintk(DBGLVL_BUS, "%s() new_swp = %x\n", __func__, new_swp);
	dprintk(DBGLVL_BUS, "%s() bus->m_dwSizeSetRing = %x\n", __func__,
		bus->m_dwSizeSetRing);

	

	
	if (new_swp > bus->m_dwSizeSetRing) {

		
		new_swp -= bus->m_dwSizeSetRing;

		space_rem = bus->m_dwSizeSetRing - curr_swp;

		dprintk(DBGLVL_BUS, "%s() space_rem = %x\n", __func__,
			space_rem);

		dprintk(DBGLVL_BUS, "%s() sizeof(*msg) = %d\n", __func__,
			(u32)sizeof(*msg));

		if (space_rem < sizeof(*msg)) {
			dprintk(DBGLVL_BUS, "%s() tr4\n", __func__);

			
			memcpy(bus->m_pdwSetRing + curr_swp, msg, space_rem);
			memcpy(bus->m_pdwSetRing, (u8 *)msg + space_rem,
				sizeof(*msg) - space_rem);

			memcpy(bus->m_pdwSetRing + sizeof(*msg) - space_rem,
				buf, msg->size);

		} else if (space_rem == sizeof(*msg)) {
			dprintk(DBGLVL_BUS, "%s() tr5\n", __func__);

			
			memcpy(bus->m_pdwSetRing + curr_swp, msg, sizeof(*msg));
			memcpy(bus->m_pdwSetRing, buf, msg->size);

		} else {
			
			memcpy(bus->m_pdwSetRing + curr_swp, msg, sizeof(*msg));
			if (msg->size > 0) {
				memcpy(bus->m_pdwSetRing + curr_swp +
					sizeof(*msg), buf, space_rem -
					sizeof(*msg));
				memcpy(bus->m_pdwSetRing, (u8 *)buf +
					space_rem - sizeof(*msg),
					bytes_to_write - space_rem);
			}

		}

	} 
	else {
		dprintk(DBGLVL_BUS, "%s() tr6\n", __func__);

		
		memcpy(bus->m_pdwSetRing + curr_swp, msg, sizeof(*msg));
		memcpy(bus->m_pdwSetRing + curr_swp + sizeof(*msg), buf,
			msg->size);
	}

	dprintk(DBGLVL_BUS, "%s() new_swp = %x\n", __func__, new_swp);

	

	
	*bus->m_pdwSetWritePos = cpu_to_le32(new_swp);
	ret = SAA_OK;

out:
	mutex_unlock(&bus->lock);
	return ret;
}


int saa7164_bus_get(struct saa7164_dev *dev, tmComResInfo_t* msg, void *buf,
	int peekonly)
{
	tmComResBusInfo_t *bus = &dev->bus;
	u32 bytes_to_read, write_distance, curr_grp, curr_gwp,
		new_grp, buf_size, space_rem;
	tmComResInfo_t msg_tmp;
	int ret = SAA_ERR_BAD_PARAMETER;

	if (msg == 0)
		return ret;

	if (msg->size > dev->bus.m_wMaxReqSize) {
		printk(KERN_ERR "%s() Exceeded dev->bus.m_wMaxReqSize\n",
			__func__);
		return ret;
	}

	if ((peekonly == 0) && (msg->size > 0) && (buf == 0)) {
		printk(KERN_ERR
			"%s() Missing msg buf, size should be %d bytes\n",
			__func__, msg->size);
		return ret;
	}

	mutex_lock(&bus->lock);

	
	curr_gwp = le32_to_cpu(*bus->m_pdwGetWritePos);
	curr_grp = le32_to_cpu(*bus->m_pdwGetReadPos);

	if (curr_gwp == curr_grp) {
		dprintk(DBGLVL_BUS, "%s() No message on the bus\n", __func__);
		ret = SAA_ERR_EMPTY;
		goto out;
	}

	bytes_to_read = sizeof(*msg);

	
	write_distance = 0;
	if (curr_gwp >= curr_grp)
		
		write_distance = curr_gwp - curr_grp;
	else
		
		write_distance = curr_gwp + bus->m_dwSizeGetRing - curr_grp;

	if (bytes_to_read > write_distance) {
		printk(KERN_ERR "%s() No message/response found\n", __func__);
		ret = SAA_ERR_INVALID_COMMAND;
		goto out;
	}

	
	new_grp = curr_grp + bytes_to_read;
	if (new_grp > bus->m_dwSizeGetRing) {

		
		new_grp -= bus->m_dwSizeGetRing;
		space_rem = bus->m_dwSizeGetRing - curr_grp;

		memcpy(&msg_tmp, bus->m_pdwGetRing + curr_grp, space_rem);
		memcpy((u8 *)&msg_tmp + space_rem, bus->m_pdwGetRing,
			bytes_to_read - space_rem);

	} else {
		
		memcpy(&msg_tmp, bus->m_pdwGetRing + curr_grp, bytes_to_read);
	}

	
	
	if (peekonly) {
		memcpy(msg, &msg_tmp, sizeof(*msg));
		goto peekout;
	}

	
	if ((msg_tmp.id != msg->id) || (msg_tmp.command != msg->command) ||
		(msg_tmp.controlselector != msg->controlselector) ||
		(msg_tmp.seqno != msg->seqno) || (msg_tmp.size != msg->size)) {

		printk(KERN_ERR "%s() Unexpected msg miss-match\n", __func__);
		saa7164_bus_dumpmsg(dev, msg, buf);
		saa7164_bus_dumpmsg(dev, &msg_tmp, 0);
		ret = SAA_ERR_INVALID_COMMAND;
		goto out;
	}

	
	buf_size = msg->size;

	bytes_to_read = sizeof(*msg) + msg->size;
	
	write_distance = 0;
	if (curr_gwp >= curr_grp)
		
		write_distance = curr_gwp - curr_grp;
	else
		
		write_distance = curr_gwp + bus->m_dwSizeGetRing - curr_grp;

	if (bytes_to_read > write_distance) {
		printk(KERN_ERR "%s() Invalid bus state, missing msg "
			"or mangled ring, faulty H/W / bad code?\n", __func__);
		ret = SAA_ERR_INVALID_COMMAND;
		goto out;
	}

	
	new_grp = curr_grp + bytes_to_read;
	if (new_grp > bus->m_dwSizeGetRing) {

		
		new_grp -= bus->m_dwSizeGetRing;
		space_rem = bus->m_dwSizeGetRing - curr_grp;

		if (space_rem < sizeof(*msg)) {
			
			memcpy(msg, bus->m_pdwGetRing + curr_grp, space_rem);
			memcpy((u8 *)msg + space_rem, bus->m_pdwGetRing,
				sizeof(*msg) - space_rem);
			if (buf)
				memcpy(buf, bus->m_pdwGetRing + sizeof(*msg) -
					space_rem, buf_size);

		} else if (space_rem == sizeof(*msg)) {
			memcpy(msg, bus->m_pdwGetRing + curr_grp, sizeof(*msg));
			if (buf)
				memcpy(buf, bus->m_pdwGetRing, buf_size);
		} else {
			
			memcpy(msg, bus->m_pdwGetRing + curr_grp, sizeof(*msg));
			if (buf) {
				memcpy(buf, bus->m_pdwGetRing + curr_grp +
					sizeof(*msg), space_rem - sizeof(*msg));
				memcpy(buf + space_rem - sizeof(*msg),
					bus->m_pdwGetRing, bytes_to_read -
					space_rem);
			}

		}

	} else {
		
		memcpy(msg, bus->m_pdwGetRing + curr_grp, sizeof(*msg));
		if (buf)
			memcpy(buf, bus->m_pdwGetRing + curr_grp + sizeof(*msg),
				buf_size);
	}

	
	*bus->m_pdwGetReadPos = cpu_to_le32(new_grp);

peekout:
	msg->size = le16_to_cpu(msg->size);
	msg->command = le16_to_cpu(msg->command);
	msg->controlselector = le16_to_cpu(msg->controlselector);
	ret = SAA_OK;
out:
	mutex_unlock(&bus->lock);
	return ret;
}

