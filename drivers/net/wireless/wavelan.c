

#include "wavelan.p.h"		






static u8 wv_irq_to_psa(int irq)
{
	if (irq < 0 || irq >= ARRAY_SIZE(irqvals))
		return 0;

	return irqvals[irq];
}



static int __init wv_psa_to_irq(u8 irqval)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(irqvals); i++)
		if (irqvals[i] == irqval)
			return i;

	return -1;
}






static inline u16 hasr_read(unsigned long ioaddr)
{
	return (inw(HASR(ioaddr)));
}				



static inline void hacr_write(unsigned long ioaddr, u16 hacr)
{
	outw(hacr, HACR(ioaddr));
}				



static void hacr_write_slow(unsigned long ioaddr, u16 hacr)
{
	hacr_write(ioaddr, hacr);
	
	mdelay(1);
}				



static inline void set_chan_attn(unsigned long ioaddr, u16 hacr)
{
	hacr_write(ioaddr, hacr | HACR_CA);
}				



static inline void wv_hacr_reset(unsigned long ioaddr)
{
	hacr_write_slow(ioaddr, HACR_RESET);
	hacr_write(ioaddr, HACR_DEFAULT);
}				



static inline void wv_16_off(unsigned long ioaddr, u16 hacr)
{
	hacr &= ~HACR_16BITS;
	hacr_write(ioaddr, hacr);
}				



static inline void wv_16_on(unsigned long ioaddr, u16 hacr)
{
	hacr |= HACR_16BITS;
	hacr_write(ioaddr, hacr);
}				



static inline void wv_ints_off(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	
	lp->hacr &= ~HACR_INTRON;
	hacr_write(ioaddr, lp->hacr);
}				



static inline void wv_ints_on(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;

	lp->hacr |= HACR_INTRON;
	hacr_write(ioaddr, lp->hacr);
}				







static void psa_read(unsigned long ioaddr, u16 hacr, int o,	
		     u8 * b,	
		     int n)
{				
	wv_16_off(ioaddr, hacr);

	while (n-- > 0) {
		outw(o, PIOR2(ioaddr));
		o++;
		*b++ = inb(PIOP2(ioaddr));
	}

	wv_16_on(ioaddr, hacr);
}				



static void psa_write(unsigned long ioaddr, u16 hacr, int o,	
		      u8 * b,	
		      int n)
{				
	int count = 0;

	wv_16_off(ioaddr, hacr);

	while (n-- > 0) {
		outw(o, PIOR2(ioaddr));
		o++;

		outb(*b, PIOP2(ioaddr));
		b++;

		
		count = 0;
		while ((count++ < 100) &&
		       (hasr_read(ioaddr) & HASR_PSA_BUSY)) mdelay(1);
	}

	wv_16_on(ioaddr, hacr);
}				

#ifdef SET_PSA_CRC


static u16 psa_crc(u8 * psa,	
			      int size)
{				
	int byte_cnt;		
	u16 crc_bytes = 0;	
	int bit_cnt;		

	for (byte_cnt = 0; byte_cnt < size; byte_cnt++) {
		crc_bytes ^= psa[byte_cnt];	

		for (bit_cnt = 1; bit_cnt < 9; bit_cnt++) {
			if (crc_bytes & 0x0001)
				crc_bytes = (crc_bytes >> 1) ^ 0xA001;
			else
				crc_bytes >>= 1;
		}
	}

	return crc_bytes;
}				
#endif				



static void update_psa_checksum(struct net_device * dev, unsigned long ioaddr, u16 hacr)
{
#ifdef SET_PSA_CRC
	psa_t psa;
	u16 crc;

	
	psa_read(ioaddr, hacr, 0, (unsigned char *) &psa, sizeof(psa));

	
	crc = psa_crc((unsigned char *) &psa,
		      sizeof(psa) - sizeof(psa.psa_crc[0]) -
		      sizeof(psa.psa_crc[1])
		      - sizeof(psa.psa_crc_status));

	psa.psa_crc[0] = crc & 0xFF;
	psa.psa_crc[1] = (crc & 0xFF00) >> 8;

	
	psa_write(ioaddr, hacr, (char *) &psa.psa_crc - (char *) &psa,
		  (unsigned char *) &psa.psa_crc, 2);

#ifdef DEBUG_IOCTL_INFO
	printk(KERN_DEBUG "%s: update_psa_checksum(): crc = 0x%02x%02x\n",
	       dev->name, psa.psa_crc[0], psa.psa_crc[1]);

	
	crc = psa_crc((unsigned char *) &psa,
		      sizeof(psa) - sizeof(psa.psa_crc_status));

	if (crc != 0)
		printk(KERN_WARNING
		       "%s: update_psa_checksum(): CRC does not agree with PSA data (even after recalculating)\n",
		       dev->name);
#endif				
#endif				
}				



static void mmc_out(unsigned long ioaddr, u16 o, u8 d)
{
	int count = 0;

	
	while ((count++ < 100) && (inw(HASR(ioaddr)) & HASR_MMC_BUSY))
		udelay(10);

	outw((u16) (((u16) d << 8) | (o << 1) | 1), MMCR(ioaddr));
}



static void mmc_write(unsigned long ioaddr, u8 o, u8 * b, int n)
{
	o += n;
	b += n;

	while (n-- > 0)
		mmc_out(ioaddr, --o, *(--b));
}				



static u8 mmc_in(unsigned long ioaddr, u16 o)
{
	int count = 0;

	while ((count++ < 100) && (inw(HASR(ioaddr)) & HASR_MMC_BUSY))
		udelay(10);
	outw(o << 1, MMCR(ioaddr));

	while ((count++ < 100) && (inw(HASR(ioaddr)) & HASR_MMC_BUSY))
		udelay(10);
	return (u8) (inw(MMCR(ioaddr)) >> 8);
}



static inline void mmc_read(unsigned long ioaddr, u8 o, u8 * b, int n)
{
	o += n;
	b += n;

	while (n-- > 0)
		*(--b) = mmc_in(ioaddr, --o);
}				



static inline int mmc_encr(unsigned long ioaddr)
{				
	int temp;

	temp = mmc_in(ioaddr, mmroff(0, mmr_des_avail));
	if ((temp != MMR_DES_AVAIL_DES) && (temp != MMR_DES_AVAIL_AES))
		return 0;
	else
		return temp;
}



static inline void fee_wait(unsigned long ioaddr,	
			    int delay,	
			    int number)
{				
	int count = 0;		

	while ((count++ < number) &&
	       (mmc_in(ioaddr, mmroff(0, mmr_fee_status)) &
		MMR_FEE_STATUS_BUSY)) udelay(delay);
}



static void fee_read(unsigned long ioaddr,	
		     u16 o,	
		     u16 * b,	
		     int n)
{				
	b += n;			

	
	mmc_out(ioaddr, mmwoff(0, mmw_fee_addr), o + n - 1);

	
	while (n-- > 0) {
		
		mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl),
			MMW_FEE_CTRL_READ);

		
		fee_wait(ioaddr, 10, 100);

		
		*--b = ((mmc_in(ioaddr, mmroff(0, mmr_fee_data_h)) << 8) |
			mmc_in(ioaddr, mmroff(0, mmr_fee_data_l)));
	}
}




static void fee_write(unsigned long ioaddr,	
		      u16 o,	
		      u16 * b,	
		      int n)
{				
	b += n;			

#ifdef EEPROM_IS_PROTECTED	
#ifdef DOESNT_SEEM_TO_WORK	
	
	mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PRREAD);

	fee_wait(ioaddr, 10, 100);

	
	printk("Protected 2:  %02X-%02X\n",
	       mmc_in(ioaddr, mmroff(0, mmr_fee_data_h)),
	       mmc_in(ioaddr, mmroff(0, mmr_fee_data_l)));
#endif				

	
	mmc_out(ioaddr, mmwoff(0, mmw_fee_addr), MMW_FEE_ADDR_EN);
	mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PREN);

	fee_wait(ioaddr, 10, 100);

	
	mmc_out(ioaddr, mmwoff(0, mmw_fee_addr), o + n);
	mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PRWRITE);
#ifdef DOESNT_SEEM_TO_WORK	
	
	mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PRCLEAR);
#endif				

	fee_wait(ioaddr, 10, 100);
#endif				

	
	mmc_out(ioaddr, mmwoff(0, mmw_fee_addr), MMW_FEE_ADDR_EN);
	mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_WREN);

	fee_wait(ioaddr, 10, 100);

	
	mmc_out(ioaddr, mmwoff(0, mmw_fee_addr), o + n - 1);

	
	while (n-- > 0) {
		
		mmc_out(ioaddr, mmwoff(0, mmw_fee_data_h), (*--b) >> 8);
		mmc_out(ioaddr, mmwoff(0, mmw_fee_data_l), *b & 0xFF);

		
		mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl),
			MMW_FEE_CTRL_WRITE);

		
		mdelay(10);
		fee_wait(ioaddr, 10, 100);
	}

	
	mmc_out(ioaddr, mmwoff(0, mmw_fee_addr), MMW_FEE_ADDR_DS);
	mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_WDS);

	fee_wait(ioaddr, 10, 100);

#ifdef EEPROM_IS_PROTECTED	
	
	mmc_out(ioaddr, mmwoff(0, mmw_fee_addr), 0x00);
	mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PRWRITE);

	fee_wait(ioaddr, 10, 100);
#endif				
}






static  void obram_read(unsigned long ioaddr,
				   u16 o, u8 * b, int n)
{
	outw(o, PIOR1(ioaddr));
	insw(PIOP1(ioaddr), (unsigned short *) b, (n + 1) >> 1);
}



static inline void obram_write(unsigned long ioaddr, u16 o, u8 * b, int n)
{
	outw(o, PIOR1(ioaddr));
	outsw(PIOP1(ioaddr), (unsigned short *) b, (n + 1) >> 1);
}



static void wv_ack(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	u16 scb_cs;
	int i;

	obram_read(ioaddr, scboff(OFFSET_SCB, scb_status),
		   (unsigned char *) &scb_cs, sizeof(scb_cs));
	scb_cs &= SCB_ST_INT;

	if (scb_cs == 0)
		return;

	obram_write(ioaddr, scboff(OFFSET_SCB, scb_command),
		    (unsigned char *) &scb_cs, sizeof(scb_cs));

	set_chan_attn(ioaddr, lp->hacr);

	for (i = 1000; i > 0; i--) {
		obram_read(ioaddr, scboff(OFFSET_SCB, scb_command),
			   (unsigned char *) &scb_cs, sizeof(scb_cs));
		if (scb_cs == 0)
			break;

		udelay(10);
	}
	udelay(100);

#ifdef DEBUG_CONFIG_ERROR
	if (i <= 0)
		printk(KERN_INFO
		       "%s: wv_ack(): board not accepting command.\n",
		       dev->name);
#endif
}



static int wv_synchronous_cmd(struct net_device * dev, const char *str)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	u16 scb_cmd;
	ach_t cb;
	int i;

	scb_cmd = SCB_CMD_CUC & SCB_CMD_CUC_GO;
	obram_write(ioaddr, scboff(OFFSET_SCB, scb_command),
		    (unsigned char *) &scb_cmd, sizeof(scb_cmd));

	set_chan_attn(ioaddr, lp->hacr);

	for (i = 1000; i > 0; i--) {
		obram_read(ioaddr, OFFSET_CU, (unsigned char *) &cb,
			   sizeof(cb));
		if (cb.ac_status & AC_SFLD_C)
			break;

		udelay(10);
	}
	udelay(100);

	if (i <= 0 || !(cb.ac_status & AC_SFLD_OK)) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_INFO "%s: %s failed; status = 0x%x\n",
		       dev->name, str, cb.ac_status);
#endif
#ifdef DEBUG_I82586_SHOW
		wv_scb_show(ioaddr);
#endif
		return -1;
	}

	
	wv_ack(dev);

	return 0;
}



static int
wv_config_complete(struct net_device * dev, unsigned long ioaddr, net_local * lp)
{
	unsigned short mcs_addr;
	unsigned short status;
	int ret;

#ifdef DEBUG_INTERRUPT_TRACE
	printk(KERN_DEBUG "%s: ->wv_config_complete()\n", dev->name);
#endif

	mcs_addr = lp->tx_first_in_use + sizeof(ac_tx_t) + sizeof(ac_nop_t)
	    + sizeof(tbd_t) + sizeof(ac_cfg_t) + sizeof(ac_ias_t);

	
	obram_read(ioaddr, acoff(mcs_addr, ac_status),
		   (unsigned char *) &status, sizeof(status));

	
	if ((status & AC_SFLD_C) == 0)
		ret = 0;	
	else {
#ifdef DEBUG_CONFIG_ERROR
		unsigned short cfg_addr;
		unsigned short ias_addr;

		
		if ((status & AC_SFLD_OK) != AC_SFLD_OK)
			printk(KERN_INFO
			       "%s: wv_config_complete(): set_multicast_address failed; status = 0x%x\n",
			       dev->name, status);

		
		ias_addr = mcs_addr - sizeof(ac_ias_t);
		obram_read(ioaddr, acoff(ias_addr, ac_status),
			   (unsigned char *) &status, sizeof(status));
		if ((status & AC_SFLD_OK) != AC_SFLD_OK)
			printk(KERN_INFO
			       "%s: wv_config_complete(): set_MAC_address failed; status = 0x%x\n",
			       dev->name, status);

		
		cfg_addr = ias_addr - sizeof(ac_cfg_t);
		obram_read(ioaddr, acoff(cfg_addr, ac_status),
			   (unsigned char *) &status, sizeof(status));
		if ((status & AC_SFLD_OK) != AC_SFLD_OK)
			printk(KERN_INFO
			       "%s: wv_config_complete(): configure failed; status = 0x%x\n",
			       dev->name, status);
#endif	

		ret = 1;	
	}

#ifdef DEBUG_INTERRUPT_TRACE
	printk(KERN_DEBUG "%s: <-wv_config_complete() - %d\n", dev->name,
	       ret);
#endif
	return ret;
}



