

#ifndef MPI2_IOC_H
#define MPI2_IOC_H






typedef struct _MPI2_IOC_INIT_REQUEST
{
    U8                      WhoInit;                        
    U8                      Reserved1;                      
    U8                      ChainOffset;                    
    U8                      Function;                       
    U16                     Reserved2;                      
    U8                      Reserved3;                      
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved4;                      
    U16                     MsgVersion;                     
    U16                     HeaderVersion;                  
    U32                     Reserved5;                      
    U32                     Reserved6;                      
    U16                     Reserved7;                      
    U16                     SystemRequestFrameSize;         
    U16                     ReplyDescriptorPostQueueDepth;  
    U16                     ReplyFreeQueueDepth;            
    U32                     SenseBufferAddressHigh;         
    U32                     SystemReplyAddressHigh;         
    U64                     SystemRequestFrameBaseAddress;  
    U64                     ReplyDescriptorPostQueueAddress;
    U64                     ReplyFreeQueueAddress;          
    U64                     TimeStamp;                      
} MPI2_IOC_INIT_REQUEST, MPI2_POINTER PTR_MPI2_IOC_INIT_REQUEST,
  Mpi2IOCInitRequest_t, MPI2_POINTER pMpi2IOCInitRequest_t;


#define MPI2_WHOINIT_NOT_INITIALIZED            (0x00)
#define MPI2_WHOINIT_SYSTEM_BIOS                (0x01)
#define MPI2_WHOINIT_ROM_BIOS                   (0x02)
#define MPI2_WHOINIT_PCI_PEER                   (0x03)
#define MPI2_WHOINIT_HOST_DRIVER                (0x04)
#define MPI2_WHOINIT_MANUFACTURER               (0x05)


#define MPI2_IOCINIT_MSGVERSION_MAJOR_MASK      (0xFF00)
#define MPI2_IOCINIT_MSGVERSION_MAJOR_SHIFT     (8)
#define MPI2_IOCINIT_MSGVERSION_MINOR_MASK      (0x00FF)
#define MPI2_IOCINIT_MSGVERSION_MINOR_SHIFT     (0)


#define MPI2_IOCINIT_HDRVERSION_UNIT_MASK       (0xFF00)
#define MPI2_IOCINIT_HDRVERSION_UNIT_SHIFT      (8)
#define MPI2_IOCINIT_HDRVERSION_DEV_MASK        (0x00FF)
#define MPI2_IOCINIT_HDRVERSION_DEV_SHIFT       (0)


#define MPI2_RDPQ_DEPTH_MIN                     (16)



typedef struct _MPI2_IOC_INIT_REPLY
{
    U8                      WhoInit;                        
    U8                      Reserved1;                      
    U8                      MsgLength;                      
    U8                      Function;                       
    U16                     Reserved2;                      
    U8                      Reserved3;                      
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved4;                      
    U16                     Reserved5;                      
    U16                     IOCStatus;                      
    U32                     IOCLogInfo;                     
} MPI2_IOC_INIT_REPLY, MPI2_POINTER PTR_MPI2_IOC_INIT_REPLY,
  Mpi2IOCInitReply_t, MPI2_POINTER pMpi2IOCInitReply_t;





typedef struct _MPI2_IOC_FACTS_REQUEST
{
    U16                     Reserved1;                      
    U8                      ChainOffset;                    
    U8                      Function;                       
    U16                     Reserved2;                      
    U8                      Reserved3;                      
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved4;                      
} MPI2_IOC_FACTS_REQUEST, MPI2_POINTER PTR_MPI2_IOC_FACTS_REQUEST,
  Mpi2IOCFactsRequest_t, MPI2_POINTER pMpi2IOCFactsRequest_t;



typedef struct _MPI2_IOC_FACTS_REPLY
{
    U16                     MsgVersion;                     
    U8                      MsgLength;                      
    U8                      Function;                       
    U16                     HeaderVersion;                  
    U8                      IOCNumber;                      
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved1;                      
    U16                     IOCExceptions;                  
    U16                     IOCStatus;                      
    U32                     IOCLogInfo;                     
    U8                      MaxChainDepth;                  
    U8                      WhoInit;                        
    U8                      NumberOfPorts;                  
    U8                      Reserved2;                      
    U16                     RequestCredit;                  
    U16                     ProductID;                      
    U32                     IOCCapabilities;                
    MPI2_VERSION_UNION      FWVersion;                      
    U16                     IOCRequestFrameSize;            
    U16                     Reserved3;                      
    U16                     MaxInitiators;                  
    U16                     MaxTargets;                     
    U16                     MaxSasExpanders;                
    U16                     MaxEnclosures;                  
    U16                     ProtocolFlags;                  
    U16                     HighPriorityCredit;             
    U16                     MaxReplyDescriptorPostQueueDepth; 
    U8                      ReplyFrameSize;                 
    U8                      MaxVolumes;                     
    U16                     MaxDevHandle;                   
    U16                     MaxPersistentEntries;           
    U32                     Reserved4;                      
} MPI2_IOC_FACTS_REPLY, MPI2_POINTER PTR_MPI2_IOC_FACTS_REPLY,
  Mpi2IOCFactsReply_t, MPI2_POINTER pMpi2IOCFactsReply_t;


#define MPI2_IOCFACTS_MSGVERSION_MAJOR_MASK             (0xFF00)
#define MPI2_IOCFACTS_MSGVERSION_MAJOR_SHIFT            (8)
#define MPI2_IOCFACTS_MSGVERSION_MINOR_MASK             (0x00FF)
#define MPI2_IOCFACTS_MSGVERSION_MINOR_SHIFT            (0)


#define MPI2_IOCFACTS_HDRVERSION_UNIT_MASK              (0xFF00)
#define MPI2_IOCFACTS_HDRVERSION_UNIT_SHIFT             (8)
#define MPI2_IOCFACTS_HDRVERSION_DEV_MASK               (0x00FF)
#define MPI2_IOCFACTS_HDRVERSION_DEV_SHIFT              (0)


#define MPI2_IOCFACTS_EXCEPT_IR_FOREIGN_CONFIG_MAX      (0x0100)

#define MPI2_IOCFACTS_EXCEPT_BOOTSTAT_MASK              (0x00E0)
#define MPI2_IOCFACTS_EXCEPT_BOOTSTAT_GOOD              (0x0000)
#define MPI2_IOCFACTS_EXCEPT_BOOTSTAT_BACKUP            (0x0020)
#define MPI2_IOCFACTS_EXCEPT_BOOTSTAT_RESTORED          (0x0040)
#define MPI2_IOCFACTS_EXCEPT_BOOTSTAT_CORRUPT_BACKUP    (0x0060)

#define MPI2_IOCFACTS_EXCEPT_METADATA_UNSUPPORTED       (0x0010)
#define MPI2_IOCFACTS_EXCEPT_MANUFACT_CHECKSUM_FAIL     (0x0008)
#define MPI2_IOCFACTS_EXCEPT_FW_CHECKSUM_FAIL           (0x0004)
#define MPI2_IOCFACTS_EXCEPT_RAID_CONFIG_INVALID        (0x0002)
#define MPI2_IOCFACTS_EXCEPT_CONFIG_CHECKSUM_FAIL       (0x0001)






