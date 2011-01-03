

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/nmi.h>
#include <linux/acpi.h>
#include <linux/efi.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/jiffies.h>
#include <linux/semaphore.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include <acpi/acpi.h>
#include <acpi/acpi_bus.h>
#include <acpi/processor.h>

#define _COMPONENT		ACPI_OS_SERVICES
ACPI_MODULE_NAME("osl");
#define PREFIX		"ACPI: "
struct acpi_os_dpc {
	acpi_osd_exec_callback function;
	void *context;
	struct work_struct work;
	int wait;
};

#ifdef CONFIG_ACPI_CUSTOM_DSDT
#include CONFIG_ACPI_CUSTOM_DSDT_FILE
#endif

#ifdef ENABLE_DEBUGGER
#include <linux/kdb.h>


int acpi_in_debugger;
EXPORT_SYMBOL(acpi_in_debugger);

extern char line_buf[80];
#endif				

static unsigned int acpi_irq_irq;
static acpi_osd_handler acpi_irq_handler;
static void *acpi_irq_context;
static struct workqueue_struct *kacpid_wq;
static struct workqueue_struct *kacpi_notify_wq;
static struct workqueue_struct *kacpi_hotplug_wq;

struct acpi_res_list {
	resource_size_t start;
	resource_size_t end;
	acpi_adr_space_type resource_type; 
	char name[5];   
	struct list_head resource_list;
	int count;
};

static LIST_HEAD(resource_list_head);
static DEFINE_SPINLOCK(acpi_res_lock);

#define	OSI_STRING_LENGTH_MAX 64	
static char osi_additional_string[OSI_STRING_LENGTH_MAX];



static struct osi_linux {
	unsigned int	enable:1;
	unsigned int	dmi:1;
	unsigned int	cmdline:1;
	unsigned int	known:1;
} osi_linux = { 0, 0, 0, 0};

static void __init acpi_request_region (struct acpi_generic_address *addr,
	unsigned int length, char *desc)
{
	struct resource *res;

	if (!addr->address || !length)
		return;

	if (addr->space_id == ACPI_ADR_SPACE_SYSTEM_IO)
		res = request_region(addr->address, length, desc);
	else if (addr->space_id == ACPI_ADR_SPACE_SYSTEM_MEMORY)
		res = request_mem_region(addr->address, length, desc);
}

static int __init acpi_reserve_resources(void)
{
	acpi_request_region(&acpi_gbl_FADT.xpm1a_event_block, acpi_gbl_FADT.pm1_event_length,
		"ACPI PM1a_EVT_BLK");

	acpi_request_region(&acpi_gbl_FADT.xpm1b_event_block, acpi_gbl_FADT.pm1_event_length,
		"ACPI PM1b_EVT_BLK");

	acpi_request_region(&acpi_gbl_FADT.xpm1a_control_block, acpi_gbl_FADT.pm1_control_length,
		"ACPI PM1a_CNT_BLK");

	acpi_request_region(&acpi_gbl_FADT.xpm1b_control_block, acpi_gbl_FADT.pm1_control_length,
		"ACPI PM1b_CNT_BLK");

	if (acpi_gbl_FADT.pm_timer_length == 4)
		acpi_request_region(&acpi_gbl_FADT.xpm_timer_block, 4, "ACPI PM_TMR");

	acpi_request_region(&acpi_gbl_FADT.xpm2_control_block, acpi_gbl_FADT.pm2_control_length,
		"ACPI PM2_CNT_BLK");

	

	if (!(acpi_gbl_FADT.gpe0_block_length & 0x1))
		acpi_request_region(&acpi_gbl_FADT.xgpe0_block,
			       acpi_gbl_FADT.gpe0_block_length, "ACPI GPE0_BLK");

	if (!(acpi_gbl_FADT.gpe1_block_length & 0x1))
		acpi_request_region(&acpi_gbl_FADT.xgpe1_block,
			       acpi_gbl_FADT.gpe1_block_length, "ACPI GPE1_BLK");

	return 0;
}
device_initcall(acpi_reserve_resources);

acpi_status __init acpi_os_initialize(void)
{
	return AE_OK;
}

static void bind_to_cpu0(struct work_struct *work)
{
	set_cpus_allowed_ptr(current, cpumask_of(0));
	kfree(work);
}