static int wv_complete(struct net_device * dev, unsigned long ioaddr, net_local * lp)
{
	int nreaped = 0;

#ifdef DEBUG_INTERRUPT_TRACE
	printk(KERN_DEBUG "%s: ->wv_complete()\n", dev->name);
#endif

	
	while (lp->tx_first_in_use != I82586NULL) {
		unsigned short tx_status;

		
		obram_read(ioaddr, acoff(lp->tx_first_in_use, ac_status),
			   (unsigned char *) &tx_status,
			   sizeof(tx_status));

		
		if ((tx_status & AC_SFLD_C) == 0)
			break;

		
		if (tx_status == 0xFFFF)
			if (!wv_config_complete(dev, ioaddr, lp))
				break;	

		
		nreaped++;
		--lp->tx_n_in_use;



		
		if (lp->tx_n_in_use <= 0)
			lp->tx_first_in_use = I82586NULL;
		else {
			
			lp->tx_first_in_use += TXBLOCKZ;
			if (lp->tx_first_in_use >=
			    OFFSET_CU +
			    NTXBLOCKS * TXBLOCKZ) lp->tx_first_in_use -=
				    NTXBLOCKS * TXBLOCKZ;
		}

		
		if (tx_status == 0xFFFF)
			continue;

		
		if (tx_status & AC_SFLD_OK) {
			int ncollisions;

			dev->stats.tx_packets++;
			ncollisions = tx_status & AC_SFLD_MAXCOL;
			dev->stats.collisions += ncollisions;
#ifdef DEBUG_TX_INFO
			if (ncollisions > 0)
				printk(KERN_DEBUG
				       "%s: wv_complete(): tx completed after %d collisions.\n",
				       dev->name, ncollisions);
#endif
		} else {
			dev->stats.tx_errors++;
			if (tx_status & AC_SFLD_S10) {
				dev->stats.tx_carrier_errors++;
#ifdef DEBUG_TX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_complete(): tx error: no CS.\n",
				       dev->name);
#endif
			}
			if (tx_status & AC_SFLD_S9) {
				dev->stats.tx_carrier_errors++;
#ifdef DEBUG_TX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_complete(): tx error: lost CTS.\n",
				       dev->name);
#endif
			}
			if (tx_status & AC_SFLD_S8) {
				dev->stats.tx_fifo_errors++;
#ifdef DEBUG_TX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_complete(): tx error: slow DMA.\n",
				       dev->name);
#endif
			}
			if (tx_status & AC_SFLD_S6) {
				dev->stats.tx_heartbeat_errors++;
#ifdef DEBUG_TX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_complete(): tx error: heart beat.\n",
				       dev->name);
#endif
			}
			if (tx_status & AC_SFLD_S5) {
				dev->stats.tx_aborted_errors++;
#ifdef DEBUG_TX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_complete(): tx error: too many collisions.\n",
				       dev->name);
#endif
			}
		}

#ifdef DEBUG_TX_INFO
		printk(KERN_DEBUG
		       "%s: wv_complete(): tx completed, tx_status 0x%04x\n",
		       dev->name, tx_status);
#endif
	}

#ifdef DEBUG_INTERRUPT_INFO
	if (nreaped > 1)
		printk(KERN_DEBUG "%s: wv_complete(): reaped %d\n",
		       dev->name, nreaped);
#endif

	
	if (lp->tx_n_in_use < NTXBLOCKS - 1) {
		netif_wake_queue(dev);
	}
#ifdef DEBUG_INTERRUPT_TRACE
	printk(KERN_DEBUG "%s: <-wv_complete()\n", dev->name);
#endif
	return nreaped;
}



static void wv_82586_reconfig(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long flags;

	
	lp->reconfig_82586 = 1;

	
	if((netif_running(dev)) && !(netif_queue_stopped(dev))) {
		spin_lock_irqsave(&lp->spinlock, flags);
		
		wv_82586_config(dev);
		spin_unlock_irqrestore(&lp->spinlock, flags);
	}
	else {
#ifdef DEBUG_CONFIG_INFO
		printk(KERN_DEBUG
		       "%s: wv_82586_reconfig(): delayed (state = %lX)\n",
			       dev->name, dev->state);
#endif
	}
}




#ifdef DEBUG_PSA_SHOW


static void wv_psa_show(psa_t * p)
{
	printk(KERN_DEBUG "##### WaveLAN PSA contents: #####\n");
	printk(KERN_DEBUG "psa_io_base_addr_1: 0x%02X %02X %02X %02X\n",
	       p->psa_io_base_addr_1,
	       p->psa_io_base_addr_2,
	       p->psa_io_base_addr_3, p->psa_io_base_addr_4);
	printk(KERN_DEBUG "psa_rem_boot_addr_1: 0x%02X %02X %02X\n",
	       p->psa_rem_boot_addr_1,
	       p->psa_rem_boot_addr_2, p->psa_rem_boot_addr_3);
	printk(KERN_DEBUG "psa_holi_params: 0x%02x, ", p->psa_holi_params);
	printk("psa_int_req_no: %d\n", p->psa_int_req_no);
#ifdef DEBUG_SHOW_UNUSED
	printk(KERN_DEBUG "psa_unused0[]: %pM\n", p->psa_unused0);
#endif				
	printk(KERN_DEBUG "psa_univ_mac_addr[]: %pM\n", p->psa_univ_mac_addr);
	printk(KERN_DEBUG "psa_local_mac_addr[]: %pM\n", p->psa_local_mac_addr);
	printk(KERN_DEBUG "psa_univ_local_sel: %d, ",
	       p->psa_univ_local_sel);
	printk("psa_comp_number: %d, ", p->psa_comp_number);
	printk("psa_thr_pre_set: 0x%02x\n", p->psa_thr_pre_set);
	printk(KERN_DEBUG "psa_feature_select/decay_prm: 0x%02x, ",
	       p->psa_feature_select);
	printk("psa_subband/decay_update_prm: %d\n", p->psa_subband);
	printk(KERN_DEBUG "psa_quality_thr: 0x%02x, ", p->psa_quality_thr);
	printk("psa_mod_delay: 0x%02x\n", p->psa_mod_delay);
	printk(KERN_DEBUG "psa_nwid: 0x%02x%02x, ", p->psa_nwid[0],
	       p->psa_nwid[1]);
	printk("psa_nwid_select: %d\n", p->psa_nwid_select);
	printk(KERN_DEBUG "psa_encryption_select: %d, ",
	       p->psa_encryption_select);
	printk
	    ("psa_encryption_key[]: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
	     p->psa_encryption_key[0], p->psa_encryption_key[1],
	     p->psa_encryption_key[2], p->psa_encryption_key[3],
	     p->psa_encryption_key[4], p->psa_encryption_key[5],
	     p->psa_encryption_key[6], p->psa_encryption_key[7]);
	printk(KERN_DEBUG "psa_databus_width: %d\n", p->psa_databus_width);
	printk(KERN_DEBUG "psa_call_code/auto_squelch: 0x%02x, ",
	       p->psa_call_code[0]);
	printk
	    ("psa_call_code[]: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
	     p->psa_call_code[0], p->psa_call_code[1], p->psa_call_code[2],
	     p->psa_call_code[3], p->psa_call_code[4], p->psa_call_code[5],
	     p->psa_call_code[6], p->psa_call_code[7]);
#ifdef DEBUG_SHOW_UNUSED
	printk(KERN_DEBUG "psa_reserved[]: %02X:%02X\n",
	       p->psa_reserved[0],
	       p->psa_reserved[1]);
#endif				
	printk(KERN_DEBUG "psa_conf_status: %d, ", p->psa_conf_status);
	printk("psa_crc: 0x%02x%02x, ", p->psa_crc[0], p->psa_crc[1]);
	printk("psa_crc_status: 0x%02x\n", p->psa_crc_status);
}				
#endif				

#ifdef DEBUG_MMC_SHOW


static void wv_mmc_show(struct net_device * dev)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	mmr_t m;

	
	if (hasr_read(ioaddr) & HASR_NO_CLK) {
		printk(KERN_WARNING
		       "%s: wv_mmc_show: modem not connected\n",
		       dev->name);
		return;
	}

	
	mmc_out(ioaddr, mmwoff(0, mmw_freeze), 1);
	mmc_read(ioaddr, 0, (u8 *) & m, sizeof(m));
	mmc_out(ioaddr, mmwoff(0, mmw_freeze), 0);

	
	lp->wstats.discard.nwid +=
	    (m.mmr_wrong_nwid_h << 8) | m.mmr_wrong_nwid_l;

	printk(KERN_DEBUG "##### WaveLAN modem status registers: #####\n");
#ifdef DEBUG_SHOW_UNUSED
	printk(KERN_DEBUG
	       "mmc_unused0[]: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
	       m.mmr_unused0[0], m.mmr_unused0[1], m.mmr_unused0[2],
	       m.mmr_unused0[3], m.mmr_unused0[4], m.mmr_unused0[5],
	       m.mmr_unused0[6], m.mmr_unused0[7]);
#endif				
	printk(KERN_DEBUG "Encryption algorithm: %02X - Status: %02X\n",
	       m.mmr_des_avail, m.mmr_des_status);
#ifdef DEBUG_SHOW_UNUSED
	printk(KERN_DEBUG "mmc_unused1[]: %02X:%02X:%02X:%02X:%02X\n",
	       m.mmr_unused1[0],
	       m.mmr_unused1[1],
	       m.mmr_unused1[2], m.mmr_unused1[3], m.mmr_unused1[4]);
#endif				
	printk(KERN_DEBUG "dce_status: 0x%x [%s%s%s%s]\n",
	       m.mmr_dce_status,
	       (m.
		mmr_dce_status & MMR_DCE_STATUS_RX_BUSY) ?
	       "energy detected," : "",
	       (m.
		mmr_dce_status & MMR_DCE_STATUS_LOOPT_IND) ?
	       "loop test indicated," : "",
	       (m.
		mmr_dce_status & MMR_DCE_STATUS_TX_BUSY) ?
	       "transmitter on," : "",
	       (m.
		mmr_dce_status & MMR_DCE_STATUS_JBR_EXPIRED) ?
	       "jabber timer expired," : "");
	printk(KERN_DEBUG "Dsp ID: %02X\n", m.mmr_dsp_id);
#ifdef DEBUG_SHOW_UNUSED
	printk(KERN_DEBUG "mmc_unused2[]: %02X:%02X\n",
	       m.mmr_unused2[0], m.mmr_unused2[1]);
#endif				
	printk(KERN_DEBUG "# correct_nwid: %d, # wrong_nwid: %d\n",
	       (m.mmr_correct_nwid_h << 8) | m.mmr_correct_nwid_l,
	       (m.mmr_wrong_nwid_h << 8) | m.mmr_wrong_nwid_l);
	printk(KERN_DEBUG "thr_pre_set: 0x%x [current signal %s]\n",
	       m.mmr_thr_pre_set & MMR_THR_PRE_SET,
	       (m.
		mmr_thr_pre_set & MMR_THR_PRE_SET_CUR) ? "above" :
	       "below");
	printk(KERN_DEBUG "signal_lvl: %d [%s], ",
	       m.mmr_signal_lvl & MMR_SIGNAL_LVL,
	       (m.
		mmr_signal_lvl & MMR_SIGNAL_LVL_VALID) ? "new msg" :
	       "no new msg");
	printk("silence_lvl: %d [%s], ",
	       m.mmr_silence_lvl & MMR_SILENCE_LVL,
	       (m.
		mmr_silence_lvl & MMR_SILENCE_LVL_VALID) ? "update done" :
	       "no new update");
	printk("sgnl_qual: 0x%x [%s]\n", m.mmr_sgnl_qual & MMR_SGNL_QUAL,
	       (m.
		mmr_sgnl_qual & MMR_SGNL_QUAL_ANT) ? "Antenna 1" :
	       "Antenna 0");
#ifdef DEBUG_SHOW_UNUSED
	printk(KERN_DEBUG "netw_id_l: %x\n", m.mmr_netw_id_l);
#endif				
}				
#endif				

#ifdef DEBUG_I82586_SHOW


