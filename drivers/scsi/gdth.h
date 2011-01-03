#ifndef _GDTH_H
#define _GDTH_H



#include <linux/types.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif




#define GDTH_VERSION_STR        "3.05"
#define GDTH_VERSION            3
#define GDTH_SUBVERSION         5


#define PROTOCOL_VERSION        1


#define OEM_ID_ICP      0x941c
#define OEM_ID_INTEL    0x8000


#define GDT_ISA         0x01                    
#define GDT_EISA        0x02                    
#define GDT_PCI         0x03                    
#define GDT_PCINEW      0x04                    
#define GDT_PCIMPR      0x05                    

#define GDT3_ID         0x0130941c              
#define GDT3A_ID        0x0230941c              
#define GDT3B_ID        0x0330941c              

#define GDT2_ID         0x0120941c              



#ifndef PCI_VENDOR_ID_VORTEX
#define PCI_VENDOR_ID_VORTEX            0x1119  
#endif
#ifndef PCI_VENDOR_ID_INTEL
#define PCI_VENDOR_ID_INTEL             0x8086  
#endif

#ifndef PCI_DEVICE_ID_VORTEX_GDT60x0

#define PCI_DEVICE_ID_VORTEX_GDT60x0    0       
#define PCI_DEVICE_ID_VORTEX_GDT6000B   1       

#define PCI_DEVICE_ID_VORTEX_GDT6x10    2       
#define PCI_DEVICE_ID_VORTEX_GDT6x20    3       
#define PCI_DEVICE_ID_VORTEX_GDT6530    4       
#define PCI_DEVICE_ID_VORTEX_GDT6550    5       

#define PCI_DEVICE_ID_VORTEX_GDT6x17    6       
#define PCI_DEVICE_ID_VORTEX_GDT6x27    7       
#define PCI_DEVICE_ID_VORTEX_GDT6537    8       
#define PCI_DEVICE_ID_VORTEX_GDT6557    9       

#define PCI_DEVICE_ID_VORTEX_GDT6x15    10      
#define PCI_DEVICE_ID_VORTEX_GDT6x25    11      
#define PCI_DEVICE_ID_VORTEX_GDT6535    12      
#define PCI_DEVICE_ID_VORTEX_GDT6555    13      
#endif

#ifndef PCI_DEVICE_ID_VORTEX_GDT6x17RP

#define PCI_DEVICE_ID_VORTEX_GDT6x17RP  0x100   
#define PCI_DEVICE_ID_VORTEX_GDT6x27RP  0x101   
#define PCI_DEVICE_ID_VORTEX_GDT6537RP  0x102   
#define PCI_DEVICE_ID_VORTEX_GDT6557RP  0x103   

#define PCI_DEVICE_ID_VORTEX_GDT6x11RP  0x104   
#define PCI_DEVICE_ID_VORTEX_GDT6x21RP  0x105   
#endif
#ifndef PCI_DEVICE_ID_VORTEX_GDT6x17RD

#define PCI_DEVICE_ID_VORTEX_GDT6x17RD  0x110   
#define PCI_DEVICE_ID_VORTEX_GDT6x27RD  0x111   
#define PCI_DEVICE_ID_VORTEX_GDT6537RD  0x112   
#define PCI_DEVICE_ID_VORTEX_GDT6557RD  0x113   

#define PCI_DEVICE_ID_VORTEX_GDT6x11RD  0x114   
#define PCI_DEVICE_ID_VORTEX_GDT6x21RD  0x115   

#define PCI_DEVICE_ID_VORTEX_GDT6x18RD  0x118   
#define PCI_DEVICE_ID_VORTEX_GDT6x28RD  0x119   
#define PCI_DEVICE_ID_VORTEX_GDT6x38RD  0x11A   
#define PCI_DEVICE_ID_VORTEX_GDT6x58RD  0x11B   