#define MPI2_IOCFACTS_CAPABILITY_MSI_X_INDEX            (0x00008000)
#define MPI2_IOCFACTS_CAPABILITY_RAID_ACCELERATOR       (0x00004000)
#define MPI2_IOCFACTS_CAPABILITY_EVENT_REPLAY           (0x00002000)
#define MPI2_IOCFACTS_CAPABILITY_INTEGRATED_RAID        (0x00001000)
#define MPI2_IOCFACTS_CAPABILITY_TLR                    (0x00000800)
#define MPI2_IOCFACTS_CAPABILITY_MULTICAST              (0x00000100)
#define MPI2_IOCFACTS_CAPABILITY_BIDIRECTIONAL_TARGET   (0x00000080)
#define MPI2_IOCFACTS_CAPABILITY_EEDP                   (0x00000040)
#define MPI2_IOCFACTS_CAPABILITY_SNAPSHOT_BUFFER        (0x00000010)
#define MPI2_IOCFACTS_CAPABILITY_DIAG_TRACE_BUFFER      (0x00000008)
#define MPI2_IOCFACTS_CAPABILITY_TASK_SET_FULL_HANDLING (0x00000004)


#define MPI2_IOCFACTS_PROTOCOL_SCSI_TARGET              (0x0001)
#define MPI2_IOCFACTS_PROTOCOL_SCSI_INITIATOR           (0x0002)





typedef struct _MPI2_PORT_FACTS_REQUEST
{
    U16                     Reserved1;                      
    U8                      ChainOffset;                    
    U8                      Function;                       
    U16                     Reserved2;                      
    U8                      PortNumber;                     
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved3;                      
} MPI2_PORT_FACTS_REQUEST, MPI2_POINTER PTR_MPI2_PORT_FACTS_REQUEST,
  Mpi2PortFactsRequest_t, MPI2_POINTER pMpi2PortFactsRequest_t;


typedef struct _MPI2_PORT_FACTS_REPLY
{
    U16                     Reserved1;                      
    U8                      MsgLength;                      
    U8                      Function;                       
    U16                     Reserved2;                      
    U8                      PortNumber;                     
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved3;                      
    U16                     Reserved4;                      
    U16                     IOCStatus;                      
    U32                     IOCLogInfo;                     
    U8                      Reserved5;                      
    U8                      PortType;                       
    U16                     Reserved6;                      
    U16                     MaxPostedCmdBuffers;            
    U16                     Reserved7;                      
} MPI2_PORT_FACTS_REPLY, MPI2_POINTER PTR_MPI2_PORT_FACTS_REPLY,
  Mpi2PortFactsReply_t, MPI2_POINTER pMpi2PortFactsReply_t;


#define MPI2_PORTFACTS_PORTTYPE_INACTIVE            (0x00)
#define MPI2_PORTFACTS_PORTTYPE_FC                  (0x10)
#define MPI2_PORTFACTS_PORTTYPE_ISCSI               (0x20)
#define MPI2_PORTFACTS_PORTTYPE_SAS_PHYSICAL        (0x30)
#define MPI2_PORTFACTS_PORTTYPE_SAS_VIRTUAL         (0x31)





typedef struct _MPI2_PORT_ENABLE_REQUEST
{
    U16                     Reserved1;                      
    U8                      ChainOffset;                    
    U8                      Function;                       
    U8                      Reserved2;                      
    U8                      PortFlags;                      
    U8                      Reserved3;                      
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved4;                      
} MPI2_PORT_ENABLE_REQUEST, MPI2_POINTER PTR_MPI2_PORT_ENABLE_REQUEST,
  Mpi2PortEnableRequest_t, MPI2_POINTER pMpi2PortEnableRequest_t;



typedef struct _MPI2_PORT_ENABLE_REPLY
{
    U16                     Reserved1;                      
    U8                      MsgLength;                      
    U8                      Function;                       
    U8                      Reserved2;                      
    U8                      PortFlags;                      
    U8                      Reserved3;                      
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved4;                      
    U16                     Reserved5;                      
    U16                     IOCStatus;                      
    U32                     IOCLogInfo;                     
} MPI2_PORT_ENABLE_REPLY, MPI2_POINTER PTR_MPI2_PORT_ENABLE_REPLY,
  Mpi2PortEnableReply_t, MPI2_POINTER pMpi2PortEnableReply_t;





#define MPI2_EVENT_NOTIFY_EVENTMASK_WORDS           (4)

typedef struct _MPI2_EVENT_NOTIFICATION_REQUEST
{
    U16                     Reserved1;                      
    U8                      ChainOffset;                    
    U8                      Function;                       
    U16                     Reserved2;                      
    U8                      Reserved3;                      
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved4;                      
    U32                     Reserved5;                      
    U32                     Reserved6;                      
    U32                     EventMasks[MPI2_EVENT_NOTIFY_EVENTMASK_WORDS];
    U16                     SASBroadcastPrimitiveMasks;     
    U16                     Reserved7;                      
    U32                     Reserved8;                      
} MPI2_EVENT_NOTIFICATION_REQUEST,
  MPI2_POINTER PTR_MPI2_EVENT_NOTIFICATION_REQUEST,
  Mpi2EventNotificationRequest_t, MPI2_POINTER pMpi2EventNotificationRequest_t;



typedef struct _MPI2_EVENT_NOTIFICATION_REPLY
{
    U16                     EventDataLength;                
    U8                      MsgLength;                      
    U8                      Function;                       
    U16                     Reserved1;                      
    U8                      AckRequired;                    
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved2;                      
    U16                     Reserved3;                      
    U16                     IOCStatus;                      
    U32                     IOCLogInfo;                     
    U16                     Event;                          
    U16                     Reserved4;                      
    U32                     EventContext;                   
    U32                     EventData[1];                   
} MPI2_EVENT_NOTIFICATION_REPLY, MPI2_POINTER PTR_MPI2_EVENT_NOTIFICATION_REPLY,
  Mpi2EventNotificationReply_t, MPI2_POINTER pMpi2EventNotificationReply_t;


#define MPI2_EVENT_NOTIFICATION_ACK_NOT_REQUIRED    (0x00)
#define MPI2_EVENT_NOTIFICATION_ACK_REQUIRED        (0x01)


