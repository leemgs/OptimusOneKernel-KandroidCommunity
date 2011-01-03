



#ifndef __ACRESRC_H__
#define __ACRESRC_H__



#include "amlresrc.h"


#if (!defined(ACPI_MISALIGNMENT_NOT_SUPPORTED) && !defined(ACPI_PACKED_POINTERS_NOT_SUPPORTED))
#pragma pack(1)
#endif


typedef const struct acpi_rsconvert_info {
	u8 opcode;
	u8 resource_offset;
	u8 aml_offset;
	u8 value;

} acpi_rsconvert_info;



#define ACPI_RSC_INITGET                0
#define ACPI_RSC_INITSET                1
#define ACPI_RSC_FLAGINIT               2
#define ACPI_RSC_1BITFLAG               3
#define ACPI_RSC_2BITFLAG               4
#define ACPI_RSC_COUNT                  5
#define ACPI_RSC_COUNT16                6
#define ACPI_RSC_LENGTH                 7
#define ACPI_RSC_MOVE8                  8
#define ACPI_RSC_MOVE16                 9
#define ACPI_RSC_MOVE32                 10
#define ACPI_RSC_MOVE64                 11
#define ACPI_RSC_SET8                   12
#define ACPI_RSC_DATA8                  13
#define ACPI_RSC_ADDRESS                14
#define ACPI_RSC_SOURCE                 15
#define ACPI_RSC_SOURCEX                16
#define ACPI_RSC_BITMASK                17
#define ACPI_RSC_BITMASK16              18
#define ACPI_RSC_EXIT_NE                19
#define ACPI_RSC_EXIT_LE                20
#define ACPI_RSC_EXIT_EQ                21



#define ACPI_RSC_COMPARE_AML_LENGTH     0
#define ACPI_RSC_COMPARE_VALUE          1

#define ACPI_RSC_TABLE_SIZE(d)          (sizeof (d) / sizeof (struct acpi_rsconvert_info))

#define ACPI_RS_OFFSET(f)               (u8) ACPI_OFFSET (struct acpi_resource,f)
#define AML_OFFSET(f)                   (u8) ACPI_OFFSET (union aml_resource,f)

typedef const struct acpi_rsdump_info {
	u8 opcode;
	u8 offset;
	char *name;
	const char **pointer;

} acpi_rsdump_info;



#define ACPI_RSD_TITLE                  0
#define ACPI_RSD_LITERAL                1
#define ACPI_RSD_STRING                 2
#define ACPI_RSD_UINT8                  3
#define ACPI_RSD_UINT16                 4
#define ACPI_RSD_UINT32                 5
#define ACPI_RSD_UINT64                 6
#define ACPI_RSD_1BITFLAG               7
#define ACPI_RSD_2BITFLAG               8
#define ACPI_RSD_SHORTLIST              9
#define ACPI_RSD_LONGLIST               10
#define ACPI_RSD_DWORDLIST              11
#define ACPI_RSD_ADDRESS                12
#define ACPI_RSD_SOURCE                 13



#pragma pack()



extern const u8 acpi_gbl_aml_resource_sizes[];
extern struct acpi_rsconvert_info *acpi_gbl_set_resource_dispatch[];



extern const u8 acpi_gbl_resource_struct_sizes[];
extern struct acpi_rsconvert_info *acpi_gbl_get_resource_dispatch[];

struct acpi_vendor_walk_info {
	struct acpi_vendor_uuid *uuid;
	struct acpi_buffer *buffer;
	acpi_status status;
};


acpi_status
acpi_rs_create_resource_list(union acpi_operand_object *aml_buffer,
			     struct acpi_buffer *output_buffer);

acpi_status
acpi_rs_create_aml_resources(struct acpi_resource *linked_list_buffer,
			     struct acpi_buffer *output_buffer);

acpi_status
acpi_rs_create_pci_routing_table(union acpi_operand_object *package_object,
				 struct acpi_buffer *output_buffer);



acpi_status
acpi_rs_get_prt_method_data(struct acpi_namespace_node *node,
			    struct acpi_buffer *ret_buffer);

acpi_status
acpi_rs_get_crs_method_data(struct acpi_namespace_node *node,
			    struct acpi_buffer *ret_buffer);

acpi_status
acpi_rs_get_prs_method_data(struct acpi_namespace_node *node,
			    struct acpi_buffer *ret_buffer);

acpi_status
acpi_rs_get_method_data(acpi_handle handle,
			char *path, struct acpi_buffer *ret_buffer);

acpi_status
acpi_rs_set_srs_method_data(struct acpi_namespace_node *node,
			    struct acpi_buffer *ret_buffer);


acpi_status
acpi_rs_get_list_length(u8 * aml_buffer,
			u32 aml_buffer_length, acpi_size * size_needed);

acpi_status
acpi_rs_get_aml_length(struct acpi_resource *linked_list_buffer,
		       acpi_size * size_needed);

acpi_status
acpi_rs_get_pci_routing_table_length(union acpi_operand_object *package_object,
				     acpi_size * buffer_size_needed);

