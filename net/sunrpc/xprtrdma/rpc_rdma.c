



#include "xprt_rdma.h"

#include <linux/highmem.h>

#ifdef RPC_DEBUG
# define RPCDBG_FACILITY	RPCDBG_TRANS
#endif

enum rpcrdma_chunktype {
	rpcrdma_noch = 0,
	rpcrdma_readch,
	rpcrdma_areadch,
	rpcrdma_writech,
	rpcrdma_replych
};

#ifdef RPC_DEBUG
static const char transfertypes[][12] = {
	"pure inline",	
	" read chunk",	
	"*read chunk",	
	"write chunk",	
	"reply chunk"	
};
#endif



static int
rpcrdma_convert_iovs(struct xdr_buf *xdrbuf, unsigned int pos,
	enum rpcrdma_chunktype type, struct rpcrdma_mr_seg *seg, int nsegs)
{
	int len, n = 0, p;

	if (pos == 0 && xdrbuf->head[0].iov_len) {
		seg[n].mr_page = NULL;
		seg[n].mr_offset = xdrbuf->head[0].iov_base;
		seg[n].mr_len = xdrbuf->head[0].iov_len;
		++n;
	}

	if (xdrbuf->page_len && (xdrbuf->pages[0] != NULL)) {
		if (n == nsegs)
			return 0;
		seg[n].mr_page = xdrbuf->pages[0];
		seg[n].mr_offset = (void *)(unsigned long) xdrbuf->page_base;
		seg[n].mr_len = min_t(u32,
			PAGE_SIZE - xdrbuf->page_base, xdrbuf->page_len);
		len = xdrbuf->page_len - seg[n].mr_len;
		++n;
		p = 1;
		while (len > 0) {
			if (n == nsegs)
				return 0;
			seg[n].mr_page = xdrbuf->pages[p];
			seg[n].mr_offset = NULL;
			seg[n].mr_len = min_t(u32, PAGE_SIZE, len);
			len -= seg[n].mr_len;
			++n;
			++p;
		}
	}

	if (xdrbuf->tail[0].iov_len) {
		
		if (xdrbuf->tail[0].iov_len < 4 && xprt_rdma_pad_optimize)
			return n;
		if (n == nsegs)
			return 0;
		seg[n].mr_page = NULL;
		seg[n].mr_offset = xdrbuf->tail[0].iov_base;
		seg[n].mr_len = xdrbuf->tail[0].iov_len;
		++n;
	}

	return n;
}



static unsigned int
rpcrdma_create_chunks(struct rpc_rqst *rqst, struct xdr_buf *target,
		struct rpcrdma_msg *headerp, enum rpcrdma_chunktype type)
{
	struct rpcrdma_req *req = rpcr_to_rdmar(rqst);
	struct rpcrdma_xprt *r_xprt = rpcx_to_rdmax(rqst->rq_task->tk_xprt);
	int nsegs, nchunks = 0;
	unsigned int pos;
	struct rpcrdma_mr_seg *seg = req->rl_segments;
	struct rpcrdma_read_chunk *cur_rchunk = NULL;
	struct rpcrdma_write_array *warray = NULL;
	struct rpcrdma_write_chunk *cur_wchunk = NULL;
	__be32 *iptr = headerp->rm_body.rm_chunks;

	if (type == rpcrdma_readch || type == rpcrdma_areadch) {
		
		cur_rchunk = (struct rpcrdma_read_chunk *) iptr;
	} else {
		
		*iptr++ = xdr_zero;	
		if (type == rpcrdma_replych)
			*iptr++ = xdr_zero;	
		warray = (struct rpcrdma_write_array *) iptr;
		cur_wchunk = (struct rpcrdma_write_chunk *) (warray + 1);
	}

	if (type == rpcrdma_replych || type == rpcrdma_areadch)
		pos = 0;
	else
		pos = target->head[0].iov_len;

	nsegs = rpcrdma_convert_iovs(target, pos, type, seg, RPCRDMA_MAX_SEGS);
	if (nsegs == 0)
		return 0;

