





#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <asm/byteorder.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/irq.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/init.h>

#ifdef CONFIG_PPC_PMAC
#include <asm/machdep.h>
#include <asm/pmac_feature.h>
#include <asm/prom.h>
#include <asm/pci-bridge.h>
#endif

#include "csr1212.h"
#include "ieee1394.h"
#include "ieee1394_types.h"
#include "hosts.h"
#include "dma.h"
#include "iso.h"
#include "ieee1394_core.h"
#include "highlevel.h"
#include "ohci1394.h"

#ifdef CONFIG_IEEE1394_VERBOSEDEBUG
#define OHCI1394_DEBUG
#endif

#ifdef DBGMSG
#undef DBGMSG
#endif

#ifdef OHCI1394_DEBUG
#define DBGMSG(fmt, args...) \
printk(KERN_INFO "%s: fw-host%d: " fmt "\n" , OHCI1394_DRIVER_NAME, ohci->host->id , ## args)
#else
#define DBGMSG(fmt, args...) do {} while (0)
#endif


#define PRINT_G(level, fmt, args...) \
printk(level "%s: " fmt "\n" , OHCI1394_DRIVER_NAME , ## args)


#define PRINT(level, fmt, args...) \
printk(level "%s: fw-host%d: " fmt "\n" , OHCI1394_DRIVER_NAME, ohci->host->id , ## args)


static int phys_dma = 1;
module_param(phys_dma, int, 0444);
MODULE_PARM_DESC(phys_dma, "Enable physical DMA (default = 1).");

static void dma_trm_tasklet(unsigned long data);
static void dma_trm_reset(struct dma_trm_ctx *d);

static int alloc_dma_rcv_ctx(struct ti_ohci *ohci, struct dma_rcv_ctx *d,
			     enum context_type type, int ctx, int num_desc,
			     int buf_size, int split_buf_size, int context_base);
static void free_dma_rcv_ctx(struct dma_rcv_ctx *d);

static int alloc_dma_trm_ctx(struct ti_ohci *ohci, struct dma_trm_ctx *d,
			     enum context_type type, int ctx, int num_desc,
			     int context_base);

static void ohci1394_pci_remove(struct pci_dev *pdev);

#ifndef __LITTLE_ENDIAN
static const size_t hdr_sizes[] = {
	3,	
	4,	
	3,	
	0,	
	3,	
	4,	
	3,	
	4,	
	1,	
	4,	
	2,	
	4,	
		
};

static inline void header_le32_to_cpu(quadlet_t *data, unsigned char tcode)
{
	size_t size;

	if (unlikely(tcode >= ARRAY_SIZE(hdr_sizes)))
		return;

	size = hdr_sizes[tcode];
	while (size--)
		data[size] = le32_to_cpu(data[size]);
}
#else
#define header_le32_to_cpu(w,x) do {} while (0)
#endif 



static u8 get_phy_reg(struct ti_ohci *ohci, u8 addr)
{
	int i;
	unsigned long flags;
	quadlet_t r;

	spin_lock_irqsave (&ohci->phy_reg_lock, flags);

	reg_write(ohci, OHCI1394_PhyControl, (addr << 8) | 0x00008000);

	for (i = 0; i < OHCI_LOOP_COUNT; i++) {
		if (reg_read(ohci, OHCI1394_PhyControl) & 0x80000000)
			break;

		mdelay(1);
	}

	r = reg_read(ohci, OHCI1394_PhyControl);

	if (i >= OHCI_LOOP_COUNT)
		PRINT (KERN_ERR, "Get PHY Reg timeout [0x%08x/0x%08x/%d]",
		       r, r & 0x80000000, i);

	spin_unlock_irqrestore (&ohci->phy_reg_lock, flags);

	return (r & 0x00ff0000) >> 16;
}

static void set_phy_reg(struct ti_ohci *ohci, u8 addr, u8 data)
{
	int i;
	unsigned long flags;
	u32 r = 0;

	spin_lock_irqsave (&ohci->phy_reg_lock, flags);

	reg_write(ohci, OHCI1394_PhyControl, (addr << 8) | data | 0x00004000);

	for (i = 0; i < OHCI_LOOP_COUNT; i++) {
		r = reg_read(ohci, OHCI1394_PhyControl);
		if (!(r & 0x00004000))
			break;

		mdelay(1);
	}

	if (i == OHCI_LOOP_COUNT)
		PRINT (KERN_ERR, "Set PHY Reg timeout [0x%08x/0x%08x/%d]",
		       r, r & 0x00004000, i);

	spin_unlock_irqrestore (&ohci->phy_reg_lock, flags);

	return;
}


static void set_phy_reg_mask(struct ti_ohci *ohci, u8 addr, u8 data)
{
	u8 old;

	old = get_phy_reg (ohci, addr);
	old |= data;
	set_phy_reg (ohci, addr, old);

	return;
}

static void handle_selfid(struct ti_ohci *ohci, struct hpsb_host *host,
				int phyid, int isroot)
{
	quadlet_t *q = ohci->selfid_buf_cpu;
	quadlet_t self_id_count=reg_read(ohci, OHCI1394_SelfIDCount);
	size_t size;
	quadlet_t q0, q1;

	

	if (ohci->selfid_swap)
		q0 = le32_to_cpu(q[0]);
	else
		q0 = q[0];

	if ((self_id_count & 0x80000000) ||
	    ((self_id_count & 0x00FF0000) != (q0 & 0x00FF0000))) {
		PRINT(KERN_ERR,
		      "Error in reception of SelfID packets [0x%08x/0x%08x] (count: %d)",
		      self_id_count, q0, ohci->self_id_errors);

		
		if (ohci->self_id_errors<OHCI1394_MAX_SELF_ID_ERRORS) {
			set_phy_reg_mask (ohci, 1, 0x40);
			ohci->self_id_errors++;
		} else {
			PRINT(KERN_ERR,
			      "Too many errors on SelfID error reception, giving up!");
		}
		return;
	}

	
	ohci->self_id_errors = 0;

	size = ((self_id_count & 0x00001FFC) >> 2) - 1;
	q++;

	while (size > 0) {
		if (ohci->selfid_swap) {
			q0 = le32_to_cpu(q[0]);
			q1 = le32_to_cpu(q[1]);
		} else {
			q0 = q[0];
			q1 = q[1];
		}

		if (q0 == ~q1) {
			DBGMSG ("SelfID packet 0x%x received", q0);
			hpsb_selfid_received(host, cpu_to_be32(q0));
			if (((q0 & 0x3f000000) >> 24) == phyid)
				DBGMSG ("SelfID for this node is 0x%08x", q0);
		} else {
			PRINT(KERN_ERR,
			      "SelfID is inconsistent [0x%08x/0x%08x]", q0, q1);
		}
		q += 2;
		size -= 2;
	}

	DBGMSG("SelfID complete");

	return;
}

static void ohci_soft_reset(struct ti_ohci *ohci) {
	int i;

	reg_write(ohci, OHCI1394_HCControlSet, OHCI1394_HCControl_softReset);

	for (i = 0; i < OHCI_LOOP_COUNT; i++) {
		if (!(reg_read(ohci, OHCI1394_HCControlSet) & OHCI1394_HCControl_softReset))
			break;
		mdelay(1);
	}
	DBGMSG ("Soft reset finished");
}



static void initialize_dma_rcv_ctx(struct dma_rcv_ctx *d, int generate_irq)
{
	struct ti_ohci *ohci = (struct ti_ohci*)(d->ohci);
	int i;

	ohci1394_stop_context(ohci, d->ctrlClear, NULL);

	for (i=0; i<d->num_desc; i++) {
		u32 c;

		c = DMA_CTL_INPUT_MORE | DMA_CTL_UPDATE | DMA_CTL_BRANCH;
		if (generate_irq)
			c |= DMA_CTL_IRQ;

		d->prg_cpu[i]->control = cpu_to_le32(c | d->buf_size);

		
		if (i + 1 < d->num_desc) {
			d->prg_cpu[i]->branchAddress =
				cpu_to_le32((d->prg_bus[i+1] & 0xfffffff0) | 0x1);
		} else {
			d->prg_cpu[i]->branchAddress =
				cpu_to_le32((d->prg_bus[0] & 0xfffffff0));
		}

		d->prg_cpu[i]->address = cpu_to_le32(d->buf_bus[i]);
		d->prg_cpu[i]->status = cpu_to_le32(d->buf_size);
	}

        d->buf_ind = 0;
        d->buf_offset = 0;

	if (d->type == DMA_CTX_ISO) {
		
		reg_write(ohci, d->ctrlClear, 0xffffffff);

		
		reg_write(ohci, d->ctrlSet, 0xd0000000);

		
		reg_write(ohci, d->ctxtMatch, 0xf0000000);

		
		reg_write(ohci, OHCI1394_IRMultiChanMaskHiClear, 0xffffffff);
		reg_write(ohci, OHCI1394_IRMultiChanMaskLoClear, 0xffffffff);

		
		reg_write(ohci, OHCI1394_IsoRecvIntMaskSet, 1 << d->ctx);
	}

	
	reg_write(ohci, d->cmdPtr, d->prg_bus[0] | 0x1);

	
	reg_write(ohci, d->ctrlSet, 0x00008000);

	DBGMSG("Receive DMA ctx=%d initialized", d->ctx);
}


static void initialize_dma_trm_ctx(struct dma_trm_ctx *d)
{
	struct ti_ohci *ohci = (struct ti_ohci*)(d->ohci);

	
	ohci1394_stop_context(ohci, d->ctrlClear, NULL);

        d->prg_ind = 0;
	d->sent_ind = 0;
	d->free_prgs = d->num_desc;
        d->branchAddrPtr = NULL;
	INIT_LIST_HEAD(&d->fifo_list);
	INIT_LIST_HEAD(&d->pending_list);

	if (d->type == DMA_CTX_ISO) {
		
		reg_write(ohci, OHCI1394_IsoXmitIntMaskSet, 1 << d->ctx);
	}

	DBGMSG("Transmit DMA ctx=%d initialized", d->ctx);
}


static int get_nb_iso_ctx(struct ti_ohci *ohci, int reg)
{
	int i,ctx=0;
	u32 tmp;

	reg_write(ohci, reg, 0xffffffff);
	tmp = reg_read(ohci, reg);

	DBGMSG("Iso contexts reg: %08x implemented: %08x", reg, tmp);

	
	for (i=0; i<32; i++) {
	    	if (tmp & 1) ctx++;
		tmp >>= 1;
	}
	return ctx;
}


static void ohci_initialize(struct ti_ohci *ohci)
{
	quadlet_t buf;
	int num_ports, i;

	spin_lock_init(&ohci->phy_reg_lock);

	
	buf = reg_read(ohci, OHCI1394_BusOptions);
	buf |=  0x60000000; 
	if (hpsb_disable_irm)
		buf &= ~0x80000000;
	else
		buf |=  0x80000000; 
	buf &= ~0x00ff0000; 
	buf &= ~0x18000000; 
	reg_write(ohci, OHCI1394_BusOptions, buf);

	
	reg_write(ohci, OHCI1394_NodeID, 0x0000ffc0);

	
	reg_write(ohci, OHCI1394_HCControlSet, OHCI1394_HCControl_postedWriteEnable);

	
	reg_write(ohci, OHCI1394_LinkControlClear, 0xffffffff);

	
	reg_write(ohci, OHCI1394_LinkControlSet,
		  OHCI1394_LinkControl_CycleTimerEnable |
		  OHCI1394_LinkControl_CycleMaster);
	i = get_phy_reg(ohci, 4) | PHY_04_LCTRL;
	if (hpsb_disable_irm)
		i &= ~PHY_04_CONTENDER;
	else
		i |= PHY_04_CONTENDER;
	set_phy_reg(ohci, 4, i);

	
	reg_write(ohci, OHCI1394_SelfIDBuffer, ohci->selfid_buf_bus);

	
	reg_write(ohci, OHCI1394_LinkControlSet, OHCI1394_LinkControl_RcvSelfID);

	
	reg_write(ohci, OHCI1394_ConfigROMmap, ohci->csr_config_rom_bus);

	
	ohci->max_packet_size =
		1<<(((reg_read(ohci, OHCI1394_BusOptions)>>12)&0xf)+1);
		
	
	reg_write(ohci, OHCI1394_IsoRecvIntMaskClear, 0xffffffff);
	reg_write(ohci, OHCI1394_IsoRecvIntEventClear, 0xffffffff);

	
	reg_write(ohci, OHCI1394_IsoXmitIntMaskClear, 0xffffffff);
	reg_write(ohci, OHCI1394_IsoXmitIntEventClear, 0xffffffff);

	
	initialize_dma_rcv_ctx(&ohci->ar_req_context, 0);
	initialize_dma_rcv_ctx(&ohci->ar_resp_context, 0);

	
	initialize_dma_trm_ctx(&ohci->at_req_context);
	initialize_dma_trm_ctx(&ohci->at_resp_context);
	
	
	reg_write(ohci, OHCI1394_AsReqFilterHiSet, 0x80000000);

	
	reg_write(ohci, OHCI1394_PhyUpperBound,
		  OHCI1394_PHYS_UPPER_BOUND_PROGRAMMED >> 16);

	DBGMSG("physUpperBoundOffset=%08x",
	       reg_read(ohci, OHCI1394_PhyUpperBound));

	
	reg_write(ohci, OHCI1394_ATRetries,
		  OHCI1394_MAX_AT_REQ_RETRIES |
		  (OHCI1394_MAX_AT_RESP_RETRIES<<4) |
		  (OHCI1394_MAX_PHYS_RESP_RETRIES<<8));

	
	reg_write(ohci, OHCI1394_HCControlClear, OHCI1394_HCControl_noByteSwap);

	
	reg_write(ohci, OHCI1394_IntMaskSet,
		  OHCI1394_unrecoverableError |
		  OHCI1394_masterIntEnable |
		  OHCI1394_busReset |
		  OHCI1394_selfIDComplete |
		  OHCI1394_RSPkt |
		  OHCI1394_RQPkt |
		  OHCI1394_respTxComplete |
		  OHCI1394_reqTxComplete |
		  OHCI1394_isochRx |
		  OHCI1394_isochTx |
		  OHCI1394_postedWriteErr |
		  OHCI1394_cycleTooLong |
		  OHCI1394_cycleInconsistent);

	
	reg_write(ohci, OHCI1394_HCControlSet, OHCI1394_HCControl_linkEnable);

	buf = reg_read(ohci, OHCI1394_Version);
	PRINT(KERN_INFO, "OHCI-1394 %d.%d (PCI): IRQ=[%d]  "
	      "MMIO=[%llx-%llx]  Max Packet=[%d]  IR/IT contexts=[%d/%d]",
	      ((((buf) >> 16) & 0xf) + (((buf) >> 20) & 0xf) * 10),
	      ((((buf) >> 4) & 0xf) + ((buf) & 0xf) * 10), ohci->dev->irq,
	      (unsigned long long)pci_resource_start(ohci->dev, 0),
	      (unsigned long long)pci_resource_start(ohci->dev, 0) + OHCI1394_REGISTER_SIZE - 1,
	      ohci->max_packet_size,
	      ohci->nb_iso_rcv_ctx, ohci->nb_iso_xmit_ctx);

	
	num_ports = get_phy_reg(ohci, 2) & 0xf;
	for (i = 0; i < num_ports; i++) {
		unsigned int status;

		set_phy_reg(ohci, 7, i);
		status = get_phy_reg(ohci, 8);

		if (status & 0x20)
			set_phy_reg(ohci, 8, status & ~1);
	}

        
        if ((ohci->max_packet_size < 512) ||
	    (ohci->max_packet_size > 4096)) {
		
#ifdef CONFIG_IEEE1394_VERBOSEDEBUG
		int i;
#endif

		PRINT(KERN_DEBUG, "Serial EEPROM has suspicious values, "
                      "attempting to set max_packet_size to 512 bytes");
		reg_write(ohci, OHCI1394_BusOptions,
			  (reg_read(ohci, OHCI1394_BusOptions) & 0xf007) | 0x8002);
		ohci->max_packet_size = 512;
#ifdef CONFIG_IEEE1394_VERBOSEDEBUG
		PRINT(KERN_DEBUG, "    EEPROM Present: %d",
		      (reg_read(ohci, OHCI1394_Version) >> 24) & 0x1);
		reg_write(ohci, OHCI1394_GUID_ROM, 0x80000000);

		for (i = 0;
		     ((i < 1000) &&
		      (reg_read(ohci, OHCI1394_GUID_ROM) & 0x80000000)); i++)
			udelay(10);

		for (i = 0; i < 0x20; i++) {
			reg_write(ohci, OHCI1394_GUID_ROM, 0x02000000);
			PRINT(KERN_DEBUG, "    EEPROM %02x: %02x", i,
			      (reg_read(ohci, OHCI1394_GUID_ROM) >> 16) & 0xff);
		}
#endif
	}
}


static void insert_packet(struct ti_ohci *ohci,
			  struct dma_trm_ctx *d, struct hpsb_packet *packet)
{
	u32 cycleTimer;
	int idx = d->prg_ind;

	DBGMSG("Inserting packet for node " NODE_BUS_FMT
	       ", tlabel=%d, tcode=0x%x, speed=%d",
	       NODE_BUS_ARGS(ohci->host, packet->node_id), packet->tlabel,
	       packet->tcode, packet->speed_code);

	d->prg_cpu[idx]->begin.address = 0;
	d->prg_cpu[idx]->begin.branchAddress = 0;

	if (d->type == DMA_CTX_ASYNC_RESP) {
		
		cycleTimer = reg_read(ohci, OHCI1394_IsochronousCycleTimer);
		d->prg_cpu[idx]->begin.status = cpu_to_le32(
			(((((cycleTimer>>25)&0x7)+1)&0x7)<<13) |
			((cycleTimer&0x01fff000)>>12));

		DBGMSG("cycleTimer: %08x timeStamp: %08x",
		       cycleTimer, d->prg_cpu[idx]->begin.status);
	} else 
		d->prg_cpu[idx]->begin.status = 0;

        if ( (packet->type == hpsb_async) || (packet->type == hpsb_raw) ) {

                if (packet->type == hpsb_raw) {
			d->prg_cpu[idx]->data[0] = cpu_to_le32(OHCI1394_TCODE_PHY<<4);
                        d->prg_cpu[idx]->data[1] = cpu_to_le32(packet->header[0]);
                        d->prg_cpu[idx]->data[2] = cpu_to_le32(packet->header[1]);
                } else {
                        d->prg_cpu[idx]->data[0] = packet->speed_code<<16 |
                                (packet->header[0] & 0xFFFF);

			if (packet->tcode == TCODE_ISO_DATA) {
				
				d->prg_cpu[idx]->data[1] = packet->header[0] & 0xFFFF0000;
			} else {
				
				d->prg_cpu[idx]->data[1] =
					(packet->header[1] & 0xFFFF) |
					(packet->header[0] & 0xFFFF0000);
				d->prg_cpu[idx]->data[2] = packet->header[2];
				d->prg_cpu[idx]->data[3] = packet->header[3];
			}
			header_le32_to_cpu(d->prg_cpu[idx]->data, packet->tcode);
                }

                if (packet->data_size) { 
			if (packet->tcode == TCODE_STREAM_DATA){
				d->prg_cpu[idx]->begin.control =
					cpu_to_le32(DMA_CTL_OUTPUT_MORE |
						    DMA_CTL_IMMEDIATE | 0x8);
			} else {
				d->prg_cpu[idx]->begin.control =
					cpu_to_le32(DMA_CTL_OUTPUT_MORE |
						    DMA_CTL_IMMEDIATE | 0x10);
			}
                        d->prg_cpu[idx]->end.control =
                                cpu_to_le32(DMA_CTL_OUTPUT_LAST |
					    DMA_CTL_IRQ |
					    DMA_CTL_BRANCH |
					    packet->data_size);
                        
#if 0
                        if (cross_bound((unsigned long)packet->data,
                                        packet->data_size)>0) {
                                
                                PRINT(KERN_ERR,
                                      "%s: packet data addr: %p size %Zd bytes "
				      "cross page boundary", __func__,
                                      packet->data, packet->data_size);
                        }
#endif
                        d->prg_cpu[idx]->end.address = cpu_to_le32(
                                pci_map_single(ohci->dev, packet->data,
                                               packet->data_size,
                                               PCI_DMA_TODEVICE));

                        d->prg_cpu[idx]->end.branchAddress = 0;
                        d->prg_cpu[idx]->end.status = 0;
                        if (d->branchAddrPtr)
                                *(d->branchAddrPtr) =
					cpu_to_le32(d->prg_bus[idx] | 0x3);
                        d->branchAddrPtr =
                                &(d->prg_cpu[idx]->end.branchAddress);
                } else { 
                        if (packet->type == hpsb_raw)
                                d->prg_cpu[idx]->begin.control =
					cpu_to_le32(DMA_CTL_OUTPUT_LAST |
						    DMA_CTL_IMMEDIATE |
						    DMA_CTL_IRQ |
						    DMA_CTL_BRANCH |
						    (packet->header_size + 4));
                        else
                                d->prg_cpu[idx]->begin.control =
					cpu_to_le32(DMA_CTL_OUTPUT_LAST |
						    DMA_CTL_IMMEDIATE |
						    DMA_CTL_IRQ |
						    DMA_CTL_BRANCH |
						    packet->header_size);

                        if (d->branchAddrPtr)
                                *(d->branchAddrPtr) =
					cpu_to_le32(d->prg_bus[idx] | 0x2);
                        d->branchAddrPtr =
                                &(d->prg_cpu[idx]->begin.branchAddress);
                }

        } else { 
                d->prg_cpu[idx]->data[0] = packet->speed_code<<16 |
                        (packet->header[0] & 0xFFFF);
                d->prg_cpu[idx]->data[1] = packet->header[0] & 0xFFFF0000;
		header_le32_to_cpu(d->prg_cpu[idx]->data, packet->tcode);

                d->prg_cpu[idx]->begin.control =
			cpu_to_le32(DMA_CTL_OUTPUT_MORE |
				    DMA_CTL_IMMEDIATE | 0x8);
                d->prg_cpu[idx]->end.control =
			cpu_to_le32(DMA_CTL_OUTPUT_LAST |
				    DMA_CTL_UPDATE |
				    DMA_CTL_IRQ |
				    DMA_CTL_BRANCH |
				    packet->data_size);
                d->prg_cpu[idx]->end.address = cpu_to_le32(
				pci_map_single(ohci->dev, packet->data,
				packet->data_size, PCI_DMA_TODEVICE));

                d->prg_cpu[idx]->end.branchAddress = 0;
                d->prg_cpu[idx]->end.status = 0;
                DBGMSG("Iso xmit context info: header[%08x %08x]\n"
                       "                       begin=%08x %08x %08x %08x\n"
                       "                             %08x %08x %08x %08x\n"
                       "                       end  =%08x %08x %08x %08x",
                       d->prg_cpu[idx]->data[0], d->prg_cpu[idx]->data[1],
                       d->prg_cpu[idx]->begin.control,
                       d->prg_cpu[idx]->begin.address,
                       d->prg_cpu[idx]->begin.branchAddress,
                       d->prg_cpu[idx]->begin.status,
                       d->prg_cpu[idx]->data[0],
                       d->prg_cpu[idx]->data[1],
                       d->prg_cpu[idx]->data[2],
                       d->prg_cpu[idx]->data[3],
                       d->prg_cpu[idx]->end.control,
                       d->prg_cpu[idx]->end.address,
                       d->prg_cpu[idx]->end.branchAddress,
                       d->prg_cpu[idx]->end.status);
                if (d->branchAddrPtr)
  		        *(d->branchAddrPtr) = cpu_to_le32(d->prg_bus[idx] | 0x3);
                d->branchAddrPtr = &(d->prg_cpu[idx]->end.branchAddress);
        }
	d->free_prgs--;

	
	list_add_tail(&packet->driver_list, &d->fifo_list);
	d->prg_ind = (d->prg_ind + 1) % d->num_desc;
}


static void dma_trm_flush(struct ti_ohci *ohci, struct dma_trm_ctx *d)
{
	struct hpsb_packet *packet, *ptmp;
	int idx = d->prg_ind;
	int z = 0;

	
	list_for_each_entry_safe(packet, ptmp, &d->pending_list, driver_list) {
		if (!d->free_prgs)
			break;

		
		if (!z)
			z = (packet->data_size) ? 3 : 2;

		
		list_del_init(&packet->driver_list);
		insert_packet(ohci, d, packet);
	}

	
	if (z == 0)
		return;

	
	if (!(reg_read(ohci, d->ctrlSet) & 0x8000)) {
		u32 nodeId = reg_read(ohci, OHCI1394_NodeID);

		DBGMSG("Starting transmit DMA ctx=%d",d->ctx);
		reg_write(ohci, d->cmdPtr, d->prg_bus[idx] | z);

		
		if (!(nodeId & 0x80000000) || (nodeId & 0x3f) == 63)
			PRINT(KERN_ERR, "Running dma failed because Node ID is not valid");
		else
			reg_write(ohci, d->ctrlSet, 0x8000);
	} else {
		
		if (!(reg_read(ohci, d->ctrlSet) & 0x400))
			DBGMSG("Waking transmit DMA ctx=%d",d->ctx);

		
		reg_write(ohci, d->ctrlSet, 0x1000);
	}

	return;
}


static int ohci_transmit(struct hpsb_host *host, struct hpsb_packet *packet)
{
	struct ti_ohci *ohci = host->hostdata;
	struct dma_trm_ctx *d;
	unsigned long flags;

	if (packet->data_size > ohci->max_packet_size) {
		PRINT(KERN_ERR,
		      "Transmit packet size %Zd is too big",
		      packet->data_size);
		return -EOVERFLOW;
	}

	if (packet->type == hpsb_raw)
		d = &ohci->at_req_context;
	else if ((packet->tcode & 0x02) && (packet->tcode != TCODE_ISO_DATA))
		d = &ohci->at_resp_context;
	else
		d = &ohci->at_req_context;

	spin_lock_irqsave(&d->lock,flags);

	list_add_tail(&packet->driver_list, &d->pending_list);

	dma_trm_flush(ohci, d);

	spin_unlock_irqrestore(&d->lock,flags);

	return 0;
}

static int ohci_devctl(struct hpsb_host *host, enum devctl_cmd cmd, int arg)
{
	struct ti_ohci *ohci = host->hostdata;
	int retval = 0, phy_reg;

	switch (cmd) {
	case RESET_BUS:
		switch (arg) {
		case SHORT_RESET:
			phy_reg = get_phy_reg(ohci, 5);
			phy_reg |= 0x40;
			set_phy_reg(ohci, 5, phy_reg); 
			break;
		case LONG_RESET:
			phy_reg = get_phy_reg(ohci, 1);
			phy_reg |= 0x40;
			set_phy_reg(ohci, 1, phy_reg); 
			break;
		case SHORT_RESET_NO_FORCE_ROOT:
			phy_reg = get_phy_reg(ohci, 1);
			if (phy_reg & 0x80) {
				phy_reg &= ~0x80;
				set_phy_reg(ohci, 1, phy_reg); 
			}

			phy_reg = get_phy_reg(ohci, 5);
			phy_reg |= 0x40;
			set_phy_reg(ohci, 5, phy_reg); 
			break;
		case LONG_RESET_NO_FORCE_ROOT:
			phy_reg = get_phy_reg(ohci, 1);
			phy_reg &= ~0x80;
			phy_reg |= 0x40;
			set_phy_reg(ohci, 1, phy_reg); 
			break;
		case SHORT_RESET_FORCE_ROOT:
			phy_reg = get_phy_reg(ohci, 1);
			if (!(phy_reg & 0x80)) {
				phy_reg |= 0x80;
				set_phy_reg(ohci, 1, phy_reg); 
			}

			phy_reg = get_phy_reg(ohci, 5);
			phy_reg |= 0x40;
			set_phy_reg(ohci, 5, phy_reg); 
			break;
		case LONG_RESET_FORCE_ROOT:
			phy_reg = get_phy_reg(ohci, 1);
			phy_reg |= 0xc0;
			set_phy_reg(ohci, 1, phy_reg); 
			break;
		default:
			retval = -1;
		}
		break;

	case GET_CYCLE_COUNTER:
		retval = reg_read(ohci, OHCI1394_IsochronousCycleTimer);
		break;

	case SET_CYCLE_COUNTER:
		reg_write(ohci, OHCI1394_IsochronousCycleTimer, arg);
		break;

	case SET_BUS_ID:
		PRINT(KERN_ERR, "devctl command SET_BUS_ID err");
		break;

	case ACT_CYCLE_MASTER:
		if (arg) {
			
			u32 nodeId = reg_read(ohci, OHCI1394_NodeID);
			if ((nodeId & (1<<30)) && (nodeId & 0x3f)) {
				
				DBGMSG("Cycle master enabled");
				reg_write(ohci, OHCI1394_LinkControlSet,
					  OHCI1394_LinkControl_CycleTimerEnable |
					  OHCI1394_LinkControl_CycleMaster);
			}
		} else {
			
			reg_write(ohci, OHCI1394_LinkControlClear,
				  OHCI1394_LinkControl_CycleTimerEnable |
				  OHCI1394_LinkControl_CycleMaster |
				  OHCI1394_LinkControl_CycleSource);
		}
		break;

	case CANCEL_REQUESTS:
		DBGMSG("Cancel request received");
		dma_trm_reset(&ohci->at_req_context);
		dma_trm_reset(&ohci->at_resp_context);
		break;

	default:
		PRINT_G(KERN_ERR, "ohci_devctl cmd %d not implemented yet",
			cmd);
		break;
	}
	return retval;
}





struct ohci_iso_recv {
	struct ti_ohci *ohci;

	struct ohci1394_iso_tasklet task;
	int task_active;

	enum { BUFFER_FILL_MODE = 0,
	       PACKET_PER_BUFFER_MODE = 1 } dma_mode;

	
	struct dma_prog_region prog;
	struct dma_cmd *block; 

	
	unsigned int nblocks;

	
	unsigned int buf_stride;

	
	int block_irq_interval;

	
	int block_dma;

	
	int block_reader;

	
	int released_bytes;

	
	int dma_offset;

	
	u32 ContextControlSet;
	u32 ContextControlClear;
	u32 CommandPtr;
	u32 ContextMatch;
};

static void ohci_iso_recv_task(unsigned long data);
static void ohci_iso_recv_stop(struct hpsb_iso *iso);
static void ohci_iso_recv_shutdown(struct hpsb_iso *iso);
static int  ohci_iso_recv_start(struct hpsb_iso *iso, int cycle, int tag_mask, int sync);
static void ohci_iso_recv_program(struct hpsb_iso *iso);

static int ohci_iso_recv_init(struct hpsb_iso *iso)
{
	struct ti_ohci *ohci = iso->host->hostdata;
	struct ohci_iso_recv *recv;
	int ctx;
	int ret = -ENOMEM;

	recv = kmalloc(sizeof(*recv), GFP_KERNEL);
	if (!recv)
		return -ENOMEM;

	iso->hostdata = recv;
	recv->ohci = ohci;
	recv->task_active = 0;
	dma_prog_region_init(&recv->prog);
	recv->block = NULL;

	

	if (((iso->irq_interval == 1 && iso->dma_mode == HPSB_ISO_DMA_OLD_ABI) ||
	     iso->dma_mode == HPSB_ISO_DMA_PACKET_PER_BUFFER) && iso->channel != -1) {
		recv->dma_mode = PACKET_PER_BUFFER_MODE;
	} else {
		recv->dma_mode = BUFFER_FILL_MODE;
	}

	

	if (recv->dma_mode == BUFFER_FILL_MODE) {
		recv->buf_stride = PAGE_SIZE;

		
		recv->nblocks = iso->buf_size/PAGE_SIZE - 1;
		if (recv->nblocks < 3) {
			DBGMSG("ohci_iso_recv_init: DMA buffer too small");
			goto err;
		}

		
		if (iso->irq_interval == 1)
			recv->block_irq_interval = 1;
		else
			recv->block_irq_interval = iso->irq_interval *
							((recv->nblocks+1)/iso->buf_packets);
		if (recv->block_irq_interval*4 > recv->nblocks)
			recv->block_irq_interval = recv->nblocks/4;
		if (recv->block_irq_interval < 1)
			recv->block_irq_interval = 1;

	} else {
		int max_packet_size;

		recv->nblocks = iso->buf_packets;
		recv->block_irq_interval = iso->irq_interval;
		if (recv->block_irq_interval * 4 > iso->buf_packets)
			recv->block_irq_interval = iso->buf_packets / 4;
		if (recv->block_irq_interval < 1)
		recv->block_irq_interval = 1;

		
		

		max_packet_size = iso->buf_size / iso->buf_packets;

		for (recv->buf_stride = 8; recv->buf_stride < max_packet_size;
		    recv->buf_stride *= 2);

		if (recv->buf_stride*iso->buf_packets > iso->buf_size ||
		   recv->buf_stride > PAGE_SIZE) {
			
			DBGMSG("ohci_iso_recv_init: problem choosing a buffer stride");
			goto err;
		}
	}

	recv->block_reader = 0;
	recv->released_bytes = 0;
	recv->block_dma = 0;
	recv->dma_offset = 0;

	
	if (dma_prog_region_alloc(&recv->prog,
				 sizeof(struct dma_cmd) * recv->nblocks,
				 recv->ohci->dev))
		goto err;

	recv->block = (struct dma_cmd*) recv->prog.kvirt;

	ohci1394_init_iso_tasklet(&recv->task,
				  iso->channel == -1 ? OHCI_ISO_MULTICHANNEL_RECEIVE :
				                       OHCI_ISO_RECEIVE,
				  ohci_iso_recv_task, (unsigned long) iso);

	if (ohci1394_register_iso_tasklet(recv->ohci, &recv->task) < 0) {
		ret = -EBUSY;
		goto err;
	}

	recv->task_active = 1;

	
	ctx = recv->task.context;
	recv->ContextControlSet = OHCI1394_IsoRcvContextControlSet + 32 * ctx;
	recv->ContextControlClear = OHCI1394_IsoRcvContextControlClear + 32 * ctx;
	recv->CommandPtr = OHCI1394_IsoRcvCommandPtr + 32 * ctx;
	recv->ContextMatch = OHCI1394_IsoRcvContextMatch + 32 * ctx;

	if (iso->channel == -1) {
		
		reg_write(recv->ohci, OHCI1394_IRMultiChanMaskHiClear, 0xFFFFFFFF);
		reg_write(recv->ohci, OHCI1394_IRMultiChanMaskLoClear, 0xFFFFFFFF);
	}

	
	ohci_iso_recv_program(iso);

	DBGMSG("ohci_iso_recv_init: %s mode, DMA buffer is %lu pages"
	       " (%u bytes), using %u blocks, buf_stride %u, block_irq_interval %d",
	       recv->dma_mode == BUFFER_FILL_MODE ?
	       "buffer-fill" : "packet-per-buffer",
	       iso->buf_size/PAGE_SIZE, iso->buf_size,
	       recv->nblocks, recv->buf_stride, recv->block_irq_interval);

	return 0;

err:
	ohci_iso_recv_shutdown(iso);
	return ret;
}

static void ohci_iso_recv_stop(struct hpsb_iso *iso)
{
	struct ohci_iso_recv *recv = iso->hostdata;

	
	reg_write(recv->ohci, OHCI1394_IsoRecvIntMaskClear, 1 << recv->task.context);

	
	ohci1394_stop_context(recv->ohci, recv->ContextControlClear, NULL);
}

static void ohci_iso_recv_shutdown(struct hpsb_iso *iso)
{
	struct ohci_iso_recv *recv = iso->hostdata;

	if (recv->task_active) {
		ohci_iso_recv_stop(iso);
		ohci1394_unregister_iso_tasklet(recv->ohci, &recv->task);
		recv->task_active = 0;
	}

	dma_prog_region_free(&recv->prog);
	kfree(recv);
	iso->hostdata = NULL;
}


static void ohci_iso_recv_program(struct hpsb_iso *iso)
{
	struct ohci_iso_recv *recv = iso->hostdata;
	int blk;

	
	u32 *prev_branch = NULL;

	for (blk = 0; blk < recv->nblocks; blk++) {
		u32 control;

		
		struct dma_cmd *cmd = &recv->block[blk];

		
		unsigned long prog_offset = blk * sizeof(struct dma_cmd);

		
		unsigned long buf_offset = blk * recv->buf_stride;

		if (recv->dma_mode == BUFFER_FILL_MODE) {
			control = 2 << 28; 
		} else {
			control = 3 << 28; 
		}

		control |= 8 << 24; 

		
		if (blk == recv->nblocks-1 || (blk % recv->block_irq_interval) == 0) {
			control |= 3 << 20; 
		}

		control |= 3 << 18; 
		control |= recv->buf_stride;

		cmd->control = cpu_to_le32(control);
		cmd->address = cpu_to_le32(dma_region_offset_to_bus(&iso->data_buf, buf_offset));
		cmd->branchAddress = 0; 
		cmd->status = cpu_to_le32(recv->buf_stride);

		
		if (prev_branch) {
			*prev_branch = cpu_to_le32(dma_prog_region_offset_to_bus(&recv->prog, prog_offset) | 1);
		}

		prev_branch = &cmd->branchAddress;
	}

	
}


static void ohci_iso_recv_change_channel(struct hpsb_iso *iso, unsigned char channel, int listen)
{
	struct ohci_iso_recv *recv = iso->hostdata;
	int reg, i;

	if (channel < 32) {
		reg = listen ? OHCI1394_IRMultiChanMaskLoSet : OHCI1394_IRMultiChanMaskLoClear;
		i = channel;
	} else {
		reg = listen ? OHCI1394_IRMultiChanMaskHiSet : OHCI1394_IRMultiChanMaskHiClear;
		i = channel - 32;
	}

	reg_write(recv->ohci, reg, (1 << i));

	
	mb();
	reg_read(recv->ohci, OHCI1394_IsochronousCycleTimer);
}

static void ohci_iso_recv_set_channel_mask(struct hpsb_iso *iso, u64 mask)
{
	struct ohci_iso_recv *recv = iso->hostdata;
	int i;

	for (i = 0; i < 64; i++) {
		if (mask & (1ULL << i)) {
			if (i < 32)
				reg_write(recv->ohci, OHCI1394_IRMultiChanMaskLoSet, (1 << i));
			else
				reg_write(recv->ohci, OHCI1394_IRMultiChanMaskHiSet, (1 << (i-32)));
		} else {
			if (i < 32)
				reg_write(recv->ohci, OHCI1394_IRMultiChanMaskLoClear, (1 << i));
			else
				reg_write(recv->ohci, OHCI1394_IRMultiChanMaskHiClear, (1 << (i-32)));
		}
	}

	
	mb();
	reg_read(recv->ohci, OHCI1394_IsochronousCycleTimer);
}

static int ohci_iso_recv_start(struct hpsb_iso *iso, int cycle, int tag_mask, int sync)
{
	struct ohci_iso_recv *recv = iso->hostdata;
	struct ti_ohci *ohci = recv->ohci;
	u32 command, contextMatch;

	reg_write(recv->ohci, recv->ContextControlClear, 0xFFFFFFFF);
	wmb();

	
	command = (1 << 30);

	if (recv->dma_mode == BUFFER_FILL_MODE)
		command |= (1 << 31);

	reg_write(recv->ohci, recv->ContextControlSet, command);

	
	contextMatch = tag_mask << 28;

	if (iso->channel == -1) {
		
		reg_write(recv->ohci, recv->ContextControlSet, (1 << 28));
	} else {
		
		contextMatch |= iso->channel;
	}

	if (cycle != -1) {
		u32 seconds;

		
		reg_write(recv->ohci, recv->ContextControlSet, (1 << 29));

		
		cycle &= 0x1FFF;

		
		seconds = reg_read(recv->ohci, OHCI1394_IsochronousCycleTimer) >> 25;

		
		seconds += 1;

		cycle |= (seconds & 3) << 13;

		contextMatch |= cycle << 12;
	}

	if (sync != -1) {
		
		struct dma_cmd *cmd = &recv->block[recv->block_dma];
		cmd->control |= cpu_to_le32(DMA_CTL_WAIT);

		
		contextMatch |= (sync&0xf)<<8;
	}

	reg_write(recv->ohci, recv->ContextMatch, contextMatch);

	
	command = dma_prog_region_offset_to_bus(&recv->prog,
						recv->block_dma * sizeof(struct dma_cmd));
	command |= 1; 

	reg_write(recv->ohci, recv->CommandPtr, command);

	
	reg_write(recv->ohci, OHCI1394_IsoRecvIntMaskSet, 1 << recv->task.context);

	wmb();

	
	reg_write(recv->ohci, recv->ContextControlSet, 0x8000);

	
	mb();
	reg_read(recv->ohci, OHCI1394_IsochronousCycleTimer);

	
	if (!(reg_read(recv->ohci, recv->ContextControlSet) & 0x8000)) {
		PRINT(KERN_ERR,
		      "Error starting IR DMA (ContextControl 0x%08x)\n",
		      reg_read(recv->ohci, recv->ContextControlSet));
		return -1;
	}

	return 0;
}

static void ohci_iso_recv_release_block(struct ohci_iso_recv *recv, int block)
{
	
	

	int next_i = block;
	int prev_i = (next_i == 0) ? (recv->nblocks - 1) : (next_i - 1);

	struct dma_cmd *next = &recv->block[next_i];
	struct dma_cmd *prev = &recv->block[prev_i];
	
	
	if ((block < 0) || (block > recv->nblocks))
		return;

	
	next->branchAddress = 0;
	next->control |= cpu_to_le32(3 << 20);
	next->status = cpu_to_le32(recv->buf_stride);

	
	prev->branchAddress = cpu_to_le32(dma_prog_region_offset_to_bus(&recv->prog,
									sizeof(struct dma_cmd) * next_i)
					  | 1); 

	
	if ((prev_i % recv->block_irq_interval) == 0) {
		prev->control |= cpu_to_le32(3 << 20); 
	} else {
		prev->control &= cpu_to_le32(~(3<<20)); 
	}
	wmb();

	
	reg_write(recv->ohci, recv->ContextControlSet, (1 << 12));
}

static void ohci_iso_recv_bufferfill_release(struct ohci_iso_recv *recv,
					     struct hpsb_iso_packet_info *info)
{
	
	recv->released_bytes += info->total_len;

	
	while (recv->released_bytes > recv->buf_stride) {
		ohci_iso_recv_release_block(recv, recv->block_reader);
		recv->block_reader = (recv->block_reader + 1) % recv->nblocks;
		recv->released_bytes -= recv->buf_stride;
	}
}

static inline void ohci_iso_recv_release(struct hpsb_iso *iso, struct hpsb_iso_packet_info *info)
{
	struct ohci_iso_recv *recv = iso->hostdata;
	if (recv->dma_mode == BUFFER_FILL_MODE) {
		ohci_iso_recv_bufferfill_release(recv, info);
	} else {
		ohci_iso_recv_release_block(recv, info - iso->infos);
	}
}


static void ohci_iso_recv_bufferfill_parse(struct hpsb_iso *iso, struct ohci_iso_recv *recv)
{
	int wake = 0;
	int runaway = 0;
	struct ti_ohci *ohci = recv->ohci;

	while (1) {
		
		

		unsigned int offset;
		unsigned short len, cycle, total_len;
		unsigned char channel, tag, sy;

		unsigned char *p = iso->data_buf.kvirt;

		unsigned int this_block = recv->dma_offset/recv->buf_stride;

		
		if (runaway++ > 100000) {
			atomic_inc(&iso->overflows);
			PRINT(KERN_ERR,
			      "IR DMA error - Runaway during buffer parsing!\n");
			break;
		}

		
		if (this_block == recv->block_dma)
			break;

		wake = 1;

		

		

		len = p[recv->dma_offset+2] | (p[recv->dma_offset+3] << 8);

		if (len > 4096) {
			PRINT(KERN_ERR,
			      "IR DMA error - bogus 'len' value %u\n", len);
		}

		channel = p[recv->dma_offset+1] & 0x3F;
		tag = p[recv->dma_offset+1] >> 6;
		sy = p[recv->dma_offset+0] & 0xF;

		
		recv->dma_offset += 4;

		
		if (recv->dma_offset >= recv->buf_stride*recv->nblocks) {
			recv->dma_offset -= recv->buf_stride*recv->nblocks;
		}

		
		offset = recv->dma_offset;

		
		recv->dma_offset += len;

		total_len = len + 8; 
		
		if (len % 4) {
			recv->dma_offset += 4 - (len%4);
			total_len += 4 - (len%4);
		}

		
		if (recv->dma_offset >= recv->buf_stride*recv->nblocks) {
			

			int guard_off = recv->buf_stride*recv->nblocks;
			int tail_len = len - (guard_off - offset);

			if (tail_len > 0  && tail_len < recv->buf_stride) {
				memcpy(iso->data_buf.kvirt + guard_off,
				       iso->data_buf.kvirt,
				       tail_len);
			}

			recv->dma_offset -= recv->buf_stride*recv->nblocks;
		}

		
		cycle = p[recv->dma_offset+0] | (p[recv->dma_offset+1]<<8);
		cycle &= 0x1FFF;

		
		recv->dma_offset += 4;

		
		if (recv->dma_offset >= recv->buf_stride*recv->nblocks) {
			recv->dma_offset -= recv->buf_stride*recv->nblocks;
		}

		hpsb_iso_packet_received(iso, offset, len, total_len, cycle, channel, tag, sy);
	}

	if (wake)
		hpsb_iso_wake(iso);
}

static void ohci_iso_recv_bufferfill_task(struct hpsb_iso *iso, struct ohci_iso_recv *recv)
{
	int loop;
	struct ti_ohci *ohci = recv->ohci;

	
	for (loop = 0; loop < recv->nblocks; loop++) {

		
		struct dma_cmd *im = &recv->block[recv->block_dma];

		
		u16 xferstatus = le32_to_cpu(im->status) >> 16;

		
		u16 rescount = le32_to_cpu(im->status) & 0xFFFF;

		unsigned char event = xferstatus & 0x1F;

		if (!event) {
			
			break;
		}

		if (event != 0x11) {
			atomic_inc(&iso->overflows);
			PRINT(KERN_ERR,
			      "IR DMA error - OHCI error code 0x%02x\n", event);
		}

		if (rescount != 0) {
			
			break;
		}

		

		
		dma_region_sync_for_cpu(&iso->data_buf, recv->block_dma*recv->buf_stride, recv->buf_stride);

		
		im->status = recv->buf_stride;

		
		recv->block_dma = (recv->block_dma + 1) % recv->nblocks;

		if ((recv->block_dma+1) % recv->nblocks == recv->block_reader) {
			atomic_inc(&iso->overflows);
			DBGMSG("ISO reception overflow - "
			       "ran out of DMA blocks");
		}
	}

	
	ohci_iso_recv_bufferfill_parse(iso, recv);
}

static void ohci_iso_recv_packetperbuf_task(struct hpsb_iso *iso, struct ohci_iso_recv *recv)
{
	int count;
	int wake = 0;
	struct ti_ohci *ohci = recv->ohci;

	
	for (count = 0; count < recv->nblocks; count++) {
		u32 packet_len = 0;

		
		struct dma_cmd *il = ((struct dma_cmd*) recv->prog.kvirt) + iso->pkt_dma;

		
		u16 xferstatus = le32_to_cpu(il->status) >> 16;
		u16 rescount = le32_to_cpu(il->status) & 0xFFFF;

		unsigned char event = xferstatus & 0x1F;

		if (!event) {
			
			goto out;
		}

		if (event == 0x11) {
			

			
			packet_len = recv->buf_stride - rescount;

		} else if (event == 0x02) {
			PRINT(KERN_ERR, "IR DMA error - packet too long for buffer\n");
		} else if (event) {
			PRINT(KERN_ERR, "IR DMA error - OHCI error code 0x%02x\n", event);
		}

		
		dma_region_sync_for_cpu(&iso->data_buf, iso->pkt_dma * recv->buf_stride, recv->buf_stride);

		
		{
			
			unsigned char *hdr;

			unsigned int offset;
			unsigned short cycle;
			unsigned char channel, tag, sy;

			offset = iso->pkt_dma * recv->buf_stride;
			hdr = iso->data_buf.kvirt + offset;

			
			offset += 8;
			packet_len -= 8;

			cycle = (hdr[0] | (hdr[1] << 8)) & 0x1FFF;
			channel = hdr[5] & 0x3F;
			tag = hdr[5] >> 6;
			sy = hdr[4] & 0xF;

			hpsb_iso_packet_received(iso, offset, packet_len,
					recv->buf_stride, cycle, channel, tag, sy);
		}

		
		il->status = recv->buf_stride;

		wake = 1;
		recv->block_dma = iso->pkt_dma;
	}

out:
	if (wake)
		hpsb_iso_wake(iso);
}

static void ohci_iso_recv_task(unsigned long data)
{
	struct hpsb_iso *iso = (struct hpsb_iso*) data;
	struct ohci_iso_recv *recv = iso->hostdata;

	if (recv->dma_mode == BUFFER_FILL_MODE)
		ohci_iso_recv_bufferfill_task(iso, recv);
	else
		ohci_iso_recv_packetperbuf_task(iso, recv);
}



struct ohci_iso_xmit {
	struct ti_ohci *ohci;
	struct dma_prog_region prog;
	struct ohci1394_iso_tasklet task;
	int task_active;
	int last_cycle;
	atomic_t skips;

	u32 ContextControlSet;
	u32 ContextControlClear;
	u32 CommandPtr;
};



struct iso_xmit_cmd {
	struct dma_cmd output_more_immediate;
	u8 iso_hdr[8];
	u32 unused[2];
	struct dma_cmd output_last;
};

static int ohci_iso_xmit_init(struct hpsb_iso *iso);
static int ohci_iso_xmit_start(struct hpsb_iso *iso, int cycle);
static void ohci_iso_xmit_shutdown(struct hpsb_iso *iso);
static void ohci_iso_xmit_task(unsigned long data);

static int ohci_iso_xmit_init(struct hpsb_iso *iso)
{
	struct ohci_iso_xmit *xmit;
	unsigned int prog_size;
	int ctx;
	int ret = -ENOMEM;

	xmit = kmalloc(sizeof(*xmit), GFP_KERNEL);
	if (!xmit)
		return -ENOMEM;

	iso->hostdata = xmit;
	xmit->ohci = iso->host->hostdata;
	xmit->task_active = 0;
	xmit->last_cycle = -1;
	atomic_set(&iso->skips, 0);

	dma_prog_region_init(&xmit->prog);

	prog_size = sizeof(struct iso_xmit_cmd) * iso->buf_packets;

	if (dma_prog_region_alloc(&xmit->prog, prog_size, xmit->ohci->dev))
		goto err;

	ohci1394_init_iso_tasklet(&xmit->task, OHCI_ISO_TRANSMIT,
				  ohci_iso_xmit_task, (unsigned long) iso);

	if (ohci1394_register_iso_tasklet(xmit->ohci, &xmit->task) < 0) {
		ret = -EBUSY;
		goto err;
	}

	xmit->task_active = 1;

	
	ctx = xmit->task.context;
	xmit->ContextControlSet = OHCI1394_IsoXmitContextControlSet + 16 * ctx;
	xmit->ContextControlClear = OHCI1394_IsoXmitContextControlClear + 16 * ctx;
	xmit->CommandPtr = OHCI1394_IsoXmitCommandPtr + 16 * ctx;

	return 0;

err:
	ohci_iso_xmit_shutdown(iso);
	return ret;
}

static void ohci_iso_xmit_stop(struct hpsb_iso *iso)
{
	struct ohci_iso_xmit *xmit = iso->hostdata;
	struct ti_ohci *ohci = xmit->ohci;

	
	reg_write(xmit->ohci, OHCI1394_IsoXmitIntMaskClear, 1 << xmit->task.context);

	
	if (ohci1394_stop_context(xmit->ohci, xmit->ContextControlClear, NULL)) {
		
		PRINT(KERN_ERR,
		      "you probably exceeded the OHCI card's bandwidth limit - "
		      "reload the module and reduce xmit bandwidth");
	}
}

static void ohci_iso_xmit_shutdown(struct hpsb_iso *iso)
{
	struct ohci_iso_xmit *xmit = iso->hostdata;

	if (xmit->task_active) {
		ohci_iso_xmit_stop(iso);
		ohci1394_unregister_iso_tasklet(xmit->ohci, &xmit->task);
		xmit->task_active = 0;
	}

	dma_prog_region_free(&xmit->prog);
	kfree(xmit);
	iso->hostdata = NULL;
}

static void ohci_iso_xmit_task(unsigned long data)
{
	struct hpsb_iso *iso = (struct hpsb_iso*) data;
	struct ohci_iso_xmit *xmit = iso->hostdata;
	struct ti_ohci *ohci = xmit->ohci;
	int wake = 0;
	int count;

	
	for (count = 0; count < iso->buf_packets; count++) {
		int cycle;

		
		struct iso_xmit_cmd *cmd = dma_region_i(&xmit->prog, struct iso_xmit_cmd, iso->pkt_dma);

		
		u16 xferstatus = le32_to_cpu(cmd->output_last.status) >> 16;
		u8  event = xferstatus & 0x1F;

		if (!event) {
			
			break;
		}

		if (event != 0x11)
			PRINT(KERN_ERR,
			      "IT DMA error - OHCI error code 0x%02x\n", event);

		
		wake = 1;

		
		cycle = le32_to_cpu(cmd->output_last.status) & 0x1FFF;

		if (xmit->last_cycle > -1) {
			int cycle_diff = cycle - xmit->last_cycle;
			int skip;

			
			if (cycle_diff < 0) {
				cycle_diff += 8000;
				if (cycle_diff < 0)
					PRINT(KERN_ERR, "bogus cycle diff %d\n",
					      cycle_diff);
			}

			skip = cycle_diff - 1;
			if (skip > 0) {
				DBGMSG("skipped %d cycles without packet loss", skip);
				atomic_add(skip, &iso->skips);
			}
		}
		xmit->last_cycle = cycle;

		
		hpsb_iso_packet_sent(iso, cycle, event != 0x11);

		
		cmd->output_last.status = 0;
	}

	if (wake)
		hpsb_iso_wake(iso);
}

static int ohci_iso_xmit_queue(struct hpsb_iso *iso, struct hpsb_iso_packet_info *info)
{
	struct ohci_iso_xmit *xmit = iso->hostdata;
	struct ti_ohci *ohci = xmit->ohci;

	int next_i, prev_i;
	struct iso_xmit_cmd *next, *prev;

	unsigned int offset;
	unsigned short len;
	unsigned char tag, sy;

	
	if (cross_bound(info->offset, info->len)) {
		PRINT(KERN_ERR,
		      "rawiso xmit: packet %u crosses a page boundary",
		      iso->first_packet);
		return -EINVAL;
	}

	offset = info->offset;
	len = info->len;
	tag = info->tag;
	sy = info->sy;

	
	dma_region_sync_for_device(&iso->data_buf, offset, len);

	
	
	

	next_i = iso->first_packet;
	prev_i = (next_i == 0) ? (iso->buf_packets - 1) : (next_i - 1);

	next = dma_region_i(&xmit->prog, struct iso_xmit_cmd, next_i);
	prev = dma_region_i(&xmit->prog, struct iso_xmit_cmd, prev_i);

	
	memset(next, 0, sizeof(struct iso_xmit_cmd));
	next->output_more_immediate.control = cpu_to_le32(0x02000008);

	

	
	next->iso_hdr[0] = 0xA0 | (sy & 0xF);

	
	next->iso_hdr[1] = (tag << 6) | (iso->channel & 0x3F);

	
	next->iso_hdr[2] = iso->speed & 0x7;

	
	next->iso_hdr[6] = len & 0xFF;
	next->iso_hdr[7] = len >> 8;

	
	next->output_last.control = cpu_to_le32(1 << 28);
	next->output_last.control |= cpu_to_le32(1 << 27); 
	next->output_last.control |= cpu_to_le32(3 << 20); 
	next->output_last.control |= cpu_to_le32(3 << 18); 
	next->output_last.control |= cpu_to_le32(len);

	
	next->output_last.address = cpu_to_le32(dma_region_offset_to_bus(&iso->data_buf, offset));

	

	

	
	prev->output_last.branchAddress = cpu_to_le32(
		dma_prog_region_offset_to_bus(&xmit->prog, sizeof(struct iso_xmit_cmd) * next_i) | 3);

	
	next->output_more_immediate.branchAddress =
			prev->output_last.branchAddress;

	
	if (prev_i % iso->irq_interval) {
		prev->output_last.control &= cpu_to_le32(~(3 << 20)); 
	} else {
		prev->output_last.control |= cpu_to_le32(3 << 20); 
	}

	wmb();

	
	reg_write(xmit->ohci, xmit->ContextControlSet, 1 << 12);

	
	mb();
	reg_read(xmit->ohci, OHCI1394_IsochronousCycleTimer);

	return 0;
}

static int ohci_iso_xmit_start(struct hpsb_iso *iso, int cycle)
{
	struct ohci_iso_xmit *xmit = iso->hostdata;
	struct ti_ohci *ohci = xmit->ohci;

	
	reg_write(xmit->ohci, xmit->ContextControlClear, 0xFFFFFFFF);
	wmb();

	
	reg_write(xmit->ohci, xmit->CommandPtr,
		  dma_prog_region_offset_to_bus(&xmit->prog, iso->pkt_dma * sizeof(struct iso_xmit_cmd)) | 3);

	
	if (cycle != -1) {
		u32 start = cycle & 0x1FFF;

		
		u32 seconds = reg_read(xmit->ohci, OHCI1394_IsochronousCycleTimer) >> 25;

		
		seconds += 1;

		start |= (seconds & 3) << 13;

		reg_write(xmit->ohci, xmit->ContextControlSet, 0x80000000 | (start << 16));
	}

	
	reg_write(xmit->ohci, OHCI1394_IsoXmitIntMaskSet, 1 << xmit->task.context);

	
	reg_write(xmit->ohci, xmit->ContextControlSet, 0x8000);
	mb();

	
	udelay(100);

	
	if (!(reg_read(xmit->ohci, xmit->ContextControlSet) & 0x8000)) {
		PRINT(KERN_ERR, "Error starting IT DMA (ContextControl 0x%08x)\n",
		      reg_read(xmit->ohci, xmit->ContextControlSet));
		return -1;
	}

	return 0;
}

static int ohci_isoctl(struct hpsb_iso *iso, enum isoctl_cmd cmd, unsigned long arg)
{

	switch(cmd) {
	case XMIT_INIT:
		return ohci_iso_xmit_init(iso);
	case XMIT_START:
		return ohci_iso_xmit_start(iso, arg);
	case XMIT_STOP:
		ohci_iso_xmit_stop(iso);
		return 0;
	case XMIT_QUEUE:
		return ohci_iso_xmit_queue(iso, (struct hpsb_iso_packet_info*) arg);
	case XMIT_SHUTDOWN:
		ohci_iso_xmit_shutdown(iso);
		return 0;

	case RECV_INIT:
		return ohci_iso_recv_init(iso);
	case RECV_START: {
		int *args = (int*) arg;
		return ohci_iso_recv_start(iso, args[0], args[1], args[2]);
	}
	case RECV_STOP:
		ohci_iso_recv_stop(iso);
		return 0;
	case RECV_RELEASE:
		ohci_iso_recv_release(iso, (struct hpsb_iso_packet_info*) arg);
		return 0;
	case RECV_FLUSH:
		ohci_iso_recv_task((unsigned long) iso);
		return 0;
	case RECV_SHUTDOWN:
		ohci_iso_recv_shutdown(iso);
		return 0;
	case RECV_LISTEN_CHANNEL:
		ohci_iso_recv_change_channel(iso, arg, 1);
		return 0;
	case RECV_UNLISTEN_CHANNEL:
		ohci_iso_recv_change_channel(iso, arg, 0);
		return 0;
	case RECV_SET_CHANNEL_MASK:
		ohci_iso_recv_set_channel_mask(iso, *((u64*) arg));
		return 0;

	default:
		PRINT_G(KERN_ERR, "ohci_isoctl cmd %d not implemented yet",
			cmd);
		break;
	}
	return -EINVAL;
}






static void dma_trm_reset(struct dma_trm_ctx *d)
{
	unsigned long flags;
	LIST_HEAD(packet_list);
	struct ti_ohci *ohci = d->ohci;
	struct hpsb_packet *packet, *ptmp;

	ohci1394_stop_context(ohci, d->ctrlClear, NULL);

	

	spin_lock_irqsave(&d->lock, flags);

	list_splice_init(&d->fifo_list, &packet_list);
	list_splice_init(&d->pending_list, &packet_list);

	d->branchAddrPtr = NULL;
	d->sent_ind = d->prg_ind;
	d->free_prgs = d->num_desc;

	spin_unlock_irqrestore(&d->lock, flags);

	if (list_empty(&packet_list))
		return;

	PRINT(KERN_INFO, "AT dma reset ctx=%d, aborting transmission", d->ctx);

	
	list_for_each_entry_safe(packet, ptmp, &packet_list, driver_list) {
		list_del_init(&packet->driver_list);
		hpsb_packet_sent(ohci->host, packet, ACKX_ABORTED);
	}
}

static void ohci_schedule_iso_tasklets(struct ti_ohci *ohci,
				       quadlet_t rx_event,
				       quadlet_t tx_event)
{
	struct ohci1394_iso_tasklet *t;
	unsigned long mask;
	unsigned long flags;

	spin_lock_irqsave(&ohci->iso_tasklet_list_lock, flags);

	list_for_each_entry(t, &ohci->iso_tasklet_list, link) {
		mask = 1 << t->context;

		if (t->type == OHCI_ISO_TRANSMIT) {
			if (tx_event & mask)
				tasklet_schedule(&t->tasklet);
		} else {
			
			if (rx_event & mask)
				tasklet_schedule(&t->tasklet);
		}
	}

	spin_unlock_irqrestore(&ohci->iso_tasklet_list_lock, flags);
}

static irqreturn_t ohci_irq_handler(int irq, void *dev_id)
{
	quadlet_t event, node_id;
	struct ti_ohci *ohci = (struct ti_ohci *)dev_id;
	struct hpsb_host *host = ohci->host;
	int phyid = -1, isroot = 0;
	unsigned long flags;

	
	spin_lock_irqsave(&ohci->event_lock, flags);
	event = reg_read(ohci, OHCI1394_IntEventClear);
	reg_write(ohci, OHCI1394_IntEventClear, event & ~OHCI1394_busReset);
	spin_unlock_irqrestore(&ohci->event_lock, flags);

	if (!event)
		return IRQ_NONE;

	
	if (event == ~(u32) 0) {
		DBGMSG("Device removed.");
		return IRQ_NONE;
	}

	DBGMSG("IntEvent: %08x", event);

	if (event & OHCI1394_unrecoverableError) {
		int ctx;
		PRINT(KERN_ERR, "Unrecoverable error!");

		if (reg_read(ohci, OHCI1394_AsReqTrContextControlSet) & 0x800)
			PRINT(KERN_ERR, "Async Req Tx Context died: "
				"ctrl[%08x] cmdptr[%08x]",
				reg_read(ohci, OHCI1394_AsReqTrContextControlSet),
				reg_read(ohci, OHCI1394_AsReqTrCommandPtr));

		if (reg_read(ohci, OHCI1394_AsRspTrContextControlSet) & 0x800)
			PRINT(KERN_ERR, "Async Rsp Tx Context died: "
				"ctrl[%08x] cmdptr[%08x]",
				reg_read(ohci, OHCI1394_AsRspTrContextControlSet),
				reg_read(ohci, OHCI1394_AsRspTrCommandPtr));

		if (reg_read(ohci, OHCI1394_AsReqRcvContextControlSet) & 0x800)
			PRINT(KERN_ERR, "Async Req Rcv Context died: "
				"ctrl[%08x] cmdptr[%08x]",
				reg_read(ohci, OHCI1394_AsReqRcvContextControlSet),
				reg_read(ohci, OHCI1394_AsReqRcvCommandPtr));

		if (reg_read(ohci, OHCI1394_AsRspRcvContextControlSet) & 0x800)
			PRINT(KERN_ERR, "Async Rsp Rcv Context died: "
				"ctrl[%08x] cmdptr[%08x]",
				reg_read(ohci, OHCI1394_AsRspRcvContextControlSet),
				reg_read(ohci, OHCI1394_AsRspRcvCommandPtr));

		for (ctx = 0; ctx < ohci->nb_iso_xmit_ctx; ctx++) {
			if (reg_read(ohci, OHCI1394_IsoXmitContextControlSet + (16 * ctx)) & 0x800)
				PRINT(KERN_ERR, "Iso Xmit %d Context died: "
					"ctrl[%08x] cmdptr[%08x]", ctx,
					reg_read(ohci, OHCI1394_IsoXmitContextControlSet + (16 * ctx)),
					reg_read(ohci, OHCI1394_IsoXmitCommandPtr + (16 * ctx)));
		}

		for (ctx = 0; ctx < ohci->nb_iso_rcv_ctx; ctx++) {
			if (reg_read(ohci, OHCI1394_IsoRcvContextControlSet + (32 * ctx)) & 0x800)
				PRINT(KERN_ERR, "Iso Recv %d Context died: "
					"ctrl[%08x] cmdptr[%08x] match[%08x]", ctx,
					reg_read(ohci, OHCI1394_IsoRcvContextControlSet + (32 * ctx)),
					reg_read(ohci, OHCI1394_IsoRcvCommandPtr + (32 * ctx)),
					reg_read(ohci, OHCI1394_IsoRcvContextMatch + (32 * ctx)));
		}

		event &= ~OHCI1394_unrecoverableError;
	}
	if (event & OHCI1394_postedWriteErr) {
		PRINT(KERN_ERR, "physical posted write error");
		
		event &= ~OHCI1394_postedWriteErr;
	}
	if (event & OHCI1394_cycleTooLong) {
		if(printk_ratelimit())
			PRINT(KERN_WARNING, "isochronous cycle too long");
		else
			DBGMSG("OHCI1394_cycleTooLong");
		reg_write(ohci, OHCI1394_LinkControlSet,
			  OHCI1394_LinkControl_CycleMaster);
		event &= ~OHCI1394_cycleTooLong;
	}
	if (event & OHCI1394_cycleInconsistent) {
		
		DBGMSG("OHCI1394_cycleInconsistent");
		event &= ~OHCI1394_cycleInconsistent;
	}
	if (event & OHCI1394_busReset) {
		
		spin_lock_irqsave(&ohci->event_lock, flags);
		reg_write(ohci, OHCI1394_IntMaskClear, OHCI1394_busReset);

		if (ohci->check_busreset) {
			int loop_count = 0;

			udelay(10);

			while (reg_read(ohci, OHCI1394_IntEventSet) & OHCI1394_busReset) {
				reg_write(ohci, OHCI1394_IntEventClear, OHCI1394_busReset);

				spin_unlock_irqrestore(&ohci->event_lock, flags);
				udelay(10);
				spin_lock_irqsave(&ohci->event_lock, flags);

				
				if (loop_count > 10000) {
					ohci_devctl(host, RESET_BUS, LONG_RESET);
					DBGMSG("Detected bus-reset loop. Forced a bus reset!");
					loop_count = 0;
				}

				loop_count++;
			}
		}
		spin_unlock_irqrestore(&ohci->event_lock, flags);
		if (!host->in_bus_reset) {
			DBGMSG("irq_handler: Bus reset requested");

			
			hpsb_bus_reset(ohci->host);
		}
		event &= ~OHCI1394_busReset;
	}
	if (event & OHCI1394_reqTxComplete) {
		struct dma_trm_ctx *d = &ohci->at_req_context;
		DBGMSG("Got reqTxComplete interrupt "
		       "status=0x%08X", reg_read(ohci, d->ctrlSet));
		if (reg_read(ohci, d->ctrlSet) & 0x800)
			ohci1394_stop_context(ohci, d->ctrlClear,
					      "reqTxComplete");
		else
			dma_trm_tasklet((unsigned long)d);
			
		event &= ~OHCI1394_reqTxComplete;
	}
	if (event & OHCI1394_respTxComplete) {
		struct dma_trm_ctx *d = &ohci->at_resp_context;
		DBGMSG("Got respTxComplete interrupt "
		       "status=0x%08X", reg_read(ohci, d->ctrlSet));
		if (reg_read(ohci, d->ctrlSet) & 0x800)
			ohci1394_stop_context(ohci, d->ctrlClear,
					      "respTxComplete");
		else
			tasklet_schedule(&d->task);
		event &= ~OHCI1394_respTxComplete;
	}
	if (event & OHCI1394_RQPkt) {
		struct dma_rcv_ctx *d = &ohci->ar_req_context;
		DBGMSG("Got RQPkt interrupt status=0x%08X",
		       reg_read(ohci, d->ctrlSet));
		if (reg_read(ohci, d->ctrlSet) & 0x800)
			ohci1394_stop_context(ohci, d->ctrlClear, "RQPkt");
		else
			tasklet_schedule(&d->task);
		event &= ~OHCI1394_RQPkt;
	}
	if (event & OHCI1394_RSPkt) {
		struct dma_rcv_ctx *d = &ohci->ar_resp_context;
		DBGMSG("Got RSPkt interrupt status=0x%08X",
		       reg_read(ohci, d->ctrlSet));
		if (reg_read(ohci, d->ctrlSet) & 0x800)
			ohci1394_stop_context(ohci, d->ctrlClear, "RSPkt");
		else
			tasklet_schedule(&d->task);
		event &= ~OHCI1394_RSPkt;
	}
	if (event & OHCI1394_isochRx) {
		quadlet_t rx_event;

		rx_event = reg_read(ohci, OHCI1394_IsoRecvIntEventSet);
		reg_write(ohci, OHCI1394_IsoRecvIntEventClear, rx_event);
		ohci_schedule_iso_tasklets(ohci, rx_event, 0);
		event &= ~OHCI1394_isochRx;
	}
	if (event & OHCI1394_isochTx) {
		quadlet_t tx_event;

		tx_event = reg_read(ohci, OHCI1394_IsoXmitIntEventSet);
		reg_write(ohci, OHCI1394_IsoXmitIntEventClear, tx_event);
		ohci_schedule_iso_tasklets(ohci, 0, tx_event);
		event &= ~OHCI1394_isochTx;
	}
	if (event & OHCI1394_selfIDComplete) {
		if (host->in_bus_reset) {
			node_id = reg_read(ohci, OHCI1394_NodeID);

			if (!(node_id & 0x80000000)) {
				PRINT(KERN_ERR,
				      "SelfID received, but NodeID invalid "
				      "(probably new bus reset occurred): %08X",
				      node_id);
				goto selfid_not_valid;
			}

			phyid =  node_id & 0x0000003f;
			isroot = (node_id & 0x40000000) != 0;

			DBGMSG("SelfID interrupt received "
			      "(phyid %d, %s)", phyid,
			      (isroot ? "root" : "not root"));

			handle_selfid(ohci, host, phyid, isroot);

			
			spin_lock_irqsave(&ohci->event_lock, flags);
			reg_write(ohci, OHCI1394_IntEventClear, OHCI1394_busReset);
			reg_write(ohci, OHCI1394_IntMaskSet, OHCI1394_busReset);
			spin_unlock_irqrestore(&ohci->event_lock, flags);

			
			if (phys_dma) {
				reg_write(ohci, OHCI1394_PhyReqFilterHiSet,
					  0xffffffff);
				reg_write(ohci, OHCI1394_PhyReqFilterLoSet,
					  0xffffffff);
			}

			DBGMSG("PhyReqFilter=%08x%08x",
			       reg_read(ohci, OHCI1394_PhyReqFilterHiSet),
			       reg_read(ohci, OHCI1394_PhyReqFilterLoSet));

			hpsb_selfid_complete(host, phyid, isroot);
		} else
			PRINT(KERN_ERR,
			      "SelfID received outside of bus reset sequence");

selfid_not_valid:
		event &= ~OHCI1394_selfIDComplete;
	}

	
	if (event)
		PRINT(KERN_ERR, "Unhandled interrupt(s) 0x%08x",
		      event);

	return IRQ_HANDLED;
}


static void insert_dma_buffer(struct dma_rcv_ctx *d, int idx)
{
	struct ti_ohci *ohci = (struct ti_ohci*)(d->ohci);
	DBGMSG("Inserting dma buf ctx=%d idx=%d", d->ctx, idx);

	d->prg_cpu[idx]->status = cpu_to_le32(d->buf_size);
	d->prg_cpu[idx]->branchAddress &= le32_to_cpu(0xfffffff0);
	idx = (idx + d->num_desc - 1 ) % d->num_desc;
	d->prg_cpu[idx]->branchAddress |= le32_to_cpu(0x00000001);

	
	wmb();
	
	
	if (!(reg_read(ohci, d->ctrlSet) & 0x400)) {
		PRINT(KERN_INFO,
		      "Waking dma ctx=%d ... processing is probably too slow",
		      d->ctx);
	}

	
	reg_write(ohci, d->ctrlSet, 0x1000);
}

#define cond_le32_to_cpu(data, noswap) \
	(noswap ? data : le32_to_cpu(data))

static const int TCODE_SIZE[16] = {20, 0, 16, -1, 16, 20, 20, 0,
			    -1, 0, -1, 0, -1, -1, 16, -1};


static inline int packet_length(struct dma_rcv_ctx *d, int idx,
				quadlet_t *buf_ptr, int offset,
				unsigned char tcode, int noswap)
{
	int length = -1;

	if (d->type == DMA_CTX_ASYNC_REQ || d->type == DMA_CTX_ASYNC_RESP) {
		length = TCODE_SIZE[tcode];
		if (length == 0) {
			if (offset + 12 >= d->buf_size) {
				length = (cond_le32_to_cpu(d->buf_cpu[(idx + 1) % d->num_desc]
						[3 - ((d->buf_size - offset) >> 2)], noswap) >> 16);
			} else {
				length = (cond_le32_to_cpu(buf_ptr[3], noswap) >> 16);
			}
			length += 20;
		}
	} else if (d->type == DMA_CTX_ISO) {
		
		length = (cond_le32_to_cpu(buf_ptr[0], noswap) >> 16) + 8;
	}

	if (length > 0 && length % 4)
		length += 4 - (length % 4);

	return length;
}


static void dma_rcv_tasklet (unsigned long data)
{
	struct dma_rcv_ctx *d = (struct dma_rcv_ctx*)data;
	struct ti_ohci *ohci = (struct ti_ohci*)(d->ohci);
	unsigned int split_left, idx, offset, rescount;
	unsigned char tcode;
	int length, bytes_left, ack;
	unsigned long flags;
	quadlet_t *buf_ptr;
	char *split_ptr;
	char msg[256];

	spin_lock_irqsave(&d->lock, flags);

	idx = d->buf_ind;
	offset = d->buf_offset;
	buf_ptr = d->buf_cpu[idx] + offset/4;

	rescount = le32_to_cpu(d->prg_cpu[idx]->status) & 0xffff;
	bytes_left = d->buf_size - rescount - offset;

	while (bytes_left > 0) {
		tcode = (cond_le32_to_cpu(buf_ptr[0], ohci->no_swap_incoming) >> 4) & 0xf;

		
		length = packet_length(d, idx, buf_ptr, offset, tcode, ohci->no_swap_incoming);

		if (length < 4) { 
			sprintf(msg,"Unexpected tcode 0x%x(0x%08x) in AR ctx=%d, length=%d",
				tcode, cond_le32_to_cpu(buf_ptr[0], ohci->no_swap_incoming),
				d->ctx, length);
			ohci1394_stop_context(ohci, d->ctrlClear, msg);
			spin_unlock_irqrestore(&d->lock, flags);
			return;
		}

		
		if ((offset + length) > d->buf_size) {
			DBGMSG("Split packet rcv'd");
			if (length > d->split_buf_size) {
				ohci1394_stop_context(ohci, d->ctrlClear,
					     "Split packet size exceeded");
				d->buf_ind = idx;
				d->buf_offset = offset;
				spin_unlock_irqrestore(&d->lock, flags);
				return;
			}

			if (le32_to_cpu(d->prg_cpu[(idx+1)%d->num_desc]->status)
			    == d->buf_size) {
				
				PRINT(KERN_INFO,
				      "Got only half a packet!");
				d->buf_ind = idx;
				d->buf_offset = offset;
				spin_unlock_irqrestore(&d->lock, flags);
				return;
			}

			split_left = length;
			split_ptr = (char *)d->spb;
			memcpy(split_ptr,buf_ptr,d->buf_size-offset);
			split_left -= d->buf_size-offset;
			split_ptr += d->buf_size-offset;
			insert_dma_buffer(d, idx);
			idx = (idx+1) % d->num_desc;
			buf_ptr = d->buf_cpu[idx];
			offset=0;

			while (split_left >= d->buf_size) {
				memcpy(split_ptr,buf_ptr,d->buf_size);
				split_ptr += d->buf_size;
				split_left -= d->buf_size;
				insert_dma_buffer(d, idx);
				idx = (idx+1) % d->num_desc;
				buf_ptr = d->buf_cpu[idx];
			}

			if (split_left > 0) {
				memcpy(split_ptr, buf_ptr, split_left);
				offset = split_left;
				buf_ptr += offset/4;
			}
		} else {
			DBGMSG("Single packet rcv'd");
			memcpy(d->spb, buf_ptr, length);
			offset += length;
			buf_ptr += length/4;
			if (offset==d->buf_size) {
				insert_dma_buffer(d, idx);
				idx = (idx+1) % d->num_desc;
				buf_ptr = d->buf_cpu[idx];
				offset=0;
			}
		}

		
		if (tcode != OHCI1394_TCODE_PHY) {
			if (!ohci->no_swap_incoming)
				header_le32_to_cpu(d->spb, tcode);
			DBGMSG("Packet received from node"
				" %d ack=0x%02X spd=%d tcode=0x%X"
				" length=%d ctx=%d tlabel=%d",
				(d->spb[1]>>16)&0x3f,
				(cond_le32_to_cpu(d->spb[length/4-1], ohci->no_swap_incoming)>>16)&0x1f,
				(cond_le32_to_cpu(d->spb[length/4-1], ohci->no_swap_incoming)>>21)&0x3,
				tcode, length, d->ctx,
				(d->spb[0]>>10)&0x3f);

			ack = (((cond_le32_to_cpu(d->spb[length/4-1], ohci->no_swap_incoming)>>16)&0x1f)
				== 0x11) ? 1 : 0;

			hpsb_packet_received(ohci->host, d->spb,
					     length-4, ack);
		}
#ifdef OHCI1394_DEBUG
		else
			PRINT (KERN_DEBUG, "Got phy packet ctx=%d ... discarded",
			       d->ctx);
#endif

	       	rescount = le32_to_cpu(d->prg_cpu[idx]->status) & 0xffff;

		bytes_left = d->buf_size - rescount - offset;

	}

	d->buf_ind = idx;
	d->buf_offset = offset;

	spin_unlock_irqrestore(&d->lock, flags);
}


static void dma_trm_tasklet (unsigned long data)
{
	struct dma_trm_ctx *d = (struct dma_trm_ctx*)data;
	struct ti_ohci *ohci = (struct ti_ohci*)(d->ohci);
	struct hpsb_packet *packet, *ptmp;
	unsigned long flags;
	u32 status, ack;
        size_t datasize;

	spin_lock_irqsave(&d->lock, flags);

	list_for_each_entry_safe(packet, ptmp, &d->fifo_list, driver_list) {
                datasize = packet->data_size;
		if (datasize && packet->type != hpsb_raw)
			status = le32_to_cpu(
				d->prg_cpu[d->sent_ind]->end.status) >> 16;
		else
			status = le32_to_cpu(
				d->prg_cpu[d->sent_ind]->begin.status) >> 16;

		if (status == 0)
			
			break;

#ifdef OHCI1394_DEBUG
		if (datasize)
			if (((le32_to_cpu(d->prg_cpu[d->sent_ind]->data[0])>>4)&0xf) == 0xa)
				DBGMSG("Stream packet sent to channel %d tcode=0x%X "
				       "ack=0x%X spd=%d dataLength=%d ctx=%d",
				       (le32_to_cpu(d->prg_cpu[d->sent_ind]->data[0])>>8)&0x3f,
				       (le32_to_cpu(d->prg_cpu[d->sent_ind]->data[0])>>4)&0xf,
				       status&0x1f, (status>>5)&0x3,
				       le32_to_cpu(d->prg_cpu[d->sent_ind]->data[1])>>16,
				       d->ctx);
			else
				DBGMSG("Packet sent to node %d tcode=0x%X tLabel="
				       "%d ack=0x%X spd=%d dataLength=%d ctx=%d",
				       (le32_to_cpu(d->prg_cpu[d->sent_ind]->data[1])>>16)&0x3f,
				       (le32_to_cpu(d->prg_cpu[d->sent_ind]->data[0])>>4)&0xf,
				       (le32_to_cpu(d->prg_cpu[d->sent_ind]->data[0])>>10)&0x3f,
				       status&0x1f, (status>>5)&0x3,
				       le32_to_cpu(d->prg_cpu[d->sent_ind]->data[3])>>16,
				       d->ctx);
		else
			DBGMSG("Packet sent to node %d tcode=0x%X tLabel="
			       "%d ack=0x%X spd=%d data=0x%08X ctx=%d",
                                (le32_to_cpu(d->prg_cpu[d->sent_ind]->data[1])
                                        >>16)&0x3f,
                                (le32_to_cpu(d->prg_cpu[d->sent_ind]->data[0])
                                        >>4)&0xf,
                                (le32_to_cpu(d->prg_cpu[d->sent_ind]->data[0])
                                        >>10)&0x3f,
                                status&0x1f, (status>>5)&0x3,
                                le32_to_cpu(d->prg_cpu[d->sent_ind]->data[3]),
                                d->ctx);
#endif

		if (status & 0x10) {
			ack = status & 0xf;
		} else {
			switch (status & 0x1f) {
			case EVT_NO_STATUS: 
			case EVT_RESERVED_A: 
			case EVT_LONG_PACKET: 
				PRINT(KERN_WARNING, "Received OHCI evt_* error 0x%x", status & 0x1f);
				ack = ACKX_SEND_ERROR;
				break;
			case EVT_MISSING_ACK:
				ack = ACKX_TIMEOUT;
				break;
			case EVT_UNDERRUN:
				ack = ACKX_SEND_ERROR;
				break;
			case EVT_OVERRUN: 
				PRINT(KERN_WARNING, "Received OHCI evt_* error 0x%x", status & 0x1f);
				ack = ACKX_SEND_ERROR;
				break;
			case EVT_DESCRIPTOR_READ:
			case EVT_DATA_READ:
			case EVT_DATA_WRITE:
				ack = ACKX_SEND_ERROR;
				break;
			case EVT_BUS_RESET: 
				PRINT(KERN_WARNING, "Received OHCI evt_* error 0x%x", status & 0x1f);
				ack = ACKX_SEND_ERROR;
				break;
			case EVT_TIMEOUT:
				ack = ACKX_TIMEOUT;
				break;
			case EVT_TCODE_ERR:
				ack = ACKX_SEND_ERROR;
				break;
			case EVT_RESERVED_B: 
			case EVT_RESERVED_C: 
				PRINT(KERN_WARNING, "Received OHCI evt_* error 0x%x", status & 0x1f);
				ack = ACKX_SEND_ERROR;
				break;
			case EVT_UNKNOWN:
			case EVT_FLUSHED:
				ack = ACKX_SEND_ERROR;
				break;
			default:
				PRINT(KERN_ERR, "Unhandled OHCI evt_* error 0x%x", status & 0x1f);
				ack = ACKX_SEND_ERROR;
				BUG();
			}
		}

		list_del_init(&packet->driver_list);
		hpsb_packet_sent(ohci->host, packet, ack);

		if (datasize)
			pci_unmap_single(ohci->dev,
					 cpu_to_le32(d->prg_cpu[d->sent_ind]->end.address),
					 datasize, PCI_DMA_TODEVICE);

		d->sent_ind = (d->sent_ind+1)%d->num_desc;
		d->free_prgs++;
	}

	dma_trm_flush(ohci, d);

	spin_unlock_irqrestore(&d->lock, flags);
}

static void free_dma_rcv_ctx(struct dma_rcv_ctx *d)
{
	int i;
	struct ti_ohci *ohci = d->ohci;

	if (ohci == NULL)
		return;

	DBGMSG("Freeing dma_rcv_ctx %d", d->ctx);

	if (d->buf_cpu) {
		for (i=0; i<d->num_desc; i++)
			if (d->buf_cpu[i] && d->buf_bus[i])
				pci_free_consistent(
					ohci->dev, d->buf_size,
					d->buf_cpu[i], d->buf_bus[i]);
		kfree(d->buf_cpu);
		kfree(d->buf_bus);
	}
	if (d->prg_cpu) {
		for (i=0; i<d->num_desc; i++)
			if (d->prg_cpu[i] && d->prg_bus[i])
				pci_pool_free(d->prg_pool, d->prg_cpu[i],
					      d->prg_bus[i]);
		pci_pool_destroy(d->prg_pool);
		kfree(d->prg_cpu);
		kfree(d->prg_bus);
	}
	kfree(d->spb);

	
	d->ohci = NULL;
}

static int
alloc_dma_rcv_ctx(struct ti_ohci *ohci, struct dma_rcv_ctx *d,
		  enum context_type type, int ctx, int num_desc,
		  int buf_size, int split_buf_size, int context_base)
{
	int i, len;
	static int num_allocs;
	static char pool_name[20];

	d->ohci = ohci;
	d->type = type;
	d->ctx = ctx;

	d->num_desc = num_desc;
	d->buf_size = buf_size;
	d->split_buf_size = split_buf_size;

	d->ctrlSet = 0;
	d->ctrlClear = 0;
	d->cmdPtr = 0;

	d->buf_cpu = kzalloc(d->num_desc * sizeof(*d->buf_cpu), GFP_ATOMIC);
	d->buf_bus = kzalloc(d->num_desc * sizeof(*d->buf_bus), GFP_ATOMIC);

	if (d->buf_cpu == NULL || d->buf_bus == NULL) {
		PRINT(KERN_ERR, "Failed to allocate %s", "DMA buffer");
		free_dma_rcv_ctx(d);
		return -ENOMEM;
	}

	d->prg_cpu = kzalloc(d->num_desc * sizeof(*d->prg_cpu), GFP_ATOMIC);
	d->prg_bus = kzalloc(d->num_desc * sizeof(*d->prg_bus), GFP_ATOMIC);

	if (d->prg_cpu == NULL || d->prg_bus == NULL) {
		PRINT(KERN_ERR, "Failed to allocate %s", "DMA prg");
		free_dma_rcv_ctx(d);
		return -ENOMEM;
	}

	d->spb = kmalloc(d->split_buf_size, GFP_ATOMIC);

	if (d->spb == NULL) {
		PRINT(KERN_ERR, "Failed to allocate %s", "split buffer");
		free_dma_rcv_ctx(d);
		return -ENOMEM;
	}
	
	len = sprintf(pool_name, "ohci1394_rcv_prg");
	sprintf(pool_name+len, "%d", num_allocs);
	d->prg_pool = pci_pool_create(pool_name, ohci->dev,
				sizeof(struct dma_cmd), 4, 0);
	if(d->prg_pool == NULL)
	{
		PRINT(KERN_ERR, "pci_pool_create failed for %s", pool_name);
		free_dma_rcv_ctx(d);
		return -ENOMEM;
	}
	num_allocs++;

	for (i=0; i<d->num_desc; i++) {
		d->buf_cpu[i] = pci_alloc_consistent(ohci->dev,
						     d->buf_size,
						     d->buf_bus+i);

		if (d->buf_cpu[i] != NULL) {
			memset(d->buf_cpu[i], 0, d->buf_size);
		} else {
			PRINT(KERN_ERR,
			      "Failed to allocate %s", "DMA buffer");
			free_dma_rcv_ctx(d);
			return -ENOMEM;
		}

		d->prg_cpu[i] = pci_pool_alloc(d->prg_pool, GFP_KERNEL, d->prg_bus+i);

                if (d->prg_cpu[i] != NULL) {
                        memset(d->prg_cpu[i], 0, sizeof(struct dma_cmd));
		} else {
			PRINT(KERN_ERR,
			      "Failed to allocate %s", "DMA prg");
			free_dma_rcv_ctx(d);
			return -ENOMEM;
		}
	}

        spin_lock_init(&d->lock);

	d->ctrlSet = context_base + OHCI1394_ContextControlSet;
	d->ctrlClear = context_base + OHCI1394_ContextControlClear;
	d->cmdPtr = context_base + OHCI1394_ContextCommandPtr;

	tasklet_init(&d->task, dma_rcv_tasklet, (unsigned long) d);
	return 0;
}

static void free_dma_trm_ctx(struct dma_trm_ctx *d)
{
	int i;
	struct ti_ohci *ohci = d->ohci;

	if (ohci == NULL)
		return;

	DBGMSG("Freeing dma_trm_ctx %d", d->ctx);

	if (d->prg_cpu) {
		for (i=0; i<d->num_desc; i++)
			if (d->prg_cpu[i] && d->prg_bus[i])
				pci_pool_free(d->prg_pool, d->prg_cpu[i],
					      d->prg_bus[i]);
		pci_pool_destroy(d->prg_pool);
		kfree(d->prg_cpu);
		kfree(d->prg_bus);
	}

	
	d->ohci = NULL;
}

static int
alloc_dma_trm_ctx(struct ti_ohci *ohci, struct dma_trm_ctx *d,
		  enum context_type type, int ctx, int num_desc,
		  int context_base)
{
	int i, len;
	static char pool_name[20];
	static int num_allocs=0;

	d->ohci = ohci;
	d->type = type;
	d->ctx = ctx;
	d->num_desc = num_desc;
	d->ctrlSet = 0;
	d->ctrlClear = 0;
	d->cmdPtr = 0;

	d->prg_cpu = kzalloc(d->num_desc * sizeof(*d->prg_cpu), GFP_KERNEL);
	d->prg_bus = kzalloc(d->num_desc * sizeof(*d->prg_bus), GFP_KERNEL);

	if (d->prg_cpu == NULL || d->prg_bus == NULL) {
		PRINT(KERN_ERR, "Failed to allocate %s", "AT DMA prg");
		free_dma_trm_ctx(d);
		return -ENOMEM;
	}

	len = sprintf(pool_name, "ohci1394_trm_prg");
	sprintf(pool_name+len, "%d", num_allocs);
	d->prg_pool = pci_pool_create(pool_name, ohci->dev,
				sizeof(struct at_dma_prg), 4, 0);
	if (d->prg_pool == NULL) {
		PRINT(KERN_ERR, "pci_pool_create failed for %s", pool_name);
		free_dma_trm_ctx(d);
		return -ENOMEM;
	}
	num_allocs++;

	for (i = 0; i < d->num_desc; i++) {
		d->prg_cpu[i] = pci_pool_alloc(d->prg_pool, GFP_KERNEL, d->prg_bus+i);

                if (d->prg_cpu[i] != NULL) {
                        memset(d->prg_cpu[i], 0, sizeof(struct at_dma_prg));
		} else {
			PRINT(KERN_ERR,
			      "Failed to allocate %s", "AT DMA prg");
			free_dma_trm_ctx(d);
			return -ENOMEM;
		}
	}

        spin_lock_init(&d->lock);

	
	d->ctrlSet = context_base + OHCI1394_ContextControlSet;
	d->ctrlClear = context_base + OHCI1394_ContextControlClear;
	d->cmdPtr = context_base + OHCI1394_ContextCommandPtr;
	tasklet_init(&d->task, dma_trm_tasklet, (unsigned long)d);
	return 0;
}

static void ohci_set_hw_config_rom(struct hpsb_host *host, __be32 *config_rom)
{
	struct ti_ohci *ohci = host->hostdata;

	reg_write(ohci, OHCI1394_ConfigROMhdr, be32_to_cpu(config_rom[0]));
	reg_write(ohci, OHCI1394_BusOptions, be32_to_cpu(config_rom[2]));

	memcpy(ohci->csr_config_rom_cpu, config_rom, OHCI_CONFIG_ROM_LEN);
}


static quadlet_t ohci_hw_csr_reg(struct hpsb_host *host, int reg,
                                 quadlet_t data, quadlet_t compare)
{
	struct ti_ohci *ohci = host->hostdata;
	int i;

	reg_write(ohci, OHCI1394_CSRData, data);
	reg_write(ohci, OHCI1394_CSRCompareData, compare);
	reg_write(ohci, OHCI1394_CSRControl, reg & 0x3);

	for (i = 0; i < OHCI_LOOP_COUNT; i++) {
		if (reg_read(ohci, OHCI1394_CSRControl) & 0x80000000)
			break;

		mdelay(1);
	}

	return reg_read(ohci, OHCI1394_CSRData);
}

static struct hpsb_host_driver ohci1394_driver = {
	.owner =		THIS_MODULE,
	.name =			OHCI1394_DRIVER_NAME,
	.set_hw_config_rom =	ohci_set_hw_config_rom,
	.transmit_packet =	ohci_transmit,
	.devctl =		ohci_devctl,
	.isoctl =               ohci_isoctl,
	.hw_csr_reg =		ohci_hw_csr_reg,
};



#ifdef CONFIG_PPC_PMAC
static void ohci1394_pmac_on(struct pci_dev *dev)
{
	if (machine_is(powermac)) {
		struct device_node *ofn = pci_device_to_OF_node(dev);

		if (ofn) {
			pmac_call_feature(PMAC_FTR_1394_CABLE_POWER, ofn, 0, 1);
			pmac_call_feature(PMAC_FTR_1394_ENABLE, ofn, 0, 1);
		}
	}
}

static void ohci1394_pmac_off(struct pci_dev *dev)
{
	if (machine_is(powermac)) {
		struct device_node *ofn = pci_device_to_OF_node(dev);

		if (ofn) {
			pmac_call_feature(PMAC_FTR_1394_ENABLE, ofn, 0, 0);
			pmac_call_feature(PMAC_FTR_1394_CABLE_POWER, ofn, 0, 0);
		}
	}
}
#else
#define ohci1394_pmac_on(dev)
#define ohci1394_pmac_off(dev)
#endif 

static int __devinit ohci1394_pci_probe(struct pci_dev *dev,
					const struct pci_device_id *ent)
{
	struct hpsb_host *host;
	struct ti_ohci *ohci;	
	resource_size_t ohci_base;
	int err = -ENOMEM;

	ohci1394_pmac_on(dev);
	if (pci_enable_device(dev)) {
		PRINT_G(KERN_ERR, "Failed to enable OHCI hardware");
		err = -ENXIO;
		goto err;
	}
        pci_set_master(dev);

	host = hpsb_alloc_host(&ohci1394_driver, sizeof(struct ti_ohci), &dev->dev);
	if (!host) {
		PRINT_G(KERN_ERR, "Failed to allocate %s", "host structure");
		goto err;
	}
	ohci = host->hostdata;
	ohci->dev = dev;
	ohci->host = host;
	ohci->init_state = OHCI_INIT_ALLOC_HOST;
	host->pdev = dev;
	pci_set_drvdata(dev, ohci);

	
	pci_write_config_dword(dev, OHCI1394_PCI_HCI_Control, 0);

	
#ifndef __LITTLE_ENDIAN
	
	if (dev->vendor == PCI_VENDOR_ID_APPLE &&
	    dev->device == PCI_DEVICE_ID_APPLE_UNI_N_FW) {
		ohci->no_swap_incoming = 1;
		ohci->selfid_swap = 0;
	} else
		ohci->selfid_swap = 1;
#endif


#ifndef PCI_DEVICE_ID_NVIDIA_NFORCE2_FW
#define PCI_DEVICE_ID_NVIDIA_NFORCE2_FW 0x006e
#endif

	
	if ((dev->vendor == PCI_VENDOR_ID_APPLE &&
	     dev->device == PCI_DEVICE_ID_APPLE_UNI_N_FW) ||
	    (dev->vendor ==  PCI_VENDOR_ID_NVIDIA &&
	     dev->device == PCI_DEVICE_ID_NVIDIA_NFORCE2_FW))
		ohci->check_busreset = 1;

	
	ohci_base = pci_resource_start(dev, 0);
	if (pci_resource_len(dev, 0) < OHCI1394_REGISTER_SIZE)
		PRINT(KERN_WARNING, "PCI resource length of 0x%llx too small!",
		      (unsigned long long)pci_resource_len(dev, 0));

	if (!request_mem_region(ohci_base, OHCI1394_REGISTER_SIZE,
				OHCI1394_DRIVER_NAME)) {
		PRINT_G(KERN_ERR, "MMIO resource (0x%llx - 0x%llx) unavailable",
			(unsigned long long)ohci_base,
			(unsigned long long)ohci_base + OHCI1394_REGISTER_SIZE);
		goto err;
	}
	ohci->init_state = OHCI_INIT_HAVE_MEM_REGION;

	ohci->registers = ioremap(ohci_base, OHCI1394_REGISTER_SIZE);
	if (ohci->registers == NULL) {
		PRINT_G(KERN_ERR, "Failed to remap registers");
		err = -ENXIO;
		goto err;
	}
	ohci->init_state = OHCI_INIT_HAVE_IOMAPPING;
	DBGMSG("Remapped memory spaces reg 0x%p", ohci->registers);

	
	ohci->csr_config_rom_cpu =
		pci_alloc_consistent(ohci->dev, OHCI_CONFIG_ROM_LEN,
				     &ohci->csr_config_rom_bus);
	if (ohci->csr_config_rom_cpu == NULL) {
		PRINT_G(KERN_ERR, "Failed to allocate %s", "buffer config rom");
		goto err;
	}
	ohci->init_state = OHCI_INIT_HAVE_CONFIG_ROM_BUFFER;

	
	ohci->selfid_buf_cpu =
		pci_alloc_consistent(ohci->dev, OHCI1394_SI_DMA_BUF_SIZE,
                      &ohci->selfid_buf_bus);
	if (ohci->selfid_buf_cpu == NULL) {
		PRINT_G(KERN_ERR, "Failed to allocate %s", "self-ID buffer");
		goto err;
	}
	ohci->init_state = OHCI_INIT_HAVE_SELFID_BUFFER;

	if ((unsigned long)ohci->selfid_buf_cpu & 0x1fff)
		PRINT(KERN_INFO, "SelfID buffer %p is not aligned on "
		      "8Kb boundary... may cause problems on some CXD3222 chip",
		      ohci->selfid_buf_cpu);

	
	ohci->self_id_errors = 0;

	ohci->init_state = OHCI_INIT_HAVE_TXRX_BUFFERS__MAYBE;
	
	if (alloc_dma_rcv_ctx(ohci, &ohci->ar_req_context,
			      DMA_CTX_ASYNC_REQ, 0, AR_REQ_NUM_DESC,
			      AR_REQ_BUF_SIZE, AR_REQ_SPLIT_BUF_SIZE,
			      OHCI1394_AsReqRcvContextBase) < 0) {
		PRINT_G(KERN_ERR, "Failed to allocate %s", "AR Req context");
		goto err;
	}
	
	if (alloc_dma_rcv_ctx(ohci, &ohci->ar_resp_context,
			      DMA_CTX_ASYNC_RESP, 0, AR_RESP_NUM_DESC,
			      AR_RESP_BUF_SIZE, AR_RESP_SPLIT_BUF_SIZE,
			      OHCI1394_AsRspRcvContextBase) < 0) {
		PRINT_G(KERN_ERR, "Failed to allocate %s", "AR Resp context");
		goto err;
	}
	
	if (alloc_dma_trm_ctx(ohci, &ohci->at_req_context,
			      DMA_CTX_ASYNC_REQ, 0, AT_REQ_NUM_DESC,
			      OHCI1394_AsReqTrContextBase) < 0) {
		PRINT_G(KERN_ERR, "Failed to allocate %s", "AT Req context");
		goto err;
	}
	
	if (alloc_dma_trm_ctx(ohci, &ohci->at_resp_context,
			      DMA_CTX_ASYNC_RESP, 1, AT_RESP_NUM_DESC,
			      OHCI1394_AsRspTrContextBase) < 0) {
		PRINT_G(KERN_ERR, "Failed to allocate %s", "AT Resp context");
		goto err;
	}
	
	ohci_soft_reset(ohci);

	
	reg_write(ohci, OHCI1394_HCControlSet, OHCI1394_HCControl_LPS);

	
	reg_write(ohci, OHCI1394_IntEventClear, 0xffffffff);
	reg_write(ohci, OHCI1394_IntMaskClear, 0xffffffff);

	
	reg_read(ohci, OHCI1394_Version);
	msleep(50);

	
	ohci->nb_iso_rcv_ctx =
		get_nb_iso_ctx(ohci, OHCI1394_IsoRecvIntMaskSet);
	ohci->nb_iso_xmit_ctx =
		get_nb_iso_ctx(ohci, OHCI1394_IsoXmitIntMaskSet);

	
	ohci->ir_ctx_usage = ~0 << ohci->nb_iso_rcv_ctx;
	ohci->it_ctx_usage = ~0 << ohci->nb_iso_xmit_ctx;

	INIT_LIST_HEAD(&ohci->iso_tasklet_list);
	spin_lock_init(&ohci->iso_tasklet_list_lock);
	ohci->ISO_channel_usage = 0;
        spin_lock_init(&ohci->IR_channel_lock);

	spin_lock_init(&ohci->event_lock);

	
	err = request_irq(dev->irq, ohci_irq_handler, IRQF_SHARED,
			  OHCI1394_DRIVER_NAME, ohci);
	if (err) {
		PRINT_G(KERN_ERR, "Failed to allocate interrupt %d", dev->irq);
		goto err;
	}
	ohci->init_state = OHCI_INIT_HAVE_IRQ;
	ohci_initialize(ohci);

	
	host->csr.guid_hi = reg_read(ohci, OHCI1394_GUIDHi);
	host->csr.guid_lo = reg_read(ohci, OHCI1394_GUIDLo);
	host->csr.cyc_clk_acc = 100;  
	host->csr.max_rec = (reg_read(ohci, OHCI1394_BusOptions) >> 12) & 0xf;
	host->csr.lnk_spd = reg_read(ohci, OHCI1394_BusOptions) & 0x7;

	if (phys_dma) {
		host->low_addr_space =
			(u64) reg_read(ohci, OHCI1394_PhyUpperBound) << 16;
		if (!host->low_addr_space)
			host->low_addr_space = OHCI1394_PHYS_UPPER_BOUND_FIXED;
	}
	host->middle_addr_space = OHCI1394_MIDDLE_ADDRESS_SPACE;

	
	if (hpsb_add_host(host)) {
		PRINT_G(KERN_ERR, "Failed to register host with highlevel");
		goto err;
	}
	ohci->init_state = OHCI_INIT_DONE;

	return 0;
err:
	ohci1394_pci_remove(dev);
	return err;
}

static void ohci1394_pci_remove(struct pci_dev *dev)
{
	struct ti_ohci *ohci;
	struct device *device;

	ohci = pci_get_drvdata(dev);
	if (!ohci)
		goto out;

	device = get_device(&ohci->host->device);

	switch (ohci->init_state) {
	case OHCI_INIT_DONE:
		hpsb_remove_host(ohci->host);

		
		reg_write(ohci, OHCI1394_ConfigROMhdr, 0);
		reg_write(ohci, OHCI1394_BusOptions,
			  (reg_read(ohci, OHCI1394_BusOptions) & 0x0000f007) |
			  0x00ff0000);
		memset(ohci->csr_config_rom_cpu, 0, OHCI_CONFIG_ROM_LEN);

	case OHCI_INIT_HAVE_IRQ:
		
		reg_write(ohci, OHCI1394_IntMaskClear, 0xffffffff);
		reg_write(ohci, OHCI1394_IntEventClear, 0xffffffff);
		reg_write(ohci, OHCI1394_IsoXmitIntMaskClear, 0xffffffff);
		reg_write(ohci, OHCI1394_IsoXmitIntEventClear, 0xffffffff);
		reg_write(ohci, OHCI1394_IsoRecvIntMaskClear, 0xffffffff);
		reg_write(ohci, OHCI1394_IsoRecvIntEventClear, 0xffffffff);

		
		set_phy_reg(ohci, 4, ~0xc0 & get_phy_reg(ohci, 4));

		
		reg_write(ohci, OHCI1394_LinkControlClear, 0xffffffff);

		
		ohci_devctl(ohci->host, RESET_BUS, LONG_RESET_NO_FORCE_ROOT);

		
		ohci_soft_reset(ohci);
		free_irq(dev->irq, ohci);

	case OHCI_INIT_HAVE_TXRX_BUFFERS__MAYBE:
		
		free_dma_rcv_ctx(&ohci->ar_req_context);
		free_dma_rcv_ctx(&ohci->ar_resp_context);
		free_dma_trm_ctx(&ohci->at_req_context);
		free_dma_trm_ctx(&ohci->at_resp_context);

	case OHCI_INIT_HAVE_SELFID_BUFFER:
		pci_free_consistent(dev, OHCI1394_SI_DMA_BUF_SIZE,
				    ohci->selfid_buf_cpu,
				    ohci->selfid_buf_bus);

	case OHCI_INIT_HAVE_CONFIG_ROM_BUFFER:
		pci_free_consistent(dev, OHCI_CONFIG_ROM_LEN,
				    ohci->csr_config_rom_cpu,
				    ohci->csr_config_rom_bus);

	case OHCI_INIT_HAVE_IOMAPPING:
		iounmap(ohci->registers);

	case OHCI_INIT_HAVE_MEM_REGION:
		release_mem_region(pci_resource_start(dev, 0),
				   OHCI1394_REGISTER_SIZE);

	case OHCI_INIT_ALLOC_HOST:
		pci_set_drvdata(dev, NULL);
	}

	if (device)
		put_device(device);
out:
	ohci1394_pmac_off(dev);
}

#ifdef CONFIG_PM
static int ohci1394_pci_suspend(struct pci_dev *dev, pm_message_t state)
{
	int err;
	struct ti_ohci *ohci = pci_get_drvdata(dev);

	if (!ohci) {
		printk(KERN_ERR "%s: tried to suspend nonexisting host\n",
		       OHCI1394_DRIVER_NAME);
		return -ENXIO;
	}
	DBGMSG("suspend called");

	
	hpsb_bus_reset(ohci->host);

	
	reg_write(ohci, OHCI1394_ConfigROMhdr, 0);
	reg_write(ohci, OHCI1394_BusOptions,
		  (reg_read(ohci, OHCI1394_BusOptions) & 0x0000f007) |
		  0x00ff0000);
	reg_write(ohci, OHCI1394_IntMaskClear, 0xffffffff);
	reg_write(ohci, OHCI1394_IntEventClear, 0xffffffff);
	reg_write(ohci, OHCI1394_IsoXmitIntMaskClear, 0xffffffff);
	reg_write(ohci, OHCI1394_IsoXmitIntEventClear, 0xffffffff);
	reg_write(ohci, OHCI1394_IsoRecvIntMaskClear, 0xffffffff);
	reg_write(ohci, OHCI1394_IsoRecvIntEventClear, 0xffffffff);
	set_phy_reg(ohci, 4, ~0xc0 & get_phy_reg(ohci, 4));
	reg_write(ohci, OHCI1394_LinkControlClear, 0xffffffff);
	ohci_devctl(ohci->host, RESET_BUS, LONG_RESET_NO_FORCE_ROOT);
	ohci_soft_reset(ohci);

	free_irq(dev->irq, ohci);
	err = pci_save_state(dev);
	if (err) {
		PRINT(KERN_ERR, "pci_save_state failed with %d", err);
		return err;
	}
	err = pci_set_power_state(dev, pci_choose_state(dev, state));
	if (err)
		DBGMSG("pci_set_power_state failed with %d", err);
	ohci1394_pmac_off(dev);

	return 0;
}

static int ohci1394_pci_resume(struct pci_dev *dev)
{
	int err;
	struct ti_ohci *ohci = pci_get_drvdata(dev);

	if (!ohci) {
		printk(KERN_ERR "%s: tried to resume nonexisting host\n",
		       OHCI1394_DRIVER_NAME);
		return -ENXIO;
	}
	DBGMSG("resume called");

	ohci1394_pmac_on(dev);
	pci_set_power_state(dev, PCI_D0);
	pci_restore_state(dev);
	err = pci_enable_device(dev);
	if (err) {
		PRINT(KERN_ERR, "pci_enable_device failed with %d", err);
		return err;
	}

	
	ohci_soft_reset(ohci);
	reg_write(ohci, OHCI1394_HCControlSet, OHCI1394_HCControl_LPS);
	reg_write(ohci, OHCI1394_IntEventClear, 0xffffffff);
	reg_write(ohci, OHCI1394_IntMaskClear, 0xffffffff);
	reg_read(ohci, OHCI1394_Version);
	msleep(50);

	err = request_irq(dev->irq, ohci_irq_handler, IRQF_SHARED,
			  OHCI1394_DRIVER_NAME, ohci);
	if (err) {
		PRINT_G(KERN_ERR, "Failed to allocate interrupt %d", dev->irq);
		return err;
	}

	ohci_initialize(ohci);

	hpsb_resume_host(ohci->host);
	return 0;
}
#endif 

static struct pci_device_id ohci1394_pci_tbl[] = {
	{
		.class = 	PCI_CLASS_SERIAL_FIREWIRE_OHCI,
		.class_mask = 	PCI_ANY_ID,
		.vendor =	PCI_ANY_ID,
		.device =	PCI_ANY_ID,
		.subvendor =	PCI_ANY_ID,
		.subdevice =	PCI_ANY_ID,
	},
	{ 0, },
};

MODULE_DEVICE_TABLE(pci, ohci1394_pci_tbl);

static struct pci_driver ohci1394_pci_driver = {
	.name =		OHCI1394_DRIVER_NAME,
	.id_table =	ohci1394_pci_tbl,
	.probe =	ohci1394_pci_probe,
	.remove =	ohci1394_pci_remove,
#ifdef CONFIG_PM
	.resume =	ohci1394_pci_resume,
	.suspend =	ohci1394_pci_suspend,
#endif
};






int ohci1394_stop_context(struct ti_ohci *ohci, int reg, char *msg)
{
	int i=0;

	
	reg_write(ohci, reg, 0x8000);

	
	while (reg_read(ohci, reg) & 0x400) {
		i++;
		if (i>5000) {
			PRINT(KERN_ERR,
			      "Runaway loop while stopping context: %s...", msg ? msg : "");
			return 1;
		}

		mb();
		udelay(10);
	}
	if (msg) PRINT(KERN_ERR, "%s: dma prg stopped", msg);
	return 0;
}

void ohci1394_init_iso_tasklet(struct ohci1394_iso_tasklet *tasklet, int type,
			       void (*func)(unsigned long), unsigned long data)
{
	tasklet_init(&tasklet->tasklet, func, data);
	tasklet->type = type;
	
	INIT_LIST_HEAD(&tasklet->link);
}

int ohci1394_register_iso_tasklet(struct ti_ohci *ohci,
				  struct ohci1394_iso_tasklet *tasklet)
{
	unsigned long flags, *usage;
	int n, i, r = -EBUSY;

	if (tasklet->type == OHCI_ISO_TRANSMIT) {
		n = ohci->nb_iso_xmit_ctx;
		usage = &ohci->it_ctx_usage;
	}
	else {
		n = ohci->nb_iso_rcv_ctx;
		usage = &ohci->ir_ctx_usage;

		
		if (tasklet->type == OHCI_ISO_MULTICHANNEL_RECEIVE) {
			if (test_and_set_bit(0, &ohci->ir_multichannel_used)) {
				return r;
			}
		}
	}

	spin_lock_irqsave(&ohci->iso_tasklet_list_lock, flags);

	for (i = 0; i < n; i++)
		if (!test_and_set_bit(i, usage)) {
			tasklet->context = i;
			list_add_tail(&tasklet->link, &ohci->iso_tasklet_list);
			r = 0;
			break;
		}

	spin_unlock_irqrestore(&ohci->iso_tasklet_list_lock, flags);

	return r;
}

void ohci1394_unregister_iso_tasklet(struct ti_ohci *ohci,
				     struct ohci1394_iso_tasklet *tasklet)
{
	unsigned long flags;

	tasklet_kill(&tasklet->tasklet);

	spin_lock_irqsave(&ohci->iso_tasklet_list_lock, flags);

	if (tasklet->type == OHCI_ISO_TRANSMIT)
		clear_bit(tasklet->context, &ohci->it_ctx_usage);
	else {
		clear_bit(tasklet->context, &ohci->ir_ctx_usage);

		if (tasklet->type == OHCI_ISO_MULTICHANNEL_RECEIVE) {
			clear_bit(0, &ohci->ir_multichannel_used);
		}
	}

	list_del(&tasklet->link);

	spin_unlock_irqrestore(&ohci->iso_tasklet_list_lock, flags);
}

EXPORT_SYMBOL(ohci1394_stop_context);
EXPORT_SYMBOL(ohci1394_init_iso_tasklet);
EXPORT_SYMBOL(ohci1394_register_iso_tasklet);
EXPORT_SYMBOL(ohci1394_unregister_iso_tasklet);



MODULE_AUTHOR("Sebastien Rougeaux <sebastien.rougeaux@anu.edu.au>");
MODULE_DESCRIPTION("Driver for PCI OHCI IEEE-1394 controllers");
MODULE_LICENSE("GPL");

static void __exit ohci1394_cleanup (void)
{
	pci_unregister_driver(&ohci1394_pci_driver);
}

static int __init ohci1394_init(void)
{
	return pci_register_driver(&ohci1394_pci_driver);
}

module_init(ohci1394_init);
module_exit(ohci1394_cleanup);
