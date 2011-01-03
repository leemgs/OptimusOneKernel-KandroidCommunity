



#include <acpi/acpi.h>
#include "accommon.h"
#include "acevents.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_EVENTS
ACPI_MODULE_NAME("evgpeblk")


static acpi_status
acpi_ev_save_method_info(acpi_handle obj_handle,
			 u32 level, void *obj_desc, void **return_value);

static acpi_status
acpi_ev_match_prw_and_gpe(acpi_handle obj_handle,
			  u32 level, void *info, void **return_value);

static struct acpi_gpe_xrupt_info *acpi_ev_get_gpe_xrupt_block(u32
							       interrupt_number);

static acpi_status
acpi_ev_delete_gpe_xrupt(struct acpi_gpe_xrupt_info *gpe_xrupt);

static acpi_status
acpi_ev_install_gpe_block(struct acpi_gpe_block_info *gpe_block,
			  u32 interrupt_number);

static acpi_status
acpi_ev_create_gpe_info_blocks(struct acpi_gpe_block_info *gpe_block);



u8 acpi_ev_valid_gpe_event(struct acpi_gpe_event_info *gpe_event_info)
{
	struct acpi_gpe_xrupt_info *gpe_xrupt_block;
	struct acpi_gpe_block_info *gpe_block;

	ACPI_FUNCTION_ENTRY();

	

	

	gpe_xrupt_block = acpi_gbl_gpe_xrupt_list_head;
	while (gpe_xrupt_block) {
		gpe_block = gpe_xrupt_block->gpe_block_list_head;

		

		while (gpe_block) {
			if ((&gpe_block->event_info[0] <= gpe_event_info) &&
			    (&gpe_block->event_info[((acpi_size)
						     gpe_block->
						     register_count) * 8] >
			     gpe_event_info)) {
				return (TRUE);
			}

			gpe_block = gpe_block->next;
		}

		gpe_xrupt_block = gpe_xrupt_block->next;
	}

	return (FALSE);
}



acpi_status
acpi_ev_walk_gpe_list(acpi_gpe_callback gpe_walk_callback, void *context)
{
	struct acpi_gpe_block_info *gpe_block;
	struct acpi_gpe_xrupt_info *gpe_xrupt_info;
	acpi_status status = AE_OK;
	acpi_cpu_flags flags;

	ACPI_FUNCTION_TRACE(ev_walk_gpe_list);

	flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);

	

	gpe_xrupt_info = acpi_gbl_gpe_xrupt_list_head;
	while (gpe_xrupt_info) {

		

		gpe_block = gpe_xrupt_info->gpe_block_list_head;
		while (gpe_block) {

			

			status =
			    gpe_walk_callback(gpe_xrupt_info, gpe_block,
					      context);
			if (ACPI_FAILURE(status)) {
				if (status == AE_CTRL_END) {	
					status = AE_OK;
				}
				goto unlock_and_exit;
			}

			gpe_block = gpe_block->next;
		}

		gpe_xrupt_info = gpe_xrupt_info->next;
	}

      unlock_and_exit:
	acpi_os_release_lock(acpi_gbl_gpe_lock, flags);
	return_ACPI_STATUS(status);
}



acpi_status
acpi_ev_delete_gpe_handlers(struct acpi_gpe_xrupt_info *gpe_xrupt_info,
			    struct acpi_gpe_block_info *gpe_block,
			    void *context)
{
	struct acpi_gpe_event_info *gpe_event_info;
	u32 i;
	u32 j;

	ACPI_FUNCTION_TRACE(ev_delete_gpe_handlers);

	

	for (i = 0; i < gpe_block->register_count; i++) {

		

		for (j = 0; j < ACPI_GPE_REGISTER_WIDTH; j++) {
			gpe_event_info = &gpe_block->event_info[((acpi_size) i *
								 ACPI_GPE_REGISTER_WIDTH)
								+ j];

			if ((gpe_event_info->flags & ACPI_GPE_DISPATCH_MASK) ==
			    ACPI_GPE_DISPATCH_HANDLER) {
				ACPI_FREE(gpe_event_info->dispatch.handler);
				gpe_event_info->dispatch.handler = NULL;
				gpe_event_info->flags &=
				    ~ACPI_GPE_DISPATCH_MASK;
			}
		}
	}

	return_ACPI_STATUS(AE_OK);
}



