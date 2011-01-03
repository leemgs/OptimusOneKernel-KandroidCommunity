#ifndef _ASM_X86_IO_APIC_H
#define _ASM_X86_IO_APIC_H

#include <linux/types.h>
#include <asm/mpspec.h>
#include <asm/apicdef.h>
#include <asm/irq_vectors.h>




#define IO_APIC_REDIR_VECTOR_MASK	0x000FF
#define IO_APIC_REDIR_DEST_LOGICAL	0x00800
#define IO_APIC_REDIR_DEST_PHYSICAL	0x00000
#define IO_APIC_REDIR_SEND_PENDING	(1 << 12)
#define IO_APIC_REDIR_REMOTE_IRR	(1 << 14)
#define IO_APIC_REDIR_LEVEL_TRIGGER	(1 << 15)
#define IO_APIC_REDIR_MASKED		(1 << 16)


union IO_APIC_reg_00 {
	u32	raw;
	struct {
		u32	__reserved_2	: 14,
			LTS		:  1,
			delivery_type	:  1,
			__reserved_1	:  8,
			ID		:  8;
	} __attribute__ ((packed)) bits;
};

union IO_APIC_reg_01 {
	u32	raw;
	struct {
		u32	version		:  8,
			__reserved_2	:  7,
			PRQ		:  1,
			entries		:  8,
			__reserved_1	:  8;
	} __attribute__ ((packed)) bits;
};

union IO_APIC_reg_02 {
	u32	raw;
	struct {
		u32	__reserved_2	: 24,
			arbitration	:  4,
			__reserved_1	:  4;
	} __attribute__ ((packed)) bits;
};

union IO_APIC_reg_03 {
	u32	raw;
	struct {
		u32	boot_DT		:  1,
			__reserved_1	: 31;
	} __attribute__ ((packed)) bits;
};

enum ioapic_irq_destination_types {
	dest_Fixed = 0,
	dest_LowestPrio = 1,
	dest_SMI = 2,
	dest__reserved_1 = 3,
	dest_NMI = 4,
	dest_INIT = 5,
	dest__reserved_2 = 6,
	dest_ExtINT = 7
};

struct IO_APIC_route_entry {
	__u32	vector		:  8,
		delivery_mode	:  3,	
		dest_mode	:  1,	
		delivery_status	:  1,
		polarity	:  1,
		irr		:  1,
		trigger		:  1,	
		mask		:  1,	
		__reserved_2	: 15;

	__u32	__reserved_3	: 24,
		dest		:  8;
} __attribute__ ((packed));

struct IR_IO_APIC_route_entry {
	__u64	vector		: 8,
		zero		: 3,
		index2		: 1,
		delivery_status : 1,
		polarity	: 1,
		irr		: 1,
		trigger		: 1,
		mask		: 1,
		reserved	: 31,
		format		: 1,
		index		: 15;
} __attribute__ ((packed));

#ifdef CONFIG_X86_IO_APIC


extern int nr_ioapics;
extern int nr_ioapic_registers[MAX_IO_APICS];

#define MP_MAX_IOAPIC_PIN 127


extern struct mpc_ioapic mp_ioapics[MAX_IO_APICS];


extern int mp_irq_entries;


extern struct mpc_intsrc mp_irqs[MAX_IRQ_SOURCES];


extern int mpc_default_type;


extern int sis_apic_bug;


extern int skip_ioapic_setup;


extern int noioapicquirk;


extern int noioapicreroute;


extern int timer_through_8259;

extern void io_apic_disable_legacy(void);


#define io_apic_assign_pci_irqs \
	(mp_irq_entries && !skip_ioapic_setup && io_apic_irqs)

extern u8 io_apic_unique_id(u8 id);
extern int io_apic_get_unique_id(int ioapic, int apic_id);
extern int io_apic_get_version(int ioapic);
extern int io_apic_get_redir_entries(int ioapic);

struct io_apic_irq_attr;
extern int io_apic_set_pci_routing(struct device *dev, int irq,
		 struct io_apic_irq_attr *irq_attr);
extern int (*ioapic_renumber_irq)(int ioapic, int irq);
extern void ioapic_init_mappings(void);
extern void ioapic_insert_resources(void);

extern struct IO_APIC_route_entry **alloc_ioapic_entries(void);
extern void free_ioapic_entries(struct IO_APIC_route_entry **ioapic_entries);
extern int save_IO_APIC_setup(struct IO_APIC_route_entry **ioapic_entries);
extern void mask_IO_APIC_setup(struct IO_APIC_route_entry **ioapic_entries);
extern int restore_IO_APIC_setup(struct IO_APIC_route_entry **ioapic_entries);

extern void probe_nr_irqs_gsi(void);

extern int setup_ioapic_entry(int apic, int irq,
			      struct IO_APIC_route_entry *entry,
			      unsigned int destination, int trigger,
			      int polarity, int vector, int pin);
extern void ioapic_write_entry(int apic, int pin,
			       struct IO_APIC_route_entry e);
extern void setup_ioapic_ids_from_mpc(void);

struct mp_ioapic_gsi{
	int gsi_base;
	int gsi_end;
};
extern struct mp_ioapic_gsi  mp_gsi_routing[];
int mp_find_ioapic(int gsi);
int mp_find_ioapic_pin(int ioapic, int gsi);
void __init mp_register_ioapic(int id, u32 address, u32 gsi_base);

#else  

#define io_apic_assign_pci_irqs 0
#define setup_ioapic_ids_from_mpc x86_init_noop
static const int timer_through_8259 = 0;
static inline void ioapic_init_mappings(void)	{ }
static inline void ioapic_insert_resources(void) { }
static inline void probe_nr_irqs_gsi(void)	{ }

#endif

#endif 
