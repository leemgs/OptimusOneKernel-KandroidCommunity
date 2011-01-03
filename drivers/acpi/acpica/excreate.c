



#include <acpi/acpi.h>
#include "accommon.h"
#include "acinterp.h"
#include "amlcode.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_EXECUTER
ACPI_MODULE_NAME("excreate")
#ifndef ACPI_NO_METHOD_EXECUTION

acpi_status acpi_ex_create_alias(struct acpi_walk_state *walk_state)
{
	struct acpi_namespace_node *target_node;
	struct acpi_namespace_node *alias_node;
	acpi_status status = AE_OK;

	ACPI_FUNCTION_TRACE(ex_create_alias);

	

	alias_node = (struct acpi_namespace_node *)walk_state->operands[0];
	target_node = (struct acpi_namespace_node *)walk_state->operands[1];

	if ((target_node->type == ACPI_TYPE_LOCAL_ALIAS) ||
	    (target_node->type == ACPI_TYPE_LOCAL_METHOD_ALIAS)) {
		
		target_node =
		    ACPI_CAST_PTR(struct acpi_namespace_node,
				  target_node->object);
	}

	
	switch (target_node->type) {

		

	case ACPI_TYPE_INTEGER:
	case ACPI_TYPE_STRING:
	case ACPI_TYPE_BUFFER:
	case ACPI_TYPE_PACKAGE:
	case ACPI_TYPE_BUFFER_FIELD:

		
	case ACPI_TYPE_DEVICE:
	case ACPI_TYPE_POWER:
	case ACPI_TYPE_PROCESSOR:
	case ACPI_TYPE_THERMAL:
	case ACPI_TYPE_LOCAL_SCOPE:

		
		alias_node->type = ACPI_TYPE_LOCAL_ALIAS;
		alias_node->object =
		    ACPI_CAST_PTR(union acpi_operand_object, target_node);
		break;

	case ACPI_TYPE_METHOD:

		
		alias_node->type = ACPI_TYPE_LOCAL_METHOD_ALIAS;
		alias_node->object =
		    ACPI_CAST_PTR(union acpi_operand_object, target_node);
		break;

	default:

		

		
		status = acpi_ns_attach_object(alias_node,
					       acpi_ns_get_attached_object
					       (target_node),
					       target_node->type);
		break;
	}

	

	return_ACPI_STATUS(status);
}



acpi_status acpi_ex_create_event(struct acpi_walk_state *walk_state)
{
	acpi_status status;
	union acpi_operand_object *obj_desc;

	ACPI_FUNCTION_TRACE(ex_create_event);

	obj_desc = acpi_ut_create_internal_object(ACPI_TYPE_EVENT);
	if (!obj_desc) {
		status = AE_NO_MEMORY;
		goto cleanup;
	}

	
	status = acpi_os_create_semaphore(ACPI_NO_UNIT_LIMIT, 0,
					  &obj_desc->event.os_semaphore);
	if (ACPI_FAILURE(status)) {
		goto cleanup;
	}

	

	status =
	    acpi_ns_attach_object((struct acpi_namespace_node *)walk_state->
				  operands[0], obj_desc, ACPI_TYPE_EVENT);

      cleanup:
	
	acpi_ut_remove_reference(obj_desc);
	return_ACPI_STATUS(status);
}



acpi_status acpi_ex_create_mutex(struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_operand_object *obj_desc;

	ACPI_FUNCTION_TRACE_PTR(ex_create_mutex, ACPI_WALK_OPERANDS);

	

	obj_desc = acpi_ut_create_internal_object(ACPI_TYPE_MUTEX);
	if (!obj_desc) {
		status = AE_NO_MEMORY;
		goto cleanup;
	}

	

	status = acpi_os_create_mutex(&obj_desc->mutex.os_mutex);
	if (ACPI_FAILURE(status)) {
		goto cleanup;
	}

	

	obj_desc->mutex.sync_level =
	    (u8) walk_state->operands[1]->integer.value;
	obj_desc->mutex.node =
	    (struct acpi_namespace_node *)walk_state->operands[0];

	status =
	    acpi_ns_attach_object(obj_desc->mutex.node, obj_desc,
				  ACPI_TYPE_MUTEX);

      cleanup:
	
	acpi_ut_remove_reference(obj_desc);
	return_ACPI_STATUS(status);
}



