#ifndef B43_H_
#define B43_H_

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/hw_random.h>
#include <linux/ssb/ssb.h>
#include <net/mac80211.h>

#include "debugfs.h"
#include "leds.h"
#include "rfkill.h"
#include "lo.h"
#include "phy_common.h"



#define B43_SUPPORTED_FIRMWARE_ID	"FW13"


#ifdef CONFIG_B43_DEBUG
# define B43_DEBUG	1
#else
# define B43_DEBUG	0
#endif

#define B43_RX_MAX_SSI			60


#define B43_MMIO_DMA0_REASON		0x20
#define B43_MMIO_DMA0_IRQ_MASK		0x24
#define B43_MMIO_DMA1_REASON		0x28
#define B43_MMIO_DMA1_IRQ_MASK		0x2C
#define B43_MMIO_DMA2_REASON		0x30
#define B43_MMIO_DMA2_IRQ_MASK		0x34
#define B43_MMIO_DMA3_REASON		0x38
#define B43_MMIO_DMA3_IRQ_MASK		0x3C
#define B43_MMIO_DMA4_REASON		0x40
#define B43_MMIO_DMA4_IRQ_MASK		0x44
#define B43_MMIO_DMA5_REASON		0x48
#define B43_MMIO_DMA5_IRQ_MASK		0x4C
#define B43_MMIO_MACCTL			0x120	
#define B43_MMIO_MACCMD			0x124	
#define B43_MMIO_GEN_IRQ_REASON		0x128
#define B43_MMIO_GEN_IRQ_MASK		0x12C
#define B43_MMIO_RAM_CONTROL		0x130
#define B43_MMIO_RAM_DATA		0x134
#define B43_MMIO_PS_STATUS		0x140
#define B43_MMIO_RADIO_HWENABLED_HI	0x158
#define B43_MMIO_SHM_CONTROL		0x160
#define B43_MMIO_SHM_DATA		0x164
#define B43_MMIO_SHM_DATA_UNALIGNED	0x166
#define B43_MMIO_XMITSTAT_0		0x170
#define B43_MMIO_XMITSTAT_1		0x174
#define B43_MMIO_REV3PLUS_TSF_LOW	0x180	
#define B43_MMIO_REV3PLUS_TSF_HIGH	0x184	
#define B43_MMIO_TSF_CFP_REP		0x188
#define B43_MMIO_TSF_CFP_START		0x18C
#define B43_MMIO_TSF_CFP_MAXDUR		0x190


#define B43_MMIO_DMA32_BASE0		0x200
#define B43_MMIO_DMA32_BASE1		0x220
#define B43_MMIO_DMA32_BASE2		0x240
#define B43_MMIO_DMA32_BASE3		0x260
#define B43_MMIO_DMA32_BASE4		0x280
#define B43_MMIO_DMA32_BASE5		0x2A0

#define B43_MMIO_DMA64_BASE0		0x200
#define B43_MMIO_DMA64_BASE1		0x240
#define B43_MMIO_DMA64_BASE2		0x280
#define B43_MMIO_DMA64_BASE3		0x2C0
#define B43_MMIO_DMA64_BASE4		0x300
#define B43_MMIO_DMA64_BASE5		0x340


#define B43_MMIO_PIO_BASE0		0x300
#define B43_MMIO_PIO_BASE1		0x310
#define B43_MMIO_PIO_BASE2		0x320
#define B43_MMIO_PIO_BASE3		0x330
#define B43_MMIO_PIO_BASE4		0x340
#define B43_MMIO_PIO_BASE5		0x350
#define B43_MMIO_PIO_BASE6		0x360
#define B43_MMIO_PIO_BASE7		0x370

#define B43_MMIO_PIO11_BASE0		0x200
#define B43_MMIO_PIO11_BASE1		0x240
#define B43_MMIO_PIO11_BASE2		0x280
#define B43_MMIO_PIO11_BASE3		0x2C0
#define B43_MMIO_PIO11_BASE4		0x300
#define B43_MMIO_PIO11_BASE5		0x340