static void bind_workqueue(struct workqueue_struct *wq)
{
	struct work_struct *work;

	work = kzalloc(sizeof(struct work_struct), GFP_KERNEL);
	INIT_WORK(work, bind_to_cpu0);
	queue_work(wq, work);
}

acpi_status acpi_os_initialize1(void)
{
	
	kacpid_wq = create_singlethread_workqueue("kacpid");
	bind_workqueue(kacpid_wq);
	kacpi_notify_wq = create_singlethread_workqueue("kacpi_notify");
	bind_workqueue(kacpi_notify_wq);
	kacpi_hotplug_wq = create_singlethread_workqueue("kacpi_hotplug");
	bind_workqueue(kacpi_hotplug_wq);
	BUG_ON(!kacpid_wq);
	BUG_ON(!kacpi_notify_wq);
	BUG_ON(!kacpi_hotplug_wq);
	return AE_OK;
}

acpi_status acpi_os_terminate(void)
{
	if (acpi_irq_handler) {
		acpi_os_remove_interrupt_handler(acpi_irq_irq,
						 acpi_irq_handler);
	}

	destroy_workqueue(kacpid_wq);
	destroy_workqueue(kacpi_notify_wq);
	destroy_workqueue(kacpi_hotplug_wq);

	return AE_OK;
}

void acpi_os_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	acpi_os_vprintf(fmt, args);
	va_end(args);
}

void acpi_os_vprintf(const char *fmt, va_list args)
{
	static char buffer[512];

	vsprintf(buffer, fmt, args);

#ifdef ENABLE_DEBUGGER
	if (acpi_in_debugger) {
		kdb_printf("%s", buffer);
	} else {
		printk(KERN_CONT "%s", buffer);
	}
#else
	printk(KERN_CONT "%s", buffer);
#endif
}

acpi_physical_address __init acpi_os_get_root_pointer(void)
{
	if (efi_enabled) {
		if (efi.acpi20 != EFI_INVALID_TABLE_ADDR)
			return efi.acpi20;
		else if (efi.acpi != EFI_INVALID_TABLE_ADDR)
			return efi.acpi;
		else {
			printk(KERN_ERR PREFIX
			       "System description tables not found\n");
			return 0;
		}
	} else {
		acpi_physical_address pa = 0;

		acpi_find_root_pointer(&pa);
		return pa;
	}
}

void __iomem *__init_refok
acpi_os_map_memory(acpi_physical_address phys, acpi_size size)
{
	if (phys > ULONG_MAX) {
		printk(KERN_ERR PREFIX "Cannot map memory that high\n");
		return NULL;
	}
	if (acpi_gbl_permanent_mmap)
		
		return ioremap((unsigned long)phys, size);
	else
		return __acpi_map_table((unsigned long)phys, size);
}
EXPORT_SYMBOL_GPL(acpi_os_map_memory);

void __ref acpi_os_unmap_memory(void __iomem *virt, acpi_size size)
{
	if (acpi_gbl_permanent_mmap)
		iounmap(virt);
	else
		__acpi_unmap_table(virt, size);
}
EXPORT_SYMBOL_GPL(acpi_os_unmap_memory);

void __init early_acpi_os_unmap_memory(void __iomem *virt, acpi_size size)
{
	if (!acpi_gbl_permanent_mmap)
		__acpi_unmap_table(virt, size);
}

#ifdef ACPI_FUTURE_USAGE
acpi_status
acpi_os_get_physical_address(void *virt, acpi_physical_address * phys)
{
	if (!phys || !virt)
		return AE_BAD_PARAMETER;

	*phys = virt_to_phys(virt);

	return AE_OK;
}
#endif

#define ACPI_MAX_OVERRIDE_LEN 100

static char acpi_os_name[ACPI_MAX_OVERRIDE_LEN];

acpi_status
acpi_os_predefined_override(const struct acpi_predefined_names *init_val,
			    acpi_string * new_val)
{
	if (!init_val || !new_val)
		return AE_BAD_PARAMETER;

	*new_val = NULL;
	if (!memcmp(init_val->name, "_OS_", 4) && strlen(acpi_os_name)) {
		printk(KERN_INFO PREFIX "Overriding _OS definition to '%s'\n",
		       acpi_os_name);
		*new_val = acpi_os_name;
	}

	return AE_OK;
}

acpi_status
acpi_os_table_override(struct acpi_table_header * existing_table,
		       struct acpi_table_header ** new_table)
{
	if (!existing_table || !new_table)
		return AE_BAD_PARAMETER;

