

#ifndef _E1000_82575_H_
#define _E1000_82575_H_

extern void igb_shutdown_serdes_link_82575(struct e1000_hw *hw);
extern void igb_rx_fifo_flush_82575(struct e1000_hw *hw);

#define ID_LED_DEFAULT_82575_SERDES ((ID_LED_DEF1_DEF2 << 12) | \
                                     (ID_LED_DEF1_DEF2 <<  8) | \
                                     (ID_LED_DEF1_DEF2 <<  4) | \
                                     (ID_LED_OFF1_ON2))

#define E1000_RAR_ENTRIES_82575   16
#define E1000_RAR_ENTRIES_82576   24


#define E1000_SRRCTL_BSIZEPKT_SHIFT                     10 
#define E1000_SRRCTL_BSIZEHDRSIZE_SHIFT                 2  
#define E1000_SRRCTL_DESCTYPE_ADV_ONEBUF                0x02000000
#define E1000_SRRCTL_DESCTYPE_HDR_SPLIT_ALWAYS          0x0A000000
#define E1000_SRRCTL_DROP_EN                            0x80000000

#define E1000_MRQC_ENABLE_RSS_4Q            0x00000002
#define E1000_MRQC_ENABLE_VMDQ              0x00000003
#define E1000_MRQC_ENABLE_VMDQ_RSS_2Q       0x00000005
#define E1000_MRQC_RSS_FIELD_IPV4_UDP       0x00400000
#define E1000_MRQC_RSS_FIELD_IPV6_UDP       0x00800000
#define E1000_MRQC_RSS_FIELD_IPV6_UDP_EX    0x01000000

#define E1000_EICR_TX_QUEUE ( \
    E1000_EICR_TX_QUEUE0 |    \
    E1000_EICR_TX_QUEUE1 |    \
    E1000_EICR_TX_QUEUE2 |    \
    E1000_EICR_TX_QUEUE3)

#define E1000_EICR_RX_QUEUE ( \
    E1000_EICR_RX_QUEUE0 |    \
    E1000_EICR_RX_QUEUE1 |    \
    E1000_EICR_RX_QUEUE2 |    \
    E1000_EICR_RX_QUEUE3)




union e1000_adv_rx_desc {
	struct {
		__le64 pkt_addr;             
		__le64 hdr_addr;             
	} read;
	struct {
		struct {
			struct {
				__le16 pkt_info;   
				__le16 hdr_info;   
			} lo_dword;
			union {
				__le32 rss;          
				struct {
					__le16 ip_id;    
					__le16 csum;     
				} csum_ip;
			} hi_dword;
		} lower;
		struct {
			__le32 status_error;     
			__le16 length;           
			__le16 vlan;             
		} upper;
	} wb;  
};

#define E1000_RXDADV_HDRBUFLEN_MASK      0x7FE0
#define E1000_RXDADV_HDRBUFLEN_SHIFT     5


union e1000_adv_tx_desc {
	struct {
		__le64 buffer_addr;    
		__le32 cmd_type_len;
		__le32 olinfo_status;
	} read;
	struct {
		__le64 rsvd;       
		__le32 nxtseq_seed;
		__le32 status;
	} wb;
};


#define E1000_ADVTXD_MAC_TSTAMP   0x00080000 
#define E1000_ADVTXD_DTYP_CTXT    0x00200000 
#define E1000_ADVTXD_DTYP_DATA    0x00300000 
#define E1000_ADVTXD_DCMD_IFCS    0x02000000 
#define E1000_ADVTXD_DCMD_DEXT    0x20000000 
#define E1000_ADVTXD_DCMD_VLE     0x40000000 
#define E1000_ADVTXD_DCMD_TSE     0x80000000 
#define E1000_ADVTXD_PAYLEN_SHIFT    14 


struct e1000_adv_tx_context_desc {
	__le32 vlan_macip_lens;
	__le32 seqnum_seed;
	__le32 type_tucmd_mlhl;
	__le32 mss_l4len_idx;
};