#define B43_MMIO_PHY_VER		0x3E0
#define B43_MMIO_PHY_RADIO		0x3E2
#define B43_MMIO_PHY0			0x3E6
#define B43_MMIO_ANTENNA		0x3E8
#define B43_MMIO_CHANNEL		0x3F0
#define B43_MMIO_CHANNEL_EXT		0x3F4
#define B43_MMIO_RADIO_CONTROL		0x3F6
#define B43_MMIO_RADIO_DATA_HIGH	0x3F8
#define B43_MMIO_RADIO_DATA_LOW		0x3FA
#define B43_MMIO_PHY_CONTROL		0x3FC
#define B43_MMIO_PHY_DATA		0x3FE
#define B43_MMIO_MACFILTER_CONTROL	0x420
#define B43_MMIO_MACFILTER_DATA		0x422
#define B43_MMIO_RCMTA_COUNT		0x43C
#define B43_MMIO_RADIO_HWENABLED_LO	0x49A
#define B43_MMIO_GPIO_CONTROL		0x49C
#define B43_MMIO_GPIO_MASK		0x49E
#define B43_MMIO_TSF_CFP_START_LOW	0x604
#define B43_MMIO_TSF_CFP_START_HIGH	0x606
#define B43_MMIO_TSF_CFP_PRETBTT	0x612
#define B43_MMIO_TSF_0			0x632	
#define B43_MMIO_TSF_1			0x634	
#define B43_MMIO_TSF_2			0x636	
#define B43_MMIO_TSF_3			0x638	
#define B43_MMIO_RNG			0x65A
#define B43_MMIO_IFSSLOT		0x684	
#define B43_MMIO_IFSCTL			0x688 
#define  B43_MMIO_IFSCTL_USE_EDCF	0x0004
#define B43_MMIO_POWERUP_DELAY		0x6A8
#define B43_MMIO_BTCOEX_CTL		0x6B4 
#define B43_MMIO_BTCOEX_STAT		0x6B6 
#define B43_MMIO_BTCOEX_TXCTL		0x6B8 


#define B43_BFL_BTCOEXIST		0x0001	
#define B43_BFL_PACTRL			0x0002	
#define B43_BFL_AIRLINEMODE		0x0004	
#define B43_BFL_RSSI			0x0008	
#define B43_BFL_ENETSPI			0x0010	
#define B43_BFL_XTAL_NOSLOW		0x0020	
#define B43_BFL_CCKHIPWR		0x0040	
#define B43_BFL_ENETADM			0x0080	
#define B43_BFL_ENETVLAN		0x0100	
#define B43_BFL_AFTERBURNER		0x0200	
#define B43_BFL_NOPCI			0x0400	
#define B43_BFL_FEM			0x0800	
#define B43_BFL_EXTLNA			0x1000	
#define B43_BFL_HGPA			0x2000	
#define B43_BFL_BTCMOD			0x4000	
#define B43_BFL_ALTIQ			0x8000	


#define B43_BFH_NOPA			0x0001	
#define B43_BFH_RSSIINV			0x0002	
#define B43_BFH_PAREF			0x0004	
#define B43_BFH_3TSWITCH		0x0008	
#define B43_BFH_PHASESHIFT		0x0010	
#define B43_BFH_BUCKBOOST		0x0020	
#define B43_BFH_FEM_BT			0x0040	


#define B43_GPIO_CONTROL		0x6c


enum {
	B43_SHM_UCODE,		
	B43_SHM_SHARED,		
	B43_SHM_SCRATCH,	
	B43_SHM_HW,		
	B43_SHM_RCMTA,		
};

#define B43_SHM_AUTOINC_R		0x0200	
#define B43_SHM_AUTOINC_W		0x0100	
#define B43_SHM_AUTOINC_RW		(B43_SHM_AUTOINC_R | \
					 B43_SHM_AUTOINC_W)


#define B43_SHM_SH_WLCOREREV		0x0016	
#define B43_SHM_SH_PCTLWDPOS		0x0008
#define B43_SHM_SH_RXPADOFF		0x0034	
#define B43_SHM_SH_FWCAPA		0x0042	
#define B43_SHM_SH_PHYVER		0x0050	
#define B43_SHM_SH_PHYTYPE		0x0052	
#define B43_SHM_SH_ANTSWAP		0x005C	
#define B43_SHM_SH_HOSTFLO		0x005E	
#define B43_SHM_SH_HOSTFMI		0x0060	
#define B43_SHM_SH_HOSTFHI		0x0062	
#define B43_SHM_SH_RFATT		0x0064	
#define B43_SHM_SH_RADAR		0x0066	
#define B43_SHM_SH_PHYTXNOI		0x006E	
#define B43_SHM_SH_RFRXSP1		0x0072	
#define B43_SHM_SH_CHAN			0x00A0	
#define  B43_SHM_SH_CHAN_5GHZ		0x0100	
#define B43_SHM_SH_BCMCFIFOID		0x0108	

