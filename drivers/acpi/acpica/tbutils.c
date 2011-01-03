



#include <acpi/acpi.h>
#include "accommon.h"
#include "actables.h"

#define _COMPONENT          ACPI_TABLES
ACPI_MODULE_NAME("tbutils")


static void acpi_tb_fix_string(char *string, acpi_size length);

static void
acpi_tb_cleanup_table_header(struct acpi_table_header *out_header,
			     struct acpi_table_header *header);

static acpi_physical_address
acpi_tb_get_root_table_entry(u8 *table_entry, u32 table_entry_size);



static acpi_status
acpi_tb_check_xsdt(acpi_physical_address address)
{
	struct acpi_table_header *table;
	u32 length;
	u64 xsdt_entry_address;
	u8 *table_entry;
	u32 table_count;
	int i;

	table = acpi_os_map_memory(address, sizeof(struct acpi_table_header));
	if (!table)
		return AE_NO_MEMORY;

	length = table->length;
	acpi_os_unmap_memory(table, sizeof(struct acpi_table_header));
	if (length < sizeof(struct acpi_table_header))
		return AE_INVALID_TABLE_LENGTH;

	table = acpi_os_map_memory(address, length);
	if (!table)
		return AE_NO_MEMORY;

	
	table_count =
		(u32) ((table->length -
		sizeof(struct acpi_table_header)) / sizeof(u64));
	table_entry =
		ACPI_CAST_PTR(u8, table) + sizeof(struct acpi_table_header);
	for (i = 0; i < table_count; i++) {
		ACPI_MOVE_64_TO_64(&xsdt_entry_address, table_entry);
		if (!xsdt_entry_address) {
			
			break;
		}
		table_entry += sizeof(u64);
	}
	acpi_os_unmap_memory(table, length);

	if (i < table_count)
		return AE_NULL_ENTRY;
	else
		return AE_OK;
}



acpi_status acpi_tb_initialize_facs(void)
{
	acpi_status status;

	status = acpi_get_table_by_index(ACPI_TABLE_INDEX_FACS,
					 ACPI_CAST_INDIRECT_PTR(struct
								acpi_table_header,
								&acpi_gbl_FACS));
	return status;
}



u8 acpi_tb_tables_loaded(void)
{

	if (acpi_gbl_root_table_list.count >= 3) {
		return (TRUE);
	}

	return (FALSE);
}



static void acpi_tb_fix_string(char *string, acpi_size length)
{

	while (length && *string) {
		if (!ACPI_IS_PRINT(*string)) {
			*string = '?';
		}
		string++;
		length--;
	}
}



static void
acpi_tb_cleanup_table_header(struct acpi_table_header *out_header,
			     struct acpi_table_header *header)
{

	ACPI_MEMCPY(out_header, header, sizeof(struct acpi_table_header));

	acpi_tb_fix_string(out_header->signature, ACPI_NAME_SIZE);
	acpi_tb_fix_string(out_header->oem_id, ACPI_OEM_ID_SIZE);
	acpi_tb_fix_string(out_header->oem_table_id, ACPI_OEM_TABLE_ID_SIZE);
	acpi_tb_fix_string(out_header->asl_compiler_id, ACPI_NAME_SIZE);
}



void
acpi_tb_print_table_header(acpi_physical_address address,
			   struct acpi_table_header *header)
{
	struct acpi_table_header local_header;

	
	if (ACPI_COMPARE_NAME(header->signature, ACPI_SIG_FACS)) {

		

		ACPI_INFO((AE_INFO, "%4.4s %p %05X",
			   header->signature, ACPI_CAST_PTR(void, address),
			   header->length));
	} else if (ACPI_COMPARE_NAME(header->signature, ACPI_SIG_RSDP)) {

		

		ACPI_MEMCPY(local_header.oem_id,
			    ACPI_CAST_PTR(struct acpi_table_rsdp,
					  header)->oem_id, ACPI_OEM_ID_SIZE);
		acpi_tb_fix_string(local_header.oem_id, ACPI_OEM_ID_SIZE);

		ACPI_INFO((AE_INFO, "RSDP %p %05X (v%.2d %6.6s)",
			   ACPI_CAST_PTR (void, address),
			   (ACPI_CAST_PTR(struct acpi_table_rsdp, header)->
			    revision >
			    0) ? ACPI_CAST_PTR(struct acpi_table_rsdp,
					       header)->length : 20,
			   ACPI_CAST_PTR(struct acpi_table_rsdp,
					 header)->revision,
			   local_header.oem_id));
	} else {
		

		acpi_tb_cleanup_table_header(&local_header, header);

		ACPI_INFO((AE_INFO,
			   "%4.4s %p %05X (v%.2d %6.6s %8.8s %08X %4.4s %08X)",
			   local_header.signature, ACPI_CAST_PTR(void, address),
			   local_header.length, local_header.revision,
			   local_header.oem_id, local_header.oem_table_id,
			   local_header.oem_revision,
			   local_header.asl_compiler_id,
			   local_header.asl_compiler_revision));

	}
}



