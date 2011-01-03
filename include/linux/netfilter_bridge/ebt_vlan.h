#ifndef __LINUX_BRIDGE_EBT_VLAN_H
#define __LINUX_BRIDGE_EBT_VLAN_H

#define EBT_VLAN_ID	0x01
#define EBT_VLAN_PRIO	0x02
#define EBT_VLAN_ENCAP	0x04
#define EBT_VLAN_MASK (EBT_VLAN_ID | EBT_VLAN_PRIO | EBT_VLAN_ENCAP)
#define EBT_VLAN_MATCH "vlan"

struct ebt_vlan_info {
	uint16_t id;		
	uint8_t prio;		
	__be16 encap;		
	uint8_t bitmask;		
	uint8_t invflags;		
};

#endif