acpi_status
acpi_rs_convert_aml_to_resources(u8 * aml,
				 u32 length,
				 u32 offset, u8 resource_index, void **context);

acpi_status
acpi_rs_convert_resources_to_aml(struct acpi_resource *resource,
				 acpi_size aml_size_needed, u8 * output_buffer);


void
acpi_rs_set_address_common(union aml_resource *aml,
			   struct acpi_resource *resource);

u8
acpi_rs_get_address_common(struct acpi_resource *resource,
			   union aml_resource *aml);


acpi_status
acpi_rs_convert_aml_to_resource(struct acpi_resource *resource,
				union aml_resource *aml,
				struct acpi_rsconvert_info *info);

acpi_status
acpi_rs_convert_resource_to_aml(struct acpi_resource *resource,
				union aml_resource *aml,
				struct acpi_rsconvert_info *info);


void
acpi_rs_move_data(void *destination,
		  void *source, u16 item_count, u8 move_type);

u8 acpi_rs_decode_bitmask(u16 mask, u8 * list);

u16 acpi_rs_encode_bitmask(u8 * list, u8 count);

acpi_rs_length
acpi_rs_get_resource_source(acpi_rs_length resource_length,
			    acpi_rs_length minimum_length,
			    struct acpi_resource_source *resource_source,
			    union aml_resource *aml, char *string_ptr);

acpi_rsdesc_size
acpi_rs_set_resource_source(union aml_resource *aml,
			    acpi_rs_length minimum_length,
			    struct acpi_resource_source *resource_source);

void
acpi_rs_set_resource_header(u8 descriptor_type,
			    acpi_rsdesc_size total_length,
			    union aml_resource *aml);

void
acpi_rs_set_resource_length(acpi_rsdesc_size total_length,
			    union aml_resource *aml);


void acpi_rs_dump_resource_list(struct acpi_resource *resource);

void acpi_rs_dump_irq_list(u8 * route_table);


extern struct acpi_rsconvert_info acpi_rs_convert_dma[];
extern struct acpi_rsconvert_info acpi_rs_convert_end_dpf[];
extern struct acpi_rsconvert_info acpi_rs_convert_io[];
extern struct acpi_rsconvert_info acpi_rs_convert_fixed_io[];
extern struct acpi_rsconvert_info acpi_rs_convert_end_tag[];
extern struct acpi_rsconvert_info acpi_rs_convert_memory24[];
extern struct acpi_rsconvert_info acpi_rs_convert_generic_reg[];
extern struct acpi_rsconvert_info acpi_rs_convert_memory32[];
extern struct acpi_rsconvert_info acpi_rs_convert_fixed_memory32[];
extern struct acpi_rsconvert_info acpi_rs_convert_address32[];
extern struct acpi_rsconvert_info acpi_rs_convert_address16[];
extern struct acpi_rsconvert_info acpi_rs_convert_ext_irq[];
extern struct acpi_rsconvert_info acpi_rs_convert_address64[];
extern struct acpi_rsconvert_info acpi_rs_convert_ext_address64[];



extern struct acpi_rsconvert_info acpi_rs_get_irq[];
extern struct acpi_rsconvert_info acpi_rs_get_start_dpf[];
extern struct acpi_rsconvert_info acpi_rs_get_vendor_small[];
extern struct acpi_rsconvert_info acpi_rs_get_vendor_large[];

extern struct acpi_rsconvert_info acpi_rs_set_irq[];
extern struct acpi_rsconvert_info acpi_rs_set_start_dpf[];
extern struct acpi_rsconvert_info acpi_rs_set_vendor[];

#if defined(ACPI_DEBUG_OUTPUT) || defined(ACPI_DEBUGGER)

extern struct acpi_rsdump_info *acpi_gbl_dump_resource_dispatch[];


extern struct acpi_rsdump_info acpi_rs_dump_irq[];
extern struct acpi_rsdump_info acpi_rs_dump_dma[];
extern struct acpi_rsdump_info acpi_rs_dump_start_dpf[];
extern struct acpi_rsdump_info acpi_rs_dump_end_dpf[];
extern struct acpi_rsdump_info acpi_rs_dump_io[];
extern struct acpi_rsdump_info acpi_rs_dump_fixed_io[];
extern struct acpi_rsdump_info acpi_rs_dump_vendor[];
extern struct acpi_rsdump_info acpi_rs_dump_end_tag[];
extern struct acpi_rsdump_info acpi_rs_dump_memory24[];
extern struct acpi_rsdump_info acpi_rs_dump_memory32[];
extern struct acpi_rsdump_info acpi_rs_dump_fixed_memory32[];
extern struct acpi_rsdump_info acpi_rs_dump_address16[];
extern struct acpi_rsdump_info acpi_rs_dump_address32[];
extern struct acpi_rsdump_info acpi_rs_dump_address64[];
extern struct acpi_rsdump_info acpi_rs_dump_ext_address64[];
extern struct acpi_rsdump_info acpi_rs_dump_ext_irq[];
extern struct acpi_rsdump_info acpi_rs_dump_generic_reg[];
#endif

#endif				