	*new_table = NULL;

#ifdef CONFIG_ACPI_CUSTOM_DSDT
	if (strncmp(existing_table->signature, "DSDT", 4) == 0)
		*new_table = (struct acpi_table_header *)AmlCode;
#endif
	if (*new_table != NULL) {
		printk(KERN_WARNING PREFIX "Override [%4.4s-%8.8s], "
			   "this is unsafe: tainting kernel\n",
		       existing_table->signature,
		       existing_table->oem_table_id);
		add_taint(TAINT_OVERRIDDEN_ACPI_TABLE);
	}
	return AE_OK;
}

static irqreturn_t acpi_irq(int irq, void *dev_id)
{
	u32 handled;

	handled = (*acpi_irq_handler) (acpi_irq_context);

	if (handled) {
		acpi_irq_handled++;
		return IRQ_HANDLED;
	} else {
		acpi_irq_not_handled++;
		return IRQ_NONE;
	}
}

acpi_status
acpi_os_install_interrupt_handler(u32 gsi, acpi_osd_handler handler,
				  void *context)
{
	unsigned int irq;

	acpi_irq_stats_init();

	
	gsi = acpi_gbl_FADT.sci_interrupt;
	if (acpi_gsi_to_irq(gsi, &irq) < 0) {
		printk(KERN_ERR PREFIX "SCI (ACPI GSI %d) not registered\n",
		       gsi);
		return AE_OK;
	}

	acpi_irq_handler = handler;
	acpi_irq_context = context;
	if (request_irq(irq, acpi_irq, IRQF_SHARED, "acpi", acpi_irq)) {
		printk(KERN_ERR PREFIX "SCI (IRQ%d) allocation failed\n", irq);
		return AE_NOT_ACQUIRED;
	}
	acpi_irq_irq = irq;

	return AE_OK;
}

acpi_status acpi_os_remove_interrupt_handler(u32 irq, acpi_osd_handler handler)
{
	if (irq) {
		free_irq(irq, acpi_irq);
		acpi_irq_handler = NULL;
		acpi_irq_irq = 0;
	}

	return AE_OK;
}



void acpi_os_sleep(acpi_integer ms)
{
	schedule_timeout_interruptible(msecs_to_jiffies(ms));
}

void acpi_os_stall(u32 us)
{
	while (us) {
		u32 delay = 1000;

		if (delay > us)
			delay = us;
		udelay(delay);
		touch_nmi_watchdog();
		us -= delay;
	}
}


u64 acpi_os_get_timer(void)
{
	static u64 t;

#ifdef	CONFIG_HPET
	
#endif

#ifdef	CONFIG_X86_PM_TIMER
	
#endif
	if (!t)
		printk(KERN_ERR PREFIX "acpi_os_get_timer() TBD\n");

	return ++t;
}

acpi_status acpi_os_read_port(acpi_io_address port, u32 * value, u32 width)
{
	u32 dummy;

	if (!value)
		value = &dummy;

	*value = 0;
	if (width <= 8) {
		*(u8 *) value = inb(port);
	} else if (width <= 16) {
		*(u16 *) value = inw(port);
	} else if (width <= 32) {
		*(u32 *) value = inl(port);
	} else {
		BUG();
	}

	return AE_OK;
}

EXPORT_SYMBOL(acpi_os_read_port);

acpi_status acpi_os_write_port(acpi_io_address port, u32 value, u32 width)
{
	if (width <= 8) {
		outb(value, port);
	} else if (width <= 16) {
		outw(value, port);
	} else if (width <= 32) {
		outl(value, port);
	} else {
		BUG();
	}

	return AE_OK;
}

EXPORT_SYMBOL(acpi_os_write_port);

acpi_status
acpi_os_read_memory(acpi_physical_address phys_addr, u32 * value, u32 width)
{
	u32 dummy;
	void __iomem *virt_addr;

	virt_addr = ioremap(phys_addr, width);
	if (!value)
		value = &dummy;

	switch (width) {
	case 8:
		*(u8 *) value = readb(virt_addr);
		break;
	case 16:
		*(u16 *) value = readw(virt_addr);
		break;
	case 32:
		*(u32 *) value = readl(virt_addr);
		break;
	default:
		BUG();
	}

	iounmap(virt_addr);

	return AE_OK;
}

