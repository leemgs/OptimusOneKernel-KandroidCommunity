

#include <defs/bfa_defs_port.h>
#include <cs/bfa_trc.h>
#include <cs/bfa_log.h>
#include <cs/bfa_debug.h>
#include <port/bfa_port.h>
#include <bfi/bfi.h>
#include <bfi/bfi_port.h>
#include <bfa_ioc.h>
#include <cna/bfa_cna_trcmod.h>

BFA_TRC_FILE(CNA, PORT);

#define bfa_ioc_portid(__ioc) ((__ioc)->port_id)
#define bfa_lpuid(__arg) bfa_ioc_portid(&(__arg)->ioc)

static void
bfa_port_stats_swap(struct bfa_port_s *port, union bfa_pport_stats_u *stats)
{
	u32       *dip = (u32 *) stats;
	u32        t0, t1;
	int             i;

	for (i = 0; i < sizeof(union bfa_pport_stats_u) / sizeof(u32);
	     i += 2) {
		t0 = dip[i];
		t1 = dip[i + 1];
#ifdef __BIGENDIAN
		dip[i] = bfa_os_ntohl(t0);
		dip[i + 1] = bfa_os_ntohl(t1);
#else
		dip[i] = bfa_os_ntohl(t1);
		dip[i + 1] = bfa_os_ntohl(t0);
#endif
	}

    
}


static void
bfa_port_enable_isr(struct bfa_port_s *port, bfa_status_t status)
{
	bfa_assert(0);
}


static void
bfa_port_disable_isr(struct bfa_port_s *port, bfa_status_t status)
{
	bfa_assert(0);
}


static void
bfa_port_get_stats_isr(struct bfa_port_s *port, bfa_status_t status)
{
	port->stats_status = status;
	port->stats_busy = BFA_FALSE;

	if (status == BFA_STATUS_OK) {
		memcpy(port->stats, port->stats_dma.kva,
		       sizeof(union bfa_pport_stats_u));
		bfa_port_stats_swap(port, port->stats);
	}

	if (port->stats_cbfn) {
		port->stats_cbfn(port->stats_cbarg, status);
		port->stats_cbfn = NULL;
	}
}


static void
bfa_port_clear_stats_isr(struct bfa_port_s *port, bfa_status_t status)
{
	port->stats_status = status;
	port->stats_busy = BFA_FALSE;

	if (port->stats_cbfn) {
		port->stats_cbfn(port->stats_cbarg, status);
		port->stats_cbfn = NULL;
	}
}


static void
bfa_port_isr(void *cbarg, struct bfi_mbmsg_s *m)
{
	struct bfa_port_s *port = (struct bfa_port_s *)cbarg;
	union bfi_port_i2h_msg_u *i2hmsg;

	i2hmsg = (union bfi_port_i2h_msg_u *)m;
	bfa_trc(port, m->mh.msg_id);

	switch (m->mh.msg_id) {
	case BFI_PORT_I2H_ENABLE_RSP:
		if (port->endis_pending == BFA_FALSE)
			break;
		bfa_port_enable_isr(port, i2hmsg->enable_rsp.status);
		break;

	case BFI_PORT_I2H_DISABLE_RSP:
		if (port->endis_pending == BFA_FALSE)
			break;
		bfa_port_disable_isr(port, i2hmsg->disable_rsp.status);
		break;

	case BFI_PORT_I2H_GET_STATS_RSP:
		
		if (port->stats_busy == BFA_FALSE)
			break;
		bfa_port_get_stats_isr(port, i2hmsg->getstats_rsp.status);
		break;

	case BFI_PORT_I2H_CLEAR_STATS_RSP:
		if (port->stats_busy == BFA_FALSE)
			break;
		bfa_port_clear_stats_isr(port, i2hmsg->clearstats_rsp.status);
		break;

	default:
		bfa_assert(0);
	}
}


u32
bfa_port_meminfo(void)
{
	return BFA_ROUNDUP(sizeof(union bfa_pport_stats_u), BFA_DMA_ALIGN_SZ);
}


void
bfa_port_mem_claim(struct bfa_port_s *port, u8 *dma_kva, u64 dma_pa)
{
	port->stats_dma.kva = dma_kva;
	port->stats_dma.pa = dma_pa;
}


bfa_status_t
bfa_port_enable(struct bfa_port_s *port, bfa_port_endis_cbfn_t cbfn,
		void *cbarg)
{
	struct bfi_port_generic_req_s *m;

	
	bfa_assert(0);

	if (!bfa_ioc_is_operational(port->ioc)) {
		bfa_trc(port, BFA_STATUS_IOC_FAILURE);
		return BFA_STATUS_IOC_FAILURE;
	}

	if (port->endis_pending) {
		bfa_trc(port, BFA_STATUS_DEVBUSY);
		return BFA_STATUS_DEVBUSY;
	}

	m = (struct bfi_port_generic_req_s *)port->endis_mb.msg;

	port->msgtag++;
	port->endis_cbfn = cbfn;
	port->endis_cbarg = cbarg;
	port->endis_pending = BFA_TRUE;

	bfi_h2i_set(m->mh, BFI_MC_PORT, BFI_PORT_H2I_ENABLE_REQ,
		    bfa_ioc_portid(port->ioc));
	bfa_ioc_mbox_queue(port->ioc, &port->endis_mb);

	return BFA_STATUS_OK;
}


