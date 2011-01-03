

#ifndef _E1000_DEFINES_H_
#define _E1000_DEFINES_H_


#define REQ_TX_DESCRIPTOR_MULTIPLE  8
#define REQ_RX_DESCRIPTOR_MULTIPLE  8


#define E1000_IVAR_VALID        0x80


#define E1000_RXD_STAT_DD       0x01    
#define E1000_RXD_STAT_EOP      0x02    
#define E1000_RXD_STAT_IXSM     0x04    
#define E1000_RXD_STAT_VP       0x08    
#define E1000_RXD_STAT_UDPCS    0x10    
#define E1000_RXD_STAT_TCPCS    0x20    
#define E1000_RXD_STAT_IPCS     0x40    
#define E1000_RXD_ERR_SE        0x02    
#define E1000_RXD_SPC_VLAN_MASK 0x0FFF  

#define E1000_RXDEXT_STATERR_CE    0x01000000
#define E1000_RXDEXT_STATERR_SE    0x02000000
#define E1000_RXDEXT_STATERR_SEQ   0x04000000
#define E1000_RXDEXT_STATERR_CXE   0x10000000
#define E1000_RXDEXT_STATERR_TCPE  0x20000000
#define E1000_RXDEXT_STATERR_IPE   0x40000000
#define E1000_RXDEXT_STATERR_RXE   0x80000000



#define E1000_RXDEXT_ERR_FRAME_ERR_MASK ( \
    E1000_RXDEXT_STATERR_CE  |            \
    E1000_RXDEXT_STATERR_SE  |            \
    E1000_RXDEXT_STATERR_SEQ |            \
    E1000_RXDEXT_STATERR_CXE |            \
    E1000_RXDEXT_STATERR_RXE)


#define E1000_CTRL_RST      0x04000000  


#define E1000_STATUS_FD         0x00000001      
#define E1000_STATUS_LU         0x00000002      
#define E1000_STATUS_TXOFF      0x00000010      
#define E1000_STATUS_SPEED_10   0x00000000      
#define E1000_STATUS_SPEED_100  0x00000040      
#define E1000_STATUS_SPEED_1000 0x00000080      

#define SPEED_10    10
#define SPEED_100   100
#define SPEED_1000  1000
#define HALF_DUPLEX 1
#define FULL_DUPLEX 2


#define E1000_TXD_POPTS_IXSM 0x01       
#define E1000_TXD_POPTS_TXSM 0x02       
#define E1000_TXD_CMD_DEXT   0x20000000 
#define E1000_TXD_STAT_DD    0x00000001 

#define MAX_JUMBO_FRAME_SIZE    0x3F00


#define VLAN_TAG_SIZE              4    


#define E1000_SUCCESS      0
#define E1000_ERR_CONFIG   3
#define E1000_ERR_MAC_INIT 5
#define E1000_ERR_MBX      15

#ifndef ETH_ADDR_LEN
#define ETH_ADDR_LEN                 6
#endif


#define E1000_SRRCTL_BSIZEPKT_SHIFT                     10 
#define E1000_SRRCTL_BSIZEHDRSIZE_MASK                  0x00000F00
#define E1000_SRRCTL_BSIZEHDRSIZE_SHIFT                 2  
#define E1000_SRRCTL_DESCTYPE_ADV_ONEBUF                0x02000000
#define E1000_SRRCTL_DESCTYPE_HDR_SPLIT_ALWAYS          0x0A000000
#define E1000_SRRCTL_DESCTYPE_MASK                      0x0E000000
#define E1000_SRRCTL_DROP_EN                            0x80000000

#define E1000_SRRCTL_BSIZEPKT_MASK      0x0000007F
#define E1000_SRRCTL_BSIZEHDR_MASK      0x00003F00


#define E1000_TXDCTL_QUEUE_ENABLE  0x02000000 
#define E1000_RXDCTL_QUEUE_ENABLE  0x02000000 


#define E1000_DCA_TXCTRL_TX_WB_RO_EN (1 << 11) 

#define E1000_VF_INIT_TIMEOUT 200 

#endif 
