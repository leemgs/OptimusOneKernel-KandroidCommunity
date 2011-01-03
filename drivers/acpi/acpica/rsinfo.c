



#include <acpi/acpi.h>
#include "accommon.h"
#include "acresrc.h"

#define _COMPONENT          ACPI_RESOURCES
ACPI_MODULE_NAME("rsinfo")



struct acpi_rsconvert_info *acpi_gbl_set_resource_dispatch[] = {
	acpi_rs_set_irq,	
	acpi_rs_convert_dma,	
	acpi_rs_set_start_dpf,	
	acpi_rs_convert_end_dpf,	
	acpi_rs_convert_io,	
	acpi_rs_convert_fixed_io,	
	acpi_rs_set_vendor,	
	acpi_rs_convert_end_tag,	
	acpi_rs_convert_memory24,	
	acpi_rs_convert_memory32,	
	acpi_rs_convert_fixed_memory32,	
	acpi_rs_convert_address16,	
	acpi_rs_convert_address32,	
	acpi_rs_convert_address64,	
	acpi_rs_convert_ext_address64,	
	acpi_rs_convert_ext_irq,	
	acpi_rs_convert_generic_reg	
};



struct acpi_rsconvert_info *acpi_gbl_get_resource_dispatch[] = {
	

	NULL,			
	NULL,			
	NULL,			
	NULL,			
	acpi_rs_get_irq,	
	acpi_rs_convert_dma,	
	acpi_rs_get_start_dpf,	
	acpi_rs_convert_end_dpf,	
	acpi_rs_convert_io,	
	acpi_rs_convert_fixed_io,	
	NULL,			
	NULL,			
	NULL,			
	NULL,			
	acpi_rs_get_vendor_small,	
	acpi_rs_convert_end_tag,	

	

	NULL,			
	acpi_rs_convert_memory24,	
	acpi_rs_convert_generic_reg,	
	NULL,			
	acpi_rs_get_vendor_large,	
	acpi_rs_convert_memory32,	
	acpi_rs_convert_fixed_memory32,	
	acpi_rs_convert_address32,	
	acpi_rs_convert_address16,	
	acpi_rs_convert_ext_irq,	
	acpi_rs_convert_address64,	
	acpi_rs_convert_ext_address64	
};

#ifdef ACPI_FUTURE_USAGE
#if defined(ACPI_DEBUG_OUTPUT) || defined(ACPI_DEBUGGER)



struct acpi_rsdump_info *acpi_gbl_dump_resource_dispatch[] = {
	acpi_rs_dump_irq,	
	acpi_rs_dump_dma,	
	acpi_rs_dump_start_dpf,	
	acpi_rs_dump_end_dpf,	
	acpi_rs_dump_io,	
	acpi_rs_dump_fixed_io,	
	acpi_rs_dump_vendor,	
	acpi_rs_dump_end_tag,	
	acpi_rs_dump_memory24,	
	acpi_rs_dump_memory32,	
	acpi_rs_dump_fixed_memory32,	
	acpi_rs_dump_address16,	
	acpi_rs_dump_address32,	
	acpi_rs_dump_address64,	
	acpi_rs_dump_ext_address64,	
	acpi_rs_dump_ext_irq,	
	acpi_rs_dump_generic_reg,	
};
#endif

#endif				

const u8 acpi_gbl_aml_resource_sizes[] = {
	sizeof(struct aml_resource_irq),	
	sizeof(struct aml_resource_dma),	
	sizeof(struct aml_resource_start_dependent),	
	sizeof(struct aml_resource_end_dependent),	
	sizeof(struct aml_resource_io),	
	sizeof(struct aml_resource_fixed_io),	
	sizeof(struct aml_resource_vendor_small),	
	sizeof(struct aml_resource_end_tag),	
	sizeof(struct aml_resource_memory24),	
	sizeof(struct aml_resource_memory32),	
	sizeof(struct aml_resource_fixed_memory32),	
	sizeof(struct aml_resource_address16),	
	sizeof(struct aml_resource_address32),	
	sizeof(struct aml_resource_address64),	
	sizeof(struct aml_resource_extended_address64),	
	sizeof(struct aml_resource_extended_irq),	
	sizeof(struct aml_resource_generic_register)	
};

const u8 acpi_gbl_resource_struct_sizes[] = {
	

	0,
	0,
	0,
	0,
	ACPI_RS_SIZE(struct acpi_resource_irq),
	ACPI_RS_SIZE(struct acpi_resource_dma),
	ACPI_RS_SIZE(struct acpi_resource_start_dependent),
	ACPI_RS_SIZE_MIN,
	ACPI_RS_SIZE(struct acpi_resource_io),
	ACPI_RS_SIZE(struct acpi_resource_fixed_io),
	0,
	0,
	0,
	0,
	ACPI_RS_SIZE(struct acpi_resource_vendor),
	ACPI_RS_SIZE_MIN,

	

	0,
	ACPI_RS_SIZE(struct acpi_resource_memory24),
	ACPI_RS_SIZE(struct acpi_resource_generic_register),
	0,
	ACPI_RS_SIZE(struct acpi_resource_vendor),
	ACPI_RS_SIZE(struct acpi_resource_memory32),
	ACPI_RS_SIZE(struct acpi_resource_fixed_memory32),
	ACPI_RS_SIZE(struct acpi_resource_address32),
	ACPI_RS_SIZE(struct acpi_resource_address16),
	ACPI_RS_SIZE(struct acpi_resource_extended_irq),
	ACPI_RS_SIZE(struct acpi_resource_address64),
	ACPI_RS_SIZE(struct acpi_resource_extended_address64)
};
