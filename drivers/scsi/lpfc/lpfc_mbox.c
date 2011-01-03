

#include <linux/blkdev.h>
#include <linux/pci.h>
#include <linux/interrupt.h>

#include <scsi/scsi_device.h>
#include <scsi/scsi_transport_fc.h>

#include <scsi/scsi.h>

#include "lpfc_hw4.h"
#include "lpfc_hw.h"
#include "lpfc_sli.h"
#include "lpfc_sli4.h"
#include "lpfc_nl.h"
#include "lpfc_disc.h"
#include "lpfc_scsi.h"
#include "lpfc.h"
#include "lpfc_logmsg.h"
#include "lpfc_crtn.h"
#include "lpfc_compat.h"


int
lpfc_dump_static_vport(struct lpfc_hba *phba, LPFC_MBOXQ_t *pmb,
		uint16_t offset)
{
	MAILBOX_t *mb;
	struct lpfc_dmabuf *mp;

	mb = &pmb->u.mb;

	
	memset(pmb, 0, sizeof(LPFC_MBOXQ_t));
	mb->mbxCommand = MBX_DUMP_MEMORY;
	mb->un.varDmp.type = DMP_NV_PARAMS;
	mb->un.varDmp.entry_index = offset;
	mb->un.varDmp.region_id = DMP_REGION_VPORT;
	mb->mbxOwner = OWN_HOST;

	
	if (phba->sli_rev != LPFC_SLI_REV4) {
		mb->un.varDmp.cv = 1;
		mb->un.varDmp.word_cnt = DMP_RSP_SIZE/sizeof(uint32_t);
		return 0;
	}

	
	mp = kmalloc(sizeof(struct lpfc_dmabuf), GFP_KERNEL);
	if (mp)
		mp->virt = lpfc_mbuf_alloc(phba, 0, &mp->phys);

	if (!mp || !mp->virt) {
		kfree(mp);
		lpfc_printf_log(phba, KERN_ERR, LOG_MBOX,
			"2605 lpfc_dump_static_vport: memory"
			" allocation failed\n");
		return 1;
	}
	memset(mp->virt, 0, LPFC_BPL_SIZE);
	INIT_LIST_HEAD(&mp->list);
	
	pmb->context2 = (uint8_t *) mp;
	mb->un.varWords[3] = putPaddrLow(mp->phys);
	mb->un.varWords[4] = putPaddrHigh(mp->phys);
	mb->un.varDmp.sli4_length = sizeof(struct static_vport_info);

	return 0;
}


void
lpfc_down_link(struct lpfc_hba *phba, LPFC_MBOXQ_t *pmb)
{
	MAILBOX_t *mb;
	memset(pmb, 0, sizeof(LPFC_MBOXQ_t));
	mb = &pmb->u.mb;
	mb->mbxCommand = MBX_DOWN_LINK;
	mb->mbxOwner = OWN_HOST;
}


void
lpfc_dump_mem(struct lpfc_hba *phba, LPFC_MBOXQ_t *pmb, uint16_t offset,
		uint16_t region_id)
{
	MAILBOX_t *mb;
	void *ctx;

	mb = &pmb->u.mb;
	ctx = pmb->context2;

	
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));
	mb->mbxCommand = MBX_DUMP_MEMORY;
	mb->un.varDmp.cv = 1;
	mb->un.varDmp.type = DMP_NV_PARAMS;
	mb->un.varDmp.entry_index = offset;
	mb->un.varDmp.region_id = region_id;
	mb->un.varDmp.word_cnt = (DMP_RSP_SIZE / sizeof (uint32_t));
	mb->un.varDmp.co = 0;
	mb->un.varDmp.resp_offset = 0;
	pmb->context2 = ctx;
	mb->mbxOwner = OWN_HOST;
	return;
}


void
lpfc_dump_wakeup_param(struct lpfc_hba *phba, LPFC_MBOXQ_t *pmb)
{
	MAILBOX_t *mb;
	void *ctx;

	mb = &pmb->u.mb;
	
	ctx = pmb->context2;

	
	memset(pmb, 0, sizeof(LPFC_MBOXQ_t));
	mb->mbxCommand = MBX_DUMP_MEMORY;
	mb->mbxOwner = OWN_HOST;
	mb->un.varDmp.cv = 1;
	mb->un.varDmp.type = DMP_NV_PARAMS;
	mb->un.varDmp.entry_index = 0;
	mb->un.varDmp.region_id = WAKE_UP_PARMS_REGION_ID;
	mb->un.varDmp.word_cnt = WAKE_UP_PARMS_WORD_SIZE;
	mb->un.varDmp.co = 0;
	mb->un.varDmp.resp_offset = 0;
	pmb->context2 = ctx;
	return;
}


void
lpfc_read_nv(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb)
{
	MAILBOX_t *mb;

	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));
	mb->mbxCommand = MBX_READ_NV;
	mb->mbxOwner = OWN_HOST;
	return;
}


void
lpfc_config_async(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb,
		uint32_t ring)
{
	MAILBOX_t *mb;

	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));
	mb->mbxCommand = MBX_ASYNCEVT_ENABLE;
	mb->un.varCfgAsyncEvent.ring = ring;
	mb->mbxOwner = OWN_HOST;
	return;
}


void
lpfc_heart_beat(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb)
{
	MAILBOX_t *mb;

	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));
	mb->mbxCommand = MBX_HEARTBEAT;
	mb->mbxOwner = OWN_HOST;
	return;
}


int
lpfc_read_la(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb, struct lpfc_dmabuf *mp)
{
	MAILBOX_t *mb;
	struct lpfc_sli *psli;

	psli = &phba->sli;
	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	INIT_LIST_HEAD(&mp->list);
	mb->mbxCommand = MBX_READ_LA64;
	mb->un.varReadLA.un.lilpBde64.tus.f.bdeSize = 128;
	mb->un.varReadLA.un.lilpBde64.addrHigh = putPaddrHigh(mp->phys);
	mb->un.varReadLA.un.lilpBde64.addrLow = putPaddrLow(mp->phys);

	
	pmb->context1 = (uint8_t *) mp;
	mb->mbxOwner = OWN_HOST;
	return (0);
}


