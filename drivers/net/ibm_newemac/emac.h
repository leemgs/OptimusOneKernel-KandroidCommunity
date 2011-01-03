
#ifndef __IBM_NEWEMAC_H
#define __IBM_NEWEMAC_H

#include <linux/types.h>


struct emac_regs {
	
	u32 mr0;			
	u32 mr1;			
	u32 tmr0;			
	u32 tmr1;			
	u32 rmr;			
	u32 isr;			
	u32 iser;			
	u32 iahr;			
	u32 ialr;			
	u32 vtpid;			
	u32 vtci;			
	u32 ptr;			
	union {
		
		struct {
			u32 iaht1;	
			u32 iaht2;	
			u32 iaht3;	
			u32 iaht4;	
			u32 gaht1;	
			u32 gaht2;	
			u32 gaht3;	
			u32 gaht4;	
		} emac4;
		
		struct {
			u32 mahr;	
			u32 malr;	
			u32 mmahr;	
			u32 mmalr;	
			u32 rsvd0[4];
		} emac4sync;
	} u0;
	
	u32 lsah;
	u32 lsal;
	u32 ipgvr;			
	u32 stacr;			
	u32 trtr;			
	u32 rwmr;			
	u32 octx;
	u32 ocrx;
	union {
		
		struct {
			u32 ipcr;
		} emac4;
		
		struct {
			u32 rsvd1;
			u32 revid;
 			u32 rsvd2[2];
			u32 iaht1;	
			u32 iaht2;	
			u32 iaht3;	
			u32 iaht4;	
			u32 iaht5;	
			u32 iaht6;	
			u32 iaht7;	
			u32 iaht8;	
			u32 gaht1;	
			u32 gaht2;	
			u32 gaht3;	
			u32 gaht4;	
			u32 gaht5;	
			u32 gaht6;	
			u32 gaht7;	
			u32 gaht8;	
			u32 tpc;	
		} emac4sync;
	} u1;
};


#define PHY_MODE_NA	0
#define PHY_MODE_MII	1
#define PHY_MODE_RMII	2
#define PHY_MODE_SMII	3
#define PHY_MODE_RGMII	4
#define PHY_MODE_TBI	5
#define PHY_MODE_GMII	6
#define PHY_MODE_RTBI	7
#define PHY_MODE_SGMII	8


#define EMAC_MR0_RXI			0x80000000
#define EMAC_MR0_TXI			0x40000000
#define EMAC_MR0_SRST			0x20000000
#define EMAC_MR0_TXE			0x10000000
#define EMAC_MR0_RXE			0x08000000
#define EMAC_MR0_WKE			0x04000000


#define EMAC_MR1_FDE			0x80000000
#define EMAC_MR1_ILE			0x40000000
#define EMAC_MR1_VLE			0x20000000
#define EMAC_MR1_EIFC			0x10000000
#define EMAC_MR1_APP			0x08000000
#define EMAC_MR1_IST			0x01000000

#define EMAC_MR1_MF_MASK		0x00c00000
#define EMAC_MR1_MF_10			0x00000000
#define EMAC_MR1_MF_100			0x00400000
#define EMAC_MR1_MF_1000		0x00800000
#define EMAC_MR1_MF_1000GPCS		0x00c00000
#define EMAC_MR1_MF_IPPA(id)		(((id) & 0x1f) << 6)

#define EMAC_MR1_RFS_4K			0x00300000
#define EMAC_MR1_RFS_16K		0x00000000
#define EMAC_MR1_TFS_2K			0x00080000
#define EMAC_MR1_TR0_MULT		0x00008000
#define EMAC_MR1_JPSM			0x00000000
#define EMAC_MR1_MWSW_001		0x00000000
#define EMAC_MR1_BASE(opb)		(EMAC_MR1_TFS_2K | EMAC_MR1_TR0_MULT)