#define PCI_DEVICE_ID_VORTEX_GDT7x18RN  0x168   
#define PCI_DEVICE_ID_VORTEX_GDT7x28RN  0x169   
#define PCI_DEVICE_ID_VORTEX_GDT7x38RN  0x16A   
#define PCI_DEVICE_ID_VORTEX_GDT7x58RN  0x16B   
#endif

#ifndef PCI_DEVICE_ID_VORTEX_GDT6x19RD

#define PCI_DEVICE_ID_VORTEX_GDT6x19RD  0x210   
#define PCI_DEVICE_ID_VORTEX_GDT6x29RD  0x211   

#define PCI_DEVICE_ID_VORTEX_GDT7x19RN  0x260   
#define PCI_DEVICE_ID_VORTEX_GDT7x29RN  0x261   
#endif

#ifndef PCI_DEVICE_ID_VORTEX_GDTMAXRP

#define PCI_DEVICE_ID_VORTEX_GDTMAXRP   0x2ff   
#endif

#ifndef PCI_DEVICE_ID_VORTEX_GDTNEWRX

#define PCI_DEVICE_ID_VORTEX_GDTNEWRX   0x300
#endif

#ifndef PCI_DEVICE_ID_VORTEX_GDTNEWRX2

#define PCI_DEVICE_ID_VORTEX_GDTNEWRX2  0x301
#endif        

#ifndef PCI_DEVICE_ID_INTEL_SRC

#define PCI_DEVICE_ID_INTEL_SRC         0x600
#endif

#ifndef PCI_DEVICE_ID_INTEL_SRC_XSCALE

#define PCI_DEVICE_ID_INTEL_SRC_XSCALE  0x601
#endif


#define GDTH_SCRATCH    PAGE_SIZE               
#define GDTH_MAXCMDS    120
#define GDTH_MAXC_P_L   16                      
#define GDTH_MAX_RAW    2                       
#define MAXOFFSETS      128
#define MAXHA           16
#define MAXID           127
#define MAXLUN          8
#define MAXBUS          6
#define MAX_EVENTS      100                     
#define MAX_RES_ARGS    40                      
#define MAXCYLS         1024
#define HEADS           64
#define SECS            32                      
#define MEDHEADS        127
#define MEDSECS         63                      
#define BIGHEADS        255
#define BIGSECS         63                      


#define UNUSED_CMND     ((Scsi_Cmnd *)-1)
#define INTERNAL_CMND   ((Scsi_Cmnd *)-2)
#define SCREEN_CMND     ((Scsi_Cmnd *)-3)
#define SPECIAL_SCP(p)  (p==UNUSED_CMND || p==INTERNAL_CMND || p==SCREEN_CMND)


#define SCSIRAWSERVICE  3
#define CACHESERVICE    9
#define SCREENSERVICE   11


#define MSG_INV_HANDLE  -1                      
#define MSGLEN          16                      
#define MSG_SIZE        34                      
#define MSG_REQUEST     0                       


#define SECTOR_SIZE     0x200                   


#define DPMEM_MAGIC     0xC0FFEE11
#define IC_HEADER_BYTES 48
#define IC_QUEUE_BYTES  4
#define DPMEM_COMMAND_OFFSET    IC_HEADER_BYTES+IC_QUEUE_BYTES*MAXOFFSETS


#define CLUSTER_DRIVE         1
#define CLUSTER_MOUNTED       2
#define CLUSTER_RESERVED      4
#define CLUSTER_RESERVE_STATE (CLUSTER_DRIVE|CLUSTER_MOUNTED|CLUSTER_RESERVED)


#define GDT_INIT        0                       
#define GDT_READ        1                       
#define GDT_WRITE       2                       
#define GDT_INFO        3                       
#define GDT_FLUSH       4                       
#define GDT_IOCTL       5                       
#define GDT_DEVTYPE     9                       
#define GDT_MOUNT       10                      
#define GDT_UNMOUNT     11                      
#define GDT_SET_FEAT    12                      
#define GDT_GET_FEAT    13                      
#define GDT_WRITE_THR   16                      
#define GDT_READ_THR    17                      
#define GDT_EXT_INFO    18                      
#define GDT_RESET       19                      
#define GDT_RESERVE_DRV 20                      
#define GDT_RELEASE_DRV 21                      
#define GDT_CLUST_INFO  22                      
#define GDT_RW_ATTRIBS  23                      
#define GDT_CLUST_RESET 24                      
#define GDT_FREEZE_IO   25                      
#define GDT_UNFREEZE_IO 26                      
#define GDT_X_INIT_HOST 29                      
#define GDT_X_INFO      30                      


