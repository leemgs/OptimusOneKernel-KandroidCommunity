



#include <fcs/bfa_fcs.h>
#include <bfa_svc.h>
#include <fcs/bfa_fcs_fabric.h>
#include "fcs.h"
#include "fcs_trcmod.h"
#include "fcs_fabric.h"
#include "fcs_uf.h"

BFA_TRC_FILE(FCS, UF);


static void
bfa_fcs_uf_recv(void *cbarg, struct bfa_uf_s *uf)
{
	struct bfa_fcs_s      *fcs = (struct bfa_fcs_s *) cbarg;
	struct fchs_s         *fchs = bfa_uf_get_frmbuf(uf);
	u16        len = bfa_uf_get_frmlen(uf);
	struct fc_vft_s       *vft;
	struct bfa_fcs_fabric_s *fabric;

	
	if (fchs->routing == FC_RTG_EXT_HDR &&
		fchs->cat_info == FC_CAT_VFT_HDR) {
		bfa_stats(fcs, uf.tagged);
		vft = bfa_uf_get_frmbuf(uf);
		if (fcs->port_vfid == vft->vf_id)
			fabric = &fcs->fabric;
		else
			fabric = bfa_fcs_vf_lookup(fcs, (u16) vft->vf_id);

		
		if (!fabric) {
			bfa_assert(0);
			bfa_stats(fcs, uf.vfid_unknown);
			bfa_uf_free(uf);
			return;
		}

		
		fchs = (struct fchs_s *) (vft + 1);
		len -= sizeof(struct fc_vft_s);

		bfa_trc(fcs, vft->vf_id);
	} else {
		bfa_stats(fcs, uf.untagged);
		fabric = &fcs->fabric;
	}

	bfa_trc(fcs, ((u32 *) fchs)[0]);
	bfa_trc(fcs, ((u32 *) fchs)[1]);
	bfa_trc(fcs, ((u32 *) fchs)[2]);
	bfa_trc(fcs, ((u32 *) fchs)[3]);
	bfa_trc(fcs, ((u32 *) fchs)[4]);
	bfa_trc(fcs, ((u32 *) fchs)[5]);
	bfa_trc(fcs, len);

	bfa_fcs_fabric_uf_recv(fabric, fchs, len);
	bfa_uf_free(uf);
}

void
bfa_fcs_uf_modinit(struct bfa_fcs_s *fcs)
{
	bfa_uf_recv_register(fcs->bfa, bfa_fcs_uf_recv, fcs);
}

void
bfa_fcs_uf_modexit(struct bfa_fcs_s *fcs)
{
	bfa_fcs_modexit_comp(fcs);
}