#define EMAC4_MR1_RFS_2K		0x00100000
#define EMAC4_MR1_RFS_4K		0x00180000
#define EMAC4_MR1_RFS_16K		0x00280000
#define EMAC4_MR1_TFS_2K       		0x00020000
#define EMAC4_MR1_TFS_4K		0x00030000
#define EMAC4_MR1_TFS_16K		0x00050000
#define EMAC4_MR1_TR			0x00008000
#define EMAC4_MR1_MWSW_001		0x00001000
#define EMAC4_MR1_JPSM			0x00000800
#define EMAC4_MR1_OBCI_MASK		0x00000038
#define EMAC4_MR1_OBCI_50		0x00000000
#define EMAC4_MR1_OBCI_66		0x00000008
#define EMAC4_MR1_OBCI_83		0x00000010
#define EMAC4_MR1_OBCI_100		0x00000018
#define EMAC4_MR1_OBCI_100P		0x00000020
#define EMAC4_MR1_OBCI(freq)		((freq) <= 50  ? EMAC4_MR1_OBCI_50 : \
					 (freq) <= 66  ? EMAC4_MR1_OBCI_66 : \
					 (freq) <= 83  ? EMAC4_MR1_OBCI_83 : \
					 (freq) <= 100 ? EMAC4_MR1_OBCI_100 : \
						EMAC4_MR1_OBCI_100P)


#define EMAC_TMR0_GNP			0x80000000
#define EMAC_TMR0_DEFAULT		0x00000000
#define EMAC4_TMR0_TFAE_2_32		0x00000001
#define EMAC4_TMR0_TFAE_4_64		0x00000002
#define EMAC4_TMR0_TFAE_8_128		0x00000003
#define EMAC4_TMR0_TFAE_16_256		0x00000004
#define EMAC4_TMR0_TFAE_32_512		0x00000005
#define EMAC4_TMR0_TFAE_64_1024		0x00000006
#define EMAC4_TMR0_TFAE_128_2048	0x00000007
#define EMAC4_TMR0_DEFAULT		EMAC4_TMR0_TFAE_2_32
#define EMAC_TMR0_XMIT			(EMAC_TMR0_GNP | EMAC_TMR0_DEFAULT)
#define EMAC4_TMR0_XMIT			(EMAC_TMR0_GNP | EMAC4_TMR0_DEFAULT)



#define EMAC_TMR1(l,h)			(((l) << 27) | (((h) & 0xff) << 16))
#define EMAC4_TMR1(l,h)			(((l) << 27) | (((h) & 0x3ff) << 14))


#define EMAC_RMR_SP			0x80000000
#define EMAC_RMR_SFCS			0x40000000
#define EMAC_RMR_RRP			0x20000000
#define EMAC_RMR_RFP			0x10000000
#define EMAC_RMR_ROP			0x08000000
#define EMAC_RMR_RPIR			0x04000000
#define EMAC_RMR_PPP			0x02000000
#define EMAC_RMR_PME			0x01000000
#define EMAC_RMR_PMME			0x00800000
#define EMAC_RMR_IAE			0x00400000
#define EMAC_RMR_MIAE			0x00200000
#define EMAC_RMR_BAE			0x00100000
#define EMAC_RMR_MAE			0x00080000
#define EMAC_RMR_BASE			0x00000000
#define EMAC4_RMR_RFAF_2_32		0x00000001
#define EMAC4_RMR_RFAF_4_64		0x00000002
#define EMAC4_RMR_RFAF_8_128		0x00000003
#define EMAC4_RMR_RFAF_16_256		0x00000004
#define EMAC4_RMR_RFAF_32_512		0x00000005
#define EMAC4_RMR_RFAF_64_1024		0x00000006
#define EMAC4_RMR_RFAF_128_2048		0x00000007
#define EMAC4_RMR_BASE			EMAC4_RMR_RFAF_128_2048


#define EMAC4_ISR_TXPE			0x20000000
#define EMAC4_ISR_RXPE			0x10000000
#define EMAC4_ISR_TXUE			0x08000000
#define EMAC4_ISR_RXOE			0x04000000
#define EMAC_ISR_OVR			0x02000000
#define EMAC_ISR_PP			0x01000000
#define EMAC_ISR_BP			0x00800000
#define EMAC_ISR_RP			0x00400000
#define EMAC_ISR_SE			0x00200000
#define EMAC_ISR_ALE			0x00100000
#define EMAC_ISR_BFCS			0x00080000
#define EMAC_ISR_PTLE			0x00040000
#define EMAC_ISR_ORE			0x00020000
#define EMAC_ISR_IRE			0x00010000
#define EMAC_ISR_SQE			0x00000080
#define EMAC_ISR_TE			0x00000040
#define EMAC_ISR_MOS			0x00000002
#define EMAC_ISR_MOF			0x00000001