void
lpfc_clear_la(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb)
{
	MAILBOX_t *mb;

	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->un.varClearLA.eventTag = phba->fc_eventTag;
	mb->mbxCommand = MBX_CLEAR_LA;
	mb->mbxOwner = OWN_HOST;
	return;
}


void
lpfc_config_link(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb)
{
	struct lpfc_vport  *vport = phba->pport;
	MAILBOX_t *mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	
	if (phba->cfg_cr_delay) {
		mb->un.varCfgLnk.cr = 1;
		mb->un.varCfgLnk.ci = 1;
		mb->un.varCfgLnk.cr_delay = phba->cfg_cr_delay;
		mb->un.varCfgLnk.cr_count = phba->cfg_cr_count;
	}

	mb->un.varCfgLnk.myId = vport->fc_myDID;
	mb->un.varCfgLnk.edtov = phba->fc_edtov;
	mb->un.varCfgLnk.arbtov = phba->fc_arbtov;
	mb->un.varCfgLnk.ratov = phba->fc_ratov;
	mb->un.varCfgLnk.rttov = phba->fc_rttov;
	mb->un.varCfgLnk.altov = phba->fc_altov;
	mb->un.varCfgLnk.crtov = phba->fc_crtov;
	mb->un.varCfgLnk.citov = phba->fc_citov;

	if (phba->cfg_ack0)
		mb->un.varCfgLnk.ack0_enable = 1;

	mb->mbxCommand = MBX_CONFIG_LINK;
	mb->mbxOwner = OWN_HOST;
	return;
}


int
lpfc_config_msi(struct lpfc_hba *phba, LPFC_MBOXQ_t *pmb)
{
	MAILBOX_t *mb = &pmb->u.mb;
	uint32_t attentionConditions[2];

	
	if (phba->cfg_use_msi != 2) {
		lpfc_printf_log(phba, KERN_ERR, LOG_INIT,
				"0475 Not configured for supporting MSI-X "
				"cfg_use_msi: 0x%x\n", phba->cfg_use_msi);
		return -EINVAL;
	}

	if (phba->sli_rev < 3) {
		lpfc_printf_log(phba, KERN_ERR, LOG_INIT,
				"0476 HBA not supporting SLI-3 or later "
				"SLI Revision: 0x%x\n", phba->sli_rev);
		return -EINVAL;
	}

	
	memset(pmb, 0, sizeof(LPFC_MBOXQ_t));

	

	
	attentionConditions[0] = (HA_R0ATT | HA_R1ATT | HA_R2ATT | HA_ERATT |
				  HA_LATT | HA_MBATT);
	attentionConditions[1] = 0;

	mb->un.varCfgMSI.attentionConditions[0] = attentionConditions[0];
	mb->un.varCfgMSI.attentionConditions[1] = attentionConditions[1];

	
#ifdef __BIG_ENDIAN_BITFIELD
	
	mb->un.varCfgMSI.messageNumberByHA[HA_R0_POS] = 1;
	
	mb->un.varCfgMSI.messageNumberByHA[HA_R1_POS] = 1;
#else   
	
	mb->un.varCfgMSI.messageNumberByHA[HA_R0_POS^3] = 1;
	
	mb->un.varCfgMSI.messageNumberByHA[HA_R1_POS^3] = 1;
#endif
	
	mb->un.varCfgMSI.autoClearHA[0] = attentionConditions[0];
	mb->un.varCfgMSI.autoClearHA[1] = attentionConditions[1];

	
	mb->un.varCfgMSI.autoClearHA[0] = 0;
	mb->un.varCfgMSI.autoClearHA[1] = 0;

	
	mb->mbxCommand = MBX_CONFIG_MSI;
	mb->mbxOwner = OWN_HOST;

	return 0;
}


void
lpfc_init_link(struct lpfc_hba * phba,
	       LPFC_MBOXQ_t * pmb, uint32_t topology, uint32_t linkspeed)
{
	lpfc_vpd_t *vpd;
	struct lpfc_sli *psli;
	MAILBOX_t *mb;

	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	psli = &phba->sli;
	switch (topology) {
	case FLAGS_TOPOLOGY_MODE_LOOP_PT:
		mb->un.varInitLnk.link_flags = FLAGS_TOPOLOGY_MODE_LOOP;
		mb->un.varInitLnk.link_flags |= FLAGS_TOPOLOGY_FAILOVER;
		break;
	case FLAGS_TOPOLOGY_MODE_PT_PT:
		mb->un.varInitLnk.link_flags = FLAGS_TOPOLOGY_MODE_PT_PT;
		break;
	case FLAGS_TOPOLOGY_MODE_LOOP:
		mb->un.varInitLnk.link_flags = FLAGS_TOPOLOGY_MODE_LOOP;
		break;
	case FLAGS_TOPOLOGY_MODE_PT_LOOP:
		mb->un.varInitLnk.link_flags = FLAGS_TOPOLOGY_MODE_PT_PT;
		mb->un.varInitLnk.link_flags |= FLAGS_TOPOLOGY_FAILOVER;
		break;
	case FLAGS_LOCAL_LB:
		mb->un.varInitLnk.link_flags = FLAGS_LOCAL_LB;
		break;
	}

	
	mb->un.varInitLnk.link_flags |= FLAGS_IMED_ABORT;

	
	vpd = &phba->vpd;
	if (vpd->rev.feaLevelHigh >= 0x02){
		switch(linkspeed){
			case LINK_SPEED_1G:
			case LINK_SPEED_2G:
			case LINK_SPEED_4G:
			case LINK_SPEED_8G:
				mb->un.varInitLnk.link_flags |=
							FLAGS_LINK_SPEED;
				mb->un.varInitLnk.link_speed = linkspeed;
			break;
			case LINK_SPEED_AUTO:
			default:
				mb->un.varInitLnk.link_speed =
							LINK_SPEED_AUTO;
			break;
		}

	}
	else
		mb->un.varInitLnk.link_speed = LINK_SPEED_AUTO;

	mb->mbxCommand = (volatile uint8_t)MBX_INIT_LINK;
	mb->mbxOwner = OWN_HOST;
	mb->un.varInitLnk.fabric_AL_PA = phba->fc_pref_ALPA;
	return;
}


