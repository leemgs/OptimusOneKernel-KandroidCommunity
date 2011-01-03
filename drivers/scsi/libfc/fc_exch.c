



#include <linux/timer.h>
#include <linux/gfp.h>
#include <linux/err.h>

#include <scsi/fc/fc_fc2.h>

#include <scsi/libfc.h>
#include <scsi/fc_encode.h>

u16	fc_cpu_mask;		
EXPORT_SYMBOL(fc_cpu_mask);
static u16	fc_cpu_order;	
static struct kmem_cache *fc_em_cachep;        




struct fc_exch_pool {
	u16		next_index;	
	u16		total_exches;	
	spinlock_t	lock;		
	struct list_head	ex_list;	
};


struct fc_exch_mgr {
	enum fc_class	class;		
	struct kref	kref;		
	u16		min_xid;	
	u16		max_xid;	
	struct list_head	ex_list;	
	mempool_t	*ep_pool;	
	u16		pool_max_index;	
	struct fc_exch_pool *pool;	

	
	struct {
		atomic_t no_free_exch;
		atomic_t no_free_exch_xid;
		atomic_t xid_not_found;
		atomic_t xid_busy;
		atomic_t seq_not_found;
		atomic_t non_bls_resp;
	} stats;
};
#define	fc_seq_exch(sp) container_of(sp, struct fc_exch, seq)

struct fc_exch_mgr_anchor {
	struct list_head ema_list;
	struct fc_exch_mgr *mp;
	bool (*match)(struct fc_frame *);
};

static void fc_exch_rrq(struct fc_exch *);
static void fc_seq_ls_acc(struct fc_seq *);
static void fc_seq_ls_rjt(struct fc_seq *, enum fc_els_rjt_reason,
			  enum fc_els_rjt_explan);
static void fc_exch_els_rec(struct fc_seq *, struct fc_frame *);
static void fc_exch_els_rrq(struct fc_seq *, struct fc_frame *);
static struct fc_seq *fc_seq_start_next_locked(struct fc_seq *sp);






static char *fc_exch_rctl_names[] = FC_RCTL_NAMES_INIT;

#define FC_TABLE_SIZE(x)   (sizeof(x) / sizeof(x[0]))

static inline const char *fc_exch_name_lookup(unsigned int op, char **table,
					      unsigned int max_index)
{
	const char *name = NULL;

	if (op < max_index)
		name = table[op];
	if (!name)
		name = "unknown";
	return name;
}

static const char *fc_exch_rctl_name(unsigned int op)
{
	return fc_exch_name_lookup(op, fc_exch_rctl_names,
				   FC_TABLE_SIZE(fc_exch_rctl_names));
}


static void fc_exch_hold(struct fc_exch *ep)
{
	atomic_inc(&ep->ex_refcnt);
}


static void fc_exch_setup_hdr(struct fc_exch *ep, struct fc_frame *fp,
			      u32 f_ctl)
{
	struct fc_frame_header *fh = fc_frame_header_get(fp);
	u16 fill;

	fr_sof(fp) = ep->class;
	if (ep->seq.cnt)
		fr_sof(fp) = fc_sof_normal(ep->class);

	if (f_ctl & FC_FC_END_SEQ) {
		fr_eof(fp) = FC_EOF_T;
		if (fc_sof_needs_ack(ep->class))
			fr_eof(fp) = FC_EOF_N;
		
		fill = fr_len(fp) & 3;
		if (fill) {
			fill = 4 - fill;
			
			skb_put(fp_skb(fp), fill);
			hton24(fh->fh_f_ctl, f_ctl | fill);
		}
	} else {
		WARN_ON(fr_len(fp) % 4 != 0);	
		fr_eof(fp) = FC_EOF_N;
	}

	
	fh->fh_ox_id = htons(ep->oxid);
	fh->fh_rx_id = htons(ep->rxid);
	fh->fh_seq_id = ep->seq.id;
	fh->fh_seq_cnt = htons(ep->seq.cnt);
}



static void fc_exch_release(struct fc_exch *ep)
{
	struct fc_exch_mgr *mp;

	if (atomic_dec_and_test(&ep->ex_refcnt)) {
		mp = ep->em;
		if (ep->destructor)
			ep->destructor(&ep->seq, ep->arg);
		WARN_ON(!(ep->esb_stat & ESB_ST_COMPLETE));
		mempool_free(ep, mp->ep_pool);
	}
}

static int fc_exch_done_locked(struct fc_exch *ep)
{
	int rc = 1;

	
	ep->resp = NULL;
	if (ep->state & FC_EX_DONE)
		return rc;
	ep->esb_stat |= ESB_ST_COMPLETE;

	if (!(ep->esb_stat & ESB_ST_REC_QUAL)) {
		ep->state |= FC_EX_DONE;
		if (cancel_delayed_work(&ep->timeout_work))
			atomic_dec(&ep->ex_refcnt); 
		rc = 0;
	}
	return rc;
}

static inline struct fc_exch *fc_exch_ptr_get(struct fc_exch_pool *pool,
					      u16 index)
{
	struct fc_exch **exches = (struct fc_exch **)(pool + 1);
	return exches[index];
}

static inline void fc_exch_ptr_set(struct fc_exch_pool *pool, u16 index,
				   struct fc_exch *ep)
{
	((struct fc_exch **)(pool + 1))[index] = ep;
}

static void fc_exch_delete(struct fc_exch *ep)
{
	struct fc_exch_pool *pool;

	pool = ep->pool;
	spin_lock_bh(&pool->lock);
	WARN_ON(pool->total_exches <= 0);
	pool->total_exches--;
	fc_exch_ptr_set(pool, (ep->xid - ep->em->min_xid) >> fc_cpu_order,
			NULL);
	list_del(&ep->ex_list);
	spin_unlock_bh(&pool->lock);
	fc_exch_release(ep);	
}


static inline void fc_exch_timer_set_locked(struct fc_exch *ep,
					    unsigned int timer_msec)
{
	if (ep->state & (FC_EX_RST_CLEANUP | FC_EX_DONE))
		return;

	FC_EXCH_DBG(ep, "Exchange timer armed\n");

	if (schedule_delayed_work(&ep->timeout_work,
				  msecs_to_jiffies(timer_msec)))
		fc_exch_hold(ep);		
}


static void fc_exch_timer_set(struct fc_exch *ep, unsigned int timer_msec)
{
	spin_lock_bh(&ep->ex_lock);
	fc_exch_timer_set_locked(ep, timer_msec);
	spin_unlock_bh(&ep->ex_lock);
}