#define MPI2_EVENT_LOG_DATA                         (0x0001)
#define MPI2_EVENT_STATE_CHANGE                     (0x0002)
#define MPI2_EVENT_HARD_RESET_RECEIVED              (0x0005)
#define MPI2_EVENT_EVENT_CHANGE                     (0x000A)
#define MPI2_EVENT_TASK_SET_FULL                    (0x000E)
#define MPI2_EVENT_SAS_DEVICE_STATUS_CHANGE         (0x000F)
#define MPI2_EVENT_IR_OPERATION_STATUS              (0x0014)
#define MPI2_EVENT_SAS_DISCOVERY                    (0x0016)
#define MPI2_EVENT_SAS_BROADCAST_PRIMITIVE          (0x0017)
#define MPI2_EVENT_SAS_INIT_DEVICE_STATUS_CHANGE    (0x0018)
#define MPI2_EVENT_SAS_INIT_TABLE_OVERFLOW          (0x0019)
#define MPI2_EVENT_SAS_TOPOLOGY_CHANGE_LIST         (0x001C)
#define MPI2_EVENT_SAS_ENCL_DEVICE_STATUS_CHANGE    (0x001D)
#define MPI2_EVENT_IR_VOLUME                        (0x001E)
#define MPI2_EVENT_IR_PHYSICAL_DISK                 (0x001F)
#define MPI2_EVENT_IR_CONFIGURATION_CHANGE_LIST     (0x0020)
#define MPI2_EVENT_LOG_ENTRY_ADDED                  (0x0021)
#define MPI2_EVENT_SAS_PHY_COUNTER                  (0x0022)





#define MPI2_EVENT_DATA_LOG_DATA_LENGTH             (0x1C)

typedef struct _MPI2_EVENT_DATA_LOG_ENTRY_ADDED
{
    U64         TimeStamp;                          
    U32         Reserved1;                          
    U16         LogSequence;                        
    U16         LogEntryQualifier;                  
    U8          VP_ID;                              
    U8          VF_ID;                              
    U16         Reserved2;                          
    U8          LogData[MPI2_EVENT_DATA_LOG_DATA_LENGTH];
} MPI2_EVENT_DATA_LOG_ENTRY_ADDED,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_LOG_ENTRY_ADDED,
  Mpi2EventDataLogEntryAdded_t, MPI2_POINTER pMpi2EventDataLogEntryAdded_t;



typedef struct _MPI2_EVENT_DATA_HARD_RESET_RECEIVED
{
    U8                      Reserved1;                      
    U8                      Port;                           
    U16                     Reserved2;                      
} MPI2_EVENT_DATA_HARD_RESET_RECEIVED,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_HARD_RESET_RECEIVED,
  Mpi2EventDataHardResetReceived_t,
  MPI2_POINTER pMpi2EventDataHardResetReceived_t;



typedef struct _MPI2_EVENT_DATA_TASK_SET_FULL
{
    U16                     DevHandle;                      
    U16                     CurrentDepth;                   
} MPI2_EVENT_DATA_TASK_SET_FULL, MPI2_POINTER PTR_MPI2_EVENT_DATA_TASK_SET_FULL,
  Mpi2EventDataTaskSetFull_t, MPI2_POINTER pMpi2EventDataTaskSetFull_t;




typedef struct _MPI2_EVENT_DATA_SAS_DEVICE_STATUS_CHANGE
{
    U16                     TaskTag;                        
    U8                      ReasonCode;                     
    U8                      Reserved1;                      
    U8                      ASC;                            
    U8                      ASCQ;                           
    U16                     DevHandle;                      
    U32                     Reserved2;                      
    U64                     SASAddress;                     
    U8                      LUN[8];                         
} MPI2_EVENT_DATA_SAS_DEVICE_STATUS_CHANGE,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_SAS_DEVICE_STATUS_CHANGE,
  Mpi2EventDataSasDeviceStatusChange_t,
  MPI2_POINTER pMpi2EventDataSasDeviceStatusChange_t;


#define MPI2_EVENT_SAS_DEV_STAT_RC_SMART_DATA                           (0x05)
#define MPI2_EVENT_SAS_DEV_STAT_RC_UNSUPPORTED                          (0x07)
#define MPI2_EVENT_SAS_DEV_STAT_RC_INTERNAL_DEVICE_RESET                (0x08)
#define MPI2_EVENT_SAS_DEV_STAT_RC_TASK_ABORT_INTERNAL                  (0x09)
#define MPI2_EVENT_SAS_DEV_STAT_RC_ABORT_TASK_SET_INTERNAL              (0x0A)
#define MPI2_EVENT_SAS_DEV_STAT_RC_CLEAR_TASK_SET_INTERNAL              (0x0B)
#define MPI2_EVENT_SAS_DEV_STAT_RC_QUERY_TASK_INTERNAL                  (0x0C)
#define MPI2_EVENT_SAS_DEV_STAT_RC_ASYNC_NOTIFICATION                   (0x0D)
#define MPI2_EVENT_SAS_DEV_STAT_RC_CMP_INTERNAL_DEV_RESET               (0x0E)
#define MPI2_EVENT_SAS_DEV_STAT_RC_CMP_TASK_ABORT_INTERNAL              (0x0F)
#define MPI2_EVENT_SAS_DEV_STAT_RC_SATA_INIT_FAILURE                    (0x10)
#define MPI2_EVENT_SAS_DEV_STAT_RC_EXPANDER_REDUCED_FUNCTIONALITY       (0x11)
#define MPI2_EVENT_SAS_DEV_STAT_RC_CMP_EXPANDER_REDUCED_FUNCTIONALITY   (0x12)




typedef struct _MPI2_EVENT_DATA_IR_OPERATION_STATUS
{
    U16                     VolDevHandle;               
    U16                     Reserved1;                  
    U8                      RAIDOperation;              
    U8                      PercentComplete;            
    U16                     Reserved2;                  
    U32                     Resereved3;                 
} MPI2_EVENT_DATA_IR_OPERATION_STATUS,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_IR_OPERATION_STATUS,
  Mpi2EventDataIrOperationStatus_t,
  MPI2_POINTER pMpi2EventDataIrOperationStatus_t;


#define MPI2_EVENT_IR_RAIDOP_RESYNC                     (0x00)
#define MPI2_EVENT_IR_RAIDOP_ONLINE_CAP_EXPANSION       (0x01)
#define MPI2_EVENT_IR_RAIDOP_CONSISTENCY_CHECK          (0x02)
#define MPI2_EVENT_IR_RAIDOP_BACKGROUND_INIT            (0x03)
#define MPI2_EVENT_IR_RAIDOP_MAKE_DATA_CONSISTENT       (0x04)




typedef struct _MPI2_EVENT_DATA_IR_VOLUME
{
    U16                     VolDevHandle;               
    U8                      ReasonCode;                 
    U8                      Reserved1;                  
    U32                     NewValue;                   
    U32                     PreviousValue;              
} MPI2_EVENT_DATA_IR_VOLUME, MPI2_POINTER PTR_MPI2_EVENT_DATA_IR_VOLUME,
  Mpi2EventDataIrVolume_t, MPI2_POINTER pMpi2EventDataIrVolume_t;


#define MPI2_EVENT_IR_VOLUME_RC_SETTINGS_CHANGED        (0x01)
#define MPI2_EVENT_IR_VOLUME_RC_STATUS_FLAGS_CHANGED    (0x02)
#define MPI2_EVENT_IR_VOLUME_RC_STATE_CHANGED           (0x03)




