

#include <linux/module.h>
#include <linux/delay.h>
#include "hermes.h"
#include "hermes_dld.h"

#define PFX "hermes_dld: "


#define HERMES_AUX_ENABLE	0x8000	
#define HERMES_AUX_DISABLE	0x4000	
#define HERMES_AUX_ENABLED	0xC000	
#define HERMES_AUX_DISABLED	0x0000	

#define HERMES_AUX_PW0	0xFE01
#define HERMES_AUX_PW1	0xDC23
#define HERMES_AUX_PW2	0xBA45


#define HERMES_PROGRAM_DISABLE             (0x0000 | HERMES_CMD_DOWNLD)
#define HERMES_PROGRAM_ENABLE_VOLATILE     (0x0100 | HERMES_CMD_DOWNLD)
#define HERMES_PROGRAM_ENABLE_NON_VOLATILE (0x0200 | HERMES_CMD_DOWNLD)
#define HERMES_PROGRAM_NON_VOLATILE        (0x0300 | HERMES_CMD_DOWNLD)


#define PDI_END		0x00000000	
#define BLOCK_END	0xFFFFFFFF	
#define TEXT_END	0x1A		


#define MAX_DL_SIZE 1024
#define LIMIT_PROGRAM_SIZE 0




struct dblock {
	__le32 addr;		
	__le16 len;		
	char data[0];		
} __attribute__ ((packed));


struct pdr {
	__le32 id;		
	__le32 addr;		
	__le32 len;		
	char next[0];		
} __attribute__ ((packed));


struct pdi {
	__le16 len;		
	__le16 id;		
	char data[0];		
} __attribute__ ((packed));



static inline u32
dblock_addr(const struct dblock *blk)
{
	return le32_to_cpu(blk->addr);
}

static inline u32
dblock_len(const struct dblock *blk)
{
	return le16_to_cpu(blk->len);
}



static inline u32
pdr_id(const struct pdr *pdr)
{
	return le32_to_cpu(pdr->id);
}

static inline u32
pdr_addr(const struct pdr *pdr)
{
	return le32_to_cpu(pdr->addr);
}

static inline u32
pdr_len(const struct pdr *pdr)
{
	return le32_to_cpu(pdr->len);
}



static inline u32
pdi_id(const struct pdi *pdi)
{
	return le16_to_cpu(pdi->id);
}


static inline u32
pdi_len(const struct pdi *pdi)
{
	return 2 * (le16_to_cpu(pdi->len) - 1);
}



static inline void
hermes_aux_setaddr(hermes_t *hw, u32 addr)
{
	hermes_write_reg(hw, HERMES_AUXPAGE, (u16) (addr >> 7));
	hermes_write_reg(hw, HERMES_AUXOFFSET, (u16) (addr & 0x7F));
}

static inline int
hermes_aux_control(hermes_t *hw, int enabled)
{
	int desired_state = enabled ? HERMES_AUX_ENABLED : HERMES_AUX_DISABLED;
	int action = enabled ? HERMES_AUX_ENABLE : HERMES_AUX_DISABLE;
	int i;

	
	if (hermes_read_reg(hw, HERMES_CONTROL) == desired_state)
		return 0;

	hermes_write_reg(hw, HERMES_PARAM0, HERMES_AUX_PW0);
	hermes_write_reg(hw, HERMES_PARAM1, HERMES_AUX_PW1);
	hermes_write_reg(hw, HERMES_PARAM2, HERMES_AUX_PW2);
	hermes_write_reg(hw, HERMES_CONTROL, action);

	for (i = 0; i < 20; i++) {
		udelay(10);
		if (hermes_read_reg(hw, HERMES_CONTROL) ==
		    desired_state)
			return 0;
	}

	return -EBUSY;
}




static const struct pdr *
hermes_find_pdr(const struct pdr *first_pdr, u32 record_id, const void *end)
{
	const struct pdr *pdr = first_pdr;

	end -= sizeof(struct pdr);

	while (((void *) pdr <= end) &&
	       (pdr_id(pdr) != PDI_END)) {
		
		if (pdr_len(pdr) < 2)
			return NULL;

		
		if (pdr_id(pdr) == record_id)
			return pdr;

		pdr = (struct pdr *) pdr->next;
	}
	return NULL;
}


static const struct pdi *
hermes_find_pdi(const struct pdi *first_pdi, u32 record_id, const void *end)
{
	const struct pdi *pdi = first_pdi;

	end -= sizeof(struct pdi);

	while (((void *) pdi <= end) &&
	       (pdi_id(pdi) != PDI_END)) {

		
		if (pdi_id(pdi) == record_id)
			return pdi;

		pdi = (struct pdi *) &pdi->data[pdi_len(pdi)];
	}
	return NULL;
}