int fc_seq_exch_abort(const struct fc_seq *req_sp, unsigned int timer_msec)
{
	struct fc_seq *sp;
	struct fc_exch *ep;
	struct fc_frame *fp;
	int error;

	ep = fc_seq_exch(req_sp);

	spin_lock_bh(&ep->ex_lock);
	if (ep->esb_stat & (ESB_ST_COMPLETE | ESB_ST_ABNORMAL) ||
	    ep->state & (FC_EX_DONE | FC_EX_RST_CLEANUP)) {
		spin_unlock_bh(&ep->ex_lock);
		return -ENXIO;
	}

	
	sp = fc_seq_start_next_locked(&ep->seq);
	if (!sp) {
		spin_unlock_bh(&ep->ex_lock);
		return -ENOMEM;
	}

	ep->esb_stat |= ESB_ST_SEQ_INIT | ESB_ST_ABNORMAL;
	if (timer_msec)
		fc_exch_timer_set_locked(ep, timer_msec);
	spin_unlock_bh(&ep->ex_lock);

	
	if (!ep->sid)
		return 0;

	
	fp = fc_frame_alloc(ep->lp, 0);
	if (fp) {
		fc_fill_fc_hdr(fp, FC_RCTL_BA_ABTS, ep->did, ep->sid,
			       FC_TYPE_BLS, FC_FC_END_SEQ | FC_FC_SEQ_INIT, 0);
		error = fc_seq_send(ep->lp, sp, fp);
	} else
		error = -ENOBUFS;
	return error;
}
EXPORT_SYMBOL(fc_seq_exch_abort);


static void fc_exch_timeout(struct work_struct *work)
{
	struct fc_exch *ep = container_of(work, struct fc_exch,
					  timeout_work.work);
	struct fc_seq *sp = &ep->seq;
	void (*resp)(struct fc_seq *, struct fc_frame *fp, void *arg);
	void *arg;
	u32 e_stat;
	int rc = 1;

	FC_EXCH_DBG(ep, "Exchange timed out\n");

	spin_lock_bh(&ep->ex_lock);
	if (ep->state & (FC_EX_RST_CLEANUP | FC_EX_DONE))
		goto unlock;

	e_stat = ep->esb_stat;
	if (e_stat & ESB_ST_COMPLETE) {
		ep->esb_stat = e_stat & ~ESB_ST_REC_QUAL;
		spin_unlock_bh(&ep->ex_lock);
		if (e_stat & ESB_ST_REC_QUAL)
			fc_exch_rrq(ep);
		goto done;
	} else {
		resp = ep->resp;
		arg = ep->arg;
		ep->resp = NULL;
		if (e_stat & ESB_ST_ABNORMAL)
			rc = fc_exch_done_locked(ep);
		spin_unlock_bh(&ep->ex_lock);
		if (!rc)
			fc_exch_delete(ep);
		if (resp)
			resp(sp, ERR_PTR(-FC_EX_TIMEOUT), arg);
		fc_seq_exch_abort(sp, 2 * ep->r_a_tov);
		goto done;
	}
unlock:
	spin_unlock_bh(&ep->ex_lock);
done:
	
	fc_exch_release(ep);
}


static struct fc_seq *fc_seq_alloc(struct fc_exch *ep, u8 seq_id)
{
	struct fc_seq *sp;

	sp = &ep->seq;
	sp->ssb_stat = 0;
	sp->cnt = 0;
	sp->id = seq_id;
	return sp;
}


static struct fc_exch *fc_exch_em_alloc(struct fc_lport *lport,
					struct fc_exch_mgr *mp)
{
	struct fc_exch *ep;
	unsigned int cpu;
	u16 index;
	struct fc_exch_pool *pool;

	
	ep = mempool_alloc(mp->ep_pool, GFP_ATOMIC);
	if (!ep) {
		atomic_inc(&mp->stats.no_free_exch);
		goto out;
	}
	memset(ep, 0, sizeof(*ep));

	cpu = smp_processor_id();
	pool = per_cpu_ptr(mp->pool, cpu);
	spin_lock_bh(&pool->lock);
	index = pool->next_index;
	
	while (fc_exch_ptr_get(pool, index)) {
		index = index == mp->pool_max_index ? 0 : index + 1;
		if (index == pool->next_index)
			goto err;
	}
	pool->next_index = index == mp->pool_max_index ? 0 : index + 1;

	fc_exch_hold(ep);	
	spin_lock_init(&ep->ex_lock);
	
	spin_lock_bh(&ep->ex_lock);

	fc_exch_ptr_set(pool, index, ep);
	list_add_tail(&ep->ex_list, &pool->ex_list);
	fc_seq_alloc(ep, ep->seq_id++);
	pool->total_exches++;
	spin_unlock_bh(&pool->lock);

	
	ep->oxid = ep->xid = (index << fc_cpu_order | cpu) + mp->min_xid;
	ep->em = mp;
	ep->pool = pool;
	ep->lp = lport;
	ep->f_ctl = FC_FC_FIRST_SEQ;	
	ep->rxid = FC_XID_UNKNOWN;
	ep->class = mp->class;
	INIT_DELAYED_WORK(&ep->timeout_work, fc_exch_timeout);
out:
	return ep;
err:
	spin_unlock_bh(&pool->lock);
	atomic_inc(&mp->stats.no_free_exch_xid);
	mempool_free(ep, mp->ep_pool);
	return NULL;
}


struct fc_exch *fc_exch_alloc(struct fc_lport *lport, struct fc_frame *fp)
{
	struct fc_exch_mgr_anchor *ema;
	struct fc_exch *ep;

	list_for_each_entry(ema, &lport->ema_list, ema_list) {
		if (!ema->match || ema->match(fp)) {
			ep = fc_exch_em_alloc(lport, ema->mp);
			if (ep)
				return ep;
		}
	}
	return NULL;
}
EXPORT_SYMBOL(fc_exch_alloc);


static struct fc_exch *fc_exch_find(struct fc_exch_mgr *mp, u16 xid)
{
	struct fc_exch_pool *pool;
	struct fc_exch *ep = NULL;

	if ((xid >= mp->min_xid) && (xid <= mp->max_xid)) {
		pool = per_cpu_ptr(mp->pool, xid & fc_cpu_mask);
		spin_lock_bh(&pool->lock);
		ep = fc_exch_ptr_get(pool, (xid - mp->min_xid) >> fc_cpu_order);
		if (ep) {
			fc_exch_hold(ep);
			WARN_ON(ep->xid != xid);
		}
		spin_unlock_bh(&pool->lock);
	}
	return ep;
}

void fc_exch_done(struct fc_seq *sp)
{
	struct fc_exch *ep = fc_seq_exch(sp);
	int rc;

	spin_lock_bh(&ep->ex_lock);
	rc = fc_exch_done_locked(ep);
	spin_unlock_bh(&ep->ex_lock);
	if (!rc)
		fc_exch_delete(ep);
}
EXPORT_SYMBOL(fc_exch_done);


static struct fc_exch *fc_exch_resp(struct fc_lport *lport,
				    struct fc_exch_mgr *mp,
				    struct fc_frame *fp)
{
	struct fc_exch *ep;
	struct fc_frame_header *fh;

	ep = fc_exch_alloc(lport, fp);
	if (ep) {
		ep->class = fc_frame_class(fp);

		
		ep->f_ctl |= FC_FC_EX_CTX;	
		ep->f_ctl &= ~FC_FC_FIRST_SEQ;	
		fh = fc_frame_header_get(fp);
		ep->sid = ntoh24(fh->fh_d_id);
		ep->did = ntoh24(fh->fh_s_id);
		ep->oid = ep->did;

		
		ep->rxid = ep->xid;
		ep->oxid = ntohs(fh->fh_ox_id);
		ep->esb_stat |= ESB_ST_RESP | ESB_ST_SEQ_INIT;
		if ((ntoh24(fh->fh_f_ctl) & FC_FC_SEQ_INIT) == 0)
			ep->esb_stat &= ~ESB_ST_SEQ_INIT;

		fc_exch_hold(ep);	
		spin_unlock_bh(&ep->ex_lock);	
	}
	return ep;
}


