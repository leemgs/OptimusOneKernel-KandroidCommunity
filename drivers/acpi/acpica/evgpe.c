



#include <acpi/acpi.h>
#include "accommon.h"
#include "acevents.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_EVENTS
ACPI_MODULE_NAME("evgpe")


static void ACPI_SYSTEM_XFACE acpi_ev_asynch_execute_gpe_method(void *context);



acpi_status
acpi_ev_set_gpe_type(struct acpi_gpe_event_info *gpe_event_info, u8 type)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_set_gpe_type);

	

	switch (type) {
	case ACPI_GPE_TYPE_WAKE:
	case ACPI_GPE_TYPE_RUNTIME:
	case ACPI_GPE_TYPE_WAKE_RUN:
		break;

	default:
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	

	status = acpi_ev_disable_gpe(gpe_event_info);

	

	gpe_event_info->flags &= ~ACPI_GPE_TYPE_MASK;
	gpe_event_info->flags |= type;
	return_ACPI_STATUS(status);
}



acpi_status
acpi_ev_update_gpe_enable_masks(struct acpi_gpe_event_info *gpe_event_info,
				u8 type)
{
	struct acpi_gpe_register_info *gpe_register_info;
	u8 register_bit;

	ACPI_FUNCTION_TRACE(ev_update_gpe_enable_masks);

	gpe_register_info = gpe_event_info->register_info;
	if (!gpe_register_info) {
		return_ACPI_STATUS(AE_NOT_EXIST);
	}

	register_bit = (u8)
	    (1 <<
	     (gpe_event_info->gpe_number - gpe_register_info->base_gpe_number));

	

	if (type == ACPI_GPE_DISABLE) {
		ACPI_CLEAR_BIT(gpe_register_info->enable_for_wake,
			       register_bit);
		ACPI_CLEAR_BIT(gpe_register_info->enable_for_run, register_bit);
		return_ACPI_STATUS(AE_OK);
	}

	

	switch (gpe_event_info->flags & ACPI_GPE_TYPE_MASK) {
	case ACPI_GPE_TYPE_WAKE:
		ACPI_SET_BIT(gpe_register_info->enable_for_wake, register_bit);
		ACPI_CLEAR_BIT(gpe_register_info->enable_for_run, register_bit);
		break;

	case ACPI_GPE_TYPE_RUNTIME:
		ACPI_CLEAR_BIT(gpe_register_info->enable_for_wake,
			       register_bit);
		ACPI_SET_BIT(gpe_register_info->enable_for_run, register_bit);
		break;

	case ACPI_GPE_TYPE_WAKE_RUN:
		ACPI_SET_BIT(gpe_register_info->enable_for_wake, register_bit);
		ACPI_SET_BIT(gpe_register_info->enable_for_run, register_bit);
		break;

	default:
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	return_ACPI_STATUS(AE_OK);
}



acpi_status
acpi_ev_enable_gpe(struct acpi_gpe_event_info *gpe_event_info,
		   u8 write_to_hardware)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_enable_gpe);

	

	status =
	    acpi_ev_update_gpe_enable_masks(gpe_event_info, ACPI_GPE_ENABLE);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	switch (gpe_event_info->flags & ACPI_GPE_TYPE_MASK) {
	case ACPI_GPE_TYPE_WAKE:

		ACPI_SET_BIT(gpe_event_info->flags, ACPI_GPE_WAKE_ENABLED);
		break;

	case ACPI_GPE_TYPE_WAKE_RUN:

		ACPI_SET_BIT(gpe_event_info->flags, ACPI_GPE_WAKE_ENABLED);

		

	case ACPI_GPE_TYPE_RUNTIME:

		ACPI_SET_BIT(gpe_event_info->flags, ACPI_GPE_RUN_ENABLED);

		if (write_to_hardware) {

			

			status = acpi_hw_clear_gpe(gpe_event_info);
			if (ACPI_FAILURE(status)) {
				return_ACPI_STATUS(status);
			}

			

			status = acpi_hw_write_gpe_enable_reg(gpe_event_info);
		}
		break;

	default:
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	return_ACPI_STATUS(AE_OK);
}



acpi_status acpi_ev_disable_gpe(struct acpi_gpe_event_info *gpe_event_info)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_disable_gpe);

	

	status =
	    acpi_ev_update_gpe_enable_masks(gpe_event_info, ACPI_GPE_DISABLE);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	switch (gpe_event_info->flags & ACPI_GPE_TYPE_MASK) {
	case ACPI_GPE_TYPE_WAKE:
		ACPI_CLEAR_BIT(gpe_event_info->flags, ACPI_GPE_WAKE_ENABLED);
		break;

	case ACPI_GPE_TYPE_WAKE_RUN:
		ACPI_CLEAR_BIT(gpe_event_info->flags, ACPI_GPE_WAKE_ENABLED);

		

	case ACPI_GPE_TYPE_RUNTIME:

		

		ACPI_CLEAR_BIT(gpe_event_info->flags, ACPI_GPE_RUN_ENABLED);
		break;

	default:
		break;
	}

	
	status = acpi_hw_low_disable_gpe(gpe_event_info);
	return_ACPI_STATUS(status);
}



