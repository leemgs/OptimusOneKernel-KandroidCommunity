

#ifndef _TIPC_MSG_H
#define _TIPC_MSG_H

#include "core.h"

#define TIPC_VERSION              2

#define SHORT_H_SIZE              24	
#define DIR_MSG_H_SIZE            32	
#define LONG_H_SIZE               40	
#define MCAST_H_SIZE              44	
#define INT_H_SIZE                40	
#define MIN_H_SIZE                24	
#define MAX_H_SIZE                60	

#define MAX_MSG_SIZE (MAX_H_SIZE + TIPC_MAX_USER_MSG_SIZE)





static inline void msg_set_word(struct tipc_msg *m, u32 w, u32 val)
{
	m->hdr[w] = htonl(val);
}

static inline void msg_set_bits(struct tipc_msg *m, u32 w,
				u32 pos, u32 mask, u32 val)
{
	val = (val & mask) << pos;
	mask = mask << pos;
	m->hdr[w] &= ~htonl(mask);
	m->hdr[w] |= htonl(val);
}

static inline void msg_swap_words(struct tipc_msg *msg, u32 a, u32 b)
{
	u32 temp = msg->hdr[a];

	msg->hdr[a] = msg->hdr[b];
	msg->hdr[b] = temp;
}



static inline u32 msg_version(struct tipc_msg *m)
{
	return msg_bits(m, 0, 29, 7);
}

static inline void msg_set_version(struct tipc_msg *m)
{
	msg_set_bits(m, 0, 29, 7, TIPC_VERSION);
}

static inline u32 msg_user(struct tipc_msg *m)
{
	return msg_bits(m, 0, 25, 0xf);
}

static inline u32 msg_isdata(struct tipc_msg *m)
{
	return (msg_user(m) <= TIPC_CRITICAL_IMPORTANCE);
}

static inline void msg_set_user(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 0, 25, 0xf, n);
}

static inline void msg_set_importance(struct tipc_msg *m, u32 i)
{
	msg_set_user(m, i);
}

static inline void msg_set_hdr_sz(struct tipc_msg *m,u32 n)
{
	msg_set_bits(m, 0, 21, 0xf, n>>2);
}

static inline int msg_non_seq(struct tipc_msg *m)
{
	return msg_bits(m, 0, 20, 1);
}

static inline void msg_set_non_seq(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 0, 20, 1, n);
}

static inline int msg_dest_droppable(struct tipc_msg *m)
{
	return msg_bits(m, 0, 19, 1);
}

static inline void msg_set_dest_droppable(struct tipc_msg *m, u32 d)
{
	msg_set_bits(m, 0, 19, 1, d);
}

static inline int msg_src_droppable(struct tipc_msg *m)
{
	return msg_bits(m, 0, 18, 1);
}

static inline void msg_set_src_droppable(struct tipc_msg *m, u32 d)
{
	msg_set_bits(m, 0, 18, 1, d);
}

static inline void msg_set_size(struct tipc_msg *m, u32 sz)
{
	m->hdr[0] = htonl((msg_word(m, 0) & ~0x1ffff) | sz);
}




static inline void msg_set_type(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 1, 29, 0x7, n);
}

static inline void msg_set_errcode(struct tipc_msg *m, u32 err)
{
	msg_set_bits(m, 1, 25, 0xf, err);
}

static inline u32 msg_reroute_cnt(struct tipc_msg *m)
{
	return msg_bits(m, 1, 21, 0xf);
}

static inline void msg_incr_reroute_cnt(struct tipc_msg *m)
{
	msg_set_bits(m, 1, 21, 0xf, msg_reroute_cnt(m) + 1);
}

static inline void msg_reset_reroute_cnt(struct tipc_msg *m)
{
	msg_set_bits(m, 1, 21, 0xf, 0);
}

static inline u32 msg_lookup_scope(struct tipc_msg *m)
{
	return msg_bits(m, 1, 19, 0x3);
}

static inline void msg_set_lookup_scope(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 1, 19, 0x3, n);
}

static inline u32 msg_bcast_ack(struct tipc_msg *m)
{
	return msg_bits(m, 1, 0, 0xffff);
}

static inline void msg_set_bcast_ack(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 1, 0, 0xffff, n);
}




