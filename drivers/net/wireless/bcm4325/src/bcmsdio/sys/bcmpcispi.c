

#include <typedefs.h>
#include <bcmutils.h>

#include <sdio.h>		
#include <bcmsdbus.h>		
#include <sdiovar.h>		

#include <pcicfg.h>
#include <bcmsdspi.h>
#include <bcmspi.h>
#include <bcmpcispi.h>		





#define SPIPCI_RREG R_REG
#define SPIPCI_WREG W_REG


#define	SPIPCI_ANDREG(osh, r, v) SPIPCI_WREG(osh, (r), (SPIPCI_RREG(osh, r) & (v)))
#define	SPIPCI_ORREG(osh, r, v)	SPIPCI_WREG(osh, (r), (SPIPCI_RREG(osh, r) | (v)))


int bcmpcispi_dump = 0;		

typedef struct spih_info_ {
	uint		bar0;		
	uint		bar1;		
	osl_t 		*osh;		
	spih_pciregs_t	*pciregs;	
	spih_regs_t	*regs;		
	uint8		rev;		
} spih_info_t;



bool
spi_hw_attach(sdioh_info_t *sd)
{
	osl_t *osh;
	spih_info_t *si;

	sd_trace(("%s: enter\n", __FUNCTION__));

	osh = sd->osh;

	if ((si = (spih_info_t *)MALLOC(osh, sizeof(spih_info_t))) == NULL) {
		sd_err(("%s: out of memory, malloced %d bytes\n", __FUNCTION__, MALLOCED(osh)));
		return FALSE;
	}

	bzero(si, sizeof(spih_info_t));

	sd->controller = si;

	si->osh = sd->osh;
	si->rev = OSL_PCI_READ_CONFIG(sd->osh, PCI_CFG_REV, 4) & 0xFF;

	if (si->rev < 3) {
		sd_err(("Host controller %d not supported, please upgrade to rev >= 3\n", si->rev));
		MFREE(osh, si, sizeof(spih_info_t));
		return (FALSE);
	}

	sd_err(("Attaching to Generic PCI SPI Host Controller Rev %d\n", si->rev));

	
	ASSERT(si->rev >= 3);

	si->bar0 = sd->bar0;

	
	if (si->rev < 10) {
		si->pciregs = (spih_pciregs_t *)spi_reg_map(osh,
		                                              (uintptr)si->bar0,
		                                              sizeof(spih_pciregs_t));
		sd_err(("Mapped PCI Core regs to BAR0 at %p\n", si->pciregs));

		si->bar1 = OSL_PCI_READ_CONFIG(sd->osh, PCI_CFG_BAR1, 4);
		si->regs = (spih_regs_t *)spi_reg_map(osh,
		                                        (uintptr)si->bar1,
		                                        sizeof(spih_regs_t));
		sd_err(("Mapped SPI Controller regs to BAR1 at %p\n", si->regs));
	} else {
		si->regs = (spih_regs_t *)spi_reg_map(osh,
		                                              (uintptr)si->bar0,
		                                              sizeof(spih_regs_t));
		sd_err(("Mapped SPI Controller regs to BAR0 at %p\n", si->regs));
		si->pciregs = NULL;
	}
	
	SPIPCI_WREG(osh, &si->regs->spih_ctrl, 0x000000d1);

	
	SPIPCI_WREG(osh, &si->regs->spih_ext, 0x00000000);

	
	SPIPCI_WREG(osh, &si->regs->spih_gpio_data, SPIH_CS);

	
	
	
	SPIPCI_WREG(osh, &si->regs->spih_gpio_ctrl, (SPIH_CS | SPIH_SLOT_POWER));

	
	while ((SPIPCI_RREG(osh, &si->regs->spih_stat) & SPIH_RFEMPTY) == 0) {
		SPIPCI_RREG(osh, &si->regs->spih_data);
	}

	
	OSL_DELAY(250000);

	
	if (si->rev >= 4) {
		if (SPIPCI_RREG(osh, &si->regs->spih_gpio_data) & SPIH_CARD_DETECT) {
			sd_err(("%s: no card detected in SD slot\n", __FUNCTION__));
			spi_reg_unmap(osh, (uintptr)si->regs, sizeof(spih_regs_t));
			if (si->pciregs) {
				spi_reg_unmap(osh, (uintptr)si->pciregs, sizeof(spih_pciregs_t));
			}
			MFREE(osh, si, sizeof(spih_info_t));
			return FALSE;
		}
	}

	
	SPIPCI_WREG(osh, &si->regs->spih_int_edge, 0x80000000);

	
	SPIPCI_WREG(osh, &si->regs->spih_int_pol, 0x40000004);

	
	if (si->pciregs) {
		SPIPCI_WREG(osh, &si->pciregs->ICR, PCI_INT_PROP_EN);
	}

	sd_trace(("%s: exit\n", __FUNCTION__));
	return TRUE;
}