#define GDT_RESERVE     14                      
#define GDT_RELEASE     15                      
#define GDT_RESERVE_ALL 16                      
#define GDT_RELEASE_ALL 17                      
#define GDT_RESET_BUS   18                      
#define GDT_SCAN_START  19                      
#define GDT_SCAN_END    20                        
#define GDT_X_INIT_RAW  21                      


#define GDT_REALTIME    3                       
#define GDT_X_INIT_SCR  4                       


#define SCSI_DR_INFO    0x00                                       
#define SCSI_CHAN_CNT   0x05                       
#define SCSI_DR_LIST    0x06                    
#define SCSI_DEF_CNT    0x15                    
#define DSK_STATISTICS  0x4b                    
#define IOCHAN_DESC     0x5d                    
#define IOCHAN_RAW_DESC 0x5e                    
#define L_CTRL_PATTERN  0x20000000L             
#define ARRAY_INFO      0x12                    
#define ARRAY_DRV_LIST  0x0f                    
#define ARRAY_DRV_LIST2 0x34                    
#define LA_CTRL_PATTERN 0x10000000L             
#define CACHE_DRV_CNT   0x01                    
#define CACHE_DRV_LIST  0x02                    
#define CACHE_INFO      0x04                    
#define CACHE_CONFIG    0x05                    
#define CACHE_DRV_INFO  0x07                    
#define BOARD_FEATURES  0x15                    
#define BOARD_INFO      0x28                    
#define SET_PERF_MODES  0x82                    
#define GET_PERF_MODES  0x83                    
#define CACHE_READ_OEM_STRING_RECORD 0x84        
#define HOST_GET        0x10001L                
#define IO_CHANNEL      0x00020000L             
#define INVALID_CHANNEL 0x0000ffffL             


#define S_OK            1                       
#define S_GENERR        6                       
#define S_BSY           7                       
#define S_CACHE_UNKNOWN 12                      
#define S_RAW_SCSI      12                      
#define S_RAW_ILL       0xff                    
#define S_NOFUNC        -2                      
#define S_CACHE_RESERV  -24                        


#define INIT_RETRIES    100000                  
#define INIT_TIMEOUT    100000                  
#define POLL_TIMEOUT    10000                   


#define DEFAULT_PRI     0x20
#define IOCTL_PRI       0x10
#define HIGH_PRI        0x08


#define GDTH_DATA_IN    0x01000000L             
#define GDTH_DATA_OUT   0x00000000L             


#define ID0REG          0x0c80                  
#define EINTENABREG     0x0c89                  
#define SEMA0REG        0x0c8a                  
#define SEMA1REG        0x0c8b                  
#define LDOORREG        0x0c8d                  
#define EDENABREG       0x0c8e                  
#define EDOORREG        0x0c8f                  
#define MAILBOXREG      0x0c90                  
#define EISAREG         0x0cc0                  


#define LINUX_OS        8                       
#define SECS32          0x1f                    
#define BIOS_ID_OFFS    0x10                    
#define LOCALBOARD      0                       
#define ASYNCINDEX      0                       
#define SPEZINDEX       1                       
#define COALINDEX       (GDTH_MAXCMDS + 2)


#define SCATTER_GATHER  1                       
#define GDT_WR_THROUGH  0x100                   
#define GDT_64BIT       0x200                   

#include "gdth_ioctl.h"


typedef struct {                               
    ulong32     msg_handle;                     
    ulong32     msg_len;                        
    ulong32     msg_alen;                       
    unchar      msg_answer;                     
    unchar      msg_ext;                        
    unchar      msg_reserved[2];
    char        msg_text[MSGLEN+2];             
} PACKED gdth_msg_str;





