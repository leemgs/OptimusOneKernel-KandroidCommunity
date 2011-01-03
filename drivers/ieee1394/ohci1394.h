

#ifndef _OHCI1394_H
#define _OHCI1394_H

#include "ieee1394_types.h"
#include <asm/io.h>

#define OHCI1394_DRIVER_NAME      "ohci1394"

#define OHCI1394_MAX_AT_REQ_RETRIES	0xf
#define OHCI1394_MAX_AT_RESP_RETRIES	0x2
#define OHCI1394_MAX_PHYS_RESP_RETRIES	0x8
#define OHCI1394_MAX_SELF_ID_ERRORS	16

#define AR_REQ_NUM_DESC		4		
#define AR_REQ_BUF_SIZE		PAGE_SIZE	
#define AR_REQ_SPLIT_BUF_SIZE	PAGE_SIZE	

#define AR_RESP_NUM_DESC	4		
#define AR_RESP_BUF_SIZE	PAGE_SIZE	
#define AR_RESP_SPLIT_BUF_SIZE	PAGE_SIZE	

#define IR_NUM_DESC		16		
#define IR_BUF_SIZE		PAGE_SIZE	
#define IR_SPLIT_BUF_SIZE	PAGE_SIZE	

#define IT_NUM_DESC		16	

#define AT_REQ_NUM_DESC		32	
#define AT_RESP_NUM_DESC	32	

#define OHCI_LOOP_COUNT		100	

#define OHCI_CONFIG_ROM_LEN	1024	

#define OHCI1394_SI_DMA_BUF_SIZE	8192 


#define OHCI1394_PCI_HCI_Control 0x40

struct dma_cmd {
        u32 control;
        u32 address;
        u32 branchAddress;
        u32 status;
};


struct at_dma_prg {
	struct dma_cmd begin;
	quadlet_t data[4];
	struct dma_cmd end;
	quadlet_t pad[4]; 
};


enum context_type { DMA_CTX_ASYNC_REQ, DMA_CTX_ASYNC_RESP, DMA_CTX_ISO };


struct dma_rcv_ctx {
	struct ti_ohci *ohci;
	enum context_type type;
	int ctx;
	unsigned int num_desc;

	unsigned int buf_size;
	unsigned int split_buf_size;

	
        struct dma_cmd **prg_cpu;
        dma_addr_t *prg_bus;
	struct pci_pool *prg_pool;

	
        quadlet_t **buf_cpu;
        dma_addr_t *buf_bus;

        unsigned int buf_ind;
        unsigned int buf_offset;
        quadlet_t *spb;
        spinlock_t lock;
        struct tasklet_struct task;
	int ctrlClear;
	int ctrlSet;
	int cmdPtr;
	int ctxtMatch;
};


struct dma_trm_ctx {
	struct ti_ohci *ohci;
	enum context_type type;
	int ctx;
	unsigned int num_desc;

	
        struct at_dma_prg **prg_cpu;
	dma_addr_t *prg_bus;
	struct pci_pool *prg_pool;

        unsigned int prg_ind;
        unsigned int sent_ind;
	int free_prgs;
        quadlet_t *branchAddrPtr;

	
	struct list_head fifo_list;

	
	struct list_head pending_list;

        spinlock_t lock;
        struct tasklet_struct task;
	int ctrlClear;
	int ctrlSet;
	int cmdPtr;
};

struct ohci1394_iso_tasklet {
	struct tasklet_struct tasklet;
	struct list_head link;
	int context;
	enum { OHCI_ISO_TRANSMIT, OHCI_ISO_RECEIVE,
	       OHCI_ISO_MULTICHANNEL_RECEIVE } type;
};

struct ti_ohci {
        struct pci_dev *dev;