static void wv_scb_show(unsigned long ioaddr)
{
	scb_t scb;

	obram_read(ioaddr, OFFSET_SCB, (unsigned char *) &scb,
		   sizeof(scb));

	printk(KERN_DEBUG "##### WaveLAN system control block: #####\n");

	printk(KERN_DEBUG "status: ");
	printk("stat 0x%x[%s%s%s%s] ",
	       (scb.
		scb_status & (SCB_ST_CX | SCB_ST_FR | SCB_ST_CNA |
			      SCB_ST_RNR)) >> 12,
	       (scb.
		scb_status & SCB_ST_CX) ? "command completion interrupt," :
	       "", (scb.scb_status & SCB_ST_FR) ? "frame received," : "",
	       (scb.
		scb_status & SCB_ST_CNA) ? "command unit not active," : "",
	       (scb.
		scb_status & SCB_ST_RNR) ? "receiving unit not ready," :
	       "");
	printk("cus 0x%x[%s%s%s] ", (scb.scb_status & SCB_ST_CUS) >> 8,
	       ((scb.scb_status & SCB_ST_CUS) ==
		SCB_ST_CUS_IDLE) ? "idle" : "",
	       ((scb.scb_status & SCB_ST_CUS) ==
		SCB_ST_CUS_SUSP) ? "suspended" : "",
	       ((scb.scb_status & SCB_ST_CUS) ==
		SCB_ST_CUS_ACTV) ? "active" : "");
	printk("rus 0x%x[%s%s%s%s]\n", (scb.scb_status & SCB_ST_RUS) >> 4,
	       ((scb.scb_status & SCB_ST_RUS) ==
		SCB_ST_RUS_IDLE) ? "idle" : "",
	       ((scb.scb_status & SCB_ST_RUS) ==
		SCB_ST_RUS_SUSP) ? "suspended" : "",
	       ((scb.scb_status & SCB_ST_RUS) ==
		SCB_ST_RUS_NRES) ? "no resources" : "",
	       ((scb.scb_status & SCB_ST_RUS) ==
		SCB_ST_RUS_RDY) ? "ready" : "");

	printk(KERN_DEBUG "command: ");
	printk("ack 0x%x[%s%s%s%s] ",
	       (scb.
		scb_command & (SCB_CMD_ACK_CX | SCB_CMD_ACK_FR |
			       SCB_CMD_ACK_CNA | SCB_CMD_ACK_RNR)) >> 12,
	       (scb.
		scb_command & SCB_CMD_ACK_CX) ? "ack cmd completion," : "",
	       (scb.
		scb_command & SCB_CMD_ACK_FR) ? "ack frame received," : "",
	       (scb.
		scb_command & SCB_CMD_ACK_CNA) ? "ack CU not active," : "",
	       (scb.
		scb_command & SCB_CMD_ACK_RNR) ? "ack RU not ready," : "");
	printk("cuc 0x%x[%s%s%s%s%s] ",
	       (scb.scb_command & SCB_CMD_CUC) >> 8,
	       ((scb.scb_command & SCB_CMD_CUC) ==
		SCB_CMD_CUC_NOP) ? "nop" : "",
	       ((scb.scb_command & SCB_CMD_CUC) ==
		SCB_CMD_CUC_GO) ? "start cbl_offset" : "",
	       ((scb.scb_command & SCB_CMD_CUC) ==
		SCB_CMD_CUC_RES) ? "resume execution" : "",
	       ((scb.scb_command & SCB_CMD_CUC) ==
		SCB_CMD_CUC_SUS) ? "suspend execution" : "",
	       ((scb.scb_command & SCB_CMD_CUC) ==
		SCB_CMD_CUC_ABT) ? "abort execution" : "");
	printk("ruc 0x%x[%s%s%s%s%s]\n",
	       (scb.scb_command & SCB_CMD_RUC) >> 4,
	       ((scb.scb_command & SCB_CMD_RUC) ==
		SCB_CMD_RUC_NOP) ? "nop" : "",
	       ((scb.scb_command & SCB_CMD_RUC) ==
		SCB_CMD_RUC_GO) ? "start rfa_offset" : "",
	       ((scb.scb_command & SCB_CMD_RUC) ==
		SCB_CMD_RUC_RES) ? "resume reception" : "",
	       ((scb.scb_command & SCB_CMD_RUC) ==
		SCB_CMD_RUC_SUS) ? "suspend reception" : "",
	       ((scb.scb_command & SCB_CMD_RUC) ==
		SCB_CMD_RUC_ABT) ? "abort reception" : "");

	printk(KERN_DEBUG "cbl_offset 0x%x ", scb.scb_cbl_offset);
	printk("rfa_offset 0x%x\n", scb.scb_rfa_offset);

	printk(KERN_DEBUG "crcerrs %d ", scb.scb_crcerrs);
	printk("alnerrs %d ", scb.scb_alnerrs);
	printk("rscerrs %d ", scb.scb_rscerrs);
	printk("ovrnerrs %d\n", scb.scb_ovrnerrs);
}



static void wv_ru_show(struct net_device * dev)
{
	printk(KERN_DEBUG
	       "##### WaveLAN i82586 receiver unit status: #####\n");
	printk(KERN_DEBUG "ru:");
	
	printk("\n");
}				



static void wv_cu_show_one(struct net_device * dev, net_local * lp, int i, u16 p)
{
	unsigned long ioaddr;
	ac_tx_t actx;

	ioaddr = dev->base_addr;

	printk("%d: 0x%x:", i, p);

	obram_read(ioaddr, p, (unsigned char *) &actx, sizeof(actx));
	printk(" status=0x%x,", actx.tx_h.ac_status);
	printk(" command=0x%x,", actx.tx_h.ac_command);

	

	printk("|");
}



static void wv_cu_show(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned int i;
	u16 p;

	printk(KERN_DEBUG
	       "##### WaveLAN i82586 command unit status: #####\n");

	printk(KERN_DEBUG);
	for (i = 0, p = lp->tx_first_in_use; i < NTXBLOCKS; i++) {
		wv_cu_show_one(dev, lp, i, p);

		p += TXBLOCKZ;
		if (p >= OFFSET_CU + NTXBLOCKS * TXBLOCKZ)
			p -= NTXBLOCKS * TXBLOCKZ;
	}
	printk("\n");
}
#endif				

#ifdef DEBUG_DEVICE_SHOW


static void wv_dev_show(struct net_device * dev)
{
	printk(KERN_DEBUG "dev:");
	printk(" state=%lX,", dev->state);
	printk(" trans_start=%ld,", dev->trans_start);
	printk(" flags=0x%x,", dev->flags);
	printk("\n");
}				



static void wv_local_show(struct net_device * dev)
{
	net_local *lp;

	lp = netdev_priv(dev);

	printk(KERN_DEBUG "local:");
	printk(" tx_n_in_use=%d,", lp->tx_n_in_use);
	printk(" hacr=0x%x,", lp->hacr);
	printk(" rx_head=0x%x,", lp->rx_head);
	printk(" rx_last=0x%x,", lp->rx_last);
	printk(" tx_first_free=0x%x,", lp->tx_first_free);
	printk(" tx_first_in_use=0x%x,", lp->tx_first_in_use);
	printk("\n");
}				
#endif				

#if defined(DEBUG_RX_INFO) || defined(DEBUG_TX_INFO)


static inline void wv_packet_info(u8 * p,	
				  int length,	
				  char *msg1,	
				  char *msg2)
{				
	int i;
	int maxi;

	printk(KERN_DEBUG
	       "%s: %s(): dest %pM, length %d\n",
	       msg1, msg2, p, length);
	printk(KERN_DEBUG
	       "%s: %s(): src %pM, type 0x%02X%02X\n",
	       msg1, msg2, &p[6], p[12], p[13]);

#ifdef DEBUG_PACKET_DUMP

	printk(KERN_DEBUG "data=\"");

	if ((maxi = length) > DEBUG_PACKET_DUMP)
		maxi = DEBUG_PACKET_DUMP;
	for (i = 14; i < maxi; i++)
		if (p[i] >= ' ' && p[i] <= '~')
			printk(" %c", p[i]);
		else
			printk("%02X", p[i]);
	if (maxi < length)
		printk("..");
	printk("\"\n");
	printk(KERN_DEBUG "\n");
#endif				
}
#endif				



static void wv_init_info(struct net_device * dev)
{
	short ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	psa_t psa;

	
	psa_read(ioaddr, lp->hacr, 0, (unsigned char *) &psa, sizeof(psa));

#ifdef DEBUG_PSA_SHOW
	wv_psa_show(&psa);
#endif
#ifdef DEBUG_MMC_SHOW
	wv_mmc_show(dev);
#endif
#ifdef DEBUG_I82586_SHOW
	wv_cu_show(dev);
#endif

#ifdef DEBUG_BASIC_SHOW
	
	printk(KERN_NOTICE "%s: WaveLAN at %#x, %pM, IRQ %d",
	       dev->name, ioaddr, dev->dev_addr, dev->irq);

	
	if (psa.psa_nwid_select)
		printk(", nwid 0x%02X-%02X", psa.psa_nwid[0],
		       psa.psa_nwid[1]);
	else
		printk(", nwid off");

	
	if (!(mmc_in(ioaddr, mmroff(0, mmr_fee_status)) &
	      (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY))) {
		unsigned short freq;

		
		fee_read(ioaddr, 0x00, &freq, 1);

		
		printk(", 2.00, %ld", (freq >> 6) + 2400L);

		
		if (freq & 0x20)
			printk(".5");
	} else {
		printk(", PC");
		switch (psa.psa_comp_number) {
		case PSA_COMP_PC_AT_915:
		case PSA_COMP_PC_AT_2400:
			printk("-AT");
			break;
		case PSA_COMP_PC_MC_915:
		case PSA_COMP_PC_MC_2400:
			printk("-MC");
			break;
		case PSA_COMP_PCMCIA_915:
			printk("MCIA");
			break;
		default:
			printk("?");
		}
		printk(", ");
		switch (psa.psa_subband) {
		case PSA_SUBBAND_915:
			printk("915");
			break;
		case PSA_SUBBAND_2425:
			printk("2425");
			break;
		case PSA_SUBBAND_2460:
			printk("2460");
			break;
		case PSA_SUBBAND_2484:
			printk("2484");
			break;
		case PSA_SUBBAND_2430_5:
			printk("2430.5");
			break;
		default:
			printk("?");
		}
	}

	printk(" MHz\n");
#endif				

#ifdef DEBUG_VERSION_SHOW
	
	printk(KERN_NOTICE "%s", version);
#endif
}				







static void wavelan_set_multicast_list(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);

#ifdef DEBUG_IOCTL_TRACE
	printk(KERN_DEBUG "%s: ->wavelan_set_multicast_list()\n",
	       dev->name);
#endif

#ifdef DEBUG_IOCTL_INFO
	printk(KERN_DEBUG
	       "%s: wavelan_set_multicast_list(): setting Rx mode %02X to %d addresses.\n",
	       dev->name, dev->flags, dev->mc_count);
#endif

	
	if ((dev->flags & IFF_PROMISC) ||
	    (dev->flags & IFF_ALLMULTI) ||
	    (dev->mc_count > I82586_MAX_MULTICAST_ADDRESSES)) {
		
		if (!lp->promiscuous) {
			lp->promiscuous = 1;
			lp->mc_count = 0;

			wv_82586_reconfig(dev);
		}
	} else
		
	if (dev->mc_list != (struct dev_mc_list *) NULL) {
		
#ifdef MULTICAST_AVOID
		if (lp->promiscuous || (dev->mc_count != lp->mc_count))
#endif
		{
			lp->promiscuous = 0;
			lp->mc_count = dev->mc_count;

			wv_82586_reconfig(dev);
		}
	} else {
		
		if (lp->promiscuous || lp->mc_count == 0) {
			lp->promiscuous = 0;
			lp->mc_count = 0;

			wv_82586_reconfig(dev);
		}
	}
#ifdef DEBUG_IOCTL_TRACE
	printk(KERN_DEBUG "%s: <-wavelan_set_multicast_list()\n",
	       dev->name);
#endif
}



#ifdef SET_MAC_ADDRESS
static int wavelan_set_mac_address(struct net_device * dev, void *addr)
{
	struct sockaddr *mac = addr;

	
	memcpy(dev->dev_addr, mac->sa_data, WAVELAN_ADDR_SIZE);

	
	wv_82586_reconfig(dev);

	return 0;
}
#endif				




static int wv_set_frequency(unsigned long ioaddr,	
				   iw_freq * frequency)
{
	const int BAND_NUM = 10;	
	long freq = 0L;		
#ifdef DEBUG_IOCTL_INFO
	int i;
#endif

	
	
	if ((frequency->e == 1) &&
	    (frequency->m >= (int) 2.412e8)
	    && (frequency->m <= (int) 2.487e8)) {
		freq = ((frequency->m / 10000) - 24000L) / 5;
	}

	
	
	if ((frequency->e == 0) && (frequency->m < BAND_NUM)) {
		
		freq = channel_bands[frequency->m] >> 1;
	}

	
	if (freq != 0L) {
		u16 table[10];	

		
		fee_read(ioaddr, 0x71, table, 10);

#ifdef DEBUG_IOCTL_INFO
		printk(KERN_DEBUG "Frequency table: ");
		for (i = 0; i < 10; i++) {
			printk(" %04X", table[i]);
		}
		printk("\n");
#endif

		
		if (!(table[9 - ((freq - 24) / 16)] &
		      (1 << ((freq - 24) % 16)))) return -EINVAL;	
	} else
		return -EINVAL;

	
	if (freq != 0L) {
		unsigned short area[16];
		unsigned short dac[2];
		unsigned short area_verify[16];
		unsigned short dac_verify[2];
		
		unsigned short power_limit[] = { 40, 80, 120, 160, 0 };
		int power_band = 0;	
		unsigned short power_adjust;	

		
		power_band = 0;
		while ((freq > power_limit[power_band]) &&
		       (power_limit[++power_band] != 0));

		
		fee_read(ioaddr, 0x00, area, 16);

		
		fee_read(ioaddr, 0x60, dac, 2);

		
		fee_read(ioaddr, 0x6B - (power_band >> 1), &power_adjust,
			 1);
		if (power_band & 0x1)
			power_adjust >>= 8;
		else
			power_adjust &= 0xFF;

#ifdef DEBUG_IOCTL_INFO
		printk(KERN_DEBUG "WaveLAN EEPROM Area 1: ");
		for (i = 0; i < 16; i++) {
			printk(" %04X", area[i]);
		}
		printk("\n");

		printk(KERN_DEBUG "WaveLAN EEPROM DAC: %04X %04X\n",
		       dac[0], dac[1]);
#endif

		
		area[0] = ((freq << 5) & 0xFFE0) | (area[0] & 0x1F);

		
		area[3] = (freq >> 1) + 2400L - 352L;
		area[2] = ((freq & 0x1) << 4) | (area[2] & 0xFFEF);

		
		area[13] = (freq >> 1) + 2400L;
		area[12] = ((freq & 0x1) << 4) | (area[2] & 0xFFEF);

		

		
		dac[1] = ((power_adjust >> 1) & 0x7F) | (dac[1] & 0xFF80);
		dac[0] = ((power_adjust & 0x1) << 4) | (dac[0] & 0xFFEF);

		
		fee_write(ioaddr, 0x00, area, 16);

		
		fee_write(ioaddr, 0x60, dac, 2);

		

		
		fee_read(ioaddr, 0x00, area_verify, 16);

		
		fee_read(ioaddr, 0x60, dac_verify, 2);

		
		if (memcmp(area, area_verify, 16 * 2) ||
		    memcmp(dac, dac_verify, 2 * 2)) {
#ifdef DEBUG_IOCTL_ERROR
			printk(KERN_INFO
			       "WaveLAN: wv_set_frequency: unable to write new frequency to EEPROM(?).\n");
#endif
			return -EOPNOTSUPP;
		}

		
		mmc_out(ioaddr, mmwoff(0, mmw_fee_addr), 0x0F);
		mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl),
			MMW_FEE_CTRL_READ | MMW_FEE_CTRL_DWLD);

		
		fee_wait(ioaddr, 100, 100);

		
		mmc_out(ioaddr, mmwoff(0, mmw_fee_addr), 0x61);
		mmc_out(ioaddr, mmwoff(0, mmw_fee_ctrl),
			MMW_FEE_CTRL_READ | MMW_FEE_CTRL_DWLD);

		
		fee_wait(ioaddr, 100, 100);

