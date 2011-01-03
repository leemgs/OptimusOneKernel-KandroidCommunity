



#include <acpi/acpi.h>
#include "accommon.h"
#include "acevents.h"
#include "acnamesp.h"
#include "acinterp.h"

#define _COMPONENT          ACPI_EVENTS
ACPI_MODULE_NAME("evmisc")


static void ACPI_SYSTEM_XFACE acpi_ev_notify_dispatch(void *context);

static u32 acpi_ev_global_lock_handler(void *context);

static acpi_status acpi_ev_remove_global_lock_handler(void);



u8 acpi_ev_is_notify_object(struct acpi_namespace_node *node)
{
	switch (node->type) {
	case ACPI_TYPE_DEVICE:
	case ACPI_TYPE_PROCESSOR:
	case ACPI_TYPE_THERMAL:
		
		return (TRUE);

	default:
		return (FALSE);
	}
}



acpi_status
acpi_ev_queue_notify_request(struct acpi_namespace_node * node,
			     u32 notify_value)
{
	union acpi_operand_object *obj_desc;
	union acpi_operand_object *handler_obj = NULL;
	union acpi_generic_state *notify_info;
	acpi_status status = AE_OK;

	ACPI_FUNCTION_NAME(ev_queue_notify_request);

	
	ACPI_DEBUG_PRINT((ACPI_DB_INFO,
			  "Dispatching Notify on [%4.4s] Node %p Value 0x%2.2X (%s)\n",
			  acpi_ut_get_node_name(node), node, notify_value,
			  acpi_ut_get_notify_name(notify_value)));

	

	obj_desc = acpi_ns_get_attached_object(node);
	if (obj_desc) {

		

		switch (node->type) {

			

		case ACPI_TYPE_DEVICE:
		case ACPI_TYPE_THERMAL:
		case ACPI_TYPE_PROCESSOR:

			if (notify_value <= ACPI_MAX_SYS_NOTIFY) {
				handler_obj =
				    obj_desc->common_notify.system_notify;
			} else {
				handler_obj =
				    obj_desc->common_notify.device_notify;
			}
			break;

		default:

			

			return (AE_TYPE);
		}
	}

	
	if ((acpi_gbl_system_notify.handler &&
	     (notify_value <= ACPI_MAX_SYS_NOTIFY)) ||
	    (acpi_gbl_device_notify.handler &&
	     (notify_value > ACPI_MAX_SYS_NOTIFY)) || handler_obj) {
		notify_info = acpi_ut_create_generic_state();
		if (!notify_info) {
			return (AE_NO_MEMORY);
		}

		if (!handler_obj) {
			ACPI_DEBUG_PRINT((ACPI_DB_INFO,
					  "Executing system notify handler for Notify (%4.4s, %X) "
					  "node %p\n",
					  acpi_ut_get_node_name(node),
					  notify_value, node));
		}

		notify_info->common.descriptor_type =
		    ACPI_DESC_TYPE_STATE_NOTIFY;
		notify_info->notify.node = node;
		notify_info->notify.value = (u16) notify_value;
		notify_info->notify.handler_obj = handler_obj;

		status =
		    acpi_os_execute(OSL_NOTIFY_HANDLER, acpi_ev_notify_dispatch,
				    notify_info);
		if (ACPI_FAILURE(status)) {
			acpi_ut_delete_generic_state(notify_info);
		}
	} else {
		

		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "No notify handler for Notify (%4.4s, %X) node %p\n",
				  acpi_ut_get_node_name(node), notify_value,
				  node));
	}

	return (status);
}



static void ACPI_SYSTEM_XFACE acpi_ev_notify_dispatch(void *context)
{
	union acpi_generic_state *notify_info =
	    (union acpi_generic_state *)context;
	acpi_notify_handler global_handler = NULL;
	void *global_context = NULL;
	union acpi_operand_object *handler_obj;

	ACPI_FUNCTION_ENTRY();

	
	if (notify_info->notify.value <= ACPI_MAX_SYS_NOTIFY) {

		

		if (acpi_gbl_system_notify.handler) {
			global_handler = acpi_gbl_system_notify.handler;
			global_context = acpi_gbl_system_notify.context;
		}
	} else {
		

		if (acpi_gbl_device_notify.handler) {
			global_handler = acpi_gbl_device_notify.handler;
			global_context = acpi_gbl_device_notify.context;
		}
	}

	

	if (global_handler) {
		global_handler(notify_info->notify.node,
			       notify_info->notify.value, global_context);
	}

	

	handler_obj = notify_info->notify.handler_obj;
	if (handler_obj) {
		handler_obj->notify.handler(notify_info->notify.node,
					    notify_info->notify.value,
					    handler_obj->notify.context);
	}

	

	acpi_ut_delete_generic_state(notify_info);
}



static u32 acpi_ev_global_lock_handler(void *context)
{
	u8 acquired = FALSE;

	
	ACPI_ACQUIRE_GLOBAL_LOCK(acpi_gbl_FACS, acquired);
	if (acquired) {

		

		acpi_gbl_global_lock_acquired = TRUE;
		

		if (ACPI_FAILURE
		    (acpi_os_signal_semaphore
		     (acpi_gbl_global_lock_semaphore, 1))) {
			ACPI_ERROR((AE_INFO,
				    "Could not signal Global Lock semaphore"));
		}
	}

	return (ACPI_INTERRUPT_HANDLED);
}