int
lpfc_read_sparam(struct lpfc_hba *phba, LPFC_MBOXQ_t *pmb, int vpi)
{
	struct lpfc_dmabuf *mp;
	MAILBOX_t *mb;
	struct lpfc_sli *psli;

	psli = &phba->sli;
	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->mbxOwner = OWN_HOST;

	

	mp = kmalloc(sizeof (struct lpfc_dmabuf), GFP_KERNEL);
	if (mp)
		mp->virt = lpfc_mbuf_alloc(phba, 0, &mp->phys);
	if (!mp || !mp->virt) {
		kfree(mp);
		mb->mbxCommand = MBX_READ_SPARM64;
		
		lpfc_printf_log(phba, KERN_WARNING, LOG_MBOX,
			        "0301 READ_SPARAM: no buffers\n");
		return (1);
	}
	INIT_LIST_HEAD(&mp->list);
	mb->mbxCommand = MBX_READ_SPARM64;
	mb->un.varRdSparm.un.sp64.tus.f.bdeSize = sizeof (struct serv_parm);
	mb->un.varRdSparm.un.sp64.addrHigh = putPaddrHigh(mp->phys);
	mb->un.varRdSparm.un.sp64.addrLow = putPaddrLow(mp->phys);
	mb->un.varRdSparm.vpi = vpi + phba->vpi_base;

	
	pmb->context1 = mp;

	return (0);
}


void
lpfc_unreg_did(struct lpfc_hba * phba, uint16_t vpi, uint32_t did,
	       LPFC_MBOXQ_t * pmb)
{
	MAILBOX_t *mb;

	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->un.varUnregDID.did = did;
	if (vpi != 0xffff)
		vpi += phba->vpi_base;
	mb->un.varUnregDID.vpi = vpi;

	mb->mbxCommand = MBX_UNREG_D_ID;
	mb->mbxOwner = OWN_HOST;
	return;
}


void
lpfc_read_config(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb)
{
	MAILBOX_t *mb;

	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->mbxCommand = MBX_READ_CONFIG;
	mb->mbxOwner = OWN_HOST;
	return;
}


void
lpfc_read_lnk_stat(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb)
{
	MAILBOX_t *mb;

	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->mbxCommand = MBX_READ_LNK_STAT;
	mb->mbxOwner = OWN_HOST;
	return;
}


int
lpfc_reg_rpi(struct lpfc_hba *phba, uint16_t vpi, uint32_t did,
	       uint8_t *param, LPFC_MBOXQ_t *pmb, uint32_t flag)
{
	MAILBOX_t *mb = &pmb->u.mb;
	uint8_t *sparam;
	struct lpfc_dmabuf *mp;

	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->un.varRegLogin.rpi = 0;
	if (phba->sli_rev == LPFC_SLI_REV4) {
		mb->un.varRegLogin.rpi = lpfc_sli4_alloc_rpi(phba);
		if (mb->un.varRegLogin.rpi == LPFC_RPI_ALLOC_ERROR)
			return 1;
	}

	mb->un.varRegLogin.vpi = vpi + phba->vpi_base;
	mb->un.varRegLogin.did = did;
	mb->un.varWords[30] = flag;	

	mb->mbxOwner = OWN_HOST;

	
	mp = kmalloc(sizeof (struct lpfc_dmabuf), GFP_KERNEL);
	if (mp)
		mp->virt = lpfc_mbuf_alloc(phba, 0, &mp->phys);
	if (!mp || !mp->virt) {
		kfree(mp);
		mb->mbxCommand = MBX_REG_LOGIN64;
		
		lpfc_printf_log(phba, KERN_WARNING, LOG_MBOX,
				"0302 REG_LOGIN: no buffers, VPI:%d DID:x%x, "
				"flag x%x\n", vpi, did, flag);
		return (1);
	}
	INIT_LIST_HEAD(&mp->list);
	sparam = mp->virt;

	
	memcpy(sparam, param, sizeof (struct serv_parm));

	
	pmb->context1 = (uint8_t *) mp;

	mb->mbxCommand = MBX_REG_LOGIN64;
	mb->un.varRegLogin.un.sp64.tus.f.bdeSize = sizeof (struct serv_parm);
	mb->un.varRegLogin.un.sp64.addrHigh = putPaddrHigh(mp->phys);
	mb->un.varRegLogin.un.sp64.addrLow = putPaddrLow(mp->phys);

	return (0);
}


void
lpfc_unreg_login(struct lpfc_hba *phba, uint16_t vpi, uint32_t rpi,
		 LPFC_MBOXQ_t * pmb)
{
	MAILBOX_t *mb;

	mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->un.varUnregLogin.rpi = (uint16_t) rpi;
	mb->un.varUnregLogin.rsvd1 = 0;
	mb->un.varUnregLogin.vpi = vpi + phba->vpi_base;

	mb->mbxCommand = MBX_UNREG_LOGIN;
	mb->mbxOwner = OWN_HOST;

	return;
}


void
lpfc_reg_vpi(struct lpfc_vport *vport, LPFC_MBOXQ_t *pmb)
{
	MAILBOX_t *mb = &pmb->u.mb;

	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->un.varRegVpi.vpi = vport->vpi + vport->phba->vpi_base;
	mb->un.varRegVpi.sid = vport->fc_myDID;
	mb->un.varRegVpi.vfi = vport->vfi + vport->phba->vfi_base;

	mb->mbxCommand = MBX_REG_VPI;
	mb->mbxOwner = OWN_HOST;
	return;

}


void
lpfc_unreg_vpi(struct lpfc_hba *phba, uint16_t vpi, LPFC_MBOXQ_t *pmb)
{
	MAILBOX_t *mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->un.varUnregVpi.vpi = vpi + phba->vpi_base;

	mb->mbxCommand = MBX_UNREG_VPI;
	mb->mbxOwner = OWN_HOST;
	return;

}