#ifdef DEBUG_IOCTL_INFO
		

		printk(KERN_DEBUG "WaveLAN EEPROM Area 1: ");
		for (i = 0; i < 16; i++) {
			printk(" %04X", area_verify[i]);
		}
		printk("\n");

		printk(KERN_DEBUG "WaveLAN EEPROM DAC:  %04X %04X\n",
		       dac_verify[0], dac_verify[1]);
#endif

		return 0;
	} else
		return -EINVAL;	
}



static int wv_frequency_list(unsigned long ioaddr,	
				    iw_freq * list,	
				    int max)
{				
	u16 table[10];	
	long freq = 0L;		
	int i;			
	int c = 0;		

	
	fee_read(ioaddr, 0x71  , table, 10);

	
	i = 0;
	for (freq = 0; freq < 150; freq++)
		
		if (table[9 - (freq / 16)] & (1 << (freq % 16))) {
			
			while ((c < ARRAY_SIZE(channel_bands)) &&
				(((channel_bands[c] >> 1) - 24) < freq)) 
				c++;
			list[i].i = c;	

			
			list[i].m = (((freq + 24) * 5) + 24000L) * 10000;
			list[i++].e = 1;

			
			if (i >= max)
				return (i);
		}

	return (i);
}

#ifdef IW_WIRELESS_SPY


static inline void wl_spy_gather(struct net_device * dev,
				 u8 *	mac,	
				 u8 *	stats)	
{
	struct iw_quality wstats;

	wstats.qual = stats[2] & MMR_SGNL_QUAL;
	wstats.level = stats[0] & MMR_SIGNAL_LVL;
	wstats.noise = stats[1] & MMR_SILENCE_LVL;
	wstats.updated = 0x7;

	
	wireless_spy_update(dev, mac, &wstats);
}
#endif 

#ifdef HISTOGRAM


static inline void wl_his_gather(struct net_device * dev, u8 * stats)
{				
	net_local *lp = netdev_priv(dev);
	u8 level = stats[0] & MMR_SIGNAL_LVL;
	int i;

	
	i = 0;
	while ((i < (lp->his_number - 1))
	       && (level >= lp->his_range[i++]));

	
	(lp->his_sum[i])++;
}
#endif 



static int wavelan_get_name(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	strcpy(wrqu->name, "WaveLAN");
	return 0;
}



static int wavelan_set_nwid(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	psa_t psa;
	mm_t m;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if (!wrqu->nwid.disabled) {
		
		psa.psa_nwid[0] = (wrqu->nwid.value & 0xFF00) >> 8;
		psa.psa_nwid[1] = wrqu->nwid.value & 0xFF;
		psa.psa_nwid_select = 0x01;
		psa_write(ioaddr, lp->hacr,
			  (char *) psa.psa_nwid - (char *) &psa,
			  (unsigned char *) psa.psa_nwid, 3);

		
		m.w.mmw_netw_id_l = psa.psa_nwid[1];
		m.w.mmw_netw_id_h = psa.psa_nwid[0];
		mmc_write(ioaddr,
			  (char *) &m.w.mmw_netw_id_l -
			  (char *) &m,
			  (unsigned char *) &m.w.mmw_netw_id_l, 2);
		mmc_out(ioaddr, mmwoff(0, mmw_loopt_sel), 0x00);
	} else {
		
		psa.psa_nwid_select = 0x00;
		psa_write(ioaddr, lp->hacr,
			  (char *) &psa.psa_nwid_select -
			  (char *) &psa,
			  (unsigned char *) &psa.psa_nwid_select,
			  1);

		
		mmc_out(ioaddr, mmwoff(0, mmw_loopt_sel),
			MMW_LOOPT_SEL_DIS_NWID);
	}
	
	update_psa_checksum(dev, ioaddr, lp->hacr);

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_nwid(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	psa_read(ioaddr, lp->hacr,
		 (char *) psa.psa_nwid - (char *) &psa,
		 (unsigned char *) psa.psa_nwid, 3);
	wrqu->nwid.value = (psa.psa_nwid[0] << 8) + psa.psa_nwid[1];
	wrqu->nwid.disabled = !(psa.psa_nwid_select);
	wrqu->nwid.fixed = 1;	

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_set_freq(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	unsigned long flags;
	int ret;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if (!(mmc_in(ioaddr, mmroff(0, mmr_fee_status)) &
	      (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY)))
		ret = wv_set_frequency(ioaddr, &(wrqu->freq));
	else
		ret = -EOPNOTSUPP;

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_freq(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if (!(mmc_in(ioaddr, mmroff(0, mmr_fee_status)) &
	      (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY))) {
		unsigned short freq;

		
		fee_read(ioaddr, 0x00, &freq, 1);
		wrqu->freq.m = ((freq >> 5) * 5 + 24000L) * 10000;
		wrqu->freq.e = 1;
	} else {
		psa_read(ioaddr, lp->hacr,
			 (char *) &psa.psa_subband - (char *) &psa,
			 (unsigned char *) &psa.psa_subband, 1);

		if (psa.psa_subband <= 4) {
			wrqu->freq.m = fixed_bands[psa.psa_subband];
			wrqu->freq.e = (psa.psa_subband != 0);
		} else
			ret = -EOPNOTSUPP;
	}

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_set_sens(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	
	psa.psa_thr_pre_set = wrqu->sens.value & 0x3F;
	psa_write(ioaddr, lp->hacr,
		  (char *) &psa.psa_thr_pre_set - (char *) &psa,
		  (unsigned char *) &psa.psa_thr_pre_set, 1);
	
	update_psa_checksum(dev, ioaddr, lp->hacr);
	mmc_out(ioaddr, mmwoff(0, mmw_thr_pre_set),
		psa.psa_thr_pre_set);

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_sens(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	psa_read(ioaddr, lp->hacr,
		 (char *) &psa.psa_thr_pre_set - (char *) &psa,
		 (unsigned char *) &psa.psa_thr_pre_set, 1);
	wrqu->sens.value = psa.psa_thr_pre_set & 0x3F;
	wrqu->sens.fixed = 1;

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_set_encode(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu,
			      char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	unsigned long flags;
	psa_t psa;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);

	
	if (!mmc_encr(ioaddr)) {
		ret = -EOPNOTSUPP;
	}

	
	if((wrqu->encoding.length != 8) && (wrqu->encoding.length != 0)) {
		ret = -EINVAL;
	}

	if(!ret) {
		
		if (wrqu->encoding.length == 8) {
			
			memcpy(psa.psa_encryption_key, extra,
			       wrqu->encoding.length);
			psa.psa_encryption_select = 1;

			psa_write(ioaddr, lp->hacr,
				  (char *) &psa.psa_encryption_select -
				  (char *) &psa,
				  (unsigned char *) &psa.
				  psa_encryption_select, 8 + 1);

			mmc_out(ioaddr, mmwoff(0, mmw_encr_enable),
				MMW_ENCR_ENABLE_EN | MMW_ENCR_ENABLE_MODE);
			mmc_write(ioaddr, mmwoff(0, mmw_encr_key),
				  (unsigned char *) &psa.
				  psa_encryption_key, 8);
		}

		
		if (wrqu->encoding.flags & IW_ENCODE_DISABLED) {
			psa.psa_encryption_select = 0;
			psa_write(ioaddr, lp->hacr,
				  (char *) &psa.psa_encryption_select -
				  (char *) &psa,
				  (unsigned char *) &psa.
				  psa_encryption_select, 1);

			mmc_out(ioaddr, mmwoff(0, mmw_encr_enable), 0);
		}
		
		update_psa_checksum(dev, ioaddr, lp->hacr);
	}

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_encode(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu,
			      char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if (!mmc_encr(ioaddr)) {
		ret = -EOPNOTSUPP;
	} else {
		
		psa_read(ioaddr, lp->hacr,
			 (char *) &psa.psa_encryption_select -
			 (char *) &psa,
			 (unsigned char *) &psa.
			 psa_encryption_select, 1 + 8);

		
		if (psa.psa_encryption_select)
			wrqu->encoding.flags = IW_ENCODE_ENABLED;
		else
			wrqu->encoding.flags = IW_ENCODE_DISABLED;
		wrqu->encoding.flags |= mmc_encr(ioaddr);

		
		wrqu->encoding.length = 8;
		memcpy(extra, psa.psa_encryption_key, wrqu->encoding.length);
	}

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_range(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	struct iw_range *range = (struct iw_range *) extra;
	unsigned long flags;
	int ret = 0;

	
	wrqu->data.length = sizeof(struct iw_range);

	
	memset(range, 0, sizeof(struct iw_range));

	
	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 9;

	
	range->throughput = 1.6 * 1000 * 1000;	
	range->min_nwid = 0x0000;
	range->max_nwid = 0xFFFF;

	range->sensitivity = 0x3F;
	range->max_qual.qual = MMR_SGNL_QUAL;
	range->max_qual.level = MMR_SIGNAL_LVL;
	range->max_qual.noise = MMR_SILENCE_LVL;
	range->avg_qual.qual = MMR_SGNL_QUAL; 
	
	range->avg_qual.level = 30;
	range->avg_qual.noise = 8;

	range->num_bitrates = 1;
	range->bitrate[0] = 2000000;	

	
	range->event_capa[0] = (IW_EVENT_CAPA_MASK(0x8B02) |
				IW_EVENT_CAPA_MASK(0x8B04));
	range->event_capa[1] = IW_EVENT_CAPA_K_1;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if (!(mmc_in(ioaddr, mmroff(0, mmr_fee_status)) &
	      (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY))) {
		range->num_channels = 10;
		range->num_frequency = wv_frequency_list(ioaddr, range->freq,
							IW_MAX_FREQUENCIES);
	} else
		range->num_channels = range->num_frequency = 0;

	
	if (mmc_encr(ioaddr)) {
		range->encoding_size[0] = 8;	
		range->num_encoding_sizes = 1;
		range->max_encoding_tokens = 1;	
	} else {
		range->num_encoding_sizes = 0;
		range->max_encoding_tokens = 0;
	}

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_set_qthr(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	psa_t psa;
	unsigned long flags;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	psa.psa_quality_thr = *(extra) & 0x0F;
	psa_write(ioaddr, lp->hacr,
		  (char *) &psa.psa_quality_thr - (char *) &psa,
		  (unsigned char *) &psa.psa_quality_thr, 1);
	
	update_psa_checksum(dev, ioaddr, lp->hacr);
	mmc_out(ioaddr, mmwoff(0, mmw_quality_thr),
		psa.psa_quality_thr);

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return 0;
}



static int wavelan_get_qthr(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);	
	psa_t psa;
	unsigned long flags;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	psa_read(ioaddr, lp->hacr,
		 (char *) &psa.psa_quality_thr - (char *) &psa,
		 (unsigned char *) &psa.psa_quality_thr, 1);
	*(extra) = psa.psa_quality_thr & 0x0F;

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return 0;
}

#ifdef HISTOGRAM


static int wavelan_set_histo(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	net_local *lp = netdev_priv(dev);	

	
	if (wrqu->data.length > 16) {
		return(-E2BIG);
	}

	
	lp->his_number = 0;

	
	if (wrqu->data.length > 0) {
		
		memcpy(lp->his_range, extra, wrqu->data.length);

		{
		  int i;
		  printk(KERN_DEBUG "Histo :");
		  for(i = 0; i < wrqu->data.length; i++)
		    printk(" %d", lp->his_range[i]);
		  printk("\n");
		}

		
		memset(lp->his_sum, 0x00, sizeof(long) * 16);
	}

	
	lp->his_number = wrqu->data.length;

	return(0);
}



static int wavelan_get_histo(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	net_local *lp = netdev_priv(dev);	

	
	wrqu->data.length = lp->his_number;

	
	if(lp->his_number > 0)
		memcpy(extra, lp->his_sum, sizeof(long) * lp->his_number);

	return(0);
}
#endif			




static const iw_handler		wavelan_handler[] =
{
	NULL,				
	wavelan_get_name,		
	wavelan_set_nwid,		
	wavelan_get_nwid,		
	wavelan_set_freq,		
	wavelan_get_freq,		
	NULL,				
	NULL,				
	wavelan_set_sens,		
	wavelan_get_sens,		
	NULL,				
	wavelan_get_range,		
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	iw_handler_set_spy,		
	iw_handler_get_spy,		
	iw_handler_set_thrspy,		
	iw_handler_get_thrspy,		
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	
	wavelan_set_encode,		
	wavelan_get_encode,		
};

static const iw_handler		wavelan_private_handler[] =
{
	wavelan_set_qthr,		
	wavelan_get_qthr,		
#ifdef HISTOGRAM
	wavelan_set_histo,		
	wavelan_get_histo,		
#endif	
};

static const struct iw_priv_args wavelan_private_args[] = {

  { SIOCSIPQTHR, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, 0, "setqualthr" },
  { SIOCGIPQTHR, 0, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, "getqualthr" },
  { SIOCSIPHISTO, IW_PRIV_TYPE_BYTE | 16,                    0, "sethisto" },
  { SIOCGIPHISTO, 0,                     IW_PRIV_TYPE_INT | 16, "gethisto" },
};

static const struct iw_handler_def	wavelan_handler_def =
{
	.num_standard	= ARRAY_SIZE(wavelan_handler),
	.num_private	= ARRAY_SIZE(wavelan_private_handler),
	.num_private_args = ARRAY_SIZE(wavelan_private_args),
	.standard	= wavelan_handler,
	.private	= wavelan_private_handler,
	.private_args	= wavelan_private_args,
	.get_wireless_stats = wavelan_get_wireless_stats,
};



static iw_stats *wavelan_get_wireless_stats(struct net_device * dev)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	mmr_t m;
	iw_stats *wstats;
	unsigned long flags;

#ifdef DEBUG_IOCTL_TRACE
	printk(KERN_DEBUG "%s: ->wavelan_get_wireless_stats()\n",
	       dev->name);
#endif

	
	if (lp == (net_local *) NULL)
		return (iw_stats *) NULL;
	
	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	wstats = &lp->wstats;

	
	mmc_out(ioaddr, mmwoff(0, mmw_freeze), 1);

	mmc_read(ioaddr, mmroff(0, mmr_dce_status), &m.mmr_dce_status, 1);
	mmc_read(ioaddr, mmroff(0, mmr_wrong_nwid_l), &m.mmr_wrong_nwid_l,
		 2);
	mmc_read(ioaddr, mmroff(0, mmr_thr_pre_set), &m.mmr_thr_pre_set,
		 4);

	mmc_out(ioaddr, mmwoff(0, mmw_freeze), 0);

	
	wstats->status = m.mmr_dce_status & MMR_DCE_STATUS;
	wstats->qual.qual = m.mmr_sgnl_qual & MMR_SGNL_QUAL;
	wstats->qual.level = m.mmr_signal_lvl & MMR_SIGNAL_LVL;
	wstats->qual.noise = m.mmr_silence_lvl & MMR_SILENCE_LVL;
	wstats->qual.updated = (((m. mmr_signal_lvl & MMR_SIGNAL_LVL_VALID) >> 7) 
			| ((m.mmr_signal_lvl & MMR_SIGNAL_LVL_VALID) >> 6) 
			| ((m.mmr_silence_lvl & MMR_SILENCE_LVL_VALID) >> 5));
	wstats->discard.nwid += (m.mmr_wrong_nwid_h << 8) | m.mmr_wrong_nwid_l;
	wstats->discard.code = 0L;
	wstats->discard.misc = 0L;

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

#ifdef DEBUG_IOCTL_TRACE
	printk(KERN_DEBUG "%s: <-wavelan_get_wireless_stats()\n",
	       dev->name);
#endif
	return &lp->wstats;
}






static void
wv_packet_read(struct net_device * dev, u16 buf_off, int sksize)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	struct sk_buff *skb;

#ifdef DEBUG_RX_TRACE
	printk(KERN_DEBUG "%s: ->wv_packet_read(0x%X, %d)\n",
	       dev->name, buf_off, sksize);
#endif

	
	if ((skb = dev_alloc_skb(sksize)) == (struct sk_buff *) NULL) {
#ifdef DEBUG_RX_ERROR
		printk(KERN_INFO
		       "%s: wv_packet_read(): could not alloc_skb(%d, GFP_ATOMIC).\n",
		       dev->name, sksize);
#endif
		dev->stats.rx_dropped++;
		return;
	}

	
	obram_read(ioaddr, buf_off, skb_put(skb, sksize), sksize);
	skb->protocol = eth_type_trans(skb, dev);

#ifdef DEBUG_RX_INFO
	wv_packet_info(skb_mac_header(skb), sksize, dev->name,
		       "wv_packet_read");
#endif				

	
	if (
#ifdef IW_WIRELESS_SPY		
		   (lp->spy_data.spy_number > 0) ||
#endif 
#ifdef HISTOGRAM
		   (lp->his_number > 0) ||
#endif 
		   0) {
		u8 stats[3];	

		
		
		mmc_out(ioaddr, mmwoff(0, mmw_freeze), 1);
		mmc_read(ioaddr, mmroff(0, mmr_signal_lvl), stats, 3);
		mmc_out(ioaddr, mmwoff(0, mmw_freeze), 0);

#ifdef DEBUG_RX_INFO
		printk(KERN_DEBUG
		       "%s: wv_packet_read(): Signal level %d/63, Silence level %d/63, signal quality %d/16\n",
		       dev->name, stats[0] & 0x3F, stats[1] & 0x3F,
		       stats[2] & 0x0F);
#endif

		
#ifdef IW_WIRELESS_SPY
		wl_spy_gather(dev, skb_mac_header(skb) + WAVELAN_ADDR_SIZE,
			      stats);
#endif 
#ifdef HISTOGRAM
		wl_his_gather(dev, stats);
#endif 
	}

	
	netif_rx(skb);

	
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += sksize;

#ifdef DEBUG_RX_TRACE
	printk(KERN_DEBUG "%s: <-wv_packet_read()\n", dev->name);
#endif
}



static void wv_receive(struct net_device * dev)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	fd_t fd;
	rbd_t rbd;
	int nreaped = 0;

#ifdef DEBUG_RX_TRACE
	printk(KERN_DEBUG "%s: ->wv_receive()\n", dev->name);
#endif

	
	for (;;) {
		obram_read(ioaddr, lp->rx_head, (unsigned char *) &fd,
			   sizeof(fd));

		

		
		if ((fd.fd_status & FD_STATUS_C) != FD_STATUS_C)
			break;	

		nreaped++;

		
		if ((fd.fd_status & FD_STATUS_OK) == FD_STATUS_OK) {
			
			if (fd.fd_rbd_offset != I82586NULL) {
				
				obram_read(ioaddr, fd.fd_rbd_offset,
					   (unsigned char *) &rbd,
					   sizeof(rbd));

#ifdef DEBUG_RX_ERROR
				if ((rbd.rbd_status & RBD_STATUS_EOF) !=
				    RBD_STATUS_EOF) printk(KERN_INFO
							   "%s: wv_receive(): missing EOF flag.\n",
							   dev->name);

				if ((rbd.rbd_status & RBD_STATUS_F) !=
				    RBD_STATUS_F) printk(KERN_INFO
							 "%s: wv_receive(): missing F flag.\n",
							 dev->name);
#endif				

				
				wv_packet_read(dev, rbd.rbd_bufl,
					       rbd.
					       rbd_status &
					       RBD_STATUS_ACNT);
			}
#ifdef DEBUG_RX_ERROR
			else	
				printk(KERN_INFO
				       "%s: wv_receive(): frame has no data.\n",
				       dev->name);
#endif
		} else {	

			dev->stats.rx_errors++;

#ifdef DEBUG_RX_INFO
			printk(KERN_DEBUG
			       "%s: wv_receive(): frame not received successfully (%X).\n",
			       dev->name, fd.fd_status);
#endif

#ifdef DEBUG_RX_ERROR
			if ((fd.fd_status & FD_STATUS_S6) != 0)
				printk(KERN_INFO
				       "%s: wv_receive(): no EOF flag.\n",
				       dev->name);
#endif

			if ((fd.fd_status & FD_STATUS_S7) != 0) {
				dev->stats.rx_length_errors++;
#ifdef DEBUG_RX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_receive(): frame too short.\n",
				       dev->name);
#endif
			}

			if ((fd.fd_status & FD_STATUS_S8) != 0) {
				dev->stats.rx_over_errors++;
#ifdef DEBUG_RX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_receive(): rx DMA overrun.\n",
				       dev->name);
#endif
			}

			if ((fd.fd_status & FD_STATUS_S9) != 0) {
				dev->stats.rx_fifo_errors++;
#ifdef DEBUG_RX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_receive(): ran out of resources.\n",
				       dev->name);
#endif
			}

			if ((fd.fd_status & FD_STATUS_S10) != 0) {
				dev->stats.rx_frame_errors++;
#ifdef DEBUG_RX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_receive(): alignment error.\n",
				       dev->name);
#endif
			}

			if ((fd.fd_status & FD_STATUS_S11) != 0) {
				dev->stats.rx_crc_errors++;
#ifdef DEBUG_RX_FAIL
				printk(KERN_DEBUG
				       "%s: wv_receive(): CRC error.\n",
				       dev->name);
#endif
			}
		}

		fd.fd_status = 0;
		obram_write(ioaddr, fdoff(lp->rx_head, fd_status),
			    (unsigned char *) &fd.fd_status,
			    sizeof(fd.fd_status));

		fd.fd_command = FD_COMMAND_EL;
		obram_write(ioaddr, fdoff(lp->rx_head, fd_command),
			    (unsigned char *) &fd.fd_command,
			    sizeof(fd.fd_command));

		fd.fd_command = 0;
		obram_write(ioaddr, fdoff(lp->rx_last, fd_command),
			    (unsigned char *) &fd.fd_command,
			    sizeof(fd.fd_command));

		lp->rx_last = lp->rx_head;
		lp->rx_head = fd.fd_link_offset;
	}			

#ifdef DEBUG_RX_INFO
	if (nreaped > 1)
		printk(KERN_DEBUG "%s: wv_receive(): reaped %d\n",
		       dev->name, nreaped);
#endif
#ifdef DEBUG_RX_TRACE
	printk(KERN_DEBUG "%s: <-wv_receive()\n", dev->name);
#endif
}






