

#ifndef __BFA_DEFS_IM_COMMON_H__
#define __BFA_DEFS_IM_COMMON_H__

#define	BFA_ADAPTER_NAME_LEN	256
#define BFA_ADAPTER_GUID_LEN    256
#define RESERVED_VLAN_NAME      L"PORT VLAN"
#define PASSTHRU_VLAN_NAME      L"PASSTHRU VLAN"

	u64	tx_pkt_cnt;
	u64	rx_pkt_cnt;
	u32	duration;
	u8		status;
} bfa_im_stats_t, *pbfa_im_stats_t;

#endif 