typedef struct _MPI2_EVENT_DATA_IR_PHYSICAL_DISK
{
    U16                     Reserved1;                  
    U8                      ReasonCode;                 
    U8                      PhysDiskNum;                
    U16                     PhysDiskDevHandle;          
    U16                     Reserved2;                  
    U16                     Slot;                       
    U16                     EnclosureHandle;            
    U32                     NewValue;                   
    U32                     PreviousValue;              
} MPI2_EVENT_DATA_IR_PHYSICAL_DISK,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_IR_PHYSICAL_DISK,
  Mpi2EventDataIrPhysicalDisk_t, MPI2_POINTER pMpi2EventDataIrPhysicalDisk_t;


#define MPI2_EVENT_IR_PHYSDISK_RC_SETTINGS_CHANGED      (0x01)
#define MPI2_EVENT_IR_PHYSDISK_RC_STATUS_FLAGS_CHANGED  (0x02)
#define MPI2_EVENT_IR_PHYSDISK_RC_STATE_CHANGED         (0x03)





#ifndef MPI2_EVENT_IR_CONFIG_ELEMENT_COUNT
#define MPI2_EVENT_IR_CONFIG_ELEMENT_COUNT          (1)
#endif

typedef struct _MPI2_EVENT_IR_CONFIG_ELEMENT
{
    U16                     ElementFlags;               
    U16                     VolDevHandle;               
    U8                      ReasonCode;                 
    U8                      PhysDiskNum;                
    U16                     PhysDiskDevHandle;          
} MPI2_EVENT_IR_CONFIG_ELEMENT, MPI2_POINTER PTR_MPI2_EVENT_IR_CONFIG_ELEMENT,
  Mpi2EventIrConfigElement_t, MPI2_POINTER pMpi2EventIrConfigElement_t;


#define MPI2_EVENT_IR_CHANGE_EFLAGS_ELEMENT_TYPE_MASK   (0x000F)
#define MPI2_EVENT_IR_CHANGE_EFLAGS_VOLUME_ELEMENT      (0x0000)
#define MPI2_EVENT_IR_CHANGE_EFLAGS_VOLPHYSDISK_ELEMENT (0x0001)
#define MPI2_EVENT_IR_CHANGE_EFLAGS_HOTSPARE_ELEMENT    (0x0002)


#define MPI2_EVENT_IR_CHANGE_RC_ADDED                   (0x01)
#define MPI2_EVENT_IR_CHANGE_RC_REMOVED                 (0x02)
#define MPI2_EVENT_IR_CHANGE_RC_NO_CHANGE               (0x03)
#define MPI2_EVENT_IR_CHANGE_RC_HIDE                    (0x04)
#define MPI2_EVENT_IR_CHANGE_RC_UNHIDE                  (0x05)
#define MPI2_EVENT_IR_CHANGE_RC_VOLUME_CREATED          (0x06)
#define MPI2_EVENT_IR_CHANGE_RC_VOLUME_DELETED          (0x07)
#define MPI2_EVENT_IR_CHANGE_RC_PD_CREATED              (0x08)
#define MPI2_EVENT_IR_CHANGE_RC_PD_DELETED              (0x09)

typedef struct _MPI2_EVENT_DATA_IR_CONFIG_CHANGE_LIST
{
    U8                              NumElements;        
    U8                              Reserved1;          
    U8                              Reserved2;          
    U8                              ConfigNum;          
    U32                             Flags;              
    MPI2_EVENT_IR_CONFIG_ELEMENT    ConfigElement[MPI2_EVENT_IR_CONFIG_ELEMENT_COUNT];    
} MPI2_EVENT_DATA_IR_CONFIG_CHANGE_LIST,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_IR_CONFIG_CHANGE_LIST,
  Mpi2EventDataIrConfigChangeList_t,
  MPI2_POINTER pMpi2EventDataIrConfigChangeList_t;


#define MPI2_EVENT_IR_CHANGE_FLAGS_FOREIGN_CONFIG   (0x00000001)




typedef struct _MPI2_EVENT_DATA_SAS_DISCOVERY
{
    U8                      Flags;                      
    U8                      ReasonCode;                 
    U8                      PhysicalPort;               
    U8                      Reserved1;                  
    U32                     DiscoveryStatus;            
} MPI2_EVENT_DATA_SAS_DISCOVERY,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_SAS_DISCOVERY,
  Mpi2EventDataSasDiscovery_t, MPI2_POINTER pMpi2EventDataSasDiscovery_t;


#define MPI2_EVENT_SAS_DISC_DEVICE_CHANGE                   (0x02)
#define MPI2_EVENT_SAS_DISC_IN_PROGRESS                     (0x01)


#define MPI2_EVENT_SAS_DISC_RC_STARTED                      (0x01)
#define MPI2_EVENT_SAS_DISC_RC_COMPLETED                    (0x02)


#define MPI2_EVENT_SAS_DISC_DS_MAX_ENCLOSURES_EXCEED            (0x80000000)
#define MPI2_EVENT_SAS_DISC_DS_MAX_EXPANDERS_EXCEED             (0x40000000)
#define MPI2_EVENT_SAS_DISC_DS_MAX_DEVICES_EXCEED               (0x20000000)
#define MPI2_EVENT_SAS_DISC_DS_MAX_TOPO_PHYS_EXCEED             (0x10000000)
#define MPI2_EVENT_SAS_DISC_DS_DOWNSTREAM_INITIATOR             (0x08000000)
#define MPI2_EVENT_SAS_DISC_DS_MULTI_SUBTRACTIVE_SUBTRACTIVE    (0x00008000)
#define MPI2_EVENT_SAS_DISC_DS_EXP_MULTI_SUBTRACTIVE            (0x00004000)
#define MPI2_EVENT_SAS_DISC_DS_MULTI_PORT_DOMAIN                (0x00002000)
#define MPI2_EVENT_SAS_DISC_DS_TABLE_TO_SUBTRACTIVE_LINK        (0x00001000)
#define MPI2_EVENT_SAS_DISC_DS_UNSUPPORTED_DEVICE               (0x00000800)
#define MPI2_EVENT_SAS_DISC_DS_TABLE_LINK                       (0x00000400)
#define MPI2_EVENT_SAS_DISC_DS_SUBTRACTIVE_LINK                 (0x00000200)
#define MPI2_EVENT_SAS_DISC_DS_SMP_CRC_ERROR                    (0x00000100)
#define MPI2_EVENT_SAS_DISC_DS_SMP_FUNCTION_FAILED              (0x00000080)
#define MPI2_EVENT_SAS_DISC_DS_INDEX_NOT_EXIST                  (0x00000040)
#define MPI2_EVENT_SAS_DISC_DS_OUT_ROUTE_ENTRIES                (0x00000020)
#define MPI2_EVENT_SAS_DISC_DS_SMP_TIMEOUT                      (0x00000010)
#define MPI2_EVENT_SAS_DISC_DS_MULTIPLE_PORTS                   (0x00000004)
#define MPI2_EVENT_SAS_DISC_DS_UNADDRESSABLE_DEVICE             (0x00000002)
#define MPI2_EVENT_SAS_DISC_DS_LOOP_DETECTED                    (0x00000001)