static int wv_packet_write(struct net_device * dev, void *buf, short length)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	unsigned short txblock;
	unsigned short txpred;
	unsigned short tx_addr;
	unsigned short nop_addr;
	unsigned short tbd_addr;
	unsigned short buf_addr;
	ac_tx_t tx;
	ac_nop_t nop;
	tbd_t tbd;
	int clen = length;
	unsigned long flags;

#ifdef DEBUG_TX_TRACE
	printk(KERN_DEBUG "%s: ->wv_packet_write(%d)\n", dev->name,
	       length);
#endif

	spin_lock_irqsave(&lp->spinlock, flags);

	
	if (lp->tx_n_in_use == (NTXBLOCKS - 1)) {
#ifdef DEBUG_TX_ERROR
		printk(KERN_INFO "%s: wv_packet_write(): Tx queue full.\n",
		       dev->name);
#endif
		spin_unlock_irqrestore(&lp->spinlock, flags);
		return 1;
	}

	
	txblock = lp->tx_first_free;
	txpred = txblock - TXBLOCKZ;
	if (txpred < OFFSET_CU)
		txpred += NTXBLOCKS * TXBLOCKZ;
	lp->tx_first_free += TXBLOCKZ;
	if (lp->tx_first_free >= OFFSET_CU + NTXBLOCKS * TXBLOCKZ)
		lp->tx_first_free -= NTXBLOCKS * TXBLOCKZ;

	lp->tx_n_in_use++;

	
	tx_addr = txblock;
	nop_addr = tx_addr + sizeof(tx);
	tbd_addr = nop_addr + sizeof(nop);
	buf_addr = tbd_addr + sizeof(tbd);

	
	tx.tx_h.ac_status = 0;
	obram_write(ioaddr, toff(ac_tx_t, tx_addr, tx_h.ac_status),
		    (unsigned char *) &tx.tx_h.ac_status,
		    sizeof(tx.tx_h.ac_status));

	
	nop.nop_h.ac_status = 0;
	obram_write(ioaddr, toff(ac_nop_t, nop_addr, nop_h.ac_status),
		    (unsigned char *) &nop.nop_h.ac_status,
		    sizeof(nop.nop_h.ac_status));
	nop.nop_h.ac_link = nop_addr;
	obram_write(ioaddr, toff(ac_nop_t, nop_addr, nop_h.ac_link),
		    (unsigned char *) &nop.nop_h.ac_link,
		    sizeof(nop.nop_h.ac_link));

	
	tbd.tbd_status = TBD_STATUS_EOF | (TBD_STATUS_ACNT & clen);
	tbd.tbd_next_bd_offset = I82586NULL;
	tbd.tbd_bufl = buf_addr;
	tbd.tbd_bufh = 0;
	obram_write(ioaddr, tbd_addr, (unsigned char *) &tbd, sizeof(tbd));

	
	obram_write(ioaddr, buf_addr, buf, length);

	
	nop_addr = txpred + sizeof(tx);
	nop.nop_h.ac_status = 0;
	obram_write(ioaddr, toff(ac_nop_t, nop_addr, nop_h.ac_status),
		    (unsigned char *) &nop.nop_h.ac_status,
		    sizeof(nop.nop_h.ac_status));
	nop.nop_h.ac_link = txblock;
	obram_write(ioaddr, toff(ac_nop_t, nop_addr, nop_h.ac_link),
		    (unsigned char *) &nop.nop_h.ac_link,
		    sizeof(nop.nop_h.ac_link));

	
	dev->trans_start = jiffies;

	
	dev->stats.tx_bytes += length;

	if (lp->tx_first_in_use == I82586NULL)
		lp->tx_first_in_use = txblock;

	if (lp->tx_n_in_use < NTXBLOCKS - 1)
		netif_wake_queue(dev);

	spin_unlock_irqrestore(&lp->spinlock, flags);
	