static acpi_status
acpi_ev_save_method_info(acpi_handle obj_handle,
			 u32 level, void *obj_desc, void **return_value)
{
	struct acpi_gpe_block_info *gpe_block = (void *)obj_desc;
	struct acpi_gpe_event_info *gpe_event_info;
	u32 gpe_number;
	char name[ACPI_NAME_SIZE + 1];
	u8 type;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_save_method_info);

	
	ACPI_MOVE_32_TO_32(name,
			   &((struct acpi_namespace_node *)obj_handle)->name.
			   integer);
	name[ACPI_NAME_SIZE] = 0;

	
	switch (name[1]) {
	case 'L':
		type = ACPI_GPE_LEVEL_TRIGGERED;
		break;

	case 'E':
		type = ACPI_GPE_EDGE_TRIGGERED;
		break;

	default:
		

		ACPI_DEBUG_PRINT((ACPI_DB_LOAD,
				  "Ignoring unknown GPE method type: %s "
				  "(name not of form _Lxx or _Exx)", name));
		return_ACPI_STATUS(AE_OK);
	}

	

	gpe_number = ACPI_STRTOUL(&name[2], NULL, 16);
	if (gpe_number == ACPI_UINT32_MAX) {

		

		ACPI_DEBUG_PRINT((ACPI_DB_LOAD,
				  "Could not extract GPE number from name: %s "
				  "(name is not of form _Lxx or _Exx)", name));
		return_ACPI_STATUS(AE_OK);
	}

	

	if ((gpe_number < gpe_block->block_base_number) ||
	    (gpe_number >= (gpe_block->block_base_number +
			    (gpe_block->register_count * 8)))) {
		
		return_ACPI_STATUS(AE_OK);
	}

	
	gpe_event_info =
	    &gpe_block->event_info[gpe_number - gpe_block->block_base_number];

	gpe_event_info->flags = (u8)
	    (type | ACPI_GPE_DISPATCH_METHOD | ACPI_GPE_TYPE_RUNTIME);

	gpe_event_info->dispatch.method_node =
	    (struct acpi_namespace_node *)obj_handle;

	

	status = acpi_ev_enable_gpe(gpe_event_info, FALSE);

	ACPI_DEBUG_PRINT((ACPI_DB_LOAD,
			  "Registered GPE method %s as GPE number 0x%.2X\n",
			  name, gpe_number));
	return_ACPI_STATUS(status);
}