static inline u32 msg_ack(struct tipc_msg *m)
{
	return msg_bits(m, 2, 16, 0xffff);
}

static inline void msg_set_ack(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 2, 16, 0xffff, n);
}

static inline u32 msg_seqno(struct tipc_msg *m)
{
	return msg_bits(m, 2, 0, 0xffff);
}

static inline void msg_set_seqno(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 2, 0, 0xffff, n);
}



static inline u32 msg_destnode_cache(struct tipc_msg *m)
{
	return m->hdr[2];
}

static inline void msg_set_destnode_cache(struct tipc_msg *m, u32 dnode)
{
	m->hdr[2] = dnode;
}




static inline void msg_set_prevnode(struct tipc_msg *m, u32 a)
{
	msg_set_word(m, 3, a);
}

static inline void msg_set_origport(struct tipc_msg *m, u32 p)
{
	msg_set_word(m, 4, p);
}

static inline void msg_set_destport(struct tipc_msg *m, u32 p)
{
	msg_set_word(m, 5, p);
}

static inline void msg_set_mc_netid(struct tipc_msg *m, u32 p)
{
	msg_set_word(m, 5, p);
}

static inline void msg_set_orignode(struct tipc_msg *m, u32 a)
{
	msg_set_word(m, 6, a);
}

static inline void msg_set_destnode(struct tipc_msg *m, u32 a)
{
	msg_set_word(m, 7, a);
}

static inline int msg_is_dest(struct tipc_msg *m, u32 d)
{
	return(msg_short(m) || (msg_destnode(m) == d));
}

static inline u32 msg_routed(struct tipc_msg *m)
{
	if (likely(msg_short(m)))
		return 0;
	return(msg_destnode(m) ^ msg_orignode(m)) >> 11;
}

static inline void msg_set_nametype(struct tipc_msg *m, u32 n)
{
	msg_set_word(m, 8, n);
}

static inline u32 msg_transp_seqno(struct tipc_msg *m)
{
	return msg_word(m, 8);
}

static inline void msg_set_timestamp(struct tipc_msg *m, u32 n)
{
	msg_set_word(m, 8, n);
}

static inline u32 msg_timestamp(struct tipc_msg *m)
{
	return msg_word(m, 8);
}

static inline void msg_set_transp_seqno(struct tipc_msg *m, u32 n)
{
	msg_set_word(m, 8, n);
}

static inline void msg_set_namelower(struct tipc_msg *m, u32 n)
{
	msg_set_word(m, 9, n);
}

static inline void msg_set_nameinst(struct tipc_msg *m, u32 n)
{
	msg_set_namelower(m, n);
}

static inline void msg_set_nameupper(struct tipc_msg *m, u32 n)
{
	msg_set_word(m, 10, n);
}

static inline struct tipc_msg *msg_get_wrapped(struct tipc_msg *m)
{
	return (struct tipc_msg *)msg_data(m);
}






#define  BCAST_PROTOCOL       5
#define  MSG_BUNDLER          6
#define  LINK_PROTOCOL        7
#define  CONN_MANAGER         8
#define  ROUTE_DISTRIBUTOR    9
#define  CHANGEOVER_PROTOCOL  10
#define  NAME_DISTRIBUTOR     11
#define  MSG_FRAGMENTER       12
#define  LINK_CONFIG          13
#define  DSC_H_SIZE           40



#define CONN_PROBE        0
#define CONN_PROBE_REPLY  1
#define CONN_ACK          2



#define PUBLICATION       0
#define WITHDRAWAL        1




static inline u32 msg_seq_gap(struct tipc_msg *m)
{
	return msg_bits(m, 1, 16, 0x1fff);
}

static inline void msg_set_seq_gap(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 1, 16, 0x1fff, n);
}

static inline u32 msg_req_links(struct tipc_msg *m)
{
	return msg_bits(m, 1, 16, 0xfff);
}

static inline void msg_set_req_links(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 1, 16, 0xfff, n);
}




static inline u32 msg_dest_domain(struct tipc_msg *m)
{
	return msg_word(m, 2);
}

static inline void msg_set_dest_domain(struct tipc_msg *m, u32 n)
{
	msg_set_word(m, 2, n);
}