#ifdef DEBUG_TX_INFO
	wv_packet_info((u8 *) buf, length, dev->name,
		       "wv_packet_write");
#endif				

#ifdef DEBUG_TX_TRACE
	printk(KERN_DEBUG "%s: <-wv_packet_write()\n", dev->name);
#endif

	return 0;
}



static netdev_tx_t wavelan_packet_xmit(struct sk_buff *skb,
					     struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long flags;
	char data[ETH_ZLEN];

#ifdef DEBUG_TX_TRACE
	printk(KERN_DEBUG "%s: ->wavelan_packet_xmit(0x%X)\n", dev->name,
	       (unsigned) skb);
#endif

	
	netif_stop_queue(dev);

	
	if (lp->reconfig_82586) {
		spin_lock_irqsave(&lp->spinlock, flags);
		wv_82586_config(dev);
		spin_unlock_irqrestore(&lp->spinlock, flags);
		
		if (lp->tx_n_in_use == (NTXBLOCKS - 1))
			return NETDEV_TX_BUSY;
	}

	
	
	if (skb->len < ETH_ZLEN) {
		memset(data, 0, ETH_ZLEN);
		skb_copy_from_linear_data(skb, data, skb->len);
		
		if(wv_packet_write(dev, data, ETH_ZLEN))
			return NETDEV_TX_BUSY;	
	}
	else if(wv_packet_write(dev, skb->data, skb->len))
		return NETDEV_TX_BUSY;	


	dev_kfree_skb(skb);

#ifdef DEBUG_TX_TRACE
	printk(KERN_DEBUG "%s: <-wavelan_packet_xmit()\n", dev->name);
#endif
	return NETDEV_TX_OK;
}






static int wv_mmc_init(struct net_device * dev)
{
	unsigned long ioaddr = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	psa_t psa;
	mmw_t m;
	int configured;

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: ->wv_mmc_init()\n", dev->name);
#endif

	
	psa_read(ioaddr, lp->hacr, 0, (unsigned char *) &psa, sizeof(psa));

#ifdef USE_PSA_CONFIG
	configured = psa.psa_conf_status & 1;
#else
	configured = 0;
#endif

	
	if (!configured) {
		
		psa.psa_nwid[0] = 0;
		psa.psa_nwid[1] = 0;

		
		psa.psa_nwid_select = 0;

		
		psa.psa_encryption_select = 0;

		
		if (psa.psa_comp_number & 1)
			psa.psa_thr_pre_set = 0x01;
		else
			psa.psa_thr_pre_set = 0x04;
		psa.psa_quality_thr = 0x03;

		
		psa.psa_conf_status |= 1;

#ifdef USE_PSA_CONFIG
		
		psa_write(ioaddr, lp->hacr,
			  (char *) psa.psa_nwid - (char *) &psa,
			  (unsigned char *) psa.psa_nwid, 4);
		psa_write(ioaddr, lp->hacr,
			  (char *) &psa.psa_thr_pre_set - (char *) &psa,
			  (unsigned char *) &psa.psa_thr_pre_set, 1);
		psa_write(ioaddr, lp->hacr,
			  (char *) &psa.psa_quality_thr - (char *) &psa,
			  (unsigned char *) &psa.psa_quality_thr, 1);
		psa_write(ioaddr, lp->hacr,
			  (char *) &psa.psa_conf_status - (char *) &psa,
			  (unsigned char *) &psa.psa_conf_status, 1);
		
		update_psa_checksum(dev, ioaddr, lp->hacr);
#endif
	}

	
	memset(&m, 0x00, sizeof(m));

	
	m.mmw_netw_id_l = psa.psa_nwid[1];
	m.mmw_netw_id_h = psa.psa_nwid[0];

	if (psa.psa_nwid_select & 1)
		m.mmw_loopt_sel = 0x00;
	else
		m.mmw_loopt_sel = MMW_LOOPT_SEL_DIS_NWID;

	memcpy(&m.mmw_encr_key, &psa.psa_encryption_key,
	       sizeof(m.mmw_encr_key));

	if (psa.psa_encryption_select)
		m.mmw_encr_enable =
		    MMW_ENCR_ENABLE_EN | MMW_ENCR_ENABLE_MODE;
	else
		m.mmw_encr_enable = 0;

	m.mmw_thr_pre_set = psa.psa_thr_pre_set & 0x3F;
	m.mmw_quality_thr = psa.psa_quality_thr & 0x0F;

	
	m.mmw_jabber_enable = 0x01;
	m.mmw_freeze = 0;
	m.mmw_anten_sel = MMW_ANTEN_SEL_ALG_EN;
	m.mmw_ifs = 0x20;
	m.mmw_mod_delay = 0x04;
	m.mmw_jam_time = 0x38;

	m.mmw_des_io_invert = 0;
	m.mmw_decay_prm = 0;
	m.mmw_decay_updat_prm = 0;

	
	mmc_write(ioaddr, 0, (u8 *) & m, sizeof(m));

	

	
	
	if (!(mmc_in(ioaddr, mmroff(0, mmr_fee_status)) &
	      (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY))) {
		
		m.mmw_fee_addr = 0x0F;
		m.mmw_fee_ctrl = MMW_FEE_CTRL_READ | MMW_FEE_CTRL_DWLD;
		mmc_write(ioaddr, (char *) &m.mmw_fee_ctrl - (char *) &m,
			  (unsigned char *) &m.mmw_fee_ctrl, 2);

		
		fee_wait(ioaddr, 100, 100);

#ifdef DEBUG_CONFIG_INFO
		
		mmc_read(ioaddr, (char *) &m.mmw_fee_data_l - (char *) &m,
			 (unsigned char *) &m.mmw_fee_data_l, 2);

		
		printk(KERN_DEBUG
		       "%s: WaveLAN 2.00 recognised (frequency select).  Current frequency = %ld\n",
		       dev->name,
		       ((m.
			 mmw_fee_data_h << 4) | (m.mmw_fee_data_l >> 4)) *
		       5 / 2 + 24000L);
#endif

		
		m.mmw_fee_addr = 0x61;
		m.mmw_fee_ctrl = MMW_FEE_CTRL_READ | MMW_FEE_CTRL_DWLD;
		mmc_write(ioaddr, (char *) &m.mmw_fee_ctrl - (char *) &m,
			  (unsigned char *) &m.mmw_fee_ctrl, 2);

		
	}
	
#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: <-wv_mmc_init()\n", dev->name);
#endif
	return 0;
}



static int wv_ru_start(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	u16 scb_cs;
	fd_t fd;
	rbd_t rbd;
	u16 rx;
	u16 rx_next;
	int i;

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: ->wv_ru_start()\n", dev->name);
#endif

	obram_read(ioaddr, scboff(OFFSET_SCB, scb_status),
		   (unsigned char *) &scb_cs, sizeof(scb_cs));
	if ((scb_cs & SCB_ST_RUS) == SCB_ST_RUS_RDY)
		return 0;

	lp->rx_head = OFFSET_RU;

	for (i = 0, rx = lp->rx_head; i < NRXBLOCKS; i++, rx = rx_next) {
		rx_next =
		    (i == NRXBLOCKS - 1) ? lp->rx_head : rx + RXBLOCKZ;

		fd.fd_status = 0;
		fd.fd_command = (i == NRXBLOCKS - 1) ? FD_COMMAND_EL : 0;
		fd.fd_link_offset = rx_next;
		fd.fd_rbd_offset = rx + sizeof(fd);
		obram_write(ioaddr, rx, (unsigned char *) &fd, sizeof(fd));

		rbd.rbd_status = 0;
		rbd.rbd_next_rbd_offset = I82586NULL;
		rbd.rbd_bufl = rx + sizeof(fd) + sizeof(rbd);
		rbd.rbd_bufh = 0;
		rbd.rbd_el_size = RBD_EL | (RBD_SIZE & MAXDATAZ);
		obram_write(ioaddr, rx + sizeof(fd),
			    (unsigned char *) &rbd, sizeof(rbd));

		lp->rx_last = rx;
	}

	obram_write(ioaddr, scboff(OFFSET_SCB, scb_rfa_offset),
		    (unsigned char *) &lp->rx_head, sizeof(lp->rx_head));

	scb_cs = SCB_CMD_RUC_GO;
	obram_write(ioaddr, scboff(OFFSET_SCB, scb_command),
		    (unsigned char *) &scb_cs, sizeof(scb_cs));

	set_chan_attn(ioaddr, lp->hacr);

	for (i = 1000; i > 0; i--) {
		obram_read(ioaddr, scboff(OFFSET_SCB, scb_command),
			   (unsigned char *) &scb_cs, sizeof(scb_cs));
		if (scb_cs == 0)
			break;

		udelay(10);
	}

	if (i <= 0) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_INFO
		       "%s: wavelan_ru_start(): board not accepting command.\n",
		       dev->name);
#endif
		return -1;
	}
#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: <-wv_ru_start()\n", dev->name);
#endif
	return 0;
}



static int wv_cu_start(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	int i;
	u16 txblock;
	u16 first_nop;
	u16 scb_cs;

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: ->wv_cu_start()\n", dev->name);
#endif

	lp->tx_first_free = OFFSET_CU;
	lp->tx_first_in_use = I82586NULL;

	for (i = 0, txblock = OFFSET_CU;
	     i < NTXBLOCKS; i++, txblock += TXBLOCKZ) {
		ac_tx_t tx;
		ac_nop_t nop;
		tbd_t tbd;
		unsigned short tx_addr;
		unsigned short nop_addr;
		unsigned short tbd_addr;
		unsigned short buf_addr;

		tx_addr = txblock;
		nop_addr = tx_addr + sizeof(tx);
		tbd_addr = nop_addr + sizeof(nop);
		buf_addr = tbd_addr + sizeof(tbd);

		tx.tx_h.ac_status = 0;
		tx.tx_h.ac_command = acmd_transmit | AC_CFLD_I;
		tx.tx_h.ac_link = nop_addr;
		tx.tx_tbd_offset = tbd_addr;
		obram_write(ioaddr, tx_addr, (unsigned char *) &tx,
			    sizeof(tx));

		nop.nop_h.ac_status = 0;
		nop.nop_h.ac_command = acmd_nop;
		nop.nop_h.ac_link = nop_addr;
		obram_write(ioaddr, nop_addr, (unsigned char *) &nop,
			    sizeof(nop));

		tbd.tbd_status = TBD_STATUS_EOF;
		tbd.tbd_next_bd_offset = I82586NULL;
		tbd.tbd_bufl = buf_addr;
		tbd.tbd_bufh = 0;
		obram_write(ioaddr, tbd_addr, (unsigned char *) &tbd,
			    sizeof(tbd));
	}

	first_nop =
	    OFFSET_CU + (NTXBLOCKS - 1) * TXBLOCKZ + sizeof(ac_tx_t);
	obram_write(ioaddr, scboff(OFFSET_SCB, scb_cbl_offset),
		    (unsigned char *) &first_nop, sizeof(first_nop));

	scb_cs = SCB_CMD_CUC_GO;
	obram_write(ioaddr, scboff(OFFSET_SCB, scb_command),
		    (unsigned char *) &scb_cs, sizeof(scb_cs));

	set_chan_attn(ioaddr, lp->hacr);

	for (i = 1000; i > 0; i--) {
		obram_read(ioaddr, scboff(OFFSET_SCB, scb_command),
			   (unsigned char *) &scb_cs, sizeof(scb_cs));
		if (scb_cs == 0)
			break;

		udelay(10);
	}

	if (i <= 0) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_INFO
		       "%s: wavelan_cu_start(): board not accepting command.\n",
		       dev->name);
#endif
		return -1;
	}

	lp->tx_n_in_use = 0;
	netif_start_queue(dev);
#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: <-wv_cu_start()\n", dev->name);
#endif
	return 0;
}



static int wv_82586_start(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	scp_t scp;		
	iscp_t iscp;		
	scb_t scb;		
	ach_t cb;		
	u8 zeroes[512];
	int i;

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: ->wv_82586_start()\n", dev->name);
#endif

	
	memset(&zeroes[0], 0x00, sizeof(zeroes));
	for (i = 0; i < I82586_MEMZ; i += sizeof(zeroes))
		obram_write(ioaddr, i, &zeroes[0], sizeof(zeroes));

	
	memset(&scp, 0x00, sizeof(scp));
	scp.scp_sysbus = SCP_SY_16BBUS;
	scp.scp_iscpl = OFFSET_ISCP;
	obram_write(ioaddr, OFFSET_SCP, (unsigned char *) &scp,
		    sizeof(scp));

	memset(&iscp, 0x00, sizeof(iscp));
	iscp.iscp_busy = 1;
	iscp.iscp_offset = OFFSET_SCB;
	obram_write(ioaddr, OFFSET_ISCP, (unsigned char *) &iscp,
		    sizeof(iscp));

	
	memset(&scb, 0x00, sizeof(scb));
	scb.scb_command = SCB_CMD_RESET;
	scb.scb_cbl_offset = OFFSET_CU;
	scb.scb_rfa_offset = OFFSET_RU;
	obram_write(ioaddr, OFFSET_SCB, (unsigned char *) &scb,
		    sizeof(scb));

	set_chan_attn(ioaddr, lp->hacr);

	
	for (i = 1000; i > 0; i--) {
		obram_read(ioaddr, OFFSET_ISCP, (unsigned char *) &iscp,
			   sizeof(iscp));

		if (iscp.iscp_busy == (unsigned short) 0)
			break;

		udelay(10);
	}

	if (i <= 0) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_INFO
		       "%s: wv_82586_start(): iscp_busy timeout.\n",
		       dev->name);
