



#include <acpi/acpi.h>
#include "accommon.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_UTILITIES
ACPI_MODULE_NAME("uteval")


static struct acpi_interface_info acpi_interfaces_supported[] = {
	

	{"Windows 2000", ACPI_OSI_WIN_2000},	
	{"Windows 2001", ACPI_OSI_WIN_XP},	
	{"Windows 2001 SP1", ACPI_OSI_WIN_XP_SP1},	
	{"Windows 2001.1", ACPI_OSI_WINSRV_2003},	
	{"Windows 2001 SP2", ACPI_OSI_WIN_XP_SP2},	
	{"Windows 2001.1 SP1", ACPI_OSI_WINSRV_2003_SP1},	
	{"Windows 2006", ACPI_OSI_WIN_VISTA},	
	{"Windows 2006.1", ACPI_OSI_WINSRV_2008},	
	{"Windows 2006 SP1", ACPI_OSI_WIN_VISTA_SP1},	
	{"Windows 2009", ACPI_OSI_WIN_7},	

	

	{"Extended Address Space Descriptor", 0}

	
};



acpi_status acpi_ut_osi_implementation(struct acpi_walk_state *walk_state)
{
	acpi_status status;
	union acpi_operand_object *string_desc;
	union acpi_operand_object *return_desc;
	u32 return_value;
	u32 i;

	ACPI_FUNCTION_TRACE(ut_osi_implementation);

	

	string_desc = walk_state->arguments[0].object;
	if (!string_desc || (string_desc->common.type != ACPI_TYPE_STRING)) {
		return_ACPI_STATUS(AE_TYPE);
	}

	

	return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
	if (!return_desc) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	

	return_value = 0;

	

	for (i = 0; i < ACPI_ARRAY_LENGTH(acpi_interfaces_supported); i++) {
		if (!ACPI_STRCMP(string_desc->string.pointer,
				 acpi_interfaces_supported[i].name)) {
			
			if (acpi_interfaces_supported[i].value >
			    acpi_gbl_osi_data) {
				acpi_gbl_osi_data =
				    acpi_interfaces_supported[i].value;
			}

			return_value = ACPI_UINT32_MAX;
			goto exit;
		}
	}

	
	status = acpi_os_validate_interface(string_desc->string.pointer);
	if (ACPI_SUCCESS(status)) {

		

		return_value = ACPI_UINT32_MAX;
	}

exit:
	ACPI_DEBUG_PRINT_RAW ((ACPI_DB_INFO,
		"ACPI: BIOS _OSI(%s) is %ssupported\n",
		string_desc->string.pointer, return_value == 0 ? "not " : ""));

	

	return_desc->integer.value = return_value;
	walk_state->return_desc = return_desc;
	return_ACPI_STATUS (AE_OK);
}



acpi_status acpi_osi_invalidate(char *interface)
{
	int i;

	for (i = 0; i < ACPI_ARRAY_LENGTH(acpi_interfaces_supported); i++) {
		if (!ACPI_STRCMP(interface, acpi_interfaces_supported[i].name)) {
			*acpi_interfaces_supported[i].name = '\0';
			return AE_OK;
		}
	}
	return AE_NOT_FOUND;
}



acpi_status
acpi_ut_evaluate_object(struct acpi_namespace_node *prefix_node,
			char *path,
			u32 expected_return_btypes,
			union acpi_operand_object **return_desc)
{
	struct acpi_evaluate_info *info;
	acpi_status status;
	u32 return_btype;

	ACPI_FUNCTION_TRACE(ut_evaluate_object);

	

	info = ACPI_ALLOCATE_ZEROED(sizeof(struct acpi_evaluate_info));
	if (!info) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	info->prefix_node = prefix_node;
	info->pathname = path;

	