static acpi_status
acpi_ev_match_prw_and_gpe(acpi_handle obj_handle,
			  u32 level, void *info, void **return_value)
{
	struct acpi_gpe_walk_info *gpe_info = (void *)info;
	struct acpi_namespace_node *gpe_device;
	struct acpi_gpe_block_info *gpe_block;
	struct acpi_namespace_node *target_gpe_device;
	struct acpi_gpe_event_info *gpe_event_info;
	union acpi_operand_object *pkg_desc;
	union acpi_operand_object *obj_desc;
	u32 gpe_number;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_match_prw_and_gpe);

	

	status = acpi_ut_evaluate_object(obj_handle, METHOD_NAME__PRW,
					 ACPI_BTYPE_PACKAGE, &pkg_desc);
	if (ACPI_FAILURE(status)) {

		

		return_ACPI_STATUS(AE_OK);
	}

	

	if (pkg_desc->package.count < 2) {
		goto cleanup;
	}

	

	gpe_device = gpe_info->gpe_device;
	gpe_block = gpe_info->gpe_block;

	
	obj_desc = pkg_desc->package.elements[0];

	if (obj_desc->common.type == ACPI_TYPE_INTEGER) {

		

		target_gpe_device = acpi_gbl_fadt_gpe_device;

		

		gpe_number = (u32) obj_desc->integer.value;
	} else if (obj_desc->common.type == ACPI_TYPE_PACKAGE) {

		

		if ((obj_desc->package.count < 2) ||
		    ((obj_desc->package.elements[0])->common.type !=
		     ACPI_TYPE_LOCAL_REFERENCE) ||
		    ((obj_desc->package.elements[1])->common.type !=
		     ACPI_TYPE_INTEGER)) {
			goto cleanup;
		}

		

		target_gpe_device =
		    obj_desc->package.elements[0]->reference.node;
		gpe_number = (u32) obj_desc->package.elements[1]->integer.value;
	} else {
		

		goto cleanup;
	}

	
	if ((gpe_device == target_gpe_device) &&
	    (gpe_number >= gpe_block->block_base_number) &&
	    (gpe_number < gpe_block->block_base_number +
	     (gpe_block->register_count * 8))) {
		gpe_event_info = &gpe_block->event_info[gpe_number -
							gpe_block->
							block_base_number];

		

		gpe_event_info->flags &=
		    ~(ACPI_GPE_WAKE_ENABLED | ACPI_GPE_RUN_ENABLED);

		status =
		    acpi_ev_set_gpe_type(gpe_event_info, ACPI_GPE_TYPE_WAKE);
		if (ACPI_FAILURE(status)) {
			goto cleanup;
		}

		status =
		    acpi_ev_update_gpe_enable_masks(gpe_event_info,
						    ACPI_GPE_DISABLE);
	}

      cleanup:
	acpi_ut_remove_reference(pkg_desc);
	return_ACPI_STATUS(AE_OK);
}



static struct acpi_gpe_xrupt_info *acpi_ev_get_gpe_xrupt_block(u32
							       interrupt_number)
{
	struct acpi_gpe_xrupt_info *next_gpe_xrupt;
	struct acpi_gpe_xrupt_info *gpe_xrupt;
	acpi_status status;
	acpi_cpu_flags flags;

	ACPI_FUNCTION_TRACE(ev_get_gpe_xrupt_block);

	

	next_gpe_xrupt = acpi_gbl_gpe_xrupt_list_head;
	while (next_gpe_xrupt) {
		if (next_gpe_xrupt->interrupt_number == interrupt_number) {
			return_PTR(next_gpe_xrupt);
		}

		next_gpe_xrupt = next_gpe_xrupt->next;
	}

	

	gpe_xrupt = ACPI_ALLOCATE_ZEROED(sizeof(struct acpi_gpe_xrupt_info));
	if (!gpe_xrupt) {
		return_PTR(NULL);
	}

	gpe_xrupt->interrupt_number = interrupt_number;

	

	flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);
	if (acpi_gbl_gpe_xrupt_list_head) {
		next_gpe_xrupt = acpi_gbl_gpe_xrupt_list_head;
		while (next_gpe_xrupt->next) {
			next_gpe_xrupt = next_gpe_xrupt->next;
		}

		next_gpe_xrupt->next = gpe_xrupt;
		gpe_xrupt->previous = next_gpe_xrupt;
	} else {
		acpi_gbl_gpe_xrupt_list_head = gpe_xrupt;
	}
	acpi_os_release_lock(acpi_gbl_gpe_lock, flags);

	

	if (interrupt_number != acpi_gbl_FADT.sci_interrupt) {
		status = acpi_os_install_interrupt_handler(interrupt_number,
							   acpi_ev_gpe_xrupt_handler,
							   gpe_xrupt);
		if (ACPI_FAILURE(status)) {
			ACPI_ERROR((AE_INFO,
				    "Could not install GPE interrupt handler at level 0x%X",
				    interrupt_number));
			return_PTR(NULL);
		}
	}

	return_PTR(gpe_xrupt);
}



