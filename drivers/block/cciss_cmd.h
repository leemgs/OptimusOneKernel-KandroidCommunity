#ifndef CCISS_CMD_H
#define CCISS_CMD_H



#define CISS_VERSION "1.00"


#define SENSEINFOBYTES          32
#define MAXSGENTRIES            31
#define MAXREPLYQS              256


#define CMD_SUCCESS             0x0000
#define CMD_TARGET_STATUS       0x0001
#define CMD_DATA_UNDERRUN       0x0002
#define CMD_DATA_OVERRUN        0x0003
#define CMD_INVALID             0x0004
#define CMD_PROTOCOL_ERR        0x0005
#define CMD_HARDWARE_ERR        0x0006
#define CMD_CONNECTION_LOST     0x0007
#define CMD_ABORTED             0x0008
#define CMD_ABORT_FAILED        0x0009
#define CMD_UNSOLICITED_ABORT   0x000A
#define CMD_TIMEOUT             0x000B
#define CMD_UNABORTABLE		0x000C


#define POWER_OR_RESET			0x29
#define STATE_CHANGED			0x2a
#define UNIT_ATTENTION_CLEARED		0x2f
#define LUN_FAILED			0x3e
#define REPORT_LUNS_CHANGED		0x3f



	
#define POWER_ON_RESET			0x00
#define POWER_ON_REBOOT			0x01
#define SCSI_BUS_RESET			0x02
#define MSA_TARGET_RESET		0x03
#define CONTROLLER_FAILOVER		0x04
#define TRANSCEIVER_SE			0x05
#define TRANSCEIVER_LVD			0x06

	
#define RESERVATION_PREEMPTED		0x03
#define ASYM_ACCESS_CHANGED		0x06
#define LUN_CAPACITY_CHANGED		0x09


#define XFER_NONE               0x00
#define XFER_WRITE              0x01
#define XFER_READ               0x02
#define XFER_RSVD               0x03


#define ATTR_UNTAGGED           0x00
#define ATTR_SIMPLE             0x04
#define ATTR_HEADOFQUEUE        0x05
#define ATTR_ORDERED            0x06
#define ATTR_ACA                0x07


#define TYPE_CMD				0x00
#define TYPE_MSG				0x01


#define CFG_VENDORID            0x00
#define CFG_DEVICEID            0x02
#define CFG_I2OBAR              0x10
#define CFG_MEM1BAR             0x14


#define I2O_IBDB_SET            0x20
#define I2O_IBDB_CLEAR          0x70
#define I2O_INT_STATUS          0x30
#define I2O_INT_MASK            0x34
#define I2O_IBPOST_Q            0x40
#define I2O_OBPOST_Q            0x44
#define I2O_DMA1_CFG		0x214


#define CFGTBL_ChangeReq        0x00000001l
#define CFGTBL_AccCmds          0x00000001l

#define CFGTBL_Trans_Simple     0x00000002l

#define CFGTBL_BusType_Ultra2   0x00000001l
#define CFGTBL_BusType_Ultra3   0x00000002l
#define CFGTBL_BusType_Fibre1G  0x00000100l
#define CFGTBL_BusType_Fibre2G  0x00000200l
typedef struct _vals32
{
        __u32   lower;
        __u32   upper;
} vals32;

typedef union _u64bit
{
   vals32	val32;
   __u64	val;
} u64bit;


#define BYTE __u8
#define WORD __u16
#define HWORD __u16
#define DWORD __u32
#define QWORD vals32 




#define CISS_MAX_LUN	1024
#define CISS_MAX_PHYS_LUN	1024


#pragma pack(1)	

#define CISS_INQUIRY 0x12

typedef struct _InquiryData_struct
{
  BYTE data_byte[36];
} InquiryData_struct;

#define CISS_REPORT_LOG 0xc2    
#define CISS_REPORT_PHYS 0xc3   

typedef struct _ReportLUNdata_struct
{
  BYTE LUNListLength[4];
  DWORD reserved;
  BYTE LUN[CISS_MAX_LUN][8];
} ReportLunData_struct;

#define CCISS_READ_CAPACITY 0x25  
typedef struct _ReadCapdata_struct
{
  BYTE total_size[4];	
  BYTE block_size[4];	
} ReadCapdata_struct;

#define CCISS_READ_CAPACITY_16 0x9e 



#define CCISS_READ_CAPACITY_16_SERVICE_ACT 0x10

typedef struct _ReadCapdata_struct_16
{
	BYTE total_size[8];   
	BYTE block_size[4];   
	BYTE prot_en:1;       
	BYTE rto_en:1;        
	BYTE reserved:6;      
	BYTE reserved2[18];   
} ReadCapdata_struct_16;



#define CCISS_READ_10   0x28    
#define CCISS_WRITE_10  0x2a    
#define CCISS_READ_16   0x88    
#define CCISS_WRITE_16  0x8a    



