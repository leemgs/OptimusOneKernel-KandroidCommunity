



#include <acpi/acpi.h>
#include "accommon.h"
#include "acdispat.h"
#include "acinterp.h"

#define _COMPONENT          ACPI_EXECUTER
ACPI_MODULE_NAME("exfield")


acpi_status
acpi_ex_read_data_from_field(struct acpi_walk_state *walk_state,
			     union acpi_operand_object *obj_desc,
			     union acpi_operand_object **ret_buffer_desc)
{
	acpi_status status;
	union acpi_operand_object *buffer_desc;
	acpi_size length;
	void *buffer;
	u32 function;

	ACPI_FUNCTION_TRACE_PTR(ex_read_data_from_field, obj_desc);

	

	if (!obj_desc) {
		return_ACPI_STATUS(AE_AML_NO_OPERAND);
	}
	if (!ret_buffer_desc) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	if (obj_desc->common.type == ACPI_TYPE_BUFFER_FIELD) {
		
		if (!(obj_desc->common.flags & AOPOBJ_DATA_VALID)) {
			status = acpi_ds_get_buffer_field_arguments(obj_desc);
			if (ACPI_FAILURE(status)) {
				return_ACPI_STATUS(status);
			}
		}
	} else if ((obj_desc->common.type == ACPI_TYPE_LOCAL_REGION_FIELD) &&
		   (obj_desc->field.region_obj->region.space_id ==
		    ACPI_ADR_SPACE_SMBUS
		    || obj_desc->field.region_obj->region.space_id ==
		    ACPI_ADR_SPACE_IPMI)) {
		
		if (obj_desc->field.region_obj->region.space_id ==
		    ACPI_ADR_SPACE_SMBUS) {
			length = ACPI_SMBUS_BUFFER_SIZE;
			function =
			    ACPI_READ | (obj_desc->field.attribute << 16);
		} else {	

			length = ACPI_IPMI_BUFFER_SIZE;
			function = ACPI_READ;
		}

		buffer_desc = acpi_ut_create_buffer_object(length);
		if (!buffer_desc) {
			return_ACPI_STATUS(AE_NO_MEMORY);
		}

		

		acpi_ex_acquire_global_lock(obj_desc->common_field.field_flags);

		

		status = acpi_ex_access_region(obj_desc, 0,
					       ACPI_CAST_PTR(acpi_integer,
							     buffer_desc->
							     buffer.pointer),
					       function);
		acpi_ex_release_global_lock(obj_desc->common_field.field_flags);
		goto exit;
	}

	
	length =
	    (acpi_size) ACPI_ROUND_BITS_UP_TO_BYTES(obj_desc->field.bit_length);
	if (length > acpi_gbl_integer_byte_width) {

		

		buffer_desc = acpi_ut_create_buffer_object(length);
		if (!buffer_desc) {
			return_ACPI_STATUS(AE_NO_MEMORY);
		}
		buffer = buffer_desc->buffer.pointer;
	} else {
		

		buffer_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!buffer_desc) {
			return_ACPI_STATUS(AE_NO_MEMORY);
		}

		length = acpi_gbl_integer_byte_width;
		buffer_desc->integer.value = 0;
		buffer = &buffer_desc->integer.value;
	}

	ACPI_DEBUG_PRINT((ACPI_DB_BFIELD,
			  "FieldRead [TO]:   Obj %p, Type %X, Buf %p, ByteLen %X\n",
			  obj_desc, obj_desc->common.type, buffer,
			  (u32) length));
	ACPI_DEBUG_PRINT((ACPI_DB_BFIELD,
			  "FieldRead [FROM]: BitLen %X, BitOff %X, ByteOff %X\n",
			  obj_desc->common_field.bit_length,
			  obj_desc->common_field.start_field_bit_offset,
			  obj_desc->common_field.base_byte_offset));

	

	acpi_ex_acquire_global_lock(obj_desc->common_field.field_flags);

	

	status = acpi_ex_extract_from_field(obj_desc, buffer, (u32) length);
	acpi_ex_release_global_lock(obj_desc->common_field.field_flags);

      exit:
	if (ACPI_FAILURE(status)) {
		acpi_ut_remove_reference(buffer_desc);
	} else {
		*ret_buffer_desc = buffer_desc;
	}

	return_ACPI_STATUS(status);
}



