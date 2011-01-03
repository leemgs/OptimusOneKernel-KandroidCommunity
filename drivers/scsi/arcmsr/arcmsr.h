
#include <linux/interrupt.h>

struct device_attribute;

#define ARCMSR_MAX_OUTSTANDING_CMD						256
#define ARCMSR_MAX_FREECCB_NUM							320
#define ARCMSR_DRIVER_VERSION		     "Driver Version 1.20.00.15 2008/02/27"
#define ARCMSR_SCSI_INITIATOR_ID						255
#define ARCMSR_MAX_XFER_SECTORS							512
#define ARCMSR_MAX_XFER_SECTORS_B						4096
#define ARCMSR_MAX_TARGETID							17
#define ARCMSR_MAX_TARGETLUN							8
#define ARCMSR_MAX_CMD_PERLUN		                 ARCMSR_MAX_OUTSTANDING_CMD
#define ARCMSR_MAX_QBUFFER							4096
#define ARCMSR_MAX_SG_ENTRIES							38
#define ARCMSR_MAX_HBB_POSTQUEUE						264

#define ARC_SUCCESS                                                       0
#define ARC_FAILURE                                                       1

#define dma_addr_hi32(addr)               (uint32_t) ((addr>>16)>>16)
#define dma_addr_lo32(addr)               (uint32_t) (addr & 0xffffffff)

struct CMD_MESSAGE
{
      uint32_t HeaderLength;
      uint8_t  Signature[8];
      uint32_t Timeout;
      uint32_t ControlCode;
      uint32_t ReturnCode;
      uint32_t Length;
};

struct CMD_MESSAGE_FIELD
{
    struct CMD_MESSAGE			cmdmessage;
    uint8_t				messagedatabuffer[1032];
};

#define ARCMSR_MESSAGE_FAIL			0x0001

#define ARECA_SATA_RAID				0x90000000

#define FUNCTION_READ_RQBUFFER			0x0801
#define FUNCTION_WRITE_WQBUFFER			0x0802
#define FUNCTION_CLEAR_RQBUFFER			0x0803
#define FUNCTION_CLEAR_WQBUFFER			0x0804
#define FUNCTION_CLEAR_ALLQBUFFER		0x0805
#define FUNCTION_RETURN_CODE_3F			0x0806
#define FUNCTION_SAY_HELLO			0x0807
#define FUNCTION_SAY_GOODBYE			0x0808
#define FUNCTION_FLUSH_ADAPTER_CACHE		0x0809

#define ARCMSR_MESSAGE_READ_RQBUFFER       \
	ARECA_SATA_RAID | FUNCTION_READ_RQBUFFER
#define ARCMSR_MESSAGE_WRITE_WQBUFFER      \
	ARECA_SATA_RAID | FUNCTION_WRITE_WQBUFFER
#define ARCMSR_MESSAGE_CLEAR_RQBUFFER      \
	ARECA_SATA_RAID | FUNCTION_CLEAR_RQBUFFER
#define ARCMSR_MESSAGE_CLEAR_WQBUFFER      \
	ARECA_SATA_RAID | FUNCTION_CLEAR_WQBUFFER
#define ARCMSR_MESSAGE_CLEAR_ALLQBUFFER    \
	ARECA_SATA_RAID | FUNCTION_CLEAR_ALLQBUFFER
#define ARCMSR_MESSAGE_RETURN_CODE_3F      \
	ARECA_SATA_RAID | FUNCTION_RETURN_CODE_3F
#define ARCMSR_MESSAGE_SAY_HELLO           \
	ARECA_SATA_RAID | FUNCTION_SAY_HELLO
#define ARCMSR_MESSAGE_SAY_GOODBYE         \
	ARECA_SATA_RAID | FUNCTION_SAY_GOODBYE
#define ARCMSR_MESSAGE_FLUSH_ADAPTER_CACHE \
	ARECA_SATA_RAID | FUNCTION_FLUSH_ADAPTER_CACHE