static enum fc_pf_rjt_reason fc_seq_lookup_recip(struct fc_lport *lport,
						 struct fc_exch_mgr *mp,
						 struct fc_frame *fp)
{
	struct fc_frame_header *fh = fc_frame_header_get(fp);
	struct fc_exch *ep = NULL;
	struct fc_seq *sp = NULL;
	enum fc_pf_rjt_reason reject = FC_RJT_NONE;
	u32 f_ctl;
	u16 xid;

	f_ctl = ntoh24(fh->fh_f_ctl);
	WARN_ON((f_ctl & FC_FC_SEQ_CTX) != 0);

	
	if (f_ctl & FC_FC_EX_CTX) {
		xid = ntohs(fh->fh_ox_id);	
		ep = fc_exch_find(mp, xid);
		if (!ep) {
			atomic_inc(&mp->stats.xid_not_found);
			reject = FC_RJT_OX_ID;
			goto out;
		}
		if (ep->rxid == FC_XID_UNKNOWN)
			ep->rxid = ntohs(fh->fh_rx_id);
		else if (ep->rxid != ntohs(fh->fh_rx_id)) {
			reject = FC_RJT_OX_ID;
			goto rel;
		}
	} else {
		xid = ntohs(fh->fh_rx_id);	

		
		if (xid == 0 && fh->fh_r_ctl == FC_RCTL_ELS_REQ &&
		    fc_frame_payload_op(fp) == ELS_TEST) {
			fh->fh_rx_id = htons(FC_XID_UNKNOWN);
			xid = FC_XID_UNKNOWN;
		}

		
		ep = fc_exch_find(mp, xid);
		if ((f_ctl & FC_FC_FIRST_SEQ) && fc_sof_is_init(fr_sof(fp))) {
			if (ep) {
				atomic_inc(&mp->stats.xid_busy);
				reject = FC_RJT_RX_ID;
				goto rel;
			}
			ep = fc_exch_resp(lport, mp, fp);
			if (!ep) {
				reject = FC_RJT_EXCH_EST;	
				goto out;
			}
			xid = ep->xid;	
		} else if (!ep) {
			atomic_inc(&mp->stats.xid_not_found);
			reject = FC_RJT_RX_ID;	
			goto out;
		}
	}

	
	if (fc_sof_is_init(fr_sof(fp))) {
		sp = fc_seq_start_next(&ep->seq);
		if (!sp) {
			reject = FC_RJT_SEQ_XS;	
			goto rel;
		}
		sp->id = fh->fh_seq_id;
		sp->ssb_stat |= SSB_ST_RESP;
	} else {
		sp = &ep->seq;
		if (sp->id != fh->fh_seq_id) {
			atomic_inc(&mp->stats.seq_not_found);
			reject = FC_RJT_SEQ_ID;	
			goto rel;
		}
	}
	WARN_ON(ep != fc_seq_exch(sp));

	if (f_ctl & FC_FC_SEQ_INIT)
		ep->esb_stat |= ESB_ST_SEQ_INIT;

	fr_seq(fp) = sp;
out:
	return reject;
rel:
	fc_exch_done(&ep->seq);
	fc_exch_release(ep);	
	return reject;
}


static struct fc_seq *fc_seq_lookup_orig(struct fc_exch_mgr *mp,
					 struct fc_frame *fp)
{
	struct fc_frame_header *fh = fc_frame_header_get(fp);
	struct fc_exch *ep;
	struct fc_seq *sp = NULL;
	u32 f_ctl;
	u16 xid;

	f_ctl = ntoh24(fh->fh_f_ctl);
	WARN_ON((f_ctl & FC_FC_SEQ_CTX) != FC_FC_SEQ_CTX);
	xid = ntohs((f_ctl & FC_FC_EX_CTX) ? fh->fh_ox_id : fh->fh_rx_id);
	ep = fc_exch_find(mp, xid);
	if (!ep)
		return NULL;
	if (ep->seq.id == fh->fh_seq_id) {
		
		sp = &ep->seq;
		if ((f_ctl & FC_FC_EX_CTX) != 0 &&
		    ep->rxid == FC_XID_UNKNOWN) {
			ep->rxid = ntohs(fh->fh_rx_id);
		}
	}
	fc_exch_release(ep);
	return sp;
}


static void fc_exch_set_addr(struct fc_exch *ep,
			     u32 orig_id, u32 resp_id)
{
	ep->oid = orig_id;
	if (ep->esb_stat & ESB_ST_RESP) {
		ep->sid = resp_id;
		ep->did = orig_id;
	} else {
		ep->sid = orig_id;
		ep->did = resp_id;
	}
}

static struct fc_seq *fc_seq_start_next_locked(struct fc_seq *sp)
{
	struct fc_exch *ep = fc_seq_exch(sp);

	sp = fc_seq_alloc(ep, ep->seq_id++);
	FC_EXCH_DBG(ep, "f_ctl %6x seq %2x\n",
		    ep->f_ctl, sp->id);
	return sp;
}

struct fc_seq *fc_seq_start_next(struct fc_seq *sp)
{
	struct fc_exch *ep = fc_seq_exch(sp);

	spin_lock_bh(&ep->ex_lock);
	sp = fc_seq_start_next_locked(sp);
	spin_unlock_bh(&ep->ex_lock);

	return sp;
}
EXPORT_SYMBOL(fc_seq_start_next);

int fc_seq_send(struct fc_lport *lp, struct fc_seq *sp, struct fc_frame *fp)
{
	struct fc_exch *ep;
	struct fc_frame_header *fh = fc_frame_header_get(fp);
	int error;
	u32	f_ctl;

	ep = fc_seq_exch(sp);
	WARN_ON((ep->esb_stat & ESB_ST_SEQ_INIT) != ESB_ST_SEQ_INIT);

	f_ctl = ntoh24(fh->fh_f_ctl);
	fc_exch_setup_hdr(ep, fp, f_ctl);

	
	if (fr_max_payload(fp))
		sp->cnt += DIV_ROUND_UP((fr_len(fp) - sizeof(*fh)),
					fr_max_payload(fp));
	else
		sp->cnt++;

	
	error = lp->tt.frame_send(lp, fp);

	
	spin_lock_bh(&ep->ex_lock);
	ep->f_ctl = f_ctl & ~FC_FC_FIRST_SEQ;	
	if (f_ctl & (FC_FC_END_SEQ | FC_FC_SEQ_INIT))
		ep->esb_stat &= ~ESB_ST_SEQ_INIT;
	spin_unlock_bh(&ep->ex_lock);
	return error;
}
EXPORT_SYMBOL(fc_seq_send);

void fc_seq_els_rsp_send(struct fc_seq *sp, enum fc_els_cmd els_cmd,
			 struct fc_seq_els_data *els_data)
{
	switch (els_cmd) {
	case ELS_LS_RJT:
		fc_seq_ls_rjt(sp, els_data->reason, els_data->explan);
		break;
	case ELS_LS_ACC:
		fc_seq_ls_acc(sp);
		break;
	case ELS_RRQ:
		fc_exch_els_rrq(sp, els_data->fp);
		break;
	case ELS_REC:
		fc_exch_els_rec(sp, els_data->fp);
		break;
	default:
		FC_EXCH_DBG(fc_seq_exch(sp), "Invalid ELS CMD:%x\n", els_cmd);
	}
}
EXPORT_SYMBOL(fc_seq_els_rsp_send);