static inline u32 msg_bcgap_after(struct tipc_msg *m)
{
	return msg_bits(m, 2, 16, 0xffff);
}

static inline void msg_set_bcgap_after(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 2, 16, 0xffff, n);
}

static inline u32 msg_bcgap_to(struct tipc_msg *m)
{
	return msg_bits(m, 2, 0, 0xffff);
}

static inline void msg_set_bcgap_to(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 2, 0, 0xffff, n);
}




static inline u32 msg_last_bcast(struct tipc_msg *m)
{
	return msg_bits(m, 4, 16, 0xffff);
}

static inline void msg_set_last_bcast(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 4, 16, 0xffff, n);
}


static inline u32 msg_fragm_no(struct tipc_msg *m)
{
	return msg_bits(m, 4, 16, 0xffff);
}

static inline void msg_set_fragm_no(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 4, 16, 0xffff, n);
}


static inline u32 msg_next_sent(struct tipc_msg *m)
{
	return msg_bits(m, 4, 0, 0xffff);
}

static inline void msg_set_next_sent(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 4, 0, 0xffff, n);
}


static inline u32 msg_long_msgno(struct tipc_msg *m)
{
	return msg_bits(m, 4, 0, 0xffff);
}

static inline void msg_set_long_msgno(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 4, 0, 0xffff, n);
}

static inline u32 msg_bc_netid(struct tipc_msg *m)
{
	return msg_word(m, 4);
}

static inline void msg_set_bc_netid(struct tipc_msg *m, u32 id)
{
	msg_set_word(m, 4, id);
}

static inline u32 msg_link_selector(struct tipc_msg *m)
{
	return msg_bits(m, 4, 0, 1);
}

static inline void msg_set_link_selector(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 4, 0, 1, (n & 1));
}



static inline u32 msg_session(struct tipc_msg *m)
{
	return msg_bits(m, 5, 16, 0xffff);
}

static inline void msg_set_session(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 5, 16, 0xffff, n);
}

static inline u32 msg_probe(struct tipc_msg *m)
{
	return msg_bits(m, 5, 0, 1);
}

static inline void msg_set_probe(struct tipc_msg *m, u32 val)
{
	msg_set_bits(m, 5, 0, 1, (val & 1));
}

static inline char msg_net_plane(struct tipc_msg *m)
{
	return msg_bits(m, 5, 1, 7) + 'A';
}

static inline void msg_set_net_plane(struct tipc_msg *m, char n)
{
	msg_set_bits(m, 5, 1, 7, (n - 'A'));
}

static inline u32 msg_linkprio(struct tipc_msg *m)
{
	return msg_bits(m, 5, 4, 0x1f);
}

static inline void msg_set_linkprio(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 5, 4, 0x1f, n);
}

static inline u32 msg_bearer_id(struct tipc_msg *m)
{
	return msg_bits(m, 5, 9, 0x7);
}

static inline void msg_set_bearer_id(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 5, 9, 0x7, n);
}

static inline u32 msg_redundant_link(struct tipc_msg *m)
{
	return msg_bits(m, 5, 12, 0x1);
}

static inline void msg_set_redundant_link(struct tipc_msg *m)
{
	msg_set_bits(m, 5, 12, 0x1, 1);
}

static inline void msg_clear_redundant_link(struct tipc_msg *m)
{
	msg_set_bits(m, 5, 12, 0x1, 0);
}




static inline u32 msg_msgcnt(struct tipc_msg *m)
{
	return msg_bits(m, 9, 16, 0xffff);
}

static inline void msg_set_msgcnt(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 9, 16, 0xffff, n);
}

static inline u32 msg_bcast_tag(struct tipc_msg *m)
{
	return msg_bits(m, 9, 16, 0xffff);
}

static inline void msg_set_bcast_tag(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 9, 16, 0xffff, n);
}

static inline u32 msg_max_pkt(struct tipc_msg *m)
{
	return (msg_bits(m, 9, 16, 0xffff) * 4);
}

static inline void msg_set_max_pkt(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 9, 16, 0xffff, (n / 4));
}

static inline u32 msg_link_tolerance(struct tipc_msg *m)
{
	return msg_bits(m, 9, 0, 0xffff);
}