#endif
		return -1;
	}

	
	for (i = 15; i > 0; i--) {
		obram_read(ioaddr, OFFSET_SCB, (unsigned char *) &scb,
			   sizeof(scb));

		if (scb.scb_status == (SCB_ST_CX | SCB_ST_CNA))
			break;

		udelay(10);
	}

	if (i <= 0) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_INFO
		       "%s: wv_82586_start(): status: expected 0x%02x, got 0x%02x.\n",
		       dev->name, SCB_ST_CX | SCB_ST_CNA, scb.scb_status);
#endif
		return -1;
	}

	wv_ack(dev);

	
	memset(&cb, 0x00, sizeof(cb));
	cb.ac_command = AC_CFLD_EL | (AC_CFLD_CMD & acmd_diagnose);
	cb.ac_link = OFFSET_CU;
	obram_write(ioaddr, OFFSET_CU, (unsigned char *) &cb, sizeof(cb));

	if (wv_synchronous_cmd(dev, "diag()") == -1)
		return -1;

	obram_read(ioaddr, OFFSET_CU, (unsigned char *) &cb, sizeof(cb));
	if (cb.ac_status & AC_SFLD_FAIL) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_INFO
		       "%s: wv_82586_start(): i82586 Self Test failed.\n",
		       dev->name);
#endif
		return -1;
	}
#ifdef DEBUG_I82586_SHOW
	wv_scb_show(ioaddr);
#endif

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: <-wv_82586_start()\n", dev->name);
#endif
	return 0;
}



static void wv_82586_config(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	unsigned short txblock;
	unsigned short txpred;
	unsigned short tx_addr;
	unsigned short nop_addr;
	unsigned short tbd_addr;
	unsigned short cfg_addr;
	unsigned short ias_addr;
	unsigned short mcs_addr;
	ac_tx_t tx;
	ac_nop_t nop;
	ac_cfg_t cfg;		
	ac_ias_t ias;		
	ac_mcs_t mcs;		
	struct dev_mc_list *dmi;

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: ->wv_82586_config()\n", dev->name);
#endif

	
	if (lp->tx_n_in_use == (NTXBLOCKS - 1)) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_INFO "%s: wv_82586_config(): Tx queue full.\n",
		       dev->name);
#endif
		return;
	}

	
	txblock = lp->tx_first_free;
	txpred = txblock - TXBLOCKZ;
	if (txpred < OFFSET_CU)
		txpred += NTXBLOCKS * TXBLOCKZ;
	lp->tx_first_free += TXBLOCKZ;
	if (lp->tx_first_free >= OFFSET_CU + NTXBLOCKS * TXBLOCKZ)
		lp->tx_first_free -= NTXBLOCKS * TXBLOCKZ;

	lp->tx_n_in_use++;

	
	tx_addr = txblock;
	nop_addr = tx_addr + sizeof(tx);
	tbd_addr = nop_addr + sizeof(nop);
	cfg_addr = tbd_addr + sizeof(tbd_t);	
	ias_addr = cfg_addr + sizeof(cfg);
	mcs_addr = ias_addr + sizeof(ias);

	
	tx.tx_h.ac_status = 0xFFFF;	
	obram_write(ioaddr, toff(ac_tx_t, tx_addr, tx_h.ac_status),
		    (unsigned char *) &tx.tx_h.ac_status,
		    sizeof(tx.tx_h.ac_status));

	
	nop.nop_h.ac_status = 0;
	obram_write(ioaddr, toff(ac_nop_t, nop_addr, nop_h.ac_status),
		    (unsigned char *) &nop.nop_h.ac_status,
		    sizeof(nop.nop_h.ac_status));
	nop.nop_h.ac_link = nop_addr;
	obram_write(ioaddr, toff(ac_nop_t, nop_addr, nop_h.ac_link),
		    (unsigned char *) &nop.nop_h.ac_link,
		    sizeof(nop.nop_h.ac_link));

	
	memset(&cfg, 0x00, sizeof(cfg));

	
	cfg.cfg_byte_cnt =
	    AC_CFG_BYTE_CNT(sizeof(ac_cfg_t) - sizeof(ach_t));
	cfg.cfg_fifolim = AC_CFG_FIFOLIM(4);
	cfg.cfg_byte8 = AC_CFG_SAV_BF(1) | AC_CFG_SRDY(0);
	cfg.cfg_byte9 = AC_CFG_ELPBCK(0) |
	    AC_CFG_ILPBCK(0) |
	    AC_CFG_PRELEN(AC_CFG_PLEN_2) |
	    AC_CFG_ALOC(1) | AC_CFG_ADDRLEN(WAVELAN_ADDR_SIZE);
	cfg.cfg_byte10 = AC_CFG_BOFMET(1) |
	    AC_CFG_ACR(6) | AC_CFG_LINPRIO(0);
	cfg.cfg_ifs = 0x20;
	cfg.cfg_slotl = 0x0C;
	cfg.cfg_byte13 = AC_CFG_RETRYNUM(15) | AC_CFG_SLTTMHI(0);
	cfg.cfg_byte14 = AC_CFG_FLGPAD(0) |
	    AC_CFG_BTSTF(0) |
	    AC_CFG_CRC16(0) |
	    AC_CFG_NCRC(0) |
	    AC_CFG_TNCRS(1) |
	    AC_CFG_MANCH(0) |
	    AC_CFG_BCDIS(0) | AC_CFG_PRM(lp->promiscuous);
	cfg.cfg_byte15 = AC_CFG_ICDS(0) |
	    AC_CFG_CDTF(0) | AC_CFG_ICSS(0) | AC_CFG_CSTF(0);

	cfg.cfg_min_frm_len = AC_CFG_MNFRM(8);

	cfg.cfg_h.ac_command = (AC_CFLD_CMD & acmd_configure);
	cfg.cfg_h.ac_link = ias_addr;
	obram_write(ioaddr, cfg_addr, (unsigned char *) &cfg, sizeof(cfg));

	
	memset(&ias, 0x00, sizeof(ias));
	ias.ias_h.ac_command = (AC_CFLD_CMD & acmd_ia_setup);
	ias.ias_h.ac_link = mcs_addr;
	memcpy(&ias.ias_addr[0], (unsigned char *) &dev->dev_addr[0],
	       sizeof(ias.ias_addr));
	obram_write(ioaddr, ias_addr, (unsigned char *) &ias, sizeof(ias));

	
	memset(&mcs, 0x00, sizeof(mcs));
	mcs.mcs_h.ac_command = AC_CFLD_I | (AC_CFLD_CMD & acmd_mc_setup);
	mcs.mcs_h.ac_link = nop_addr;
	mcs.mcs_cnt = WAVELAN_ADDR_SIZE * lp->mc_count;
	obram_write(ioaddr, mcs_addr, (unsigned char *) &mcs, sizeof(mcs));

	
	if (lp->mc_count) {
		for (dmi = dev->mc_list; dmi; dmi = dmi->next)
			outsw(PIOP1(ioaddr), (u16 *) dmi->dmi_addr,
			      WAVELAN_ADDR_SIZE >> 1);

#ifdef DEBUG_CONFIG_INFO
		printk(KERN_DEBUG
		       "%s: wv_82586_config(): set %d multicast addresses:\n",
		       dev->name, lp->mc_count);
		for (dmi = dev->mc_list; dmi; dmi = dmi->next)
			printk(KERN_DEBUG " %pM\n", dmi->dmi_addr);
#endif
	}

	
	nop_addr = txpred + sizeof(tx);
	nop.nop_h.ac_status = 0;
	obram_write(ioaddr, toff(ac_nop_t, nop_addr, nop_h.ac_status),
		    (unsigned char *) &nop.nop_h.ac_status,
		    sizeof(nop.nop_h.ac_status));
	nop.nop_h.ac_link = cfg_addr;
	obram_write(ioaddr, toff(ac_nop_t, nop_addr, nop_h.ac_link),
		    (unsigned char *) &nop.nop_h.ac_link,
		    sizeof(nop.nop_h.ac_link));

	
	lp->reconfig_82586 = 0;

	if (lp->tx_first_in_use == I82586NULL)
		lp->tx_first_in_use = txblock;

	if (lp->tx_n_in_use == (NTXBLOCKS - 1))
		netif_stop_queue(dev);

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: <-wv_82586_config()\n", dev->name);
#endif
}



static void wv_82586_stop(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	u16 scb_cmd;

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: ->wv_82586_stop()\n", dev->name);
#endif

	
	scb_cmd =
	    (SCB_CMD_CUC & SCB_CMD_CUC_SUS) | (SCB_CMD_RUC &
					       SCB_CMD_RUC_SUS);
	obram_write(ioaddr, scboff(OFFSET_SCB, scb_command),
		    (unsigned char *) &scb_cmd, sizeof(scb_cmd));
	set_chan_attn(ioaddr, lp->hacr);

	
	wv_ints_off(dev);

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: <-wv_82586_stop()\n", dev->name);
#endif
}



static int wv_hw_reset(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: ->wv_hw_reset(dev=0x%x)\n", dev->name,
	       (unsigned int) dev);
#endif

	
	lp->nresets++;

	wv_hacr_reset(ioaddr);
	lp->hacr = HACR_DEFAULT;

	if ((wv_mmc_init(dev) < 0) || (wv_82586_start(dev) < 0))
		return -1;

	
	wv_ints_on(dev);

	
	if (wv_cu_start(dev) < 0)
		return -1;

	
	wv_82586_config(dev);

	
	if (wv_ru_start(dev) < 0)
		return -1;

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: <-wv_hw_reset()\n", dev->name);
#endif
	return 0;
}



static int wv_check_ioaddr(unsigned long ioaddr, u8 * mac)
{
	int i;			

	
	if (!request_region(ioaddr, sizeof(ha_t), "wavelan probe"))
		return -EBUSY;		

	
	wv_hacr_reset(ioaddr);

	
	psa_read(ioaddr, HACR_DEFAULT, psaoff(0, psa_univ_mac_addr),
		 mac, 6);

	release_region(ioaddr, sizeof(ha_t));

	
	for (i = 0; i < ARRAY_SIZE(MAC_ADDRESSES); i++)
		if ((mac[0] == MAC_ADDRESSES[i][0]) &&
		    (mac[1] == MAC_ADDRESSES[i][1]) &&
		    (mac[2] == MAC_ADDRESSES[i][2]))
			return 0;

#ifdef DEBUG_CONFIG_INFO
	printk(KERN_WARNING
	       "WaveLAN (0x%3X): your MAC address might be %02X:%02X:%02X.\n",
	       ioaddr, mac[0], mac[1], mac[2]);
#endif
	return -ENODEV;
}




static irqreturn_t wavelan_interrupt(int irq, void *dev_id)
{
	struct net_device *dev;
	unsigned long ioaddr;
	net_local *lp;
	u16 hasr;
	u16 status;
	u16 ack_cmd;

	dev = dev_id;

#ifdef DEBUG_INTERRUPT_TRACE
	printk(KERN_DEBUG "%s: ->wavelan_interrupt()\n", dev->name);
#endif

	lp = netdev_priv(dev);
	ioaddr = dev->base_addr;

#ifdef DEBUG_INTERRUPT_INFO
	
	if(spin_is_locked(&lp->spinlock))
		printk(KERN_DEBUG
		       "%s: wavelan_interrupt(): spinlock is already locked !!!\n",
		       dev->name);
#endif

	
	spin_lock(&lp->spinlock);

	

	
	hasr = hasr_read(ioaddr);

#ifdef DEBUG_INTERRUPT_INFO
	printk(KERN_INFO
	       "%s: wavelan_interrupt(): hasr 0x%04x; hacr 0x%04x.\n",
	       dev->name, hasr, lp->hacr);
#endif

	
	if ((hasr & HASR_MMC_INTR) && (lp->hacr & HACR_MMC_INT_ENABLE)) {
		u8 dce_status;

		
		mmc_read(ioaddr, mmroff(0, mmr_dce_status), &dce_status,
			 sizeof(dce_status));

#ifdef DEBUG_INTERRUPT_ERROR
		printk(KERN_INFO
		       "%s: wavelan_interrupt(): unexpected mmc interrupt: status 0x%04x.\n",
		       dev->name, dce_status);
#endif
	}

	
	if (((hasr & HASR_82586_INTR) == 0) ||
	    ((lp->hacr & HACR_82586_INT_ENABLE) == 0)) {
#ifdef DEBUG_INTERRUPT_ERROR
		printk(KERN_INFO
		       "%s: wavelan_interrupt(): interrupt not coming from i82586 - hasr 0x%04x.\n",
		       dev->name, hasr);
#endif
		spin_unlock (&lp->spinlock);
		return IRQ_NONE;
	}

	
	obram_read(ioaddr, scboff(OFFSET_SCB, scb_status),
		   (unsigned char *) &status, sizeof(status));

	
	ack_cmd = status & SCB_ST_INT;
	obram_write(ioaddr, scboff(OFFSET_SCB, scb_command),
		    (unsigned char *) &ack_cmd, sizeof(ack_cmd));
	set_chan_attn(ioaddr, lp->hacr);

#ifdef DEBUG_INTERRUPT_INFO
	printk(KERN_DEBUG "%s: wavelan_interrupt(): status 0x%04x.\n",
	       dev->name, status);
#endif

	
	if ((status & SCB_ST_CX) == SCB_ST_CX) {
#ifdef DEBUG_INTERRUPT_INFO
		printk(KERN_DEBUG
		       "%s: wavelan_interrupt(): command completed.\n",
		       dev->name);
#endif
		wv_complete(dev, ioaddr, lp);
	}

	
	if ((status & SCB_ST_FR) == SCB_ST_FR) {
#ifdef DEBUG_INTERRUPT_INFO
		printk(KERN_DEBUG
		       "%s: wavelan_interrupt(): received packet.\n",
		       dev->name);
#endif
		wv_receive(dev);
	}

	
	if (((status & SCB_ST_CNA) == SCB_ST_CNA) ||
	    (((status & SCB_ST_CUS) != SCB_ST_CUS_ACTV) &&
	     (netif_running(dev)))) {
#ifdef DEBUG_INTERRUPT_ERROR
		printk(KERN_INFO
		       "%s: wavelan_interrupt(): CU inactive -- restarting\n",
		       dev->name);
#endif
		wv_hw_reset(dev);
	}

	
	if (((status & SCB_ST_RNR) == SCB_ST_RNR) ||
	    (((status & SCB_ST_RUS) != SCB_ST_RUS_RDY) &&
	     (netif_running(dev)))) {
#ifdef DEBUG_INTERRUPT_ERROR
		printk(KERN_INFO
		       "%s: wavelan_interrupt(): RU not ready -- restarting\n",
		       dev->name);
#endif
		wv_hw_reset(dev);
	}

	
	spin_unlock (&lp->spinlock);

#ifdef DEBUG_INTERRUPT_TRACE
	printk(KERN_DEBUG "%s: <-wavelan_interrupt()\n", dev->name);
#endif
	return IRQ_HANDLED;
}



