#ifndef __MACH_MX25_H__
#define __MACH_MX25_H__

#define MX25_AIPS1_BASE_ADDR		0x43F00000
#define MX25_AIPS1_BASE_ADDR_VIRT	0xFC000000
#define MX25_AIPS1_SIZE			SZ_1M
#define MX25_AIPS2_BASE_ADDR		0x53F00000
#define MX25_AIPS2_BASE_ADDR_VIRT	0xFC200000
#define MX25_AIPS2_SIZE			SZ_1M
#define MX25_AVIC_BASE_ADDR		0x68000000
#define MX25_AVIC_BASE_ADDR_VIRT	0xFC400000
#define MX25_AVIC_SIZE			SZ_1M

#define MX25_IOMUXC_BASE_ADDR		(MX25_AIPS1_BASE_ADDR + 0xac000)

#define MX25_CRM_BASE_ADDR		(MX25_AIPS2_BASE_ADDR + 0x80000)
#define MX25_GPT1_BASE_ADDR		(MX25_AIPS2_BASE_ADDR + 0x90000)
#define MX25_WDOG_BASE_ADDR		(MX25_AIPS2_BASE_ADDR + 0xdc000)

#define MX25_GPIO1_BASE_ADDR_VIRT	(MX25_AIPS2_BASE_ADDR_VIRT + 0xcc000)
#define MX25_GPIO2_BASE_ADDR_VIRT	(MX25_AIPS2_BASE_ADDR_VIRT + 0xd0000)
#define MX25_GPIO3_BASE_ADDR_VIRT	(MX25_AIPS2_BASE_ADDR_VIRT + 0xa4000)
#define MX25_GPIO4_BASE_ADDR_VIRT	(MX25_AIPS2_BASE_ADDR_VIRT + 0x9c000)

#define MX25_AIPS1_IO_ADDRESS(x)  \
	(((x) - MX25_AIPS1_BASE_ADDR) + MX25_AIPS1_BASE_ADDR_VIRT)
#define MX25_AIPS2_IO_ADDRESS(x)  \
	(((x) - MX25_AIPS2_BASE_ADDR) + MX25_AIPS2_BASE_ADDR_VIRT)
#define MX25_AVIC_IO_ADDRESS(x)  \
	(((x) - MX25_AVIC_BASE_ADDR) + MX25_AVIC_BASE_ADDR_VIRT)

#define __in_range(addr, name)	((addr) >= name##_BASE_ADDR && (addr) < name##_BASE_ADDR + name##_SIZE)

#define MX25_IO_ADDRESS(x)					\
	(void __force __iomem *)				\
	(__in_range(x, MX25_AIPS1) ? MX25_AIPS1_IO_ADDRESS(x) :	\
	__in_range(x, MX25_AIPS2) ? MX25_AIPS2_IO_ADDRESS(x) :	\
	__in_range(x, MX25_AVIC) ? MX25_AVIC_IO_ADDRESS(x) :	\
	0xDEADBEEF)

#define UART1_BASE_ADDR			0x43f90000
#define UART2_BASE_ADDR			0x43f94000

#endif 