static void fc_seq_send_last(struct fc_seq *sp, struct fc_frame *fp,
			     enum fc_rctl rctl, enum fc_fh_type fh_type)
{
	u32 f_ctl;
	struct fc_exch *ep = fc_seq_exch(sp);

	f_ctl = FC_FC_LAST_SEQ | FC_FC_END_SEQ | FC_FC_SEQ_INIT;
	f_ctl |= ep->f_ctl;
	fc_fill_fc_hdr(fp, rctl, ep->did, ep->sid, fh_type, f_ctl, 0);
	fc_seq_send(ep->lp, sp, fp);
}


static void fc_seq_send_ack(struct fc_seq *sp, const struct fc_frame *rx_fp)
{
	struct fc_frame *fp;
	struct fc_frame_header *rx_fh;
	struct fc_frame_header *fh;
	struct fc_exch *ep = fc_seq_exch(sp);
	struct fc_lport *lp = ep->lp;
	unsigned int f_ctl;

	
	if (fc_sof_needs_ack(fr_sof(rx_fp))) {
		fp = fc_frame_alloc(lp, 0);
		if (!fp)
			return;

		fh = fc_frame_header_get(fp);
		fh->fh_r_ctl = FC_RCTL_ACK_1;
		fh->fh_type = FC_TYPE_BLS;

		
		rx_fh = fc_frame_header_get(rx_fp);
		f_ctl = ntoh24(rx_fh->fh_f_ctl);
		f_ctl &= FC_FC_EX_CTX | FC_FC_SEQ_CTX |
			FC_FC_FIRST_SEQ | FC_FC_LAST_SEQ |
			FC_FC_END_SEQ | FC_FC_END_CONN | FC_FC_SEQ_INIT |
			FC_FC_RETX_SEQ | FC_FC_UNI_TX;
		f_ctl ^= FC_FC_EX_CTX | FC_FC_SEQ_CTX;
		hton24(fh->fh_f_ctl, f_ctl);

		fc_exch_setup_hdr(ep, fp, f_ctl);
		fh->fh_seq_id = rx_fh->fh_seq_id;
		fh->fh_seq_cnt = rx_fh->fh_seq_cnt;
		fh->fh_parm_offset = htonl(1);	

		fr_sof(fp) = fr_sof(rx_fp);
		if (f_ctl & FC_FC_END_SEQ)
			fr_eof(fp) = FC_EOF_T;
		else
			fr_eof(fp) = FC_EOF_N;

		(void) lp->tt.frame_send(lp, fp);
	}
}


static void fc_exch_send_ba_rjt(struct fc_frame *rx_fp,
				enum fc_ba_rjt_reason reason,
				enum fc_ba_rjt_explan explan)
{
	struct fc_frame *fp;
	struct fc_frame_header *rx_fh;
	struct fc_frame_header *fh;
	struct fc_ba_rjt *rp;
	struct fc_lport *lp;
	unsigned int f_ctl;

	lp = fr_dev(rx_fp);
	fp = fc_frame_alloc(lp, sizeof(*rp));
	if (!fp)
		return;
	fh = fc_frame_header_get(fp);
	rx_fh = fc_frame_header_get(rx_fp);

	memset(fh, 0, sizeof(*fh) + sizeof(*rp));

	rp = fc_frame_payload_get(fp, sizeof(*rp));
	rp->br_reason = reason;
	rp->br_explan = explan;

	
	memcpy(fh->fh_s_id, rx_fh->fh_d_id, 3);
	memcpy(fh->fh_d_id, rx_fh->fh_s_id, 3);
	fh->fh_ox_id = rx_fh->fh_ox_id;
	fh->fh_rx_id = rx_fh->fh_rx_id;
	fh->fh_seq_cnt = rx_fh->fh_seq_cnt;
	fh->fh_r_ctl = FC_RCTL_BA_RJT;
	fh->fh_type = FC_TYPE_BLS;

	
	f_ctl = ntoh24(rx_fh->fh_f_ctl);
	f_ctl &= FC_FC_EX_CTX | FC_FC_SEQ_CTX |
		FC_FC_END_CONN | FC_FC_SEQ_INIT |
		FC_FC_RETX_SEQ | FC_FC_UNI_TX;
	f_ctl ^= FC_FC_EX_CTX | FC_FC_SEQ_CTX;
	f_ctl |= FC_FC_LAST_SEQ | FC_FC_END_SEQ;
	f_ctl &= ~FC_FC_FIRST_SEQ;
	hton24(fh->fh_f_ctl, f_ctl);

	fr_sof(fp) = fc_sof_class(fr_sof(rx_fp));
	fr_eof(fp) = FC_EOF_T;
	if (fc_sof_needs_ack(fr_sof(fp)))
		fr_eof(fp) = FC_EOF_N;

	(void) lp->tt.frame_send(lp, fp);
}


static void fc_exch_recv_abts(struct fc_exch *ep, struct fc_frame *rx_fp)
{
	struct fc_frame *fp;
	struct fc_ba_acc *ap;
	struct fc_frame_header *fh;
	struct fc_seq *sp;

	if (!ep)
		goto reject;
	spin_lock_bh(&ep->ex_lock);
	if (ep->esb_stat & ESB_ST_COMPLETE) {
		spin_unlock_bh(&ep->ex_lock);
		goto reject;
	}
	if (!(ep->esb_stat & ESB_ST_REC_QUAL))
		fc_exch_hold(ep);		
	ep->esb_stat |= ESB_ST_ABNORMAL | ESB_ST_REC_QUAL;
	fc_exch_timer_set_locked(ep, ep->r_a_tov);

	fp = fc_frame_alloc(ep->lp, sizeof(*ap));
	if (!fp) {
		spin_unlock_bh(&ep->ex_lock);
		goto free;
	}
	fh = fc_frame_header_get(fp);
	ap = fc_frame_payload_get(fp, sizeof(*ap));
	memset(ap, 0, sizeof(*ap));
	sp = &ep->seq;
	ap->ba_high_seq_cnt = htons(0xffff);
	if (sp->ssb_stat & SSB_ST_RESP) {
		ap->ba_seq_id = sp->id;
		ap->ba_seq_id_val = FC_BA_SEQ_ID_VAL;
		ap->ba_high_seq_cnt = fh->fh_seq_cnt;
		ap->ba_low_seq_cnt = htons(sp->cnt);
	}
	sp = fc_seq_start_next_locked(sp);
	spin_unlock_bh(&ep->ex_lock);
	fc_seq_send_last(sp, fp, FC_RCTL_BA_ACC, FC_TYPE_BLS);
	fc_frame_free(rx_fp);
	return;

reject:
	fc_exch_send_ba_rjt(rx_fp, FC_BA_RJT_UNABLE, FC_BA_RJT_INV_XID);
free:
	fc_frame_free(rx_fp);
}