static void
lpfc_config_pcb_setup(struct lpfc_hba * phba)
{
	struct lpfc_sli *psli = &phba->sli;
	struct lpfc_sli_ring *pring;
	PCB_t *pcbp = phba->pcb;
	dma_addr_t pdma_addr;
	uint32_t offset;
	uint32_t iocbCnt = 0;
	int i;

	pcbp->maxRing = (psli->num_rings - 1);

	for (i = 0; i < psli->num_rings; i++) {
		pring = &psli->ring[i];

		pring->sizeCiocb = phba->sli_rev == 3 ? SLI3_IOCB_CMD_SIZE:
							SLI2_IOCB_CMD_SIZE;
		pring->sizeRiocb = phba->sli_rev == 3 ? SLI3_IOCB_RSP_SIZE:
							SLI2_IOCB_RSP_SIZE;
		
		if ((pring->numCiocb == 0) || (pring->numRiocb == 0)) {
			pcbp->rdsc[i].cmdEntries = 0;
			pcbp->rdsc[i].rspEntries = 0;
			pcbp->rdsc[i].cmdAddrHigh = 0;
			pcbp->rdsc[i].rspAddrHigh = 0;
			pcbp->rdsc[i].cmdAddrLow = 0;
			pcbp->rdsc[i].rspAddrLow = 0;
			pring->cmdringaddr = NULL;
			pring->rspringaddr = NULL;
			continue;
		}
		
		pring->cmdringaddr = (void *)&phba->IOCBs[iocbCnt];
		pcbp->rdsc[i].cmdEntries = pring->numCiocb;

		offset = (uint8_t *) &phba->IOCBs[iocbCnt] -
			 (uint8_t *) phba->slim2p.virt;
		pdma_addr = phba->slim2p.phys + offset;
		pcbp->rdsc[i].cmdAddrHigh = putPaddrHigh(pdma_addr);
		pcbp->rdsc[i].cmdAddrLow = putPaddrLow(pdma_addr);
		iocbCnt += pring->numCiocb;

		
		pring->rspringaddr = (void *) &phba->IOCBs[iocbCnt];

		pcbp->rdsc[i].rspEntries = pring->numRiocb;
		offset = (uint8_t *)&phba->IOCBs[iocbCnt] -
			 (uint8_t *)phba->slim2p.virt;
		pdma_addr = phba->slim2p.phys + offset;
		pcbp->rdsc[i].rspAddrHigh = putPaddrHigh(pdma_addr);
		pcbp->rdsc[i].rspAddrLow = putPaddrLow(pdma_addr);
		iocbCnt += pring->numRiocb;
	}
}


void
lpfc_read_rev(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb)
{
	MAILBOX_t *mb = &pmb->u.mb;
	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));
	mb->un.varRdRev.cv = 1;
	mb->un.varRdRev.v3req = 1; 
	mb->mbxCommand = MBX_READ_REV;
	mb->mbxOwner = OWN_HOST;
	return;
}


static void
lpfc_build_hbq_profile2(struct config_hbq_var *hbqmb,
			struct lpfc_hbq_init  *hbq_desc)
{
	hbqmb->profiles.profile2.seqlenbcnt = hbq_desc->seqlenbcnt;
	hbqmb->profiles.profile2.maxlen     = hbq_desc->maxlen;
	hbqmb->profiles.profile2.seqlenoff  = hbq_desc->seqlenoff;
}


static void
lpfc_build_hbq_profile3(struct config_hbq_var *hbqmb,
			struct lpfc_hbq_init  *hbq_desc)
{
	hbqmb->profiles.profile3.seqlenbcnt = hbq_desc->seqlenbcnt;
	hbqmb->profiles.profile3.maxlen     = hbq_desc->maxlen;
	hbqmb->profiles.profile3.cmdcodeoff = hbq_desc->cmdcodeoff;
	hbqmb->profiles.profile3.seqlenoff  = hbq_desc->seqlenoff;
	memcpy(&hbqmb->profiles.profile3.cmdmatch, hbq_desc->cmdmatch,
	       sizeof(hbqmb->profiles.profile3.cmdmatch));
}


static void
lpfc_build_hbq_profile5(struct config_hbq_var *hbqmb,
			struct lpfc_hbq_init  *hbq_desc)
{
	hbqmb->profiles.profile5.seqlenbcnt = hbq_desc->seqlenbcnt;
	hbqmb->profiles.profile5.maxlen     = hbq_desc->maxlen;
	hbqmb->profiles.profile5.cmdcodeoff = hbq_desc->cmdcodeoff;
	hbqmb->profiles.profile5.seqlenoff  = hbq_desc->seqlenoff;
	memcpy(&hbqmb->profiles.profile5.cmdmatch, hbq_desc->cmdmatch,
	       sizeof(hbqmb->profiles.profile5.cmdmatch));
}


void
lpfc_config_hbq(struct lpfc_hba *phba, uint32_t id,
		 struct lpfc_hbq_init *hbq_desc,
		uint32_t hbq_entry_index, LPFC_MBOXQ_t *pmb)
{
	int i;
	MAILBOX_t *mb = &pmb->u.mb;
	struct config_hbq_var *hbqmb = &mb->un.varCfgHbq;

	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));
	hbqmb->hbqId = id;
	hbqmb->entry_count = hbq_desc->entry_count;   
	hbqmb->recvNotify = hbq_desc->rn;             
	hbqmb->numMask    = hbq_desc->mask_count;     
	hbqmb->profile    = hbq_desc->profile;	      
	hbqmb->ringMask   = hbq_desc->ring_mask;      
	hbqmb->headerLen  = hbq_desc->headerLen;      
	hbqmb->logEntry   = hbq_desc->logEntry;       
	hbqmb->hbqaddrLow = putPaddrLow(phba->hbqslimp.phys) +
		hbq_entry_index * sizeof(struct lpfc_hbq_entry);
	hbqmb->hbqaddrHigh = putPaddrHigh(phba->hbqslimp.phys);

	mb->mbxCommand = MBX_CONFIG_HBQ;
	mb->mbxOwner = OWN_HOST;

				
	if (hbq_desc->profile == 2)
		lpfc_build_hbq_profile2(hbqmb, hbq_desc);
	else if (hbq_desc->profile == 3)
		lpfc_build_hbq_profile3(hbqmb, hbq_desc);
	else if (hbq_desc->profile == 5)
		lpfc_build_hbq_profile5(hbqmb, hbq_desc);

	
	if (!hbq_desc->mask_count)
		return;

	
	for (i = 0; i < hbq_desc->mask_count; i++) {
		hbqmb->hbqMasks[i].tmatch = hbq_desc->hbqMasks[i].tmatch;
		hbqmb->hbqMasks[i].tmask  = hbq_desc->hbqMasks[i].tmask;
		hbqmb->hbqMasks[i].rctlmatch = hbq_desc->hbqMasks[i].rctlmatch;
		hbqmb->hbqMasks[i].rctlmask  = hbq_desc->hbqMasks[i].rctlmask;
	}

	return;
}


