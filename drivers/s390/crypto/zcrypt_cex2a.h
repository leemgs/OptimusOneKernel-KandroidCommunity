

#ifndef _ZCRYPT_CEX2A_H_
#define _ZCRYPT_CEX2A_H_


struct type50_hdr {
	unsigned char	reserved1;
	unsigned char	msg_type_code;	
	unsigned short	msg_len;
	unsigned char	reserved2;
	unsigned char	ignored;
	unsigned short	reserved3;
} __attribute__((packed));

#define TYPE50_TYPE_CODE	0x50

#define TYPE50_MEB1_FMT		0x0001
#define TYPE50_MEB2_FMT		0x0002
#define TYPE50_CRB1_FMT		0x0011
#define TYPE50_CRB2_FMT		0x0012


struct type50_meb1_msg {
	struct type50_hdr header;
	unsigned short	keyblock_type;	
	unsigned char	reserved[6];
	unsigned char	exponent[128];
	unsigned char	modulus[128];
	unsigned char	message[128];
} __attribute__((packed));


struct type50_meb2_msg {
	struct type50_hdr header;
	unsigned short	keyblock_type;	
	unsigned char	reserved[6];
	unsigned char	exponent[256];
	unsigned char	modulus[256];
	unsigned char	message[256];
} __attribute__((packed));


struct type50_crb1_msg {
	struct type50_hdr header;
	unsigned short	keyblock_type;	
	unsigned char	reserved[6];
	unsigned char	p[64];
	unsigned char	q[64];
	unsigned char	dp[64];
	unsigned char	dq[64];
	unsigned char	u[64];
	unsigned char	message[128];
} __attribute__((packed));


struct type50_crb2_msg {
	struct type50_hdr header;
	unsigned short	keyblock_type;	
	unsigned char	reserved[6];
	unsigned char	p[128];
	unsigned char	q[128];
	unsigned char	dp[128];
	unsigned char	dq[128];
	unsigned char	u[128];
	unsigned char	message[256];
} __attribute__((packed));



#define TYPE80_RSP_CODE 0x80

struct type80_hdr {
	unsigned char	reserved1;
	unsigned char	type;		
	unsigned short	len;
	unsigned char	code;		
	unsigned char	reserved2[3];
	unsigned char	reserved3[8];
} __attribute__((packed));

int zcrypt_cex2a_init(void);
void zcrypt_cex2a_exit(void);

#endif 