#define B43_SHM_SH_TSSI_CCK		0x0058	
#define B43_SHM_SH_TSSI_OFDM_A		0x0068	
#define B43_SHM_SH_TSSI_OFDM_G		0x0070	
#define  B43_TSSI_MAX			0x7F	

#define B43_SHM_SH_SIZE01		0x0098	
#define B43_SHM_SH_SIZE23		0x009A	
#define B43_SHM_SH_SIZE45		0x009C	
#define B43_SHM_SH_SIZE67		0x009E	

#define B43_SHM_SH_JSSI0		0x0088	
#define B43_SHM_SH_JSSI1		0x008A	
#define B43_SHM_SH_JSSIAUX		0x008C	

#define B43_SHM_SH_DEFAULTIV		0x003C	
#define B43_SHM_SH_NRRXTRANS		0x003E	
#define B43_SHM_SH_KTP			0x0056	
#define B43_SHM_SH_TKIPTSCTTAK		0x0318
#define B43_SHM_SH_KEYIDXBLOCK		0x05D4	
#define B43_SHM_SH_PSM			0x05F4	

#define B43_SHM_SH_EDCFSTAT		0x000E	
#define B43_SHM_SH_TXFCUR		0x0030	
#define B43_SHM_SH_EDCFQ		0x0240	

#define B43_SHM_SH_SLOTT		0x0010	
#define B43_SHM_SH_DTIMPER		0x0012	
#define B43_SHM_SH_NOSLPZNATDTIM	0x004C	

#define B43_SHM_SH_BTL0			0x0018	
#define B43_SHM_SH_BTL1			0x001A	
#define B43_SHM_SH_BTSFOFF		0x001C	
#define B43_SHM_SH_TIMBPOS		0x001E	
#define B43_SHM_SH_DTIMP		0x0012	
#define B43_SHM_SH_MCASTCOOKIE		0x00A8	
#define B43_SHM_SH_SFFBLIM		0x0044	
#define B43_SHM_SH_LFFBLIM		0x0046	
#define B43_SHM_SH_BEACPHYCTL		0x0054	
#define B43_SHM_SH_EXTNPHYCTL		0x00B0	

#define B43_SHM_SH_ACKCTSPHYCTL		0x0022	

#define B43_SHM_SH_PRSSID		0x0160	
#define B43_SHM_SH_PRSSIDLEN		0x0048	
#define B43_SHM_SH_PRTLEN		0x004A	
#define B43_SHM_SH_PRMAXTIME		0x0074	
#define B43_SHM_SH_PRPHYCTL		0x0188	

#define B43_SHM_SH_OFDMDIRECT		0x01C0	
#define B43_SHM_SH_OFDMBASIC		0x01E0	
#define B43_SHM_SH_CCKDIRECT		0x0200	
#define B43_SHM_SH_CCKBASIC		0x0220	

#define B43_SHM_SH_UCODEREV		0x0000	
#define B43_SHM_SH_UCODEPATCH		0x0002	
#define B43_SHM_SH_UCODEDATE		0x0004	
#define B43_SHM_SH_UCODETIME		0x0006	
#define B43_SHM_SH_UCODESTAT		0x0040	
#define  B43_SHM_SH_UCODESTAT_INVALID	0
#define  B43_SHM_SH_UCODESTAT_INIT	1
#define  B43_SHM_SH_UCODESTAT_ACTIVE	2
#define  B43_SHM_SH_UCODESTAT_SUSP	3	
#define  B43_SHM_SH_UCODESTAT_SLEEP	4	
#define B43_SHM_SH_MAXBFRAMES		0x0080	
#define B43_SHM_SH_SPUWKUP		0x0094	
#define B43_SHM_SH_PRETBTT		0x0096	


