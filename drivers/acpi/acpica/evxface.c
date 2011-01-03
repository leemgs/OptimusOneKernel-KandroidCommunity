



#include <acpi/acpi.h>
#include "accommon.h"
#include "acnamesp.h"
#include "acevents.h"
#include "acinterp.h"

#define _COMPONENT          ACPI_EVENTS
ACPI_MODULE_NAME("evxface")


#ifdef ACPI_FUTURE_USAGE
acpi_status acpi_install_exception_handler(acpi_exception_handler handler)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(acpi_install_exception_handler);

	status = acpi_ut_acquire_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	if (acpi_gbl_exception_handler) {
		status = AE_ALREADY_EXISTS;
		goto cleanup;
	}

	

	acpi_gbl_exception_handler = handler;

      cleanup:
	(void)acpi_ut_release_mutex(ACPI_MTX_EVENTS);
	return_ACPI_STATUS(status);
}

ACPI_EXPORT_SYMBOL(acpi_install_exception_handler)
#endif				

acpi_status
acpi_install_fixed_event_handler(u32 event,
				 acpi_event_handler handler, void *context)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(acpi_install_fixed_event_handler);

	

	if (event > ACPI_EVENT_MAX) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	status = acpi_ut_acquire_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	if (NULL != acpi_gbl_fixed_event_handlers[event].handler) {
		status = AE_ALREADY_EXISTS;
		goto cleanup;
	}

	

	acpi_gbl_fixed_event_handlers[event].handler = handler;
	acpi_gbl_fixed_event_handlers[event].context = context;

	status = acpi_clear_event(event);
	if (ACPI_SUCCESS(status))
		status = acpi_enable_event(event, 0);
	if (ACPI_FAILURE(status)) {
		ACPI_WARNING((AE_INFO, "Could not enable fixed event %X",
			      event));

		

		acpi_gbl_fixed_event_handlers[event].handler = NULL;
		acpi_gbl_fixed_event_handlers[event].context = NULL;
	} else {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "Enabled fixed event %X, Handler=%p\n", event,
				  handler));
	}

      cleanup:
	(void)acpi_ut_release_mutex(ACPI_MTX_EVENTS);
	return_ACPI_STATUS(status);
}

ACPI_EXPORT_SYMBOL(acpi_install_fixed_event_handler)


acpi_status
acpi_remove_fixed_event_handler(u32 event, acpi_event_handler handler)
{
	acpi_status status = AE_OK;

	ACPI_FUNCTION_TRACE(acpi_remove_fixed_event_handler);

	

	if (event > ACPI_EVENT_MAX) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	status = acpi_ut_acquire_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	status = acpi_disable_event(event, 0);

	

	acpi_gbl_fixed_event_handlers[event].handler = NULL;
	acpi_gbl_fixed_event_handlers[event].context = NULL;

	if (ACPI_FAILURE(status)) {
		ACPI_WARNING((AE_INFO,
			      "Could not write to fixed event enable register %X",
			      event));
	} else {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Disabled fixed event %X\n",
				  event));
	}

	(void)acpi_ut_release_mutex(ACPI_MTX_EVENTS);
	return_ACPI_STATUS(status);
}

ACPI_EXPORT_SYMBOL(acpi_remove_fixed_event_handler)