typedef struct _MPI2_EVENT_DATA_SAS_BROADCAST_PRIMITIVE
{
    U8                      PhyNum;                     
    U8                      Port;                       
    U8                      PortWidth;                  
    U8                      Primitive;                  
} MPI2_EVENT_DATA_SAS_BROADCAST_PRIMITIVE,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_SAS_BROADCAST_PRIMITIVE,
  Mpi2EventDataSasBroadcastPrimitive_t,
  MPI2_POINTER pMpi2EventDataSasBroadcastPrimitive_t;


#define MPI2_EVENT_PRIMITIVE_CHANGE                         (0x01)
#define MPI2_EVENT_PRIMITIVE_SES                            (0x02)
#define MPI2_EVENT_PRIMITIVE_EXPANDER                       (0x03)
#define MPI2_EVENT_PRIMITIVE_ASYNCHRONOUS_EVENT             (0x04)
#define MPI2_EVENT_PRIMITIVE_RESERVED3                      (0x05)
#define MPI2_EVENT_PRIMITIVE_RESERVED4                      (0x06)
#define MPI2_EVENT_PRIMITIVE_CHANGE0_RESERVED               (0x07)
#define MPI2_EVENT_PRIMITIVE_CHANGE1_RESERVED               (0x08)




typedef struct _MPI2_EVENT_DATA_SAS_INIT_DEV_STATUS_CHANGE
{
    U8                      ReasonCode;                 
    U8                      PhysicalPort;               
    U16                     DevHandle;                  
    U64                     SASAddress;                 
} MPI2_EVENT_DATA_SAS_INIT_DEV_STATUS_CHANGE,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_SAS_INIT_DEV_STATUS_CHANGE,
  Mpi2EventDataSasInitDevStatusChange_t,
  MPI2_POINTER pMpi2EventDataSasInitDevStatusChange_t;


#define MPI2_EVENT_SAS_INIT_RC_ADDED                (0x01)
#define MPI2_EVENT_SAS_INIT_RC_NOT_RESPONDING       (0x02)




typedef struct _MPI2_EVENT_DATA_SAS_INIT_TABLE_OVERFLOW
{
    U16                     MaxInit;                    
    U16                     CurrentInit;                
    U64                     SASAddress;                 
} MPI2_EVENT_DATA_SAS_INIT_TABLE_OVERFLOW,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_SAS_INIT_TABLE_OVERFLOW,
  Mpi2EventDataSasInitTableOverflow_t,
  MPI2_POINTER pMpi2EventDataSasInitTableOverflow_t;





#ifndef MPI2_EVENT_SAS_TOPO_PHY_COUNT
#define MPI2_EVENT_SAS_TOPO_PHY_COUNT           (1)
#endif

typedef struct _MPI2_EVENT_SAS_TOPO_PHY_ENTRY
{
    U16                     AttachedDevHandle;          
    U8                      LinkRate;                   
    U8                      PhyStatus;                  
} MPI2_EVENT_SAS_TOPO_PHY_ENTRY, MPI2_POINTER PTR_MPI2_EVENT_SAS_TOPO_PHY_ENTRY,
  Mpi2EventSasTopoPhyEntry_t, MPI2_POINTER pMpi2EventSasTopoPhyEntry_t;

typedef struct _MPI2_EVENT_DATA_SAS_TOPOLOGY_CHANGE_LIST
{
    U16                             EnclosureHandle;            
    U16                             ExpanderDevHandle;          
    U8                              NumPhys;                    
    U8                              Reserved1;                  
    U16                             Reserved2;                  
    U8                              NumEntries;                 
    U8                              StartPhyNum;                
    U8                              ExpStatus;                  
    U8                              PhysicalPort;               
    MPI2_EVENT_SAS_TOPO_PHY_ENTRY   PHY[MPI2_EVENT_SAS_TOPO_PHY_COUNT]; 
} MPI2_EVENT_DATA_SAS_TOPOLOGY_CHANGE_LIST,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_SAS_TOPOLOGY_CHANGE_LIST,
  Mpi2EventDataSasTopologyChangeList_t,
  MPI2_POINTER pMpi2EventDataSasTopologyChangeList_t;


#define MPI2_EVENT_SAS_TOPO_ES_ADDED                        (0x01)
#define MPI2_EVENT_SAS_TOPO_ES_NOT_RESPONDING               (0x02)
#define MPI2_EVENT_SAS_TOPO_ES_RESPONDING                   (0x03)
#define MPI2_EVENT_SAS_TOPO_ES_DELAY_NOT_RESPONDING         (0x04)


#define MPI2_EVENT_SAS_TOPO_LR_CURRENT_MASK                 (0xF0)
#define MPI2_EVENT_SAS_TOPO_LR_CURRENT_SHIFT                (4)
#define MPI2_EVENT_SAS_TOPO_LR_PREV_MASK                    (0x0F)
#define MPI2_EVENT_SAS_TOPO_LR_PREV_SHIFT                   (0)

#define MPI2_EVENT_SAS_TOPO_LR_UNKNOWN_LINK_RATE            (0x00)
#define MPI2_EVENT_SAS_TOPO_LR_PHY_DISABLED                 (0x01)
#define MPI2_EVENT_SAS_TOPO_LR_NEGOTIATION_FAILED           (0x02)
#define MPI2_EVENT_SAS_TOPO_LR_SATA_OOB_COMPLETE            (0x03)
#define MPI2_EVENT_SAS_TOPO_LR_PORT_SELECTOR                (0x04)
#define MPI2_EVENT_SAS_TOPO_LR_SMP_RESET_IN_PROGRESS        (0x05)
#define MPI2_EVENT_SAS_TOPO_LR_RATE_1_5                     (0x08)
#define MPI2_EVENT_SAS_TOPO_LR_RATE_3_0                     (0x09)
#define MPI2_EVENT_SAS_TOPO_LR_RATE_6_0                     (0x0A)


#define MPI2_EVENT_SAS_TOPO_PHYSTATUS_VACANT                (0x80)
#define MPI2_EVENT_SAS_TOPO_PS_MULTIPLEX_CHANGE             (0x10)

#define MPI2_EVENT_SAS_TOPO_RC_MASK                         (0x0F)
#define MPI2_EVENT_SAS_TOPO_RC_TARG_ADDED                   (0x01)
#define MPI2_EVENT_SAS_TOPO_RC_TARG_NOT_RESPONDING          (0x02)
#define MPI2_EVENT_SAS_TOPO_RC_PHY_CHANGED                  (0x03)
#define MPI2_EVENT_SAS_TOPO_RC_NO_CHANGE                    (0x04)
#define MPI2_EVENT_SAS_TOPO_RC_DELAY_NOT_RESPONDING         (0x05)