#define CDB_LEN10	10
#define CDB_LEN16	16


#define BMIC_READ 0x26
#define BMIC_WRITE 0x27
#define BMIC_CACHE_FLUSH 0xc2
#define CCISS_CACHE_FLUSH 0x01	


typedef union _SCSI3Addr_struct {
   struct {
    BYTE Dev;
    BYTE Bus:6;
    BYTE Mode:2;        
  } PeripDev;
   struct {
    BYTE DevLSB;
    BYTE DevMSB:6;
    BYTE Mode:2;        
  } LogDev;
   struct {
    BYTE Dev:5;
    BYTE Bus:3;
    BYTE Targ:6;
    BYTE Mode:2;        
  } LogUnit;
} SCSI3Addr_struct;

typedef struct _PhysDevAddr_struct {
  DWORD             TargetId:24;
  DWORD             Bus:6;
  DWORD             Mode:2;
  SCSI3Addr_struct  Target[2]; 
} PhysDevAddr_struct;
  
typedef struct _LogDevAddr_struct {
  DWORD            VolId:30;
  DWORD            Mode:2;
  BYTE             reserved[4];
} LogDevAddr_struct;

typedef union _LUNAddr_struct {
  BYTE               LunAddrBytes[8];
  SCSI3Addr_struct   SCSI3Lun[4];
  PhysDevAddr_struct PhysDev;
  LogDevAddr_struct  LogDev;
} LUNAddr_struct;

#define CTLR_LUNID "\0\0\0\0\0\0\0\0"

typedef struct _CommandListHeader_struct {
  BYTE              ReplyQueue;
  BYTE              SGList;
  HWORD             SGTotal;
  QWORD             Tag;
  LUNAddr_struct    LUN;
} CommandListHeader_struct;
typedef struct _RequestBlock_struct {
  BYTE   CDBLen;
  struct {
    BYTE Type:3;
    BYTE Attribute:3;
    BYTE Direction:2;
  } Type;
  HWORD  Timeout;
  BYTE   CDB[16];
} RequestBlock_struct;
typedef struct _ErrDescriptor_struct {
  QWORD  Addr;
  DWORD  Len;
} ErrDescriptor_struct;
typedef struct _SGDescriptor_struct {
  QWORD  Addr;
  DWORD  Len;
  DWORD  Ext;
} SGDescriptor_struct;

typedef union _MoreErrInfo_struct{
  struct {
    BYTE  Reserved[3];
    BYTE  Type;
    DWORD ErrorInfo;
  }Common_Info;
  struct{
    BYTE  Reserved[2];
    BYTE  offense_size;
    BYTE  offense_num; 
    DWORD offense_value;
  }Invalid_Cmd;
}MoreErrInfo_struct;
typedef struct _ErrorInfo_struct {
  BYTE               ScsiStatus;
  BYTE               SenseLen;
  HWORD              CommandStatus;
  DWORD              ResidualCnt;
  MoreErrInfo_struct MoreErrInfo;
  BYTE               SenseInfo[SENSEINFOBYTES];
} ErrorInfo_struct;


#define CMD_RWREQ       0x00
#define CMD_IOCTL_PEND  0x01
#define CMD_SCSI	0x03
#define CMD_MSG_DONE	0x04
#define CMD_MSG_TIMEOUT 0x05
#define CMD_MSG_STALE	0xff


#define PADSIZE (sizeof(long) - 4)
typedef struct _CommandList_struct {
  CommandListHeader_struct Header;
  RequestBlock_struct      Request;
  ErrDescriptor_struct     ErrDesc;
  SGDescriptor_struct      SG[MAXSGENTRIES];
	 
  __u32			   busaddr; 
  ErrorInfo_struct * 	   err_info;  
  int			   ctlr;
  int			   cmd_type; 
  long			   cmdindex;
  struct hlist_node list;
  struct request *	   rq;
  struct completion *waiting;
  int	 retry_count;
  void * scsi_cmd;
  char   pad[PADSIZE];
} CommandList_struct;


typedef struct _HostWrite_struct {
  DWORD TransportRequest;
  DWORD Reserved;
  DWORD CoalIntDelay;
  DWORD CoalIntCount;
} HostWrite_struct;

typedef struct _CfgTable_struct {
  BYTE             Signature[4];
  DWORD            SpecValence;
  DWORD            TransportSupport;
  DWORD            TransportActive;
  HostWrite_struct HostWrite;
  DWORD            CmdsOutMax;
  DWORD            BusTypes;
  DWORD            Reserved; 
  BYTE             ServerName[16];
  DWORD            HeartBeat;
  DWORD            SCSI_Prefetch;
} CfgTable_struct;
#pragma pack()	 
#endif 