	do {
		
		int n = rpcrdma_register_external(seg, nsegs,
						cur_wchunk != NULL, r_xprt);
		if (n <= 0)
			goto out;
		if (cur_rchunk) {	
			cur_rchunk->rc_discrim = xdr_one;
			
			cur_rchunk->rc_position = htonl(pos);
			cur_rchunk->rc_target.rs_handle = htonl(seg->mr_rkey);
			cur_rchunk->rc_target.rs_length = htonl(seg->mr_len);
			xdr_encode_hyper(
					(__be32 *)&cur_rchunk->rc_target.rs_offset,
					seg->mr_base);
			dprintk("RPC:       %s: read chunk "
				"elem %d@0x%llx:0x%x pos %u (%s)\n", __func__,
				seg->mr_len, (unsigned long long)seg->mr_base,
				seg->mr_rkey, pos, n < nsegs ? "more" : "last");
			cur_rchunk++;
			r_xprt->rx_stats.read_chunk_count++;
		} else {		
			cur_wchunk->wc_target.rs_handle = htonl(seg->mr_rkey);
			cur_wchunk->wc_target.rs_length = htonl(seg->mr_len);
			xdr_encode_hyper(
					(__be32 *)&cur_wchunk->wc_target.rs_offset,
					seg->mr_base);
			dprintk("RPC:       %s: %s chunk "
				"elem %d@0x%llx:0x%x (%s)\n", __func__,
				(type == rpcrdma_replych) ? "reply" : "write",
				seg->mr_len, (unsigned long long)seg->mr_base,
				seg->mr_rkey, n < nsegs ? "more" : "last");
			cur_wchunk++;
			if (type == rpcrdma_replych)
				r_xprt->rx_stats.reply_chunk_count++;
			else
				r_xprt->rx_stats.write_chunk_count++;
			r_xprt->rx_stats.total_rdma_request += seg->mr_len;
		}
		nchunks++;
		seg   += n;
		nsegs -= n;
	} while (nsegs);

	
	req->rl_nchunks = nchunks;

	BUG_ON(nchunks == 0);

	
	if (cur_rchunk) {
		iptr = (__be32 *) cur_rchunk;
		*iptr++ = xdr_zero;	
		*iptr++ = xdr_zero;	
		*iptr++ = xdr_zero;	
	} else {
		warray->wc_discrim = xdr_one;
		warray->wc_nchunks = htonl(nchunks);
		iptr = (__be32 *) cur_wchunk;
		if (type == rpcrdma_writech) {
			*iptr++ = xdr_zero; 
			*iptr++ = xdr_zero; 
		}
	}

	
	return (unsigned char *)iptr - (unsigned char *)headerp;

out:
	for (pos = 0; nchunks--;)
		pos += rpcrdma_deregister_external(
				&req->rl_segments[pos], r_xprt, NULL);
	return 0;
}


static int
rpcrdma_inline_pullup(struct rpc_rqst *rqst, int pad)
{
	int i, npages, curlen;
	int copy_len;
	unsigned char *srcp, *destp;
	struct rpcrdma_xprt *r_xprt = rpcx_to_rdmax(rqst->rq_xprt);

	destp = rqst->rq_svec[0].iov_base;
	curlen = rqst->rq_svec[0].iov_len;
	destp += curlen;
	
	pad -= (curlen + 36);
	if (pad < 0 || rqst->rq_slen - curlen < RPCRDMA_INLINE_PAD_THRESH)
		pad = 0;	

	dprintk("RPC:       %s: pad %d destp 0x%p len %d hdrlen %d\n",
		__func__, pad, destp, rqst->rq_slen, curlen);

	copy_len = rqst->rq_snd_buf.page_len;

	if (rqst->rq_snd_buf.tail[0].iov_len) {
		curlen = rqst->rq_snd_buf.tail[0].iov_len;
		if (destp + copy_len != rqst->rq_snd_buf.tail[0].iov_base) {
			memmove(destp + copy_len,
				rqst->rq_snd_buf.tail[0].iov_base, curlen);
			r_xprt->rx_stats.pullup_copy_count += curlen;
		}
		dprintk("RPC:       %s: tail destp 0x%p len %d\n",
			__func__, destp + copy_len, curlen);
		rqst->rq_svec[0].iov_len += curlen;
	}

	r_xprt->rx_stats.pullup_copy_count += copy_len;
	npages = PAGE_ALIGN(rqst->rq_snd_buf.page_base+copy_len) >> PAGE_SHIFT;
	for (i = 0; copy_len && i < npages; i++) {
		if (i == 0)
			curlen = PAGE_SIZE - rqst->rq_snd_buf.page_base;
		else
			curlen = PAGE_SIZE;
		if (curlen > copy_len)
			curlen = copy_len;
		dprintk("RPC:       %s: page %d destp 0x%p len %d curlen %d\n",
			__func__, i, destp, copy_len, curlen);
		srcp = kmap_atomic(rqst->rq_snd_buf.pages[i],
					KM_SKB_SUNRPC_DATA);
		if (i == 0)
			memcpy(destp, srcp+rqst->rq_snd_buf.page_base, curlen);
		else
			memcpy(destp, srcp, curlen);
		kunmap_atomic(srcp, KM_SKB_SUNRPC_DATA);
		rqst->rq_svec[0].iov_len += curlen;
		destp += curlen;
		copy_len -= curlen;
	}
	
	return pad;
}



