#ifndef _VTX_H
#define _VTX_H





#define VTXIOCGETINFO	_IOR  (0x81,  1, vtx_info_t)
#define VTXIOCCLRPAGE	_IOW  (0x81,  2, vtx_pagereq_t)
#define VTXIOCCLRFOUND	_IOW  (0x81,  3, vtx_pagereq_t)
#define VTXIOCPAGEREQ	_IOW  (0x81,  4, vtx_pagereq_t)
#define VTXIOCGETSTAT	_IOW  (0x81,  5, vtx_pagereq_t)
#define VTXIOCGETPAGE	_IOW  (0x81,  6, vtx_pagereq_t)
#define VTXIOCSTOPDAU	_IOW  (0x81,  7, vtx_pagereq_t)
#define VTXIOCPUTPAGE	_IO   (0x81,  8)
#define VTXIOCSETDISP	_IO   (0x81,  9)
#define VTXIOCPUTSTAT	_IO   (0x81, 10)
#define VTXIOCCLRCACHE	_IO   (0x81, 11)
#define VTXIOCSETVIRT	_IOW  (0x81, 12, long)


#define VTXIOCGETINFO_OLD  0x7101  
#define VTXIOCCLRPAGE_OLD  0x7102  
#define VTXIOCCLRFOUND_OLD 0x7103  
#define VTXIOCPAGEREQ_OLD  0x7104  
#define VTXIOCGETSTAT_OLD  0x7105  
#define VTXIOCGETPAGE_OLD  0x7106  
#define VTXIOCSTOPDAU_OLD  0x7107  
#define VTXIOCPUTPAGE_OLD  0x7108  
#define VTXIOCSETDISP_OLD  0x7109  
#define VTXIOCPUTSTAT_OLD  0x710a  
#define VTXIOCCLRCACHE_OLD 0x710b  
#define VTXIOCSETVIRT_OLD  0x710c  



#define SAA5243 0
#define SAA5246 1
#define SAA5249 2
#define SAA5248 3
#define XSTV5346 4

typedef struct {
	int version_major, version_minor;	
						
	int numpages;				
	int cct_type;				
}
vtx_info_t;




#define MIN_UNIT   (1<<0)
#define MIN_TEN    (1<<1)
#define HR_UNIT    (1<<2)
#define HR_TEN     (1<<3)
#define PG_UNIT    (1<<4)
#define PG_TEN     (1<<5)
#define PG_HUND    (1<<6)
#define PGMASK_MAX (1<<7)
#define PGMASK_PAGE (PG_HUND | PG_TEN | PG_UNIT)
#define PGMASK_HOUR (HR_TEN | HR_UNIT)
#define PGMASK_MINUTE (MIN_TEN | MIN_UNIT)

typedef struct
{
	int page;	
	int hour;	
	int minute;	
	int pagemask;	
	int pgbuf;	
	int start;	
	int end;	
	void __user *buffer;	
}
vtx_pagereq_t;




#define VTX_PAGESIZE (40 * 24)
#define VTX_VIRTUALSIZE (40 * 49)

typedef struct
{
	int pagenum;			
	int hour;			
	int minute;			
	int charset;			
	unsigned delete : 1;		
	unsigned headline : 1;		
	unsigned subtitle : 1;		
	unsigned supp_header : 1;	
	unsigned update : 1;		
	unsigned inter_seq : 1;		
	unsigned dis_disp : 1;		
	unsigned serial : 1;		
	unsigned notfound : 1;		
	unsigned pblf : 1;		
	unsigned hamming : 1;		
}
vtx_pageinfo_t;

#endif 
