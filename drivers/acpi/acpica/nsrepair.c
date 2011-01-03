



#include <acpi/acpi.h>
#include "accommon.h"
#include "acnamesp.h"
#include "acpredef.h"

#define _COMPONENT          ACPI_NAMESPACE
ACPI_MODULE_NAME("nsrepair")


acpi_status
acpi_ns_repair_object(struct acpi_predefined_data *data,
		      u32 expected_btypes,
		      u32 package_index,
		      union acpi_operand_object **return_object_ptr)
{
	union acpi_operand_object *return_object = *return_object_ptr;
	union acpi_operand_object *new_object;
	acpi_size length;

	switch (return_object->common.type) {
	case ACPI_TYPE_BUFFER:

		

		if (!(expected_btypes & ACPI_RTYPE_STRING)) {
			return (AE_AML_OPERAND_TYPE);
		}

		
		length = 0;
		while ((length < return_object->buffer.length) &&
		       (return_object->buffer.pointer[length])) {
			length++;
		}

		

		new_object = acpi_ut_create_string_object(length);
		if (!new_object) {
			return (AE_NO_MEMORY);
		}

		
		ACPI_MEMCPY(new_object->string.pointer,
			    return_object->buffer.pointer, length);

		
		if (package_index != ACPI_NOT_PACKAGE_ELEMENT) {
			new_object->common.reference_count =
			    return_object->common.reference_count;

			if (return_object->common.reference_count > 1) {
				return_object->common.reference_count--;
			}

			ACPI_WARN_PREDEFINED((AE_INFO, data->pathname,
					      data->node_flags,
					      "Converted Buffer to expected String at index %u",
					      package_index));
		} else {
			ACPI_WARN_PREDEFINED((AE_INFO, data->pathname,
					      data->node_flags,
					      "Converted Buffer to expected String"));
		}

		

		acpi_ut_remove_reference(return_object);
		*return_object_ptr = new_object;
		data->flags |= ACPI_OBJECT_REPAIRED;
		return (AE_OK);

	default:
		break;
	}

	return (AE_AML_OPERAND_TYPE);
}



acpi_status
acpi_ns_repair_package_list(struct acpi_predefined_data *data,
			    union acpi_operand_object **obj_desc_ptr)
{
	union acpi_operand_object *pkg_obj_desc;

	
	pkg_obj_desc = acpi_ut_create_package_object(1);
	if (!pkg_obj_desc) {
		return (AE_NO_MEMORY);
	}

	pkg_obj_desc->package.elements[0] = *obj_desc_ptr;

	

	*obj_desc_ptr = pkg_obj_desc;
	data->flags |= ACPI_OBJECT_REPAIRED;

	ACPI_WARN_PREDEFINED((AE_INFO, data->pathname, data->node_flags,
			      "Incorrectly formed Package, attempting repair"));

	return (AE_OK);
}
