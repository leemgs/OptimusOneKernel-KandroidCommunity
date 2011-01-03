

#ifndef __SH_FLCTL_H__
#define __SH_FLCTL_H__

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>


#define FLCMNCR(f)		(f->reg + 0x0)
#define FLCMDCR(f)		(f->reg + 0x4)
#define FLCMCDR(f)		(f->reg + 0x8)
#define FLADR(f)		(f->reg + 0xC)
#define FLADR2(f)		(f->reg + 0x3C)
#define FLDATAR(f)		(f->reg + 0x10)
#define FLDTCNTR(f)		(f->reg + 0x14)
#define FLINTDMACR(f)		(f->reg + 0x18)
#define FLBSYTMR(f)		(f->reg + 0x1C)
#define FLBSYCNT(f)		(f->reg + 0x20)
#define FLDTFIFO(f)		(f->reg + 0x24)
#define FLECFIFO(f)		(f->reg + 0x28)
#define FLTRCR(f)		(f->reg + 0x2C)
#define	FL4ECCRESULT0(f)	(f->reg + 0x80)
#define	FL4ECCRESULT1(f)	(f->reg + 0x84)
#define	FL4ECCRESULT2(f)	(f->reg + 0x88)
#define	FL4ECCRESULT3(f)	(f->reg + 0x8C)
#define	FL4ECCCR(f)		(f->reg + 0x90)
#define	FL4ECCCNT(f)		(f->reg + 0x94)
#define	FLERRADR(f)		(f->reg + 0x98)


#define ECCPOS2		(0x1 << 25)
#define _4ECCCNTEN	(0x1 << 24)
#define _4ECCEN		(0x1 << 23)
#define _4ECCCORRECT	(0x1 << 22)
#define SNAND_E		(0x1 << 18)	
#define QTSEL_E		(0x1 << 17)
#define ENDIAN		(0x1 << 16)	
#define FCKSEL_E	(0x1 << 15)
#define ECCPOS_00	(0x00 << 12)
#define ECCPOS_01	(0x01 << 12)
#define ECCPOS_02	(0x02 << 12)
#define ACM_SACCES_MODE	(0x01 << 10)
#define NANWF_E		(0x1 << 9)
#define SE_D		(0x1 << 8)	
#define	CE1_ENABLE	(0x1 << 4)	
#define	CE0_ENABLE	(0x1 << 3)	
#define	TYPESEL_SET	(0x1 << 0)


#define ADRCNT2_E	(0x1 << 31)	
#define ADRMD_E		(0x1 << 26)	
#define CDSRC_E		(0x1 << 25)	
#define DOSR_E		(0x1 << 24)	
#define SELRW		(0x1 << 21)	
#define DOADR_E		(0x1 << 20)	
#define ADRCNT_1	(0x00 << 18)	
#define ADRCNT_2	(0x01 << 18)	
#define ADRCNT_3	(0x02 << 18)	
#define ADRCNT_4	(0x03 << 18)	
#define DOCMD2_E	(0x1 << 17)	
#define DOCMD1_E	(0x1 << 16)	


#define TRSTRT		(0x1 << 0)	
#define TREND		(0x1 << 1)	


#define	_4ECCFA		(0x1 << 2)	
#define	_4ECCEND	(0x1 << 1)	
#define	_4ECCEXST	(0x1 << 0)	

#define INIT_FL4ECCRESULT_VAL	0x03FF03FF
#define LOOP_TIMEOUT_MAX	0x00010000

#define mtd_to_flctl(mtd)	container_of(mtd, struct sh_flctl, mtd)

struct sh_flctl {
	struct mtd_info		mtd;
	struct nand_chip	chip;
	void __iomem		*reg;

	uint8_t	done_buff[2048 + 64];	
	int	read_bytes;
	int	index;
	int	seqin_column;		
	int	seqin_page_addr;	
	uint32_t seqin_read_cmd;		
	int	erase1_page_addr;	
	uint32_t erase_ADRCNT;		
	uint32_t rw_ADRCNT;	

	int	hwecc_cant_correct[4];

	unsigned page_size:1;	
	unsigned hwecc:1;	
};

struct sh_flctl_platform_data {
	struct mtd_partition	*parts;
	int			nr_parts;
	unsigned long		flcmncr_val;

	unsigned has_hwecc:1;
};

#endif	