acpi_status
acpi_install_notify_handler(acpi_handle device,
			    u32 handler_type,
			    acpi_notify_handler handler, void *context)
{
	union acpi_operand_object *obj_desc;
	union acpi_operand_object *notify_obj;
	struct acpi_namespace_node *node;
	acpi_status status;

	ACPI_FUNCTION_TRACE(acpi_install_notify_handler);

	

	if ((!device) ||
	    (!handler) || (handler_type > ACPI_MAX_NOTIFY_HANDLER_TYPE)) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	status = acpi_ut_acquire_mutex(ACPI_MTX_NAMESPACE);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	node = acpi_ns_map_handle_to_node(device);
	if (!node) {
		status = AE_BAD_PARAMETER;
		goto unlock_and_exit;
	}

	
	if (device == ACPI_ROOT_OBJECT) {

		

		if (((handler_type & ACPI_SYSTEM_NOTIFY) &&
		     acpi_gbl_system_notify.handler) ||
		    ((handler_type & ACPI_DEVICE_NOTIFY) &&
		     acpi_gbl_device_notify.handler)) {
			status = AE_ALREADY_EXISTS;
			goto unlock_and_exit;
		}

		if (handler_type & ACPI_SYSTEM_NOTIFY) {
			acpi_gbl_system_notify.node = node;
			acpi_gbl_system_notify.handler = handler;
			acpi_gbl_system_notify.context = context;
		}

		if (handler_type & ACPI_DEVICE_NOTIFY) {
			acpi_gbl_device_notify.node = node;
			acpi_gbl_device_notify.handler = handler;
			acpi_gbl_device_notify.context = context;
		}

		
	}

	
	else {
		

		if (!acpi_ev_is_notify_object(node)) {
			status = AE_TYPE;
			goto unlock_and_exit;
		}

		

		obj_desc = acpi_ns_get_attached_object(node);
		if (obj_desc) {

			

			if (((handler_type & ACPI_SYSTEM_NOTIFY) &&
			     obj_desc->common_notify.system_notify) ||
			    ((handler_type & ACPI_DEVICE_NOTIFY) &&
			     obj_desc->common_notify.device_notify)) {
				status = AE_ALREADY_EXISTS;
				goto unlock_and_exit;
			}
		} else {
			

			obj_desc = acpi_ut_create_internal_object(node->type);
			if (!obj_desc) {
				status = AE_NO_MEMORY;
				goto unlock_and_exit;
			}

			

			status =
			    acpi_ns_attach_object(device, obj_desc, node->type);

			

			acpi_ut_remove_reference(obj_desc);
			if (ACPI_FAILURE(status)) {
				goto unlock_and_exit;
			}
		}

		

		notify_obj =
		    acpi_ut_create_internal_object(ACPI_TYPE_LOCAL_NOTIFY);
		if (!notify_obj) {
			status = AE_NO_MEMORY;
			goto unlock_and_exit;
		}

		notify_obj->notify.node = node;
		notify_obj->notify.handler = handler;
		notify_obj->notify.context = context;

		if (handler_type & ACPI_SYSTEM_NOTIFY) {
			obj_desc->common_notify.system_notify = notify_obj;
		}

		if (handler_type & ACPI_DEVICE_NOTIFY) {
			obj_desc->common_notify.device_notify = notify_obj;
		}

		if (handler_type == ACPI_ALL_NOTIFY) {

			

			acpi_ut_add_reference(notify_obj);
		}
	}

      unlock_and_exit:
	(void)acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
	return_ACPI_STATUS(status);
}

ACPI_EXPORT_SYMBOL(acpi_install_notify_handler)


