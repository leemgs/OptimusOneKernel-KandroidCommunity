

#ifndef _ZCRYPT_PCICC_H_
#define _ZCRYPT_PCICC_H_


struct type6_hdr {
	unsigned char reserved1;	
	unsigned char type;		
	unsigned char reserved2[2];	
	unsigned char right[4];		
	unsigned char reserved3[2];	
	unsigned char reserved4[2];	
	unsigned char apfs[4];		
	unsigned int  offset1;		
	unsigned int  offset2;		
	unsigned int  offset3;		
	unsigned int  offset4;		
	unsigned char agent_id[16];	
					
					
					
					
					
					
	unsigned char rqid[2];		
	unsigned char reserved5[2];	
	unsigned char function_code[2];	
	unsigned char reserved6[2];	
	unsigned int  ToCardLen1;	
	unsigned int  ToCardLen2;	
	unsigned int  ToCardLen3;	
	unsigned int  ToCardLen4;	
	unsigned int  FromCardLen1;	
	unsigned int  FromCardLen2;	
	unsigned int  FromCardLen3;	
	unsigned int  FromCardLen4;	
} __attribute__((packed));


struct CPRB {
	unsigned short cprb_len;	
	unsigned char cprb_ver_id;	
	unsigned char pad_000;		
	unsigned char srpi_rtcode[4];	
	unsigned char srpi_verb;	
	unsigned char flags;		
	unsigned char func_id[2];	
	unsigned char checkpoint_flag;	
	unsigned char resv2;		
	unsigned short req_parml;	
					
	unsigned char req_parmp[4];	
	unsigned char req_datal[4];	
					
	unsigned char req_datap[4];	
					
	unsigned short rpl_parml;	
					
	unsigned char pad_001[2];	
	unsigned char rpl_parmp[4];	
	unsigned char rpl_datal[4];	
	unsigned char rpl_datap[4];	
					
	unsigned short ccp_rscode;	
	unsigned short ccp_rtcode;	
	unsigned char repd_parml[2];	
	unsigned char mac_data_len[2];	
	unsigned char repd_datal[4];	
	unsigned char req_pc[2];	
	unsigned char res_origin[8];	
	unsigned char mac_value[8];	
	unsigned char logon_id[8];	
	unsigned char usage_domain[2];	
	unsigned char resv3[18];	
	unsigned short svr_namel;	
	unsigned char svr_name[8];	
} __attribute__((packed));


struct type86_hdr {
	unsigned char reserved1;	
	unsigned char type;		
	unsigned char format;		
	unsigned char reserved2;	
	unsigned char reply_code;	
	unsigned char reserved3[3];	
} __attribute__((packed));

#define TYPE86_RSP_CODE 0x86
#define TYPE86_FMT2	0x02

struct type86_fmt2_ext {
	unsigned char	  reserved[4];	
	unsigned char	  apfs[4];	
	unsigned int	  count1;	
	unsigned int	  offset1;	
	unsigned int	  count2;	
	unsigned int	  offset2;	
	unsigned int	  count3;	
	unsigned int	  offset3;	
	unsigned int	  count4;	
	unsigned int	  offset4;	
} __attribute__((packed));

struct function_and_rules_block {
	unsigned char function_code[2];
	unsigned short ulen;
	unsigned char only_rule[8];
} __attribute__((packed));

int zcrypt_pcicc_init(void);
void zcrypt_pcicc_exit(void);

#endif 
