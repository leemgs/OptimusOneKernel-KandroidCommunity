

#ifndef ISCSI_SW_TCP_H
#define ISCSI_SW_TCP_H

#include <scsi/libiscsi.h>
#include <scsi/libiscsi_tcp.h>

struct socket;
struct iscsi_tcp_conn;


struct iscsi_sw_tcp_send {
	struct iscsi_hdr	*hdr;
	struct iscsi_segment	segment;
	struct iscsi_segment	data_segment;
};

struct iscsi_sw_tcp_conn {
	struct iscsi_conn	*iscsi_conn;
	struct socket		*sock;

	struct iscsi_sw_tcp_send out;
	
	void			(*old_data_ready)(struct sock *, int);
	void			(*old_state_change)(struct sock *);
	void			(*old_write_space)(struct sock *);

	
	struct hash_desc	tx_hash;	
	struct hash_desc	rx_hash;	

	
	uint32_t		sendpage_failures_cnt;
	uint32_t		discontiguous_hdr_cnt;

	ssize_t (*sendpage)(struct socket *, struct page *, int, size_t, int);
};

struct iscsi_sw_tcp_hdrbuf {
	struct iscsi_hdr	hdrbuf;
	char			hdrextbuf[ISCSI_MAX_AHS_SIZE +
		                                  ISCSI_DIGEST_SIZE];
};

#endif 
