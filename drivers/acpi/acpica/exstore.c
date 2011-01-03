




#include <acpi/acpi.h>
#include "accommon.h"
#include "acdispat.h"
#include "acinterp.h"
#include "amlcode.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_EXECUTER
ACPI_MODULE_NAME("exstore")


static void
acpi_ex_do_debug_object(union acpi_operand_object *source_desc,
			u32 level, u32 index);

static acpi_status
acpi_ex_store_object_to_index(union acpi_operand_object *val_desc,
			      union acpi_operand_object *dest_desc,
			      struct acpi_walk_state *walk_state);



static void
acpi_ex_do_debug_object(union acpi_operand_object *source_desc,
			u32 level, u32 index)
{
	u32 i;

	ACPI_FUNCTION_TRACE_PTR(ex_do_debug_object, source_desc);

	

	if (!((level > 0) && index == 0)) {
		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "[ACPI Debug] %*s",
				      level, " "));
	}

	

	if (index > 0) {
		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT,
				      "(%.2u) ", index - 1));
	}

	if (!source_desc) {
		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "[Null Object]\n"));
		return_VOID;
	}

	if (ACPI_GET_DESCRIPTOR_TYPE(source_desc) == ACPI_DESC_TYPE_OPERAND) {
		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "%s ",
				      acpi_ut_get_object_type_name
				      (source_desc)));

		if (!acpi_ut_valid_internal_object(source_desc)) {
			ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT,
					      "%p, Invalid Internal Object!\n",
					      source_desc));
			return_VOID;
		}
	} else if (ACPI_GET_DESCRIPTOR_TYPE(source_desc) ==
		   ACPI_DESC_TYPE_NAMED) {
		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "%s: %p\n",
				      acpi_ut_get_type_name(((struct
							      acpi_namespace_node
							      *)source_desc)->
							    type),
				      source_desc));
		return_VOID;
	} else {
		return_VOID;
	}

	

	switch (source_desc->common.type) {
	case ACPI_TYPE_INTEGER:

		

		if (acpi_gbl_integer_byte_width == 4) {
			ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "0x%8.8X\n",
					      (u32) source_desc->integer.
					      value));
		} else {
			ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT,
					      "0x%8.8X%8.8X\n",
					      ACPI_FORMAT_UINT64(source_desc->
								 integer.
								 value)));
		}
		break;

	case ACPI_TYPE_BUFFER:

		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "[0x%.2X]\n",
				      (u32) source_desc->buffer.length));
		ACPI_DUMP_BUFFER(source_desc->buffer.pointer,
				 (source_desc->buffer.length <
				  256) ? source_desc->buffer.length : 256);
		break;

	case ACPI_TYPE_STRING:

		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "[0x%.2X] \"%s\"\n",
				      source_desc->string.length,
				      source_desc->string.pointer));
		break;

	case ACPI_TYPE_PACKAGE:

		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT,
				      "[Contains 0x%.2X Elements]\n",
				      source_desc->package.count));

		

		for (i = 0; i < source_desc->package.count; i++) {
			acpi_ex_do_debug_object(source_desc->package.
						elements[i], level + 4, i + 1);
		}
		break;

	case ACPI_TYPE_LOCAL_REFERENCE:

		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "[%s] ",
				      acpi_ut_get_reference_name(source_desc)));

		

		switch (source_desc->reference.class) {
		case ACPI_REFCLASS_INDEX:

			ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "0x%X\n",
					      source_desc->reference.value));
			break;

		case ACPI_REFCLASS_TABLE:

			

			ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT,
					      "Table Index 0x%X\n",
					      source_desc->reference.value));
			return;

		default:
			break;
		}

		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "  "));

		

		if (source_desc->reference.node) {
			if (ACPI_GET_DESCRIPTOR_TYPE
			    (source_desc->reference.node) !=
			    ACPI_DESC_TYPE_NAMED) {
				ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT,
						      " %p - Not a valid namespace node\n",
						      source_desc->reference.
						      node));
			} else {
				ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT,
						      "Node %p [%4.4s] ",
						      source_desc->reference.
						      node,
						      (source_desc->reference.
						       node)->name.ascii));

				switch ((source_desc->reference.node)->type) {

					

				case ACPI_TYPE_DEVICE:
					acpi_os_printf("Device\n");
					break;

				case ACPI_TYPE_THERMAL:
					acpi_os_printf("Thermal Zone\n");
					break;

				default:
					acpi_ex_do_debug_object((source_desc->
								 reference.
								 node)->object,
								level + 4, 0);
					break;
				}
			}
		} else if (source_desc->reference.object) {
			if (ACPI_GET_DESCRIPTOR_TYPE
			    (source_desc->reference.object) ==
			    ACPI_DESC_TYPE_NAMED) {
				acpi_ex_do_debug_object(((struct
							  acpi_namespace_node *)
							 source_desc->reference.
							 object)->object,
							level + 4, 0);
			} else {
				acpi_ex_do_debug_object(source_desc->reference.
							object, level + 4, 0);
			}
		}
		break;

	default:

		ACPI_DEBUG_PRINT_RAW((ACPI_DB_DEBUG_OBJECT, "%p\n",
				      source_desc));
		break;
	}

	ACPI_DEBUG_PRINT_RAW((ACPI_DB_EXEC, "\n"));
	return_VOID;
}