acpi_status
acpi_os_write_memory(acpi_physical_address phys_addr, u32 value, u32 width)
{
	void __iomem *virt_addr;

	virt_addr = ioremap(phys_addr, width);

	switch (width) {
	case 8:
		writeb(value, virt_addr);
		break;
	case 16:
		writew(value, virt_addr);
		break;
	case 32:
		writel(value, virt_addr);
		break;
	default:
		BUG();
	}

	iounmap(virt_addr);

	return AE_OK;
}

acpi_status
acpi_os_read_pci_configuration(struct acpi_pci_id * pci_id, u32 reg,
			       u32 *value, u32 width)
{
	int result, size;

	if (!value)
		return AE_BAD_PARAMETER;

	switch (width) {
	case 8:
		size = 1;
		break;
	case 16:
		size = 2;
		break;
	case 32:
		size = 4;
		break;
	default:
		return AE_ERROR;
	}

	result = raw_pci_read(pci_id->segment, pci_id->bus,
				PCI_DEVFN(pci_id->device, pci_id->function),
				reg, size, value);

	return (result ? AE_ERROR : AE_OK);
}

acpi_status
acpi_os_write_pci_configuration(struct acpi_pci_id * pci_id, u32 reg,
				acpi_integer value, u32 width)
{
	int result, size;

	switch (width) {
	case 8:
		size = 1;
		break;
	case 16:
		size = 2;
		break;
	case 32:
		size = 4;
		break;
	default:
		return AE_ERROR;
	}

	result = raw_pci_write(pci_id->segment, pci_id->bus,
				PCI_DEVFN(pci_id->device, pci_id->function),
				reg, size, value);

	return (result ? AE_ERROR : AE_OK);
}


static void acpi_os_derive_pci_id_2(acpi_handle rhandle,	
				    acpi_handle chandle,	
				    struct acpi_pci_id **id,
				    int *is_bridge, u8 * bus_number)
{
	acpi_handle handle;
	struct acpi_pci_id *pci_id = *id;
	acpi_status status;
	unsigned long long temp;
	acpi_object_type type;

	acpi_get_parent(chandle, &handle);
	if (handle != rhandle) {
		acpi_os_derive_pci_id_2(rhandle, handle, &pci_id, is_bridge,
					bus_number);

		status = acpi_get_type(handle, &type);
		if ((ACPI_FAILURE(status)) || (type != ACPI_TYPE_DEVICE))
			return;

		status = acpi_evaluate_integer(handle, METHOD_NAME__ADR, NULL,
					  &temp);
		if (ACPI_SUCCESS(status)) {
			u32 val;
			pci_id->device = ACPI_HIWORD(ACPI_LODWORD(temp));
			pci_id->function = ACPI_LOWORD(ACPI_LODWORD(temp));

			if (*is_bridge)
				pci_id->bus = *bus_number;

			
			status =
			    acpi_os_read_pci_configuration(pci_id, 0x0e, &val,
							   8);
			if (ACPI_SUCCESS(status)
			    && ((val & 0x7f) == 1 || (val & 0x7f) == 2)) {
				status =
				    acpi_os_read_pci_configuration(pci_id, 0x18,
								   &val, 8);
				if (!ACPI_SUCCESS(status)) {
					
					return;
				}
				*is_bridge = 1;
				pci_id->bus = val;
				status =
				    acpi_os_read_pci_configuration(pci_id, 0x19,
								   &val, 8);
				if (ACPI_SUCCESS(status)) {
					*bus_number = val;
				}
			} else
				*is_bridge = 0;
		}
	}
}

void acpi_os_derive_pci_id(acpi_handle rhandle,	
			   acpi_handle chandle,	
			   struct acpi_pci_id **id)
{
	int is_bridge = 1;
	u8 bus_number = (*id)->bus;

	acpi_os_derive_pci_id_2(rhandle, chandle, id, &is_bridge, &bus_number);
}

static void acpi_os_execute_deferred(struct work_struct *work)
{
	struct acpi_os_dpc *dpc = container_of(work, struct acpi_os_dpc, work);

	if (dpc->wait)
		acpi_os_wait_events_complete(NULL);

	dpc->function(dpc->context);
	kfree(dpc);
}