	enum {
		OHCI_INIT_ALLOC_HOST,
		OHCI_INIT_HAVE_MEM_REGION,
		OHCI_INIT_HAVE_IOMAPPING,
		OHCI_INIT_HAVE_CONFIG_ROM_BUFFER,
		OHCI_INIT_HAVE_SELFID_BUFFER,
		OHCI_INIT_HAVE_TXRX_BUFFERS__MAYBE,
		OHCI_INIT_HAVE_IRQ,
		OHCI_INIT_DONE,
	} init_state;

        
        void __iomem *registers;

	
        quadlet_t *selfid_buf_cpu;
        dma_addr_t selfid_buf_bus;

	
        quadlet_t *csr_config_rom_cpu;
        dma_addr_t csr_config_rom_bus;
	int csr_config_rom_length;

	unsigned int max_packet_size;

        
	struct dma_rcv_ctx ar_resp_context;
	struct dma_rcv_ctx ar_req_context;

	
	struct dma_trm_ctx at_resp_context;
	struct dma_trm_ctx at_req_context;

        
	int nb_iso_rcv_ctx;
	unsigned long ir_ctx_usage; 
	unsigned long ir_multichannel_used; 
        spinlock_t IR_channel_lock;

        
	int nb_iso_xmit_ctx;
	unsigned long it_ctx_usage; 

        u64 ISO_channel_usage;

        
        struct hpsb_host *host;

        int phyid, isroot;

        spinlock_t phy_reg_lock;
	spinlock_t event_lock;

	int self_id_errors;

	
	struct list_head iso_tasklet_list;
	spinlock_t iso_tasklet_list_lock;

	
	unsigned int selfid_swap:1;
	
	unsigned int no_swap_incoming:1;

	
	unsigned int check_busreset:1;
};

static inline int cross_bound(unsigned long addr, unsigned int size)
{
	if (size == 0)
		return 0;

	if (size > PAGE_SIZE)
		return 1;

	if (addr >> PAGE_SHIFT != (addr + size - 1) >> PAGE_SHIFT)
		return 1;

	return 0;
}


static inline void reg_write(const struct ti_ohci *ohci, int offset, u32 data)
{
        writel(data, ohci->registers + offset);
}

static inline u32 reg_read(const struct ti_ohci *ohci, int offset)
{
        return readl(ohci->registers + offset);
}



#define OHCI1394_REGISTER_SIZE                0x800



#define OHCI1394_ContextControlSet            0x000
#define OHCI1394_ContextControlClear          0x004
#define OHCI1394_ContextCommandPtr            0x00C