int
rpcrdma_marshal_req(struct rpc_rqst *rqst)
{
	struct rpc_xprt *xprt = rqst->rq_task->tk_xprt;
	struct rpcrdma_xprt *r_xprt = rpcx_to_rdmax(xprt);
	struct rpcrdma_req *req = rpcr_to_rdmar(rqst);
	char *base;
	size_t hdrlen, rpclen, padlen;
	enum rpcrdma_chunktype rtype, wtype;
	struct rpcrdma_msg *headerp;

	
	base = rqst->rq_svec[0].iov_base;
	rpclen = rqst->rq_svec[0].iov_len;

	
	headerp = (struct rpcrdma_msg *) req->rl_base;
	
	headerp->rm_xid = rqst->rq_xid;
	headerp->rm_vers = xdr_one;
	headerp->rm_credit = htonl(r_xprt->rx_buf.rb_max_requests);
	headerp->rm_type = htonl(RDMA_MSG);

	

	
	if (rqst->rq_rcv_buf.buflen <= RPCRDMA_INLINE_READ_THRESHOLD(rqst))
		wtype = rpcrdma_noch;
	else if (rqst->rq_rcv_buf.page_len == 0)
		wtype = rpcrdma_replych;
	else if (rqst->rq_rcv_buf.flags & XDRBUF_READ)
		wtype = rpcrdma_writech;
	else
		wtype = rpcrdma_replych;

	
	if (rqst->rq_snd_buf.len <= RPCRDMA_INLINE_WRITE_THRESHOLD(rqst))
		rtype = rpcrdma_noch;
	else if (rqst->rq_snd_buf.page_len == 0)
		rtype = rpcrdma_areadch;
	else
		rtype = rpcrdma_readch;

	
	if (rtype != rpcrdma_noch && wtype == rpcrdma_replych)
		wtype = rpcrdma_noch;
	BUG_ON(rtype != rpcrdma_noch && wtype != rpcrdma_noch);

	if (r_xprt->rx_ia.ri_memreg_strategy == RPCRDMA_BOUNCEBUFFERS &&
	    (rtype != rpcrdma_noch || wtype != rpcrdma_noch)) {
		
		dprintk("RPC:       %s: too much data (%d/%d) for inline\n",
			__func__, rqst->rq_rcv_buf.len, rqst->rq_snd_buf.len);
		return -1;
	}

	hdrlen = 28; 
	padlen = 0;

	
	if (rtype == rpcrdma_noch) {

		padlen = rpcrdma_inline_pullup(rqst,
						RPCRDMA_INLINE_PAD_VALUE(rqst));

		if (padlen) {
			headerp->rm_type = htonl(RDMA_MSGP);
			headerp->rm_body.rm_padded.rm_align =
				htonl(RPCRDMA_INLINE_PAD_VALUE(rqst));
			headerp->rm_body.rm_padded.rm_thresh =
				htonl(RPCRDMA_INLINE_PAD_THRESH);
			headerp->rm_body.rm_padded.rm_pempty[0] = xdr_zero;
			headerp->rm_body.rm_padded.rm_pempty[1] = xdr_zero;
			headerp->rm_body.rm_padded.rm_pempty[2] = xdr_zero;
			hdrlen += 2 * sizeof(u32); 
			BUG_ON(wtype != rpcrdma_noch);

		} else {
			headerp->rm_body.rm_nochunks.rm_empty[0] = xdr_zero;
			headerp->rm_body.rm_nochunks.rm_empty[1] = xdr_zero;
			headerp->rm_body.rm_nochunks.rm_empty[2] = xdr_zero;
			
			rpclen = rqst->rq_svec[0].iov_len;
			
			if (wtype == rpcrdma_noch &&
			    r_xprt->rx_ia.ri_memreg_strategy > RPCRDMA_REGISTER)
				wtype = rpcrdma_replych;
		}
	}

	
	if (rtype != rpcrdma_noch) {
		hdrlen = rpcrdma_create_chunks(rqst,
					&rqst->rq_snd_buf, headerp, rtype);
		wtype = rtype;	

	} else if (wtype != rpcrdma_noch) {
		hdrlen = rpcrdma_create_chunks(rqst,
					&rqst->rq_rcv_buf, headerp, wtype);
	}

	if (hdrlen == 0)
		return -1;

	dprintk("RPC:       %s: %s: hdrlen %zd rpclen %zd padlen %zd"
		" headerp 0x%p base 0x%p lkey 0x%x\n",
		__func__, transfertypes[wtype], hdrlen, rpclen, padlen,
		headerp, base, req->rl_iov.lkey);

	
	req->rl_send_iov[0].addr = req->rl_iov.addr;
	req->rl_send_iov[0].length = hdrlen;
	req->rl_send_iov[0].lkey = req->rl_iov.lkey;

	req->rl_send_iov[1].addr = req->rl_iov.addr + (base - req->rl_base);
	req->rl_send_iov[1].length = rpclen;
	req->rl_send_iov[1].lkey = req->rl_iov.lkey;

	req->rl_niovs = 2;

	if (padlen) {
		struct rpcrdma_ep *ep = &r_xprt->rx_ep;

		req->rl_send_iov[2].addr = ep->rep_pad.addr;
		req->rl_send_iov[2].length = padlen;
		req->rl_send_iov[2].lkey = ep->rep_pad.lkey;

		req->rl_send_iov[3].addr = req->rl_send_iov[1].addr + rpclen;
		req->rl_send_iov[3].length = rqst->rq_slen - rpclen;
		req->rl_send_iov[3].lkey = req->rl_iov.lkey;

		req->rl_niovs = 4;
	}

	return 0;
}