#define B43_SHM_SC_MINCONT		0x0003	
#define B43_SHM_SC_MAXCONT		0x0004	
#define B43_SHM_SC_CURCONT		0x0005	
#define B43_SHM_SC_SRLIMIT		0x0006	
#define B43_SHM_SC_LRLIMIT		0x0007	
#define B43_SHM_SC_DTIMC		0x0008	
#define B43_SHM_SC_BTL0LEN		0x0015	
#define B43_SHM_SC_BTL1LEN		0x0016	
#define B43_SHM_SC_SCFB			0x0017	
#define B43_SHM_SC_LCFB			0x0018	


#define B43_MMIO_RADIO_HWENABLED_HI_MASK (1 << 16)
#define B43_MMIO_RADIO_HWENABLED_LO_MASK (1 << 4)


#define B43_HF_ANTDIVHELP	0x000000000001ULL 
#define B43_HF_SYMW		0x000000000002ULL 
#define B43_HF_RXPULLW		0x000000000004ULL 
#define B43_HF_CCKBOOST		0x000000000008ULL 
#define B43_HF_BTCOEX		0x000000000010ULL 
#define B43_HF_GDCW		0x000000000020ULL 
#define B43_HF_OFDMPABOOST	0x000000000040ULL 
#define B43_HF_ACPR		0x000000000080ULL 
#define B43_HF_EDCF		0x000000000100ULL 
#define B43_HF_TSSIRPSMW	0x000000000200ULL 
#define B43_HF_20IN40IQW	0x000000000200ULL 
#define B43_HF_DSCRQ		0x000000000400ULL 
#define B43_HF_ACIW		0x000000000800ULL 
#define B43_HF_2060W		0x000000001000ULL 
#define B43_HF_RADARW		0x000000002000ULL 
#define B43_HF_USEDEFKEYS	0x000000004000ULL 
#define B43_HF_AFTERBURNER	0x000000008000ULL 
#define B43_HF_BT4PRIOCOEX	0x000000010000ULL 
#define B43_HF_FWKUP		0x000000020000ULL 
#define B43_HF_VCORECALC	0x000000040000ULL 
#define B43_HF_PCISCW		0x000000080000ULL 
#define B43_HF_4318TSSI		0x000000200000ULL 
#define B43_HF_FBCMCFIFO	0x000000400000ULL 
#define B43_HF_HWPCTL		0x000000800000ULL 
#define B43_HF_BTCOEXALT	0x000001000000ULL 
#define B43_HF_TXBTCHECK	0x000002000000ULL 
#define B43_HF_SKCFPUP		0x000004000000ULL 
#define B43_HF_N40W		0x000008000000ULL 
#define B43_HF_ANTSEL		0x000020000000ULL 
#define B43_HF_BT3COEXT		0x000020000000ULL 
#define B43_HF_BTCANT		0x000040000000ULL 
#define B43_HF_ANTSELEN		0x000100000000ULL 
#define B43_HF_ANTSELMODE	0x000200000000ULL 
#define B43_HF_MLADVW		0x001000000000ULL 
#define B43_HF_PR45960W		0x080000000000ULL 


#define B43_FWCAPA_HWCRYPTO	0x0001
#define B43_FWCAPA_QOS		0x0002


#define B43_MACFILTER_SELF		0x0000
#define B43_MACFILTER_BSSID		0x0003


#define B43_PCTL_IN			0xB0
#define B43_PCTL_OUT			0xB4
#define B43_PCTL_OUTENABLE		0xB8
#define B43_PCTL_XTAL_POWERUP		0x40
#define B43_PCTL_PLL_POWERDOWN		0x80


#define B43_PCTL_CLK_FAST		0x00
#define B43_PCTL_CLK_SLOW		0x01
#define B43_PCTL_CLK_DYNAMIC		0x02

#define B43_PCTL_FORCE_SLOW		0x0800
#define B43_PCTL_FORCE_PLL		0x1000
#define B43_PCTL_DYN_XTAL		0x2000


#define B43_PHYTYPE_A			0x00
#define B43_PHYTYPE_B			0x01
#define B43_PHYTYPE_G			0x02
#define B43_PHYTYPE_N			0x04
#define B43_PHYTYPE_LP			0x05


