

#ifndef _LINUX_SUNRPC_RPC_RDMA_H
#define _LINUX_SUNRPC_RPC_RDMA_H

struct rpcrdma_segment {
	__be32 rs_handle;	
	__be32 rs_length;	
	__be64 rs_offset;	
};


struct rpcrdma_read_chunk {
	__be32 rc_discrim;	
	__be32 rc_position;	
	struct rpcrdma_segment rc_target;
};


struct rpcrdma_write_chunk {
	struct rpcrdma_segment wc_target;
};


struct rpcrdma_write_array {
	__be32 wc_discrim;	
	__be32 wc_nchunks;	
	struct rpcrdma_write_chunk wc_array[0];
};

struct rpcrdma_msg {
	__be32 rm_xid;	
	__be32 rm_vers;	
	__be32 rm_credit;	
	__be32 rm_type;	
	union {

		struct {			
			__be32 rm_empty[3];	
		} rm_nochunks;

		struct {			
			__be32 rm_align;	
			__be32 rm_thresh;	
			__be32 rm_pempty[3];	
		} rm_padded;

		__be32 rm_chunks[0];	

	} rm_body;
};

#define RPCRDMA_HDRLEN_MIN	28

enum rpcrdma_errcode {
	ERR_VERS = 1,
	ERR_CHUNK = 2
};

struct rpcrdma_err_vers {
	uint32_t rdma_vers_low;	
	uint32_t rdma_vers_high;
};

enum rpcrdma_proc {
	RDMA_MSG = 0,		
	RDMA_NOMSG = 1,		
	RDMA_MSGP = 2,		
	RDMA_DONE = 3,		
	RDMA_ERROR = 4		
};

#endif				