void
lpfc_config_ring(struct lpfc_hba * phba, int ring, LPFC_MBOXQ_t * pmb)
{
	int i;
	MAILBOX_t *mb = &pmb->u.mb;
	struct lpfc_sli *psli;
	struct lpfc_sli_ring *pring;

	memset(pmb, 0, sizeof (LPFC_MBOXQ_t));

	mb->un.varCfgRing.ring = ring;
	mb->un.varCfgRing.maxOrigXchg = 0;
	mb->un.varCfgRing.maxRespXchg = 0;
	mb->un.varCfgRing.recvNotify = 1;

	psli = &phba->sli;
	pring = &psli->ring[ring];
	mb->un.varCfgRing.numMask = pring->num_mask;
	mb->mbxCommand = MBX_CONFIG_RING;
	mb->mbxOwner = OWN_HOST;

	
	if (pring->prt[0].profile) {
		mb->un.varCfgRing.profile = pring->prt[0].profile;
		return;
	}

	
	for (i = 0; i < pring->num_mask; i++) {
		mb->un.varCfgRing.rrRegs[i].rval = pring->prt[i].rctl;
		if (mb->un.varCfgRing.rrRegs[i].rval != FC_ELS_REQ)
			mb->un.varCfgRing.rrRegs[i].rmask = 0xff;
		else
			mb->un.varCfgRing.rrRegs[i].rmask = 0xfe;
		mb->un.varCfgRing.rrRegs[i].tval = pring->prt[i].type;
		mb->un.varCfgRing.rrRegs[i].tmask = 0xff;
	}

	return;
}


void
lpfc_config_port(struct lpfc_hba *phba, LPFC_MBOXQ_t *pmb)
{
	MAILBOX_t __iomem *mb_slim = (MAILBOX_t __iomem *) phba->MBslimaddr;
	MAILBOX_t *mb = &pmb->u.mb;
	dma_addr_t pdma_addr;
	uint32_t bar_low, bar_high;
	size_t offset;
	struct lpfc_hgp hgp;
	int i;
	uint32_t pgp_offset;

	memset(pmb, 0, sizeof(LPFC_MBOXQ_t));
	mb->mbxCommand = MBX_CONFIG_PORT;
	mb->mbxOwner = OWN_HOST;

	mb->un.varCfgPort.pcbLen = sizeof(PCB_t);

	offset = (uint8_t *)phba->pcb - (uint8_t *)phba->slim2p.virt;
	pdma_addr = phba->slim2p.phys + offset;
	mb->un.varCfgPort.pcbLow = putPaddrLow(pdma_addr);
	mb->un.varCfgPort.pcbHigh = putPaddrHigh(pdma_addr);

	
	mb->un.varCfgPort.hps = 1;

	

	if (phba->sli_rev == LPFC_SLI_REV3 && phba->vpd.sli3Feat.cerbm) {
		if (phba->cfg_enable_bg)
			mb->un.varCfgPort.cbg = 1; 
		mb->un.varCfgPort.cdss = 1; 
		mb->un.varCfgPort.cerbm = 1; 
		mb->un.varCfgPort.ccrp = 1; 
		mb->un.varCfgPort.cinb = 1; 
		mb->un.varCfgPort.max_hbq = lpfc_sli_hbq_count();
		if (phba->max_vpi && phba->cfg_enable_npiv &&
		    phba->vpd.sli3Feat.cmv) {
			mb->un.varCfgPort.max_vpi = LPFC_MAX_VPI;
			mb->un.varCfgPort.cmv = 1;
		} else
			mb->un.varCfgPort.max_vpi = phba->max_vpi = 0;
	} else
		phba->sli_rev = LPFC_SLI_REV2;
	mb->un.varCfgPort.sli_mode = phba->sli_rev;

	
	phba->pcb->type = TYPE_NATIVE_SLI2;
	phba->pcb->feature = FEATURE_INITIAL_SLI2;

	
	phba->pcb->mailBoxSize = sizeof(MAILBOX_t);
	offset = (uint8_t *)phba->mbox - (uint8_t *)phba->slim2p.virt;
	pdma_addr = phba->slim2p.phys + offset;
	phba->pcb->mbAddrHigh = putPaddrHigh(pdma_addr);
	phba->pcb->mbAddrLow = putPaddrLow(pdma_addr);

	


	pci_read_config_dword(phba->pcidev, PCI_BASE_ADDRESS_0, &bar_low);
	pci_read_config_dword(phba->pcidev, PCI_BASE_ADDRESS_1, &bar_high);

	

	if (phba->sli_rev == 3) {
		phba->host_gp = &mb_slim->us.s3.host[0];
		phba->hbq_put = &mb_slim->us.s3.hbq_put[0];
	} else {
		phba->host_gp = &mb_slim->us.s2.host[0];
		phba->hbq_put = NULL;
	}

	
	phba->pcb->hgpAddrLow = (bar_low & PCI_BASE_ADDRESS_MEM_MASK) +
		(void __iomem *)phba->host_gp -
		(void __iomem *)phba->MBslimaddr;
	if (bar_low & PCI_BASE_ADDRESS_MEM_TYPE_64)
		phba->pcb->hgpAddrHigh = bar_high;
	else
		phba->pcb->hgpAddrHigh = 0;
	
	memset(&hgp, 0, sizeof(struct lpfc_hgp));

	for (i=0; i < phba->sli.num_rings; i++) {
		lpfc_memcpy_to_slim(phba->host_gp + i, &hgp,
				    sizeof(*phba->host_gp));
	}

	
	if (phba->sli_rev == 3)
		pgp_offset = offsetof(struct lpfc_sli2_slim,
				      mbx.us.s3_pgp.port);
	else
		pgp_offset = offsetof(struct lpfc_sli2_slim, mbx.us.s2.port);
	pdma_addr = phba->slim2p.phys + pgp_offset;
	phba->pcb->pgpAddrHigh = putPaddrHigh(pdma_addr);
	phba->pcb->pgpAddrLow = putPaddrLow(pdma_addr);

	
	lpfc_config_pcb_setup(phba);

	
	if (lpfc_is_LC_HBA(phba->pcidev->device)) {
		uint32_t hbainit[5];

		lpfc_hba_init(phba, hbainit);

		memcpy(&mb->un.varCfgPort.hbainit, hbainit, 20);
	}

	
	lpfc_sli_pcimem_bcopy(phba->pcb, phba->pcb, sizeof(PCB_t));
}


