

#ifndef __LINUX_IF_PPPOPNS_H
#define __LINUX_IF_PPPOPNS_H

#include <linux/socket.h>
#include <linux/types.h>

#define PX_PROTO_OPNS	3

struct sockaddr_pppopns {
	sa_family_t	sa_family;	
	unsigned int	sa_protocol;	
	int		tcp_socket;
	__u16		local;
	__u16		remote;
} __attribute__((packed));

#endif 