#define ARCMSR_MESSAGE_RETURNCODE_OK              0x00000001
#define ARCMSR_MESSAGE_RETURNCODE_ERROR           0x00000006
#define ARCMSR_MESSAGE_RETURNCODE_3F              0x0000003F

#define IS_SG64_ADDR                0x01000000 
struct  SG32ENTRY
{
	__le32					length;
	__le32					address;
};
struct  SG64ENTRY
{
	__le32					length;
	__le32					address;
	__le32					addresshigh;
};
struct SGENTRY_UNION
{
	union
	{
		struct SG32ENTRY            sg32entry;
		struct SG64ENTRY            sg64entry;
	}u;
};

struct QBUFFER
{
	uint32_t      data_len;
	uint8_t       data[124];
};

struct FIRMWARE_INFO
{
	uint32_t      signature;		
	uint32_t      request_len;		
	uint32_t      numbers_queue;		
	uint32_t      sdram_size;               
	uint32_t      ide_channels;		
	char          vendor[40];		
	char          model[8];			
	char          firmware_ver[16];     	
	char          device_map[16];		
};

#define ARCMSR_SIGNATURE_GET_CONFIG		      0x87974060
#define ARCMSR_SIGNATURE_SET_CONFIG		      0x87974063

#define ARCMSR_INBOUND_MESG0_NOP		      0x00000000
#define ARCMSR_INBOUND_MESG0_GET_CONFIG		      0x00000001
#define ARCMSR_INBOUND_MESG0_SET_CONFIG               0x00000002
#define ARCMSR_INBOUND_MESG0_ABORT_CMD                0x00000003
#define ARCMSR_INBOUND_MESG0_STOP_BGRB                0x00000004
#define ARCMSR_INBOUND_MESG0_FLUSH_CACHE              0x00000005
#define ARCMSR_INBOUND_MESG0_START_BGRB               0x00000006
#define ARCMSR_INBOUND_MESG0_CHK331PENDING            0x00000007
#define ARCMSR_INBOUND_MESG0_SYNC_TIMER               0x00000008

#define ARCMSR_INBOUND_DRIVER_DATA_WRITE_OK           0x00000001
#define ARCMSR_INBOUND_DRIVER_DATA_READ_OK            0x00000002
#define ARCMSR_OUTBOUND_IOP331_DATA_WRITE_OK          0x00000001
#define ARCMSR_OUTBOUND_IOP331_DATA_READ_OK           0x00000002

#define ARCMSR_CCBPOST_FLAG_SGL_BSIZE                 0x80000000
#define ARCMSR_CCBPOST_FLAG_IAM_BIOS                  0x40000000
#define ARCMSR_CCBREPLY_FLAG_IAM_BIOS                 0x40000000
#define ARCMSR_CCBREPLY_FLAG_ERROR                    0x10000000

#define ARCMSR_OUTBOUND_MESG1_FIRMWARE_OK             0x80000000




#define ARCMSR_DRV2IOP_DOORBELL                       0x00020400
#define ARCMSR_DRV2IOP_DOORBELL_MASK                  0x00020404

#define ARCMSR_IOP2DRV_DOORBELL                       0x00020408
#define ARCMSR_IOP2DRV_DOORBELL_MASK                  0x0002040C


#define ARCMSR_IOP2DRV_DATA_WRITE_OK                  0x00000001

#define ARCMSR_IOP2DRV_DATA_READ_OK                   0x00000002
#define ARCMSR_IOP2DRV_CDB_DONE                       0x00000004
#define ARCMSR_IOP2DRV_MESSAGE_CMD_DONE               0x00000008

#define ARCMSR_DOORBELL_HANDLE_INT		      0x0000000F
#define ARCMSR_DOORBELL_INT_CLEAR_PATTERN   	      0xFF00FFF0
#define ARCMSR_MESSAGE_INT_CLEAR_PATTERN	      0xFF00FFF7

#define ARCMSR_MESSAGE_GET_CONFIG		      0x00010008

#define ARCMSR_MESSAGE_SET_CONFIG		      0x00020008

#define ARCMSR_MESSAGE_ABORT_CMD		      0x00030008