bool
spi_hw_detach(sdioh_info_t *sd)
{
	spih_info_t *si = (spih_info_t *)sd->controller;
	osl_t *osh = si->osh;
	spih_regs_t *regs = si->regs;
	spih_pciregs_t *pciregs = si->pciregs;

	sd_trace(("%s: enter\n", __FUNCTION__));

	SPIPCI_WREG(osh, &regs->spih_ctrl, 0x00000010);
	SPIPCI_WREG(osh, &regs->spih_gpio_ctrl, 0x00000000);	
	SPIPCI_WREG(osh, &regs->spih_int_mask, 0x00000000);	
	SPIPCI_WREG(osh, &regs->spih_hex_disp, 0x0000DEAF);
	SPIPCI_WREG(osh, &regs->spih_int_edge, 0x00000000);
	SPIPCI_WREG(osh, &regs->spih_int_pol, 0x00000000);
	SPIPCI_WREG(osh, &regs->spih_hex_disp, 0x0000DEAD);

	
	if (si->pciregs) {
		SPIPCI_WREG(osh, &pciregs->ICR, 0x00000000);
		spi_reg_unmap(osh, (uintptr)pciregs, sizeof(spih_pciregs_t));
	}
	spi_reg_unmap(osh, (uintptr)regs, sizeof(spih_regs_t));

	MFREE(osh, si, sizeof(spih_info_t));

	sd->controller = NULL;

	sd_trace(("%s: exit\n", __FUNCTION__));
	return TRUE;
}


static bool
sdspi_switch_clock(sdioh_info_t *sd, bool ext_clk)
{
	spih_info_t *si = (spih_info_t *)sd->controller;
	osl_t *osh = si->osh;
	spih_regs_t *regs = si->regs;

	
	SPIPCI_WREG(osh, &regs->spih_pll_ctrl, ext_clk ? SPIH_EXT_CLK : 0);

	SPINWAIT(((SPIPCI_RREG(osh, &regs->spih_pll_status) & SPIH_PLL_LOCKED)
	          != SPIH_PLL_LOCKED), 1000);
	if ((SPIPCI_RREG(osh, &regs->spih_pll_status) & SPIH_PLL_LOCKED) != SPIH_PLL_LOCKED) {
		sd_err(("%s: timeout waiting for PLL to lock\n", __FUNCTION__));
		return (FALSE);
	}
	return (TRUE);

}