static inline void msg_set_link_tolerance(struct tipc_msg *m, u32 n)
{
	msg_set_bits(m, 9, 0, 0xffff, n);
}




static inline u32 msg_remote_node(struct tipc_msg *m)
{
	return msg_word(m, msg_hdr_sz(m)/4);
}

static inline void msg_set_remote_node(struct tipc_msg *m, u32 a)
{
	msg_set_word(m, msg_hdr_sz(m)/4, a);
}

static inline void msg_set_dataoctet(struct tipc_msg *m, u32 pos)
{
	msg_data(m)[pos + 4] = 1;
}



#define FIRST_FRAGMENT     0
#define FRAGMENT           1
#define LAST_FRAGMENT      2



#define STATE_MSG       0
#define RESET_MSG       1
#define ACTIVATE_MSG    2


#define DUPLICATE_MSG    0
#define ORIGINAL_MSG     1


#define EXT_ROUTING_TABLE    0
#define LOCAL_ROUTING_TABLE  1
#define SLAVE_ROUTING_TABLE  2
#define ROUTE_ADDITION       3
#define ROUTE_REMOVAL        4



#define DSC_REQ_MSG          0
#define DSC_RESP_MSG         1

static inline u32 msg_tot_importance(struct tipc_msg *m)
{
	if (likely(msg_isdata(m))) {
		if (likely(msg_orignode(m) == tipc_own_addr))
			return msg_importance(m);
		return msg_importance(m) + 4;
	}
	if ((msg_user(m) == MSG_FRAGMENTER)  &&
	    (msg_type(m) == FIRST_FRAGMENT))
		return msg_importance(msg_get_wrapped(m));
	return msg_importance(m);
}


static inline void msg_init(struct tipc_msg *m, u32 user, u32 type,
			    u32 hsize, u32 destnode)
{
	memset(m, 0, hsize);
	msg_set_version(m);
	msg_set_user(m, user);
	msg_set_hdr_sz(m, hsize);
	msg_set_size(m, hsize);
	msg_set_prevnode(m, tipc_own_addr);
	msg_set_type(m, type);
	if (!msg_short(m)) {
		msg_set_orignode(m, tipc_own_addr);
		msg_set_destnode(m, destnode);
	}
}



static inline int msg_calc_data_size(struct iovec const *msg_sect, u32 num_sect)
{
	int dsz = 0;
	int i;

	for (i = 0; i < num_sect; i++)
		dsz += msg_sect[i].iov_len;
	return dsz;
}



static inline int msg_build(struct tipc_msg *hdr,
			    struct iovec const *msg_sect, u32 num_sect,
			    int max_size, int usrmem, struct sk_buff** buf)
{
	int dsz, sz, hsz, pos, res, cnt;

	dsz = msg_calc_data_size(msg_sect, num_sect);
	if (unlikely(dsz > TIPC_MAX_USER_MSG_SIZE)) {
		*buf = NULL;
		return -EINVAL;
	}

	pos = hsz = msg_hdr_sz(hdr);
	sz = hsz + dsz;
	msg_set_size(hdr, sz);
	if (unlikely(sz > max_size)) {
		*buf = NULL;
		return dsz;
	}

	*buf = buf_acquire(sz);
	if (!(*buf))
		return -ENOMEM;
	skb_copy_to_linear_data(*buf, hdr, hsz);
	for (res = 1, cnt = 0; res && (cnt < num_sect); cnt++) {
		if (likely(usrmem))
			res = !copy_from_user((*buf)->data + pos,
					      msg_sect[cnt].iov_base,
					      msg_sect[cnt].iov_len);
		else
			skb_copy_to_linear_data_offset(*buf, pos,
						       msg_sect[cnt].iov_base,
						       msg_sect[cnt].iov_len);
		pos += msg_sect[cnt].iov_len;
	}
	if (likely(res))
		return dsz;

	buf_discard(*buf);
	*buf = NULL;
	return -EFAULT;
}

static inline void msg_set_media_addr(struct tipc_msg *m, struct tipc_media_addr *a)
{
	memcpy(&((int *)m)[5], a, sizeof(*a));
}

static inline void msg_get_media_addr(struct tipc_msg *m, struct tipc_media_addr *a)
{
	memcpy(a, &((int*)m)[5], sizeof(*a));
}

#endif