static int
hermes_plug_pdi(hermes_t *hw, const struct pdr *first_pdr,
		const struct pdi *pdi, const void *pdr_end)
{
	const struct pdr *pdr;

	
	pdr = hermes_find_pdr(first_pdr, pdi_id(pdi), pdr_end);

	
	if (!pdr)
		return 0;

	
	if (pdi_len(pdi) != pdr_len(pdr))
		return -EINVAL;

	
	hermes_aux_setaddr(hw, pdr_addr(pdr));
	hermes_write_bytes(hw, HERMES_AUXDATA, pdi->data, pdi_len(pdi));

	return 0;
}


int hermes_read_pda(hermes_t *hw,
		    __le16 *pda,
		    u32 pda_addr,
		    u16 pda_len,
		    int use_eeprom) 
{
	int ret;
	u16 pda_size;
	u16 data_len = pda_len;
	__le16 *data = pda;

	if (use_eeprom) {
		

		
		ret = hermes_docmd_wait(hw, HERMES_CMD_READMIF, 0, NULL);
		if (ret)
			return ret;
	} else {
		
		pda[0] = cpu_to_le16(pda_len - 2);
			
		pda[1] = cpu_to_le16(0x0800); 
		data_len = pda_len - 4;
		data = pda + 2;
	}

	
	ret = hermes_aux_control(hw, 1);
	pr_debug(PFX "AUX enable returned %d\n", ret);
	if (ret)
		return ret;

	
	hermes_aux_setaddr(hw, pda_addr);
	hermes_read_words(hw, HERMES_AUXDATA, data, data_len / 2);

	
	ret = hermes_aux_control(hw, 0);
	pr_debug(PFX "AUX disable returned %d\n", ret);

	
	pda_size = le16_to_cpu(pda[0]);
	pr_debug(PFX "Actual PDA length %d, Max allowed %d\n",
		 pda_size, pda_len);
	if (pda_size > pda_len)
		return -EINVAL;

	return 0;
}


int hermes_apply_pda(hermes_t *hw,
		     const char *first_pdr,
		     const void *pdr_end,
		     const __le16 *pda,
		     const void *pda_end)
{
	int ret;
	const struct pdi *pdi;
	const struct pdr *pdr;

	pdr = (const struct pdr *) first_pdr;
	pda_end -= sizeof(struct pdi);

	
	pdi = (const struct pdi *) (pda + 2);
	while (((void *) pdi <= pda_end) &&
	       (pdi_id(pdi) != PDI_END)) {
		ret = hermes_plug_pdi(hw, pdr, pdi, pdr_end);
		if (ret)
			return ret;

		
		pdi = (const struct pdi *) &pdi->data[pdi_len(pdi)];
	}
	return 0;
}


size_t
hermes_blocks_length(const char *first_block, const void *end)
{
	const struct dblock *blk = (const struct dblock *) first_block;
	int total_len = 0;
	int len;

	end -= sizeof(*blk);

	
	while (((void *) blk <= end) &&
	       (dblock_addr(blk) != BLOCK_END)) {
		len = dblock_len(blk);
		total_len += sizeof(*blk) + len;
		blk = (struct dblock *) &blk->data[len];
	}

	return total_len;
}




int hermesi_program_init(hermes_t *hw, u32 offset)
{
	int err;

	
	
	
	

	
	hermes_write_regn(hw, EVACK, 0xFFFF);

	
	err = hermes_doicmd_wait(hw,
				 0x0100 | HERMES_CMD_INIT,
				 0, 0, 0, NULL);
	if (err)
		return err;

	err = hermes_doicmd_wait(hw,
				 0x0000 | HERMES_CMD_INIT,
				 0, 0, 0, NULL);
	if (err)
		return err;

	err = hermes_aux_control(hw, 1);
	pr_debug(PFX "AUX enable returned %d\n", err);

	if (err)
		return err;

	pr_debug(KERN_DEBUG PFX "Enabling volatile, EP 0x%08x\n", offset);
	err = hermes_doicmd_wait(hw,
				 HERMES_PROGRAM_ENABLE_VOLATILE,
				 offset & 0xFFFFu,
				 offset >> 16,
				 0,
				 NULL);
	pr_debug(PFX "PROGRAM_ENABLE returned %d\n", err);

	return err;
}