void
lpfc_kill_board(struct lpfc_hba * phba, LPFC_MBOXQ_t * pmb)
{
	MAILBOX_t *mb = &pmb->u.mb;

	memset(pmb, 0, sizeof(LPFC_MBOXQ_t));
	mb->mbxCommand = MBX_KILL_BOARD;
	mb->mbxOwner = OWN_HOST;
	return;
}


void
lpfc_mbox_put(struct lpfc_hba * phba, LPFC_MBOXQ_t * mbq)
{
	struct lpfc_sli *psli;

	psli = &phba->sli;

	list_add_tail(&mbq->list, &psli->mboxq);

	psli->mboxq_cnt++;

	return;
}


LPFC_MBOXQ_t *
lpfc_mbox_get(struct lpfc_hba * phba)
{
	LPFC_MBOXQ_t *mbq = NULL;
	struct lpfc_sli *psli = &phba->sli;

	list_remove_head((&psli->mboxq), mbq, LPFC_MBOXQ_t, list);
	if (mbq)
		psli->mboxq_cnt--;

	return mbq;
}


void
__lpfc_mbox_cmpl_put(struct lpfc_hba *phba, LPFC_MBOXQ_t *mbq)
{
	list_add_tail(&mbq->list, &phba->sli.mboxq_cmpl);
}


void
lpfc_mbox_cmpl_put(struct lpfc_hba *phba, LPFC_MBOXQ_t *mbq)
{
	unsigned long iflag;

	
	spin_lock_irqsave(&phba->hbalock, iflag);
	__lpfc_mbox_cmpl_put(phba, mbq);
	spin_unlock_irqrestore(&phba->hbalock, iflag);
	return;
}


int
lpfc_mbox_cmd_check(struct lpfc_hba *phba, LPFC_MBOXQ_t *mboxq)
{
	
	if (mboxq->mbox_cmpl && mboxq->mbox_cmpl != lpfc_sli_def_mbox_cmpl &&
	    mboxq->mbox_cmpl != lpfc_sli_wake_mbox_wait) {
		if (!mboxq->vport) {
			lpfc_printf_log(phba, KERN_ERR, LOG_MBOX | LOG_VPORT,
					"1814 Mbox x%x failed, no vport\n",
					mboxq->u.mb.mbxCommand);
			dump_stack();
			return -ENODEV;
		}
	}
	return 0;
}


int
lpfc_mbox_dev_check(struct lpfc_hba *phba)
{
	
	if (unlikely(pci_channel_offline(phba->pcidev)))
		return -ENODEV;

	
	if (phba->link_state == LPFC_HBA_ERROR)
		return -ENODEV;

	return 0;
}


int
lpfc_mbox_tmo_val(struct lpfc_hba *phba, int cmd)
{
	switch (cmd) {
	case MBX_WRITE_NV:	
	case MBX_UPDATE_CFG:	
	case MBX_DOWN_LOAD:	
	case MBX_DEL_LD_ENTRY:	
	case MBX_LOAD_AREA:	
	case MBX_WRITE_WWN:     
	case MBX_LOAD_EXP_ROM:	
		return LPFC_MBOX_TMO_FLASH_CMD;
	case MBX_SLI4_CONFIG:	
		return LPFC_MBOX_SLI4_CONFIG_TMO;
	}
	return LPFC_MBOX_TMO;
}


void
lpfc_sli4_mbx_sge_set(struct lpfcMboxq *mbox, uint32_t sgentry,
		      dma_addr_t phyaddr, uint32_t length)
{
	struct lpfc_mbx_nembed_cmd *nembed_sge;

	nembed_sge = (struct lpfc_mbx_nembed_cmd *)
				&mbox->u.mqe.un.nembed_cmd;
	nembed_sge->sge[sgentry].pa_lo = putPaddrLow(phyaddr);
	nembed_sge->sge[sgentry].pa_hi = putPaddrHigh(phyaddr);
	nembed_sge->sge[sgentry].length = length;
}


void
lpfc_sli4_mbx_sge_get(struct lpfcMboxq *mbox, uint32_t sgentry,
		      struct lpfc_mbx_sge *sge)
{
	struct lpfc_mbx_nembed_cmd *nembed_sge;

	nembed_sge = (struct lpfc_mbx_nembed_cmd *)
				&mbox->u.mqe.un.nembed_cmd;
	sge->pa_lo = nembed_sge->sge[sgentry].pa_lo;
	sge->pa_hi = nembed_sge->sge[sgentry].pa_hi;
	sge->length = nembed_sge->sge[sgentry].length;
}


void
lpfc_sli4_mbox_cmd_free(struct lpfc_hba *phba, struct lpfcMboxq *mbox)
{
	struct lpfc_mbx_sli4_config *sli4_cfg;
	struct lpfc_mbx_sge sge;
	dma_addr_t phyaddr;
	uint32_t sgecount, sgentry;

	sli4_cfg = &mbox->u.mqe.un.sli4_config;

	
	if (bf_get(lpfc_mbox_hdr_emb, &sli4_cfg->header.cfg_mhdr)) {
		mempool_free(mbox, phba->mbox_mem_pool);
		return;
	}

	
	sgecount = bf_get(lpfc_mbox_hdr_sge_cnt, &sli4_cfg->header.cfg_mhdr);
	
	if (unlikely(!mbox->sge_array)) {
		mempool_free(mbox, phba->mbox_mem_pool);
		return;
	}
	
	for (sgentry = 0; sgentry < sgecount; sgentry++) {
		lpfc_sli4_mbx_sge_get(mbox, sgentry, &sge);
		phyaddr = getPaddr(sge.pa_hi, sge.pa_lo);
		dma_free_coherent(&phba->pcidev->dev, PAGE_SIZE,
				  mbox->sge_array->addr[sgentry], phyaddr);
	}
	
	kfree(mbox->sge_array);
	
	mempool_free(mbox, phba->mbox_mem_pool);
}


