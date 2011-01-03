

#include "et131x_version.h"
#include "et131x_defs.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>

#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/in.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/pci.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/ioport.h>
#include <linux/crc32.h>

#include "et1310_phy.h"
#include "et1310_pm.h"
#include "et1310_jagcore.h"
#include "et1310_mac.h"

#include "et131x_adapter.h"
#include "et131x_initpci.h"


void ConfigMACRegs1(struct et131x_adapter *etdev)
{
	struct _MAC_t __iomem *pMac = &etdev->regs->mac;
	MAC_STATION_ADDR1_t station1;
	MAC_STATION_ADDR2_t station2;
	MAC_IPG_t ipg;
	MAC_HFDP_t hfdp;
	MII_MGMT_CFG_t mii_mgmt_cfg;

	
	writel(0xC00F0000, &pMac->cfg1.value);

	
	ipg.bits.non_B2B_ipg_1 = 0x38;		
	ipg.bits.non_B2B_ipg_2 = 0x58;		
	ipg.bits.min_ifg_enforce = 0x50;	
	ipg.bits.B2B_ipg = 0x60;		
	writel(ipg.value, &pMac->ipg.value);

	
	hfdp.bits.alt_beb_trunc = 0xA;
	hfdp.bits.alt_beb_enable = 0x0;
	hfdp.bits.bp_no_backoff = 0x0;
	hfdp.bits.no_backoff = 0x0;
	hfdp.bits.excess_defer = 0x1;
	hfdp.bits.rexmit_max = 0xF;
	hfdp.bits.coll_window = 0x37;		
	writel(hfdp.value, &pMac->hfdp.value);

	
	writel(0, &pMac->if_ctrl.value);

	
	mii_mgmt_cfg.bits.reset_mii_mgmt = 0;
	mii_mgmt_cfg.bits.scan_auto_incremt = 0;
	mii_mgmt_cfg.bits.preamble_suppress = 0;
	mii_mgmt_cfg.bits.mgmt_clk_reset = 0x7;
	writel(mii_mgmt_cfg.value, &pMac->mii_mgmt_cfg.value);

	
	station2.bits.Octet1 = etdev->CurrentAddress[0];
	station2.bits.Octet2 = etdev->CurrentAddress[1];
	station1.bits.Octet3 = etdev->CurrentAddress[2];
	station1.bits.Octet4 = etdev->CurrentAddress[3];
	station1.bits.Octet5 = etdev->CurrentAddress[4];
	station1.bits.Octet6 = etdev->CurrentAddress[5];
	writel(station1.value, &pMac->station_addr_1.value);
	writel(station2.value, &pMac->station_addr_2.value);

	
	writel(etdev->RegistryJumboPacket + 4, &pMac->max_fm_len.value);

	
	writel(0, &pMac->cfg1.value);
}


void ConfigMACRegs2(struct et131x_adapter *etdev)
{
	int32_t delay = 0;
	struct _MAC_t __iomem *pMac = &etdev->regs->mac;
	MAC_CFG1_t cfg1;
	MAC_CFG2_t cfg2;
	MAC_IF_CTRL_t ifctrl;
	TXMAC_CTL_t ctl;

	ctl.value = readl(&etdev->regs->txmac.ctl.value);
	cfg1.value = readl(&pMac->cfg1.value);
	cfg2.value = readl(&pMac->cfg2.value);
	ifctrl.value = readl(&pMac->if_ctrl.value);

	if (etdev->linkspeed == TRUEPHY_SPEED_1000MBPS) {
		cfg2.bits.if_mode = 0x2;
		ifctrl.bits.phy_mode = 0x0;
	} else {
		cfg2.bits.if_mode = 0x1;
		ifctrl.bits.phy_mode = 0x1;
	}

	
	cfg1.bits.rx_enable = 0x1;
	cfg1.bits.tx_enable = 0x1;

	
	cfg1.bits.tx_flow = 0x1;

	if ((etdev->FlowControl == RxOnly) ||
	    (etdev->FlowControl == Both)) {
		cfg1.bits.rx_flow = 0x1;
	} else {
		cfg1.bits.rx_flow = 0x0;
	}

	
	cfg1.bits.loop_back = 0;

	writel(cfg1.value, &pMac->cfg1.value);

	
	cfg2.bits.preamble_len = 0x7;
	cfg2.bits.huge_frame = 0x0;
	
	cfg2.bits.len_check = 0x1;

	if (etdev->RegistryPhyLoopbk == false) {
		cfg2.bits.pad_crc = 0x1;
		cfg2.bits.crc_enable = 0x1;
	} else {
		cfg2.bits.pad_crc = 0;
		cfg2.bits.crc_enable = 0;
	}

	
	cfg2.bits.full_duplex = etdev->duplex_mode;
	ifctrl.bits.ghd_mode = !etdev->duplex_mode;

	writel(ifctrl.value, &pMac->if_ctrl.value);
	writel(cfg2.value, &pMac->cfg2.value);

	do {
		udelay(10);
		delay++;
		cfg1.value = readl(&pMac->cfg1.value);
	} while ((!cfg1.bits.syncd_rx_en || !cfg1.bits.syncd_tx_en) &&
								 delay < 100);

	if (delay == 100) {
		dev_warn(&etdev->pdev->dev,
		    "Syncd bits did not respond correctly cfg1 word 0x%08x\n",
			cfg1.value);
	}

	
	ctl.bits.txmac_en = 0x1;
	ctl.bits.fc_disable = 0x1;
	writel(ctl.value, &etdev->regs->txmac.ctl.value);

	
	if (etdev->Flags & fMP_ADAPTER_LOWER_POWER) {
		et131x_rx_dma_enable(etdev);
		et131x_tx_dma_enable(etdev);
	}
}