static void fc_exch_recv_req(struct fc_lport *lp, struct fc_exch_mgr *mp,
			     struct fc_frame *fp)
{
	struct fc_frame_header *fh = fc_frame_header_get(fp);
	struct fc_seq *sp = NULL;
	struct fc_exch *ep = NULL;
	enum fc_sof sof;
	enum fc_eof eof;
	u32 f_ctl;
	enum fc_pf_rjt_reason reject;

	fr_seq(fp) = NULL;
	reject = fc_seq_lookup_recip(lp, mp, fp);
	if (reject == FC_RJT_NONE) {
		sp = fr_seq(fp);	
		ep = fc_seq_exch(sp);
		sof = fr_sof(fp);
		eof = fr_eof(fp);
		f_ctl = ntoh24(fh->fh_f_ctl);
		fc_seq_send_ack(sp, fp);

		
		if (ep->resp)
			ep->resp(sp, fp, ep->arg);
		else
			lp->tt.lport_recv(lp, sp, fp);
		fc_exch_release(ep);	
	} else {
		FC_LPORT_DBG(lp, "exch/seq lookup failed: reject %x\n", reject);
		fc_frame_free(fp);
	}
}


static void fc_exch_recv_seq_resp(struct fc_exch_mgr *mp, struct fc_frame *fp)
{
	struct fc_frame_header *fh = fc_frame_header_get(fp);
	struct fc_seq *sp;
	struct fc_exch *ep;
	enum fc_sof sof;
	u32 f_ctl;
	void (*resp)(struct fc_seq *, struct fc_frame *fp, void *arg);
	void *ex_resp_arg;
	int rc;

	ep = fc_exch_find(mp, ntohs(fh->fh_ox_id));
	if (!ep) {
		atomic_inc(&mp->stats.xid_not_found);
		goto out;
	}
	if (ep->esb_stat & ESB_ST_COMPLETE) {
		atomic_inc(&mp->stats.xid_not_found);
		goto out;
	}
	if (ep->rxid == FC_XID_UNKNOWN)
		ep->rxid = ntohs(fh->fh_rx_id);
	if (ep->sid != 0 && ep->sid != ntoh24(fh->fh_d_id)) {
		atomic_inc(&mp->stats.xid_not_found);
		goto rel;
	}
	if (ep->did != ntoh24(fh->fh_s_id) &&
	    ep->did != FC_FID_FLOGI) {
		atomic_inc(&mp->stats.xid_not_found);
		goto rel;
	}
	sof = fr_sof(fp);
	if (fc_sof_is_init(sof)) {
		sp = fc_seq_start_next(&ep->seq);
		sp->id = fh->fh_seq_id;
		sp->ssb_stat |= SSB_ST_RESP;
	} else {
		sp = &ep->seq;
		if (sp->id != fh->fh_seq_id) {
			atomic_inc(&mp->stats.seq_not_found);
			goto rel;
		}
	}
	f_ctl = ntoh24(fh->fh_f_ctl);
	fr_seq(fp) = sp;
	if (f_ctl & FC_FC_SEQ_INIT)
		ep->esb_stat |= ESB_ST_SEQ_INIT;

	if (fc_sof_needs_ack(sof))
		fc_seq_send_ack(sp, fp);
	resp = ep->resp;
	ex_resp_arg = ep->arg;

	if (fh->fh_type != FC_TYPE_FCP && fr_eof(fp) == FC_EOF_T &&
	    (f_ctl & (FC_FC_LAST_SEQ | FC_FC_END_SEQ)) ==
	    (FC_FC_LAST_SEQ | FC_FC_END_SEQ)) {
		spin_lock_bh(&ep->ex_lock);
		rc = fc_exch_done_locked(ep);
		WARN_ON(fc_seq_exch(sp) != ep);
		spin_unlock_bh(&ep->ex_lock);
		if (!rc)
			fc_exch_delete(ep);
	}

	
	if (resp)
		resp(sp, fp, ex_resp_arg);
	else
		fc_frame_free(fp);
	fc_exch_release(ep);
	return;
rel:
	fc_exch_release(ep);
out:
	fc_frame_free(fp);
}


static void fc_exch_recv_resp(struct fc_exch_mgr *mp, struct fc_frame *fp)
{
	struct fc_seq *sp;

	sp = fc_seq_lookup_orig(mp, fp);	

	if (!sp)
		atomic_inc(&mp->stats.xid_not_found);
	else
		atomic_inc(&mp->stats.non_bls_resp);

	fc_frame_free(fp);
}


static void fc_exch_abts_resp(struct fc_exch *ep, struct fc_frame *fp)
{
	void (*resp)(struct fc_seq *, struct fc_frame *fp, void *arg);
	void *ex_resp_arg;
	struct fc_frame_header *fh;
	struct fc_ba_acc *ap;
	struct fc_seq *sp;
	u16 low;
	u16 high;
	int rc = 1, has_rec = 0;

	fh = fc_frame_header_get(fp);
	FC_EXCH_DBG(ep, "exch: BLS rctl %x - %s\n", fh->fh_r_ctl,
		    fc_exch_rctl_name(fh->fh_r_ctl));

	if (cancel_delayed_work_sync(&ep->timeout_work))
		fc_exch_release(ep);	

	spin_lock_bh(&ep->ex_lock);
	switch (fh->fh_r_ctl) {
	case FC_RCTL_BA_ACC:
		ap = fc_frame_payload_get(fp, sizeof(*ap));
		if (!ap)
			break;

		
		low = ntohs(ap->ba_low_seq_cnt);
		high = ntohs(ap->ba_high_seq_cnt);
		if ((ep->esb_stat & ESB_ST_REC_QUAL) == 0 &&
		    (ap->ba_seq_id_val != FC_BA_SEQ_ID_VAL ||
		     ap->ba_seq_id == ep->seq_id) && low != high) {
			ep->esb_stat |= ESB_ST_REC_QUAL;
			fc_exch_hold(ep);  
			has_rec = 1;
		}
		break;
	case FC_RCTL_BA_RJT:
		break;
	default:
		break;
	}

	resp = ep->resp;
	ex_resp_arg = ep->arg;

	
	sp = &ep->seq;
	
	if (ep->fh_type != FC_TYPE_FCP &&
	    ntoh24(fh->fh_f_ctl) & FC_FC_LAST_SEQ)
		rc = fc_exch_done_locked(ep);
	spin_unlock_bh(&ep->ex_lock);
	if (!rc)
		fc_exch_delete(ep);

	if (resp)
		resp(sp, fp, ex_resp_arg);
	else
		fc_frame_free(fp);

	if (has_rec)
		fc_exch_timer_set(ep, ep->r_a_tov);

}