#define E1000_ADVTXD_MACLEN_SHIFT    9  
#define E1000_ADVTXD_TUCMD_IPV4    0x00000400  
#define E1000_ADVTXD_TUCMD_L4T_TCP 0x00000800  
#define E1000_ADVTXD_TUCMD_L4T_SCTP 0x00001000 

#define E1000_ADVTXD_L4LEN_SHIFT     8  
#define E1000_ADVTXD_MSS_SHIFT      16  




#define E1000_TXDCTL_QUEUE_ENABLE  0x02000000 



#define E1000_RXDCTL_QUEUE_ENABLE  0x02000000 


#define E1000_DCA_CTRL_DCA_MODE_DISABLE 0x01 
#define E1000_DCA_CTRL_DCA_MODE_CB2     0x02 

#define E1000_DCA_RXCTRL_CPUID_MASK 0x0000001F 
#define E1000_DCA_RXCTRL_DESC_DCA_EN (1 << 5) 
#define E1000_DCA_RXCTRL_HEAD_DCA_EN (1 << 6) 
#define E1000_DCA_RXCTRL_DATA_DCA_EN (1 << 7) 

#define E1000_DCA_TXCTRL_CPUID_MASK 0x0000001F 
#define E1000_DCA_TXCTRL_DESC_DCA_EN (1 << 5) 
#define E1000_DCA_TXCTRL_TX_WB_RO_EN (1 << 11) 


#define E1000_DCA_TXCTRL_CPUID_MASK_82576 0xFF000000 
#define E1000_DCA_RXCTRL_CPUID_MASK_82576 0xFF000000 
#define E1000_DCA_TXCTRL_CPUID_SHIFT 24 
#define E1000_DCA_RXCTRL_CPUID_SHIFT 24 

#define MAX_NUM_VFS                   8

#define E1000_DTXSWC_VMDQ_LOOPBACK_EN (1 << 31)  


#define E1000_VT_CTL_DEFAULT_POOL_SHIFT 7
#define E1000_VT_CTL_DEFAULT_POOL_MASK  (0x7 << E1000_VT_CTL_DEFAULT_POOL_SHIFT)


#define E1000_VT_CTL_IGNORE_MAC         (1 << 28)
#define E1000_VT_CTL_DISABLE_DEF_POOL   (1 << 29)
#define E1000_VT_CTL_VM_REPL_EN         (1 << 30)


#define E1000_VMOLR_RLPML_MASK 0x00003FFF 
#define E1000_VMOLR_LPE        0x00010000 
#define E1000_VMOLR_RSSE       0x00020000 
#define E1000_VMOLR_AUPE       0x01000000 
#define E1000_VMOLR_ROMPE      0x02000000 
#define E1000_VMOLR_ROPE       0x04000000 
#define E1000_VMOLR_BAM        0x08000000 
#define E1000_VMOLR_MPME       0x10000000 
#define E1000_VMOLR_STRVLAN    0x40000000 
#define E1000_VMOLR_STRCRC     0x80000000 

#define E1000_VLVF_ARRAY_SIZE     32
#define E1000_VLVF_VLANID_MASK    0x00000FFF
#define E1000_VLVF_POOLSEL_SHIFT  12
#define E1000_VLVF_POOLSEL_MASK   (0xFF << E1000_VLVF_POOLSEL_SHIFT)
#define E1000_VLVF_LVLAN          0x00100000
#define E1000_VLVF_VLANID_ENABLE  0x80000000

#define E1000_IOVCTL 0x05BBC
#define E1000_IOVCTL_REUSE_VFQ 0x00000001

#define ALL_QUEUES   0xFFFF

void igb_vmdq_set_loopback_pf(struct e1000_hw *, bool);
void igb_vmdq_set_replication_pf(struct e1000_hw *, bool);

#endif