typedef struct {
    ulong32     status;
    ulong32     ext_status;
    ulong32     info0;
    ulong32     info1;
} PACKED gdth_coal_status;


typedef struct {
    ulong32     version;            
    ulong32     st_mode;            
    ulong32     st_buff_addr1;      
    ulong32     st_buff_u_addr1;    
    ulong32     st_buff_indx1;      
    ulong32     st_buff_addr2;      
    ulong32     st_buff_u_addr2;    
    ulong32     st_buff_indx2;      
    ulong32     st_buff_size;       
    ulong32     cmd_mode;            
    ulong32     cmd_buff_addr1;        
    ulong32     cmd_buff_u_addr1;   
    ulong32     cmd_buff_indx1;     
    ulong32     cmd_buff_addr2;        
    ulong32     cmd_buff_u_addr2;   
    ulong32     cmd_buff_indx2;     
    ulong32     cmd_buff_size;      
    ulong32     reserved1;
    ulong32     reserved2;
} PACKED gdth_perf_modes;


typedef struct {
    unchar      vendor[8];                      
    unchar      product[16];                    
    unchar      revision[4];                    
    ulong32     sy_rate;                        
    ulong32     sy_max_rate;                    
    ulong32     no_ldrive;                      
    ulong32     blkcnt;                         
    ushort      blksize;                        
    unchar      available;                      
    unchar      init;                           
    unchar      devtype;                        
    unchar      rm_medium;                      
    unchar      wp_medium;                      
    unchar      ansi;                           
    unchar      protocol;                       
    unchar      sync;                           
    unchar      disc;                           
    unchar      queueing;                       
    unchar      cached;                         
    unchar      target_id;                      
    unchar      lun;                            
    unchar      orphan;                         
    ulong32     last_error;                     
    ulong32     last_result;                    
    ulong32     check_errors;                   
    unchar      percent;                        
    unchar      last_check;                     
    unchar      res[2];
    ulong32     flags;                          
    unchar      multi_bus;                      
    unchar      mb_status;                      
    unchar      res2[2];
    unchar      mb_alt_status;                  
    unchar      mb_alt_bid;                     
    unchar      mb_alt_tid;                     
    unchar      res3;
    unchar      fc_flag;                        
    unchar      res4;
    ushort      fc_frame_size;                  
    char        wwn[8];                         
} PACKED gdth_diskinfo_str;


typedef struct {
    ulong32     channel_no;                     
    ulong32     drive_cnt;                      
    unchar      siop_id;                        
    unchar      siop_state;                      
} PACKED gdth_getch_str;


typedef struct {
    ulong32     sc_no;                          
    ulong32     sc_cnt;                         
    ulong32     sc_list[MAXID];                 
} PACKED gdth_drlist_str;


typedef struct {
    unchar      sddc_type;                      
    unchar      sddc_format;                    
    unchar      sddc_len;                       
    unchar      sddc_res;
    ulong32     sddc_cnt;                       
} PACKED gdth_defcnt_str;


typedef struct {
    ulong32     bid;                            
    ulong32     first;                          
    ulong32     entries;                        
    ulong32     count;                          
    ulong32     mon_time;                       
    struct {
        unchar  tid;                            
        unchar  lun;                            
        unchar  res[2];
        ulong32 blk_size;                       
        ulong32 rd_count;                       
        ulong32 wr_count;                       
        ulong32 rd_blk_count;                   
        ulong32 wr_blk_count;                   
        ulong32 retries;                        
        ulong32 reassigns;                      
    } PACKED list[1];
} PACKED gdth_dskstat_str;


typedef struct {
    ulong32     version;                        
    unchar      list_entries;                   
    unchar      first_chan;                     
    unchar      last_chan;                      
    unchar      chan_count;                     
    ulong32     list_offset;                    
} PACKED gdth_iochan_header;