#define OHCI1394_Version                      0x000
#define OHCI1394_GUID_ROM                     0x004
#define OHCI1394_ATRetries                    0x008
#define OHCI1394_CSRData                      0x00C
#define OHCI1394_CSRCompareData               0x010
#define OHCI1394_CSRControl                   0x014
#define OHCI1394_ConfigROMhdr                 0x018
#define OHCI1394_BusID                        0x01C
#define OHCI1394_BusOptions                   0x020
#define OHCI1394_GUIDHi                       0x024
#define OHCI1394_GUIDLo                       0x028
#define OHCI1394_ConfigROMmap                 0x034
#define OHCI1394_PostedWriteAddressLo         0x038
#define OHCI1394_PostedWriteAddressHi         0x03C
#define OHCI1394_VendorID                     0x040
#define OHCI1394_HCControlSet                 0x050
#define OHCI1394_HCControlClear               0x054
#define  OHCI1394_HCControl_noByteSwap		0x40000000
#define  OHCI1394_HCControl_programPhyEnable	0x00800000
#define  OHCI1394_HCControl_aPhyEnhanceEnable	0x00400000
#define  OHCI1394_HCControl_LPS			0x00080000
#define  OHCI1394_HCControl_postedWriteEnable	0x00040000
#define  OHCI1394_HCControl_linkEnable		0x00020000
#define  OHCI1394_HCControl_softReset		0x00010000
#define OHCI1394_SelfIDBuffer                 0x064
#define OHCI1394_SelfIDCount                  0x068
#define OHCI1394_IRMultiChanMaskHiSet         0x070
#define OHCI1394_IRMultiChanMaskHiClear       0x074
#define OHCI1394_IRMultiChanMaskLoSet         0x078
#define OHCI1394_IRMultiChanMaskLoClear       0x07C
#define OHCI1394_IntEventSet                  0x080
#define OHCI1394_IntEventClear                0x084
#define OHCI1394_IntMaskSet                   0x088
#define OHCI1394_IntMaskClear                 0x08C
#define OHCI1394_IsoXmitIntEventSet           0x090
#define OHCI1394_IsoXmitIntEventClear         0x094
#define OHCI1394_IsoXmitIntMaskSet            0x098
#define OHCI1394_IsoXmitIntMaskClear          0x09C
#define OHCI1394_IsoRecvIntEventSet           0x0A0
#define OHCI1394_IsoRecvIntEventClear         0x0A4
#define OHCI1394_IsoRecvIntMaskSet            0x0A8
#define OHCI1394_IsoRecvIntMaskClear          0x0AC
#define OHCI1394_InitialBandwidthAvailable    0x0B0
#define OHCI1394_InitialChannelsAvailableHi   0x0B4
#define OHCI1394_InitialChannelsAvailableLo   0x0B8
#define OHCI1394_FairnessControl              0x0DC
#define OHCI1394_LinkControlSet               0x0E0
#define OHCI1394_LinkControlClear             0x0E4
#define  OHCI1394_LinkControl_RcvSelfID		0x00000200
#define  OHCI1394_LinkControl_RcvPhyPkt		0x00000400
#define  OHCI1394_LinkControl_CycleTimerEnable	0x00100000
#define  OHCI1394_LinkControl_CycleMaster	0x00200000
#define  OHCI1394_LinkControl_CycleSource	0x00400000
#define OHCI1394_NodeID                       0x0E8
#define OHCI1394_PhyControl                   0x0EC
#define OHCI1394_IsochronousCycleTimer        0x0F0
#define OHCI1394_AsReqFilterHiSet             0x100
#define OHCI1394_AsReqFilterHiClear           0x104
#define OHCI1394_AsReqFilterLoSet             0x108
#define OHCI1394_AsReqFilterLoClear           0x10C
#define OHCI1394_PhyReqFilterHiSet            0x110
#define OHCI1394_PhyReqFilterHiClear          0x114
#define OHCI1394_PhyReqFilterLoSet            0x118
#define OHCI1394_PhyReqFilterLoClear          0x11C
#define OHCI1394_PhyUpperBound                0x120

#define OHCI1394_AsReqTrContextBase           0x180
#define OHCI1394_AsReqTrContextControlSet     0x180
#define OHCI1394_AsReqTrContextControlClear   0x184
#define OHCI1394_AsReqTrCommandPtr            0x18C

#define OHCI1394_AsRspTrContextBase           0x1A0
#define OHCI1394_AsRspTrContextControlSet     0x1A0
#define OHCI1394_AsRspTrContextControlClear   0x1A4
#define OHCI1394_AsRspTrCommandPtr            0x1AC

#define OHCI1394_AsReqRcvContextBase          0x1C0
#define OHCI1394_AsReqRcvContextControlSet    0x1C0
#define OHCI1394_AsReqRcvContextControlClear  0x1C4
#define OHCI1394_AsReqRcvCommandPtr           0x1CC

#define OHCI1394_AsRspRcvContextBase          0x1E0
#define OHCI1394_AsRspRcvContextControlSet    0x1E0
#define OHCI1394_AsRspRcvContextControlClear  0x1E4
#define OHCI1394_AsRspRcvCommandPtr           0x1EC



#define OHCI1394_IsoXmitContextBase           0x200
#define OHCI1394_IsoXmitContextControlSet     0x200
#define OHCI1394_IsoXmitContextControlClear   0x204
#define OHCI1394_IsoXmitCommandPtr            0x20C