void ConfigRxMacRegs(struct et131x_adapter *etdev)
{
	struct _RXMAC_t __iomem *pRxMac = &etdev->regs->rxmac;
	RXMAC_WOL_SA_LO_t sa_lo;
	RXMAC_WOL_SA_HI_t sa_hi;
	RXMAC_PF_CTRL_t pf_ctrl = { 0 };

	
	writel(0x8, &pRxMac->ctrl.value);

	
	writel(0, &pRxMac->crc0.value);
	writel(0, &pRxMac->crc12.value);
	writel(0, &pRxMac->crc34.value);

	
	writel(0, &pRxMac->mask0_word0);
	writel(0, &pRxMac->mask0_word1);
	writel(0, &pRxMac->mask0_word2);
	writel(0, &pRxMac->mask0_word3);

	writel(0, &pRxMac->mask1_word0);
	writel(0, &pRxMac->mask1_word1);
	writel(0, &pRxMac->mask1_word2);
	writel(0, &pRxMac->mask1_word3);

	writel(0, &pRxMac->mask2_word0);
	writel(0, &pRxMac->mask2_word1);
	writel(0, &pRxMac->mask2_word2);
	writel(0, &pRxMac->mask2_word3);

	writel(0, &pRxMac->mask3_word0);
	writel(0, &pRxMac->mask3_word1);
	writel(0, &pRxMac->mask3_word2);
	writel(0, &pRxMac->mask3_word3);

	writel(0, &pRxMac->mask4_word0);
	writel(0, &pRxMac->mask4_word1);
	writel(0, &pRxMac->mask4_word2);
	writel(0, &pRxMac->mask4_word3);

	
	sa_lo.bits.sa3 = etdev->CurrentAddress[2];
	sa_lo.bits.sa4 = etdev->CurrentAddress[3];
	sa_lo.bits.sa5 = etdev->CurrentAddress[4];
	sa_lo.bits.sa6 = etdev->CurrentAddress[5];
	writel(sa_lo.value, &pRxMac->sa_lo.value);

	sa_hi.bits.sa1 = etdev->CurrentAddress[0];
	sa_hi.bits.sa2 = etdev->CurrentAddress[1];
	writel(sa_hi.value, &pRxMac->sa_hi.value);

	
	writel(0, &pRxMac->pf_ctrl.value);

	
	if (etdev->PacketFilter & ET131X_PACKET_TYPE_DIRECTED) {
		SetupDeviceForUnicast(etdev);
		pf_ctrl.bits.filter_uni_en = 1;
	} else {
		writel(0, &pRxMac->uni_pf_addr1.value);
		writel(0, &pRxMac->uni_pf_addr2.value);
		writel(0, &pRxMac->uni_pf_addr3.value);
	}

	
	if (etdev->PacketFilter & ET131X_PACKET_TYPE_ALL_MULTICAST) {
		pf_ctrl.bits.filter_multi_en = 0;
	} else {
		pf_ctrl.bits.filter_multi_en = 1;
		SetupDeviceForMulticast(etdev);
	}

	
	pf_ctrl.bits.min_pkt_size = NIC_MIN_PACKET_SIZE + 4;
	pf_ctrl.bits.filter_frag_en = 1;

	if (etdev->RegistryJumboPacket > 8192) {
		RXMAC_MCIF_CTRL_MAX_SEG_t mcif_ctrl_max_seg;

		
		mcif_ctrl_max_seg.bits.seg_en = 0x1;
		mcif_ctrl_max_seg.bits.fc_en = 0x0;
		mcif_ctrl_max_seg.bits.max_size = 0x10;

		writel(mcif_ctrl_max_seg.value,
		       &pRxMac->mcif_ctrl_max_seg.value);
	} else {
		writel(0, &pRxMac->mcif_ctrl_max_seg.value);
	}

	
	writel(0, &pRxMac->mcif_water_mark.value);

	
	writel(0, &pRxMac->mif_ctrl.value);

	
	writel(0, &pRxMac->space_avail.value);

	
	if (etdev->linkspeed == TRUEPHY_SPEED_100MBPS)
		writel(0x30038, &pRxMac->mif_ctrl.value);
	else
		writel(0x30030, &pRxMac->mif_ctrl.value);

	
	writel(pf_ctrl.value, &pRxMac->pf_ctrl.value);
	writel(0x9, &pRxMac->ctrl.value);
}