typedef struct {
    gdth_iochan_header  hdr;
    struct {
        ulong32         address;                
        unchar          type;                   
        unchar          local_no;               
        ushort          features;               
    } PACKED list[MAXBUS];
} PACKED gdth_iochan_str;


typedef struct {
    gdth_iochan_header  hdr;
    struct {
        unchar      proc_id;                    
        unchar      proc_defect;                
        unchar      reserved[2];
    } PACKED list[MAXBUS];
} PACKED gdth_raw_iochan_str;


typedef struct {
    ulong32     al_controller;                  
    unchar      al_cache_drive;                 
    unchar      al_status;                      
    unchar      al_res[2];     
} PACKED gdth_arraycomp_str;


typedef struct {
    unchar      ai_type;                        
    unchar      ai_cache_drive_cnt;             
    unchar      ai_state;                       
    unchar      ai_master_cd;                   
    ulong32     ai_master_controller;           
    ulong32     ai_size;                        
    ulong32     ai_striping_size;               
    ulong32     ai_secsize;                     
    ulong32     ai_err_info;                    
    unchar      ai_name[8];                     
    unchar      ai_controller_cnt;              
    unchar      ai_removable;                   
    unchar      ai_write_protected;             
    unchar      ai_devtype;                     
    gdth_arraycomp_str  ai_drives[35];          
    unchar      ai_drive_entries;               
    unchar      ai_protected;                   
    unchar      ai_verify_state;                
    unchar      ai_ext_state;                   
    unchar      ai_expand_state;                
    unchar      ai_reserved[3];
} PACKED gdth_arrayinf_str;


typedef struct {
    ulong32     controller_no;                  
    unchar      cd_handle;                      
    unchar      is_arrayd;                      
    unchar      is_master;                      
    unchar      is_parity;                      
    unchar      is_hotfix;                      
    unchar      res[3];
} PACKED gdth_alist_str;

typedef struct {
    ulong32     entries_avail;                  
    ulong32     entries_init;                   
    ulong32     first_entry;                    
    ulong32     list_offset;                    
    gdth_alist_str list[1];                     
} PACKED gdth_arcdl_str;


typedef struct {
    ulong32     version;                        
    ushort      state;                          
    ushort      strategy;                       
    ushort      write_back;                     
    ushort      block_size;                     
} PACKED gdth_cpar_str;

typedef struct {
    ulong32     csize;                          
    ulong32     read_cnt;                       
    ulong32     write_cnt;
    ulong32     tr_hits;                        
    ulong32     sec_hits;
    ulong32     sec_miss;                       
} PACKED gdth_cstat_str;

typedef struct {
    gdth_cpar_str   cpar;
    gdth_cstat_str  cstat;
} PACKED gdth_cinfo_str;


typedef struct {
    unchar      cd_name[8];                     
    ulong32     cd_devtype;                     
    ulong32     cd_ldcnt;                       
    ulong32     cd_last_error;                  
    unchar      cd_initialized;                 
    unchar      cd_removable;                   
    unchar      cd_write_protected;             
    unchar      cd_flags;                       
    ulong32     ld_blkcnt;                      
    ulong32     ld_blksize;                     
    ulong32     ld_dcnt;                        
    ulong32     ld_slave;                       
    ulong32     ld_dtype;                       
    ulong32     ld_last_error;                  
    unchar      ld_name[8];                     
    unchar      ld_error;                       
} PACKED gdth_cdrinfo_str;


typedef struct {
    ulong32     ctl_version;
    ulong32     file_major_version;
    ulong32     file_minor_version;
    ulong32     buffer_size;
    ulong32     cpy_count;
    ulong32     ext_error;
    ulong32     oem_id;
    ulong32     board_id;
} PACKED gdth_oem_str_params;