	status = acpi_ns_evaluate(info);
	if (ACPI_FAILURE(status)) {
		if (status == AE_NOT_FOUND) {
			ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
					  "[%4.4s.%s] was not found\n",
					  acpi_ut_get_node_name(prefix_node),
					  path));
		} else {
			ACPI_ERROR_METHOD("Method execution failed",
					  prefix_node, path, status);
		}

		goto cleanup;
	}

	

	if (!info->return_object) {
		if (expected_return_btypes) {
			ACPI_ERROR_METHOD("No object was returned from",
					  prefix_node, path, AE_NOT_EXIST);

			status = AE_NOT_EXIST;
		}

		goto cleanup;
	}

	

	switch ((info->return_object)->common.type) {
	case ACPI_TYPE_INTEGER:
		return_btype = ACPI_BTYPE_INTEGER;
		break;

	case ACPI_TYPE_BUFFER:
		return_btype = ACPI_BTYPE_BUFFER;
		break;

	case ACPI_TYPE_STRING:
		return_btype = ACPI_BTYPE_STRING;
		break;

	case ACPI_TYPE_PACKAGE:
		return_btype = ACPI_BTYPE_PACKAGE;
		break;

	default:
		return_btype = 0;
		break;
	}

	if ((acpi_gbl_enable_interpreter_slack) && (!expected_return_btypes)) {
		
		acpi_ut_remove_reference(info->return_object);
		goto cleanup;
	}

	

	if (!(expected_return_btypes & return_btype)) {
		ACPI_ERROR_METHOD("Return object type is incorrect",
				  prefix_node, path, AE_TYPE);

		ACPI_ERROR((AE_INFO,
			    "Type returned from %s was incorrect: %s, expected Btypes: %X",
			    path,
			    acpi_ut_get_object_type_name(info->return_object),
			    expected_return_btypes));

		

		acpi_ut_remove_reference(info->return_object);
		status = AE_TYPE;
		goto cleanup;
	}

	

	*return_desc = info->return_object;

      cleanup:
	ACPI_FREE(info);
	return_ACPI_STATUS(status);
}



acpi_status
acpi_ut_evaluate_numeric_object(char *object_name,
				struct acpi_namespace_node *device_node,
				acpi_integer *value)
{
	union acpi_operand_object *obj_desc;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ut_evaluate_numeric_object);

	status = acpi_ut_evaluate_object(device_node, object_name,
					 ACPI_BTYPE_INTEGER, &obj_desc);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	*value = obj_desc->integer.value;

	

	acpi_ut_remove_reference(obj_desc);
	return_ACPI_STATUS(status);
}



acpi_status
acpi_ut_execute_STA(struct acpi_namespace_node *device_node, u32 * flags)
{
	union acpi_operand_object *obj_desc;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ut_execute_STA);

	status = acpi_ut_evaluate_object(device_node, METHOD_NAME__STA,
					 ACPI_BTYPE_INTEGER, &obj_desc);
	if (ACPI_FAILURE(status)) {
		if (AE_NOT_FOUND == status) {
			ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
					  "_STA on %4.4s was not found, assuming device is present\n",
					  acpi_ut_get_node_name(device_node)));

			*flags = ACPI_UINT32_MAX;
			status = AE_OK;
		}

		return_ACPI_STATUS(status);
	}

	

	*flags = (u32) obj_desc->integer.value;

	

	acpi_ut_remove_reference(obj_desc);
	return_ACPI_STATUS(status);
}



acpi_status
acpi_ut_execute_power_methods(struct acpi_namespace_node *device_node,
			      const char **method_names,
			      u8 method_count, u8 *out_values)
{
	union acpi_operand_object *obj_desc;
	acpi_status status;
	acpi_status final_status = AE_NOT_FOUND;
	u32 i;

	ACPI_FUNCTION_TRACE(ut_execute_power_methods);

	for (i = 0; i < method_count; i++) {
		
		status = acpi_ut_evaluate_object(device_node,
						 ACPI_CAST_PTR(char,
							       method_names[i]),
						 ACPI_BTYPE_INTEGER, &obj_desc);
		if (ACPI_SUCCESS(status)) {
			out_values[i] = (u8)obj_desc->integer.value;

			

			acpi_ut_remove_reference(obj_desc);
			final_status = AE_OK;	
			continue;
		}

		out_values[i] = ACPI_UINT8_MAX;
		if (status == AE_NOT_FOUND) {
			continue;	
		}

		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "Failed %s on Device %4.4s, %s\n",
				  ACPI_CAST_PTR(char, method_names[i]),
				  acpi_ut_get_node_name(device_node),
				  acpi_format_exception(status)));
	}

	return_ACPI_STATUS(final_status);
}