bool
spi_start_clock(sdioh_info_t *sd, uint16 div)
{
	spih_info_t *si = (spih_info_t *)sd->controller;
	osl_t *osh = si->osh;
	spih_regs_t *regs = si->regs;
	uint32 t, espr, disp;
	uint32 disp_xtal_freq;
	bool	ext_clock = FALSE;
	char disp_string[5];

	if (div > 2048) {
		sd_err(("%s: divisor %d too large; using max of 2048\n", __FUNCTION__, div));
		div = 2048;
	} else if (div & (div - 1)) {	
		
		while ((div + 1) & div)
			div |= div >> 1;
		div++;
	}

	
	if (si->rev >= 5) {
		uint32 clk_tick;
		
		if (!sdspi_switch_clock(sd, TRUE)) {
			sd_err(("%s: error switching to external clock\n", __FUNCTION__));
		}

		
		clk_tick = SPIPCI_RREG(osh, &regs->spih_clk_count);
		if (clk_tick == SPIPCI_RREG(osh, &regs->spih_clk_count)) {

			
			if (!sdspi_switch_clock(sd, FALSE)) {
				sd_err(("%s: error switching to external clock\n", __FUNCTION__));
			}
		} else {
			ext_clock = TRUE;
		}
	}

	
	if (div == 0) {
		ext_clock = FALSE;
		div = 2;

		
		if (!sdspi_switch_clock(sd, FALSE)) {
			sd_err(("%s: error switching to external clock\n", __FUNCTION__));
		}

		sd_err(("%s: Ok to hot-swap oscillators.\n", __FUNCTION__));
	}

	
	if (ext_clock == TRUE) {
		uint32 xtal_freq;

		OSL_DELAY(1000);
		xtal_freq = SPIPCI_RREG(osh, &regs->spih_xtal_freq) * 10000;

		sd_info(("%s: Oscillator is %dHz\n", __FUNCTION__, xtal_freq));


		disp_xtal_freq = xtal_freq / 10000;

		
		if ((disp_xtal_freq % 100) > 50) {
			disp_xtal_freq += 100;
		}

		disp_xtal_freq = (disp_xtal_freq / 100) * 100;
	} else {
		sd_err(("%s: no external oscillator installed, using PCI clock.\n", __FUNCTION__));
		disp_xtal_freq = 3333;
	}

	
	sprintf(disp_string, "%04d", disp_xtal_freq / div);

	disp  = (disp_string[0] - '0') << 12;
	disp |= (disp_string[1] - '0') << 8;
	disp |= (disp_string[2] - '0') << 4;
	disp |= (disp_string[3] - '0');

	
	switch (div) {
		case 1:		espr = 0x0; break;
		case 2:		espr = 0x1; break;
		case 4:		espr = 0x2; break;
		case 8:		espr = 0x5; break;
		case 16:	espr = 0x3; break;
		case 32:	espr = 0x4; break;
		case 64:	espr = 0x6; break;
		case 128:	espr = 0x7; break;
		case 256:	espr = 0x8; break;
		case 512:	espr = 0x9; break;
		case 1024:	espr = 0xa; break;
		case 2048:	espr = 0xb; break;
		default:	espr = 0x0; ASSERT(0); break;
	}

	t = SPIPCI_RREG(osh, &regs->spih_ctrl);
	t &= ~3;
	t |= espr & 3;
	SPIPCI_WREG(osh, &regs->spih_ctrl, t);

	t = SPIPCI_RREG(osh, &regs->spih_ext);
	t &= ~3;
	t |= (espr >> 2) & 3;
	SPIPCI_WREG(osh, &regs->spih_ext, t);

	SPIPCI_WREG(osh, &regs->spih_hex_disp, disp);

	
	if (si->rev < 8) {
		
		OSL_DELAY(5000);
	}

	sd_info(("%s: SPI_CTRL=0x%08x SPI_EXT=0x%08x\n",
	         __FUNCTION__,
	         SPIPCI_RREG(osh, &regs->spih_ctrl),
	         SPIPCI_RREG(osh, &regs->spih_ext)));

	return TRUE;
}


bool
spi_controller_highspeed_mode(sdioh_info_t *sd, bool hsmode)
{
	spih_info_t *si = (spih_info_t *)sd->controller;
	osl_t *osh = si->osh;
	spih_regs_t *regs = si->regs;

	if (si->rev >= 10) {
		if (hsmode) {
			SPIPCI_ORREG(osh, &regs->spih_ext, 0x10);
		} else {
			SPIPCI_ANDREG(osh, &regs->spih_ext, ~0x10);
		}
	}

	return TRUE;
}


