



#include <acpi/acpi.h>
#include "accommon.h"
#include "amlcode.h"
#include "acdispat.h"
#include "acinterp.h"
#include "acnamesp.h"
#ifdef	ACPI_DISASSEMBLER
#include <acpi/acdisasm.h>
#endif

#define _COMPONENT          ACPI_DISPATCHER
ACPI_MODULE_NAME("dsmethod")


static acpi_status
acpi_ds_create_method_mutex(union acpi_operand_object *method_desc);



acpi_status
acpi_ds_method_error(acpi_status status, struct acpi_walk_state *walk_state)
{
	ACPI_FUNCTION_ENTRY();

	

	if (ACPI_SUCCESS(status) || (status & AE_CODE_CONTROL)) {
		return (status);
	}

	

	if (acpi_gbl_exception_handler) {

		

		acpi_ex_exit_interpreter();

		
		status = acpi_gbl_exception_handler(status,
						    walk_state->method_node ?
						    walk_state->method_node->
						    name.integer : 0,
						    walk_state->opcode,
						    walk_state->aml_offset,
						    NULL);
		acpi_ex_enter_interpreter();
	}

	acpi_ds_clear_implicit_return(walk_state);

#ifdef ACPI_DISASSEMBLER
	if (ACPI_FAILURE(status)) {

		

		acpi_dm_dump_method_info(status, walk_state, walk_state->op);
	}
#endif

	return (status);
}



static acpi_status
acpi_ds_create_method_mutex(union acpi_operand_object *method_desc)
{
	union acpi_operand_object *mutex_desc;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ds_create_method_mutex);

	

	mutex_desc = acpi_ut_create_internal_object(ACPI_TYPE_MUTEX);
	if (!mutex_desc) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	

	status = acpi_os_create_mutex(&mutex_desc->mutex.os_mutex);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	mutex_desc->mutex.sync_level = method_desc->method.sync_level;
	method_desc->method.mutex = mutex_desc;
	return_ACPI_STATUS(AE_OK);
}



acpi_status
acpi_ds_begin_method_execution(struct acpi_namespace_node *method_node,
			       union acpi_operand_object *obj_desc,
			       struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;

	ACPI_FUNCTION_TRACE_PTR(ds_begin_method_execution, method_node);

	if (!method_node) {
		return_ACPI_STATUS(AE_NULL_ENTRY);
	}

	

	if (obj_desc->method.thread_count == ACPI_UINT8_MAX) {
		ACPI_ERROR((AE_INFO,
			    "Method reached maximum reentrancy limit (255)"));
		return_ACPI_STATUS(AE_AML_METHOD_LIMIT);
	}

	
	if (obj_desc->method.method_flags & AML_METHOD_SERIALIZED) {
		
		if (!obj_desc->method.mutex) {
			status = acpi_ds_create_method_mutex(obj_desc);
			if (ACPI_FAILURE(status)) {
				return_ACPI_STATUS(status);
			}
		}

		
		if (walk_state &&
		    (walk_state->thread->current_sync_level >
		     obj_desc->method.mutex->mutex.sync_level)) {
			ACPI_ERROR((AE_INFO,
				    "Cannot acquire Mutex for method [%4.4s], current SyncLevel is too large (%d)",
				    acpi_ut_get_node_name(method_node),
				    walk_state->thread->current_sync_level));

			return_ACPI_STATUS(AE_AML_MUTEX_ORDER);
		}

		
		if (!walk_state ||
		    !obj_desc->method.mutex->mutex.thread_id ||
		    (walk_state->thread->thread_id !=
		     obj_desc->method.mutex->mutex.thread_id)) {
			
			status =
			    acpi_ex_system_wait_mutex(obj_desc->method.mutex->
						      mutex.os_mutex,
						      ACPI_WAIT_FOREVER);
			if (ACPI_FAILURE(status)) {
				return_ACPI_STATUS(status);
			}

			

			if (walk_state) {
				obj_desc->method.mutex->mutex.
				    original_sync_level =
				    walk_state->thread->current_sync_level;

				obj_desc->method.mutex->mutex.thread_id =
				    walk_state->thread->thread_id;
				walk_state->thread->current_sync_level =
				    obj_desc->method.sync_level;
			} else {
				obj_desc->method.mutex->mutex.
				    original_sync_level =
				    obj_desc->method.mutex->mutex.sync_level;
			}
		}

		

		obj_desc->method.mutex->mutex.acquisition_depth++;
	}

	
	if (!obj_desc->method.owner_id) {
		status = acpi_ut_allocate_owner_id(&obj_desc->method.owner_id);
		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}
	}

	
	obj_desc->method.thread_count++;
	return_ACPI_STATUS(status);

      cleanup:
	

	if (obj_desc->method.mutex) {
		acpi_os_release_mutex(obj_desc->method.mutex->mutex.os_mutex);
	}
	return_ACPI_STATUS(status);
}



acpi_status
acpi_ds_call_control_method(struct acpi_thread_state *thread,
			    struct acpi_walk_state *this_walk_state,
			    union acpi_parse_object *op)
{
	acpi_status status;
	struct acpi_namespace_node *method_node;
	struct acpi_walk_state *next_walk_state = NULL;
	union acpi_operand_object *obj_desc;
	struct acpi_evaluate_info *info;
	u32 i;