void ConfigTxMacRegs(struct et131x_adapter *etdev)
{
	struct _TXMAC_t __iomem *pTxMac = &etdev->regs->txmac;
	TXMAC_CF_PARAM_t Local;

	
	if (etdev->FlowControl == None) {
		writel(0, &pTxMac->cf_param.value);
	} else {
		Local.bits.cfpt = 0x40;
		Local.bits.cfep = 0x0;
		writel(Local.value, &pTxMac->cf_param.value);
	}
}

void ConfigMacStatRegs(struct et131x_adapter *etdev)
{
	struct _MAC_STAT_t __iomem *pDevMacStat =
		&etdev->regs->macStat;

	
	writel(0, &pDevMacStat->RFcs);
	writel(0, &pDevMacStat->RAln);
	writel(0, &pDevMacStat->RFlr);
	writel(0, &pDevMacStat->RDrp);
	writel(0, &pDevMacStat->RCde);
	writel(0, &pDevMacStat->ROvr);
	writel(0, &pDevMacStat->RFrg);

	writel(0, &pDevMacStat->TScl);
	writel(0, &pDevMacStat->TDfr);
	writel(0, &pDevMacStat->TMcl);
	writel(0, &pDevMacStat->TLcl);
	writel(0, &pDevMacStat->TNcl);
	writel(0, &pDevMacStat->TOvr);
	writel(0, &pDevMacStat->TUnd);

	
	{
		MAC_STAT_REG_1_t Carry1M = { 0xffffffff };

		Carry1M.bits.rdrp = 0;
		Carry1M.bits.rjbr = 1;
		Carry1M.bits.rfrg = 0;
		Carry1M.bits.rovr = 0;
		Carry1M.bits.rund = 1;
		Carry1M.bits.rcse = 1;
		Carry1M.bits.rcde = 0;
		Carry1M.bits.rflr = 0;
		Carry1M.bits.raln = 0;
		Carry1M.bits.rxuo = 1;
		Carry1M.bits.rxpf = 1;
		Carry1M.bits.rxcf = 1;
		Carry1M.bits.rbca = 1;
		Carry1M.bits.rmca = 1;
		Carry1M.bits.rfcs = 0;
		Carry1M.bits.rpkt = 1;
		Carry1M.bits.rbyt = 1;
		Carry1M.bits.trmgv = 1;
		Carry1M.bits.trmax = 1;
		Carry1M.bits.tr1k = 1;
		Carry1M.bits.tr511 = 1;
		Carry1M.bits.tr255 = 1;
		Carry1M.bits.tr127 = 1;
		Carry1M.bits.tr64 = 1;

		writel(Carry1M.value, &pDevMacStat->Carry1M.value);
	}

	{
		MAC_STAT_REG_2_t Carry2M = { 0xffffffff };

		Carry2M.bits.tdrp = 1;
		Carry2M.bits.tpfh = 1;
		Carry2M.bits.tncl = 0;
		Carry2M.bits.txcl = 1;
		Carry2M.bits.tlcl = 0;
		Carry2M.bits.tmcl = 0;
		Carry2M.bits.tscl = 0;
		Carry2M.bits.tedf = 1;
		Carry2M.bits.tdfr = 0;
		Carry2M.bits.txpf = 1;
		Carry2M.bits.tbca = 1;
		Carry2M.bits.tmca = 1;
		Carry2M.bits.tpkt = 1;
		Carry2M.bits.tbyt = 1;
		Carry2M.bits.tfrg = 1;
		Carry2M.bits.tund = 0;
		Carry2M.bits.tovr = 0;
		Carry2M.bits.txcf = 1;
		Carry2M.bits.tfcs = 1;
		Carry2M.bits.tjbr = 1;

		writel(Carry2M.value, &pDevMacStat->Carry2M.value);
	}
}