typedef struct _MPI2_EVENT_DATA_SAS_ENCL_DEV_STATUS_CHANGE
{
    U16                     EnclosureHandle;            
    U8                      ReasonCode;                 
    U8                      PhysicalPort;               
    U64                     EnclosureLogicalID;         
    U16                     NumSlots;                   
    U16                     StartSlot;                  
    U32                     PhyBits;                    
} MPI2_EVENT_DATA_SAS_ENCL_DEV_STATUS_CHANGE,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_SAS_ENCL_DEV_STATUS_CHANGE,
  Mpi2EventDataSasEnclDevStatusChange_t,
  MPI2_POINTER pMpi2EventDataSasEnclDevStatusChange_t;


#define MPI2_EVENT_SAS_ENCL_RC_ADDED                (0x01)
#define MPI2_EVENT_SAS_ENCL_RC_NOT_RESPONDING       (0x02)




typedef struct _MPI2_EVENT_DATA_SAS_PHY_COUNTER {
    U64         TimeStamp;          
    U32         Reserved1;          
    U8          PhyEventCode;       
    U8          PhyNum;             
    U16         Reserved2;          
    U32         PhyEventInfo;       
    U8          CounterType;        
    U8          ThresholdWindow;    
    U8          TimeUnits;          
    U8          Reserved3;          
    U32         EventThreshold;     
    U16         ThresholdFlags;     
    U16         Reserved4;          
} MPI2_EVENT_DATA_SAS_PHY_COUNTER,
  MPI2_POINTER PTR_MPI2_EVENT_DATA_SAS_PHY_COUNTER,
  Mpi2EventDataSasPhyCounter_t, MPI2_POINTER pMpi2EventDataSasPhyCounter_t;







typedef struct _MPI2_EVENT_ACK_REQUEST
{
    U16                     Reserved1;                      
    U8                      ChainOffset;                    
    U8                      Function;                       
    U16                     Reserved2;                      
    U8                      Reserved3;                      
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved4;                      
    U16                     Event;                          
    U16                     Reserved5;                      
    U32                     EventContext;                   
} MPI2_EVENT_ACK_REQUEST, MPI2_POINTER PTR_MPI2_EVENT_ACK_REQUEST,
  Mpi2EventAckRequest_t, MPI2_POINTER pMpi2EventAckRequest_t;



typedef struct _MPI2_EVENT_ACK_REPLY
{
    U16                     Reserved1;                      
    U8                      MsgLength;                      
    U8                      Function;                       
    U16                     Reserved2;                      
    U8                      Reserved3;                      
    U8                      MsgFlags;                       
    U8                      VP_ID;                          
    U8                      VF_ID;                          
    U16                     Reserved4;                      
    U16                     Reserved5;                      
    U16                     IOCStatus;                      
    U32                     IOCLogInfo;                     
} MPI2_EVENT_ACK_REPLY, MPI2_POINTER PTR_MPI2_EVENT_ACK_REPLY,
  Mpi2EventAckReply_t, MPI2_POINTER pMpi2EventAckReply_t;





typedef struct _MPI2_FW_DOWNLOAD_REQUEST
{
    U8                      ImageType;                  
    U8                      Reserved1;                  
    U8                      ChainOffset;                
    U8                      Function;                   
    U16                     Reserved2;                  
    U8                      Reserved3;                  
    U8                      MsgFlags;                   
    U8                      VP_ID;                      
    U8                      VF_ID;                      
    U16                     Reserved4;                  
    U32                     TotalImageSize;             
    U32                     Reserved5;                  
    MPI2_MPI_SGE_UNION      SGL;                        
} MPI2_FW_DOWNLOAD_REQUEST, MPI2_POINTER PTR_MPI2_FW_DOWNLOAD_REQUEST,
  Mpi2FWDownloadRequest, MPI2_POINTER pMpi2FWDownloadRequest;

#define MPI2_FW_DOWNLOAD_MSGFLGS_LAST_SEGMENT   (0x01)

#define MPI2_FW_DOWNLOAD_ITYPE_FW                   (0x01)
#define MPI2_FW_DOWNLOAD_ITYPE_BIOS                 (0x02)
#define MPI2_FW_DOWNLOAD_ITYPE_MANUFACTURING        (0x06)
#define MPI2_FW_DOWNLOAD_ITYPE_CONFIG_1             (0x07)
#define MPI2_FW_DOWNLOAD_ITYPE_CONFIG_2             (0x08)
#define MPI2_FW_DOWNLOAD_ITYPE_MEGARAID             (0x09)
#define MPI2_FW_DOWNLOAD_ITYPE_COMMON_BOOT_BLOCK    (0x0B)


typedef struct _MPI2_FW_DOWNLOAD_TCSGE
{
    U8                      Reserved1;                  
    U8                      ContextSize;                
    U8                      DetailsLength;              
    U8                      Flags;                      
    U32                     Reserved2;                  
    U32                     ImageOffset;                
    U32                     ImageSize;                  
} MPI2_FW_DOWNLOAD_TCSGE, MPI2_POINTER PTR_MPI2_FW_DOWNLOAD_TCSGE,
  Mpi2FWDownloadTCSGE_t, MPI2_POINTER pMpi2FWDownloadTCSGE_t;


typedef struct _MPI2_FW_DOWNLOAD_REPLY
{
    U8                      ImageType;                  
    U8                      Reserved1;                  
    U8                      MsgLength;                  
    U8                      Function;                   
    U16                     Reserved2;                  
    U8                      Reserved3;                  
    U8                      MsgFlags;                   
    U8                      VP_ID;                      
    U8                      VF_ID;                      
    U16                     Reserved4;                  
    U16                     Reserved5;                  
    U16                     IOCStatus;                  
    U32                     IOCLogInfo;                 
} MPI2_FW_DOWNLOAD_REPLY, MPI2_POINTER PTR_MPI2_FW_DOWNLOAD_REPLY,
  Mpi2FWDownloadReply_t, MPI2_POINTER pMpi2FWDownloadReply_t;





typedef struct _MPI2_FW_UPLOAD_REQUEST
{
    U8                      ImageType;                  
    U8                      Reserved1;                  
    U8                      ChainOffset;                
    U8                      Function;                   
    U16                     Reserved2;                  
    U8                      Reserved3;                  
    U8                      MsgFlags;                   
    U8                      VP_ID;                      
    U8                      VF_ID;                      
    U16                     Reserved4;                  
    U32                     Reserved5;                  
    U32                     Reserved6;                  
    MPI2_MPI_SGE_UNION      SGL;                        
} MPI2_FW_UPLOAD_REQUEST, MPI2_POINTER PTR_MPI2_FW_UPLOAD_REQUEST,
  Mpi2FWUploadRequest_t, MPI2_POINTER pMpi2FWUploadRequest_t;