acpi_status
acpi_ex_write_data_to_field(union acpi_operand_object *source_desc,
			    union acpi_operand_object *obj_desc,
			    union acpi_operand_object **result_desc)
{
	acpi_status status;
	u32 length;
	void *buffer;
	union acpi_operand_object *buffer_desc;
	u32 function;

	ACPI_FUNCTION_TRACE_PTR(ex_write_data_to_field, obj_desc);

	

	if (!source_desc || !obj_desc) {
		return_ACPI_STATUS(AE_AML_NO_OPERAND);
	}

	if (obj_desc->common.type == ACPI_TYPE_BUFFER_FIELD) {
		
		if (!(obj_desc->common.flags & AOPOBJ_DATA_VALID)) {
			status = acpi_ds_get_buffer_field_arguments(obj_desc);
			if (ACPI_FAILURE(status)) {
				return_ACPI_STATUS(status);
			}
		}
	} else if ((obj_desc->common.type == ACPI_TYPE_LOCAL_REGION_FIELD) &&
		   (obj_desc->field.region_obj->region.space_id ==
		    ACPI_ADR_SPACE_SMBUS
		    || obj_desc->field.region_obj->region.space_id ==
		    ACPI_ADR_SPACE_IPMI)) {
		
		if (source_desc->common.type != ACPI_TYPE_BUFFER) {
			ACPI_ERROR((AE_INFO,
				    "SMBus or IPMI write requires Buffer, found type %s",
				    acpi_ut_get_object_type_name(source_desc)));

			return_ACPI_STATUS(AE_AML_OPERAND_TYPE);
		}

		if (obj_desc->field.region_obj->region.space_id ==
		    ACPI_ADR_SPACE_SMBUS) {
			length = ACPI_SMBUS_BUFFER_SIZE;
			function =
			    ACPI_WRITE | (obj_desc->field.attribute << 16);
		} else {	

			length = ACPI_IPMI_BUFFER_SIZE;
			function = ACPI_WRITE;
		}

		if (source_desc->buffer.length < length) {
			ACPI_ERROR((AE_INFO,
				    "SMBus or IPMI write requires Buffer of length %X, found length %X",
				    length, source_desc->buffer.length));

			return_ACPI_STATUS(AE_AML_BUFFER_LIMIT);
		}

		

		buffer_desc = acpi_ut_create_buffer_object(length);
		if (!buffer_desc) {
			return_ACPI_STATUS(AE_NO_MEMORY);
		}

		buffer = buffer_desc->buffer.pointer;
		ACPI_MEMCPY(buffer, source_desc->buffer.pointer, length);

		

		acpi_ex_acquire_global_lock(obj_desc->common_field.field_flags);

		
		status = acpi_ex_access_region(obj_desc, 0,
					       (acpi_integer *) buffer,
					       function);
		acpi_ex_release_global_lock(obj_desc->common_field.field_flags);

		*result_desc = buffer_desc;
		return_ACPI_STATUS(status);
	}

	

	switch (source_desc->common.type) {
	case ACPI_TYPE_INTEGER:
		buffer = &source_desc->integer.value;
		length = sizeof(source_desc->integer.value);
		break;

	case ACPI_TYPE_BUFFER:
		buffer = source_desc->buffer.pointer;
		length = source_desc->buffer.length;
		break;

	case ACPI_TYPE_STRING:
		buffer = source_desc->string.pointer;
		length = source_desc->string.length;
		break;

	default:
		return_ACPI_STATUS(AE_AML_OPERAND_TYPE);
	}

	ACPI_DEBUG_PRINT((ACPI_DB_BFIELD,
			  "FieldWrite [FROM]: Obj %p (%s:%X), Buf %p, ByteLen %X\n",
			  source_desc,
			  acpi_ut_get_type_name(source_desc->common.type),
			  source_desc->common.type, buffer, length));

	ACPI_DEBUG_PRINT((ACPI_DB_BFIELD,
			  "FieldWrite [TO]:   Obj %p (%s:%X), BitLen %X, BitOff %X, ByteOff %X\n",
			  obj_desc,
			  acpi_ut_get_type_name(obj_desc->common.type),
			  obj_desc->common.type,
			  obj_desc->common_field.bit_length,
			  obj_desc->common_field.start_field_bit_offset,
			  obj_desc->common_field.base_byte_offset));

	

	acpi_ex_acquire_global_lock(obj_desc->common_field.field_flags);

	

	status = acpi_ex_insert_into_field(obj_desc, buffer, length);
	acpi_ex_release_global_lock(obj_desc->common_field.field_flags);

	return_ACPI_STATUS(status);
}