void ConfigFlowControl(struct et131x_adapter *etdev)
{
	if (etdev->duplex_mode == 0) {
		etdev->FlowControl = None;
	} else {
		char RemotePause, RemoteAsyncPause;

		ET1310_PhyAccessMiBit(etdev,
				      TRUEPHY_BIT_READ, 5, 10, &RemotePause);
		ET1310_PhyAccessMiBit(etdev,
				      TRUEPHY_BIT_READ, 5, 11,
				      &RemoteAsyncPause);

		if ((RemotePause == TRUEPHY_BIT_SET) &&
		    (RemoteAsyncPause == TRUEPHY_BIT_SET)) {
			etdev->FlowControl = etdev->RegistryFlowControl;
		} else if ((RemotePause == TRUEPHY_BIT_SET) &&
			   (RemoteAsyncPause == TRUEPHY_BIT_CLEAR)) {
			if (etdev->RegistryFlowControl == Both)
				etdev->FlowControl = Both;
			else
				etdev->FlowControl = None;
		} else if ((RemotePause == TRUEPHY_BIT_CLEAR) &&
			   (RemoteAsyncPause == TRUEPHY_BIT_CLEAR)) {
			etdev->FlowControl = None;
		} else {
			if (etdev->RegistryFlowControl == Both)
				etdev->FlowControl = RxOnly;
			else
				etdev->FlowControl = None;
		}
	}
}


void UpdateMacStatHostCounters(struct et131x_adapter *etdev)
{
	struct _ce_stats_t *stats = &etdev->Stats;
	struct _MAC_STAT_t __iomem *pDevMacStat =
		&etdev->regs->macStat;

	stats->collisions += readl(&pDevMacStat->TNcl);
	stats->first_collision += readl(&pDevMacStat->TScl);
	stats->tx_deferred += readl(&pDevMacStat->TDfr);
	stats->excessive_collisions += readl(&pDevMacStat->TMcl);
	stats->late_collisions += readl(&pDevMacStat->TLcl);
	stats->tx_uflo += readl(&pDevMacStat->TUnd);
	stats->max_pkt_error += readl(&pDevMacStat->TOvr);

	stats->alignment_err += readl(&pDevMacStat->RAln);
	stats->crc_err += readl(&pDevMacStat->RCde);
	stats->norcvbuf += readl(&pDevMacStat->RDrp);
	stats->rx_ov_flow += readl(&pDevMacStat->ROvr);
	stats->code_violations += readl(&pDevMacStat->RFcs);
	stats->length_err += readl(&pDevMacStat->RFlr);

	stats->other_errors += readl(&pDevMacStat->RFrg);
}