acpi_status
acpi_ex_store(union acpi_operand_object *source_desc,
	      union acpi_operand_object *dest_desc,
	      struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_operand_object *ref_desc = dest_desc;

	ACPI_FUNCTION_TRACE_PTR(ex_store, dest_desc);

	

	if (!source_desc || !dest_desc) {
		ACPI_ERROR((AE_INFO, "Null parameter"));
		return_ACPI_STATUS(AE_AML_NO_OPERAND);
	}

	

	if (ACPI_GET_DESCRIPTOR_TYPE(dest_desc) == ACPI_DESC_TYPE_NAMED) {
		
		status = acpi_ex_store_object_to_node(source_desc,
						      (struct
						       acpi_namespace_node *)
						      dest_desc, walk_state,
						      ACPI_IMPLICIT_CONVERSION);

		return_ACPI_STATUS(status);
	}

	

	switch (dest_desc->common.type) {
	case ACPI_TYPE_LOCAL_REFERENCE:
		break;

	case ACPI_TYPE_INTEGER:

		

		if (dest_desc->common.flags & AOPOBJ_AML_CONSTANT) {
			return_ACPI_STATUS(AE_OK);
		}

		

	default:

		

		ACPI_ERROR((AE_INFO,
			    "Target is not a Reference or Constant object - %s [%p]",
			    acpi_ut_get_object_type_name(dest_desc),
			    dest_desc));

		return_ACPI_STATUS(AE_AML_OPERAND_TYPE);
	}

	
	switch (ref_desc->reference.class) {
	case ACPI_REFCLASS_REFOF:

		

		status = acpi_ex_store_object_to_node(source_desc,
						      ref_desc->reference.
						      object, walk_state,
						      ACPI_IMPLICIT_CONVERSION);
		break;

	case ACPI_REFCLASS_INDEX:

		

		status =
		    acpi_ex_store_object_to_index(source_desc, ref_desc,
						  walk_state);
		break;

	case ACPI_REFCLASS_LOCAL:
	case ACPI_REFCLASS_ARG:

		

		status =
		    acpi_ds_store_object_to_local(ref_desc->reference.class,
						  ref_desc->reference.value,
						  source_desc, walk_state);
		break;

	case ACPI_REFCLASS_DEBUG:

		
		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "**** Write to Debug Object: Object %p %s ****:\n\n",
				  source_desc,
				  acpi_ut_get_object_type_name(source_desc)));

		acpi_ex_do_debug_object(source_desc, 0, 0);
		break;

	default:

		ACPI_ERROR((AE_INFO, "Unknown Reference Class %2.2X",
			    ref_desc->reference.class));
		ACPI_DUMP_ENTRY(ref_desc, ACPI_LV_INFO);

		status = AE_AML_INTERNAL;
		break;
	}

	return_ACPI_STATUS(status);
}