typedef struct {
    unchar      product_0_1_name[16];
    unchar      product_4_5_name[16];
    unchar      product_cluster_name[16];
    unchar      product_reserved[16];
    unchar      scsi_cluster_target_vendor_id[16];
    unchar      cluster_raid_fw_name[16];
    unchar      oem_brand_name[16];
    unchar      oem_raid_type[16];
    unchar      bios_type[13];
    unchar      bios_title[50];
    unchar      oem_company_name[37];
    ulong32     pci_id_1;
    ulong32     pci_id_2;
    unchar      validation_status[80];
    unchar      reserved_1[4];
    unchar      scsi_host_drive_inquiry_vendor_id[16];
    unchar      library_file_template[16];
    unchar      reserved_2[16];
    unchar      tool_name_1[32];
    unchar      tool_name_2[32];
    unchar      tool_name_3[32];
    unchar      oem_contact_1[84];
    unchar      oem_contact_2[84];
    unchar      oem_contact_3[84];
} PACKED gdth_oem_str;

typedef struct {
    gdth_oem_str_params params;
    gdth_oem_str        text;
} PACKED gdth_oem_str_ioctl;


typedef struct {
    unchar      chaining;                       
    unchar      striping;                       
    unchar      mirroring;                      
    unchar      raid;                           
} PACKED gdth_bfeat_str;


typedef struct {
    ulong32     ser_no;                         
    unchar      oem_id[2];                      
    ushort      ep_flags;                       
    ulong32     proc_id;                        
    ulong32     memsize;                        
    unchar      mem_banks;                      
    unchar      chan_type;                      
    unchar      chan_count;                     
    unchar      rdongle_pres;                   
    ulong32     epr_fw_ver;                     
    ulong32     upd_fw_ver;                     
    ulong32     upd_revision;                   
    char        type_string[16];                
    char        raid_string[16];                
    unchar      update_pres;                    
    unchar      xor_pres;                       
    unchar      prom_type;                      
    unchar      prom_count;                     
    ulong32     dup_pres;                       
    ulong32     chan_pres;                      
    ulong32     mem_pres;                       
    unchar      ft_bus_system;                  
    unchar      subtype_valid;                  
    unchar      board_subtype;                  
    unchar      ramparity_pres;                 
} PACKED gdth_binfo_str; 


typedef struct {
    char        name[8];                        
    ulong32     size;                           
    unchar      host_drive;                     
    unchar      log_drive;                      
    unchar      reserved;
    unchar      rw_attribs;                     
    ulong32     start_sec;                      
} PACKED gdth_hentry_str;

typedef struct {
    ulong32     entries;                        
    ulong32     offset;                         
    unchar      secs_p_head;                    
    unchar      heads_p_cyl;                    
    unchar      reserved;
    unchar      clust_drvtype;                  
    ulong32     location;                       
    gdth_hentry_str entry[MAX_HDRIVES];         
} PACKED gdth_hget_str;    





typedef struct {
    unchar              S_Cmd_Indx;             
    unchar volatile     S_Status;               
    ushort              reserved1;
    ulong32             S_Info[4];              
    unchar volatile     Sema0;                  
    unchar              reserved2[3];
    unchar              Cmd_Index;              
    unchar              reserved3[3];
    ushort volatile     Status;                 
    ushort              Service;                
    ulong32             Info[2];                
    struct {
        ushort          offset;                 
        ushort          serv_id;                
    } PACKED comm_queue[MAXOFFSETS];            
    ulong32             bios_reserved[2];
    unchar              gdt_dpr_cmd[1];         
} PACKED gdt_dpr_if;


typedef struct {
    ulong32     magic;                          
    ushort      need_deinit;                    
    unchar      switch_support;                 
    unchar      padding[9];
    unchar      os_used[16];                    
    unchar      unused[28];
    unchar      fw_magic;                       
} PACKED gdt_pci_sram;


typedef struct {
    unchar      os_used[16];                    
    ushort      need_deinit;                    
    unchar      switch_support;                 
    unchar      padding;
} PACKED gdt_eisa_sram;