acpi_status acpi_tb_verify_checksum(struct acpi_table_header *table, u32 length)
{
	u8 checksum;

	

	checksum = acpi_tb_checksum(ACPI_CAST_PTR(u8, table), length);

	

	if (checksum) {
		ACPI_WARNING((AE_INFO,
			      "Incorrect checksum in table [%4.4s] - %2.2X, should be %2.2X",
			      table->signature, table->checksum,
			      (u8) (table->checksum - checksum)));

#if (ACPI_CHECKSUM_ABORT)

		return (AE_BAD_CHECKSUM);
#endif
	}

	return (AE_OK);
}



u8 acpi_tb_checksum(u8 *buffer, u32 length)
{
	u8 sum = 0;
	u8 *end = buffer + length;

	while (buffer < end) {
		sum = (u8) (sum + *(buffer++));
	}

	return sum;
}



void
acpi_tb_install_table(acpi_physical_address address,
		      char *signature, u32 table_index)
{
	u8 flags;
	acpi_status status;
	struct acpi_table_header *table_to_install;
	struct acpi_table_header *mapped_table;
	struct acpi_table_header *override_table = NULL;

	if (!address) {
		ACPI_ERROR((AE_INFO,
			    "Null physical address for ACPI table [%s]",
			    signature));
		return;
	}

	

	mapped_table =
	    acpi_os_map_memory(address, sizeof(struct acpi_table_header));
	if (!mapped_table) {
		return;
	}

	

	if (signature && !ACPI_COMPARE_NAME(mapped_table->signature, signature)) {
		ACPI_ERROR((AE_INFO,
			    "Invalid signature 0x%X for ACPI table, expected [%s]",
			    *ACPI_CAST_PTR(u32, mapped_table->signature),
			    signature));
		goto unmap_and_exit;
	}

	
	status = acpi_os_table_override(mapped_table, &override_table);
	if (ACPI_SUCCESS(status) && override_table) {
		ACPI_INFO((AE_INFO,
			   "%4.4s @ 0x%p Table override, replaced with:",
			   mapped_table->signature, ACPI_CAST_PTR(void,
								  address)));

		acpi_gbl_root_table_list.tables[table_index].pointer =
		    override_table;
		address = ACPI_PTR_TO_PHYSADDR(override_table);

		table_to_install = override_table;
		flags = ACPI_TABLE_ORIGIN_OVERRIDE;
	} else {
		table_to_install = mapped_table;
		flags = ACPI_TABLE_ORIGIN_MAPPED;
	}

	

	acpi_gbl_root_table_list.tables[table_index].address = address;
	acpi_gbl_root_table_list.tables[table_index].length =
	    table_to_install->length;
	acpi_gbl_root_table_list.tables[table_index].flags = flags;

	ACPI_MOVE_32_TO_32(&
			   (acpi_gbl_root_table_list.tables[table_index].
			    signature), table_to_install->signature);

	acpi_tb_print_table_header(address, table_to_install);

	if (table_index == ACPI_TABLE_INDEX_DSDT) {

		

		acpi_ut_set_integer_width(table_to_install->revision);
	}

      unmap_and_exit:
	acpi_os_unmap_memory(mapped_table, sizeof(struct acpi_table_header));
}



static acpi_physical_address
acpi_tb_get_root_table_entry(u8 *table_entry, u32 table_entry_size)
{
	u64 address64;

	
	if (table_entry_size == sizeof(u32)) {
		
		return ((acpi_physical_address)
			(*ACPI_CAST_PTR(u32, table_entry)));
	} else {
		
		ACPI_MOVE_64_TO_64(&address64, table_entry);

#if ACPI_MACHINE_WIDTH == 32
		if (address64 > ACPI_UINT32_MAX) {

			

			ACPI_WARNING((AE_INFO,
				      "64-bit Physical Address in XSDT is too large (%8.8X%8.8X),"
				      " truncating",
				      ACPI_FORMAT_UINT64(address64)));
		}
#endif
		return ((acpi_physical_address) (address64));
	}
}