static acpi_status __acpi_os_execute(acpi_execute_type type,
	acpi_osd_exec_callback function, void *context, int hp)
{
	acpi_status status = AE_OK;
	struct acpi_os_dpc *dpc;
	struct workqueue_struct *queue;
	int ret;
	ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
			  "Scheduling function [%p(%p)] for deferred execution.\n",
			  function, context));

	

	dpc = kmalloc(sizeof(struct acpi_os_dpc), GFP_ATOMIC);
	if (!dpc)
		return AE_NO_MEMORY;

	dpc->function = function;
	dpc->context = context;

	
	queue = hp ? kacpi_hotplug_wq :
		(type == OSL_NOTIFY_HANDLER ? kacpi_notify_wq : kacpid_wq);
	dpc->wait = hp ? 1 : 0;
	INIT_WORK(&dpc->work, acpi_os_execute_deferred);
	ret = queue_work(queue, &dpc->work);

	if (!ret) {
		printk(KERN_ERR PREFIX
			  "Call to queue_work() failed.\n");
		status = AE_ERROR;
		kfree(dpc);
	}
	return status;
}

acpi_status acpi_os_execute(acpi_execute_type type,
			    acpi_osd_exec_callback function, void *context)
{
	return __acpi_os_execute(type, function, context, 0);
}
EXPORT_SYMBOL(acpi_os_execute);

acpi_status acpi_os_hotplug_execute(acpi_osd_exec_callback function,
	void *context)
{
	return __acpi_os_execute(0, function, context, 1);
}

void acpi_os_wait_events_complete(void *context)
{
	flush_workqueue(kacpid_wq);
	flush_workqueue(kacpi_notify_wq);
}

EXPORT_SYMBOL(acpi_os_wait_events_complete);


acpi_status acpi_os_create_lock(acpi_spinlock * handle)
{
	spin_lock_init(*handle);

	return AE_OK;
}


void acpi_os_delete_lock(acpi_spinlock handle)
{
	return;
}

acpi_status
acpi_os_create_semaphore(u32 max_units, u32 initial_units, acpi_handle * handle)
{
	struct semaphore *sem = NULL;

	sem = acpi_os_allocate(sizeof(struct semaphore));
	if (!sem)
		return AE_NO_MEMORY;
	memset(sem, 0, sizeof(struct semaphore));

	sema_init(sem, initial_units);

	*handle = (acpi_handle *) sem;

	ACPI_DEBUG_PRINT((ACPI_DB_MUTEX, "Creating semaphore[%p|%d].\n",
			  *handle, initial_units));

	return AE_OK;
}



acpi_status acpi_os_delete_semaphore(acpi_handle handle)
{
	struct semaphore *sem = (struct semaphore *)handle;

	if (!sem)
		return AE_BAD_PARAMETER;

	ACPI_DEBUG_PRINT((ACPI_DB_MUTEX, "Deleting semaphore[%p].\n", handle));

	BUG_ON(!list_empty(&sem->wait_list));
	kfree(sem);
	sem = NULL;

	return AE_OK;
}


acpi_status acpi_os_wait_semaphore(acpi_handle handle, u32 units, u16 timeout)
{
	acpi_status status = AE_OK;
	struct semaphore *sem = (struct semaphore *)handle;
	long jiffies;
	int ret = 0;

	if (!sem || (units < 1))
		return AE_BAD_PARAMETER;

	if (units > 1)
		return AE_SUPPORT;

	ACPI_DEBUG_PRINT((ACPI_DB_MUTEX, "Waiting for semaphore[%p|%d|%d]\n",
			  handle, units, timeout));

	if (timeout == ACPI_WAIT_FOREVER)
		jiffies = MAX_SCHEDULE_TIMEOUT;
	else
		jiffies = msecs_to_jiffies(timeout);
	
	ret = down_timeout(sem, jiffies);
	if (ret)
		status = AE_TIME;

	if (ACPI_FAILURE(status)) {
		ACPI_DEBUG_PRINT((ACPI_DB_MUTEX,
				  "Failed to acquire semaphore[%p|%d|%d], %s",
				  handle, units, timeout,
				  acpi_format_exception(status)));
	} else {
		ACPI_DEBUG_PRINT((ACPI_DB_MUTEX,
				  "Acquired semaphore[%p|%d|%d]", handle,
				  units, timeout));
	}

	return status;
}


acpi_status acpi_os_signal_semaphore(acpi_handle handle, u32 units)
{
	struct semaphore *sem = (struct semaphore *)handle;

	if (!sem || (units < 1))
		return AE_BAD_PARAMETER;

	if (units > 1)
		return AE_SUPPORT;

	ACPI_DEBUG_PRINT((ACPI_DB_MUTEX, "Signaling semaphore[%p|%d]\n", handle,
			  units));

	up(sem);

	return AE_OK;
}