acpi_status acpi_ev_init_global_lock_handler(void)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_init_global_lock_handler);

	

	status = acpi_install_fixed_event_handler(ACPI_EVENT_GLOBAL,
						  acpi_ev_global_lock_handler,
						  NULL);

	
	if (status == AE_NO_HARDWARE_RESPONSE) {
		ACPI_ERROR((AE_INFO,
			    "No response from Global Lock hardware, disabling lock"));

		acpi_gbl_global_lock_present = FALSE;
		return_ACPI_STATUS(AE_OK);
	}

	acpi_gbl_global_lock_present = TRUE;
	return_ACPI_STATUS(status);
}



static acpi_status acpi_ev_remove_global_lock_handler(void)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_remove_global_lock_handler);

	acpi_gbl_global_lock_present = FALSE;
	status = acpi_remove_fixed_event_handler(ACPI_EVENT_GLOBAL,
						 acpi_ev_global_lock_handler);

	return_ACPI_STATUS(status);
}


static acpi_thread_id acpi_ev_global_lock_thread_id;
static int acpi_ev_global_lock_acquired;

acpi_status acpi_ev_acquire_global_lock(u16 timeout)
{
	acpi_status status = AE_OK;
	u8 acquired = FALSE;

	ACPI_FUNCTION_TRACE(ev_acquire_global_lock);

	
	status = acpi_ex_system_wait_mutex(
			acpi_gbl_global_lock_mutex->mutex.os_mutex, 0);
	if (status == AE_TIME) {
		if (acpi_ev_global_lock_thread_id == acpi_os_get_thread_id()) {
			acpi_ev_global_lock_acquired++;
			return AE_OK;
		}
	}

	if (ACPI_FAILURE(status)) {
		status = acpi_ex_system_wait_mutex(
				acpi_gbl_global_lock_mutex->mutex.os_mutex,
				timeout);
	}
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	acpi_ev_global_lock_thread_id = acpi_os_get_thread_id();
	acpi_ev_global_lock_acquired++;

	
	acpi_gbl_global_lock_handle++;
	if (acpi_gbl_global_lock_handle == 0) {
		acpi_gbl_global_lock_handle = 1;
	}

	
	if (!acpi_gbl_global_lock_present) {
		acpi_gbl_global_lock_acquired = TRUE;
		return_ACPI_STATUS(AE_OK);
	}

	

	ACPI_ACQUIRE_GLOBAL_LOCK(acpi_gbl_FACS, acquired);
	if (acquired) {

		

		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "Acquired hardware Global Lock\n"));

		acpi_gbl_global_lock_acquired = TRUE;
		return_ACPI_STATUS(AE_OK);
	}

	
	ACPI_DEBUG_PRINT((ACPI_DB_EXEC, "Waiting for hardware Global Lock\n"));

	
	status = acpi_ex_system_wait_semaphore(acpi_gbl_global_lock_semaphore,
					       ACPI_WAIT_FOREVER);

	return_ACPI_STATUS(status);
}



acpi_status acpi_ev_release_global_lock(void)
{
	u8 pending = FALSE;
	acpi_status status = AE_OK;

	ACPI_FUNCTION_TRACE(ev_release_global_lock);

	

	if (!acpi_gbl_global_lock_acquired) {
		ACPI_WARNING((AE_INFO,
			      "Cannot release the ACPI Global Lock, it has not been acquired"));
		return_ACPI_STATUS(AE_NOT_ACQUIRED);
	}

	acpi_ev_global_lock_acquired--;
	if (acpi_ev_global_lock_acquired > 0) {
		return AE_OK;
	}

	if (acpi_gbl_global_lock_present) {

		

		ACPI_RELEASE_GLOBAL_LOCK(acpi_gbl_FACS, pending);

		
		if (pending) {
			status =
			    acpi_write_bit_register
			    (ACPI_BITREG_GLOBAL_LOCK_RELEASE,
			     ACPI_ENABLE_EVENT);
		}

		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "Released hardware Global Lock\n"));
	}

	acpi_gbl_global_lock_acquired = FALSE;

	
	acpi_ev_global_lock_thread_id = NULL;
	acpi_ev_global_lock_acquired = 0;
	acpi_os_release_mutex(acpi_gbl_global_lock_mutex->mutex.os_mutex);
	return_ACPI_STATUS(status);
}



void acpi_ev_terminate(void)
{
	u32 i;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_terminate);

	if (acpi_gbl_events_initialized) {
		

		

		for (i = 0; i < ACPI_NUM_FIXED_EVENTS; i++) {
			status = acpi_disable_event(i, 0);
			if (ACPI_FAILURE(status)) {
				ACPI_ERROR((AE_INFO,
					    "Could not disable fixed event %d",
					    (u32) i));
			}
		}

		

		status = acpi_ev_walk_gpe_list(acpi_hw_disable_gpe_block, NULL);

		

		status = acpi_ev_remove_sci_handler();
		if (ACPI_FAILURE(status)) {
			ACPI_ERROR((AE_INFO, "Could not remove SCI handler"));
		}

		status = acpi_ev_remove_global_lock_handler();
		if (ACPI_FAILURE(status)) {
			ACPI_ERROR((AE_INFO,
				    "Could not remove Global Lock handler"));
		}
	}

	

	status = acpi_ev_walk_gpe_list(acpi_ev_delete_gpe_handlers, NULL);

	

	if (acpi_gbl_original_mode == ACPI_SYS_MODE_LEGACY) {
		status = acpi_disable();
		if (ACPI_FAILURE(status)) {
			ACPI_WARNING((AE_INFO, "AcpiDisable failed"));
		}
	}
	return_VOID;
}