static acpi_status
acpi_ev_delete_gpe_xrupt(struct acpi_gpe_xrupt_info *gpe_xrupt)
{
	acpi_status status;
	acpi_cpu_flags flags;

	ACPI_FUNCTION_TRACE(ev_delete_gpe_xrupt);

	

	if (gpe_xrupt->interrupt_number == acpi_gbl_FADT.sci_interrupt) {
		gpe_xrupt->gpe_block_list_head = NULL;
		return_ACPI_STATUS(AE_OK);
	}

	

	status =
	    acpi_os_remove_interrupt_handler(gpe_xrupt->interrupt_number,
					     acpi_ev_gpe_xrupt_handler);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);
	if (gpe_xrupt->previous) {
		gpe_xrupt->previous->next = gpe_xrupt->next;
	} else {
		

		acpi_gbl_gpe_xrupt_list_head = gpe_xrupt->next;
	}

	if (gpe_xrupt->next) {
		gpe_xrupt->next->previous = gpe_xrupt->previous;
	}
	acpi_os_release_lock(acpi_gbl_gpe_lock, flags);

	

	ACPI_FREE(gpe_xrupt);
	return_ACPI_STATUS(AE_OK);
}



static acpi_status
acpi_ev_install_gpe_block(struct acpi_gpe_block_info *gpe_block,
			  u32 interrupt_number)
{
	struct acpi_gpe_block_info *next_gpe_block;
	struct acpi_gpe_xrupt_info *gpe_xrupt_block;
	acpi_status status;
	acpi_cpu_flags flags;

	ACPI_FUNCTION_TRACE(ev_install_gpe_block);

	status = acpi_ut_acquire_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	gpe_xrupt_block = acpi_ev_get_gpe_xrupt_block(interrupt_number);
	if (!gpe_xrupt_block) {
		status = AE_NO_MEMORY;
		goto unlock_and_exit;
	}

	

	flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);
	if (gpe_xrupt_block->gpe_block_list_head) {
		next_gpe_block = gpe_xrupt_block->gpe_block_list_head;
		while (next_gpe_block->next) {
			next_gpe_block = next_gpe_block->next;
		}

		next_gpe_block->next = gpe_block;
		gpe_block->previous = next_gpe_block;
	} else {
		gpe_xrupt_block->gpe_block_list_head = gpe_block;
	}

	gpe_block->xrupt_block = gpe_xrupt_block;
	acpi_os_release_lock(acpi_gbl_gpe_lock, flags);

      unlock_and_exit:
	status = acpi_ut_release_mutex(ACPI_MTX_EVENTS);
	return_ACPI_STATUS(status);
}



acpi_status acpi_ev_delete_gpe_block(struct acpi_gpe_block_info *gpe_block)
{
	acpi_status status;
	acpi_cpu_flags flags;

	ACPI_FUNCTION_TRACE(ev_install_gpe_block);

	status = acpi_ut_acquire_mutex(ACPI_MTX_EVENTS);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	status =
	    acpi_hw_disable_gpe_block(gpe_block->xrupt_block, gpe_block, NULL);

	if (!gpe_block->previous && !gpe_block->next) {

		

		status = acpi_ev_delete_gpe_xrupt(gpe_block->xrupt_block);
		if (ACPI_FAILURE(status)) {
			goto unlock_and_exit;
		}
	} else {
		

		flags = acpi_os_acquire_lock(acpi_gbl_gpe_lock);
		if (gpe_block->previous) {
			gpe_block->previous->next = gpe_block->next;
		} else {
			gpe_block->xrupt_block->gpe_block_list_head =
			    gpe_block->next;
		}

		if (gpe_block->next) {
			gpe_block->next->previous = gpe_block->previous;
		}
		acpi_os_release_lock(acpi_gbl_gpe_lock, flags);
	}

	acpi_current_gpe_count -=
	    gpe_block->register_count * ACPI_GPE_REGISTER_WIDTH;

	

	ACPI_FREE(gpe_block->register_info);
	ACPI_FREE(gpe_block->event_info);
	ACPI_FREE(gpe_block);

      unlock_and_exit:
	status = acpi_ut_release_mutex(ACPI_MTX_EVENTS);
	return_ACPI_STATUS(status);
}