#define B43_PHY_ILT_A_CTRL		0x0072
#define B43_PHY_ILT_A_DATA1		0x0073
#define B43_PHY_ILT_A_DATA2		0x0074
#define B43_PHY_G_LO_CONTROL		0x0810
#define B43_PHY_ILT_G_CTRL		0x0472
#define B43_PHY_ILT_G_DATA1		0x0473
#define B43_PHY_ILT_G_DATA2		0x0474
#define B43_PHY_A_PCTL			0x007B
#define B43_PHY_G_PCTL			0x0029
#define B43_PHY_A_CRS			0x0029
#define B43_PHY_RADIO_BITFIELD		0x0401
#define B43_PHY_G_CRS			0x0429
#define B43_PHY_NRSSILT_CTRL		0x0803
#define B43_PHY_NRSSILT_DATA		0x0804


#define B43_RADIOCTL_ID			0x01


#define B43_MACCTL_ENABLED		0x00000001	
#define B43_MACCTL_PSM_RUN		0x00000002	
#define B43_MACCTL_PSM_JMP0		0x00000004	
#define B43_MACCTL_SHM_ENABLED		0x00000100	
#define B43_MACCTL_SHM_UPPER		0x00000200	
#define B43_MACCTL_IHR_ENABLED		0x00000400	
#define B43_MACCTL_PSM_DBG		0x00002000	
#define B43_MACCTL_GPOUTSMSK		0x0000C000	
#define B43_MACCTL_BE			0x00010000	
#define B43_MACCTL_INFRA		0x00020000	
#define B43_MACCTL_AP			0x00040000	
#define B43_MACCTL_RADIOLOCK		0x00080000	
#define B43_MACCTL_BEACPROMISC		0x00100000	
#define B43_MACCTL_KEEP_BADPLCP		0x00200000	
#define B43_MACCTL_KEEP_CTL		0x00400000	
#define B43_MACCTL_KEEP_BAD		0x00800000	
#define B43_MACCTL_PROMISC		0x01000000	
#define B43_MACCTL_HWPS			0x02000000	
#define B43_MACCTL_AWAKE		0x04000000	
#define B43_MACCTL_CLOSEDNET		0x08000000	
#define B43_MACCTL_TBTTHOLD		0x10000000	
#define B43_MACCTL_DISCTXSTAT		0x20000000	
#define B43_MACCTL_DISCPMQ		0x40000000	
#define B43_MACCTL_GMODE		0x80000000	


#define B43_MACCMD_BEACON0_VALID	0x00000001	
#define B43_MACCMD_BEACON1_VALID	0x00000002	
#define B43_MACCMD_DFQ_VALID		0x00000004	
#define B43_MACCMD_CCA			0x00000008	
#define B43_MACCMD_BGNOISE		0x00000010	


#define B43_TMSLOW_GMODE		0x20000000	
#define B43_TMSLOW_PHYCLKSPEED		0x00C00000	
#define  B43_TMSLOW_PHYCLKSPEED_40MHZ	0x00000000	
#define  B43_TMSLOW_PHYCLKSPEED_80MHZ	0x00400000	
#define  B43_TMSLOW_PHYCLKSPEED_160MHZ	0x00800000	
#define B43_TMSLOW_PLLREFSEL		0x00200000	
#define B43_TMSLOW_MACPHYCLKEN		0x00100000	
#define B43_TMSLOW_PHYRESET		0x00080000	
#define B43_TMSLOW_PHYCLKEN		0x00040000	


#define B43_TMSHIGH_DUALBAND_PHY	0x00080000	
#define B43_TMSHIGH_FCLOCK		0x00040000	
#define B43_TMSHIGH_HAVE_5GHZ_PHY	0x00020000	
#define B43_TMSHIGH_HAVE_2GHZ_PHY	0x00010000	


