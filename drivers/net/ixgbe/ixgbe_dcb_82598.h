

#ifndef _DCB_82598_CONFIG_H_
#define _DCB_82598_CONFIG_H_



#define IXGBE_DPMCS_MTSOS_SHIFT 16
#define IXGBE_DPMCS_TDPAC       0x00000001 
#define IXGBE_DPMCS_TRM         0x00000010 
#define IXGBE_DPMCS_ARBDIS      0x00000040 
#define IXGBE_DPMCS_TSOEF       0x00080000 

#define IXGBE_RUPPBMR_MQA       0x80000000 

#define IXGBE_RT2CR_MCL_SHIFT   12 
#define IXGBE_RT2CR_LSP         0x80000000 

#define IXGBE_RDRXCTL_MPBEN     0x00000010 
#define IXGBE_RDRXCTL_MCEN      0x00000040 

#define IXGBE_TDTQ2TCCR_MCL_SHIFT   12
#define IXGBE_TDTQ2TCCR_BWG_SHIFT   9
#define IXGBE_TDTQ2TCCR_GSP     0x40000000
#define IXGBE_TDTQ2TCCR_LSP     0x80000000

#define IXGBE_TDPT2TCCR_MCL_SHIFT   12
#define IXGBE_TDPT2TCCR_BWG_SHIFT   9
#define IXGBE_TDPT2TCCR_GSP     0x40000000
#define IXGBE_TDPT2TCCR_LSP     0x80000000

#define IXGBE_PDPMCS_TPPAC      0x00000020 
#define IXGBE_PDPMCS_ARBDIS     0x00000040 
#define IXGBE_PDPMCS_TRM        0x00000100 

#define IXGBE_DTXCTL_ENDBUBD    0x00000004 

#define IXGBE_TXPBSIZE_40KB     0x0000A000 
#define IXGBE_RXPBSIZE_48KB     0x0000C000 
#define IXGBE_RXPBSIZE_64KB     0x00010000 
#define IXGBE_RXPBSIZE_80KB     0x00014000 

#define IXGBE_RDRXCTL_RDMTS_1_2 0x00000000




s32 ixgbe_dcb_config_pfc_82598(struct ixgbe_hw *, struct ixgbe_dcb_config *);
s32 ixgbe_dcb_get_pfc_stats_82598(struct ixgbe_hw *, struct ixgbe_hw_stats *,
                                  u8);


s32 ixgbe_dcb_config_tc_stats_82598(struct ixgbe_hw *);
s32 ixgbe_dcb_get_tc_stats_82598(struct ixgbe_hw *, struct ixgbe_hw_stats *,
                                 u8);


s32 ixgbe_dcb_config_tx_desc_arbiter_82598(struct ixgbe_hw *,
                                           struct ixgbe_dcb_config *);
s32 ixgbe_dcb_config_tx_data_arbiter_82598(struct ixgbe_hw *,
                                           struct ixgbe_dcb_config *);
s32 ixgbe_dcb_config_rx_arbiter_82598(struct ixgbe_hw *,
                                      struct ixgbe_dcb_config *);


s32 ixgbe_dcb_hw_config_82598(struct ixgbe_hw *, struct ixgbe_dcb_config *);

#endif 