#ifdef ACPI_FUTURE_USAGE
u32 acpi_os_get_line(char *buffer)
{

#ifdef ENABLE_DEBUGGER
	if (acpi_in_debugger) {
		u32 chars;

		kdb_read(buffer, sizeof(line_buf));

		
		chars = strlen(buffer) - 1;
		buffer[chars] = '\0';
	}
#endif

	return 0;
}
#endif				

acpi_status acpi_os_signal(u32 function, void *info)
{
	switch (function) {
	case ACPI_SIGNAL_FATAL:
		printk(KERN_ERR PREFIX "Fatal opcode executed\n");
		break;
	case ACPI_SIGNAL_BREAKPOINT:
		
		break;
	default:
		break;
	}

	return AE_OK;
}

static int __init acpi_os_name_setup(char *str)
{
	char *p = acpi_os_name;
	int count = ACPI_MAX_OVERRIDE_LEN - 1;

	if (!str || !*str)
		return 0;

	for (; count-- && str && *str; str++) {
		if (isalnum(*str) || *str == ' ' || *str == ':')
			*p++ = *str;
		else if (*str == '\'' || *str == '"')
			continue;
		else
			break;
	}
	*p = 0;

	return 1;

}

__setup("acpi_os_name=", acpi_os_name_setup);

static void __init set_osi_linux(unsigned int enable)
{
	if (osi_linux.enable != enable) {
		osi_linux.enable = enable;
		printk(KERN_NOTICE PREFIX "%sed _OSI(Linux)\n",
			enable ? "Add": "Delet");
	}
	return;
}

static void __init acpi_cmdline_osi_linux(unsigned int enable)
{
	osi_linux.cmdline = 1;	
	set_osi_linux(enable);

	return;
}

void __init acpi_dmi_osi_linux(int enable, const struct dmi_system_id *d)
{
	osi_linux.dmi = 1;	

	printk(KERN_NOTICE PREFIX "DMI detected: %s\n", d->ident);

	if (enable == -1)
		return;

	osi_linux.known = 1;	

	set_osi_linux(enable);

	return;
}


int __init acpi_osi_setup(char *str)
{
	if (str == NULL || *str == '\0') {
		printk(KERN_INFO PREFIX "_OSI method disabled\n");
		acpi_gbl_create_osi_method = FALSE;
	} else if (!strcmp("!Linux", str)) {
		acpi_cmdline_osi_linux(0);	
	} else if (*str == '!') {
		if (acpi_osi_invalidate(++str) == AE_OK)
			printk(KERN_INFO PREFIX "Deleted _OSI(%s)\n", str);
	} else if (!strcmp("Linux", str)) {
		acpi_cmdline_osi_linux(1);	
	} else if (*osi_additional_string == '\0') {
		strncpy(osi_additional_string, str, OSI_STRING_LENGTH_MAX);
		printk(KERN_INFO PREFIX "Added _OSI(%s)\n", str);
	}

	return 1;
}

__setup("acpi_osi=", acpi_osi_setup);


static int __init acpi_serialize_setup(char *str)
{
	printk(KERN_INFO PREFIX "serialize enabled\n");

	acpi_gbl_all_methods_serialized = TRUE;

	return 1;
}

__setup("acpi_serialize", acpi_serialize_setup);


static int __init acpi_wake_gpes_always_on_setup(char *str)
{
	printk(KERN_INFO PREFIX "wake GPEs not disabled\n");

	acpi_gbl_leave_wake_gpes_disabled = FALSE;

	return 1;
}

__setup("acpi_wake_gpes_always_on", acpi_wake_gpes_always_on_setup);


#define ENFORCE_RESOURCES_STRICT 2
#define ENFORCE_RESOURCES_LAX    1
#define ENFORCE_RESOURCES_NO     0

static unsigned int acpi_enforce_resources = ENFORCE_RESOURCES_STRICT;

static int __init acpi_enforce_resources_setup(char *str)
{
	if (str == NULL || *str == '\0')
		return 0;

	if (!strcmp("strict", str))
		acpi_enforce_resources = ENFORCE_RESOURCES_STRICT;
	else if (!strcmp("lax", str))
		acpi_enforce_resources = ENFORCE_RESOURCES_LAX;
	else if (!strcmp("no", str))
		acpi_enforce_resources = ENFORCE_RESOURCES_NO;

	return 1;
}

