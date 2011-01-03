
#include <linux/types.h>


#define VMI_SIGNATURE 0x696d5663   

#define PCI_VENDOR_ID_VMWARE            0x15AD
#define PCI_DEVICE_ID_VMWARE_VMI        0x0801


#define VMI_API_REV_MAJOR       3
#define VMI_API_REV_MINOR       0

#define VMI_CALL_CPUID			0
#define VMI_CALL_WRMSR			1
#define VMI_CALL_RDMSR			2
#define VMI_CALL_SetGDT			3
#define VMI_CALL_SetLDT			4
#define VMI_CALL_SetIDT			5
#define VMI_CALL_SetTR			6
#define VMI_CALL_GetGDT			7
#define VMI_CALL_GetLDT			8
#define VMI_CALL_GetIDT			9
#define VMI_CALL_GetTR			10
#define VMI_CALL_WriteGDTEntry		11
#define VMI_CALL_WriteLDTEntry		12
#define VMI_CALL_WriteIDTEntry		13
#define VMI_CALL_UpdateKernelStack	14
#define VMI_CALL_SetCR0			15
#define VMI_CALL_SetCR2			16
#define VMI_CALL_SetCR3			17
#define VMI_CALL_SetCR4			18
#define VMI_CALL_GetCR0			19
#define VMI_CALL_GetCR2			20
#define VMI_CALL_GetCR3			21
#define VMI_CALL_GetCR4			22
#define VMI_CALL_WBINVD			23
#define VMI_CALL_SetDR			24
#define VMI_CALL_GetDR			25
#define VMI_CALL_RDPMC			26
#define VMI_CALL_RDTSC			27
#define VMI_CALL_CLTS			28
#define VMI_CALL_EnableInterrupts	29
#define VMI_CALL_DisableInterrupts	30
#define VMI_CALL_GetInterruptMask	31
#define VMI_CALL_SetInterruptMask	32
#define VMI_CALL_IRET			33
#define VMI_CALL_SYSEXIT		34
#define VMI_CALL_Halt			35
#define VMI_CALL_Reboot			36
#define VMI_CALL_Shutdown		37
#define VMI_CALL_SetPxE			38
#define VMI_CALL_SetPxELong		39
#define VMI_CALL_UpdatePxE		40
#define VMI_CALL_UpdatePxELong		41
#define VMI_CALL_MachineToPhysical	42
#define VMI_CALL_PhysicalToMachine	43
#define VMI_CALL_AllocatePage		44
#define VMI_CALL_ReleasePage		45
#define VMI_CALL_InvalPage		46
#define VMI_CALL_FlushTLB		47
#define VMI_CALL_SetLinearMapping	48

#define VMI_CALL_SetIOPLMask		61
#define VMI_CALL_SetInitialAPState	62
#define VMI_CALL_APICWrite		63
#define VMI_CALL_APICRead		64
#define VMI_CALL_IODelay		65
#define VMI_CALL_SetLazyMode		73




#define VMI_PAGE_PAE             0x10  
#define VMI_PAGE_CLONE           0x20  
#define VMI_PAGE_ZEROED          0x40  



#define VMI_PAGE_PT              0x01
#define VMI_PAGE_PD              0x02
#define VMI_PAGE_PDP             0x04
#define VMI_PAGE_PML4            0x08

#define VMI_PAGE_NORMAL          0x00 


#define VMI_PAGE_CURRENT_AS      0x10 
#define VMI_PAGE_DEFER           0x20 
#define VMI_PAGE_VA_MASK         0xfffff000

#ifdef CONFIG_X86_PAE
#define VMI_PAGE_L1		(VMI_PAGE_PT | VMI_PAGE_PAE | VMI_PAGE_ZEROED)
#define VMI_PAGE_L2		(VMI_PAGE_PD | VMI_PAGE_PAE | VMI_PAGE_ZEROED)
#else
#define VMI_PAGE_L1		(VMI_PAGE_PT | VMI_PAGE_ZEROED)
#define VMI_PAGE_L2		(VMI_PAGE_PD | VMI_PAGE_ZEROED)
#endif


#define VMI_FLUSH_TLB            0x01
#define VMI_FLUSH_GLOBAL         0x02




#define VMI_RELOCATION_NONE     0
#define VMI_RELOCATION_CALL_REL 1
#define VMI_RELOCATION_JUMP_REL 2
#define VMI_RELOCATION_NOP	3

#ifndef __ASSEMBLY__
struct vmi_relocation_info {
	unsigned char           *eip;
	unsigned char           type;
	unsigned char           reserved[3];
};
#endif




#ifndef __ASSEMBLY__

struct vrom_header {
	u16     rom_signature;  
	u8      rom_length;     
	u8      rom_entry[4];   
	u8      rom_pad0;       
	u32     vrom_signature; 
	u8      api_version_min;
	u8      api_version_maj;
	u8      jump_slots;     
	u8      reserved1;      
	u32     virtual_top;    
	u16     reserved2;      
	u16	license_offs;	
	u16     pci_header_offs;
	u16     pnp_header_offs;
	u32     rom_pad3;       
	u8      reserved[96];   
	char    vmi_init[8];    
	char    get_reloc[8];   
} __attribute__((packed));

struct pnp_header {
	char sig[4];
	char rev;
	char size;
	short next;
	short res;
	long devID;
	unsigned short manufacturer_offset;
	unsigned short product_offset;
} __attribute__((packed));

struct pci_header {
	char sig[4];
	short vendorID;
	short deviceID;
	short vpdData;
	short size;
	char rev;
	char class;
	char subclass;
	char interface;
	short chunks;
	char rom_version_min;
	char rom_version_maj;
	char codetype;
	char lastRom;
	short reserved;
} __attribute__((packed));


#ifdef CONFIG_VMI
extern void vmi_init(void);
extern void vmi_activate(void);
extern void vmi_bringup(void);
#else
static inline void vmi_init(void) {}
static inline void vmi_activate(void) {}
static inline void vmi_bringup(void) {}
#endif


struct vmi_ap_state {
	u32 cr0;
	u32 cr2;
	u32 cr3;
	u32 cr4;

	u64 efer;

	u32 eip;
	u32 eflags;
	u32 eax;
	u32 ebx;
	u32 ecx;
	u32 edx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;
	u16 cs;
	u16 ss;
	u16 ds;
	u16 es;
	u16 fs;
	u16 gs;
	u16 ldtr;

	u16 gdtr_limit;
	u32 gdtr_base;
	u32 idtr_base;
	u16 idtr_limit;
};

#endif