static int
rpcrdma_count_chunks(struct rpcrdma_rep *rep, unsigned int max, int wrchunk, __be32 **iptrp)
{
	unsigned int i, total_len;
	struct rpcrdma_write_chunk *cur_wchunk;

	i = ntohl(**iptrp);	
	if (i > max)
		return -1;
	cur_wchunk = (struct rpcrdma_write_chunk *) (*iptrp + 1);
	total_len = 0;
	while (i--) {
		struct rpcrdma_segment *seg = &cur_wchunk->wc_target;
		ifdebug(FACILITY) {
			u64 off;
			xdr_decode_hyper((__be32 *)&seg->rs_offset, &off);
			dprintk("RPC:       %s: chunk %d@0x%llx:0x%x\n",
				__func__,
				ntohl(seg->rs_length),
				(unsigned long long)off,
				ntohl(seg->rs_handle));
		}
		total_len += ntohl(seg->rs_length);
		++cur_wchunk;
	}
	
	if (wrchunk) {
		__be32 *w = (__be32 *) cur_wchunk;
		if (*w++ != xdr_zero)
			return -1;
		cur_wchunk = (struct rpcrdma_write_chunk *) w;
	}
	if ((char *) cur_wchunk > rep->rr_base + rep->rr_len)
		return -1;

	*iptrp = (__be32 *) cur_wchunk;
	return total_len;
}


