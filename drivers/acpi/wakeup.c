

#include <linux/init.h>
#include <linux/acpi.h>
#include <acpi/acpi_drivers.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "internal.h"
#include "sleep.h"


#define _COMPONENT		ACPI_SYSTEM_COMPONENT
ACPI_MODULE_NAME("wakeup_devices")



void acpi_enable_wakeup_device_prep(u8 sleep_state)
{
	struct list_head *node, *next;

	list_for_each_safe(node, next, &acpi_wakeup_device_list) {
		struct acpi_device *dev = container_of(node,
						       struct acpi_device,
						       wakeup_list);

		if (!dev->wakeup.flags.valid ||
		    !dev->wakeup.state.enabled ||
		    (sleep_state > (u32) dev->wakeup.sleep_state))
			continue;

		acpi_enable_wakeup_device_power(dev, sleep_state);
	}
}


void acpi_enable_wakeup_device(u8 sleep_state)
{
	struct list_head *node, *next;

	
	list_for_each_safe(node, next, &acpi_wakeup_device_list) {
		struct acpi_device *dev =
			container_of(node, struct acpi_device, wakeup_list);

		if (!dev->wakeup.flags.valid)
			continue;

		
		if ((!dev->wakeup.state.enabled && !dev->wakeup.prepare_count)
		    || sleep_state > (u32) dev->wakeup.sleep_state) {
			if (dev->wakeup.flags.run_wake) {
				
				acpi_set_gpe_type(dev->wakeup.gpe_device,
						  dev->wakeup.gpe_number,
						  ACPI_GPE_TYPE_RUNTIME);
			}
			continue;
		}
		if (!dev->wakeup.flags.run_wake)
			acpi_enable_gpe(dev->wakeup.gpe_device,
					dev->wakeup.gpe_number);
	}
}


void acpi_disable_wakeup_device(u8 sleep_state)
{
	struct list_head *node, *next;

	list_for_each_safe(node, next, &acpi_wakeup_device_list) {
		struct acpi_device *dev =
			container_of(node, struct acpi_device, wakeup_list);

		if (!dev->wakeup.flags.valid)
			continue;

		if ((!dev->wakeup.state.enabled && !dev->wakeup.prepare_count)
		    || sleep_state > (u32) dev->wakeup.sleep_state) {
			if (dev->wakeup.flags.run_wake) {
				acpi_set_gpe_type(dev->wakeup.gpe_device,
						  dev->wakeup.gpe_number,
						  ACPI_GPE_TYPE_WAKE_RUN);
				
				acpi_enable_gpe(dev->wakeup.gpe_device,
						dev->wakeup.gpe_number);
			}
			continue;
		}

		acpi_disable_wakeup_device_power(dev);
		
		if (!dev->wakeup.flags.run_wake) {
			acpi_disable_gpe(dev->wakeup.gpe_device,
					 dev->wakeup.gpe_number);
			acpi_clear_gpe(dev->wakeup.gpe_device,
				       dev->wakeup.gpe_number, ACPI_NOT_ISR);
		}
	}
}

int __init acpi_wakeup_device_init(void)
{
	struct list_head *node, *next;

	mutex_lock(&acpi_device_lock);
	list_for_each_safe(node, next, &acpi_wakeup_device_list) {
		struct acpi_device *dev = container_of(node,
						       struct acpi_device,
						       wakeup_list);
		
		if (!dev->wakeup.flags.run_wake || dev->wakeup.state.enabled)
			continue;
		acpi_set_gpe_type(dev->wakeup.gpe_device,
				  dev->wakeup.gpe_number,
				  ACPI_GPE_TYPE_WAKE_RUN);
		acpi_enable_gpe(dev->wakeup.gpe_device,
				dev->wakeup.gpe_number);
		dev->wakeup.state.enabled = 1;
	}
	mutex_unlock(&acpi_device_lock);
	return 0;
}