#define EMAC_STACR_PHYD_MASK		0xffff
#define EMAC_STACR_PHYD_SHIFT		16
#define EMAC_STACR_OC			0x00008000
#define EMAC_STACR_PHYE			0x00004000
#define EMAC_STACR_STAC_MASK		0x00003000
#define EMAC_STACR_STAC_READ		0x00001000
#define EMAC_STACR_STAC_WRITE		0x00002000
#define EMAC_STACR_OPBC_MASK		0x00000C00
#define EMAC_STACR_OPBC_50		0x00000000
#define EMAC_STACR_OPBC_66		0x00000400
#define EMAC_STACR_OPBC_83		0x00000800
#define EMAC_STACR_OPBC_100		0x00000C00
#define EMAC_STACR_OPBC(freq)		((freq) <= 50 ? EMAC_STACR_OPBC_50 : \
					 (freq) <= 66 ? EMAC_STACR_OPBC_66 : \
					 (freq) <= 83 ? EMAC_STACR_OPBC_83 : EMAC_STACR_OPBC_100)
#define EMAC_STACR_BASE(opb)		EMAC_STACR_OPBC(opb)
#define EMAC4_STACR_BASE(opb)		0x00000000
#define EMAC_STACR_PCDA_MASK		0x1f
#define EMAC_STACR_PCDA_SHIFT		5
#define EMAC_STACR_PRA_MASK		0x1f
#define EMACX_STACR_STAC_MASK		0x00003800
#define EMACX_STACR_STAC_READ		0x00001000
#define EMACX_STACR_STAC_WRITE		0x00000800
#define EMACX_STACR_STAC_IND_ADDR	0x00002000
#define EMACX_STACR_STAC_IND_READ	0x00003800
#define EMACX_STACR_STAC_IND_READINC	0x00003000
#define EMACX_STACR_STAC_IND_WRITE	0x00002800



#define EMAC_TRTR_SHIFT_EMAC4		24
#define EMAC_TRTR_SHIFT		27


#define EMAC_TX_CTRL_GFCS		0x0200
#define EMAC_TX_CTRL_GP			0x0100
#define EMAC_TX_CTRL_ISA		0x0080
#define EMAC_TX_CTRL_RSA		0x0040
#define EMAC_TX_CTRL_IVT		0x0020
#define EMAC_TX_CTRL_RVT		0x0010
#define EMAC_TX_CTRL_TAH_CSUM		0x000e


#define EMAC_TX_ST_BFCS			0x0200
#define EMAC_TX_ST_LCS			0x0080
#define EMAC_TX_ST_ED			0x0040
#define EMAC_TX_ST_EC			0x0020
#define EMAC_TX_ST_LC			0x0010
#define EMAC_TX_ST_MC			0x0008
#define EMAC_TX_ST_SC			0x0004
#define EMAC_TX_ST_UR			0x0002
#define EMAC_TX_ST_SQE			0x0001
#define EMAC_IS_BAD_TX			(EMAC_TX_ST_LCS | EMAC_TX_ST_ED | \
					 EMAC_TX_ST_EC | EMAC_TX_ST_LC | \
					 EMAC_TX_ST_MC | EMAC_TX_ST_UR)
#define EMAC_IS_BAD_TX_TAH		(EMAC_TX_ST_LCS | EMAC_TX_ST_ED | \
					 EMAC_TX_ST_EC | EMAC_TX_ST_LC)


#define EMAC_RX_ST_OE			0x0200
#define EMAC_RX_ST_PP			0x0100
#define EMAC_RX_ST_BP			0x0080
#define EMAC_RX_ST_RP			0x0040
#define EMAC_RX_ST_SE			0x0020
#define EMAC_RX_ST_AE			0x0010
#define EMAC_RX_ST_BFCS			0x0008
#define EMAC_RX_ST_PTL			0x0004
#define EMAC_RX_ST_ORE			0x0002
#define EMAC_RX_ST_IRE			0x0001
#define EMAC_RX_TAH_BAD_CSUM		0x0003
#define EMAC_BAD_RX_MASK		(EMAC_RX_ST_OE | EMAC_RX_ST_BP | \
					 EMAC_RX_ST_RP | EMAC_RX_ST_SE | \
					 EMAC_RX_ST_AE | EMAC_RX_ST_BFCS | \
					 EMAC_RX_ST_PTL | EMAC_RX_ST_ORE | \
					 EMAC_RX_ST_IRE )
#endif 