acpi_status
acpi_remove_notify_handler(acpi_handle device,
			   u32 handler_type, acpi_notify_handler handler)
{
	union acpi_operand_object *notify_obj;
	union acpi_operand_object *obj_desc;
	struct acpi_namespace_node *node;
	acpi_status status;

	ACPI_FUNCTION_TRACE(acpi_remove_notify_handler);

	

	if ((!device) ||
	    (!handler) || (handler_type > ACPI_MAX_NOTIFY_HANDLER_TYPE)) {
		status = AE_BAD_PARAMETER;
		goto exit;
	}

	status = acpi_ut_acquire_mutex(ACPI_MTX_NAMESPACE);
	if (ACPI_FAILURE(status)) {
		goto exit;
	}

	

	node = acpi_ns_map_handle_to_node(device);
	if (!node) {
		status = AE_BAD_PARAMETER;
		goto unlock_and_exit;
	}

	

	if (device == ACPI_ROOT_OBJECT) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "Removing notify handler for namespace root object\n"));

		if (((handler_type & ACPI_SYSTEM_NOTIFY) &&
		     !acpi_gbl_system_notify.handler) ||
		    ((handler_type & ACPI_DEVICE_NOTIFY) &&
		     !acpi_gbl_device_notify.handler)) {
			status = AE_NOT_EXIST;
			goto unlock_and_exit;
		}

		

		(void)acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
		acpi_os_wait_events_complete(NULL);
		status = acpi_ut_acquire_mutex(ACPI_MTX_NAMESPACE);
		if (ACPI_FAILURE(status)) {
			goto exit;
		}

		if (handler_type & ACPI_SYSTEM_NOTIFY) {
			acpi_gbl_system_notify.node = NULL;
			acpi_gbl_system_notify.handler = NULL;
			acpi_gbl_system_notify.context = NULL;
		}

		if (handler_type & ACPI_DEVICE_NOTIFY) {
			acpi_gbl_device_notify.node = NULL;
			acpi_gbl_device_notify.handler = NULL;
			acpi_gbl_device_notify.context = NULL;
		}
	}

	

	else {
		

		if (!acpi_ev_is_notify_object(node)) {
			status = AE_TYPE;
			goto unlock_and_exit;
		}

		

		obj_desc = acpi_ns_get_attached_object(node);
		if (!obj_desc) {
			status = AE_NOT_EXIST;
			goto unlock_and_exit;
		}

		

		if (handler_type & ACPI_SYSTEM_NOTIFY) {
			notify_obj = obj_desc->common_notify.system_notify;
			if (!notify_obj) {
				status = AE_NOT_EXIST;
				goto unlock_and_exit;
			}

			if (notify_obj->notify.handler != handler) {
				status = AE_BAD_PARAMETER;
				goto unlock_and_exit;
			}
			

			(void)acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
			acpi_os_wait_events_complete(NULL);
			status = acpi_ut_acquire_mutex(ACPI_MTX_NAMESPACE);
			if (ACPI_FAILURE(status)) {
				goto exit;
			}

			
			obj_desc->common_notify.system_notify = NULL;
			acpi_ut_remove_reference(notify_obj);
		}

		if (handler_type & ACPI_DEVICE_NOTIFY) {
			notify_obj = obj_desc->common_notify.device_notify;
			if (!notify_obj) {
				status = AE_NOT_EXIST;
				goto unlock_and_exit;
			}

			if (notify_obj->notify.handler != handler) {
				status = AE_BAD_PARAMETER;
				goto unlock_and_exit;
			}
			

			(void)acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
			acpi_os_wait_events_complete(NULL);
			status = acpi_ut_acquire_mutex(ACPI_MTX_NAMESPACE);
			if (ACPI_FAILURE(status)) {
				goto exit;
			}

			
			obj_desc->common_notify.device_notify = NULL;
			acpi_ut_remove_reference(notify_obj);
		}
	}

      unlock_and_exit:
	(void)acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
      exit:
	if (ACPI_FAILURE(status))
		ACPI_EXCEPTION((AE_INFO, status, "Removing notify handler"));
	return_ACPI_STATUS(status);
}

ACPI_EXPORT_SYMBOL(acpi_remove_notify_handler)


acpi_status
acpi_install_gpe_handler(acpi_handle gpe_device,
			 u32 gpe_number,
			 u32 type, acpi_event_handler address, void *context)
{
	struct acpi_gpe_event_info *gpe_event_info;
	struct acpi_handler_info *handler;
	acpi_status status;
	acpi_cpu_flags flags;

	ACPI_FUNCTION_TRACE(acpi_install_gpe_handler);

	

	if ((!address) || (type > ACPI_GPE_XRUPT_TYPE_MASK)) {
		status = AE_BAD_PARAMETER;
		goto exit;
	}

	status = acpi_ut_acquire_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		goto exit;
	}

	

	gpe_event_info = acpi_ev_get_gpe_event_info(gpe_device, gpe_number);
	if (!gpe_event_info) {
		status = AE_BAD_PARAMETER;
		goto unlock_and_exit;
	}

	

	if ((gpe_event_info->flags & ACPI_GPE_DISPATCH_MASK) ==
	    ACPI_GPE_DISPATCH_HANDLER) {
		status = AE_ALREADY_EXISTS;
		goto unlock_and_exit;
	}

	

	handler = ACPI_ALLOCATE_ZEROED(sizeof(struct acpi_handler_info));
	if (!handler) {
		status = AE_NO_MEMORY;
		goto unlock_and_exit;
	}

	handler->address = address;
	handler->context = context;
	handler->method_node = gpe_event_info->dispatch.method_node;

	

	status = acpi_ev_disable_gpe(gpe_event_info);
	if (ACPI_FAILURE(status)) {
		goto unlock_and_exit;
	}

	

	flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);
	gpe_event_info->dispatch.handler = handler;

	

	gpe_event_info->flags &=
	    ~(ACPI_GPE_XRUPT_TYPE_MASK | ACPI_GPE_DISPATCH_MASK);
	gpe_event_info->flags |= (u8) (type | ACPI_GPE_DISPATCH_HANDLER);

	acpi_os_release_lock(acpi_gbl_gpe_lock, flags);

      unlock_and_exit:
	(void)acpi_ut_release_mutex(ACPI_MTX_EVENTS);
      exit:
	if (ACPI_FAILURE(status))
		ACPI_EXCEPTION((AE_INFO, status,
				"Installing notify handler failed"));
	return_ACPI_STATUS(status);
}