static acpi_status
acpi_ev_create_gpe_info_blocks(struct acpi_gpe_block_info *gpe_block)
{
	struct acpi_gpe_register_info *gpe_register_info = NULL;
	struct acpi_gpe_event_info *gpe_event_info = NULL;
	struct acpi_gpe_event_info *this_event;
	struct acpi_gpe_register_info *this_register;
	u32 i;
	u32 j;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_create_gpe_info_blocks);

	

	gpe_register_info = ACPI_ALLOCATE_ZEROED((acpi_size) gpe_block->
						 register_count *
						 sizeof(struct
							acpi_gpe_register_info));
	if (!gpe_register_info) {
		ACPI_ERROR((AE_INFO,
			    "Could not allocate the GpeRegisterInfo table"));
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	
	gpe_event_info = ACPI_ALLOCATE_ZEROED(((acpi_size) gpe_block->
					       register_count *
					       ACPI_GPE_REGISTER_WIDTH) *
					      sizeof(struct
						     acpi_gpe_event_info));
	if (!gpe_event_info) {
		ACPI_ERROR((AE_INFO,
			    "Could not allocate the GpeEventInfo table"));
		status = AE_NO_MEMORY;
		goto error_exit;
	}

	

	gpe_block->register_info = gpe_register_info;
	gpe_block->event_info = gpe_event_info;

	
	this_register = gpe_register_info;
	this_event = gpe_event_info;

	for (i = 0; i < gpe_block->register_count; i++) {

		

		this_register->base_gpe_number =
		    (u8) (gpe_block->block_base_number +
			  (i * ACPI_GPE_REGISTER_WIDTH));

		this_register->status_address.address =
		    gpe_block->block_address.address + i;

		this_register->enable_address.address =
		    gpe_block->block_address.address + i +
		    gpe_block->register_count;

		this_register->status_address.space_id =
		    gpe_block->block_address.space_id;
		this_register->enable_address.space_id =
		    gpe_block->block_address.space_id;
		this_register->status_address.bit_width =
		    ACPI_GPE_REGISTER_WIDTH;
		this_register->enable_address.bit_width =
		    ACPI_GPE_REGISTER_WIDTH;
		this_register->status_address.bit_offset = 0;
		this_register->enable_address.bit_offset = 0;

		

		for (j = 0; j < ACPI_GPE_REGISTER_WIDTH; j++) {
			this_event->gpe_number =
			    (u8) (this_register->base_gpe_number + j);
			this_event->register_info = this_register;
			this_event++;
		}

		

		status = acpi_hw_write(0x00, &this_register->enable_address);
		if (ACPI_FAILURE(status)) {
			goto error_exit;
		}

		

		status = acpi_hw_write(0xFF, &this_register->status_address);
		if (ACPI_FAILURE(status)) {
			goto error_exit;
		}

		this_register++;
	}

	return_ACPI_STATUS(AE_OK);

      error_exit:
	if (gpe_register_info) {
		ACPI_FREE(gpe_register_info);
	}
	if (gpe_event_info) {
		ACPI_FREE(gpe_event_info);
	}

	return_ACPI_STATUS(status);
}



acpi_status
acpi_ev_create_gpe_block(struct acpi_namespace_node *gpe_device,
			 struct acpi_generic_address *gpe_block_address,
			 u32 register_count,
			 u8 gpe_block_base_number,
			 u32 interrupt_number,
			 struct acpi_gpe_block_info **return_gpe_block)
{
	acpi_status status;
	struct acpi_gpe_block_info *gpe_block;