#define ARCMSR_MESSAGE_STOP_BGRB		      0x00040008

#define ARCMSR_MESSAGE_FLUSH_CACHE                    0x00050008

#define ARCMSR_MESSAGE_START_BGRB		      0x00060008
#define ARCMSR_MESSAGE_START_DRIVER_MODE	      0x000E0008
#define ARCMSR_MESSAGE_SET_POST_WINDOW		      0x000F0008
#define ARCMSR_MESSAGE_ACTIVE_EOI_MODE		    0x00100008

#define ARCMSR_MESSAGE_FIRMWARE_OK		      0x80000000

#define ARCMSR_DRV2IOP_DATA_WRITE_OK                  0x00000001

#define ARCMSR_DRV2IOP_DATA_READ_OK                   0x00000002
#define ARCMSR_DRV2IOP_CDB_POSTED                     0x00000004
#define ARCMSR_DRV2IOP_MESSAGE_CMD_POSTED             0x00000008
#define ARCMSR_DRV2IOP_END_OF_INTERRUPT		0x00000010



#define ARCMSR_IOCTL_WBUFFER			      0x0000fe00

#define ARCMSR_IOCTL_RBUFFER			      0x0000ff00

#define ARCMSR_MSGCODE_RWBUFFER			      0x0000fa00

struct ARCMSR_CDB
{
	uint8_t							Bus;
	uint8_t							TargetID;
	uint8_t							LUN;
	uint8_t							Function;
	uint8_t							CdbLength;
	uint8_t							sgcount;
	uint8_t							Flags;
#define ARCMSR_CDB_FLAG_SGL_BSIZE          0x01
#define ARCMSR_CDB_FLAG_BIOS               0x02
#define ARCMSR_CDB_FLAG_WRITE              0x04
#define ARCMSR_CDB_FLAG_SIMPLEQ            0x00
#define ARCMSR_CDB_FLAG_HEADQ              0x08
#define ARCMSR_CDB_FLAG_ORDEREDQ           0x10

	uint8_t							Reserved1;
	uint32_t						Context;
	uint32_t						DataLength;
	uint8_t							Cdb[16];
	uint8_t							DeviceStatus;
#define ARCMSR_DEV_CHECK_CONDITION	    0x02
#define ARCMSR_DEV_SELECT_TIMEOUT	    0xF0
#define ARCMSR_DEV_ABORTED		    0xF1
#define ARCMSR_DEV_INIT_FAIL		    0xF2

	uint8_t							SenseData[15];
	union
	{
		struct SG32ENTRY                sg32entry[ARCMSR_MAX_SG_ENTRIES];
		struct SG64ENTRY                sg64entry[ARCMSR_MAX_SG_ENTRIES];
	} u;
};

struct MessageUnit_A
{
	uint32_t	resrved0[4];			
	uint32_t	inbound_msgaddr0;		
	uint32_t	inbound_msgaddr1;		
	uint32_t	outbound_msgaddr0;		
	uint32_t	outbound_msgaddr1;		
	uint32_t	inbound_doorbell;		
	uint32_t	inbound_intstatus;		
	uint32_t	inbound_intmask;		
	uint32_t	outbound_doorbell;		
	uint32_t	outbound_intstatus;		
	uint32_t	outbound_intmask;		
	uint32_t	reserved1[2];			
	uint32_t	inbound_queueport;		
	uint32_t	outbound_queueport;     	
	uint32_t	reserved2[2];			
	uint32_t	reserved3[492];			
	uint32_t	reserved4[128];			
	uint32_t	message_rwbuffer[256];		
	uint32_t	message_wbuffer[32];		
	uint32_t	reserved5[32];			
	uint32_t	message_rbuffer[32];		
	uint32_t	reserved6[32];			
};