#define OHCI1394_IsoRcvContextBase            0x400
#define OHCI1394_IsoRcvContextControlSet      0x400
#define OHCI1394_IsoRcvContextControlClear    0x404
#define OHCI1394_IsoRcvCommandPtr             0x40C
#define OHCI1394_IsoRcvContextMatch           0x410



#define OHCI1394_reqTxComplete           0x00000001
#define OHCI1394_respTxComplete          0x00000002
#define OHCI1394_ARRQ                    0x00000004
#define OHCI1394_ARRS                    0x00000008
#define OHCI1394_RQPkt                   0x00000010
#define OHCI1394_RSPkt                   0x00000020
#define OHCI1394_isochTx                 0x00000040
#define OHCI1394_isochRx                 0x00000080
#define OHCI1394_postedWriteErr          0x00000100
#define OHCI1394_lockRespErr             0x00000200
#define OHCI1394_selfIDComplete          0x00010000
#define OHCI1394_busReset                0x00020000
#define OHCI1394_phy                     0x00080000
#define OHCI1394_cycleSynch              0x00100000
#define OHCI1394_cycle64Seconds          0x00200000
#define OHCI1394_cycleLost               0x00400000
#define OHCI1394_cycleInconsistent       0x00800000
#define OHCI1394_unrecoverableError      0x01000000
#define OHCI1394_cycleTooLong            0x02000000
#define OHCI1394_phyRegRcvd              0x04000000
#define OHCI1394_masterIntEnable         0x80000000


#define DMA_CTL_OUTPUT_MORE              0x00000000
#define DMA_CTL_OUTPUT_LAST              0x10000000
#define DMA_CTL_INPUT_MORE               0x20000000
#define DMA_CTL_INPUT_LAST               0x30000000
#define DMA_CTL_UPDATE                   0x08000000
#define DMA_CTL_IMMEDIATE                0x02000000
#define DMA_CTL_IRQ                      0x00300000
#define DMA_CTL_BRANCH                   0x000c0000
#define DMA_CTL_WAIT                     0x00030000


#define EVT_NO_STATUS		0x0	
#define EVT_RESERVED_A		0x1	
#define EVT_LONG_PACKET		0x2	
#define EVT_MISSING_ACK		0x3	
#define EVT_UNDERRUN		0x4	
#define EVT_OVERRUN		0x5	
#define EVT_DESCRIPTOR_READ	0x6	
#define EVT_DATA_READ		0x7	
#define EVT_DATA_WRITE		0x8	
#define EVT_BUS_RESET		0x9	
#define EVT_TIMEOUT		0xa	
#define EVT_TCODE_ERR		0xb	
#define EVT_RESERVED_B		0xc	
#define EVT_RESERVED_C		0xd	
#define EVT_UNKNOWN		0xe	
#define EVT_FLUSHED		0xf	

#define OHCI1394_TCODE_PHY               0xE


#define OHCI1394_PHYS_UPPER_BOUND_FIXED		0x000100000000ULL 
#define OHCI1394_PHYS_UPPER_BOUND_PROGRAMMED	0x010000000000ULL 
#define OHCI1394_MIDDLE_ADDRESS_SPACE		0xffff00000000ULL

void ohci1394_init_iso_tasklet(struct ohci1394_iso_tasklet *tasklet,
			       int type,
			       void (*func)(unsigned long),
			       unsigned long data);
int ohci1394_register_iso_tasklet(struct ti_ohci *ohci,
				  struct ohci1394_iso_tasklet *tasklet);
void ohci1394_unregister_iso_tasklet(struct ti_ohci *ohci,
				     struct ohci1394_iso_tasklet *tasklet);
int ohci1394_stop_context(struct ti_ohci *ohci, int reg, char *msg);
struct ti_ohci *ohci1394_get_struct(int card_num);

#endif