acpi_status __init
acpi_tb_parse_root_table(acpi_physical_address rsdp_address)
{
	struct acpi_table_rsdp *rsdp;
	u32 table_entry_size;
	u32 i;
	u32 table_count;
	struct acpi_table_header *table;
	acpi_physical_address address;
	acpi_physical_address uninitialized_var(rsdt_address);
	u32 length;
	u8 *table_entry;
	acpi_status status;

	ACPI_FUNCTION_TRACE(tb_parse_root_table);

	
	rsdp = acpi_os_map_memory(rsdp_address, sizeof(struct acpi_table_rsdp));
	if (!rsdp) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	acpi_tb_print_table_header(rsdp_address,
				   ACPI_CAST_PTR(struct acpi_table_header,
						 rsdp));

	

	if (rsdp->revision > 1 && rsdp->xsdt_physical_address
			&& !acpi_rsdt_forced) {
		
		address = (acpi_physical_address) rsdp->xsdt_physical_address;
		table_entry_size = sizeof(u64);
		rsdt_address = (acpi_physical_address)
					rsdp->rsdt_physical_address;
	} else {
		

		address = (acpi_physical_address) rsdp->rsdt_physical_address;
		table_entry_size = sizeof(u32);
	}

	
	acpi_os_unmap_memory(rsdp, sizeof(struct acpi_table_rsdp));

	if (table_entry_size == sizeof(u64)) {
		if (acpi_tb_check_xsdt(address) == AE_NULL_ENTRY) {
			
			address = rsdt_address;
			table_entry_size = sizeof(u32);
			ACPI_WARNING((AE_INFO, "BIOS XSDT has NULL entry, "
					"using RSDT"));
		}
	}
	

	table = acpi_os_map_memory(address, sizeof(struct acpi_table_header));
	if (!table) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	acpi_tb_print_table_header(address, table);

	

	length = table->length;
	acpi_os_unmap_memory(table, sizeof(struct acpi_table_header));

	if (length < sizeof(struct acpi_table_header)) {
		ACPI_ERROR((AE_INFO, "Invalid length 0x%X in RSDT/XSDT",
			    length));
		return_ACPI_STATUS(AE_INVALID_TABLE_LENGTH);
	}

	table = acpi_os_map_memory(address, length);
	if (!table) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	

	status = acpi_tb_verify_checksum(table, length);
	if (ACPI_FAILURE(status)) {
		acpi_os_unmap_memory(table, length);
		return_ACPI_STATUS(status);
	}

	

	table_count = (u32)((table->length - sizeof(struct acpi_table_header)) /
			    table_entry_size);
	
	table_entry =
	    ACPI_CAST_PTR(u8, table) + sizeof(struct acpi_table_header);
	acpi_gbl_root_table_list.count = 2;

	
	for (i = 0; i < table_count; i++) {
		if (acpi_gbl_root_table_list.count >=
		    acpi_gbl_root_table_list.size) {

			

			status = acpi_tb_resize_root_table_list();
			if (ACPI_FAILURE(status)) {
				ACPI_WARNING((AE_INFO,
					      "Truncating %u table entries!",
					      (unsigned) (table_count -
					       (acpi_gbl_root_table_list.
					       count - 2))));
				break;
			}
		}

		

		acpi_gbl_root_table_list.tables[acpi_gbl_root_table_list.count].
		    address =
		    acpi_tb_get_root_table_entry(table_entry, table_entry_size);

		table_entry += table_entry_size;
		acpi_gbl_root_table_list.count++;
	}

	
	acpi_os_unmap_memory(table, length);

	
	for (i = 2; i < acpi_gbl_root_table_list.count; i++) {
		acpi_tb_install_table(acpi_gbl_root_table_list.tables[i].
				      address, NULL, i);

		

		if (ACPI_COMPARE_NAME
		    (&acpi_gbl_root_table_list.tables[i].signature,
		     ACPI_SIG_FADT)) {
			acpi_tb_parse_fadt(i);
		}
	}

	return_ACPI_STATUS(AE_OK);
}