acpi_status
acpi_ex_create_region(u8 * aml_start,
		      u32 aml_length,
		      u8 region_space, struct acpi_walk_state *walk_state)
{
	acpi_status status;
	union acpi_operand_object *obj_desc;
	struct acpi_namespace_node *node;
	union acpi_operand_object *region_obj2;

	ACPI_FUNCTION_TRACE(ex_create_region);

	

	node = walk_state->op->common.node;

	
	if (acpi_ns_get_attached_object(node)) {
		return_ACPI_STATUS(AE_OK);
	}

	
	if ((region_space >= ACPI_NUM_PREDEFINED_REGIONS) &&
	    (region_space < ACPI_USER_REGION_BEGIN)) {
		ACPI_ERROR((AE_INFO, "Invalid AddressSpace type %X",
			    region_space));
		return_ACPI_STATUS(AE_AML_INVALID_SPACE_ID);
	}

	ACPI_DEBUG_PRINT((ACPI_DB_LOAD, "Region Type - %s (%X)\n",
			  acpi_ut_get_region_name(region_space), region_space));

	

	obj_desc = acpi_ut_create_internal_object(ACPI_TYPE_REGION);
	if (!obj_desc) {
		status = AE_NO_MEMORY;
		goto cleanup;
	}

	
	region_obj2 = obj_desc->common.next_object;
	region_obj2->extra.aml_start = aml_start;
	region_obj2->extra.aml_length = aml_length;

	

	obj_desc->region.space_id = region_space;
	obj_desc->region.address = 0;
	obj_desc->region.length = 0;
	obj_desc->region.node = node;

	

	status = acpi_ns_attach_object(node, obj_desc, ACPI_TYPE_REGION);

      cleanup:

	

	acpi_ut_remove_reference(obj_desc);
	return_ACPI_STATUS(status);
}



acpi_status acpi_ex_create_processor(struct acpi_walk_state *walk_state)
{
	union acpi_operand_object **operand = &walk_state->operands[0];
	union acpi_operand_object *obj_desc;
	acpi_status status;

	ACPI_FUNCTION_TRACE_PTR(ex_create_processor, walk_state);

	

	obj_desc = acpi_ut_create_internal_object(ACPI_TYPE_PROCESSOR);
	if (!obj_desc) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	

	obj_desc->processor.proc_id = (u8) operand[1]->integer.value;
	obj_desc->processor.length = (u8) operand[3]->integer.value;
	obj_desc->processor.address =
	    (acpi_io_address) operand[2]->integer.value;

	

	status = acpi_ns_attach_object((struct acpi_namespace_node *)operand[0],
				       obj_desc, ACPI_TYPE_PROCESSOR);

	

	acpi_ut_remove_reference(obj_desc);
	return_ACPI_STATUS(status);
}



acpi_status acpi_ex_create_power_resource(struct acpi_walk_state *walk_state)
{
	union acpi_operand_object **operand = &walk_state->operands[0];
	acpi_status status;
	union acpi_operand_object *obj_desc;

	ACPI_FUNCTION_TRACE_PTR(ex_create_power_resource, walk_state);

	

	obj_desc = acpi_ut_create_internal_object(ACPI_TYPE_POWER);
	if (!obj_desc) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	

	obj_desc->power_resource.system_level = (u8) operand[1]->integer.value;
	obj_desc->power_resource.resource_order =
	    (u16) operand[2]->integer.value;

	

	status = acpi_ns_attach_object((struct acpi_namespace_node *)operand[0],
				       obj_desc, ACPI_TYPE_POWER);

	

	acpi_ut_remove_reference(obj_desc);
	return_ACPI_STATUS(status);
}
#endif



acpi_status
acpi_ex_create_method(u8 * aml_start,
		      u32 aml_length, struct acpi_walk_state *walk_state)
{
	union acpi_operand_object **operand = &walk_state->operands[0];
	union acpi_operand_object *obj_desc;
	acpi_status status;
	u8 method_flags;

	ACPI_FUNCTION_TRACE_PTR(ex_create_method, walk_state);

	

	obj_desc = acpi_ut_create_internal_object(ACPI_TYPE_METHOD);
	if (!obj_desc) {
		status = AE_NO_MEMORY;
		goto exit;
	}

	

	obj_desc->method.aml_start = aml_start;
	obj_desc->method.aml_length = aml_length;

	
	method_flags = (u8) operand[1]->integer.value;

	obj_desc->method.method_flags =
	    (u8) (method_flags & ~AML_METHOD_ARG_COUNT);
	obj_desc->method.param_count =
	    (u8) (method_flags & AML_METHOD_ARG_COUNT);

	
	if (method_flags & AML_METHOD_SERIALIZED) {
		
		obj_desc->method.sync_level = (u8)
		    ((method_flags & AML_METHOD_SYNC_LEVEL) >> 4);
	}

	

	status = acpi_ns_attach_object((struct acpi_namespace_node *)operand[0],
				       obj_desc, ACPI_TYPE_METHOD);

	

	acpi_ut_remove_reference(obj_desc);

      exit:
	

	acpi_ut_remove_reference(operand[1]);
	return_ACPI_STATUS(status);
}