struct MessageUnit_B
{
	uint32_t	post_qbuffer[ARCMSR_MAX_HBB_POSTQUEUE];
	uint32_t	done_qbuffer[ARCMSR_MAX_HBB_POSTQUEUE];
	uint32_t	postq_index;
	uint32_t	doneq_index;
	void		__iomem *drv2iop_doorbell_reg;
	void		__iomem *drv2iop_doorbell_mask_reg;
	void		__iomem *iop2drv_doorbell_reg;
	void		__iomem *iop2drv_doorbell_mask_reg;
	void		__iomem *msgcode_rwbuffer_reg;
	void		__iomem *ioctl_wbuffer_reg;
	void		__iomem *ioctl_rbuffer_reg;
};


struct AdapterControlBlock
{
	uint32_t  adapter_type;                
	#define ACB_ADAPTER_TYPE_A            0x00000001	
	#define ACB_ADAPTER_TYPE_B            0x00000002	
	#define ACB_ADAPTER_TYPE_C            0x00000004	
	#define ACB_ADAPTER_TYPE_D            0x00000008	
	struct pci_dev *		pdev;
	struct Scsi_Host *		host;
	unsigned long			vir2phy_offset;
	
	uint32_t			outbound_int_enable;

	union {
		struct MessageUnit_A __iomem *	pmuA;
		struct MessageUnit_B *		pmuB;
	};
	

	uint32_t			acb_flags;
	#define ACB_F_SCSISTOPADAPTER         	0x0001
	#define ACB_F_MSG_STOP_BGRB     	0x0002
	
	#define ACB_F_MSG_START_BGRB          	0x0004
	
	#define ACB_F_IOPDATA_OVERFLOW        	0x0008
	
	#define ACB_F_MESSAGE_WQBUFFER_CLEARED	0x0010
	
	#define ACB_F_MESSAGE_RQBUFFER_CLEARED  0x0020
	
	#define ACB_F_MESSAGE_WQBUFFER_READED   0x0040
	#define ACB_F_BUS_RESET               	0x0080
	#define ACB_F_IOP_INITED              	0x0100
	

	struct CommandControlBlock *			pccb_pool[ARCMSR_MAX_FREECCB_NUM];
	
	struct list_head		ccb_free_list;
	

	atomic_t			ccboutstandingcount;
	

	void *				dma_coherent;
	
	dma_addr_t			dma_coherent_handle;
	

	uint8_t				rqbuffer[ARCMSR_MAX_QBUFFER];
	
	int32_t				rqbuf_firstindex;
	
	int32_t				rqbuf_lastindex;
	
	uint8_t				wqbuffer[ARCMSR_MAX_QBUFFER];
	
	int32_t				wqbuf_firstindex;
	
	int32_t				wqbuf_lastindex;
	
	uint8_t				devstate[ARCMSR_MAX_TARGETID][ARCMSR_MAX_TARGETLUN];
	
#define ARECA_RAID_GONE               0x55
#define ARECA_RAID_GOOD               0xaa
	uint32_t			num_resets;
	uint32_t			num_aborts;
	uint32_t			firm_request_len;
	uint32_t			firm_numbers_queue;
	uint32_t			firm_sdram_size;
	uint32_t			firm_hd_channels;
	char				firm_model[12];
	char				firm_version[20];
};

struct CommandControlBlock
{
	struct ARCMSR_CDB		arcmsr_cdb;
	
	uint32_t			cdb_shifted_phyaddr;
	
	uint32_t			reserved1;
	
#if BITS_PER_LONG == 64
	
	struct list_head		list;
	
	struct scsi_cmnd *		pcmd;
	
	struct AdapterControlBlock *	acb;
	

	uint16_t			ccb_flags;
	
	#define		CCB_FLAG_READ			0x0000
	#define		CCB_FLAG_WRITE			0x0001
	#define		CCB_FLAG_ERROR			0x0002
	#define		CCB_FLAG_FLUSHCACHE		0x0004
	#define		CCB_FLAG_MASTER_ABORTED		0x0008
	uint16_t			startdone;
	
	#define		ARCMSR_CCB_DONE			0x0000
	#define		ARCMSR_CCB_START		0x55AA
	#define		ARCMSR_CCB_ABORTED		0xAA55
	#define		ARCMSR_CCB_ILLEGAL		0xFFFF
	uint32_t			reserved2[7];
	
#else
	