	ACPI_FUNCTION_TRACE(ev_create_gpe_block);

	if (!register_count) {
		return_ACPI_STATUS(AE_OK);
	}

	

	gpe_block = ACPI_ALLOCATE_ZEROED(sizeof(struct acpi_gpe_block_info));
	if (!gpe_block) {
		return_ACPI_STATUS(AE_NO_MEMORY);
	}

	

	gpe_block->node = gpe_device;
	gpe_block->register_count = register_count;
	gpe_block->block_base_number = gpe_block_base_number;

	ACPI_MEMCPY(&gpe_block->block_address, gpe_block_address,
		    sizeof(struct acpi_generic_address));

	
	status = acpi_ev_create_gpe_info_blocks(gpe_block);
	if (ACPI_FAILURE(status)) {
		ACPI_FREE(gpe_block);
		return_ACPI_STATUS(status);
	}

	

	status = acpi_ev_install_gpe_block(gpe_block, interrupt_number);
	if (ACPI_FAILURE(status)) {
		ACPI_FREE(gpe_block);
		return_ACPI_STATUS(status);
	}

	

	status = acpi_ns_walk_namespace(ACPI_TYPE_METHOD, gpe_device,
					ACPI_UINT32_MAX, ACPI_NS_WALK_NO_UNLOCK,
					acpi_ev_save_method_info, gpe_block,
					NULL);

	

	if (return_gpe_block) {
		(*return_gpe_block) = gpe_block;
	}

	ACPI_DEBUG_PRINT((ACPI_DB_INIT,
			  "GPE %02X to %02X [%4.4s] %u regs on int 0x%X\n",
			  (u32) gpe_block->block_base_number,
			  (u32) (gpe_block->block_base_number +
				 ((gpe_block->register_count *
				   ACPI_GPE_REGISTER_WIDTH) - 1)),
			  gpe_device->name.ascii, gpe_block->register_count,
			  interrupt_number));

	

	acpi_current_gpe_count += register_count * ACPI_GPE_REGISTER_WIDTH;
	return_ACPI_STATUS(AE_OK);
}



acpi_status
acpi_ev_initialize_gpe_block(struct acpi_namespace_node *gpe_device,
			     struct acpi_gpe_block_info *gpe_block)
{
	acpi_status status;
	struct acpi_gpe_event_info *gpe_event_info;
	struct acpi_gpe_walk_info gpe_info;
	u32 wake_gpe_count;
	u32 gpe_enabled_count;
	u32 i;
	u32 j;

	ACPI_FUNCTION_TRACE(ev_initialize_gpe_block);

	

	if (!gpe_block) {
		return_ACPI_STATUS(AE_OK);
	}

	
	if (acpi_gbl_leave_wake_gpes_disabled) {
		
		gpe_info.gpe_block = gpe_block;
		gpe_info.gpe_device = gpe_device;

		status =
		    acpi_ns_walk_namespace(ACPI_TYPE_DEVICE, ACPI_ROOT_OBJECT,
					   ACPI_UINT32_MAX, ACPI_NS_WALK_UNLOCK,
					   acpi_ev_match_prw_and_gpe, &gpe_info,
					   NULL);
	}

	
	wake_gpe_count = 0;
	gpe_enabled_count = 0;

	for (i = 0; i < gpe_block->register_count; i++) {
		for (j = 0; j < 8; j++) {

			

			gpe_event_info = &gpe_block->event_info[((acpi_size) i *
								 ACPI_GPE_REGISTER_WIDTH)
								+ j];

			if (((gpe_event_info->flags & ACPI_GPE_DISPATCH_MASK) ==
			     ACPI_GPE_DISPATCH_METHOD) &&
			    (gpe_event_info->flags & ACPI_GPE_TYPE_RUNTIME)) {
				gpe_enabled_count++;
			}

			if (gpe_event_info->flags & ACPI_GPE_TYPE_WAKE) {
				wake_gpe_count++;
			}
		}
	}