static void fc_exch_recv_bls(struct fc_exch_mgr *mp, struct fc_frame *fp)
{
	struct fc_frame_header *fh;
	struct fc_exch *ep;
	u32 f_ctl;

	fh = fc_frame_header_get(fp);
	f_ctl = ntoh24(fh->fh_f_ctl);
	fr_seq(fp) = NULL;

	ep = fc_exch_find(mp, (f_ctl & FC_FC_EX_CTX) ?
			  ntohs(fh->fh_ox_id) : ntohs(fh->fh_rx_id));
	if (ep && (f_ctl & FC_FC_SEQ_INIT)) {
		spin_lock_bh(&ep->ex_lock);
		ep->esb_stat |= ESB_ST_SEQ_INIT;
		spin_unlock_bh(&ep->ex_lock);
	}
	if (f_ctl & FC_FC_SEQ_CTX) {
		
		switch (fh->fh_r_ctl) {
		case FC_RCTL_ACK_1:
		case FC_RCTL_ACK_0:
			break;
		default:
			FC_EXCH_DBG(ep, "BLS rctl %x - %s received",
				    fh->fh_r_ctl,
				    fc_exch_rctl_name(fh->fh_r_ctl));
			break;
		}
		fc_frame_free(fp);
	} else {
		switch (fh->fh_r_ctl) {
		case FC_RCTL_BA_RJT:
		case FC_RCTL_BA_ACC:
			if (ep)
				fc_exch_abts_resp(ep, fp);
			else
				fc_frame_free(fp);
			break;
		case FC_RCTL_BA_ABTS:
			fc_exch_recv_abts(ep, fp);
			break;
		default:			
			fc_frame_free(fp);
			break;
		}
	}
	if (ep)
		fc_exch_release(ep);	
}


static void fc_seq_ls_acc(struct fc_seq *req_sp)
{
	struct fc_seq *sp;
	struct fc_els_ls_acc *acc;
	struct fc_frame *fp;

	sp = fc_seq_start_next(req_sp);
	fp = fc_frame_alloc(fc_seq_exch(sp)->lp, sizeof(*acc));
	if (fp) {
		acc = fc_frame_payload_get(fp, sizeof(*acc));
		memset(acc, 0, sizeof(*acc));
		acc->la_cmd = ELS_LS_ACC;
		fc_seq_send_last(sp, fp, FC_RCTL_ELS_REP, FC_TYPE_ELS);
	}
}


static void fc_seq_ls_rjt(struct fc_seq *req_sp, enum fc_els_rjt_reason reason,
			  enum fc_els_rjt_explan explan)
{
	struct fc_seq *sp;
	struct fc_els_ls_rjt *rjt;
	struct fc_frame *fp;

	sp = fc_seq_start_next(req_sp);
	fp = fc_frame_alloc(fc_seq_exch(sp)->lp, sizeof(*rjt));
	if (fp) {
		rjt = fc_frame_payload_get(fp, sizeof(*rjt));
		memset(rjt, 0, sizeof(*rjt));
		rjt->er_cmd = ELS_LS_RJT;
		rjt->er_reason = reason;
		rjt->er_explan = explan;
		fc_seq_send_last(sp, fp, FC_RCTL_ELS_REP, FC_TYPE_ELS);
	}
}

static void fc_exch_reset(struct fc_exch *ep)
{
	struct fc_seq *sp;
	void (*resp)(struct fc_seq *, struct fc_frame *, void *);
	void *arg;
	int rc = 1;

	spin_lock_bh(&ep->ex_lock);
	ep->state |= FC_EX_RST_CLEANUP;
	
	if (cancel_delayed_work(&ep->timeout_work))
		atomic_dec(&ep->ex_refcnt);	
	resp = ep->resp;
	ep->resp = NULL;
	if (ep->esb_stat & ESB_ST_REC_QUAL)
		atomic_dec(&ep->ex_refcnt);	
	ep->esb_stat &= ~ESB_ST_REC_QUAL;
	arg = ep->arg;
	sp = &ep->seq;
	rc = fc_exch_done_locked(ep);
	spin_unlock_bh(&ep->ex_lock);
	if (!rc)
		fc_exch_delete(ep);

	if (resp)
		resp(sp, ERR_PTR(-FC_EX_CLOSED), arg);
}


static void fc_exch_pool_reset(struct fc_lport *lport,
			       struct fc_exch_pool *pool,
			       u32 sid, u32 did)
{
	struct fc_exch *ep;
	struct fc_exch *next;

	spin_lock_bh(&pool->lock);
restart:
	list_for_each_entry_safe(ep, next, &pool->ex_list, ex_list) {
		if ((lport == ep->lp) &&
		    (sid == 0 || sid == ep->sid) &&
		    (did == 0 || did == ep->did)) {
			fc_exch_hold(ep);
			spin_unlock_bh(&pool->lock);

			fc_exch_reset(ep);

			fc_exch_release(ep);
			spin_lock_bh(&pool->lock);

			
			goto restart;
		}
	}
	spin_unlock_bh(&pool->lock);
}


void fc_exch_mgr_reset(struct fc_lport *lport, u32 sid, u32 did)
{
	struct fc_exch_mgr_anchor *ema;
	unsigned int cpu;

	list_for_each_entry(ema, &lport->ema_list, ema_list) {
		for_each_possible_cpu(cpu)
			fc_exch_pool_reset(lport,
					   per_cpu_ptr(ema->mp->pool, cpu),
					   sid, did);
	}
}
EXPORT_SYMBOL(fc_exch_mgr_reset);


static void fc_exch_els_rec(struct fc_seq *sp, struct fc_frame *rfp)
{
	struct fc_frame *fp;
	struct fc_exch *ep;
	struct fc_exch_mgr *em;
	struct fc_els_rec *rp;
	struct fc_els_rec_acc *acc;
	enum fc_els_rjt_reason reason = ELS_RJT_LOGIC;
	enum fc_els_rjt_explan explan;
	u32 sid;
	u16 rxid;
	u16 oxid;

	rp = fc_frame_payload_get(rfp, sizeof(*rp));
	explan = ELS_EXPL_INV_LEN;
	if (!rp)
		goto reject;
	sid = ntoh24(rp->rec_s_id);
	rxid = ntohs(rp->rec_rx_id);
	oxid = ntohs(rp->rec_ox_id);

	
	em = fc_seq_exch(sp)->em;
	ep = fc_exch_find(em, oxid);
	explan = ELS_EXPL_OXID_RXID;
	if (ep && ep->oid == sid) {
		if (ep->rxid != FC_XID_UNKNOWN &&
		    rxid != FC_XID_UNKNOWN &&
		    ep->rxid != rxid)
			goto rel;
	} else {
		if (ep)
			fc_exch_release(ep);
		ep = NULL;
		if (rxid != FC_XID_UNKNOWN)
			ep = fc_exch_find(em, rxid);
		if (!ep)
			goto reject;
	}

	fp = fc_frame_alloc(fc_seq_exch(sp)->lp, sizeof(*acc));
	if (!fp) {
		fc_exch_done(sp);
		goto out;
	}
	sp = fc_seq_start_next(sp);
	acc = fc_frame_payload_get(fp, sizeof(*acc));
	memset(acc, 0, sizeof(*acc));
	acc->reca_cmd = ELS_LS_ACC;
	acc->reca_ox_id = rp->rec_ox_id;
	memcpy(acc->reca_ofid, rp->rec_s_id, 3);
	acc->reca_rx_id = htons(ep->rxid);
	if (ep->sid == ep->oid)
		hton24(acc->reca_rfid, ep->did);
	else
		hton24(acc->reca_rfid, ep->sid);
	acc->reca_fc4value = htonl(ep->seq.rec_data);
	acc->reca_e_stat = htonl(ep->esb_stat & (ESB_ST_RESP |
						 ESB_ST_SEQ_INIT |
						 ESB_ST_COMPLETE));
	sp = fc_seq_start_next(sp);
	fc_seq_send_last(sp, fp, FC_RCTL_ELS_REP, FC_TYPE_ELS);
out:
	fc_exch_release(ep);
	fc_frame_free(rfp);
	return;

rel:
	fc_exch_release(ep);
reject:
	fc_seq_ls_rjt(sp, reason, explan);
	fc_frame_free(rfp);
}


