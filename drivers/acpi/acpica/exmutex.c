




#include <acpi/acpi.h>
#include "accommon.h"
#include "acinterp.h"
#include "acevents.h"

#define _COMPONENT          ACPI_EXECUTER
ACPI_MODULE_NAME("exmutex")


static void
acpi_ex_link_mutex(union acpi_operand_object *obj_desc,
		   struct acpi_thread_state *thread);



void acpi_ex_unlink_mutex(union acpi_operand_object *obj_desc)
{
	struct acpi_thread_state *thread = obj_desc->mutex.owner_thread;

	if (!thread) {
		return;
	}

	

	if (obj_desc->mutex.next) {
		(obj_desc->mutex.next)->mutex.prev = obj_desc->mutex.prev;
	}

	if (obj_desc->mutex.prev) {
		(obj_desc->mutex.prev)->mutex.next = obj_desc->mutex.next;

		
		(obj_desc->mutex.prev)->mutex.original_sync_level =
		    obj_desc->mutex.original_sync_level;
	} else {
		thread->acquired_mutex_list = obj_desc->mutex.next;
	}
}



static void
acpi_ex_link_mutex(union acpi_operand_object *obj_desc,
		   struct acpi_thread_state *thread)
{
	union acpi_operand_object *list_head;

	list_head = thread->acquired_mutex_list;

	

	obj_desc->mutex.prev = NULL;
	obj_desc->mutex.next = list_head;

	

	if (list_head) {
		list_head->mutex.prev = obj_desc;
	}

	

	thread->acquired_mutex_list = obj_desc;
}



acpi_status
acpi_ex_acquire_mutex_object(u16 timeout,
			     union acpi_operand_object *obj_desc,
			     acpi_thread_id thread_id)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE_PTR(ex_acquire_mutex_object, obj_desc);

	if (!obj_desc) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	

	if (obj_desc->mutex.thread_id == thread_id) {
		
		obj_desc->mutex.acquisition_depth++;
		return_ACPI_STATUS(AE_OK);
	}

	

	if (obj_desc == acpi_gbl_global_lock_mutex) {
		status = acpi_ev_acquire_global_lock(timeout);
	} else {
		status = acpi_ex_system_wait_mutex(obj_desc->mutex.os_mutex,
						   timeout);
	}

	if (ACPI_FAILURE(status)) {

		

		return_ACPI_STATUS(status);
	}

	

	obj_desc->mutex.thread_id = thread_id;
	obj_desc->mutex.acquisition_depth = 1;
	obj_desc->mutex.original_sync_level = 0;
	obj_desc->mutex.owner_thread = NULL;	

	return_ACPI_STATUS(AE_OK);
}



acpi_status
acpi_ex_acquire_mutex(union acpi_operand_object *time_desc,
		      union acpi_operand_object *obj_desc,
		      struct acpi_walk_state *walk_state)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE_PTR(ex_acquire_mutex, obj_desc);

	if (!obj_desc) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	

	if (!walk_state->thread) {
		ACPI_ERROR((AE_INFO,
			    "Cannot acquire Mutex [%4.4s], null thread info",
			    acpi_ut_get_node_name(obj_desc->mutex.node)));
		return_ACPI_STATUS(AE_AML_INTERNAL);
	}

	
	if (walk_state->thread->current_sync_level > obj_desc->mutex.sync_level) {
		ACPI_ERROR((AE_INFO,
			    "Cannot acquire Mutex [%4.4s], current SyncLevel is too large (%d)",
			    acpi_ut_get_node_name(obj_desc->mutex.node),
			    walk_state->thread->current_sync_level));
		return_ACPI_STATUS(AE_AML_MUTEX_ORDER);
	}

	status = acpi_ex_acquire_mutex_object((u16) time_desc->integer.value,
					      obj_desc,
					      walk_state->thread->thread_id);
	if (ACPI_SUCCESS(status) && obj_desc->mutex.acquisition_depth == 1) {

		

		obj_desc->mutex.owner_thread = walk_state->thread;
		obj_desc->mutex.original_sync_level =
		    walk_state->thread->current_sync_level;
		walk_state->thread->current_sync_level =
		    obj_desc->mutex.sync_level;

		

		acpi_ex_link_mutex(obj_desc, walk_state->thread);
	}

	return_ACPI_STATUS(status);
}