	struct list_head		list;
	
	struct scsi_cmnd *		pcmd;
	
	struct AdapterControlBlock *	acb;
	

	uint16_t			ccb_flags;
	
	#define		CCB_FLAG_READ			0x0000
	#define		CCB_FLAG_WRITE			0x0001
	#define		CCB_FLAG_ERROR			0x0002
	#define		CCB_FLAG_FLUSHCACHE		0x0004
	#define		CCB_FLAG_MASTER_ABORTED		0x0008
	uint16_t			startdone;
	
	#define		ARCMSR_CCB_DONE			0x0000
	#define		ARCMSR_CCB_START		0x55AA
	#define		ARCMSR_CCB_ABORTED		0xAA55
	#define		ARCMSR_CCB_ILLEGAL		0xFFFF
	uint32_t			reserved2[3];
	
#endif
	
};

struct SENSE_DATA
{
	uint8_t				ErrorCode:7;
#define SCSI_SENSE_CURRENT_ERRORS	0x70
#define SCSI_SENSE_DEFERRED_ERRORS	0x71
	uint8_t				Valid:1;
	uint8_t				SegmentNumber;
	uint8_t				SenseKey:4;
	uint8_t				Reserved:1;
	uint8_t				IncorrectLength:1;
	uint8_t				EndOfMedia:1;
	uint8_t				FileMark:1;
	uint8_t				Information[4];
	uint8_t				AdditionalSenseLength;
	uint8_t				CommandSpecificInformation[4];
	uint8_t				AdditionalSenseCode;
	uint8_t				AdditionalSenseCodeQualifier;
	uint8_t				FieldReplaceableUnitCode;
	uint8_t				SenseKeySpecific[3];
};

#define     ARCMSR_MU_OUTBOUND_INTERRUPT_STATUS_REG                 0x30
#define     ARCMSR_MU_OUTBOUND_PCI_INT                              0x10
#define     ARCMSR_MU_OUTBOUND_POSTQUEUE_INT                        0x08
#define     ARCMSR_MU_OUTBOUND_DOORBELL_INT                         0x04
#define     ARCMSR_MU_OUTBOUND_MESSAGE1_INT                         0x02
#define     ARCMSR_MU_OUTBOUND_MESSAGE0_INT                         0x01
#define     ARCMSR_MU_OUTBOUND_HANDLE_INT                 \
                    (ARCMSR_MU_OUTBOUND_MESSAGE0_INT      \
                     |ARCMSR_MU_OUTBOUND_MESSAGE1_INT     \
                     |ARCMSR_MU_OUTBOUND_DOORBELL_INT     \
                     |ARCMSR_MU_OUTBOUND_POSTQUEUE_INT    \
                     |ARCMSR_MU_OUTBOUND_PCI_INT)

#define     ARCMSR_MU_OUTBOUND_INTERRUPT_MASK_REG                   0x34
#define     ARCMSR_MU_OUTBOUND_PCI_INTMASKENABLE                    0x10
#define     ARCMSR_MU_OUTBOUND_POSTQUEUE_INTMASKENABLE              0x08
#define     ARCMSR_MU_OUTBOUND_DOORBELL_INTMASKENABLE               0x04
#define     ARCMSR_MU_OUTBOUND_MESSAGE1_INTMASKENABLE               0x02
#define     ARCMSR_MU_OUTBOUND_MESSAGE0_INTMASKENABLE               0x01
#define     ARCMSR_MU_OUTBOUND_ALL_INTMASKENABLE                    0x1F

extern void arcmsr_post_ioctldata2iop(struct AdapterControlBlock *);
extern void arcmsr_iop_message_read(struct AdapterControlBlock *);
extern struct QBUFFER __iomem *arcmsr_get_iop_rqbuffer(struct AdapterControlBlock *);
extern struct device_attribute *arcmsr_host_attrs[];
extern int arcmsr_alloc_sysfs_attr(struct AdapterControlBlock *);
void arcmsr_free_sysfs_attr(struct AdapterControlBlock *acb);
