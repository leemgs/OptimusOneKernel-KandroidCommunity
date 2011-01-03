




#include <acpi/acpi.h>
#include "accommon.h"
#include "acevents.h"

#define _COMPONENT          ACPI_HARDWARE
ACPI_MODULE_NAME("hwgpe")


static acpi_status
acpi_hw_enable_wakeup_gpe_block(struct acpi_gpe_xrupt_info *gpe_xrupt_info,
				struct acpi_gpe_block_info *gpe_block,
				void *context);



acpi_status acpi_hw_low_disable_gpe(struct acpi_gpe_event_info *gpe_event_info)
{
	struct acpi_gpe_register_info *gpe_register_info;
	acpi_status status;
	u32 enable_mask;

	

	gpe_register_info = gpe_event_info->register_info;
	if (!gpe_register_info) {
		return (AE_NOT_EXIST);
	}

	

	status = acpi_hw_read(&enable_mask, &gpe_register_info->enable_address);
	if (ACPI_FAILURE(status)) {
		return (status);
	}

	

	ACPI_CLEAR_BIT(enable_mask, ((u32)1 <<
				     (gpe_event_info->gpe_number -
				      gpe_register_info->base_gpe_number)));

	

	status = acpi_hw_write(enable_mask, &gpe_register_info->enable_address);
	return (status);
}



acpi_status
acpi_hw_write_gpe_enable_reg(struct acpi_gpe_event_info * gpe_event_info)
{
	struct acpi_gpe_register_info *gpe_register_info;
	acpi_status status;

	ACPI_FUNCTION_ENTRY();

	

	gpe_register_info = gpe_event_info->register_info;
	if (!gpe_register_info) {
		return (AE_NOT_EXIST);
	}

	

	status = acpi_hw_write(gpe_register_info->enable_for_run,
			       &gpe_register_info->enable_address);

	return (status);
}



acpi_status acpi_hw_clear_gpe(struct acpi_gpe_event_info * gpe_event_info)
{
	acpi_status status;
	u8 register_bit;

	ACPI_FUNCTION_ENTRY();

	register_bit = (u8)(1 <<
			    (gpe_event_info->gpe_number -
			     gpe_event_info->register_info->base_gpe_number));

	
	status = acpi_hw_write(register_bit,
			       &gpe_event_info->register_info->status_address);

	return (status);
}



acpi_status
acpi_hw_get_gpe_status(struct acpi_gpe_event_info * gpe_event_info,
		       acpi_event_status * event_status)
{
	u32 in_byte;
	u8 register_bit;
	struct acpi_gpe_register_info *gpe_register_info;
	acpi_status status;
	acpi_event_status local_event_status = 0;

	ACPI_FUNCTION_ENTRY();

	if (!event_status) {
		return (AE_BAD_PARAMETER);
	}

	

	gpe_register_info = gpe_event_info->register_info;

	

	register_bit = (u8)(1 <<
			    (gpe_event_info->gpe_number -
			     gpe_event_info->register_info->base_gpe_number));

	

	if (register_bit & gpe_register_info->enable_for_run) {
		local_event_status |= ACPI_EVENT_FLAG_ENABLED;
	}

	

	if (register_bit & gpe_register_info->enable_for_wake) {
		local_event_status |= ACPI_EVENT_FLAG_WAKE_ENABLED;
	}

	

	status = acpi_hw_read(&in_byte, &gpe_register_info->status_address);
	if (ACPI_FAILURE(status)) {
		goto unlock_and_exit;
	}

	if (register_bit & in_byte) {
		local_event_status |= ACPI_EVENT_FLAG_SET;
	}

	

	(*event_status) = local_event_status;

      unlock_and_exit:
	return (status);
}



acpi_status
acpi_hw_disable_gpe_block(struct acpi_gpe_xrupt_info *gpe_xrupt_info,
			  struct acpi_gpe_block_info *gpe_block, void *context)
{
	u32 i;
	acpi_status status;

	

	for (i = 0; i < gpe_block->register_count; i++) {

		

		status =
		    acpi_hw_write(0x00,
				  &gpe_block->register_info[i].enable_address);
		if (ACPI_FAILURE(status)) {
			return (status);
		}
	}

	return (AE_OK);
}



acpi_status
acpi_hw_clear_gpe_block(struct acpi_gpe_xrupt_info *gpe_xrupt_info,
			struct acpi_gpe_block_info *gpe_block, void *context)
{
	u32 i;
	acpi_status status;

	

	for (i = 0; i < gpe_block->register_count; i++) {

		

		status =
		    acpi_hw_write(0xFF,
				  &gpe_block->register_info[i].status_address);
		if (ACPI_FAILURE(status)) {
			return (status);
		}
	}

	return (AE_OK);
}



acpi_status
acpi_hw_enable_runtime_gpe_block(struct acpi_gpe_xrupt_info *gpe_xrupt_info,
				 struct acpi_gpe_block_info *gpe_block, void *context)
{
	u32 i;
	acpi_status status;

	

	

	for (i = 0; i < gpe_block->register_count; i++) {
		if (!gpe_block->register_info[i].enable_for_run) {
			continue;
		}

		

		status =
		    acpi_hw_write(gpe_block->register_info[i].enable_for_run,
				  &gpe_block->register_info[i].enable_address);
		if (ACPI_FAILURE(status)) {
			return (status);
		}
	}

	return (AE_OK);
}



static acpi_status
acpi_hw_enable_wakeup_gpe_block(struct acpi_gpe_xrupt_info *gpe_xrupt_info,
				struct acpi_gpe_block_info *gpe_block,
				void *context)
{
	u32 i;
	acpi_status status;

	

	for (i = 0; i < gpe_block->register_count; i++) {
		if (!gpe_block->register_info[i].enable_for_wake) {
			continue;
		}

		

		status =
		    acpi_hw_write(gpe_block->register_info[i].enable_for_wake,
				  &gpe_block->register_info[i].enable_address);
		if (ACPI_FAILURE(status)) {
			return (status);
		}
	}

	return (AE_OK);
}



acpi_status acpi_hw_disable_all_gpes(void)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(hw_disable_all_gpes);

	status = acpi_ev_walk_gpe_list(acpi_hw_disable_gpe_block, NULL);
	status = acpi_ev_walk_gpe_list(acpi_hw_clear_gpe_block, NULL);
	return_ACPI_STATUS(status);
}



acpi_status acpi_hw_enable_all_runtime_gpes(void)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(hw_enable_all_runtime_gpes);

	status = acpi_ev_walk_gpe_list(acpi_hw_enable_runtime_gpe_block, NULL);
	return_ACPI_STATUS(status);
}



acpi_status acpi_hw_enable_all_wakeup_gpes(void)
{
	acpi_status status;

	ACPI_FUNCTION_TRACE(hw_enable_all_wakeup_gpes);

	status = acpi_ev_walk_gpe_list(acpi_hw_enable_wakeup_gpe_block, NULL);
	return_ACPI_STATUS(status);
}