__setup("acpi_enforce_resources=", acpi_enforce_resources_setup);


int acpi_check_resource_conflict(struct resource *res)
{
	struct acpi_res_list *res_list_elem;
	int ioport;
	int clash = 0;

	if (acpi_enforce_resources == ENFORCE_RESOURCES_NO)
		return 0;
	if (!(res->flags & IORESOURCE_IO) && !(res->flags & IORESOURCE_MEM))
		return 0;

	ioport = res->flags & IORESOURCE_IO;

	spin_lock(&acpi_res_lock);
	list_for_each_entry(res_list_elem, &resource_list_head,
			    resource_list) {
		if (ioport && (res_list_elem->resource_type
			       != ACPI_ADR_SPACE_SYSTEM_IO))
			continue;
		if (!ioport && (res_list_elem->resource_type
				!= ACPI_ADR_SPACE_SYSTEM_MEMORY))
			continue;

		if (res->end < res_list_elem->start
		    || res_list_elem->end < res->start)
			continue;
		clash = 1;
		break;
	}
	spin_unlock(&acpi_res_lock);

	if (clash) {
		if (acpi_enforce_resources != ENFORCE_RESOURCES_NO) {
			printk("%sACPI: %s resource %s [0x%llx-0x%llx]"
			       " conflicts with ACPI region %s"
			       " [0x%llx-0x%llx]\n",
			       acpi_enforce_resources == ENFORCE_RESOURCES_LAX
			       ? KERN_WARNING : KERN_ERR,
			       ioport ? "I/O" : "Memory", res->name,
			       (long long) res->start, (long long) res->end,
			       res_list_elem->name,
			       (long long) res_list_elem->start,
			       (long long) res_list_elem->end);
			if (acpi_enforce_resources == ENFORCE_RESOURCES_LAX)
				printk(KERN_NOTICE "ACPI: This conflict may"
				       " cause random problems and system"
				       " instability\n");
			printk(KERN_INFO "ACPI: If an ACPI driver is available"
			       " for this device, you should use it instead of"
			       " the native driver\n");
		}
		if (acpi_enforce_resources == ENFORCE_RESOURCES_STRICT)
			return -EBUSY;
	}
	return 0;
}
EXPORT_SYMBOL(acpi_check_resource_conflict);

int acpi_check_region(resource_size_t start, resource_size_t n,
		      const char *name)
{
	struct resource res = {
		.start = start,
		.end   = start + n - 1,
		.name  = name,
		.flags = IORESOURCE_IO,
	};

	return acpi_check_resource_conflict(&res);
}
EXPORT_SYMBOL(acpi_check_region);

int acpi_check_mem_region(resource_size_t start, resource_size_t n,
		      const char *name)
{
	struct resource res = {
		.start = start,
		.end   = start + n - 1,
		.name  = name,
		.flags = IORESOURCE_MEM,
	};

	return acpi_check_resource_conflict(&res);

}
EXPORT_SYMBOL(acpi_check_mem_region);



acpi_cpu_flags acpi_os_acquire_lock(acpi_spinlock lockp)
{
	acpi_cpu_flags flags;
	spin_lock_irqsave(lockp, flags);
	return flags;
}



void acpi_os_release_lock(acpi_spinlock lockp, acpi_cpu_flags flags)
{
	spin_unlock_irqrestore(lockp, flags);
}

#ifndef ACPI_USE_LOCAL_CACHE



acpi_status
acpi_os_create_cache(char *name, u16 size, u16 depth, acpi_cache_t ** cache)
{
	*cache = kmem_cache_create(name, size, 0, 0, NULL);
	if (*cache == NULL)
		return AE_ERROR;
	else
		return AE_OK;
}



acpi_status acpi_os_purge_cache(acpi_cache_t * cache)
{
	kmem_cache_shrink(cache);
	return (AE_OK);
}



acpi_status acpi_os_delete_cache(acpi_cache_t * cache)
{
	kmem_cache_destroy(cache);
	return (AE_OK);
}



acpi_status acpi_os_release_object(acpi_cache_t * cache, void *object)
{
	kmem_cache_free(cache, object);
	return (AE_OK);
}