static void
rpcrdma_inline_fixup(struct rpc_rqst *rqst, char *srcp, int copy_len, int pad)
{
	int i, npages, curlen, olen;
	char *destp;

	curlen = rqst->rq_rcv_buf.head[0].iov_len;
	if (curlen > copy_len) {	
		curlen = copy_len;
		rqst->rq_rcv_buf.head[0].iov_len = curlen;
	}

	dprintk("RPC:       %s: srcp 0x%p len %d hdrlen %d\n",
		__func__, srcp, copy_len, curlen);

	
	rqst->rq_rcv_buf.head[0].iov_base = srcp;
	srcp += curlen;
	copy_len -= curlen;

	olen = copy_len;
	i = 0;
	rpcx_to_rdmax(rqst->rq_xprt)->rx_stats.fixup_copy_count += olen;
	if (copy_len && rqst->rq_rcv_buf.page_len) {
		npages = PAGE_ALIGN(rqst->rq_rcv_buf.page_base +
			rqst->rq_rcv_buf.page_len) >> PAGE_SHIFT;
		for (; i < npages; i++) {
			if (i == 0)
				curlen = PAGE_SIZE - rqst->rq_rcv_buf.page_base;
			else
				curlen = PAGE_SIZE;
			if (curlen > copy_len)
				curlen = copy_len;
			dprintk("RPC:       %s: page %d"
				" srcp 0x%p len %d curlen %d\n",
				__func__, i, srcp, copy_len, curlen);
			destp = kmap_atomic(rqst->rq_rcv_buf.pages[i],
						KM_SKB_SUNRPC_DATA);
			if (i == 0)
				memcpy(destp + rqst->rq_rcv_buf.page_base,
						srcp, curlen);
			else
				memcpy(destp, srcp, curlen);
			flush_dcache_page(rqst->rq_rcv_buf.pages[i]);
			kunmap_atomic(destp, KM_SKB_SUNRPC_DATA);
			srcp += curlen;
			copy_len -= curlen;
			if (copy_len == 0)
				break;
		}
		rqst->rq_rcv_buf.page_len = olen - copy_len;
	} else
		rqst->rq_rcv_buf.page_len = 0;

	if (copy_len && rqst->rq_rcv_buf.tail[0].iov_len) {
		curlen = copy_len;
		if (curlen > rqst->rq_rcv_buf.tail[0].iov_len)
			curlen = rqst->rq_rcv_buf.tail[0].iov_len;
		if (rqst->rq_rcv_buf.tail[0].iov_base != srcp)
			memmove(rqst->rq_rcv_buf.tail[0].iov_base, srcp, curlen);
		dprintk("RPC:       %s: tail srcp 0x%p len %d curlen %d\n",
			__func__, srcp, copy_len, curlen);
		rqst->rq_rcv_buf.tail[0].iov_len = curlen;
		copy_len -= curlen; ++i;
	} else
		rqst->rq_rcv_buf.tail[0].iov_len = 0;

	if (pad) {
		
		unsigned char *p = rqst->rq_rcv_buf.tail[0].iov_base;
		while (pad--)
			p[rqst->rq_rcv_buf.tail[0].iov_len++] = 0;
	}

	if (copy_len)
		dprintk("RPC:       %s: %d bytes in"
			" %d extra segments (%d lost)\n",
			__func__, olen, i, copy_len);

	
	rqst->rq_private_buf = rqst->rq_rcv_buf;
}


void
rpcrdma_conn_func(struct rpcrdma_ep *ep)
{
	struct rpc_xprt *xprt = ep->rep_xprt;

	spin_lock_bh(&xprt->transport_lock);
	if (++xprt->connect_cookie == 0)	
		++xprt->connect_cookie;
	if (ep->rep_connected > 0) {
		if (!xprt_test_and_set_connected(xprt))
			xprt_wake_pending_tasks(xprt, 0);
	} else {
		if (xprt_test_and_clear_connected(xprt))
			xprt_wake_pending_tasks(xprt, -ENOTCONN);
	}
	spin_unlock_bh(&xprt->transport_lock);
}


static void
rpcrdma_unbind_func(struct rpcrdma_rep *rep)
{
	wake_up(&rep->rr_unbind);
}