void
spi_devintr_off(sdioh_info_t *sd)
{
	spih_info_t *si = (spih_info_t *)sd->controller;
	osl_t *osh = si->osh;
	spih_regs_t *regs = si->regs;

	sd_trace(("%s: %d\n", __FUNCTION__, sd->use_client_ints));
	if (sd->use_client_ints) {
		sd->intmask &= ~SPIH_DEV_INTR;
		SPIPCI_WREG(osh, &regs->spih_int_mask, sd->intmask);	
	}
}


void
spi_devintr_on(sdioh_info_t *sd)
{
	spih_info_t *si = (spih_info_t *)sd->controller;
	osl_t *osh = si->osh;
	spih_regs_t *regs = si->regs;

	ASSERT(sd->lockcount == 0);
	sd_trace(("%s: %d\n", __FUNCTION__, sd->use_client_ints));
	if (sd->use_client_ints) {
		if (SPIPCI_RREG(osh, &regs->spih_ctrl) & 0x02) {
			
			SPIPCI_WREG(osh, &regs->spih_int_status, SPIH_DEV_INTR);
		}
		sd->intmask |= SPIH_DEV_INTR;
		
		SPIPCI_WREG(osh, &regs->spih_int_mask, sd->intmask);
	}
}


bool
spi_check_client_intr(sdioh_info_t *sd, int *is_dev_intr)
{
	spih_info_t *si = (spih_info_t *)sd->controller;
	osl_t *osh = si->osh;
	spih_regs_t *regs = si->regs;
	bool ours = FALSE;

	uint32 raw_int, cur_int;
	ASSERT(sd);

	if (is_dev_intr)
		*is_dev_intr = FALSE;
	raw_int = SPIPCI_RREG(osh, &regs->spih_int_status);
	cur_int = raw_int & sd->intmask;
	if (cur_int & SPIH_DEV_INTR) {
		if (sd->client_intr_enabled && sd->use_client_ints) {
			sd->intrcount++;
			ASSERT(sd->intr_handler);
			ASSERT(sd->intr_handler_arg);
			(sd->intr_handler)(sd->intr_handler_arg);
			if (is_dev_intr)
				*is_dev_intr = TRUE;
		} else {
			sd_trace(("%s: Not ready for intr: enabled %d, handler 0x%p\n",
			        __FUNCTION__, sd->client_intr_enabled, sd->intr_handler));
		}
		SPIPCI_WREG(osh, &regs->spih_int_status, SPIH_DEV_INTR);
		SPIPCI_RREG(osh, &regs->spih_int_status);
		ours = TRUE;
	} else if (cur_int & SPIH_CTLR_INTR) {
		
		sd_trace(("%s: SPI CTLR interrupt: raw_int 0x%08x cur_int 0x%08x\n",
		          __FUNCTION__, raw_int, cur_int));

		
		SPIPCI_WREG(osh, &regs->spih_stat, 0x00000080);

		
		SPIPCI_WREG(osh, &regs->spih_int_status, SPIH_CTLR_INTR);
		SPIPCI_RREG(osh, &regs->spih_int_status);

		ours = TRUE;
	} else if (cur_int & SPIH_WFIFO_INTR) {
		sd_trace(("%s: SPI WR FIFO Empty interrupt: raw_int 0x%08x cur_int 0x%08x\n",
		          __FUNCTION__, raw_int, cur_int));

		
		sd->intmask &= ~SPIH_WFIFO_INTR;
		SPIPCI_WREG(osh, &regs->spih_int_mask, sd->intmask);

		sd->local_intrcount++;
		sd->got_hcint = TRUE;
		ours = TRUE;
	} else {
		
		sd_trace(("%s: Not my interrupt: raw_int 0x%08x cur_int 0x%08x\n",
		          __FUNCTION__, raw_int, cur_int));
		ours = FALSE;
	}

	return ours;
}