static void fc_exch_rrq_resp(struct fc_seq *sp, struct fc_frame *fp, void *arg)
{
	struct fc_exch *aborted_ep = arg;
	unsigned int op;

	if (IS_ERR(fp)) {
		int err = PTR_ERR(fp);

		if (err == -FC_EX_CLOSED || err == -FC_EX_TIMEOUT)
			goto cleanup;
		FC_EXCH_DBG(aborted_ep, "Cannot process RRQ, "
			    "frame error %d\n", err);
		return;
	}

	op = fc_frame_payload_op(fp);
	fc_frame_free(fp);

	switch (op) {
	case ELS_LS_RJT:
		FC_EXCH_DBG(aborted_ep, "LS_RJT for RRQ");
		
	case ELS_LS_ACC:
		goto cleanup;
	default:
		FC_EXCH_DBG(aborted_ep, "unexpected response op %x "
			    "for RRQ", op);
		return;
	}

cleanup:
	fc_exch_done(&aborted_ep->seq);
	
	fc_exch_release(aborted_ep);
}


static void fc_exch_rrq(struct fc_exch *ep)
{
	struct fc_lport *lp;
	struct fc_els_rrq *rrq;
	struct fc_frame *fp;
	u32 did;

	lp = ep->lp;

	fp = fc_frame_alloc(lp, sizeof(*rrq));
	if (!fp)
		goto retry;

	rrq = fc_frame_payload_get(fp, sizeof(*rrq));
	memset(rrq, 0, sizeof(*rrq));
	rrq->rrq_cmd = ELS_RRQ;
	hton24(rrq->rrq_s_id, ep->sid);
	rrq->rrq_ox_id = htons(ep->oxid);
	rrq->rrq_rx_id = htons(ep->rxid);

	did = ep->did;
	if (ep->esb_stat & ESB_ST_RESP)
		did = ep->sid;

	fc_fill_fc_hdr(fp, FC_RCTL_ELS_REQ, did,
		       fc_host_port_id(lp->host), FC_TYPE_ELS,
		       FC_FC_FIRST_SEQ | FC_FC_END_SEQ | FC_FC_SEQ_INIT, 0);

	if (fc_exch_seq_send(lp, fp, fc_exch_rrq_resp, NULL, ep, lp->e_d_tov))
		return;

retry:
	spin_lock_bh(&ep->ex_lock);
	if (ep->state & (FC_EX_RST_CLEANUP | FC_EX_DONE)) {
		spin_unlock_bh(&ep->ex_lock);
		
		fc_exch_release(ep);
		return;
	}
	ep->esb_stat |= ESB_ST_REC_QUAL;
	fc_exch_timer_set_locked(ep, ep->r_a_tov);
	spin_unlock_bh(&ep->ex_lock);
}



static void fc_exch_els_rrq(struct fc_seq *sp, struct fc_frame *fp)
{
	struct fc_exch *ep;		
	struct fc_els_rrq *rp;
	u32 sid;
	u16 xid;
	enum fc_els_rjt_explan explan;

	rp = fc_frame_payload_get(fp, sizeof(*rp));
	explan = ELS_EXPL_INV_LEN;
	if (!rp)
		goto reject;

	
	ep = fc_seq_exch(sp);
	sid = ntoh24(rp->rrq_s_id);		
	xid = ep->did == sid ? ntohs(rp->rrq_ox_id) : ntohs(rp->rrq_rx_id);
	ep = fc_exch_find(ep->em, xid);

	explan = ELS_EXPL_OXID_RXID;
	if (!ep)
		goto reject;
	spin_lock_bh(&ep->ex_lock);
	if (ep->oxid != ntohs(rp->rrq_ox_id))
		goto unlock_reject;
	if (ep->rxid != ntohs(rp->rrq_rx_id) &&
	    ep->rxid != FC_XID_UNKNOWN)
		goto unlock_reject;
	explan = ELS_EXPL_SID;
	if (ep->sid != sid)
		goto unlock_reject;

	
	if (ep->esb_stat & ESB_ST_REC_QUAL) {
		ep->esb_stat &= ~ESB_ST_REC_QUAL;
		atomic_dec(&ep->ex_refcnt);	
	}
	if (ep->esb_stat & ESB_ST_COMPLETE) {
		if (cancel_delayed_work(&ep->timeout_work))
			atomic_dec(&ep->ex_refcnt);	
	}

	spin_unlock_bh(&ep->ex_lock);

	
	fc_seq_ls_acc(sp);
	fc_frame_free(fp);
	return;

unlock_reject:
	spin_unlock_bh(&ep->ex_lock);
	fc_exch_release(ep);	
reject:
	fc_seq_ls_rjt(sp, ELS_RJT_LOGIC, explan);
	fc_frame_free(fp);
}

struct fc_exch_mgr_anchor *fc_exch_mgr_add(struct fc_lport *lport,
					   struct fc_exch_mgr *mp,
					   bool (*match)(struct fc_frame *))
{
	struct fc_exch_mgr_anchor *ema;

	ema = kmalloc(sizeof(*ema), GFP_ATOMIC);
	if (!ema)
		return ema;

	ema->mp = mp;
	ema->match = match;
	
	list_add_tail(&ema->ema_list, &lport->ema_list);
	kref_get(&mp->kref);
	return ema;
}
EXPORT_SYMBOL(fc_exch_mgr_add);

static void fc_exch_mgr_destroy(struct kref *kref)
{
	struct fc_exch_mgr *mp = container_of(kref, struct fc_exch_mgr, kref);

	mempool_destroy(mp->ep_pool);
	free_percpu(mp->pool);
	kfree(mp);
}

void fc_exch_mgr_del(struct fc_exch_mgr_anchor *ema)
{
	
	list_del(&ema->ema_list);
	kref_put(&ema->mp->kref, fc_exch_mgr_destroy);
	kfree(ema);
}
EXPORT_SYMBOL(fc_exch_mgr_del);

