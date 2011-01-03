



#include <asm/unaligned.h>
#include <scsi/fc/fc_gs.h>
#include <scsi/fc/fc_ns.h>
#include <scsi/fc/fc_els.h>
#include <scsi/libfc.h>
#include <scsi/fc_encode.h>


static struct fc_seq *fc_elsct_send(struct fc_lport *lport,
				    u32 did,
				    struct fc_frame *fp,
				    unsigned int op,
				    void (*resp)(struct fc_seq *,
						 struct fc_frame *fp,
						 void *arg),
				    void *arg, u32 timer_msec)
{
	enum fc_rctl r_ctl;
	enum fc_fh_type fh_type;
	int rc;

	
	if ((op >= ELS_LS_RJT) && (op <= ELS_AUTH_ELS))
		rc = fc_els_fill(lport, did, fp, op, &r_ctl, &fh_type);
	else {
		
		rc = fc_ct_fill(lport, did, fp, op, &r_ctl, &fh_type);
		did = FC_FID_DIR_SERV;
	}

	if (rc) {
		fc_frame_free(fp);
		return NULL;
	}

	fc_fill_fc_hdr(fp, r_ctl, did, fc_host_port_id(lport->host), fh_type,
		       FC_FC_FIRST_SEQ | FC_FC_END_SEQ | FC_FC_SEQ_INIT, 0);

	return lport->tt.exch_seq_send(lport, fp, resp, NULL, arg, timer_msec);
}

int fc_elsct_init(struct fc_lport *lport)
{
	if (!lport->tt.elsct_send)
		lport->tt.elsct_send = fc_elsct_send;

	return 0;
}
EXPORT_SYMBOL(fc_elsct_init);


const char *fc_els_resp_type(struct fc_frame *fp)
{
	const char *msg;
	if (IS_ERR(fp)) {
		switch (-PTR_ERR(fp)) {
		case FC_NO_ERR:
			msg = "response no error";
			break;
		case FC_EX_TIMEOUT:
			msg = "response timeout";
			break;
		case FC_EX_CLOSED:
			msg = "response closed";
			break;
		default:
			msg = "response unknown error";
			break;
		}
	} else {
		switch (fc_frame_payload_op(fp)) {
		case ELS_LS_ACC:
			msg = "accept";
			break;
		case ELS_LS_RJT:
			msg = "reject";
			break;
		default:
			msg = "response unknown ELS";
			break;
		}
	}
	return msg;
}
