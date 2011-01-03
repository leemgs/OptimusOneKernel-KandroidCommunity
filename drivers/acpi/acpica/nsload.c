



#include <acpi/acpi.h>
#include "accommon.h"
#include "acnamesp.h"
#include "acdispat.h"
#include "actables.h"

#define _COMPONENT          ACPI_NAMESPACE
ACPI_MODULE_NAME("nsload")


#ifdef ACPI_FUTURE_IMPLEMENTATION
acpi_status acpi_ns_unload_namespace(acpi_handle handle);

static acpi_status acpi_ns_delete_subtree(acpi_handle start_handle);
#endif

#ifndef ACPI_NO_METHOD_EXECUTION


acpi_status
acpi_ns_load_table(u32 table_index, struct acpi_namespace_node *node)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(ns_load_table);

	
	status = acpi_ut_acquire_mutex(ACPI_MTX_NAMESPACE);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	if (acpi_tb_is_table_loaded(table_index)) {
		status = AE_ALREADY_EXISTS;
		goto unlock;
	}

	ACPI_DEBUG_PRINT((ACPI_DB_INFO,
			  "**** Loading table into namespace ****\n"));

	status = acpi_tb_allocate_owner_id(table_index);
	if (ACPI_FAILURE(status)) {
		goto unlock;
	}

	status = acpi_ns_parse_table(table_index, node);
	if (ACPI_SUCCESS(status)) {
		acpi_tb_set_table_loaded_flag(table_index, TRUE);
	} else {
		(void)acpi_tb_release_owner_id(table_index);
	}

      unlock:
	(void)acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);

	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	
	ACPI_DEBUG_PRINT((ACPI_DB_INFO,
			  "**** Begin Table Method Parsing and Object Initialization\n"));

	status = acpi_ds_initialize_objects(table_index, node);

	ACPI_DEBUG_PRINT((ACPI_DB_INFO,
			  "**** Completed Table Method Parsing and Object Initialization\n"));

	return_ACPI_STATUS(status);
}

#ifdef ACPI_OBSOLETE_FUNCTIONS


acpi_status acpi_ns_load_namespace(void)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(acpi_load_name_space);

	

	if (acpi_gbl_DSDT == NULL) {
		ACPI_ERROR((AE_INFO, "DSDT is not in memory"));
		return_ACPI_STATUS(AE_NO_ACPI_TABLES);
	}

	
	status = acpi_ns_load_table_by_type(ACPI_TABLE_ID_DSDT);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	(void)acpi_ns_load_table_by_type(ACPI_TABLE_ID_SSDT);
	(void)acpi_ns_load_table_by_type(ACPI_TABLE_ID_PSDT);

	ACPI_DEBUG_PRINT_RAW((ACPI_DB_INIT,
			      "ACPI Namespace successfully loaded at root %p\n",
			      acpi_gbl_root_node));

	return_ACPI_STATUS(status);
}
#endif

#ifdef ACPI_FUTURE_IMPLEMENTATION


static acpi_status acpi_ns_delete_subtree(acpi_handle start_handle)
{
	acpi_status status;
	acpi_handle child_handle;
	acpi_handle parent_handle;
	acpi_handle next_child_handle;
	acpi_handle dummy;
	u32 level;

	ACPI_FUNCTION_TRACE(ns_delete_subtree);

	parent_handle = start_handle;
	child_handle = NULL;
	level = 1;

	
	while (level > 0) {

		

		status = acpi_get_next_object(ACPI_TYPE_ANY, parent_handle,
					      child_handle, &next_child_handle);

		child_handle = next_child_handle;

		

		if (ACPI_SUCCESS(status)) {

			

			if (ACPI_SUCCESS
			    (acpi_get_next_object
			     (ACPI_TYPE_ANY, child_handle, NULL, &dummy))) {
				
				level++;
				parent_handle = child_handle;
				child_handle = NULL;
			}
		} else {
			
			level--;

			

			acpi_ns_delete_children(child_handle);

			child_handle = parent_handle;
			status = acpi_get_parent(parent_handle, &parent_handle);
			if (ACPI_FAILURE(status)) {
				return_ACPI_STATUS(status);
			}
		}
	}

	

	acpi_ns_remove_node(child_handle);
	return_ACPI_STATUS(AE_OK);
}



acpi_status acpi_ns_unload_namespace(acpi_handle handle)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(ns_unload_name_space);

	

	if (!acpi_gbl_root_node) {
		return_ACPI_STATUS(AE_NO_NAMESPACE);
	}

	if (!handle) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	

	status = acpi_ns_delete_subtree(handle);

	return_ACPI_STATUS(status);
}
#endif
#endif