typedef struct {
    union {
        struct {
            unchar      bios_used[0x3c00-32];   
            ulong32     magic;                  
            ushort      need_deinit;            
            unchar      switch_support;         
            unchar      padding[9];
            unchar      os_used[16];            
        } PACKED dp_sram;
        unchar          bios_area[0x4000];      
    } bu;
    union {
        gdt_dpr_if      ic;                     
        unchar          if_area[0x3000];        
    } u;
    struct {
        unchar          memlock;                
        unchar          event;                  
        unchar          irqen;                  
        unchar          irqdel;                 
        unchar volatile Sema1;                  
        unchar          rq;                     
    } PACKED io;
} PACKED gdt2_dpram_str;


typedef struct {
    union {
        gdt_dpr_if      ic;                     
        unchar          if_area[0xff0-sizeof(gdt_pci_sram)];
    } u;
    gdt_pci_sram        gdt6sr;                 
    struct {
        unchar          unused0[1];
        unchar volatile Sema1;                  
        unchar          unused1[3];
        unchar          irqen;                  
        unchar          unused2[2];
        unchar          event;                  
        unchar          unused3[3];
        unchar          irqdel;                 
        unchar          unused4[3];
    } PACKED io;
} PACKED gdt6_dpram_str;


typedef struct {
    unchar              cfg_reg;        
    unchar              unused1[0x3f];
    unchar volatile     sema0_reg;              
    unchar volatile     sema1_reg;              
    unchar              unused2[2];
    ushort volatile     status;                 
    ushort              service;                
    ulong32             info[2];                
    unchar              unused3[0x10];
    unchar              ldoor_reg;              
    unchar              unused4[3];
    unchar volatile     edoor_reg;              
    unchar              unused5[3];
    unchar              control0;               
    unchar              control1;               
    unchar              unused6[0x16];
} PACKED gdt6c_plx_regs;


typedef struct {
    union {
        gdt_dpr_if      ic;                     
        unchar          if_area[0x4000-sizeof(gdt_pci_sram)];
    } u;
    gdt_pci_sram        gdt6sr;                 
} PACKED gdt6c_dpram_str;


typedef struct {
    unchar              unused1[16];
    unchar volatile     sema0_reg;              
    unchar              unused2;
    unchar volatile     sema1_reg;              
    unchar              unused3;
    ushort volatile     status;                 
    ushort              service;                
    ulong32             info[2];                
    unchar              ldoor_reg;              
    unchar              unused4[11];
    unchar volatile     edoor_reg;              
    unchar              unused5[7];
    unchar              edoor_en_reg;           
    unchar              unused6[27];
    ulong32             unused7[939];         
    ulong32             severity;       
    char                evt_str[256];           
} PACKED gdt6m_i960_regs;


typedef struct {
    gdt6m_i960_regs     i960r;                  
    union {
        gdt_dpr_if      ic;                     
        unchar          if_area[0x3000-sizeof(gdt_pci_sram)];
    } u;
    gdt_pci_sram        gdt6sr;                 
} PACKED gdt6m_dpram_str;



typedef struct {
    struct pci_dev      *pdev;
    ulong               dpmem;                  
    ulong               io;                     
} gdth_pci_str;