static acpi_status
acpi_ex_store_object_to_index(union acpi_operand_object *source_desc,
			      union acpi_operand_object *index_desc,
			      struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	union acpi_operand_object *obj_desc;
	union acpi_operand_object *new_desc;
	u8 value = 0;
	u32 i;

	ACPI_FUNCTION_TRACE(ex_store_object_to_index);

	
	switch (index_desc->reference.target_type) {
	case ACPI_TYPE_PACKAGE:
		
		obj_desc = *(index_desc->reference.where);

		if (source_desc->common.type == ACPI_TYPE_LOCAL_REFERENCE &&
		    source_desc->reference.class == ACPI_REFCLASS_TABLE) {

			

			acpi_ut_add_reference(source_desc);
			new_desc = source_desc;
		} else {
			

			status =
			    acpi_ut_copy_iobject_to_iobject(source_desc,
							    &new_desc,
							    walk_state);
			if (ACPI_FAILURE(status)) {
				return_ACPI_STATUS(status);
			}
		}

		if (obj_desc) {

			

			for (i = 0; i < ((union acpi_operand_object *)
					 index_desc->reference.object)->common.
			     reference_count; i++) {
				acpi_ut_remove_reference(obj_desc);
			}
		}

		*(index_desc->reference.where) = new_desc;

		

		for (i = 1; i < ((union acpi_operand_object *)
				 index_desc->reference.object)->common.
		     reference_count; i++) {
			acpi_ut_add_reference(new_desc);
		}

		break;

	case ACPI_TYPE_BUFFER_FIELD:

		

		
		obj_desc = index_desc->reference.object;
		if ((obj_desc->common.type != ACPI_TYPE_BUFFER) &&
		    (obj_desc->common.type != ACPI_TYPE_STRING)) {
			return_ACPI_STATUS(AE_AML_OPERAND_TYPE);
		}

		
		switch (source_desc->common.type) {
		case ACPI_TYPE_INTEGER:

			

			value = (u8) (source_desc->integer.value);
			break;

		case ACPI_TYPE_BUFFER:
		case ACPI_TYPE_STRING:

			

			value = source_desc->buffer.pointer[0];
			break;

		default:

			

			ACPI_ERROR((AE_INFO,
				    "Source must be Integer/Buffer/String type, not %s",
				    acpi_ut_get_object_type_name(source_desc)));
			return_ACPI_STATUS(AE_AML_OPERAND_TYPE);
		}

		

		obj_desc->buffer.pointer[index_desc->reference.value] = value;
		break;

	default:
		ACPI_ERROR((AE_INFO, "Target is not a Package or BufferField"));
		status = AE_AML_OPERAND_TYPE;
		break;
	}

	return_ACPI_STATUS(status);
}



acpi_status
acpi_ex_store_object_to_node(union acpi_operand_object *source_desc,
			     struct acpi_namespace_node *node,
			     struct acpi_walk_state *walk_state,
			     u8 implicit_conversion)
{
	acpi_status status = AE_OK;
	union acpi_operand_object *target_desc;
	union acpi_operand_object *new_desc;
	acpi_object_type target_type;

	ACPI_FUNCTION_TRACE_PTR(ex_store_object_to_node, source_desc);

	

	target_type = acpi_ns_get_type(node);
	target_desc = acpi_ns_get_attached_object(node);

	ACPI_DEBUG_PRINT((ACPI_DB_EXEC, "Storing %p(%s) into node %p(%s)\n",
			  source_desc,
			  acpi_ut_get_object_type_name(source_desc), node,
			  acpi_ut_get_type_name(target_type)));

	
	status = acpi_ex_resolve_object(&source_desc, target_type, walk_state);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	if ((!implicit_conversion) ||
	    ((walk_state->opcode == AML_COPY_OP) &&
	     (target_type != ACPI_TYPE_LOCAL_REGION_FIELD) &&
	     (target_type != ACPI_TYPE_LOCAL_BANK_FIELD) &&
	     (target_type != ACPI_TYPE_LOCAL_INDEX_FIELD))) {
		
		target_type = ACPI_TYPE_ANY;
	}

	

	switch (target_type) {
	case ACPI_TYPE_BUFFER_FIELD:
	case ACPI_TYPE_LOCAL_REGION_FIELD:
	case ACPI_TYPE_LOCAL_BANK_FIELD:
	case ACPI_TYPE_LOCAL_INDEX_FIELD:

		

		status = acpi_ex_write_data_to_field(source_desc, target_desc,
						     &walk_state->result_obj);
		break;

	case ACPI_TYPE_INTEGER:
	case ACPI_TYPE_STRING:
	case ACPI_TYPE_BUFFER:

		
		status =
		    acpi_ex_store_object_to_object(source_desc, target_desc,
						   &new_desc, walk_state);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		if (new_desc != target_desc) {
			
			status =
			    acpi_ns_attach_object(node, new_desc,
						  new_desc->common.type);

			ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
					  "Store %s into %s via Convert/Attach\n",
					  acpi_ut_get_object_type_name
					  (source_desc),
					  acpi_ut_get_object_type_name
					  (new_desc)));
		}
		break;

	default:

		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "Storing %s (%p) directly into node (%p) with no implicit conversion\n",
				  acpi_ut_get_object_type_name(source_desc),
				  source_desc, node));

		

		status = acpi_ns_attach_object(node, source_desc,
					       source_desc->common.type);
		break;
	}

	return_ACPI_STATUS(status);
}