ACPI_EXPORT_SYMBOL(acpi_install_gpe_handler)


acpi_status
acpi_remove_gpe_handler(acpi_handle gpe_device,
			u32 gpe_number, acpi_event_handler address)
{
	struct acpi_gpe_event_info *gpe_event_info;
	struct acpi_handler_info *handler;
	acpi_status status;
	acpi_cpu_flags flags;

	ACPI_FUNCTION_TRACE(acpi_remove_gpe_handler);

	

	if (!address) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	status = acpi_ut_acquire_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	gpe_event_info = acpi_ev_get_gpe_event_info(gpe_device, gpe_number);
	if (!gpe_event_info) {
		status = AE_BAD_PARAMETER;
		goto unlock_and_exit;
	}

	

	if ((gpe_event_info->flags & ACPI_GPE_DISPATCH_MASK) !=
	    ACPI_GPE_DISPATCH_HANDLER) {
		status = AE_NOT_EXIST;
		goto unlock_and_exit;
	}

	

	if (gpe_event_info->dispatch.handler->address != address) {
		status = AE_BAD_PARAMETER;
		goto unlock_and_exit;
	}

	

	status = acpi_ev_disable_gpe(gpe_event_info);
	if (ACPI_FAILURE(status)) {
		goto unlock_and_exit;
	}

	

	(void)acpi_ut_release_mutex(ACPI_MTX_EVENTS);
	acpi_os_wait_events_complete(NULL);
	status = acpi_ut_acquire_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);
	handler = gpe_event_info->dispatch.handler;

	

	gpe_event_info->dispatch.method_node = handler->method_node;
	gpe_event_info->flags &= ~ACPI_GPE_DISPATCH_MASK;	
	if (handler->method_node) {
		gpe_event_info->flags |= ACPI_GPE_DISPATCH_METHOD;
	}
	acpi_os_release_lock(acpi_gbl_gpe_lock, flags);

	

	ACPI_FREE(handler);

      unlock_and_exit:
	(void)acpi_ut_release_mutex(ACPI_MTX_EVENTS);
	return_ACPI_STATUS(status);
}

ACPI_EXPORT_SYMBOL(acpi_remove_gpe_handler)


acpi_status acpi_acquire_global_lock(u16 timeout, u32 * handle)
{
	acpi_status status;

	if (!handle) {
		return (AE_BAD_PARAMETER);
	}

	

	acpi_ex_enter_interpreter();

	status = acpi_ex_acquire_mutex_object(timeout,
					      acpi_gbl_global_lock_mutex,
					      acpi_os_get_thread_id());

	if (ACPI_SUCCESS(status)) {

		

		*handle = acpi_gbl_global_lock_handle;
	}

	acpi_ex_exit_interpreter();
	return (status);
}

ACPI_EXPORT_SYMBOL(acpi_acquire_global_lock)


acpi_status acpi_release_global_lock(u32 handle)
{
	acpi_status status;

	if (!handle || (handle != acpi_gbl_global_lock_handle)) {
		return (AE_NOT_ACQUIRED);
	}

	status = acpi_ex_release_mutex_object(acpi_gbl_global_lock_mutex);
	return (status);
}

ACPI_EXPORT_SYMBOL(acpi_release_global_lock)
