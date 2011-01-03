



#ifndef __ACTBL_H__
#define __ACTBL_H__




#define ACPI_SIG_DSDT           "DSDT"	
#define ACPI_SIG_FADT           "FACP"	
#define ACPI_SIG_FACS           "FACS"	
#define ACPI_SIG_PSDT           "PSDT"	
#define ACPI_SIG_RSDP           "RSD PTR "	
#define ACPI_SIG_RSDT           "RSDT"	
#define ACPI_SIG_XSDT           "XSDT"	
#define ACPI_SIG_SSDT           "SSDT"	
#define ACPI_RSDP_NAME          "RSDP"	


#pragma pack(1)





struct acpi_table_header {
	char signature[ACPI_NAME_SIZE];	
	u32 length;		
	u8 revision;		
	u8 checksum;		
	char oem_id[ACPI_OEM_ID_SIZE];	
	char oem_table_id[ACPI_OEM_TABLE_ID_SIZE];	
	u32 oem_revision;	
	char asl_compiler_id[ACPI_NAME_SIZE];	
	u32 asl_compiler_revision;	
};



struct acpi_generic_address {
	u8 space_id;		
	u8 bit_width;		
	u8 bit_offset;		
	u8 access_width;	
	u64 address;		
};



struct acpi_table_rsdp {
	char signature[8];	
	u8 checksum;		
	char oem_id[ACPI_OEM_ID_SIZE];	
	u8 revision;		
	u32 rsdt_physical_address;	
	u32 length;		
	u64 xsdt_physical_address;	
	u8 extended_checksum;	
	u8 reserved[3];		
};

#define ACPI_RSDP_REV0_SIZE     20	



struct acpi_table_rsdt {
	struct acpi_table_header header;	
	u32 table_offset_entry[1];	
};

struct acpi_table_xsdt {
	struct acpi_table_header header;	
	u64 table_offset_entry[1];	
};



struct acpi_table_facs {
	char signature[4];	
	u32 length;		
	u32 hardware_signature;	
	u32 firmware_waking_vector;	
	u32 global_lock;	
	u32 flags;
	u64 xfirmware_waking_vector;	
	u8 version;		
	u8 reserved[3];		
	u32 ospm_flags;		
	u8 reserved1[24];	
};



#define ACPI_GLOCK_PENDING          (1)	
#define ACPI_GLOCK_OWNED            (1<<1)	



#define ACPI_FACS_S4_BIOS_PRESENT   (1)	
#define ACPI_FACS_64BIT_WAKE        (1<<1)	



#define ACPI_FACS_64BIT_ENVIRONMENT (1)	





struct acpi_table_fadt {
	struct acpi_table_header header;	
	u32 facs;		
	u32 dsdt;		
	u8 model;		
	u8 preferred_profile;	
	u16 sci_interrupt;	
	u32 smi_command;	
	u8 acpi_enable;		
	u8 acpi_disable;	
	u8 S4bios_request;	
	u8 pstate_control;	
	u32 pm1a_event_block;	
	u32 pm1b_event_block;	
	u32 pm1a_control_block;	
	u32 pm1b_control_block;	
	u32 pm2_control_block;	
	u32 pm_timer_block;	
	u32 gpe0_block;		
	u32 gpe1_block;		
	u8 pm1_event_length;	
	u8 pm1_control_length;	
	u8 pm2_control_length;	
	u8 pm_timer_length;	
	u8 gpe0_block_length;	
	u8 gpe1_block_length;	
	u8 gpe1_base;		
	u8 cst_control;		
	u16 C2latency;		
	u16 C3latency;		
	u16 flush_size;		
	u16 flush_stride;	
	u8 duty_offset;		
	u8 duty_width;		
	u8 day_alarm;		
	u8 month_alarm;		
	u8 century;		
	u16 boot_flags;		
	u8 reserved;		
	u32 flags;		
	struct acpi_generic_address reset_register;	
	u8 reset_value;		
	u8 reserved4[3];	
	u64 Xfacs;		
	u64 Xdsdt;		
	struct acpi_generic_address xpm1a_event_block;	
	struct acpi_generic_address xpm1b_event_block;	
	struct acpi_generic_address xpm1a_control_block;	
	struct acpi_generic_address xpm1b_control_block;	
	struct acpi_generic_address xpm2_control_block;	
	struct acpi_generic_address xpm_timer_block;	
	struct acpi_generic_address xgpe0_block;	
	struct acpi_generic_address xgpe1_block;	
};



#define ACPI_FADT_LEGACY_DEVICES    (1)  	
#define ACPI_FADT_8042              (1<<1)	
#define ACPI_FADT_NO_VGA            (1<<2)	
#define ACPI_FADT_NO_MSI            (1<<3)	
#define ACPI_FADT_NO_ASPM           (1<<4)	

#define FADT2_REVISION_ID               3



#define ACPI_FADT_WBINVD            (1)	
#define ACPI_FADT_WBINVD_FLUSH      (1<<1)	
#define ACPI_FADT_C1_SUPPORTED      (1<<2)	
#define ACPI_FADT_C2_MP_SUPPORTED   (1<<3)	
#define ACPI_FADT_POWER_BUTTON      (1<<4)	
#define ACPI_FADT_SLEEP_BUTTON      (1<<5)	
#define ACPI_FADT_FIXED_RTC         (1<<6)	
#define ACPI_FADT_S4_RTC_WAKE       (1<<7)	
#define ACPI_FADT_32BIT_TIMER       (1<<8)	
#define ACPI_FADT_DOCKING_SUPPORTED (1<<9)	
#define ACPI_FADT_RESET_REGISTER    (1<<10)	
#define ACPI_FADT_SEALED_CASE       (1<<11)	
#define ACPI_FADT_HEADLESS          (1<<12)	
#define ACPI_FADT_SLEEP_TYPE        (1<<13)	
#define ACPI_FADT_PCI_EXPRESS_WAKE  (1<<14)	
#define ACPI_FADT_PLATFORM_CLOCK    (1<<15)	
#define ACPI_FADT_S4_RTC_VALID      (1<<16)	
#define ACPI_FADT_REMOTE_POWER_ON   (1<<17)	
#define ACPI_FADT_APIC_CLUSTER      (1<<18)	
#define ACPI_FADT_APIC_PHYSICAL     (1<<19)	



enum acpi_prefered_pm_profiles {
	PM_UNSPECIFIED = 0,
	PM_DESKTOP = 1,
	PM_MOBILE = 2,
	PM_WORKSTATION = 3,
	PM_ENTERPRISE_SERVER = 4,
	PM_SOHO_SERVER = 5,
	PM_APPLIANCE_PC = 6
};



#pragma pack()

#define ACPI_FADT_OFFSET(f)             (u8) ACPI_OFFSET (struct acpi_table_fadt, f)


union acpi_name_union {
	u32 integer;
	char ascii[4];
};



struct acpi_table_desc {
	acpi_physical_address address;
	struct acpi_table_header *pointer;
	u32 length;		
	union acpi_name_union signature;
	acpi_owner_id owner_id;
	u8 flags;
};



#define ACPI_TABLE_ORIGIN_UNKNOWN       (0)
#define ACPI_TABLE_ORIGIN_MAPPED        (1)
#define ACPI_TABLE_ORIGIN_ALLOCATED     (2)
#define ACPI_TABLE_ORIGIN_OVERRIDE      (4)
#define ACPI_TABLE_ORIGIN_MASK          (7)
#define ACPI_TABLE_IS_LOADED            (8)



#include <acpi/actbl1.h>
#include <acpi/actbl2.h>

#endif				