acpi_status
acpi_os_validate_interface (char *interface)
{
	if (!strncmp(osi_additional_string, interface, OSI_STRING_LENGTH_MAX))
		return AE_OK;
	if (!strcmp("Linux", interface)) {

		printk(KERN_NOTICE PREFIX
			"BIOS _OSI(Linux) query %s%s\n",
			osi_linux.enable ? "honored" : "ignored",
			osi_linux.cmdline ? " via cmdline" :
			osi_linux.dmi ? " via DMI" : "");

		if (osi_linux.enable)
			return AE_OK;
	}
	return AE_SUPPORT;
}

static inline int acpi_res_list_add(struct acpi_res_list *res)
{
	struct acpi_res_list *res_list_elem;

	list_for_each_entry(res_list_elem, &resource_list_head,
			    resource_list) {

		if (res->resource_type == res_list_elem->resource_type &&
		    res->start == res_list_elem->start &&
		    res->end == res_list_elem->end) {

			

			res_list_elem->count++;
			return 0;
		}
	}

	res->count = 1;
	list_add(&res->resource_list, &resource_list_head);
	return 1;
}

static inline void acpi_res_list_del(struct acpi_res_list *res)
{
	struct acpi_res_list *res_list_elem;

	list_for_each_entry(res_list_elem, &resource_list_head,
			    resource_list) {

		if (res->resource_type == res_list_elem->resource_type &&
		    res->start == res_list_elem->start &&
		    res->end == res_list_elem->end) {

			

			if (--res_list_elem->count == 0) {
				list_del(&res_list_elem->resource_list);
				kfree(res_list_elem);
			}
			return;
		}
	}
}

acpi_status
acpi_os_invalidate_address(
    u8                   space_id,
    acpi_physical_address   address,
    acpi_size               length)
{
	struct acpi_res_list res;

	switch (space_id) {
	case ACPI_ADR_SPACE_SYSTEM_IO:
	case ACPI_ADR_SPACE_SYSTEM_MEMORY:
		
		res.start = address;
		res.end = address + length - 1;
		res.resource_type = space_id;
		spin_lock(&acpi_res_lock);
		acpi_res_list_del(&res);
		spin_unlock(&acpi_res_lock);
		break;
	case ACPI_ADR_SPACE_PCI_CONFIG:
	case ACPI_ADR_SPACE_EC:
	case ACPI_ADR_SPACE_SMBUS:
	case ACPI_ADR_SPACE_CMOS:
	case ACPI_ADR_SPACE_PCI_BAR_TARGET:
	case ACPI_ADR_SPACE_DATA_TABLE:
	case ACPI_ADR_SPACE_FIXED_HARDWARE:
		break;
	}
	return AE_OK;
}



acpi_status
acpi_os_validate_address (
    u8                   space_id,
    acpi_physical_address   address,
    acpi_size               length,
    char *name)
{
	struct acpi_res_list *res;
	int added;
	if (acpi_enforce_resources == ENFORCE_RESOURCES_NO)
		return AE_OK;

	switch (space_id) {
	case ACPI_ADR_SPACE_SYSTEM_IO:
	case ACPI_ADR_SPACE_SYSTEM_MEMORY:
		
		res = kzalloc(sizeof(struct acpi_res_list), GFP_KERNEL);
		if (!res)
			return AE_OK;
		
		strlcpy(res->name, name, 5);
		res->start = address;
		res->end = address + length - 1;
		res->resource_type = space_id;
		spin_lock(&acpi_res_lock);
		added = acpi_res_list_add(res);
		spin_unlock(&acpi_res_lock);
		pr_debug("%s %s resource: start: 0x%llx, end: 0x%llx, "
			 "name: %s\n", added ? "Added" : "Already exist",
			 (space_id == ACPI_ADR_SPACE_SYSTEM_IO)
			 ? "SystemIO" : "System Memory",
			 (unsigned long long)res->start,
			 (unsigned long long)res->end,
			 res->name);
		if (!added)
			kfree(res);
		break;
	case ACPI_ADR_SPACE_PCI_CONFIG:
	case ACPI_ADR_SPACE_EC:
	case ACPI_ADR_SPACE_SMBUS:
	case ACPI_ADR_SPACE_CMOS:
	case ACPI_ADR_SPACE_PCI_BAR_TARGET:
	case ACPI_ADR_SPACE_DATA_TABLE:
	case ACPI_ADR_SPACE_FIXED_HARDWARE:
		break;
	}
	return AE_OK;
}

#endif