static void
hexdump(char *pfx, unsigned char *msg, int msglen)
{
	int i, col;
	char buf[80];

	ASSERT(strlen(pfx) + 49 <= sizeof(buf));

	col = 0;

	for (i = 0; i < msglen; i++, col++) {
		if (col % 16 == 0)
			strcpy(buf, pfx);
		sprintf(buf + strlen(buf), "%02x", msg[i]);
		if ((col + 1) % 16 == 0)
			printf("%s\n", buf);
		else
			sprintf(buf + strlen(buf), " ");
	}

	if (col % 16 != 0)
		printf("%s\n", buf);
}


void
spi_sendrecv(sdioh_info_t *sd, uint8 *msg_out, uint8 *msg_in, int msglen)
{
	spih_info_t *si = (spih_info_t *)sd->controller;
	osl_t *osh = si->osh;
	spih_regs_t *regs = si->regs;
	uint32 count;
	uint32 spi_data_out;
	uint32 spi_data_in;
	bool yield;

	sd_trace(("%s: enter\n", __FUNCTION__));

	if (bcmpcispi_dump) {
		printf("SENDRECV(len=%d)\n", msglen);
		hexdump(" OUT: ", msg_out, msglen);
	}

#ifdef BCMSDYIELD
	
	yield = ((msglen > 500) && (si->rev >= 8));
#else
	yield = FALSE;
#endif 

	ASSERT(msglen % 4 == 0);


	SPIPCI_ANDREG(osh, &regs->spih_gpio_data, ~SPIH_CS);	

	for (count = 0; count < (uint32)msglen/4; count++) {
		spi_data_out = ((uint32)((uint32 *)msg_out)[count]);
		SPIPCI_WREG(osh, &regs->spih_data, spi_data_out);
	}

#ifdef BCMSDYIELD
	if (yield) {
		
		SPIPCI_WREG(osh, &regs->spih_int_status, SPIH_WFIFO_INTR);
		SPIPCI_RREG(osh, &regs->spih_int_status);

		
		sd->intmask |= SPIH_WFIFO_INTR;
		sd->got_hcint = FALSE;
		SPIPCI_WREG(osh, &regs->spih_int_mask, sd->intmask);

	}
#endif 

	
	SPIPCI_ANDREG(osh, &regs->spih_gpio_data, ~0x00000020);	

	if (yield) {
		ASSERT((SPIPCI_RREG(sd->osh, &regs->spih_stat) & SPIH_WFEMPTY) == 0);
	}

	spi_waitbits(sd, yield);
	SPIPCI_ORREG(osh, &regs->spih_gpio_data, 0x00000020);	

	for (count = 0; count < (uint32)msglen/4; count++) {
		spi_data_in = SPIPCI_RREG(osh, &regs->spih_data);
		((uint32 *)msg_in)[count] = spi_data_in;
	}

	
	SPIPCI_ORREG(osh, &regs->spih_gpio_data, SPIH_CS);

	if (bcmpcispi_dump) {
		hexdump(" IN : ", msg_in, msglen);
	}
}

void
spi_spinbits(sdioh_info_t *sd)
{
	spih_info_t *si = (spih_info_t *)sd->controller;
	osl_t *osh = si->osh;
	spih_regs_t *regs = si->regs;
	uint spin_count; 

	spin_count = 0;
	while ((SPIPCI_RREG(sd->osh, &regs->spih_stat) & SPIH_WFEMPTY) == 0) {
		if (spin_count > SPI_SPIN_BOUND) {
			ASSERT(FALSE); 
		}
		spin_count++;
	}
	spin_count = 0;
	
	while ((SPIPCI_RREG(osh, &regs->spih_stat) & SPIH_STATE_MASK) != 0) {
		if (spin_count > SPI_SPIN_BOUND) {
			ASSERT(FALSE);
		}
		spin_count++;
	}
}