	ACPI_DEBUG_PRINT((ACPI_DB_INIT,
			  "Found %u Wake, Enabled %u Runtime GPEs in this block\n",
			  wake_gpe_count, gpe_enabled_count));

	

	status = acpi_hw_enable_runtime_gpe_block(NULL, gpe_block, NULL);
	if (ACPI_FAILURE(status)) {
		ACPI_ERROR((AE_INFO, "Could not enable GPEs in GpeBlock %p",
			    gpe_block));
	}

	return_ACPI_STATUS(status);
}



acpi_status acpi_ev_gpe_initialize(void)
{
	u32 register_count0 = 0;
	u32 register_count1 = 0;
	u32 gpe_number_max = 0;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ev_gpe_initialize);

	status = acpi_ut_acquire_mutex(ACPI_MTX_NAMESPACE);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	

	
	if (acpi_gbl_FADT.gpe0_block_length &&
	    acpi_gbl_FADT.xgpe0_block.address) {

		

		register_count0 = (u16) (acpi_gbl_FADT.gpe0_block_length / 2);

		gpe_number_max =
		    (register_count0 * ACPI_GPE_REGISTER_WIDTH) - 1;

		

		status = acpi_ev_create_gpe_block(acpi_gbl_fadt_gpe_device,
						  &acpi_gbl_FADT.xgpe0_block,
						  register_count0, 0,
						  acpi_gbl_FADT.sci_interrupt,
						  &acpi_gbl_gpe_fadt_blocks[0]);

		if (ACPI_FAILURE(status)) {
			ACPI_EXCEPTION((AE_INFO, status,
					"Could not create GPE Block 0"));
		}
	}

	if (acpi_gbl_FADT.gpe1_block_length &&
	    acpi_gbl_FADT.xgpe1_block.address) {

		

		register_count1 = (u16) (acpi_gbl_FADT.gpe1_block_length / 2);

		

		if ((register_count0) &&
		    (gpe_number_max >= acpi_gbl_FADT.gpe1_base)) {
			ACPI_ERROR((AE_INFO,
				    "GPE0 block (GPE 0 to %d) overlaps the GPE1 block "
				    "(GPE %d to %d) - Ignoring GPE1",
				    gpe_number_max, acpi_gbl_FADT.gpe1_base,
				    acpi_gbl_FADT.gpe1_base +
				    ((register_count1 *
				      ACPI_GPE_REGISTER_WIDTH) - 1)));

			

			register_count1 = 0;
		} else {
			

			status =
			    acpi_ev_create_gpe_block(acpi_gbl_fadt_gpe_device,
						     &acpi_gbl_FADT.xgpe1_block,
						     register_count1,
						     acpi_gbl_FADT.gpe1_base,
						     acpi_gbl_FADT.
						     sci_interrupt,
						     &acpi_gbl_gpe_fadt_blocks
						     [1]);

			if (ACPI_FAILURE(status)) {
				ACPI_EXCEPTION((AE_INFO, status,
						"Could not create GPE Block 1"));
			}

			
			gpe_number_max = acpi_gbl_FADT.gpe1_base +
			    ((register_count1 * ACPI_GPE_REGISTER_WIDTH) - 1);
		}
	}

	

	if ((register_count0 + register_count1) == 0) {

		

		ACPI_DEBUG_PRINT((ACPI_DB_INIT,
				  "There are no GPE blocks defined in the FADT\n"));
		status = AE_OK;
		goto cleanup;
	}

	

	if (gpe_number_max > ACPI_GPE_MAX) {
		ACPI_ERROR((AE_INFO,
			    "Maximum GPE number from FADT is too large: 0x%X",
			    gpe_number_max));
		status = AE_BAD_VALUE;
		goto cleanup;
	}

      cleanup:
	(void)acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
	return_ACPI_STATUS(AE_OK);
}
