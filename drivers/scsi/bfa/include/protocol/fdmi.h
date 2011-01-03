

#ifndef __FDMI_H__
#define __FDMI_H__

#include <protocol/types.h>
#include <protocol/fc.h>
#include <protocol/ct.h>

#pragma pack(1)


#define	FDMI_GRHL		0x0100
#define	FDMI_GHAT		0x0101
#define	FDMI_GRPL		0x0102
#define	FDMI_GPAT		0x0110
#define	FDMI_RHBA		0x0200
#define	FDMI_RHAT		0x0201
#define	FDMI_RPRT		0x0210
#define	FDMI_RPA		0x0211
#define	FDMI_DHBA		0x0300
#define	FDMI_DPRT		0x0310


#define	FDMI_NO_ADDITIONAL_EXP		0x00
#define	FDMI_HBA_ALREADY_REG		0x10
#define	FDMI_HBA_ATTRIB_NOT_REG		0x11
#define	FDMI_HBA_ATTRIB_MULTIPLE	0x12
#define	FDMI_HBA_ATTRIB_LENGTH_INVALID	0x13
#define	FDMI_HBA_ATTRIB_NOT_PRESENT	0x14
#define	FDMI_PORT_ORIG_NOT_IN_LIST	0x15
#define	FDMI_PORT_HBA_NOT_IN_LIST	0x16
#define	FDMI_PORT_ATTRIB_NOT_REG	0x20
#define	FDMI_PORT_NOT_REG		0x21
#define	FDMI_PORT_ATTRIB_MULTIPLE	0x22
#define	FDMI_PORT_ATTRIB_LENGTH_INVALID	0x23
#define	FDMI_PORT_ALREADY_REGISTEREED	0x24


#define	FDMI_TRANS_SPEED_1G		0x00000001
#define	FDMI_TRANS_SPEED_2G		0x00000002
#define	FDMI_TRANS_SPEED_10G		0x00000004
#define	FDMI_TRANS_SPEED_4G		0x00000008
#define	FDMI_TRANS_SPEED_8G		0x00000010
#define	FDMI_TRANS_SPEED_16G		0x00000020
#define	FDMI_TRANS_SPEED_UNKNOWN	0x00008000


enum fdmi_hba_attribute_type {
	FDMI_HBA_ATTRIB_NODENAME = 1,	
	FDMI_HBA_ATTRIB_MANUFACTURER,	
	FDMI_HBA_ATTRIB_SERIALNUM,	
	FDMI_HBA_ATTRIB_MODEL,		
	FDMI_HBA_ATTRIB_MODEL_DESC,	
	FDMI_HBA_ATTRIB_HW_VERSION,	
	FDMI_HBA_ATTRIB_DRIVER_VERSION,	
	FDMI_HBA_ATTRIB_ROM_VERSION,	
	FDMI_HBA_ATTRIB_FW_VERSION,	
	FDMI_HBA_ATTRIB_OS_NAME,	
	FDMI_HBA_ATTRIB_MAX_CT,		

	FDMI_HBA_ATTRIB_MAX_TYPE
};


enum fdmi_port_attribute_type {
	FDMI_PORT_ATTRIB_FC4_TYPES = 1,	
	FDMI_PORT_ATTRIB_SUPP_SPEED,	
	FDMI_PORT_ATTRIB_PORT_SPEED,	
	FDMI_PORT_ATTRIB_FRAME_SIZE,	
	FDMI_PORT_ATTRIB_DEV_NAME,	
	FDMI_PORT_ATTRIB_HOST_NAME,	

	FDMI_PORT_ATTR_MAX_TYPE
};


struct fdmi_attr_s {
	u16        type;
	u16        len;
	u8         value[1];
};


struct fdmi_hba_attr_s {
	u32        attr_count;	
	struct fdmi_attr_s     hba_attr;	
};


struct fdmi_port_list_s {
	u32        num_ports;	
	wwn_t           port_entry;	
};


struct fdmi_port_attr_s {
	u32        attr_count;	
	struct fdmi_attr_s     port_attr;	
};


struct fdmi_rhba_s {
	wwn_t           hba_id;		
	struct fdmi_port_list_s port_list;	
	struct fdmi_hba_attr_s hba_attr_blk;	
};


struct fdmi_rprt_s {
	wwn_t           hba_id;		
	wwn_t           port_name;	
	struct fdmi_port_attr_s port_attr_blk;	
};


struct fdmi_rpa_s {
	wwn_t           port_name;	
	struct fdmi_port_attr_s port_attr_blk;	
};

#pragma pack()

#endif