struct acpi_gpe_event_info *acpi_ev_get_gpe_event_info(acpi_handle gpe_device,
						       u32 gpe_number)
{
	union acpi_operand_object *obj_desc;
	struct acpi_gpe_block_info *gpe_block;
	u32 i;

	ACPI_FUNCTION_ENTRY();

	

	if (!gpe_device) {

		

		for (i = 0; i < ACPI_MAX_GPE_BLOCKS; i++) {
			gpe_block = acpi_gbl_gpe_fadt_blocks[i];
			if (gpe_block) {
				if ((gpe_number >= gpe_block->block_base_number)
				    && (gpe_number <
					gpe_block->block_base_number +
					(gpe_block->register_count * 8))) {
					return (&gpe_block->
						event_info[gpe_number -
							   gpe_block->
							   block_base_number]);
				}
			}
		}

		

		return (NULL);
	}

	

	obj_desc = acpi_ns_get_attached_object((struct acpi_namespace_node *)
					       gpe_device);
	if (!obj_desc || !obj_desc->device.gpe_block) {
		return (NULL);
	}

	gpe_block = obj_desc->device.gpe_block;

	if ((gpe_number >= gpe_block->block_base_number) &&
	    (gpe_number <
	     gpe_block->block_base_number + (gpe_block->register_count * 8))) {
		return (&gpe_block->
			event_info[gpe_number - gpe_block->block_base_number]);
	}

	return (NULL);
}



u32 acpi_ev_gpe_detect(struct acpi_gpe_xrupt_info * gpe_xrupt_list)
{
	acpi_status status;
	struct acpi_gpe_block_info *gpe_block;
	struct acpi_gpe_register_info *gpe_register_info;
	u32 int_status = ACPI_INTERRUPT_NOT_HANDLED;
	u8 enabled_status_byte;
	u32 status_reg;
	u32 enable_reg;
	acpi_cpu_flags flags;
	u32 i;
	u32 j;

	ACPI_FUNCTION_NAME(ev_gpe_detect);

	

	if (!gpe_xrupt_list) {
		return (int_status);
	}

	
	flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);

	

	gpe_block = gpe_xrupt_list->gpe_block_list_head;
	while (gpe_block) {
		
		for (i = 0; i < gpe_block->register_count; i++) {

			

			gpe_register_info = &gpe_block->register_info[i];

			

			status =
			    acpi_hw_read(&status_reg,
					 &gpe_register_info->status_address);
			if (ACPI_FAILURE(status)) {
				goto unlock_and_exit;
			}

			

			status =
			    acpi_hw_read(&enable_reg,
					 &gpe_register_info->enable_address);
			if (ACPI_FAILURE(status)) {
				goto unlock_and_exit;
			}

			ACPI_DEBUG_PRINT((ACPI_DB_INTERRUPTS,
					  "Read GPE Register at GPE%X: Status=%02X, Enable=%02X\n",
					  gpe_register_info->base_gpe_number,
					  status_reg, enable_reg));

			

			enabled_status_byte = (u8) (status_reg & enable_reg);
			if (!enabled_status_byte) {

				

				continue;
			}

			

			for (j = 0; j < ACPI_GPE_REGISTER_WIDTH; j++) {

				

				if (enabled_status_byte & (1 << j)) {
					
					int_status |=
					    acpi_ev_gpe_dispatch(&gpe_block->
						event_info[((acpi_size) i * ACPI_GPE_REGISTER_WIDTH) + j], j + gpe_register_info->base_gpe_number);
				}
			}
		}

		gpe_block = gpe_block->next;
	}

      unlock_and_exit:

	acpi_os_release_lock(acpi_gbl_gpe_lock, flags);
	return (int_status);
}


static void acpi_ev_asynch_enable_gpe(void *context);