acpi_status acpi_ex_release_mutex_object(union acpi_operand_object *obj_desc)
{
	acpi_status status = AE_OK;

	ACPI_FUNCTION_TRACE(ex_release_mutex_object);

	if (obj_desc->mutex.acquisition_depth == 0) {
		return (AE_NOT_ACQUIRED);
	}

	

	obj_desc->mutex.acquisition_depth--;
	if (obj_desc->mutex.acquisition_depth != 0) {

		

		return_ACPI_STATUS(AE_OK);
	}

	if (obj_desc->mutex.owner_thread) {

		

		acpi_ex_unlink_mutex(obj_desc);
		obj_desc->mutex.owner_thread = NULL;
	}

	

	if (obj_desc == acpi_gbl_global_lock_mutex) {
		status = acpi_ev_release_global_lock();
	} else {
		acpi_os_release_mutex(obj_desc->mutex.os_mutex);
	}

	

	obj_desc->mutex.thread_id = NULL;
	return_ACPI_STATUS(status);
}



acpi_status
acpi_ex_release_mutex(union acpi_operand_object *obj_desc,
		      struct acpi_walk_state *walk_state)
{
	acpi_status status = AE_OK;
	u8 previous_sync_level;

	ACPI_FUNCTION_TRACE(ex_release_mutex);

	if (!obj_desc) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	

	if (!obj_desc->mutex.owner_thread) {
		ACPI_ERROR((AE_INFO,
			    "Cannot release Mutex [%4.4s], not acquired",
			    acpi_ut_get_node_name(obj_desc->mutex.node)));
		return_ACPI_STATUS(AE_AML_MUTEX_NOT_ACQUIRED);
	}

	
	if ((obj_desc->mutex.owner_thread->thread_id !=
	     walk_state->thread->thread_id)
	    && (obj_desc != acpi_gbl_global_lock_mutex)) {
		ACPI_ERROR((AE_INFO,
			    "Thread %p cannot release Mutex [%4.4s] acquired by thread %p",
			    ACPI_CAST_PTR(void, walk_state->thread->thread_id),
			    acpi_ut_get_node_name(obj_desc->mutex.node),
			    ACPI_CAST_PTR(void,
					  obj_desc->mutex.owner_thread->
					  thread_id)));
		return_ACPI_STATUS(AE_AML_NOT_OWNER);
	}

	

	if (!walk_state->thread) {
		ACPI_ERROR((AE_INFO,
			    "Cannot release Mutex [%4.4s], null thread info",
			    acpi_ut_get_node_name(obj_desc->mutex.node)));
		return_ACPI_STATUS(AE_AML_INTERNAL);
	}

	
	if (obj_desc->mutex.sync_level !=
	    walk_state->thread->current_sync_level) {
		ACPI_ERROR((AE_INFO,
			    "Cannot release Mutex [%4.4s], SyncLevel mismatch: mutex %d current %d",
			    acpi_ut_get_node_name(obj_desc->mutex.node),
			    obj_desc->mutex.sync_level,
			    walk_state->thread->current_sync_level));
		return_ACPI_STATUS(AE_AML_MUTEX_ORDER);
	}

	
	previous_sync_level =
	    walk_state->thread->acquired_mutex_list->mutex.original_sync_level;

	status = acpi_ex_release_mutex_object(obj_desc);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	if (obj_desc->mutex.acquisition_depth == 0) {

		

		walk_state->thread->current_sync_level = previous_sync_level;
	}
	return_ACPI_STATUS(status);
}



void acpi_ex_release_all_mutexes(struct acpi_thread_state *thread)
{
	union acpi_operand_object *next = thread->acquired_mutex_list;
	union acpi_operand_object *obj_desc;

	ACPI_FUNCTION_ENTRY();

	

	while (next) {
		obj_desc = next;
		next = obj_desc->mutex.next;

		obj_desc->mutex.prev = NULL;
		obj_desc->mutex.next = NULL;
		obj_desc->mutex.acquisition_depth = 0;

		

		if (obj_desc == acpi_gbl_global_lock_mutex) {

			

			(void)acpi_ev_release_global_lock();
		} else {
			acpi_os_release_mutex(obj_desc->mutex.os_mutex);
		}

		

		obj_desc->mutex.owner_thread = NULL;
		obj_desc->mutex.thread_id = NULL;

		

		thread->current_sync_level =
		    obj_desc->mutex.original_sync_level;
	}
}
