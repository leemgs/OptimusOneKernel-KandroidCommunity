#ifndef _GDTH_IOCTL_H
#define _GDTH_IOCTL_H




#define GDTIOCTL_MASK       ('J'<<8)
#define GDTIOCTL_GENERAL    (GDTIOCTL_MASK | 0) 
#define GDTIOCTL_DRVERS     (GDTIOCTL_MASK | 1) 
#define GDTIOCTL_CTRTYPE    (GDTIOCTL_MASK | 2) 
#define GDTIOCTL_OSVERS     (GDTIOCTL_MASK | 3) 
#define GDTIOCTL_HDRLIST    (GDTIOCTL_MASK | 4) 
#define GDTIOCTL_CTRCNT     (GDTIOCTL_MASK | 5) 
#define GDTIOCTL_LOCKDRV    (GDTIOCTL_MASK | 6) 
#define GDTIOCTL_LOCKCHN    (GDTIOCTL_MASK | 7) 
#define GDTIOCTL_EVENT      (GDTIOCTL_MASK | 8) 
#define GDTIOCTL_SCSI       (GDTIOCTL_MASK | 9) 
#define GDTIOCTL_RESET_BUS  (GDTIOCTL_MASK |10) 
#define GDTIOCTL_RESCAN     (GDTIOCTL_MASK |11) 
#define GDTIOCTL_RESET_DRV  (GDTIOCTL_MASK |12) 

#define GDTIOCTL_MAGIC  0xaffe0004
#define EVENT_SIZE      294 
#define GDTH_MAXSG      32                      

#define MAX_LDRIVES     255                     
#ifdef GDTH_IOCTL_PROC
#define MAX_HDRIVES     100                     
#else
#define MAX_HDRIVES     MAX_LDRIVES             
#endif


#ifdef __KERNEL__
typedef u32     ulong32;
typedef u64     ulong64;
#endif

#define PACKED  __attribute__((packed))


typedef struct {
    ulong32     sg_ptr;                         
    ulong32     sg_len;                         
} PACKED gdth_sg_str;


typedef struct {
    ulong64     sg_ptr;                         
    ulong32     sg_len;                         
} PACKED gdth_sg64_str;


typedef struct {
    ulong32     BoardNode;                      
    ulong32     CommandIndex;                   
    ushort      OpCode;                         
    union {
        struct {
            ushort      DeviceNo;               
            ulong32     BlockNo;                
            ulong32     BlockCnt;               
            ulong32     DestAddr;               
            ulong32     sg_canz;                
            gdth_sg_str sg_lst[GDTH_MAXSG];     
        } PACKED cache;                         
        struct {
            ushort      DeviceNo;               
            ulong64     BlockNo;                
            ulong32     BlockCnt;               
            ulong64     DestAddr;               
            ulong32     sg_canz;                
            gdth_sg64_str sg_lst[GDTH_MAXSG];   
        } PACKED cache64;                       
        struct {
            ushort      param_size;             
            ulong32     subfunc;                
            ulong32     channel;                
            ulong64     p_param;                
        } PACKED ioctl;                         
        struct {
            ushort      reserved;
            union {
                struct {
                    ulong32  msg_handle;        
                    ulong64  msg_addr;          
                } PACKED msg;
                unchar       data[12];          
            } su;
        } PACKED screen;                        
        struct {
            ushort      reserved;
            ulong32     direction;              
            ulong32     mdisc_time;             
            ulong32     mcon_time;              
            ulong32     sdata;                  
            ulong32     sdlen;                  
            ulong32     clen;                   
            unchar      cmd[12];                
            unchar      target;                 
            unchar      lun;                    
            unchar      bus;                    
            unchar      priority;               
            ulong32     sense_len;              
            ulong32     sense_data;             
            ulong32     link_p;                 
            ulong32     sg_ranz;                
            gdth_sg_str sg_lst[GDTH_MAXSG];     
        } PACKED raw;                           
        struct {
            ushort      reserved;
            ulong32     direction;              
            ulong32     mdisc_time;             
            ulong32     mcon_time;              
            ulong64     sdata;                  
            ulong32     sdlen;                  
            ulong32     clen;                   
            unchar      cmd[16];                
            unchar      target;                 
            unchar      lun;                    
            unchar      bus;                    
            unchar      priority;               
            ulong32     sense_len;              
            ulong64     sense_data;             
            ulong32     sg_ranz;                
            gdth_sg64_str sg_lst[GDTH_MAXSG];   
        } PACKED raw64;                         
    } u;
    
    unchar      Service;                        
    unchar      reserved;
    ushort      Status;                         
    ulong32     Info;                           
    void        *RequestBuffer;                 
} PACKED gdth_cmd_str;