struct fc_exch_mgr *fc_exch_mgr_alloc(struct fc_lport *lp,
				      enum fc_class class,
				      u16 min_xid, u16 max_xid,
				      bool (*match)(struct fc_frame *))
{
	struct fc_exch_mgr *mp;
	u16 pool_exch_range;
	size_t pool_size;
	unsigned int cpu;
	struct fc_exch_pool *pool;

	if (max_xid <= min_xid || max_xid == FC_XID_UNKNOWN ||
	    (min_xid & fc_cpu_mask) != 0) {
		FC_LPORT_DBG(lp, "Invalid min_xid 0x:%x and max_xid 0x:%x\n",
			     min_xid, max_xid);
		return NULL;
	}

	
	mp = kzalloc(sizeof(struct fc_exch_mgr), GFP_ATOMIC);
	if (!mp)
		return NULL;

	mp->class = class;
	
	mp->min_xid = min_xid;
	mp->max_xid = max_xid;

	mp->ep_pool = mempool_create_slab_pool(2, fc_em_cachep);
	if (!mp->ep_pool)
		goto free_mp;

	
	pool_exch_range = (mp->max_xid - mp->min_xid + 1) / (fc_cpu_mask + 1);
	mp->pool_max_index = pool_exch_range - 1;

	
	pool_size = sizeof(*pool) + pool_exch_range * sizeof(struct fc_exch *);
	mp->pool = __alloc_percpu(pool_size, __alignof__(struct fc_exch_pool));
	if (!mp->pool)
		goto free_mempool;
	for_each_possible_cpu(cpu) {
		pool = per_cpu_ptr(mp->pool, cpu);
		spin_lock_init(&pool->lock);
		INIT_LIST_HEAD(&pool->ex_list);
	}

	kref_init(&mp->kref);
	if (!fc_exch_mgr_add(lp, mp, match)) {
		free_percpu(mp->pool);
		goto free_mempool;
	}

	
	kref_put(&mp->kref, fc_exch_mgr_destroy);
	return mp;

free_mempool:
	mempool_destroy(mp->ep_pool);
free_mp:
	kfree(mp);
	return NULL;
}
EXPORT_SYMBOL(fc_exch_mgr_alloc);

void fc_exch_mgr_free(struct fc_lport *lport)
{
	struct fc_exch_mgr_anchor *ema, *next;

	list_for_each_entry_safe(ema, next, &lport->ema_list, ema_list)
		fc_exch_mgr_del(ema);
}
EXPORT_SYMBOL(fc_exch_mgr_free);


struct fc_seq *fc_exch_seq_send(struct fc_lport *lp,
				struct fc_frame *fp,
				void (*resp)(struct fc_seq *,
					     struct fc_frame *fp,
					     void *arg),
				void (*destructor)(struct fc_seq *, void *),
				void *arg, u32 timer_msec)
{
	struct fc_exch *ep;
	struct fc_seq *sp = NULL;
	struct fc_frame_header *fh;
	int rc = 1;

	ep = fc_exch_alloc(lp, fp);
	if (!ep) {
		fc_frame_free(fp);
		return NULL;
	}
	ep->esb_stat |= ESB_ST_SEQ_INIT;
	fh = fc_frame_header_get(fp);
	fc_exch_set_addr(ep, ntoh24(fh->fh_s_id), ntoh24(fh->fh_d_id));
	ep->resp = resp;
	ep->destructor = destructor;
	ep->arg = arg;
	ep->r_a_tov = FC_DEF_R_A_TOV;
	ep->lp = lp;
	sp = &ep->seq;

	ep->fh_type = fh->fh_type; 
	ep->f_ctl = ntoh24(fh->fh_f_ctl);
	fc_exch_setup_hdr(ep, fp, ep->f_ctl);
	sp->cnt++;

	if (ep->xid <= lp->lro_xid)
		fc_fcp_ddp_setup(fr_fsp(fp), ep->xid);

	if (unlikely(lp->tt.frame_send(lp, fp)))
		goto err;

	if (timer_msec)
		fc_exch_timer_set_locked(ep, timer_msec);
	ep->f_ctl &= ~FC_FC_FIRST_SEQ;	

	if (ep->f_ctl & FC_FC_SEQ_INIT)
		ep->esb_stat &= ~ESB_ST_SEQ_INIT;
	spin_unlock_bh(&ep->ex_lock);
	return sp;
err:
	rc = fc_exch_done_locked(ep);
	spin_unlock_bh(&ep->ex_lock);
	if (!rc)
		fc_exch_delete(ep);
	return NULL;
}
EXPORT_SYMBOL(fc_exch_seq_send);


void fc_exch_recv(struct fc_lport *lp, struct fc_frame *fp)
{
	struct fc_frame_header *fh = fc_frame_header_get(fp);
	struct fc_exch_mgr_anchor *ema;
	u32 f_ctl, found = 0;
	u16 oxid;

	
	if (!lp || lp->state == LPORT_ST_DISABLED) {
		FC_LPORT_DBG(lp, "Receiving frames for an lport that "
			     "has not been initialized correctly\n");
		fc_frame_free(fp);
		return;
	}

	f_ctl = ntoh24(fh->fh_f_ctl);
	oxid = ntohs(fh->fh_ox_id);
	if (f_ctl & FC_FC_EX_CTX) {
		list_for_each_entry(ema, &lp->ema_list, ema_list) {
			if ((oxid >= ema->mp->min_xid) &&
			    (oxid <= ema->mp->max_xid)) {
				found = 1;
				break;
			}
		}

		if (!found) {
			FC_LPORT_DBG(lp, "Received response for out "
				     "of range oxid:%hx\n", oxid);
			fc_frame_free(fp);
			return;
		}
	} else
		ema = list_entry(lp->ema_list.prev, typeof(*ema), ema_list);

	
	switch (fr_eof(fp)) {
	case FC_EOF_T:
		if (f_ctl & FC_FC_END_SEQ)
			skb_trim(fp_skb(fp), fr_len(fp) - FC_FC_FILL(f_ctl));
		
	case FC_EOF_N:
		if (fh->fh_type == FC_TYPE_BLS)
			fc_exch_recv_bls(ema->mp, fp);
		else if ((f_ctl & (FC_FC_EX_CTX | FC_FC_SEQ_CTX)) ==
			 FC_FC_EX_CTX)
			fc_exch_recv_seq_resp(ema->mp, fp);
		else if (f_ctl & FC_FC_SEQ_CTX)
			fc_exch_recv_resp(ema->mp, fp);
		else
			fc_exch_recv_req(lp, ema->mp, fp);
		break;
	default:
		FC_LPORT_DBG(lp, "dropping invalid frame (eof %x)", fr_eof(fp));
		fc_frame_free(fp);
	}
}
EXPORT_SYMBOL(fc_exch_recv);

int fc_exch_init(struct fc_lport *lp)
{
	if (!lp->tt.seq_start_next)
		lp->tt.seq_start_next = fc_seq_start_next;

	if (!lp->tt.exch_seq_send)
		lp->tt.exch_seq_send = fc_exch_seq_send;

	if (!lp->tt.seq_send)
		lp->tt.seq_send = fc_seq_send;

	if (!lp->tt.seq_els_rsp_send)
		lp->tt.seq_els_rsp_send = fc_seq_els_rsp_send;

	if (!lp->tt.exch_done)
		lp->tt.exch_done = fc_exch_done;

	if (!lp->tt.exch_mgr_reset)
		lp->tt.exch_mgr_reset = fc_exch_mgr_reset;

	if (!lp->tt.seq_exch_abort)
		lp->tt.seq_exch_abort = fc_seq_exch_abort;

	
	fc_cpu_mask = 1;
	fc_cpu_order = 0;
	while (fc_cpu_mask < nr_cpu_ids) {
		fc_cpu_mask <<= 1;
		fc_cpu_order++;
	}
	fc_cpu_mask--;

	return 0;
}
EXPORT_SYMBOL(fc_exch_init);

int fc_setup_exch_mgr(void)
{
	fc_em_cachep = kmem_cache_create("libfc_em", sizeof(struct fc_exch),
					 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!fc_em_cachep)
		return -ENOMEM;
	return 0;
}

void fc_destroy_exch_mgr(void)
{
	kmem_cache_destroy(fc_em_cachep);
}
