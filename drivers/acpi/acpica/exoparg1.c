




#include <acpi/acpi.h>
#include "accommon.h"
#include "acparser.h"
#include "acdispat.h"
#include "acinterp.h"
#include "amlcode.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_EXECUTER
ACPI_MODULE_NAME("exoparg1")



acpi_status acpi_ex_opcode_0A_0T_1R(struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_operand_object *return_desc = NULL;

	ACPI_FUNCTION_TRACE_STR(ex_opcode_0A_0T_1R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	

	switch (walk_state->opcode) {
	case AML_TIMER_OP:	

		

		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}
		return_desc->integer.value = acpi_os_get_timer();
		break;

	default:		

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		break;
	}

      cleanup:

	

	if ((ACPI_FAILURE(status)) || walk_state->result_obj) {
		acpi_ut_remove_reference(return_desc);
		walk_state->result_obj = NULL;
	} else {
		

		walk_state->result_obj = return_desc;
	}

	return_ACPI_STATUS(status);
}



acpi_status acpi_ex_opcode_1A_0T_0R(struct acpi_walk_state *walk_state)
{
	union acpi_operand_object **operand = &walk_state->operands[0];
	acpi_status status = AE_OK;

	ACPI_FUNCTION_TRACE_STR(ex_opcode_1A_0T_0R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	

	switch (walk_state->opcode) {
	case AML_RELEASE_OP:	

		status = acpi_ex_release_mutex(operand[0], walk_state);
		break;

	case AML_RESET_OP:	

		status = acpi_ex_system_reset_event(operand[0]);
		break;

	case AML_SIGNAL_OP:	

		status = acpi_ex_system_signal_event(operand[0]);
		break;

	case AML_SLEEP_OP:	

		status = acpi_ex_system_do_suspend(operand[0]->integer.value);
		break;

	case AML_STALL_OP:	

		status =
		    acpi_ex_system_do_stall((u32) operand[0]->integer.value);
		break;

	case AML_UNLOAD_OP:	

		status = acpi_ex_unload_table(operand[0]);
		break;

	default:		

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		break;
	}

	return_ACPI_STATUS(status);
}



acpi_status acpi_ex_opcode_1A_1T_0R(struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_operand_object **operand = &walk_state->operands[0];

	ACPI_FUNCTION_TRACE_STR(ex_opcode_1A_1T_0R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	

	switch (walk_state->opcode) {
	case AML_LOAD_OP:

		status = acpi_ex_load_op(operand[0], operand[1], walk_state);
		break;

	default:		

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		goto cleanup;
	}

      cleanup:

	return_ACPI_STATUS(status);
}



acpi_status acpi_ex_opcode_1A_1T_1R(struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_operand_object **operand = &walk_state->operands[0];
	union acpi_operand_object *return_desc = NULL;
	union acpi_operand_object *return_desc2 = NULL;
	u32 temp32;
	u32 i;
	acpi_integer power_of_ten;
	acpi_integer digit;

	ACPI_FUNCTION_TRACE_STR(ex_opcode_1A_1T_1R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	

	switch (walk_state->opcode) {
	case AML_BIT_NOT_OP:
	case AML_FIND_SET_LEFT_BIT_OP:
	case AML_FIND_SET_RIGHT_BIT_OP:
	case AML_FROM_BCD_OP:
	case AML_TO_BCD_OP:
	case AML_COND_REF_OF_OP:

		

		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		switch (walk_state->opcode) {
		case AML_BIT_NOT_OP:	

			return_desc->integer.value = ~operand[0]->integer.value;
			break;

		case AML_FIND_SET_LEFT_BIT_OP:	

			return_desc->integer.value = operand[0]->integer.value;

			
			for (temp32 = 0; return_desc->integer.value &&
			     temp32 < ACPI_INTEGER_BIT_SIZE; ++temp32) {
				return_desc->integer.value >>= 1;
			}

			return_desc->integer.value = temp32;
			break;

		case AML_FIND_SET_RIGHT_BIT_OP:	

			return_desc->integer.value = operand[0]->integer.value;

			
			for (temp32 = 0; return_desc->integer.value &&
			     temp32 < ACPI_INTEGER_BIT_SIZE; ++temp32) {
				return_desc->integer.value <<= 1;
			}

			

			return_desc->integer.value =
			    temp32 ==
			    0 ? 0 : (ACPI_INTEGER_BIT_SIZE + 1) - temp32;
			break;

		case AML_FROM_BCD_OP:	

			
			power_of_ten = 1;
			return_desc->integer.value = 0;
			digit = operand[0]->integer.value;

			

			for (i = 0;
			     (i < acpi_gbl_integer_nybble_width) && (digit > 0);
			     i++) {

				

				temp32 = ((u32) digit) & 0xF;

				

				if (temp32 > 9) {
					ACPI_ERROR((AE_INFO,
						    "BCD digit too large (not decimal): 0x%X",
						    temp32));

					status = AE_AML_NUMERIC_OVERFLOW;
					goto cleanup;
				}

				

				return_desc->integer.value +=
				    (((acpi_integer) temp32) * power_of_ten);

				

				digit >>= 4;

				

				power_of_ten *= 10;
			}
			break;

		case AML_TO_BCD_OP:	

			return_desc->integer.value = 0;
			digit = operand[0]->integer.value;

			

			for (i = 0;
			     (i < acpi_gbl_integer_nybble_width) && (digit > 0);
			     i++) {
				(void)acpi_ut_short_divide(digit, 10, &digit,
							   &temp32);

				
				return_desc->integer.value |=
				    (((acpi_integer) temp32) << ACPI_MUL_4(i));
			}

			

			if (digit > 0) {
				ACPI_ERROR((AE_INFO,
					    "Integer too large to convert to BCD: %8.8X%8.8X",
					    ACPI_FORMAT_UINT64(operand[0]->
							       integer.value)));
				status = AE_AML_NUMERIC_OVERFLOW;
				goto cleanup;
			}
			break;

		case AML_COND_REF_OF_OP:	

			
			if ((struct acpi_namespace_node *)operand[0] ==
			    acpi_gbl_root_node) {
				
				return_desc->integer.value = 0;
				goto cleanup;
			}

			

			status = acpi_ex_get_object_reference(operand[0],
							      &return_desc2,
							      walk_state);
			if (ACPI_FAILURE(status)) {
				goto cleanup;
			}

			status =
			    acpi_ex_store(return_desc2, operand[1], walk_state);
			acpi_ut_remove_reference(return_desc2);

			

			return_desc->integer.value = ACPI_INTEGER_MAX;
			goto cleanup;

		default:
			
			break;
		}
		break;

	case AML_STORE_OP:	

		
		status = acpi_ex_store(operand[0], operand[1], walk_state);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		

		if (!walk_state->result_obj) {
			
			walk_state->result_obj = operand[0];
			walk_state->operands[0] = NULL;	
		}
		return_ACPI_STATUS(status);

		
	case AML_COPY_OP:	

		status =
		    acpi_ut_copy_iobject_to_iobject(operand[0], &return_desc,
						    walk_state);
		break;

	case AML_TO_DECSTRING_OP:	

		status = acpi_ex_convert_to_string(operand[0], &return_desc,
						   ACPI_EXPLICIT_CONVERT_DECIMAL);
		if (return_desc == operand[0]) {

			
			acpi_ut_add_reference(return_desc);
		}
		break;

	case AML_TO_HEXSTRING_OP:	

		status = acpi_ex_convert_to_string(operand[0], &return_desc,
						   ACPI_EXPLICIT_CONVERT_HEX);
		if (return_desc == operand[0]) {

			
			acpi_ut_add_reference(return_desc);
		}
		break;

	case AML_TO_BUFFER_OP:	

		status = acpi_ex_convert_to_buffer(operand[0], &return_desc);
		if (return_desc == operand[0]) {

			
			acpi_ut_add_reference(return_desc);
		}
		break;

	case AML_TO_INTEGER_OP:	

		status = acpi_ex_convert_to_integer(operand[0], &return_desc,
						    ACPI_ANY_BASE);
		if (return_desc == operand[0]) {

			
			acpi_ut_add_reference(return_desc);
		}
		break;

	case AML_SHIFT_LEFT_BIT_OP:	
	case AML_SHIFT_RIGHT_BIT_OP:	

		

		ACPI_ERROR((AE_INFO,
			    "%s is obsolete and not implemented",
			    acpi_ps_get_opcode_name(walk_state->opcode)));
		status = AE_SUPPORT;
		goto cleanup;

	default:		

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		goto cleanup;
	}

	if (ACPI_SUCCESS(status)) {

		

		status = acpi_ex_store(return_desc, operand[1], walk_state);
	}

      cleanup:

	

	if (ACPI_FAILURE(status)) {
		acpi_ut_remove_reference(return_desc);
	}

	

	else if (!walk_state->result_obj) {
		walk_state->result_obj = return_desc;
	}

	return_ACPI_STATUS(status);
}



acpi_status acpi_ex_opcode_1A_0T_1R(struct acpi_walk_state *walk_state)
{
	union acpi_operand_object **operand = &walk_state->operands[0];
	union acpi_operand_object *temp_desc;
	union acpi_operand_object *return_desc = NULL;
	acpi_status status = AE_OK;
	u32 type;
	acpi_integer value;

	ACPI_FUNCTION_TRACE_STR(ex_opcode_1A_0T_1R,
				acpi_ps_get_opcode_name(walk_state->opcode));

	

	switch (walk_state->opcode) {
	case AML_LNOT_OP:	

		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		
		if (!operand[0]->integer.value) {
			return_desc->integer.value = ACPI_INTEGER_MAX;
		}
		break;

	case AML_DECREMENT_OP:	
	case AML_INCREMENT_OP:	

		
		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		
		temp_desc = operand[0];
		if (ACPI_GET_DESCRIPTOR_TYPE(temp_desc) ==
		    ACPI_DESC_TYPE_OPERAND) {

			

			acpi_ut_add_reference(temp_desc);
		}

		
		status =
		    acpi_ex_resolve_operands(AML_LNOT_OP, &temp_desc,
					     walk_state);
		if (ACPI_FAILURE(status)) {
			ACPI_EXCEPTION((AE_INFO, status,
					"While resolving operands for [%s]",
					acpi_ps_get_opcode_name(walk_state->
								opcode)));

			goto cleanup;
		}

		
		if (walk_state->opcode == AML_INCREMENT_OP) {
			return_desc->integer.value =
			    temp_desc->integer.value + 1;
		} else {
			return_desc->integer.value =
			    temp_desc->integer.value - 1;
		}

		

		acpi_ut_remove_reference(temp_desc);

		
		status = acpi_ex_store(return_desc, operand[0], walk_state);
		break;

	case AML_TYPE_OP:	

		

		

		status =
		    acpi_ex_resolve_multiple(walk_state, operand[0], &type,
					     NULL);
		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}

		

		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		return_desc->integer.value = type;
		break;

	case AML_SIZE_OF_OP:	

		

		

		status = acpi_ex_resolve_multiple(walk_state,
						  operand[0], &type,
						  &temp_desc);
		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}

		
		switch (type) {
		case ACPI_TYPE_INTEGER:
			value = acpi_gbl_integer_byte_width;
			break;

		case ACPI_TYPE_STRING:
			value = temp_desc->string.length;
			break;

		case ACPI_TYPE_BUFFER:

			

			status = acpi_ds_get_buffer_arguments(temp_desc);
			value = temp_desc->buffer.length;
			break;

		case ACPI_TYPE_PACKAGE:

			

			status = acpi_ds_get_package_arguments(temp_desc);
			value = temp_desc->package.count;
			break;

		default:
			ACPI_ERROR((AE_INFO,
				    "Operand must be Buffer/Integer/String/Package - found type %s",
				    acpi_ut_get_type_name(type)));
			status = AE_AML_OPERAND_TYPE;
			goto cleanup;
		}

		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}

		
		return_desc = acpi_ut_create_internal_object(ACPI_TYPE_INTEGER);
		if (!return_desc) {
			status = AE_NO_MEMORY;
			goto cleanup;
		}

		return_desc->integer.value = value;
		break;

	case AML_REF_OF_OP:	

		status =
		    acpi_ex_get_object_reference(operand[0], &return_desc,
						 walk_state);
		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}
		break;

	case AML_DEREF_OF_OP:	

		

		if (ACPI_GET_DESCRIPTOR_TYPE(operand[0]) ==
		    ACPI_DESC_TYPE_NAMED) {
			temp_desc =
			    acpi_ns_get_attached_object((struct
							 acpi_namespace_node *)
							operand[0]);
			if (temp_desc
			    && ((temp_desc->common.type == ACPI_TYPE_STRING)
				|| (temp_desc->common.type ==
				    ACPI_TYPE_LOCAL_REFERENCE))) {
				operand[0] = temp_desc;
				acpi_ut_add_reference(temp_desc);
			} else {
				status = AE_AML_OPERAND_TYPE;
				goto cleanup;
			}
		} else {
			switch ((operand[0])->common.type) {
			case ACPI_TYPE_LOCAL_REFERENCE:
				
				switch (operand[0]->reference.class) {
				case ACPI_REFCLASS_LOCAL:
				case ACPI_REFCLASS_ARG:

					

					status =
					    acpi_ds_method_data_get_value
					    (operand[0]->reference.class,
					     operand[0]->reference.value,
					     walk_state, &temp_desc);
					if (ACPI_FAILURE(status)) {
						goto cleanup;
					}

					
					acpi_ut_remove_reference(operand[0]);
					operand[0] = temp_desc;
					break;

				case ACPI_REFCLASS_REFOF:

					

					temp_desc =
					    operand[0]->reference.object;
					acpi_ut_remove_reference(operand[0]);
					operand[0] = temp_desc;
					break;

				default:

					
					break;
				}
				break;

			case ACPI_TYPE_STRING:
				break;

			default:
				status = AE_AML_OPERAND_TYPE;
				goto cleanup;
			}
		}

		if (ACPI_GET_DESCRIPTOR_TYPE(operand[0]) !=
		    ACPI_DESC_TYPE_NAMED) {
			if ((operand[0])->common.type == ACPI_TYPE_STRING) {
				
				status =
				    acpi_ns_get_node(walk_state->scope_info->
						     scope.node,
						     operand[0]->string.pointer,
						     ACPI_NS_SEARCH_PARENT,
						     ACPI_CAST_INDIRECT_PTR
						     (struct
						      acpi_namespace_node,
						      &return_desc));
				if (ACPI_FAILURE(status)) {
					goto cleanup;
				}

				status =
				    acpi_ex_resolve_node_to_value
				    (ACPI_CAST_INDIRECT_PTR
				     (struct acpi_namespace_node, &return_desc),
				     walk_state);
				goto cleanup;
			}
		}

		

		if (ACPI_GET_DESCRIPTOR_TYPE(operand[0]) ==
		    ACPI_DESC_TYPE_NAMED) {
			
			return_desc = acpi_ns_get_attached_object((struct
								   acpi_namespace_node
								   *)
								  operand[0]);
			acpi_ut_add_reference(return_desc);
		} else {
			
			switch (operand[0]->reference.class) {
			case ACPI_REFCLASS_INDEX:

				
				switch (operand[0]->reference.target_type) {
				case ACPI_TYPE_BUFFER_FIELD:

					temp_desc =
					    operand[0]->reference.object;

					
					return_desc =
					    acpi_ut_create_internal_object
					    (ACPI_TYPE_INTEGER);
					if (!return_desc) {
						status = AE_NO_MEMORY;
						goto cleanup;
					}

					
					return_desc->integer.value =
					    temp_desc->buffer.
					    pointer[operand[0]->reference.
						    value];
					break;

				case ACPI_TYPE_PACKAGE:

					
					return_desc =
					    *(operand[0]->reference.where);
					if (return_desc) {
						acpi_ut_add_reference
						    (return_desc);
					}
					break;

				default:

					ACPI_ERROR((AE_INFO,
						    "Unknown Index TargetType %X in reference object %p",
						    operand[0]->reference.
						    target_type, operand[0]));
					status = AE_AML_OPERAND_TYPE;
					goto cleanup;
				}
				break;

			case ACPI_REFCLASS_REFOF:

				return_desc = operand[0]->reference.object;

				if (ACPI_GET_DESCRIPTOR_TYPE(return_desc) ==
				    ACPI_DESC_TYPE_NAMED) {
					return_desc =
					    acpi_ns_get_attached_object((struct
									 acpi_namespace_node
									 *)
									return_desc);
				}

				

				acpi_ut_add_reference(return_desc);
				break;

			default:
				ACPI_ERROR((AE_INFO,
					    "Unknown class in reference(%p) - %2.2X",
					    operand[0],
					    operand[0]->reference.class));

				status = AE_TYPE;
				goto cleanup;
			}
		}
		break;

	default:

		ACPI_ERROR((AE_INFO, "Unknown AML opcode %X",
			    walk_state->opcode));
		status = AE_AML_BAD_OPCODE;
		goto cleanup;
	}

      cleanup:

	

	if (ACPI_FAILURE(status)) {
		acpi_ut_remove_reference(return_desc);
	}

	

	else {
		walk_state->result_obj = return_desc;
	}

	return_ACPI_STATUS(status);
}