bfa_status_t
bfa_port_disable(struct bfa_port_s *port, bfa_port_endis_cbfn_t cbfn,
		 void *cbarg)
{
	struct bfi_port_generic_req_s *m;

	
	bfa_assert(0);

	if (!bfa_ioc_is_operational(port->ioc)) {
		bfa_trc(port, BFA_STATUS_IOC_FAILURE);
		return BFA_STATUS_IOC_FAILURE;
	}

	if (port->endis_pending) {
		bfa_trc(port, BFA_STATUS_DEVBUSY);
		return BFA_STATUS_DEVBUSY;
	}

	m = (struct bfi_port_generic_req_s *)port->endis_mb.msg;

	port->msgtag++;
	port->endis_cbfn = cbfn;
	port->endis_cbarg = cbarg;
	port->endis_pending = BFA_TRUE;

	bfi_h2i_set(m->mh, BFI_MC_PORT, BFI_PORT_H2I_DISABLE_REQ,
		    bfa_ioc_portid(port->ioc));
	bfa_ioc_mbox_queue(port->ioc, &port->endis_mb);

	return BFA_STATUS_OK;
}


bfa_status_t
bfa_port_get_stats(struct bfa_port_s *port, union bfa_pport_stats_u *stats,
		   bfa_port_stats_cbfn_t cbfn, void *cbarg)
{
	struct bfi_port_get_stats_req_s *m;

	if (!bfa_ioc_is_operational(port->ioc)) {
		bfa_trc(port, BFA_STATUS_IOC_FAILURE);
		return BFA_STATUS_IOC_FAILURE;
	}

	if (port->stats_busy) {
		bfa_trc(port, BFA_STATUS_DEVBUSY);
		return BFA_STATUS_DEVBUSY;
	}

	m = (struct bfi_port_get_stats_req_s *)port->stats_mb.msg;

	port->stats = stats;
	port->stats_cbfn = cbfn;
	port->stats_cbarg = cbarg;
	port->stats_busy = BFA_TRUE;
	bfa_dma_be_addr_set(m->dma_addr, port->stats_dma.pa);

	bfi_h2i_set(m->mh, BFI_MC_PORT, BFI_PORT_H2I_GET_STATS_REQ,
		    bfa_ioc_portid(port->ioc));
	bfa_ioc_mbox_queue(port->ioc, &port->stats_mb);

	return BFA_STATUS_OK;
}


bfa_status_t
bfa_port_clear_stats(struct bfa_port_s *port, bfa_port_stats_cbfn_t cbfn,
		     void *cbarg)
{
	struct bfi_port_generic_req_s *m;

	if (!bfa_ioc_is_operational(port->ioc)) {
		bfa_trc(port, BFA_STATUS_IOC_FAILURE);
		return BFA_STATUS_IOC_FAILURE;
	}

	if (port->stats_busy) {
		bfa_trc(port, BFA_STATUS_DEVBUSY);
		return BFA_STATUS_DEVBUSY;
	}

	m = (struct bfi_port_generic_req_s *)port->stats_mb.msg;

	port->stats_cbfn = cbfn;
	port->stats_cbarg = cbarg;
	port->stats_busy = BFA_TRUE;

	bfi_h2i_set(m->mh, BFI_MC_PORT, BFI_PORT_H2I_CLEAR_STATS_REQ,
		    bfa_ioc_portid(port->ioc));
	bfa_ioc_mbox_queue(port->ioc, &port->stats_mb);

	return BFA_STATUS_OK;
}


void
bfa_port_hbfail(void *arg)
{
	struct bfa_port_s *port = (struct bfa_port_s *)arg;

	
	if (port->stats_busy) {
		if (port->stats_cbfn)
			port->stats_cbfn(port->dev, BFA_STATUS_FAILED);
		port->stats_cbfn = NULL;
		port->stats_busy = BFA_FALSE;
	}

	
	if (port->endis_pending) {
		if (port->endis_cbfn)
			port->endis_cbfn(port->dev, BFA_STATUS_FAILED);
		port->endis_cbfn = NULL;
		port->endis_pending = BFA_FALSE;
	}
}


void
bfa_port_attach(struct bfa_port_s *port, struct bfa_ioc_s *ioc, void *dev,
		struct bfa_trc_mod_s *trcmod, struct bfa_log_mod_s *logmod)
{
	bfa_assert(port);

	port->dev = dev;
	port->ioc = ioc;
	port->trcmod = trcmod;
	port->logmod = logmod;

	port->stats_busy = port->endis_pending = BFA_FALSE;
	port->stats_cbfn = port->endis_cbfn = NULL;

	bfa_ioc_mbox_regisr(port->ioc, BFI_MC_PORT, bfa_port_isr, port);
	bfa_ioc_hbfail_init(&port->hbfail, bfa_port_hbfail, port);
	bfa_ioc_hbfail_register(port->ioc, &port->hbfail);

	bfa_trc(port, 0);
}


void
bfa_port_detach(struct bfa_port_s *port)
{
	bfa_trc(port, 0);
}