#define MPI2_FW_UPLOAD_ITYPE_FW_CURRENT         (0x00)
#define MPI2_FW_UPLOAD_ITYPE_FW_FLASH           (0x01)
#define MPI2_FW_UPLOAD_ITYPE_BIOS_FLASH         (0x02)
#define MPI2_FW_UPLOAD_ITYPE_FW_BACKUP          (0x05)
#define MPI2_FW_UPLOAD_ITYPE_MANUFACTURING      (0x06)
#define MPI2_FW_UPLOAD_ITYPE_CONFIG_1           (0x07)
#define MPI2_FW_UPLOAD_ITYPE_CONFIG_2           (0x08)
#define MPI2_FW_UPLOAD_ITYPE_MEGARAID           (0x09)
#define MPI2_FW_UPLOAD_ITYPE_COMPLETE           (0x0A)
#define MPI2_FW_UPLOAD_ITYPE_COMMON_BOOT_BLOCK  (0x0B)

typedef struct _MPI2_FW_UPLOAD_TCSGE
{
    U8                      Reserved1;                  
    U8                      ContextSize;                
    U8                      DetailsLength;              
    U8                      Flags;                      
    U32                     Reserved2;                  
    U32                     ImageOffset;                
    U32                     ImageSize;                  
} MPI2_FW_UPLOAD_TCSGE, MPI2_POINTER PTR_MPI2_FW_UPLOAD_TCSGE,
  Mpi2FWUploadTCSGE_t, MPI2_POINTER pMpi2FWUploadTCSGE_t;


typedef struct _MPI2_FW_UPLOAD_REPLY
{
    U8                      ImageType;                  
    U8                      Reserved1;                  
    U8                      MsgLength;                  
    U8                      Function;                   
    U16                     Reserved2;                  
    U8                      Reserved3;                  
    U8                      MsgFlags;                   
    U8                      VP_ID;                      
    U8                      VF_ID;                      
    U16                     Reserved4;                  
    U16                     Reserved5;                  
    U16                     IOCStatus;                  
    U32                     IOCLogInfo;                 
    U32                     ActualImageSize;            
} MPI2_FW_UPLOAD_REPLY, MPI2_POINTER PTR_MPI2_FW_UPLOAD_REPLY,
  Mpi2FWUploadReply_t, MPI2_POINTER pMPi2FWUploadReply_t;



typedef struct _MPI2_FW_IMAGE_HEADER
{
    U32                     Signature;                  
    U32                     Signature0;                 
    U32                     Signature1;                 
    U32                     Signature2;                 
    MPI2_VERSION_UNION      MPIVersion;                 
    MPI2_VERSION_UNION      FWVersion;                  
    MPI2_VERSION_UNION      NVDATAVersion;              
    MPI2_VERSION_UNION      PackageVersion;             
    U16                     VendorID;                   
    U16                     ProductID;                  
    U16                     ProtocolFlags;              
    U16                     Reserved26;                 
    U32                     IOCCapabilities;            
    U32                     ImageSize;                  
    U32                     NextImageHeaderOffset;      
    U32                     Checksum;                   
    U32                     Reserved38;                 
    U32                     Reserved3C;                 
    U32                     Reserved40;                 
    U32                     Reserved44;                 
    U32                     Reserved48;                 
    U32                     Reserved4C;                 
    U32                     Reserved50;                 
    U32                     Reserved54;                 
    U32                     Reserved58;                 
    U32                     Reserved5C;                 
    U32                     Reserved60;                 
    U32                     FirmwareVersionNameWhat;    
    U8                      FirmwareVersionName[32];    
    U32                     VendorNameWhat;             
    U8                      VendorName[32];             
    U32                     PackageNameWhat;            
    U8                      PackageName[32];            
    U32                     ReservedD0;                 
    U32                     ReservedD4;                 
    U32                     ReservedD8;                 
    U32                     ReservedDC;                 
    U32                     ReservedE0;                 
    U32                     ReservedE4;                 
    U32                     ReservedE8;                 
    U32                     ReservedEC;                 
    U32                     ReservedF0;                 
    U32                     ReservedF4;                 
    U32                     ReservedF8;                 
    U32                     ReservedFC;                 
} MPI2_FW_IMAGE_HEADER, MPI2_POINTER PTR_MPI2_FW_IMAGE_HEADER,
  Mpi2FWImageHeader_t, MPI2_POINTER pMpi2FWImageHeader_t;


#define MPI2_FW_HEADER_SIGNATURE_OFFSET         (0x00)
#define MPI2_FW_HEADER_SIGNATURE_MASK           (0xFF000000)
#define MPI2_FW_HEADER_SIGNATURE                (0xEA000000)


#define MPI2_FW_HEADER_SIGNATURE0_OFFSET        (0x04)
#define MPI2_FW_HEADER_SIGNATURE0               (0x5AFAA55A)


#define MPI2_FW_HEADER_SIGNATURE1_OFFSET        (0x08)
#define MPI2_FW_HEADER_SIGNATURE1               (0xA55AFAA5)


#define MPI2_FW_HEADER_SIGNATURE2_OFFSET        (0x0C)
#define MPI2_FW_HEADER_SIGNATURE2               (0x5AA55AFA)



#define MPI2_FW_HEADER_PID_TYPE_MASK            (0xF000)
#define MPI2_FW_HEADER_PID_TYPE_SAS             (0x2000)

#define MPI2_FW_HEADER_PID_PROD_MASK            (0x0F00)
#define MPI2_FW_HEADER_PID_PROD_A               (0x0000)

#define MPI2_FW_HEADER_PID_FAMILY_MASK          (0x00FF)

#define MPI2_FW_HEADER_PID_FAMILY_2108_SAS      (0x0010)






#define MPI2_FW_HEADER_IMAGESIZE_OFFSET         (0x2C)
#define MPI2_FW_HEADER_NEXTIMAGE_OFFSET         (0x30)
#define MPI2_FW_HEADER_VERNMHWAT_OFFSET         (0x64)

#define MPI2_FW_HEADER_WHAT_SIGNATURE           (0x29232840)

#define MPI2_FW_HEADER_SIZE                     (0x100)



typedef struct _MPI2_EXT_IMAGE_HEADER

{
    U8                      ImageType;                  
    U8                      Reserved1;                  
    U16                     Reserved2;                  
    U32                     Checksum;                   
    U32                     ImageSize;                  
    U32                     NextImageHeaderOffset;      
    U32                     PackageVersion;             
    U32                     Reserved3;                  
    U32                     Reserved4;                  
    U32                     Reserved5;                  
    U8                      IdentifyString[32];         
} MPI2_EXT_IMAGE_HEADER, MPI2_POINTER PTR_MPI2_EXT_IMAGE_HEADER,
  Mpi2ExtImageHeader_t, MPI2_POINTER pMpi2ExtImageHeader_t;


#define MPI2_EXT_IMAGE_IMAGETYPE_OFFSET         (0x00)
#define MPI2_EXT_IMAGE_IMAGESIZE_OFFSET         (0x08)
#define MPI2_EXT_IMAGE_NEXTIMAGE_OFFSET         (0x0C)

#define MPI2_EXT_IMAGE_HEADER_SIZE              (0x40)