int
lpfc_sli4_config(struct lpfc_hba *phba, struct lpfcMboxq *mbox,
		 uint8_t subsystem, uint8_t opcode, uint32_t length, bool emb)
{
	struct lpfc_mbx_sli4_config *sli4_config;
	union lpfc_sli4_cfg_shdr *cfg_shdr = NULL;
	uint32_t alloc_len;
	uint32_t resid_len;
	uint32_t pagen, pcount;
	void *viraddr;
	dma_addr_t phyaddr;

	
	memset(mbox, 0, sizeof(*mbox));
	bf_set(lpfc_mqe_command, &mbox->u.mqe, MBX_SLI4_CONFIG);

	
	sli4_config = &mbox->u.mqe.un.sli4_config;

	
	if (emb) {
		
		bf_set(lpfc_mbox_hdr_emb, &sli4_config->header.cfg_mhdr, 1);
		sli4_config->header.cfg_mhdr.payload_length =
					LPFC_MBX_CMD_HDR_LENGTH + length;
		
		bf_set(lpfc_mbox_hdr_opcode,
			&sli4_config->header.cfg_shdr.request, opcode);
		bf_set(lpfc_mbox_hdr_subsystem,
			&sli4_config->header.cfg_shdr.request, subsystem);
		sli4_config->header.cfg_shdr.request.request_length = length;
		return length;
	}

	
	pcount = (PAGE_ALIGN(length))/PAGE_SIZE;
	pcount = (pcount > LPFC_SLI4_MBX_SGE_MAX_PAGES) ?
				LPFC_SLI4_MBX_SGE_MAX_PAGES : pcount;
	
	mbox->sge_array = kmalloc(sizeof(struct lpfc_mbx_nembed_sge_virt),
				  GFP_KERNEL);
	if (!mbox->sge_array)
		return 0;

	for (pagen = 0, alloc_len = 0; pagen < pcount; pagen++) {
		
		viraddr = dma_alloc_coherent(&phba->pcidev->dev, PAGE_SIZE,
					     &phyaddr, GFP_KERNEL);
		
		if (!viraddr)
			break;
		memset(viraddr, 0, PAGE_SIZE);
		mbox->sge_array->addr[pagen] = viraddr;
		
		if (pagen == 0)
			cfg_shdr = (union lpfc_sli4_cfg_shdr *)viraddr;
		resid_len = length - alloc_len;
		if (resid_len > PAGE_SIZE) {
			lpfc_sli4_mbx_sge_set(mbox, pagen, phyaddr,
					      PAGE_SIZE);
			alloc_len += PAGE_SIZE;
		} else {
			lpfc_sli4_mbx_sge_set(mbox, pagen, phyaddr,
					      resid_len);
			alloc_len = length;
		}
	}

	
	sli4_config->header.cfg_mhdr.payload_length = alloc_len;
	bf_set(lpfc_mbox_hdr_sge_cnt, &sli4_config->header.cfg_mhdr, pagen);

	
	if (pagen > 0) {
		bf_set(lpfc_mbox_hdr_opcode, &cfg_shdr->request, opcode);
		bf_set(lpfc_mbox_hdr_subsystem, &cfg_shdr->request, subsystem);
		cfg_shdr->request.request_length =
				alloc_len - sizeof(union  lpfc_sli4_cfg_shdr);
	}
	
	lpfc_sli_pcimem_bcopy(cfg_shdr, cfg_shdr,
			      sizeof(union  lpfc_sli4_cfg_shdr));

	return alloc_len;
}


uint8_t
lpfc_sli4_mbox_opcode_get(struct lpfc_hba *phba, struct lpfcMboxq *mbox)
{
	struct lpfc_mbx_sli4_config *sli4_cfg;
	union lpfc_sli4_cfg_shdr *cfg_shdr;

	if (mbox->u.mb.mbxCommand != MBX_SLI4_CONFIG)
		return 0;
	sli4_cfg = &mbox->u.mqe.un.sli4_config;

	
	if (bf_get(lpfc_mbox_hdr_emb, &sli4_cfg->header.cfg_mhdr)) {
		cfg_shdr = &mbox->u.mqe.un.sli4_config.header.cfg_shdr;
		return bf_get(lpfc_mbox_hdr_opcode, &cfg_shdr->request);
	}

	
	if (unlikely(!mbox->sge_array))
		return 0;
	cfg_shdr = (union lpfc_sli4_cfg_shdr *)mbox->sge_array->addr[0];
	return bf_get(lpfc_mbox_hdr_opcode, &cfg_shdr->request);
}


void
lpfc_request_features(struct lpfc_hba *phba, struct lpfcMboxq *mboxq)
{
	
	memset(mboxq, 0, sizeof(LPFC_MBOXQ_t));
	bf_set(lpfc_mqe_command, &mboxq->u.mqe, MBX_SLI4_REQ_FTRS);

	
	bf_set(lpfc_mbx_rq_ftr_rq_fcpi, &mboxq->u.mqe.un.req_ftrs, 1);

	if (phba->cfg_enable_fip)
		bf_set(lpfc_mbx_rq_ftr_rq_ifip, &mboxq->u.mqe.un.req_ftrs, 0);
	else
		bf_set(lpfc_mbx_rq_ftr_rq_ifip, &mboxq->u.mqe.un.req_ftrs, 1);

	
	if (phba->cfg_enable_bg)
		bf_set(lpfc_mbx_rq_ftr_rq_dif, &mboxq->u.mqe.un.req_ftrs, 1);

	
	if (phba->max_vpi && phba->cfg_enable_npiv)
		bf_set(lpfc_mbx_rq_ftr_rq_npiv, &mboxq->u.mqe.un.req_ftrs, 1);

	return;
}


void
lpfc_init_vfi(struct lpfcMboxq *mbox, struct lpfc_vport *vport)
{
	struct lpfc_mbx_init_vfi *init_vfi;

	memset(mbox, 0, sizeof(*mbox));
	init_vfi = &mbox->u.mqe.un.init_vfi;
	bf_set(lpfc_mqe_command, &mbox->u.mqe, MBX_INIT_VFI);
	bf_set(lpfc_init_vfi_vr, init_vfi, 1);
	bf_set(lpfc_init_vfi_vt, init_vfi, 1);
	bf_set(lpfc_init_vfi_vfi, init_vfi, vport->vfi + vport->phba->vfi_base);
	bf_set(lpfc_init_vfi_fcfi, init_vfi, vport->phba->fcf.fcfi);
}