void
rpcrdma_reply_handler(struct rpcrdma_rep *rep)
{
	struct rpcrdma_msg *headerp;
	struct rpcrdma_req *req;
	struct rpc_rqst *rqst;
	struct rpc_xprt *xprt = rep->rr_xprt;
	struct rpcrdma_xprt *r_xprt = rpcx_to_rdmax(xprt);
	__be32 *iptr;
	int i, rdmalen, status;

	
	if (rep->rr_len == ~0U) {
		rpcrdma_recv_buffer_put(rep);
		if (r_xprt->rx_ep.rep_connected == 1) {
			r_xprt->rx_ep.rep_connected = -EIO;
			rpcrdma_conn_func(&r_xprt->rx_ep);
		}
		return;
	}
	if (rep->rr_len < 28) {
		dprintk("RPC:       %s: short/invalid reply\n", __func__);
		goto repost;
	}
	headerp = (struct rpcrdma_msg *) rep->rr_base;
	if (headerp->rm_vers != xdr_one) {
		dprintk("RPC:       %s: invalid version %d\n",
			__func__, ntohl(headerp->rm_vers));
		goto repost;
	}

	
	spin_lock(&xprt->transport_lock);
	rqst = xprt_lookup_rqst(xprt, headerp->rm_xid);
	if (rqst == NULL) {
		spin_unlock(&xprt->transport_lock);
		dprintk("RPC:       %s: reply 0x%p failed "
			"to match any request xid 0x%08x len %d\n",
			__func__, rep, headerp->rm_xid, rep->rr_len);
repost:
		r_xprt->rx_stats.bad_reply_count++;
		rep->rr_func = rpcrdma_reply_handler;
		if (rpcrdma_ep_post_recv(&r_xprt->rx_ia, &r_xprt->rx_ep, rep))
			rpcrdma_recv_buffer_put(rep);

		return;
	}

	
	req = rpcr_to_rdmar(rqst);

	dprintk("RPC:       %s: reply 0x%p completes request 0x%p\n"
		"                   RPC request 0x%p xid 0x%08x\n",
			__func__, rep, req, rqst, headerp->rm_xid);

	BUG_ON(!req || req->rl_reply);

	
	req->rl_reply = rep;

	
	
	switch (headerp->rm_type) {
	case htonl(RDMA_MSG):
		
		
		
		if (headerp->rm_body.rm_chunks[0] != xdr_zero ||
		    (headerp->rm_body.rm_chunks[1] == xdr_zero &&
		     headerp->rm_body.rm_chunks[2] != xdr_zero) ||
		    (headerp->rm_body.rm_chunks[1] != xdr_zero &&
		     req->rl_nchunks == 0))
			goto badheader;
		if (headerp->rm_body.rm_chunks[1] != xdr_zero) {
			
			
			iptr = &headerp->rm_body.rm_chunks[2];
			rdmalen = rpcrdma_count_chunks(rep,
						req->rl_nchunks, 1, &iptr);
			
			if (rdmalen < 0 || *iptr++ != xdr_zero)
				goto badheader;
			rep->rr_len -=
			    ((unsigned char *)iptr - (unsigned char *)headerp);
			status = rep->rr_len + rdmalen;
			r_xprt->rx_stats.total_rdma_reply += rdmalen;
			
			if (rdmalen &= 3) {
				rdmalen = 4 - rdmalen;
				status += rdmalen;
			}
		} else {
			
			rdmalen = 0;
			iptr = (__be32 *)((unsigned char *)headerp + 28);
			rep->rr_len -= 28; 
			status = rep->rr_len;
		}
		
		rpcrdma_inline_fixup(rqst, (char *)iptr, rep->rr_len, rdmalen);
		break;

	case htonl(RDMA_NOMSG):
		
		if (headerp->rm_body.rm_chunks[0] != xdr_zero ||
		    headerp->rm_body.rm_chunks[1] != xdr_zero ||
		    headerp->rm_body.rm_chunks[2] != xdr_one ||
		    req->rl_nchunks == 0)
			goto badheader;
		iptr = (__be32 *)((unsigned char *)headerp + 28);
		rdmalen = rpcrdma_count_chunks(rep, req->rl_nchunks, 0, &iptr);
		if (rdmalen < 0)
			goto badheader;
		r_xprt->rx_stats.total_rdma_reply += rdmalen;
		
		status = rdmalen;
		break;

badheader:
	default:
		dprintk("%s: invalid rpcrdma reply header (type %d):"
				" chunks[012] == %d %d %d"
				" expected chunks <= %d\n",
				__func__, ntohl(headerp->rm_type),
				headerp->rm_body.rm_chunks[0],
				headerp->rm_body.rm_chunks[1],
				headerp->rm_body.rm_chunks[2],
				req->rl_nchunks);
		status = -EIO;
		r_xprt->rx_stats.bad_reply_count++;
		break;
	}

	
	
	if (req->rl_nchunks) switch (r_xprt->rx_ia.ri_memreg_strategy) {
	case RPCRDMA_MEMWINDOWS:
		for (i = 0; req->rl_nchunks-- > 1;)
			i += rpcrdma_deregister_external(
				&req->rl_segments[i], r_xprt, NULL);
		
		rep->rr_func = rpcrdma_unbind_func;
		(void) rpcrdma_deregister_external(&req->rl_segments[i],
						   r_xprt, rep);
		break;
	case RPCRDMA_MEMWINDOWS_ASYNC:
		for (i = 0; req->rl_nchunks--;)
			i += rpcrdma_deregister_external(&req->rl_segments[i],
							 r_xprt, NULL);
		break;
	default:
		break;
	}

	dprintk("RPC:       %s: xprt_complete_rqst(0x%p, 0x%p, %d)\n",
			__func__, xprt, rqst, status);
	xprt_complete_rqst(rqst->rq_task, status);
	spin_unlock(&xprt->transport_lock);
}
