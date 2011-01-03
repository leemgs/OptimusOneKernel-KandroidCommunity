



#ifndef __PCIFW_H__
#define __PCIFW_H__

#pragma pack(1)

struct pnp_hdr_s{
  u32	signature;	
  u8	rev;		
  u8 	len;		
  u16  off;		
  u8	rsvd;		
  u8	cksum;		
  u32	pnp_dev_id;	
  u16  mfstr;		
  u16	prstr;		
  u8	devtype[3];	
  u8	devind;		
  u16	bcventr;	
  u16	rsvd2;		
  u16  sriv;		
};

struct pci_3_0_ds_s{
 u32	sig;   		
 u16	vendid;		
 u16	devid;		
 u16	devlistoff;	
 u16	len;		
 u8	rev;		
 u8	clcode[3];	
 u16	imglen;		
 u16	coderev;	
 u8	codetype;	
 u8	indr;		
 u16	mrtimglen;	
 u16	cuoff;		
 u16	dmtfclp;	
};

struct pci_optrom_hdr_s{
 u16	sig;		
 u8	len;		
 u8	inivec[3];	
 u8	rsvd[16];	
 u16	verptr;		
 u16	pcids;		
 u16	pnphdr;		
};

#pragma pack()

#endif