void
lpfc_reg_vfi(struct lpfcMboxq *mbox, struct lpfc_vport *vport, dma_addr_t phys)
{
	struct lpfc_mbx_reg_vfi *reg_vfi;

	memset(mbox, 0, sizeof(*mbox));
	reg_vfi = &mbox->u.mqe.un.reg_vfi;
	bf_set(lpfc_mqe_command, &mbox->u.mqe, MBX_REG_VFI);
	bf_set(lpfc_reg_vfi_vp, reg_vfi, 1);
	bf_set(lpfc_reg_vfi_vfi, reg_vfi, vport->vfi + vport->phba->vfi_base);
	bf_set(lpfc_reg_vfi_fcfi, reg_vfi, vport->phba->fcf.fcfi);
	bf_set(lpfc_reg_vfi_vpi, reg_vfi, vport->vpi + vport->phba->vpi_base);
	reg_vfi->bde.addrHigh = putPaddrHigh(phys);
	reg_vfi->bde.addrLow = putPaddrLow(phys);
	reg_vfi->bde.tus.f.bdeSize = sizeof(vport->fc_sparam);
	reg_vfi->bde.tus.f.bdeFlags = BUFF_TYPE_BDE_64;
	bf_set(lpfc_reg_vfi_nport_id, reg_vfi, vport->fc_myDID);
}


void
lpfc_init_vpi(struct lpfc_hba *phba, struct lpfcMboxq *mbox, uint16_t vpi)
{
	memset(mbox, 0, sizeof(*mbox));
	bf_set(lpfc_mqe_command, &mbox->u.mqe, MBX_INIT_VPI);
	bf_set(lpfc_init_vpi_vpi, &mbox->u.mqe.un.init_vpi,
	       vpi + phba->vpi_base);
	bf_set(lpfc_init_vpi_vfi, &mbox->u.mqe.un.init_vpi,
	       phba->pport->vfi + phba->vfi_base);
}


void
lpfc_unreg_vfi(struct lpfcMboxq *mbox, uint16_t vfi)
{
	memset(mbox, 0, sizeof(*mbox));
	bf_set(lpfc_mqe_command, &mbox->u.mqe, MBX_UNREG_VFI);
	bf_set(lpfc_unreg_vfi_vfi, &mbox->u.mqe.un.unreg_vfi, vfi);
}


int
lpfc_dump_fcoe_param(struct lpfc_hba *phba,
		struct lpfcMboxq *mbox)
{
	struct lpfc_dmabuf *mp = NULL;
	MAILBOX_t *mb;

	memset(mbox, 0, sizeof(*mbox));
	mb = &mbox->u.mb;

	mp = kmalloc(sizeof(struct lpfc_dmabuf), GFP_KERNEL);
	if (mp)
		mp->virt = lpfc_mbuf_alloc(phba, 0, &mp->phys);

	if (!mp || !mp->virt) {
		kfree(mp);
		
		lpfc_printf_log(phba, KERN_WARNING, LOG_MBOX,
			"2569 lpfc_dump_fcoe_param: memory"
			" allocation failed\n");
		return 1;
	}

	memset(mp->virt, 0, LPFC_BPL_SIZE);
	INIT_LIST_HEAD(&mp->list);

	
	mbox->context1 = (uint8_t *) mp;

	mb->mbxCommand = MBX_DUMP_MEMORY;
	mb->un.varDmp.type = DMP_NV_PARAMS;
	mb->un.varDmp.region_id = DMP_REGION_23;
	mb->un.varDmp.sli4_length = DMP_RGN23_SIZE;
	mb->un.varWords[3] = putPaddrLow(mp->phys);
	mb->un.varWords[4] = putPaddrHigh(mp->phys);
	return 0;
}


void
lpfc_reg_fcfi(struct lpfc_hba *phba, struct lpfcMboxq *mbox)
{
	struct lpfc_mbx_reg_fcfi *reg_fcfi;

	memset(mbox, 0, sizeof(*mbox));
	reg_fcfi = &mbox->u.mqe.un.reg_fcfi;
	bf_set(lpfc_mqe_command, &mbox->u.mqe, MBX_REG_FCFI);
	bf_set(lpfc_reg_fcfi_rq_id0, reg_fcfi, phba->sli4_hba.hdr_rq->queue_id);
	bf_set(lpfc_reg_fcfi_rq_id1, reg_fcfi, REG_FCF_INVALID_QID);
	bf_set(lpfc_reg_fcfi_rq_id2, reg_fcfi, REG_FCF_INVALID_QID);
	bf_set(lpfc_reg_fcfi_rq_id3, reg_fcfi, REG_FCF_INVALID_QID);
	bf_set(lpfc_reg_fcfi_info_index, reg_fcfi, phba->fcf.fcf_indx);
	
	bf_set(lpfc_reg_fcfi_mam, reg_fcfi,
		(~phba->fcf.addr_mode) & 0x3);
	if (phba->fcf.fcf_flag & FCF_VALID_VLAN) {
		bf_set(lpfc_reg_fcfi_vv, reg_fcfi, 1);
		bf_set(lpfc_reg_fcfi_vlan_tag, reg_fcfi, phba->fcf.vlan_id);
	}
}


void
lpfc_unreg_fcfi(struct lpfcMboxq *mbox, uint16_t fcfi)
{
	memset(mbox, 0, sizeof(*mbox));
	bf_set(lpfc_mqe_command, &mbox->u.mqe, MBX_UNREG_FCFI);
	bf_set(lpfc_unreg_fcfi, &mbox->u.mqe.un.unreg_fcfi, fcfi);
}


void
lpfc_resume_rpi(struct lpfcMboxq *mbox, struct lpfc_nodelist *ndlp)
{
	struct lpfc_mbx_resume_rpi *resume_rpi;

	memset(mbox, 0, sizeof(*mbox));
	resume_rpi = &mbox->u.mqe.un.resume_rpi;
	bf_set(lpfc_mqe_command, &mbox->u.mqe, MBX_RESUME_RPI);
	bf_set(lpfc_resume_rpi_index, resume_rpi, ndlp->nlp_rpi);
	bf_set(lpfc_resume_rpi_ii, resume_rpi, RESUME_INDEX_RPI);
	resume_rpi->event_tag = ndlp->phba->fc_eventTag;
}