static void wavelan_watchdog(struct net_device *	dev)
{
	net_local *lp = netdev_priv(dev);
	u_long		ioaddr = dev->base_addr;
	unsigned long	flags;
	unsigned int	nreaped;

#ifdef DEBUG_INTERRUPT_TRACE
	printk(KERN_DEBUG "%s: ->wavelan_watchdog()\n", dev->name);
#endif

#ifdef DEBUG_INTERRUPT_ERROR
	printk(KERN_INFO "%s: wavelan_watchdog: watchdog timer expired\n",
	       dev->name);
#endif

	
	if (lp->tx_n_in_use <= 0) {
		return;
	}

	spin_lock_irqsave(&lp->spinlock, flags);

	
	nreaped = wv_complete(dev, ioaddr, lp);

#ifdef DEBUG_INTERRUPT_INFO
	printk(KERN_DEBUG
	       "%s: wavelan_watchdog(): %d reaped, %d remain.\n",
	       dev->name, nreaped, lp->tx_n_in_use);
#endif

#ifdef DEBUG_PSA_SHOW
	{
		psa_t psa;
		psa_read(dev, 0, (unsigned char *) &psa, sizeof(psa));
		wv_psa_show(&psa);
	}
#endif
#ifdef DEBUG_MMC_SHOW
	wv_mmc_show(dev);
#endif
#ifdef DEBUG_I82586_SHOW
	wv_cu_show(dev);
#endif

	
	if (nreaped == 0) {
#ifdef DEBUG_INTERRUPT_ERROR
		printk(KERN_INFO
		       "%s: wavelan_watchdog(): cleanup failed, trying reset\n",
		       dev->name);
#endif
		wv_hw_reset(dev);
	}

	
	if (lp->tx_n_in_use < NTXBLOCKS - 1)
		netif_wake_queue(dev);

	spin_unlock_irqrestore(&lp->spinlock, flags);
	
#ifdef DEBUG_INTERRUPT_TRACE
	printk(KERN_DEBUG "%s: <-wavelan_watchdog()\n", dev->name);
#endif
}






static int wavelan_open(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long	flags;

#ifdef DEBUG_CALLBACK_TRACE
	printk(KERN_DEBUG "%s: ->wavelan_open(dev=0x%x)\n", dev->name,
	       (unsigned int) dev);
#endif

	
	if (dev->irq == 0) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_WARNING "%s: wavelan_open(): no IRQ\n",
		       dev->name);
#endif
		return -ENXIO;
	}

	if (request_irq(dev->irq, &wavelan_interrupt, 0, "WaveLAN", dev) != 0) 
	{
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_WARNING "%s: wavelan_open(): invalid IRQ\n",
		       dev->name);
#endif
		return -EAGAIN;
	}

	spin_lock_irqsave(&lp->spinlock, flags);
	
	if (wv_hw_reset(dev) != -1) {
		netif_start_queue(dev);
	} else {
		free_irq(dev->irq, dev);
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_INFO
		       "%s: wavelan_open(): impossible to start the card\n",
		       dev->name);
#endif
		spin_unlock_irqrestore(&lp->spinlock, flags);
		return -EAGAIN;
	}
	spin_unlock_irqrestore(&lp->spinlock, flags);
	
#ifdef DEBUG_CALLBACK_TRACE
	printk(KERN_DEBUG "%s: <-wavelan_open()\n", dev->name);
#endif
	return 0;
}



static int wavelan_close(struct net_device * dev)
{
	net_local *lp = netdev_priv(dev);
	unsigned long flags;

#ifdef DEBUG_CALLBACK_TRACE
	printk(KERN_DEBUG "%s: ->wavelan_close(dev=0x%x)\n", dev->name,
	       (unsigned int) dev);
#endif

	netif_stop_queue(dev);

	
	spin_lock_irqsave(&lp->spinlock, flags);
	wv_82586_stop(dev);
	spin_unlock_irqrestore(&lp->spinlock, flags);

	free_irq(dev->irq, dev);

#ifdef DEBUG_CALLBACK_TRACE
	printk(KERN_DEBUG "%s: <-wavelan_close()\n", dev->name);
#endif
	return 0;
}

static const struct net_device_ops wavelan_netdev_ops = {
	.ndo_open 		= wavelan_open,
	.ndo_stop 		= wavelan_close,
	.ndo_start_xmit		= wavelan_packet_xmit,
	.ndo_set_multicast_list = wavelan_set_multicast_list,
        .ndo_tx_timeout		= wavelan_watchdog,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
#ifdef SET_MAC_ADDRESS
	.ndo_set_mac_address	= wavelan_set_mac_address
#else
	.ndo_set_mac_address 	= eth_mac_addr,
#endif
};




static int __init wavelan_config(struct net_device *dev, unsigned short ioaddr)
{
	u8 irq_mask;
	int irq;
	net_local *lp;
	mac_addr mac;
	int err;

	if (!request_region(ioaddr, sizeof(ha_t), "wavelan"))
		return -EADDRINUSE;

	err = wv_check_ioaddr(ioaddr, mac);
	if (err)
		goto out;

	memcpy(dev->dev_addr, mac, 6);

	dev->base_addr = ioaddr;

#ifdef DEBUG_CALLBACK_TRACE
	printk(KERN_DEBUG "%s: ->wavelan_config(dev=0x%x, ioaddr=0x%lx)\n",
	       dev->name, (unsigned int) dev, ioaddr);
#endif

	
	if (dev->irq != 0) {
		irq_mask = wv_irq_to_psa(dev->irq);

		if (irq_mask == 0) {
#ifdef DEBUG_CONFIG_ERROR
			printk(KERN_WARNING
			       "%s: wavelan_config(): invalid IRQ %d ignored.\n",
			       dev->name, dev->irq);
#endif
			dev->irq = 0;
		} else {
#ifdef DEBUG_CONFIG_INFO
			printk(KERN_DEBUG
			       "%s: wavelan_config(): changing IRQ to %d\n",
			       dev->name, dev->irq);
#endif
			psa_write(ioaddr, HACR_DEFAULT,
				  psaoff(0, psa_int_req_no), &irq_mask, 1);
			
			update_psa_checksum(dev, ioaddr, HACR_DEFAULT);
			wv_hacr_reset(ioaddr);
		}
	}

	psa_read(ioaddr, HACR_DEFAULT, psaoff(0, psa_int_req_no),
		 &irq_mask, 1);
	if ((irq = wv_psa_to_irq(irq_mask)) == -1) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_INFO
		       "%s: wavelan_config(): could not wavelan_map_irq(%d).\n",
		       dev->name, irq_mask);
#endif
		err = -EAGAIN;
		goto out;
	}

	dev->irq = irq;

	dev->mem_start = 0x0000;
	dev->mem_end = 0x0000;
	dev->if_port = 0;

	
	memset(netdev_priv(dev), 0, sizeof(net_local));
	lp = netdev_priv(dev);

	
	lp->dev = dev;
	
	lp->next = wavelan_list;
	wavelan_list = lp;

	lp->hacr = HACR_DEFAULT;

	
	lp->promiscuous = 0;
	lp->mc_count = 0;

	
	spin_lock_init(&lp->spinlock);

	dev->netdev_ops = &wavelan_netdev_ops;
	dev->watchdog_timeo = WATCHDOG_JIFFIES;
	dev->wireless_handlers = &wavelan_handler_def;
	lp->wireless_data.spy_data = &lp->spy_data;
	dev->wireless_data = &lp->wireless_data;

	dev->mtu = WAVELAN_MTU;

	
	wv_init_info(dev);

#ifdef DEBUG_CALLBACK_TRACE
	printk(KERN_DEBUG "%s: <-wavelan_config()\n", dev->name);
#endif
	return 0;
out:
	release_region(ioaddr, sizeof(ha_t));
	return err;
}



struct net_device * __init wavelan_probe(int unit)
{
	struct net_device *dev;
	short base_addr;
	int def_irq;
	int i;
	int r = 0;

	
	BUILD_BUG_ON(sizeof(psa_t) != PSA_SIZE);
	BUILD_BUG_ON(sizeof(mmw_t) != MMW_SIZE);
	BUILD_BUG_ON(sizeof(mmr_t) != MMR_SIZE);
	BUILD_BUG_ON(sizeof(ha_t) != HA_SIZE);

	dev = alloc_etherdev(sizeof(net_local));
	if (!dev)
		return ERR_PTR(-ENOMEM);

	sprintf(dev->name, "eth%d", unit);
	netdev_boot_setup_check(dev);
	base_addr = dev->base_addr;
	def_irq = dev->irq;

#ifdef DEBUG_CALLBACK_TRACE
	printk(KERN_DEBUG
	       "%s: ->wavelan_probe(dev=%p (base_addr=0x%x))\n",
	       dev->name, dev, (unsigned int) dev->base_addr);
#endif

	
	if (base_addr < 0) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_WARNING
		       "%s: wavelan_probe(): invalid base address\n",
		       dev->name);
#endif
		r = -ENXIO;
	} else if (base_addr > 0x100) { 
		r = wavelan_config(dev, base_addr);
#ifdef DEBUG_CONFIG_INFO
		if (r != 0)
			printk(KERN_DEBUG
			       "%s: wavelan_probe(): no device at specified base address (0x%X) or address already in use\n",
			       dev->name, base_addr);
#endif

#ifdef DEBUG_CALLBACK_TRACE
		printk(KERN_DEBUG "%s: <-wavelan_probe()\n", dev->name);
#endif
	} else { 
		for (i = 0; i < ARRAY_SIZE(iobase); i++) {
			dev->irq = def_irq;
			if (wavelan_config(dev, iobase[i]) == 0) {
#ifdef DEBUG_CALLBACK_TRACE
				printk(KERN_DEBUG
				       "%s: <-wavelan_probe()\n",
				       dev->name);
#endif
				break;
			}
		}
		if (i == ARRAY_SIZE(iobase))
			r = -ENODEV;
	}
	if (r) 
		goto out;
	r = register_netdev(dev);
	if (r)
		goto out1;
	return dev;
out1:
	release_region(dev->base_addr, sizeof(ha_t));
	wavelan_list = wavelan_list->next;
out:
	free_netdev(dev);
	return ERR_PTR(r);
}




#ifdef	MODULE


int __init init_module(void)
{
	int ret = -EIO;		
	int i;

#ifdef DEBUG_MODULE_TRACE
	printk(KERN_DEBUG "-> init_module()\n");
#endif

	
	if (io[0] == 0) {
#ifdef DEBUG_CONFIG_ERROR
		printk(KERN_WARNING
		       "WaveLAN init_module(): doing device probing (bad !)\n");
		printk(KERN_WARNING
		       "Specify base addresses while loading module to correct the problem\n");
#endif

		
		for (i = 0; i < ARRAY_SIZE(iobase); i++)
			io[i] = iobase[i];
	}


	
	for (i = 0; i < ARRAY_SIZE(io) && io[i] != 0; i++) {
		struct net_device *dev = alloc_etherdev(sizeof(net_local));
		if (!dev)
			break;
		if (name[i])
			strcpy(dev->name, name[i]);	
		dev->base_addr = io[i];
		dev->irq = irq[i];

		
		if (wavelan_config(dev, io[i]) == 0) {
			if (register_netdev(dev) != 0) {
				release_region(dev->base_addr, sizeof(ha_t));
				wavelan_list = wavelan_list->next;
			} else {
				ret = 0;
				continue;
			}
		}
		free_netdev(dev);
	}

#ifdef DEBUG_CONFIG_ERROR
	if (!wavelan_list)
		printk(KERN_WARNING
		       "WaveLAN init_module(): no device found\n");
#endif

#ifdef DEBUG_MODULE_TRACE
	printk(KERN_DEBUG "<- init_module()\n");
#endif
	return ret;
}



void cleanup_module(void)
{
#ifdef DEBUG_MODULE_TRACE
	printk(KERN_DEBUG "-> cleanup_module()\n");
#endif

	
	while (wavelan_list) {
		struct net_device *dev = wavelan_list->dev;

#ifdef DEBUG_CONFIG_INFO
		printk(KERN_DEBUG
		       "%s: cleanup_module(): removing device at 0x%x\n",
		       dev->name, (unsigned int) dev);
#endif
		unregister_netdev(dev);

		release_region(dev->base_addr, sizeof(ha_t));
		wavelan_list = wavelan_list->next;

		free_netdev(dev);
	}

#ifdef DEBUG_MODULE_TRACE
	printk(KERN_DEBUG "<- cleanup_module()\n");
#endif
}
#endif				
MODULE_LICENSE("GPL");