void HandleMacStatInterrupt(struct et131x_adapter *etdev)
{
	MAC_STAT_REG_1_t Carry1;
	MAC_STAT_REG_2_t Carry2;

	
	Carry1.value = readl(&etdev->regs->macStat.Carry1.value);
	Carry2.value = readl(&etdev->regs->macStat.Carry2.value);

	writel(Carry1.value, &etdev->regs->macStat.Carry1.value);
	writel(Carry2.value, &etdev->regs->macStat.Carry2.value);

	
	if (Carry1.bits.rfcs)
		etdev->Stats.code_violations += COUNTER_WRAP_16_BIT;
	if (Carry1.bits.raln)
		etdev->Stats.alignment_err += COUNTER_WRAP_12_BIT;
	if (Carry1.bits.rflr)
		etdev->Stats.length_err += COUNTER_WRAP_16_BIT;
	if (Carry1.bits.rfrg)
		etdev->Stats.other_errors += COUNTER_WRAP_16_BIT;
	if (Carry1.bits.rcde)
		etdev->Stats.crc_err += COUNTER_WRAP_16_BIT;
	if (Carry1.bits.rovr)
		etdev->Stats.rx_ov_flow += COUNTER_WRAP_16_BIT;
	if (Carry1.bits.rdrp)
		etdev->Stats.norcvbuf += COUNTER_WRAP_16_BIT;
	if (Carry2.bits.tovr)
		etdev->Stats.max_pkt_error += COUNTER_WRAP_12_BIT;
	if (Carry2.bits.tund)
		etdev->Stats.tx_uflo += COUNTER_WRAP_12_BIT;
	if (Carry2.bits.tscl)
		etdev->Stats.first_collision += COUNTER_WRAP_12_BIT;
	if (Carry2.bits.tdfr)
		etdev->Stats.tx_deferred += COUNTER_WRAP_12_BIT;
	if (Carry2.bits.tmcl)
		etdev->Stats.excessive_collisions += COUNTER_WRAP_12_BIT;
	if (Carry2.bits.tlcl)
		etdev->Stats.late_collisions += COUNTER_WRAP_12_BIT;
	if (Carry2.bits.tncl)
		etdev->Stats.collisions += COUNTER_WRAP_12_BIT;
}

void SetupDeviceForMulticast(struct et131x_adapter *etdev)
{
	struct _RXMAC_t __iomem *rxmac = &etdev->regs->rxmac;
	uint32_t nIndex;
	uint32_t result;
	uint32_t hash1 = 0;
	uint32_t hash2 = 0;
	uint32_t hash3 = 0;
	uint32_t hash4 = 0;
	u32 pm_csr;

	
	if (etdev->PacketFilter & ET131X_PACKET_TYPE_MULTICAST) {
		
		for (nIndex = 0; nIndex < etdev->MCAddressCount; nIndex++) {
			result = ether_crc(6, etdev->MCList[nIndex]);

			result = (result & 0x3F800000) >> 23;

			if (result < 32) {
				hash1 |= (1 << result);
			} else if ((31 < result) && (result < 64)) {
				result -= 32;
				hash2 |= (1 << result);
			} else if ((63 < result) && (result < 96)) {
				result -= 64;
				hash3 |= (1 << result);
			} else {
				result -= 96;
				hash4 |= (1 << result);
			}
		}
	}

	
	pm_csr = readl(&etdev->regs->global.pm_csr);
	if ((pm_csr & ET_PM_PHY_SW_COMA) == 0) {
		writel(hash1, &rxmac->multi_hash1);
		writel(hash2, &rxmac->multi_hash2);
		writel(hash3, &rxmac->multi_hash3);
		writel(hash4, &rxmac->multi_hash4);
	}
}

void SetupDeviceForUnicast(struct et131x_adapter *etdev)
{
	struct _RXMAC_t __iomem *rxmac = &etdev->regs->rxmac;
	RXMAC_UNI_PF_ADDR1_t uni_pf1;
	RXMAC_UNI_PF_ADDR2_t uni_pf2;
	RXMAC_UNI_PF_ADDR3_t uni_pf3;
	u32 pm_csr;

	
	uni_pf3.bits.addr1_1 = etdev->CurrentAddress[0];
	uni_pf3.bits.addr1_2 = etdev->CurrentAddress[1];
	uni_pf3.bits.addr2_1 = etdev->CurrentAddress[0];
	uni_pf3.bits.addr2_2 = etdev->CurrentAddress[1];

	uni_pf2.bits.addr2_3 = etdev->CurrentAddress[2];
	uni_pf2.bits.addr2_4 = etdev->CurrentAddress[3];
	uni_pf2.bits.addr2_5 = etdev->CurrentAddress[4];
	uni_pf2.bits.addr2_6 = etdev->CurrentAddress[5];

	uni_pf1.bits.addr1_3 = etdev->CurrentAddress[2];
	uni_pf1.bits.addr1_4 = etdev->CurrentAddress[3];
	uni_pf1.bits.addr1_5 = etdev->CurrentAddress[4];
	uni_pf1.bits.addr1_6 = etdev->CurrentAddress[5];

	pm_csr = readl(&etdev->regs->global.pm_csr);
	if ((pm_csr & ET_PM_PHY_SW_COMA) == 0) {
		writel(uni_pf1.value, &rxmac->uni_pf_addr1.value);
		writel(uni_pf2.value, &rxmac->uni_pf_addr2.value);
		writel(uni_pf3.value, &rxmac->uni_pf_addr3.value);
	}
}