static void ACPI_SYSTEM_XFACE acpi_ev_asynch_execute_gpe_method(void *context)
{
	struct acpi_gpe_event_info *gpe_event_info = (void *)context;
	acpi_status status;
	struct acpi_gpe_event_info local_gpe_event_info;
	struct acpi_evaluate_info *info;

	ACPI_FUNCTION_TRACE(ev_asynch_execute_gpe_method);

	status = acpi_ut_acquire_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		return_VOID;
	}

	

	if (!acpi_ev_valid_gpe_event(gpe_event_info)) {
		status = acpi_ut_release_mutex(ACPI_MTX_EVENTS);
		return_VOID;
	}

	

	(void)acpi_ev_enable_gpe(gpe_event_info, FALSE);

	
	ACPI_MEMCPY(&local_gpe_event_info, gpe_event_info,
		    sizeof(struct acpi_gpe_event_info));

	status = acpi_ut_release_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		return_VOID;
	}

	
	if ((local_gpe_event_info.flags & ACPI_GPE_DISPATCH_MASK) ==
	    ACPI_GPE_DISPATCH_METHOD) {

		

		info = ACPI_ALLOCATE_ZEROED(sizeof(struct acpi_evaluate_info));
		if (!info) {
			status = AE_NO_MEMORY;
		} else {
			
			info->prefix_node =
			    local_gpe_event_info.dispatch.method_node;
			info->flags = ACPI_IGNORE_RETURN_VALUE;

			status = acpi_ns_evaluate(info);
			ACPI_FREE(info);
		}

		if (ACPI_FAILURE(status)) {
			ACPI_EXCEPTION((AE_INFO, status,
					"while evaluating GPE method [%4.4s]",
					acpi_ut_get_node_name
					(local_gpe_event_info.dispatch.
					 method_node)));
		}
	}
	
	acpi_os_execute(OSL_NOTIFY_HANDLER, acpi_ev_asynch_enable_gpe,
				gpe_event_info);
	return_VOID;
}

static void acpi_ev_asynch_enable_gpe(void *context)
{
	struct acpi_gpe_event_info *gpe_event_info = context;
	acpi_status status;
	if ((gpe_event_info->flags & ACPI_GPE_XRUPT_TYPE_MASK) ==
	    ACPI_GPE_LEVEL_TRIGGERED) {
		
		status = acpi_hw_clear_gpe(gpe_event_info);
		if (ACPI_FAILURE(status)) {
			return_VOID;
		}
	}

	
	(void)acpi_hw_write_gpe_enable_reg(gpe_event_info);
	return_VOID;
}



u32
acpi_ev_gpe_dispatch(struct acpi_gpe_event_info *gpe_event_info, u32 gpe_number)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_gpe_dispatch);

	acpi_os_gpe_count(gpe_number);

	
	if ((gpe_event_info->flags & ACPI_GPE_XRUPT_TYPE_MASK) ==
	    ACPI_GPE_EDGE_TRIGGERED) {
		status = acpi_hw_clear_gpe(gpe_event_info);
		if (ACPI_FAILURE(status)) {
			ACPI_EXCEPTION((AE_INFO, status,
					"Unable to clear GPE[%2X]",
					gpe_number));
			return_UINT32(ACPI_INTERRUPT_NOT_HANDLED);
		}
	}

	
	switch (gpe_event_info->flags & ACPI_GPE_DISPATCH_MASK) {
	case ACPI_GPE_DISPATCH_HANDLER:

		
		(void)gpe_event_info->dispatch.handler->address(gpe_event_info->
								dispatch.
								handler->
								context);

		

		if ((gpe_event_info->flags & ACPI_GPE_XRUPT_TYPE_MASK) ==
		    ACPI_GPE_LEVEL_TRIGGERED) {
			status = acpi_hw_clear_gpe(gpe_event_info);
			if (ACPI_FAILURE(status)) {
				ACPI_EXCEPTION((AE_INFO, status,
						"Unable to clear GPE[%2X]",
						gpe_number));
				return_UINT32(ACPI_INTERRUPT_NOT_HANDLED);
			}
		}
		break;

	case ACPI_GPE_DISPATCH_METHOD:

		
		status = acpi_ev_disable_gpe(gpe_event_info);
		if (ACPI_FAILURE(status)) {
			ACPI_EXCEPTION((AE_INFO, status,
					"Unable to disable GPE[%2X]",
					gpe_number));
			return_UINT32(ACPI_INTERRUPT_NOT_HANDLED);
		}

		
		status = acpi_os_execute(OSL_GPE_HANDLER,
					 acpi_ev_asynch_execute_gpe_method,
					 gpe_event_info);
		if (ACPI_FAILURE(status)) {
			ACPI_EXCEPTION((AE_INFO, status,
					"Unable to queue handler for GPE[%2X] - event disabled",
					gpe_number));
		}
		break;

	default:

		

		ACPI_ERROR((AE_INFO,
			    "No handler or method for GPE[%2X], disabling event",
			    gpe_number));

		
		status = acpi_ev_disable_gpe(gpe_event_info);
		if (ACPI_FAILURE(status)) {
			ACPI_EXCEPTION((AE_INFO, status,
					"Unable to disable GPE[%2X]",
					gpe_number));
			return_UINT32(ACPI_INTERRUPT_NOT_HANDLED);
		}
		break;
	}

	return_UINT32(ACPI_INTERRUPT_HANDLED);
}