#define MPI2_EXT_IMAGE_TYPE_UNSPECIFIED         (0x00)
#define MPI2_EXT_IMAGE_TYPE_FW                  (0x01)
#define MPI2_EXT_IMAGE_TYPE_NVDATA              (0x03)
#define MPI2_EXT_IMAGE_TYPE_BOOTLOADER          (0x04)
#define MPI2_EXT_IMAGE_TYPE_INITIALIZATION      (0x05)
#define MPI2_EXT_IMAGE_TYPE_FLASH_LAYOUT        (0x06)
#define MPI2_EXT_IMAGE_TYPE_SUPPORTED_DEVICES   (0x07)
#define MPI2_EXT_IMAGE_TYPE_MEGARAID            (0x08)

#define MPI2_EXT_IMAGE_TYPE_MAX                 (MPI2_EXT_IMAGE_TYPE_MEGARAID)






#ifndef MPI2_FLASH_NUMBER_OF_REGIONS
#define MPI2_FLASH_NUMBER_OF_REGIONS        (1)
#endif


#ifndef MPI2_FLASH_NUMBER_OF_LAYOUTS
#define MPI2_FLASH_NUMBER_OF_LAYOUTS        (1)
#endif

typedef struct _MPI2_FLASH_REGION
{
    U8                      RegionType;                 
    U8                      Reserved1;                  
    U16                     Reserved2;                  
    U32                     RegionOffset;               
    U32                     RegionSize;                 
    U32                     Reserved3;                  
} MPI2_FLASH_REGION, MPI2_POINTER PTR_MPI2_FLASH_REGION,
  Mpi2FlashRegion_t, MPI2_POINTER pMpi2FlashRegion_t;

typedef struct _MPI2_FLASH_LAYOUT
{
    U32                     FlashSize;                  
    U32                     Reserved1;                  
    U32                     Reserved2;                  
    U32                     Reserved3;                  
    MPI2_FLASH_REGION       Region[MPI2_FLASH_NUMBER_OF_REGIONS];
} MPI2_FLASH_LAYOUT, MPI2_POINTER PTR_MPI2_FLASH_LAYOUT,
  Mpi2FlashLayout_t, MPI2_POINTER pMpi2FlashLayout_t;

typedef struct _MPI2_FLASH_LAYOUT_DATA
{
    U8                      ImageRevision;              
    U8                      Reserved1;                  
    U8                      SizeOfRegion;               
    U8                      Reserved2;                  
    U16                     NumberOfLayouts;            
    U16                     RegionsPerLayout;           
    U16                     MinimumSectorAlignment;     
    U16                     Reserved3;                  
    U32                     Reserved4;                  
    MPI2_FLASH_LAYOUT       Layout[MPI2_FLASH_NUMBER_OF_LAYOUTS];
} MPI2_FLASH_LAYOUT_DATA, MPI2_POINTER PTR_MPI2_FLASH_LAYOUT_DATA,
  Mpi2FlashLayoutData_t, MPI2_POINTER pMpi2FlashLayoutData_t;


#define MPI2_FLASH_REGION_UNUSED                (0x00)
#define MPI2_FLASH_REGION_FIRMWARE              (0x01)
#define MPI2_FLASH_REGION_BIOS                  (0x02)
#define MPI2_FLASH_REGION_NVDATA                (0x03)
#define MPI2_FLASH_REGION_FIRMWARE_BACKUP       (0x05)
#define MPI2_FLASH_REGION_MFG_INFORMATION       (0x06)
#define MPI2_FLASH_REGION_CONFIG_1              (0x07)
#define MPI2_FLASH_REGION_CONFIG_2              (0x08)
#define MPI2_FLASH_REGION_MEGARAID              (0x09)
#define MPI2_FLASH_REGION_INIT                  (0x0A)


#define MPI2_FLASH_LAYOUT_IMAGE_REVISION        (0x00)






#ifndef MPI2_SUPPORTED_DEVICES_IMAGE_NUM_DEVICES
#define MPI2_SUPPORTED_DEVICES_IMAGE_NUM_DEVICES    (1)
#endif

typedef struct _MPI2_SUPPORTED_DEVICE
{
    U16                     DeviceID;                   
    U16                     VendorID;                   
    U16                     DeviceIDMask;               
    U16                     Reserved1;                  
    U8                      LowPCIRev;                  
    U8                      HighPCIRev;                 
    U16                     Reserved2;                  
    U32                     Reserved3;                  
} MPI2_SUPPORTED_DEVICE, MPI2_POINTER PTR_MPI2_SUPPORTED_DEVICE,
  Mpi2SupportedDevice_t, MPI2_POINTER pMpi2SupportedDevice_t;

typedef struct _MPI2_SUPPORTED_DEVICES_DATA
{
    U8                      ImageRevision;              
    U8                      Reserved1;                  
    U8                      NumberOfDevices;            
    U8                      Reserved2;                  
    U32                     Reserved3;                  
    MPI2_SUPPORTED_DEVICE   SupportedDevice[MPI2_SUPPORTED_DEVICES_IMAGE_NUM_DEVICES]; 
} MPI2_SUPPORTED_DEVICES_DATA, MPI2_POINTER PTR_MPI2_SUPPORTED_DEVICES_DATA,
  Mpi2SupportedDevicesData_t, MPI2_POINTER pMpi2SupportedDevicesData_t;


#define MPI2_SUPPORTED_DEVICES_IMAGE_REVISION   (0x00)




typedef struct _MPI2_INIT_IMAGE_FOOTER

{
    U32                     BootFlags;                  
    U32                     ImageSize;                  
    U32                     Signature0;                 
    U32                     Signature1;                 
    U32                     Signature2;                 
    U32                     ResetVector;                
} MPI2_INIT_IMAGE_FOOTER, MPI2_POINTER PTR_MPI2_INIT_IMAGE_FOOTER,
  Mpi2InitImageFooter_t, MPI2_POINTER pMpi2InitImageFooter_t;


#define MPI2_INIT_IMAGE_BOOTFLAGS_OFFSET        (0x00)


#define MPI2_INIT_IMAGE_IMAGESIZE_OFFSET        (0x04)


#define MPI2_INIT_IMAGE_SIGNATURE0_OFFSET       (0x08)
#define MPI2_INIT_IMAGE_SIGNATURE0              (0x5AA55AEA)


#define MPI2_INIT_IMAGE_SIGNATURE1_OFFSET       (0x0C)
#define MPI2_INIT_IMAGE_SIGNATURE1              (0xA55AEAA5)


#define MPI2_INIT_IMAGE_SIGNATURE2_OFFSET       (0x10)
#define MPI2_INIT_IMAGE_SIGNATURE2              (0x5AEAA55A)


#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_0        (0xEA)
#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_1        (0x5A)
#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_2        (0xA5)
#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_3        (0x5A)

#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_4        (0xA5)
#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_5        (0xEA)
#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_6        (0x5A)
#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_7        (0xA5)

#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_8        (0x5A)
#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_9        (0xA5)
#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_A        (0xEA)
#define MPI2_INIT_IMAGE_SIGNATURE_BYTE_B        (0x5A)


#define MPI2_INIT_IMAGE_RESETVECTOR_OFFSET      (0x14)


#endif