#define B43_IRQ_MAC_SUSPENDED		0x00000001
#define B43_IRQ_BEACON			0x00000002
#define B43_IRQ_TBTT_INDI		0x00000004
#define B43_IRQ_BEACON_TX_OK		0x00000008
#define B43_IRQ_BEACON_CANCEL		0x00000010
#define B43_IRQ_ATIM_END		0x00000020
#define B43_IRQ_PMQ			0x00000040
#define B43_IRQ_PIO_WORKAROUND		0x00000100
#define B43_IRQ_MAC_TXERR		0x00000200
#define B43_IRQ_PHY_TXERR		0x00000800
#define B43_IRQ_PMEVENT			0x00001000
#define B43_IRQ_TIMER0			0x00002000
#define B43_IRQ_TIMER1			0x00004000
#define B43_IRQ_DMA			0x00008000
#define B43_IRQ_TXFIFO_FLUSH_OK		0x00010000
#define B43_IRQ_CCA_MEASURE_OK		0x00020000
#define B43_IRQ_NOISESAMPLE_OK		0x00040000
#define B43_IRQ_UCODE_DEBUG		0x08000000
#define B43_IRQ_RFKILL			0x10000000
#define B43_IRQ_TX_OK			0x20000000
#define B43_IRQ_PHY_G_CHANGED		0x40000000
#define B43_IRQ_TIMEOUT			0x80000000

#define B43_IRQ_ALL			0xFFFFFFFF
#define B43_IRQ_MASKTEMPLATE		(B43_IRQ_TBTT_INDI | \
					 B43_IRQ_ATIM_END | \
					 B43_IRQ_PMQ | \
					 B43_IRQ_MAC_TXERR | \
					 B43_IRQ_PHY_TXERR | \
					 B43_IRQ_DMA | \
					 B43_IRQ_TXFIFO_FLUSH_OK | \
					 B43_IRQ_NOISESAMPLE_OK | \
					 B43_IRQ_UCODE_DEBUG | \
					 B43_IRQ_RFKILL | \
					 B43_IRQ_TX_OK)


#define B43_DEBUGIRQ_REASON_REG		63

#define B43_DEBUGIRQ_PANIC		0	
#define B43_DEBUGIRQ_DUMP_SHM		1	
#define B43_DEBUGIRQ_DUMP_REGS		2	
#define B43_DEBUGIRQ_MARKER		3	
#define B43_DEBUGIRQ_ACK		0xFFFF	


#define B43_MARKER_ID_REG		2
#define B43_MARKER_LINE_REG		3


#define B43_FWPANIC_REASON_REG		3

#define B43_FWPANIC_DIE			0 
#define B43_FWPANIC_RESTART		1 


#define B43_WATCHDOG_REG		1


#define B43_CCK_RATE_1MB		0x02
#define B43_CCK_RATE_2MB		0x04
#define B43_CCK_RATE_5MB		0x0B
#define B43_CCK_RATE_11MB		0x16
#define B43_OFDM_RATE_6MB		0x0C
#define B43_OFDM_RATE_9MB		0x12
#define B43_OFDM_RATE_12MB		0x18
#define B43_OFDM_RATE_18MB		0x24
#define B43_OFDM_RATE_24MB		0x30
#define B43_OFDM_RATE_36MB		0x48
#define B43_OFDM_RATE_48MB		0x60
#define B43_OFDM_RATE_54MB		0x6C

#define B43_RATE_TO_BASE100KBPS(rate)	(((rate) * 10) / 2)

#define B43_DEFAULT_SHORT_RETRY_LIMIT	7
#define B43_DEFAULT_LONG_RETRY_LIMIT	4

#define B43_PHY_TX_BADNESS_LIMIT	1000


#define B43_SEC_KEYSIZE			16

#define B43_NR_GROUP_KEYS		4

#define B43_NR_PAIRWISE_KEYS		50

enum {
	B43_SEC_ALGO_NONE = 0,	
	B43_SEC_ALGO_WEP40,
	B43_SEC_ALGO_TKIP,
	B43_SEC_ALGO_AES,
	B43_SEC_ALGO_WEP104,
	B43_SEC_ALGO_AES_LEGACY,
};

struct b43_dmaring;


#define B43_FW_TYPE_UCODE	'u'
#define B43_FW_TYPE_PCM		'p'
#define B43_FW_TYPE_IV		'i'
struct b43_fw_header {
	
	u8 type;
	
	u8 ver;
	u8 __padding[2];
	
	__be32 size;
} __attribute__((__packed__));


#define B43_IV_OFFSET_MASK	0x7FFF
#define B43_IV_32BIT		0x8000
struct b43_iv {
	__be16 offset_size;
	union {
		__be16 d16;
		__be32 d32;
	} data __attribute__((__packed__));
} __attribute__((__packed__));