	ACPI_FUNCTION_TRACE_PTR(ds_call_control_method, this_walk_state);

	ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
			  "Calling method %p, currentstate=%p\n",
			  this_walk_state->prev_op, this_walk_state));

	
	method_node = this_walk_state->method_call_node;
	if (!method_node) {
		return_ACPI_STATUS(AE_NULL_ENTRY);
	}

	obj_desc = acpi_ns_get_attached_object(method_node);
	if (!obj_desc) {
		return_ACPI_STATUS(AE_NULL_OBJECT);
	}

	

	status = acpi_ds_begin_method_execution(method_node, obj_desc,
						this_walk_state);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	next_walk_state = acpi_ds_create_walk_state(obj_desc->method.owner_id,
						    NULL, obj_desc, thread);
	if (!next_walk_state) {
		status = AE_NO_MEMORY;
		goto cleanup;
	}

	
	this_walk_state->operands[this_walk_state->num_operands] = NULL;

	
	info = ACPI_ALLOCATE_ZEROED(sizeof(struct acpi_evaluate_info));
	if (!info) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	info->parameters = &this_walk_state->operands[0];

	status = acpi_ds_init_aml_walk(next_walk_state, NULL, method_node,
				       obj_desc->method.aml_start,
				       obj_desc->method.aml_length, info,
				       ACPI_IMODE_EXECUTE);

	ACPI_FREE(info);
	if (ACPI_FAILURE(status)) {
		goto cleanup;
	}

	
	for (i = 0; i < obj_desc->method.param_count; i++) {
		acpi_ut_remove_reference(this_walk_state->operands[i]);
		this_walk_state->operands[i] = NULL;
	}

	

	this_walk_state->num_operands = 0;

	ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
			  "**** Begin nested execution of [%4.4s] **** WalkState=%p\n",
			  method_node->name.ascii, next_walk_state));

	

	if (obj_desc->method.method_flags & AML_METHOD_INTERNAL_ONLY) {
		status = obj_desc->method.implementation(next_walk_state);
		if (status == AE_OK) {
			status = AE_CTRL_TERMINATE;
		}
	}

	return_ACPI_STATUS(status);

      cleanup:

	

	acpi_ds_terminate_control_method(obj_desc, next_walk_state);
	if (next_walk_state) {
		acpi_ds_delete_walk_state(next_walk_state);
	}

	return_ACPI_STATUS(status);
}



acpi_status
acpi_ds_restart_control_method(struct acpi_walk_state *walk_state,
			       union acpi_operand_object *return_desc)
{
	acpi_status status;
	int same_as_implicit_return;

	ACPI_FUNCTION_TRACE_PTR(ds_restart_control_method, walk_state);

	ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
			  "****Restart [%4.4s] Op %p ReturnValueFromCallee %p\n",
			  acpi_ut_get_node_name(walk_state->method_node),
			  walk_state->method_call_op, return_desc));

	ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
			  "    ReturnFromThisMethodUsed?=%X ResStack %p Walk %p\n",
			  walk_state->return_used,
			  walk_state->results, walk_state));

	

	if (return_desc) {

		

		same_as_implicit_return =
		    (walk_state->implicit_return_obj == return_desc);

		

		if (walk_state->return_used) {

			

			status = acpi_ds_result_push(return_desc, walk_state);
			if (ACPI_FAILURE(status)) {
				acpi_ut_remove_reference(return_desc);
				return_ACPI_STATUS(status);
			}

			
			walk_state->return_desc = return_desc;
		}

		
		else if (!acpi_ds_do_implicit_return
			 (return_desc, walk_state, FALSE)
			 || same_as_implicit_return) {
			
			acpi_ut_remove_reference(return_desc);
		}
	}

	return_ACPI_STATUS(AE_OK);
}



void
acpi_ds_terminate_control_method(union acpi_operand_object *method_desc,
				 struct acpi_walk_state *walk_state)
{

	ACPI_FUNCTION_TRACE_PTR(ds_terminate_control_method, walk_state);

	

	if (!method_desc) {
		return_VOID;
	}

	if (walk_state) {

		

		acpi_ds_method_data_delete_all(walk_state);

		
		if (method_desc->method.mutex) {

			

			method_desc->method.mutex->mutex.acquisition_depth--;
			if (!method_desc->method.mutex->mutex.acquisition_depth) {
				walk_state->thread->current_sync_level =
				    method_desc->method.mutex->mutex.
				    original_sync_level;

				acpi_os_release_mutex(method_desc->method.
						      mutex->mutex.os_mutex);
				method_desc->method.mutex->mutex.thread_id = NULL;
			}
		}

		
		if (!(method_desc->method.flags & AOPOBJ_MODULE_LEVEL)) {
			acpi_ns_delete_namespace_by_owner(method_desc->method.
							  owner_id);
		}
	}

	

	if (method_desc->method.thread_count) {
		method_desc->method.thread_count--;
	} else {
		ACPI_ERROR((AE_INFO, "Invalid zero thread count in method"));
	}

	

	if (method_desc->method.thread_count) {
		
		ACPI_DEBUG_PRINT((ACPI_DB_DISPATCH,
				  "*** Completed execution of one thread, %d threads remaining\n",
				  method_desc->method.thread_count));
	} else {
		

		
		if ((method_desc->method.method_flags & AML_METHOD_SERIALIZED)
		    && (!method_desc->method.mutex)) {
			(void)acpi_ds_create_method_mutex(method_desc);
		}

		

		if (!(method_desc->method.flags & AOPOBJ_MODULE_LEVEL)) {
			acpi_ut_release_owner_id(&method_desc->method.owner_id);
		}
	}

	return_VOID;
}