int hermesi_program_end(hermes_t *hw)
{
	struct hermes_response resp;
	int rc = 0;
	int err;

	rc = hermes_docmd_wait(hw, HERMES_PROGRAM_DISABLE, 0, &resp);

	pr_debug(PFX "PROGRAM_DISABLE returned %d, "
		 "r0 0x%04x, r1 0x%04x, r2 0x%04x\n",
		 rc, resp.resp0, resp.resp1, resp.resp2);

	if ((rc == 0) &&
	    ((resp.status & HERMES_STATUS_CMDCODE) != HERMES_CMD_DOWNLD))
		rc = -EIO;

	err = hermes_aux_control(hw, 0);
	pr_debug(PFX "AUX disable returned %d\n", err);

	
	hermes_write_regn(hw, EVACK, 0xFFFF);

	
	(void) hermes_doicmd_wait(hw, 0x0000 | HERMES_CMD_INIT,
				  0, 0, 0, NULL);

	return rc ? rc : err;
}


int hermes_program(hermes_t *hw, const char *first_block, const void *end)
{
	const struct dblock *blk;
	u32 blkaddr;
	u32 blklen;
#if LIMIT_PROGRAM_SIZE
	u32 addr;
	u32 len;
#endif

	blk = (const struct dblock *) first_block;

	if ((void *) blk > (end - sizeof(*blk)))
		return -EIO;

	blkaddr = dblock_addr(blk);
	blklen = dblock_len(blk);

	while ((blkaddr != BLOCK_END) &&
	       (((void *) blk + blklen) <= end)) {
		pr_debug(PFX "Programming block of length %d "
			 "to address 0x%08x\n", blklen, blkaddr);

#if !LIMIT_PROGRAM_SIZE
		
		hermes_aux_setaddr(hw, blkaddr);
		hermes_write_bytes(hw, HERMES_AUXDATA, blk->data,
				   blklen);
#else
		len = (blklen < MAX_DL_SIZE) ? blklen : MAX_DL_SIZE;
		addr = blkaddr;

		while (addr < (blkaddr + blklen)) {
			pr_debug(PFX "Programming subblock of length %d "
				 "to address 0x%08x. Data @ %p\n",
				 len, addr, &blk->data[addr - blkaddr]);

			hermes_aux_setaddr(hw, addr);
			hermes_write_bytes(hw, HERMES_AUXDATA,
					   &blk->data[addr - blkaddr],
					   len);

			addr += len;
			len = ((blkaddr + blklen - addr) < MAX_DL_SIZE) ?
				(blkaddr + blklen - addr) : MAX_DL_SIZE;
		}
#endif
		blk = (const struct dblock *) &blk->data[blklen];

		if ((void *) blk > (end - sizeof(*blk)))
			return -EIO;

		blkaddr = dblock_addr(blk);
		blklen = dblock_len(blk);
	}
	return 0;
}




#define DEFINE_DEFAULT_PDR(pid, length, data)				\
static const struct {							\
	__le16 len;							\
	__le16 id;							\
	u8 val[length];							\
} __attribute__ ((packed)) default_pdr_data_##pid = {			\
	cpu_to_le16((sizeof(default_pdr_data_##pid)/			\
				sizeof(__le16)) - 1),			\
	cpu_to_le16(pid),						\
	data								\
}

#define DEFAULT_PDR(pid) default_pdr_data_##pid


DEFINE_DEFAULT_PDR(0x0005, 10, "\x00\x00\x06\x00\x01\x00\x01\x00\x01\x00");


DEFINE_DEFAULT_PDR(0x0108, 4, "\x00\x00\x00\x00");


DEFINE_DEFAULT_PDR(0x0109, 10, "\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00");


DEFINE_DEFAULT_PDR(0x0150, 2, "\x00\x3F");


DEFINE_DEFAULT_PDR(0x0160, 28,
		   "\x00\x00\x00\x00\x00\x00\x00\x00"
		   "\x00\x00\x00\x00\x00\x00\x00\x00"
		   "\x00\x00\x00\x00\x00\x00\x00\x00"
		   "\x00\x00\x00\x00");