struct b43_dma {
	struct b43_dmaring *tx_ring_AC_BK; 
	struct b43_dmaring *tx_ring_AC_BE; 
	struct b43_dmaring *tx_ring_AC_VI; 
	struct b43_dmaring *tx_ring_AC_VO; 
	struct b43_dmaring *tx_ring_mcast; 

	struct b43_dmaring *rx_ring;
};

struct b43_pio_txqueue;
struct b43_pio_rxqueue;


struct b43_pio {
	struct b43_pio_txqueue *tx_queue_AC_BK; 
	struct b43_pio_txqueue *tx_queue_AC_BE; 
	struct b43_pio_txqueue *tx_queue_AC_VI; 
	struct b43_pio_txqueue *tx_queue_AC_VO; 
	struct b43_pio_txqueue *tx_queue_mcast; 

	struct b43_pio_rxqueue *rx_queue;
};


struct b43_noise_calculation {
	bool calculation_running;
	u8 nr_samples;
	s8 samples[8][4];
};

struct b43_stats {
	u8 link_noise;
};

struct b43_key {
	
	struct ieee80211_key_conf *keyconf;
	u8 algorithm;
};


#define B43_QOS_PARAMS(queue)	(B43_SHM_SH_EDCFQ + \
				 (B43_NR_QOSPARAMS * sizeof(u16) * (queue)))
#define B43_QOS_BACKGROUND	B43_QOS_PARAMS(0)
#define B43_QOS_BESTEFFORT	B43_QOS_PARAMS(1)
#define B43_QOS_VIDEO		B43_QOS_PARAMS(2)
#define B43_QOS_VOICE		B43_QOS_PARAMS(3)


#define B43_NR_QOSPARAMS	16
enum {
	B43_QOSPARAM_TXOP = 0,
	B43_QOSPARAM_CWMIN,
	B43_QOSPARAM_CWMAX,
	B43_QOSPARAM_CWCUR,
	B43_QOSPARAM_AIFS,
	B43_QOSPARAM_BSLOTS,
	B43_QOSPARAM_REGGAP,
	B43_QOSPARAM_STATUS,
};


struct b43_qos_params {
	
	struct ieee80211_tx_queue_params p;
};

struct b43_wl;


enum b43_firmware_file_type {
	B43_FWTYPE_PROPRIETARY,
	B43_FWTYPE_OPENSOURCE,
	B43_NR_FWTYPES,
};


struct b43_request_fw_context {
	
	struct b43_wldev *dev;
	
	enum b43_firmware_file_type req_type;
	
	char errors[B43_NR_FWTYPES][128];
	
	char fwname[64];
	
	int fatal_failure;
};


struct b43_firmware_file {
	const char *filename;
	const struct firmware *data;
	
	enum b43_firmware_file_type type;
};


struct b43_firmware {
	
	struct b43_firmware_file ucode;
	
	struct b43_firmware_file pcm;
	
	struct b43_firmware_file initvals;
	
	struct b43_firmware_file initvals_band;

	
	u16 rev;
	
	u16 patch;

	
	bool opensource;
	
	bool pcm_request_failed;
};


enum {
	B43_STAT_UNINIT = 0,	
	B43_STAT_INITIALIZED = 1,	
	B43_STAT_STARTED = 2,	
};
#define b43_status(wldev)		atomic_read(&(wldev)->__init_status)
#define b43_set_status(wldev, stat)	do {			\
		atomic_set(&(wldev)->__init_status, (stat));	\
		smp_wmb();					\
					} while (0)


struct b43_wldev {
	struct ssb_device *dev;
	struct b43_wl *wl;

	
	atomic_t __init_status;

	bool bad_frames_preempt;	
	bool dfq_valid;		
	bool radio_hw_enable;	
	bool qos_enabled;		
	bool hwcrypto_enabled;		

	
	struct b43_phy phy;

	union {
		
		struct b43_dma dma;
		
		struct b43_pio pio;
	};
	
	bool __using_pio_transfers;

	
	struct b43_stats stats;

	
	u32 irq_reason;
	u32 dma_reason[6];
	
	u32 irq_mask;

	
	struct b43_noise_calculation noisecalc;
	
	int mac_suspended;

	
	struct delayed_work periodic_work;
	unsigned int periodic_state;

