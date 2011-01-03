



#include <acpi/acpi.h>
#include "accommon.h"
#include "acparser.h"
#include "acinterp.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_NAMESPACE
ACPI_MODULE_NAME("nseval")


static void
acpi_ns_exec_module_code(union acpi_operand_object *method_obj,
			 struct acpi_evaluate_info *info);



acpi_status acpi_ns_evaluate(struct acpi_evaluate_info * info)
{
	acpi_status status;
	struct acpi_namespace_node *node;

	ACPI_FUNCTION_TRACE(ns_evaluate);

	if (!info) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	

	info->return_object = NULL;
	info->param_count = 0;

	
	status = acpi_ns_get_node(info->prefix_node, info->pathname,
				  ACPI_NS_NO_UPSEARCH, &info->resolved_node);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	
	if (acpi_ns_get_type(info->resolved_node) ==
	    ACPI_TYPE_LOCAL_METHOD_ALIAS) {
		info->resolved_node =
		    ACPI_CAST_PTR(struct acpi_namespace_node,
				  info->resolved_node->object);
	}

	ACPI_DEBUG_PRINT((ACPI_DB_NAMES, "%s [%p] Value %p\n", info->pathname,
			  info->resolved_node,
			  acpi_ns_get_attached_object(info->resolved_node)));

	node = info->resolved_node;

	
	if (acpi_ns_get_type(info->resolved_node) == ACPI_TYPE_METHOD) {
		

		

		info->obj_desc =
		    acpi_ns_get_attached_object(info->resolved_node);
		if (!info->obj_desc) {
			ACPI_ERROR((AE_INFO,
				    "Control method has no attached sub-object"));
			return_ACPI_STATUS(AE_NULL_OBJECT);
		}

		

		if (info->parameters) {
			while (info->parameters[info->param_count]) {
				if (info->param_count > ACPI_METHOD_MAX_ARG) {
					return_ACPI_STATUS(AE_LIMIT);
				}
				info->param_count++;
			}
		}


		ACPI_DUMP_PATHNAME(info->resolved_node, "ACPI: Execute Method",
				   ACPI_LV_INFO, _COMPONENT);

		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "Method at AML address %p Length %X\n",
				  info->obj_desc->method.aml_start + 1,
				  info->obj_desc->method.aml_length - 1));

		
		acpi_ex_enter_interpreter();
		status = acpi_ps_execute_method(info);
		acpi_ex_exit_interpreter();
	} else {
		
		switch (info->resolved_node->type) {
		case ACPI_TYPE_DEVICE:
		case ACPI_TYPE_EVENT:
		case ACPI_TYPE_MUTEX:
		case ACPI_TYPE_REGION:
		case ACPI_TYPE_THERMAL:
		case ACPI_TYPE_LOCAL_SCOPE:

			ACPI_ERROR((AE_INFO,
				    "[%4.4s] Evaluation of object type [%s] is not supported",
				    info->resolved_node->name.ascii,
				    acpi_ut_get_type_name(info->resolved_node->
							  type)));

			return_ACPI_STATUS(AE_TYPE);

		default:
			break;
		}

		
		acpi_ex_enter_interpreter();

		

		status =
		    acpi_ex_resolve_node_to_value(&info->resolved_node, NULL);
		acpi_ex_exit_interpreter();

		
		if (ACPI_SUCCESS(status)) {
			status = AE_CTRL_RETURN_VALUE;
			info->return_object =
			    ACPI_CAST_PTR(union acpi_operand_object,
					  info->resolved_node);

			ACPI_DEBUG_PRINT((ACPI_DB_NAMES,
					  "Returning object %p [%s]\n",
					  info->return_object,
					  acpi_ut_get_object_type_name(info->
								       return_object)));
		}
	}

	
	(void)acpi_ns_check_predefined_names(node, info->param_count,
		status, &info->return_object);

	

	if (status == AE_CTRL_RETURN_VALUE) {

		

		if (info->flags & ACPI_IGNORE_RETURN_VALUE) {
			acpi_ut_remove_reference(info->return_object);
			info->return_object = NULL;
		}

		

		status = AE_OK;
	}

	ACPI_DEBUG_PRINT((ACPI_DB_NAMES,
			  "*** Completed evaluation of object %s ***\n",
			  info->pathname));

	
	return_ACPI_STATUS(status);
}



void acpi_ns_exec_module_code_list(void)
{
	union acpi_operand_object *prev;
	union acpi_operand_object *next;
	struct acpi_evaluate_info *info;
	u32 method_count = 0;

	ACPI_FUNCTION_TRACE(ns_exec_module_code_list);

	

	next = acpi_gbl_module_code_list;
	if (!next) {
		return_VOID;
	}

	

	info = ACPI_ALLOCATE(sizeof(struct acpi_evaluate_info));
	if (!info) {
		return_VOID;
	}

	

	while (next) {
		prev = next;
		next = next->method.mutex;

		

		prev->method.mutex = NULL;
		acpi_ns_exec_module_code(prev, info);
		method_count++;

		

		acpi_ut_remove_reference(prev);
	}

	ACPI_INFO((AE_INFO,
		   "Executed %u blocks of module-level executable AML code",
		   method_count));

	ACPI_FREE(info);
	acpi_gbl_module_code_list = NULL;
	return_VOID;
}



static void
acpi_ns_exec_module_code(union acpi_operand_object *method_obj,
			 struct acpi_evaluate_info *info)
{
	union acpi_operand_object *root_obj;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ns_exec_module_code);

	

	ACPI_MEMSET(info, 0, sizeof(struct acpi_evaluate_info));
	info->prefix_node = acpi_gbl_root_node;

	
	root_obj = acpi_ns_get_attached_object(acpi_gbl_root_node);
	acpi_ut_add_reference(root_obj);

	

	status = acpi_ns_attach_object(acpi_gbl_root_node, method_obj,
				       ACPI_TYPE_METHOD);
	if (ACPI_FAILURE(status)) {
		goto exit;
	}

	

	status = acpi_ns_evaluate(info);

	ACPI_DEBUG_PRINT((ACPI_DB_INIT, "Executed module-level code at %p\n",
			  method_obj->method.aml_start));

	

	acpi_ns_detach_object(acpi_gbl_root_node);

	

	status =
	    acpi_ns_attach_object(acpi_gbl_root_node, root_obj,
				  ACPI_TYPE_DEVICE);

      exit:
	acpi_ut_remove_reference(root_obj);
	return_VOID;
}