DEFINE_DEFAULT_PDR(0x0161, 256,
		   "\x3F\x01\x3F\01\x3F\x01\x3F\x01"
		   "\x3F\x01\x3F\01\x3F\x01\x3F\x01"
		   "\x3F\x01\x3F\01\x3F\x01\x3F\x01"
		   "\x3F\x01\x3F\01\x3F\x01\x3F\x01"
		   "\x3F\x01\x3E\01\x3E\x01\x3D\x01"
		   "\x3D\x01\x3C\01\x3C\x01\x3B\x01"
		   "\x3B\x01\x3A\01\x3A\x01\x39\x01"
		   "\x39\x01\x38\01\x38\x01\x37\x01"
		   "\x37\x01\x36\01\x36\x01\x35\x01"
		   "\x35\x01\x34\01\x34\x01\x33\x01"
		   "\x33\x01\x32\x01\x32\x01\x31\x01"
		   "\x31\x01\x30\x01\x30\x01\x7B\x01"
		   "\x7B\x01\x7A\x01\x7A\x01\x79\x01"
		   "\x79\x01\x78\x01\x78\x01\x77\x01"
		   "\x77\x01\x76\x01\x76\x01\x75\x01"
		   "\x75\x01\x74\x01\x74\x01\x73\x01"
		   "\x73\x01\x72\x01\x72\x01\x71\x01"
		   "\x71\x01\x70\x01\x70\x01\x68\x01"
		   "\x68\x01\x67\x01\x67\x01\x66\x01"
		   "\x66\x01\x65\x01\x65\x01\x57\x01"
		   "\x57\x01\x56\x01\x56\x01\x55\x01"
		   "\x55\x01\x54\x01\x54\x01\x53\x01"
		   "\x53\x01\x52\x01\x52\x01\x51\x01"
		   "\x51\x01\x50\x01\x50\x01\x48\x01"
		   "\x48\x01\x47\x01\x47\x01\x46\x01"
		   "\x46\x01\x45\x01\x45\x01\x44\x01"
		   "\x44\x01\x43\x01\x43\x01\x42\x01"
		   "\x42\x01\x41\x01\x41\x01\x40\x01"
		   "\x40\x01\x40\x01\x40\x01\x40\x01"
		   "\x40\x01\x40\x01\x40\x01\x40\x01"
		   "\x40\x01\x40\x01\x40\x01\x40\x01"
		   "\x40\x01\x40\x01\x40\x01\x40\x01");


int hermes_apply_pda_with_defaults(hermes_t *hw,
				   const char *first_pdr,
				   const void *pdr_end,
				   const __le16 *pda,
				   const void *pda_end)
{
	const struct pdr *pdr = (const struct pdr *) first_pdr;
	const struct pdi *first_pdi = (const struct pdi *) &pda[2];
	const struct pdi *pdi;
	const struct pdi *default_pdi = NULL;
	const struct pdi *outdoor_pdi;
	int record_id;

	pdr_end -= sizeof(struct pdr);

	while (((void *) pdr <= pdr_end) &&
	       (pdr_id(pdr) != PDI_END)) {
		
		if (pdr_len(pdr) < 2)
			break;
		record_id = pdr_id(pdr);

		pdi = hermes_find_pdi(first_pdi, record_id, pda_end);
		if (pdi)
			pr_debug(PFX "Found record 0x%04x at %p\n",
				 record_id, pdi);

		switch (record_id) {
		case 0x110: 
		case 0x120: 
			outdoor_pdi = hermes_find_pdi(first_pdi, record_id + 1,
						      pda_end);
			default_pdi = NULL;
			if (outdoor_pdi) {
				pdi = outdoor_pdi;
				pr_debug(PFX
					 "Using outdoor record 0x%04x at %p\n",
					 record_id + 1, pdi);
			}
			break;
		case 0x5: 
			default_pdi = (struct pdi *) &DEFAULT_PDR(0x0005);
			break;
		case 0x108: 
			default_pdi = (struct pdi *) &DEFAULT_PDR(0x0108);
			break;
		case 0x109: 
			default_pdi = (struct pdi *) &DEFAULT_PDR(0x0109);
			break;
		case 0x150: 
			default_pdi = (struct pdi *) &DEFAULT_PDR(0x0150);
			break;
		case 0x160: 
			default_pdi = (struct pdi *) &DEFAULT_PDR(0x0160);
			break;
		case 0x161: 
			default_pdi = (struct pdi *) &DEFAULT_PDR(0x0161);
			break;
		default:
			default_pdi = NULL;
			break;
		}
		if (!pdi && default_pdi) {
			
			pdi = default_pdi;
			pr_debug(PFX "Using default record 0x%04x at %p\n",
				 record_id, pdi);
		}

		if (pdi) {
			
			if ((pdi_len(pdi) == pdr_len(pdr)) &&
			    ((void *) pdi->data + pdi_len(pdi) < pda_end)) {
				
				hermes_aux_setaddr(hw, pdr_addr(pdr));
				hermes_write_bytes(hw, HERMES_AUXDATA,
						   pdi->data, pdi_len(pdi));
			}
		}

		pdr++;
	}
	return 0;
}