#define ES_ASYNC    1
#define ES_DRIVER   2
#define ES_TEST     3
#define ES_SYNC     4
typedef struct {
    ushort                  size;               
    union {
        char                stream[16];
        struct {
            ushort          ionode;
            ushort          service;
            ulong32         index;
        } PACKED driver;
        struct {
            ushort          ionode;
            ushort          service;
            ushort          status;
            ulong32         info;
            unchar          scsi_coord[3];
        } PACKED async;
        struct {
            ushort          ionode;
            ushort          service;
            ushort          status;
            ulong32         info;
            ushort          hostdrive;
            unchar          scsi_coord[3];
            unchar          sense_key;
        } PACKED sync;
        struct {
            ulong32         l1, l2, l3, l4;
        } PACKED test;
    } eu;
    ulong32                 severity;
    unchar                  event_string[256];          
} PACKED gdth_evt_data;

typedef struct {
    ulong32         first_stamp;
    ulong32         last_stamp;
    ushort          same_count;
    ushort          event_source;
    ushort          event_idx;
    unchar          application;
    unchar          reserved;
    gdth_evt_data   event_data;
} PACKED gdth_evt_str;


#ifdef GDTH_IOCTL_PROC

typedef struct {
    ulong32                 magic;              
    ushort                  ioctl;              
    ushort                  ionode;             
    ushort                  service;            
    ushort                  timeout;            
    union {
        struct {
            unchar          command[512];       
            unchar          data[1];            
        } general;
        struct {
            unchar          lock;               
            unchar          drive_cnt;          
            ushort          drives[MAX_HDRIVES];
        } lockdrv;
        struct {
            unchar          lock;               
            unchar          channel;            
        } lockchn;
        struct {
            int             erase;              
            int             handle;
            unchar          evt[EVENT_SIZE];    
        } event;
        struct {
            unchar          bus;                
            unchar          target;             
            unchar          lun;                
            unchar          cmd_len;            
            unchar          cmd[12];            
        } scsi;
        struct {
            ushort          hdr_no;             
            unchar          flag;               
        } rescan;
    } iu;
} gdth_iowr_str;


typedef struct {
    ulong32                 size;               
    ulong32                 status;             
    union {
        struct {
            unchar          data[1];            
        } general;
        struct {
            ushort          version;            
        } drvers;
        struct {
            unchar          type;               
            ushort          info;               
            ushort          oem_id;             
            ushort          bios_ver;           
            ushort          access;             
            ushort          ext_type;           
            ushort          device_id;          
            ushort          sub_device_id;      
        } ctrtype;
        struct {
            unchar          version;            
            unchar          subversion;         
            ushort          revision;           
        } osvers;
        struct {
            ushort          count;              
        } ctrcnt;
        struct {
            int             handle;
            unchar          evt[EVENT_SIZE];    
        } event;
        struct {
            unchar          bus;                
            unchar          target;             
            unchar          lun;                
            unchar          cluster_type;       
        } hdr_list[MAX_HDRIVES];                
    } iu;
} gdth_iord_str;
#endif


typedef struct {
    ushort ionode;                              
    ushort timeout;                             
    ulong32 info;                                
    ushort status;                              
    ulong data_len;                             
    ulong sense_len;                            
    gdth_cmd_str command;                                          
} gdth_ioctl_general;


typedef struct {
    ushort ionode;                              
    unchar lock;                                
    unchar drive_cnt;                           
    ushort drives[MAX_HDRIVES];                 
} gdth_ioctl_lockdrv;


typedef struct {
    ushort ionode;                              
    unchar lock;                                
    unchar channel;                             
} gdth_ioctl_lockchn;


typedef struct {
    unchar version;                             
    unchar subversion;                          
    ushort revision;                            
} gdth_ioctl_osvers;


typedef struct {
    ushort ionode;                              
    unchar type;                                
    ushort info;                                
    ushort oem_id;                              
    ushort bios_ver;                            
    ushort access;                              
    ushort ext_type;                            
    ushort device_id;                           
    ushort sub_device_id;                       
} gdth_ioctl_ctrtype;


typedef struct {
    ushort ionode;
    int erase;                                  
    int handle;                                 
    gdth_evt_str event;
} gdth_ioctl_event;


typedef struct {
    ushort ionode;                              
    unchar flag;                                
    ushort hdr_no;                              
    struct {
        unchar bus;                             
        unchar target;                          
        unchar lun;                             
        unchar cluster_type;                    
    } hdr_list[MAX_HDRIVES];                    
} gdth_ioctl_rescan;


typedef struct {
    ushort ionode;                              
    ushort number;                              
    ushort status;                              
} gdth_ioctl_reset;

#endif