typedef struct {
    struct Scsi_Host    *shost;
    struct list_head    list;
    ushort      	hanum;
    ushort              oem_id;                 
    ushort              type;                   
    ulong32             stype;                  
    ushort              fw_vers;                
    ushort              cache_feat;             
    ushort              raw_feat;               
    ushort              screen_feat;            
    ushort              bmic;                   
    void __iomem        *brd;                   
    ulong32             brd_phys;               
    gdt6c_plx_regs      *plx;                   
    gdth_cmd_str        cmdext;
    gdth_cmd_str        *pccb;                  
    ulong32             ccb_phys;               
#ifdef INT_COAL
    gdth_coal_status    *coal_stat;             
    ulong64             coal_stat_phys;         
#endif
    char                *pscratch;              
    ulong64             scratch_phys;           
    unchar              scratch_busy;           
    unchar              dma64_support;          
    gdth_msg_str        *pmsg;                  
    ulong64             msg_phys;               
    unchar              scan_mode;              
    unchar              irq;                    
    unchar              drq;                    
    ushort              status;                 
    ushort              service;                
    ulong32             info;
    ulong32             info2;                  
    Scsi_Cmnd           *req_first;             
    struct {
        unchar          present;                
        unchar          is_logdrv;              
        unchar          is_arraydrv;            
        unchar          is_master;              
        unchar          is_parity;              
        unchar          is_hotfix;              
        unchar          master_no;              
        unchar          lock;                   
        unchar          heads;                  
        unchar          secs;
        ushort          devtype;                
        ulong64         size;                   
        unchar          ldr_no;                 
        unchar          rw_attribs;             
        unchar          cluster_type;           
        unchar          media_changed;          
        ulong32         start_sec;              
    } hdr[MAX_LDRIVES];                         
    struct {
        unchar          lock;                   
        unchar          pdev_cnt;               
        unchar          local_no;               
        unchar          io_cnt[MAXID];          
        ulong32         address;                
        ulong32         id_list[MAXID];         
    } raw[MAXBUS];                              
    struct {
        Scsi_Cmnd       *cmnd;                  
        ushort          service;                
    } cmd_tab[GDTH_MAXCMDS];                    
    struct gdth_cmndinfo {                      
        int index;
        int internal_command;                   
        gdth_cmd_str *internal_cmd_str;         
        dma_addr_t sense_paddr;                 
        unchar priority;
	int timeout_count;			
        volatile int wait_for_completion;
        ushort status;
        ulong32 info;
        enum dma_data_direction dma_dir;
        int phase;                              
        int OpCode;
    } cmndinfo[GDTH_MAXCMDS];                   
    unchar              bus_cnt;                
    unchar              tid_cnt;                
    unchar              bus_id[MAXBUS];         
    unchar              virt_bus;               
    unchar              more_proc;              
    ushort              cmd_cnt;                
    ushort              cmd_len;                
    ushort              cmd_offs_dpmem;         
    ushort              ic_all_size;            
    gdth_cpar_str       cpar;                   
    gdth_bfeat_str      bfeat;                  
    gdth_binfo_str      binfo;                  
    gdth_evt_data       dvr;                    
    spinlock_t          smp_lock;
    struct pci_dev      *pdev;
    char                oem_name[8];
#ifdef GDTH_DMA_STATISTICS
    ulong               dma32_cnt, dma64_cnt;   
#endif
    struct scsi_device         *sdev;
} gdth_ha_str;

static inline struct gdth_cmndinfo *gdth_cmnd_priv(struct scsi_cmnd* cmd)
{
	return (struct gdth_cmndinfo *)cmd->host_scribble;
}


typedef struct {
    unchar      type_qual;
    unchar      modif_rmb;
    unchar      version;
    unchar      resp_aenc;
    unchar      add_length;
    unchar      reserved1;
    unchar      reserved2;
    unchar      misc;
    unchar      vendor[8];
    unchar      product[16];
    unchar      revision[4];
} PACKED gdth_inq_data;


typedef struct {
    ulong32     last_block_no;
    ulong32     block_length;
} PACKED gdth_rdcap_data;


typedef struct {
    ulong64     last_block_no;
    ulong32     block_length;
} PACKED gdth_rdcap16_data;


typedef struct {
    unchar      errorcode;
    unchar      segno;
    unchar      key;
    ulong32     info;
    unchar      add_length;
    ulong32     cmd_info;
    unchar      adsc;
    unchar      adsq;
    unchar      fruc;
    unchar      key_spec[3];
} PACKED gdth_sense_data;


typedef struct {
    struct {
        unchar  data_length;
        unchar  med_type;
        unchar  dev_par;
        unchar  bd_length;
    } PACKED hd;
    struct {
        unchar  dens_code;
        unchar  block_count[3];
        unchar  reserved;
        unchar  block_length[3];
    } PACKED bd;
} PACKED gdth_modep_data;


typedef struct {
    ulong       b[10];                          
} PACKED gdth_stackframe;




int gdth_proc_info(struct Scsi_Host *, char *,char **,off_t,int,int);

#endif