	struct work_struct restart_work;

	
	u16 ktp;		
	struct b43_key key[B43_NR_GROUP_KEYS * 2 + B43_NR_PAIRWISE_KEYS];

	
	struct b43_firmware fw;

	
	struct list_head list;

	
#ifdef CONFIG_B43_DEBUG
	struct b43_dfsentry *dfsentry;
	unsigned int irq_count;
	unsigned int irq_bit_count[32];
	unsigned int tx_count;
	unsigned int rx_count;
#endif
};


#include "xmit.h"


struct b43_wl {
	
	struct b43_wldev *current_dev;
	
	struct ieee80211_hw *hw;

	
	struct mutex mutex;
	
	spinlock_t hardirq_lock;

	
	u16 mac80211_initially_registered_queues;

	

	struct ieee80211_vif *vif;
	
	u8 mac_addr[ETH_ALEN];
	
	u8 bssid[ETH_ALEN];
	
	int if_type;
	
	bool operating;
	
	unsigned int filter_flags;
	
	struct ieee80211_low_level_stats ieee_stats;

#ifdef CONFIG_B43_HWRNG
	struct hwrng rng;
	bool rng_initialized;
	char rng_name[30 + 1];
#endif 

	
	struct list_head devlist;
	u8 nr_devs;

	bool radiotap_enabled;
	bool radio_enabled;

	
	struct sk_buff *current_beacon;
	bool beacon0_uploaded;
	bool beacon1_uploaded;
	bool beacon_templates_virgin; 
	struct work_struct beacon_update_trigger;

	
	struct b43_qos_params qos_params[4];

	
	struct work_struct txpower_adjust_work;

	
	struct work_struct tx_work;
	
	struct sk_buff_head tx_queue;

	
	struct b43_leds leds;

#ifdef CONFIG_B43_PIO
	
	struct b43_rxhdr_fw4 rxhdr;
	struct b43_txhdr txhdr;
	u8 rx_tail[4];
	u8 tx_tail[4];
#endif 
};

static inline struct b43_wl *hw_to_b43_wl(struct ieee80211_hw *hw)
{
	return hw->priv;
}

static inline struct b43_wldev *dev_to_b43_wldev(struct device *dev)
{
	struct ssb_device *ssb_dev = dev_to_ssb_dev(dev);
	return ssb_get_drvdata(ssb_dev);
}


static inline int b43_is_mode(struct b43_wl *wl, int type)
{
	return (wl->operating && wl->if_type == type);
}


static inline enum ieee80211_band b43_current_band(struct b43_wl *wl)
{
	return wl->hw->conf.channel->band;
}

static inline u16 b43_read16(struct b43_wldev *dev, u16 offset)
{
	return ssb_read16(dev->dev, offset);
}

static inline void b43_write16(struct b43_wldev *dev, u16 offset, u16 value)
{
	ssb_write16(dev->dev, offset, value);
}

static inline u32 b43_read32(struct b43_wldev *dev, u16 offset)
{
	return ssb_read32(dev->dev, offset);
}

static inline void b43_write32(struct b43_wldev *dev, u16 offset, u32 value)
{
	ssb_write32(dev->dev, offset, value);
}

static inline bool b43_using_pio_transfers(struct b43_wldev *dev)
{
#ifdef CONFIG_B43_PIO
	return dev->__using_pio_transfers;
#else
	return 0;
#endif
}

#ifdef CONFIG_B43_FORCE_PIO
# define B43_FORCE_PIO	1
#else
# define B43_FORCE_PIO	0
#endif



void b43info(struct b43_wl *wl, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));
void b43err(struct b43_wl *wl, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));
void b43warn(struct b43_wl *wl, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));
void b43dbg(struct b43_wl *wl, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));



#if B43_DEBUG
# define B43_WARN_ON(x)	WARN_ON(x)
#else
static inline bool __b43_warn_on_dummy(bool x) { return x; }
# define B43_WARN_ON(x)	__b43_warn_on_dummy(unlikely(!!(x)))
#endif


#define INT_TO_Q52(i)	((i) << 2)

#define Q52_TO_INT(q52)	((q52) >> 2)

#define Q52_FMT		"%u.%u"
#define Q52_ARG(q52)	Q52_TO_INT(q52), ((((q52) & 0x3) * 100) / 4)

#endif 
