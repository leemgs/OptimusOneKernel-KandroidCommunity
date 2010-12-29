


#include "wavelan_cs.p.h"		

#ifdef WAVELAN_ROAMING
static void wl_cell_expiry(unsigned long data);
static void wl_del_wavepoint(wavepoint_history *wavepoint, struct net_local *lp);
static void wv_nwid_filter(unsigned char mode, net_local *lp);
#endif  









static inline u_char
hasr_read(u_long	base)
{
  return(inb(HASR(base)));
} 



static inline void
hacr_write(u_long	base,
	   u_char	hacr)
{
  outb(hacr, HACR(base));
} 



static void
hacr_write_slow(u_long	base,
		u_char	hacr)
{
  hacr_write(base, hacr);
  
  mdelay(1);
} 



static void
psa_read(struct net_device *	dev,
	 int		o,	
	 u_char *	b,	
	 int		n)	
{
  net_local *lp = netdev_priv(dev);
  u_char __iomem *ptr = lp->mem + PSA_ADDR + (o << 1);

  while(n-- > 0)
    {
      *b++ = readb(ptr);
      
      ptr += 2;
    }
} 



static void
psa_write(struct net_device *	dev,
	  int		o,	
	  u_char *	b,	
	  int		n)	
{
  net_local *lp = netdev_priv(dev);
  u_char __iomem *ptr = lp->mem + PSA_ADDR + (o << 1);
  int		count = 0;
  unsigned int	base = dev->base_addr;
  
  volatile u_char __iomem *verify = lp->mem + PSA_ADDR +
    (psaoff(0, psa_comp_number) << 1);

  
  hacr_write(base, HACR_PWR_STAT | HACR_ROM_WEN);

  while(n-- > 0)
    {
      
      writeb(*b++, ptr);
      ptr += 2;

      
      count = 0;
      while((readb(verify) != PSA_COMP_PCMCIA_915) && (count++ < 100))
	mdelay(1);
    }

  
  hacr_write(base, HACR_DEFAULT);
} 

#ifdef SET_PSA_CRC


static u_short
psa_crc(unsigned char *	psa,	
	int		size)	
{
  int		byte_cnt;	
  u_short	crc_bytes = 0;	
  int		bit_cnt;	

  for(byte_cnt = 0; byte_cnt < size; byte_cnt++ )
    {
      crc_bytes ^= psa[byte_cnt];	

      for(bit_cnt = 1; bit_cnt < 9; bit_cnt++ )
	{
	  if(crc_bytes & 0x0001)
	    crc_bytes = (crc_bytes >> 1) ^ 0xA001;
	  else
	    crc_bytes >>= 1 ;
        }
    }

  return crc_bytes;
} 
#endif	



static void
update_psa_checksum(struct net_device *	dev)
{
#ifdef SET_PSA_CRC
  psa_t		psa;
  u_short	crc;

  
  psa_read(dev, 0, (unsigned char *) &psa, sizeof(psa));

  
  crc = psa_crc((unsigned char *) &psa,
		sizeof(psa) - sizeof(psa.psa_crc[0]) - sizeof(psa.psa_crc[1])
		- sizeof(psa.psa_crc_status));

  psa.psa_crc[0] = crc & 0xFF;
  psa.psa_crc[1] = (crc & 0xFF00) >> 8;

  
  psa_write(dev, (char *)&psa.psa_crc - (char *)&psa,
	    (unsigned char *)&psa.psa_crc, 2);

#ifdef DEBUG_IOCTL_INFO
  printk (KERN_DEBUG "%s: update_psa_checksum(): crc = 0x%02x%02x\n",
          dev->name, psa.psa_crc[0], psa.psa_crc[1]);

  
  crc = psa_crc((unsigned char *) &psa,
		 sizeof(psa) - sizeof(psa.psa_crc_status));

  if(crc != 0)
    printk(KERN_WARNING "%s: update_psa_checksum(): CRC does not agree with PSA data (even after recalculating)\n", dev->name);
#endif 
#endif	
} 



static void
mmc_out(u_long		base,
	u_short		o,
	u_char		d)
{
  int count = 0;

  
  while((count++ < 100) && (inb(HASR(base)) & HASR_MMI_BUSY))
    udelay(10);

  outb((u_char)((o << 1) | MMR_MMI_WR), MMR(base));
  outb(d, MMD(base));
}



static void
mmc_write(u_long	base,
	  u_char	o,
	  u_char *	b,
	  int		n)
{
  o += n;
  b += n;

  while(n-- > 0 )
    mmc_out(base, --o, *(--b));
} 



static u_char
mmc_in(u_long	base,
       u_short	o)
{
  int count = 0;

  while((count++ < 100) && (inb(HASR(base)) & HASR_MMI_BUSY))
    udelay(10);
  outb(o << 1, MMR(base));		

  outb(0, MMD(base));			

  while((count++ < 100) && (inb(HASR(base)) & HASR_MMI_BUSY))
    udelay(10);
  return (u_char) (inb(MMD(base)));	
}



static void
mmc_read(u_long		base,
	 u_char		o,
	 u_char *	b,
	 int		n)
{
  o += n;
  b += n;

  while(n-- > 0)
    *(--b) = mmc_in(base, --o);
} 



static inline int
mmc_encr(u_long		base)	
{
  int	temp;

  temp = mmc_in(base, mmroff(0, mmr_des_avail));
  if((temp != MMR_DES_AVAIL_DES) && (temp != MMR_DES_AVAIL_AES))
    return 0;
  else
    return temp;
}



static void
fee_wait(u_long		base,	
	 int		delay,	
	 int		number)	
{
  int		count = 0;	

  while((count++ < number) &&
	(mmc_in(base, mmroff(0, mmr_fee_status)) & MMR_FEE_STATUS_BUSY))
    udelay(delay);
}



static void
fee_read(u_long		base,	
	 u_short	o,	
	 u_short *	b,	
	 int		n)	
{
  b += n;		

  
  mmc_out(base, mmwoff(0, mmw_fee_addr), o + n - 1);

  
  while(n-- > 0)
    {
      
      mmc_out(base, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_READ);

      
      fee_wait(base, 10, 100);

      
      *--b = ((mmc_in(base, mmroff(0, mmr_fee_data_h)) << 8) |
	      mmc_in(base, mmroff(0, mmr_fee_data_l)));
    }
}




static void
fee_write(u_long	base,	
	  u_short	o,	
	  u_short *	b,	
	  int		n)	
{
  b += n;		

#ifdef EEPROM_IS_PROTECTED	
#ifdef DOESNT_SEEM_TO_WORK	
  
  mmc_out(base, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PRREAD);

  fee_wait(base, 10, 100);

  
  printk("Protected 2 : %02X-%02X\n",
	 mmc_in(base, mmroff(0, mmr_fee_data_h)),
	 mmc_in(base, mmroff(0, mmr_fee_data_l)));
#endif	

  
  mmc_out(base, mmwoff(0, mmw_fee_addr), MMW_FEE_ADDR_EN);
  mmc_out(base, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PREN);

  fee_wait(base, 10, 100);

  
  mmc_out(base, mmwoff(0, mmw_fee_addr), o + n);
  mmc_out(base, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PRWRITE);
#ifdef DOESNT_SEEM_TO_WORK	
  
  mmc_out(base, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PRCLEAR);
#endif	

  fee_wait(base, 10, 100);
#endif	

  
  mmc_out(base, mmwoff(0, mmw_fee_addr), MMW_FEE_ADDR_EN);
  mmc_out(base, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_WREN);

  fee_wait(base, 10, 100);

  
  mmc_out(base, mmwoff(0, mmw_fee_addr), o + n - 1);

  
  while(n-- > 0)
    {
      
      mmc_out(base, mmwoff(0, mmw_fee_data_h), (*--b) >> 8);
      mmc_out(base, mmwoff(0, mmw_fee_data_l), *b & 0xFF);

      
      mmc_out(base, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_WRITE);

      
      mdelay(10);
      fee_wait(base, 10, 100);
    }

  
  mmc_out(base, mmwoff(0, mmw_fee_addr), MMW_FEE_ADDR_DS);
  mmc_out(base, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_WDS);

  fee_wait(base, 10, 100);

#ifdef EEPROM_IS_PROTECTED	
  
  mmc_out(base, mmwoff(0, mmw_fee_addr), 0x00);
  mmc_out(base, mmwoff(0, mmw_fee_ctrl), MMW_FEE_CTRL_PRWRITE);

  fee_wait(base, 10, 100);
#endif	
}



#ifdef WAVELAN_ROAMING	

static unsigned char WAVELAN_BEACON_ADDRESS[] = {0x09,0x00,0x0e,0x20,0x03,0x00};
  
static void wv_roam_init(struct net_device *dev)
{
  net_local  *lp= netdev_priv(dev);

  
  printk(KERN_NOTICE "%s: Warning, you have enabled roaming on"
	 " device %s !\n", dev->name, dev->name);
  printk(KERN_NOTICE "Roaming is currently an experimental unsupported feature"
	 " of the Wavelan driver.\n");
  printk(KERN_NOTICE "It may work, but may also make the driver behave in"
	 " erratic ways or crash.\n");

  lp->wavepoint_table.head=NULL;           
  lp->wavepoint_table.num_wavepoints=0;
  lp->wavepoint_table.locked=0;
  lp->curr_point=NULL;                        
  lp->cell_search=0;
  
  lp->cell_timer.data=(long)lp;               
  lp->cell_timer.function=wl_cell_expiry;
  lp->cell_timer.expires=jiffies+CELL_TIMEOUT;
  add_timer(&lp->cell_timer);
  
  wv_nwid_filter(NWID_PROMISC,lp) ;    
  
                                           
  printk(KERN_DEBUG "WaveLAN: Roaming enabled on device %s\n",dev->name);
}
 
static void wv_roam_cleanup(struct net_device *dev)
{
  wavepoint_history *ptr,*old_ptr;
  net_local *lp= netdev_priv(dev);
  
  printk(KERN_DEBUG "WaveLAN: Roaming Disabled on device %s\n",dev->name);
  
  
  del_timer(&lp->cell_timer);          
  ptr=lp->wavepoint_table.head;        
  while(ptr!=NULL)
    {
      old_ptr=ptr;
      ptr=ptr->next;	
      wl_del_wavepoint(old_ptr,lp);	
    }
}


static void wv_nwid_filter(unsigned char mode, net_local *lp)
{
  mm_t                  m;
  unsigned long         flags;
  
#ifdef WAVELAN_ROAMING_DEBUG
  printk(KERN_DEBUG "WaveLAN: NWID promisc %s, device %s\n",(mode==NWID_PROMISC) ? "on" : "off", lp->dev->name);
#endif
  
  
  spin_lock_irqsave(&lp->spinlock, flags);
  
  m.w.mmw_loopt_sel = (mode==NWID_PROMISC) ? MMW_LOOPT_SEL_DIS_NWID : 0x00;
  mmc_write(lp->dev->base_addr, (char *)&m.w.mmw_loopt_sel - (char *)&m, (unsigned char *)&m.w.mmw_loopt_sel, 1);
  
  if(mode==NWID_PROMISC)
    lp->cell_search=1;
  else
    lp->cell_search=0;

  
  spin_unlock_irqrestore(&lp->spinlock, flags);
}


static wavepoint_history *wl_roam_check(unsigned short nwid, net_local *lp)
{
  wavepoint_history	*ptr=lp->wavepoint_table.head;
  
  while(ptr!=NULL){
    if(ptr->nwid==nwid)
      return ptr;	
    ptr=ptr->next;
  }
  return NULL;
}


static wavepoint_history *wl_new_wavepoint(unsigned short nwid, unsigned char seq, net_local* lp)
{
  wavepoint_history *new_wavepoint;

#ifdef WAVELAN_ROAMING_DEBUG	
  printk(KERN_DEBUG "WaveLAN: New Wavepoint, NWID:%.4X\n",nwid);
#endif
  
  if(lp->wavepoint_table.num_wavepoints==MAX_WAVEPOINTS)
    return NULL;
  
  new_wavepoint = kmalloc(sizeof(wavepoint_history),GFP_ATOMIC);
  if(new_wavepoint==NULL)
    return NULL;
  
  new_wavepoint->nwid=nwid;                       
  new_wavepoint->average_fast=0;                    
  new_wavepoint->average_slow=0;
  new_wavepoint->qualptr=0;                       
  new_wavepoint->last_seq=seq-1;                
  memset(new_wavepoint->sigqual,0,WAVEPOINT_HISTORY);
  
  new_wavepoint->next=lp->wavepoint_table.head;
  new_wavepoint->prev=NULL;
  
  if(lp->wavepoint_table.head!=NULL)
    lp->wavepoint_table.head->prev=new_wavepoint;
  
  lp->wavepoint_table.head=new_wavepoint;
  
  lp->wavepoint_table.num_wavepoints++;     
  
  return new_wavepoint;
}


static void wl_del_wavepoint(wavepoint_history *wavepoint, struct net_local *lp)
{
  if(wavepoint==NULL)
    return;
  
  if(lp->curr_point==wavepoint)
    lp->curr_point=NULL;
  
  if(wavepoint->prev!=NULL)
    wavepoint->prev->next=wavepoint->next;
  
  if(wavepoint->next!=NULL)
    wavepoint->next->prev=wavepoint->prev;
  
  if(lp->wavepoint_table.head==wavepoint)
    lp->wavepoint_table.head=wavepoint->next;
  
  lp->wavepoint_table.num_wavepoints--;
  kfree(wavepoint);
}

 
static void wl_cell_expiry(unsigned long data)
{
  net_local *lp=(net_local *)data;
  wavepoint_history *wavepoint=lp->wavepoint_table.head,*old_point;
  
#if WAVELAN_ROAMING_DEBUG > 1
  printk(KERN_DEBUG "WaveLAN: Wavepoint timeout, dev %s\n",lp->dev->name);
#endif
  
  if(lp->wavepoint_table.locked)
    {
#if WAVELAN_ROAMING_DEBUG > 1
      printk(KERN_DEBUG "WaveLAN: Wavepoint table locked...\n");
#endif
      
      lp->cell_timer.expires=jiffies+1; 
      add_timer(&lp->cell_timer);
      return;
    }
  
  while(wavepoint!=NULL)
    {
      if(time_after(jiffies, wavepoint->last_seen + CELL_TIMEOUT))
	{
#ifdef WAVELAN_ROAMING_DEBUG
	  printk(KERN_DEBUG "WaveLAN: Bye bye %.4X\n",wavepoint->nwid);
#endif
	  
	  old_point=wavepoint;
	  wavepoint=wavepoint->next;
	  wl_del_wavepoint(old_point,lp);
	}
      else
	wavepoint=wavepoint->next;
    }
  lp->cell_timer.expires=jiffies+CELL_TIMEOUT;
  add_timer(&lp->cell_timer);
}


static void wl_update_history(wavepoint_history *wavepoint, unsigned char sigqual, unsigned char seq)	
{
  int i=0,num_missed=0,ptr=0;
  int average_fast=0,average_slow=0;
  
  num_missed=(seq-wavepoint->last_seq)%WAVEPOINT_HISTORY;
  if(num_missed)
    for(i=0;i<num_missed;i++)
      {
	wavepoint->sigqual[wavepoint->qualptr++]=0; 
	wavepoint->qualptr %=WAVEPOINT_HISTORY;    
      }
  wavepoint->last_seen=jiffies;                 
  wavepoint->last_seq=seq;	
  wavepoint->sigqual[wavepoint->qualptr++]=sigqual;          
  wavepoint->qualptr %=WAVEPOINT_HISTORY;
  ptr=(wavepoint->qualptr-WAVEPOINT_FAST_HISTORY+WAVEPOINT_HISTORY)%WAVEPOINT_HISTORY;
  
  for(i=0;i<WAVEPOINT_FAST_HISTORY;i++)       
    {
      average_fast+=wavepoint->sigqual[ptr++];
      ptr %=WAVEPOINT_HISTORY;
    }
  
  average_slow=average_fast;
  for(i=WAVEPOINT_FAST_HISTORY;i<WAVEPOINT_HISTORY;i++)
    {
      average_slow+=wavepoint->sigqual[ptr++];
      ptr %=WAVEPOINT_HISTORY;
    }
  
  wavepoint->average_fast=average_fast/WAVEPOINT_FAST_HISTORY;
  wavepoint->average_slow=average_slow/WAVEPOINT_HISTORY;	
}


static void wv_roam_handover(wavepoint_history *wavepoint, net_local *lp)
{
  unsigned int		base = lp->dev->base_addr;
  mm_t                  m;
  unsigned long         flags;

  if(wavepoint==lp->curr_point)          
    {
      wv_nwid_filter(!NWID_PROMISC,lp);
      return;
    }
  
#ifdef WAVELAN_ROAMING_DEBUG
  printk(KERN_DEBUG "WaveLAN: Doing handover to %.4X, dev %s\n",wavepoint->nwid,lp->dev->name);
#endif
 	
  
  spin_lock_irqsave(&lp->spinlock, flags);

  m.w.mmw_netw_id_l = wavepoint->nwid & 0xFF;
  m.w.mmw_netw_id_h = (wavepoint->nwid & 0xFF00) >> 8;
  
  mmc_write(base, (char *)&m.w.mmw_netw_id_l - (char *)&m, (unsigned char *)&m.w.mmw_netw_id_l, 2);
  
  
  spin_unlock_irqrestore(&lp->spinlock, flags);

  wv_nwid_filter(!NWID_PROMISC,lp);
  lp->curr_point=wavepoint;
}


static void wl_roam_gather(struct net_device *  dev,
			   u_char *  hdr,   
			   u_char *  stats) 
{
  wavepoint_beacon *beacon= (wavepoint_beacon *)hdr; 
  unsigned short nwid=ntohs(beacon->nwid);  
  unsigned short sigqual=stats[2] & MMR_SGNL_QUAL;   
  wavepoint_history *wavepoint=NULL;                
  net_local *lp = netdev_priv(dev);              

#ifdef I_NEED_THIS_FEATURE
  
  nwid=nwid^ntohs(beacon->domain_id);
#endif

#if WAVELAN_ROAMING_DEBUG > 1
  printk(KERN_DEBUG "WaveLAN: beacon, dev %s:\n",dev->name);
  printk(KERN_DEBUG "Domain: %.4X NWID: %.4X SigQual=%d\n",ntohs(beacon->domain_id),nwid,sigqual);
#endif
  
  lp->wavepoint_table.locked=1;                            
  
  wavepoint=wl_roam_check(nwid,lp);            
  if(wavepoint==NULL)                    
    {
      wavepoint=wl_new_wavepoint(nwid,beacon->seq,lp);
      if(wavepoint==NULL)
	goto out;
    }
  if(lp->curr_point==NULL)             
    wv_roam_handover(wavepoint, lp);	         
  
  wl_update_history(wavepoint, sigqual, beacon->seq); 
  
  if(lp->curr_point->average_slow < SEARCH_THRESH_LOW) 
    if(!lp->cell_search)                  
      wv_nwid_filter(NWID_PROMISC,lp);    
  
  if(wavepoint->average_slow > 
     lp->curr_point->average_slow + WAVELAN_ROAMING_DELTA)
    wv_roam_handover(wavepoint, lp);   
  
  if(lp->curr_point->average_slow > SEARCH_THRESH_HIGH) 
    if(lp->cell_search)  
      wv_nwid_filter(!NWID_PROMISC,lp);
  
out:
  lp->wavepoint_table.locked=0;                        
}


static inline int WAVELAN_BEACON(unsigned char *data)
{
  wavepoint_beacon *beacon= (wavepoint_beacon *)data;
  static const wavepoint_beacon beacon_template={0xaa,0xaa,0x03,0x08,0x00,0x0e,0x20,0x03,0x00};
  
  if(memcmp(beacon,&beacon_template,9)==0)
    return 1;
  else
    return 0;
}
#endif	






static int
wv_82593_cmd(struct net_device *	dev,
	     char *	str,
	     int	cmd,
	     int	result)
{
  unsigned int	base = dev->base_addr;
  int		status;
  int		wait_completed;
  long		spin;

  
  spin = 1000;
  do
    {
      
      udelay(10);

      
      outb(OP0_NOP | CR0_STATUS_3, LCCR(base));
      status = inb(LCSR(base));
    }
  while(((status & SR3_EXEC_STATE_MASK) != SR3_EXEC_IDLE) && (spin-- > 0));

  
  if (spin < 0) {
#ifdef DEBUG_INTERRUPT_ERROR
      printk(KERN_INFO "wv_82593_cmd: %s timeout (previous command), status 0x%02x\n",
	     str, status);
#endif
      return(FALSE);
    }

  
  outb(cmd, LCCR(base));

  
  if(result == SR0_NO_RESULT)
    return(TRUE);

  
  wait_completed = TRUE;

  
  spin = 1000;
  do
    {
      
      udelay(10);

      
      outb(CR0_STATUS_0 | OP0_NOP, LCCR(base));
      status = inb(LCSR(base));

      
      if((status & SR0_INTERRUPT))
	{
	  
	  outb(CR0_INT_ACK | OP0_NOP, LCCR(base));

	  
	  if(((status & SR0_BOTH_RX_TX) != SR0_BOTH_RX_TX) &&
	     ((status & SR0_BOTH_RX_TX) != 0x0) &&
	     !(status & SR0_RECEPTION))
	    {
	      
	      wait_completed = FALSE;
	    }
	  else
	    {
	      
#ifdef DEBUG_INTERRUPT_INFO
	      printk(KERN_INFO "wv_82593_cmd: not our interrupt\n");
#endif
	    }
	}
    }
  while(wait_completed && (spin-- > 0));

  
  if(wait_completed)
    {
#ifdef DEBUG_INTERRUPT_ERROR
      printk(KERN_INFO "wv_82593_cmd: %s timeout, status 0x%02x\n",
	     str, status);
#endif
      return(FALSE);
    }

  
  if((status & SR0_EVENT_MASK) != result)
    {
#ifdef DEBUG_INTERRUPT_ERROR
      printk(KERN_INFO "wv_82593_cmd: %s failed, status = 0x%x\n",
	     str, status);
#endif
      return(FALSE);
    }

  return(TRUE);
} 



static inline int
wv_diag(struct net_device *	dev)
{
  return(wv_82593_cmd(dev, "wv_diag(): diagnose",
		      OP0_DIAGNOSE, SR0_DIAGNOSE_PASSED));
} 



static int
read_ringbuf(struct net_device *	dev,
	     int	addr,
	     char *	buf,
	     int	len)
{
  unsigned int	base = dev->base_addr;
  int		ring_ptr = addr;
  int		chunk_len;
  char *	buf_ptr = buf;

  
  while(len > 0)
    {
      
      outb(ring_ptr & 0xff, PIORL(base));
      outb(((ring_ptr >> 8) & PIORH_MASK), PIORH(base));

      
      if((addr + len) < (RX_BASE + RX_SIZE))
	chunk_len = len;
      else
	chunk_len = RX_BASE + RX_SIZE - addr;
      insb(PIOP(base), buf_ptr, chunk_len);
      buf_ptr += chunk_len;
      len -= chunk_len;
      ring_ptr = (ring_ptr - RX_BASE + chunk_len) % RX_SIZE + RX_BASE;
    }
  return(ring_ptr);
} 



static void
wv_82593_reconfig(struct net_device *	dev)
{
  net_local *		lp = netdev_priv(dev);
  struct pcmcia_device *		link = lp->link;
  unsigned long		flags;

  
  lp->reconfig_82593 = TRUE;

  
  if((link->open) && (netif_running(dev)) && !(netif_queue_stopped(dev)))
    {
      spin_lock_irqsave(&lp->spinlock, flags);	
      wv_82593_config(dev);
      spin_unlock_irqrestore(&lp->spinlock, flags);	
    }
  else
    {
#ifdef DEBUG_IOCTL_INFO
      printk(KERN_DEBUG
	     "%s: wv_82593_reconfig(): delayed (state = %lX, link = %d)\n",
	     dev->name, dev->state, link->open);
#endif
    }
}




#ifdef DEBUG_PSA_SHOW


static void
wv_psa_show(psa_t *	p)
{
  printk(KERN_DEBUG "##### wavelan psa contents: #####\n");
  printk(KERN_DEBUG "psa_io_base_addr_1: 0x%02X %02X %02X %02X\n",
	 p->psa_io_base_addr_1,
	 p->psa_io_base_addr_2,
	 p->psa_io_base_addr_3,
	 p->psa_io_base_addr_4);
  printk(KERN_DEBUG "psa_rem_boot_addr_1: 0x%02X %02X %02X\n",
	 p->psa_rem_boot_addr_1,
	 p->psa_rem_boot_addr_2,
	 p->psa_rem_boot_addr_3);
  printk(KERN_DEBUG "psa_holi_params: 0x%02x, ", p->psa_holi_params);
  printk("psa_int_req_no: %d\n", p->psa_int_req_no);
#ifdef DEBUG_SHOW_UNUSED
  printk(KERN_DEBUG "psa_unused0[]: %pM\n", p->psa_unused0);
#endif	
  printk(KERN_DEBUG "psa_univ_mac_addr[]: %pM\n", p->psa_univ_mac_addr);
  printk(KERN_DEBUG "psa_local_mac_addr[]: %pM\n", p->psa_local_mac_addr);
  printk(KERN_DEBUG "psa_univ_local_sel: %d, ", p->psa_univ_local_sel);
  printk("psa_comp_number: %d, ", p->psa_comp_number);
  printk("psa_thr_pre_set: 0x%02x\n", p->psa_thr_pre_set);
  printk(KERN_DEBUG "psa_feature_select/decay_prm: 0x%02x, ",
	 p->psa_feature_select);
  printk("psa_subband/decay_update_prm: %d\n", p->psa_subband);
  printk(KERN_DEBUG "psa_quality_thr: 0x%02x, ", p->psa_quality_thr);
  printk("psa_mod_delay: 0x%02x\n", p->psa_mod_delay);
  printk(KERN_DEBUG "psa_nwid: 0x%02x%02x, ", p->psa_nwid[0], p->psa_nwid[1]);
  printk("psa_nwid_select: %d\n", p->psa_nwid_select);
  printk(KERN_DEBUG "psa_encryption_select: %d, ", p->psa_encryption_select);
  printk("psa_encryption_key[]: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
	 p->psa_encryption_key[0],
	 p->psa_encryption_key[1],
	 p->psa_encryption_key[2],
	 p->psa_encryption_key[3],
	 p->psa_encryption_key[4],
	 p->psa_encryption_key[5],
	 p->psa_encryption_key[6],
	 p->psa_encryption_key[7]);
  printk(KERN_DEBUG "psa_databus_width: %d\n", p->psa_databus_width);
  printk(KERN_DEBUG "psa_call_code/auto_squelch: 0x%02x, ",
	 p->psa_call_code[0]);
  printk("psa_call_code[]: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
	 p->psa_call_code[0],
	 p->psa_call_code[1],
	 p->psa_call_code[2],
	 p->psa_call_code[3],
	 p->psa_call_code[4],
	 p->psa_call_code[5],
	 p->psa_call_code[6],
	 p->psa_call_code[7]);
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


static void
wv_mmc_show(struct net_device *	dev)
{
  unsigned int	base = dev->base_addr;
  net_local *	lp = netdev_priv(dev);
  mmr_t		m;

  
  if(hasr_read(base) & HASR_NO_CLK)
    {
      printk(KERN_WARNING "%s: wv_mmc_show: modem not connected\n",
	     dev->name);
      return;
    }

  spin_lock_irqsave(&lp->spinlock, flags);

  
  mmc_out(base, mmwoff(0, mmw_freeze), 1);
  mmc_read(base, 0, (u_char *)&m, sizeof(m));
  mmc_out(base, mmwoff(0, mmw_freeze), 0);

  
  lp->wstats.discard.nwid += (m.mmr_wrong_nwid_h << 8) | m.mmr_wrong_nwid_l;

  spin_unlock_irqrestore(&lp->spinlock, flags);

  printk(KERN_DEBUG "##### wavelan modem status registers: #####\n");
#ifdef DEBUG_SHOW_UNUSED
  printk(KERN_DEBUG "mmc_unused0[]: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
	 m.mmr_unused0[0],
	 m.mmr_unused0[1],
	 m.mmr_unused0[2],
	 m.mmr_unused0[3],
	 m.mmr_unused0[4],
	 m.mmr_unused0[5],
	 m.mmr_unused0[6],
	 m.mmr_unused0[7]);
#endif	
  printk(KERN_DEBUG "Encryption algorithm: %02X - Status: %02X\n",
	 m.mmr_des_avail, m.mmr_des_status);
#ifdef DEBUG_SHOW_UNUSED
  printk(KERN_DEBUG "mmc_unused1[]: %02X:%02X:%02X:%02X:%02X\n",
	 m.mmr_unused1[0],
	 m.mmr_unused1[1],
	 m.mmr_unused1[2],
	 m.mmr_unused1[3],
	 m.mmr_unused1[4]);
#endif	
  printk(KERN_DEBUG "dce_status: 0x%x [%s%s%s%s]\n",
	 m.mmr_dce_status,
	 (m.mmr_dce_status & MMR_DCE_STATUS_RX_BUSY) ? "energy detected,":"",
	 (m.mmr_dce_status & MMR_DCE_STATUS_LOOPT_IND) ?
	 "loop test indicated," : "",
	 (m.mmr_dce_status & MMR_DCE_STATUS_TX_BUSY) ? "transmitter on," : "",
	 (m.mmr_dce_status & MMR_DCE_STATUS_JBR_EXPIRED) ?
	 "jabber timer expired," : "");
  printk(KERN_DEBUG "Dsp ID: %02X\n",
	 m.mmr_dsp_id);
#ifdef DEBUG_SHOW_UNUSED
  printk(KERN_DEBUG "mmc_unused2[]: %02X:%02X\n",
	 m.mmr_unused2[0],
	 m.mmr_unused2[1]);
#endif	
  printk(KERN_DEBUG "# correct_nwid: %d, # wrong_nwid: %d\n",
	 (m.mmr_correct_nwid_h << 8) | m.mmr_correct_nwid_l,
	 (m.mmr_wrong_nwid_h << 8) | m.mmr_wrong_nwid_l);
  printk(KERN_DEBUG "thr_pre_set: 0x%x [current signal %s]\n",
	 m.mmr_thr_pre_set & MMR_THR_PRE_SET,
	 (m.mmr_thr_pre_set & MMR_THR_PRE_SET_CUR) ? "above" : "below");
  printk(KERN_DEBUG "signal_lvl: %d [%s], ",
	 m.mmr_signal_lvl & MMR_SIGNAL_LVL,
	 (m.mmr_signal_lvl & MMR_SIGNAL_LVL_VALID) ? "new msg" : "no new msg");
  printk("silence_lvl: %d [%s], ", m.mmr_silence_lvl & MMR_SILENCE_LVL,
	 (m.mmr_silence_lvl & MMR_SILENCE_LVL_VALID) ? "update done" : "no new update");
  printk("sgnl_qual: 0x%x [%s]\n", m.mmr_sgnl_qual & MMR_SGNL_QUAL,
	 (m.mmr_sgnl_qual & MMR_SGNL_QUAL_ANT) ? "Antenna 1" : "Antenna 0");
#ifdef DEBUG_SHOW_UNUSED
  printk(KERN_DEBUG "netw_id_l: %x\n", m.mmr_netw_id_l);
#endif	
} 
#endif	

#ifdef DEBUG_I82593_SHOW


static void
wv_ru_show(struct net_device *	dev)
{
  net_local *lp = netdev_priv(dev);

  printk(KERN_DEBUG "##### wavelan i82593 receiver status: #####\n");
  printk(KERN_DEBUG "ru: rfp %d stop %d", lp->rfp, lp->stop);
  
  printk("\n");
} 
#endif	

#ifdef DEBUG_DEVICE_SHOW


static void
wv_dev_show(struct net_device *	dev)
{
  printk(KERN_DEBUG "dev:");
  printk(" state=%lX,", dev->state);
  printk(" trans_start=%ld,", dev->trans_start);
  printk(" flags=0x%x,", dev->flags);
  printk("\n");
} 



static void
wv_local_show(struct net_device *	dev)
{
  net_local *lp = netdev_priv(dev);

  printk(KERN_DEBUG "local:");
  
  printk("\n");
} 
#endif	

#if defined(DEBUG_RX_INFO) || defined(DEBUG_TX_INFO)


static void
wv_packet_info(u_char *		p,		
	       int		length,		
	       char *		msg1,		
	       char *		msg2)		
{
  int		i;
  int		maxi;

  printk(KERN_DEBUG "%s: %s(): dest %pM, length %d\n",
	 msg1, msg2, p, length);
  printk(KERN_DEBUG "%s: %s(): src %pM, type 0x%02X%02X\n",
	 msg1, msg2, &p[6], p[12], p[13]);

#ifdef DEBUG_PACKET_DUMP

  printk(KERN_DEBUG "data=\"");

  if((maxi = length) > DEBUG_PACKET_DUMP)
    maxi = DEBUG_PACKET_DUMP;
  for(i = 14; i < maxi; i++)
    if(p[i] >= ' ' && p[i] <= '~')
      printk(" %c", p[i]);
    else
      printk("%02X", p[i]);
  if(maxi < length)
    printk("..");
  printk("\"\n");
  printk(KERN_DEBUG "\n");
#endif	
}
#endif	



static void
wv_init_info(struct net_device *	dev)
{
  unsigned int	base = dev->base_addr;
  psa_t		psa;

  
  psa_read(dev, 0, (unsigned char *) &psa, sizeof(psa));

#ifdef DEBUG_PSA_SHOW
  wv_psa_show(&psa);
#endif
#ifdef DEBUG_MMC_SHOW
  wv_mmc_show(dev);
#endif
#ifdef DEBUG_I82593_SHOW
  wv_ru_show(dev);
#endif

#ifdef DEBUG_BASIC_SHOW
  
  printk(KERN_NOTICE "%s: WaveLAN: port %#x, irq %d, hw_addr %pM",
	 dev->name, base, dev->irq, dev->dev_addr);

  
  if(psa.psa_nwid_select)
    printk(", nwid 0x%02X-%02X", psa.psa_nwid[0], psa.psa_nwid[1]);
  else
    printk(", nwid off");

  
  if(!(mmc_in(base, mmroff(0, mmr_fee_status)) &
       (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY)))
    {
      unsigned short	freq;

      
      fee_read(base, 0x00 ,
	       &freq, 1);

      
      printk(", 2.00, %ld", (freq >> 6) + 2400L);

      
      if(freq & 0x20)
	printk(".5");
    }
  else
    {
      printk(", PCMCIA, ");
      switch (psa.psa_subband)
	{
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
	  printk("unknown");
	}
    }

  printk(" MHz\n");
#endif	

#ifdef DEBUG_VERSION_SHOW
  
  printk(KERN_NOTICE "%s", version);
#endif
} 








static void
wavelan_set_multicast_list(struct net_device *	dev)
{
  net_local *	lp = netdev_priv(dev);

#ifdef DEBUG_IOCTL_TRACE
  printk(KERN_DEBUG "%s: ->wavelan_set_multicast_list()\n", dev->name);
#endif

#ifdef DEBUG_IOCTL_INFO
  printk(KERN_DEBUG "%s: wavelan_set_multicast_list(): setting Rx mode %02X to %d addresses.\n",
	 dev->name, dev->flags, dev->mc_count);
#endif

  if(dev->flags & IFF_PROMISC)
    {
      
      if(!lp->promiscuous)
	{
	  lp->promiscuous = 1;
	  lp->allmulticast = 0;
	  lp->mc_count = 0;

	  wv_82593_reconfig(dev);
	}
    }
  else
    
    if((dev->flags & IFF_ALLMULTI) ||
       (dev->mc_count > I82593_MAX_MULTICAST_ADDRESSES))
      {
	
	if(!lp->allmulticast)
	  {
	    lp->promiscuous = 0;
	    lp->allmulticast = 1;
	    lp->mc_count = 0;

	    wv_82593_reconfig(dev);
	  }
      }
    else
      
      if(dev->mc_list != (struct dev_mc_list *) NULL)
	{
	  
#ifdef MULTICAST_AVOID
	  if(lp->promiscuous || lp->allmulticast ||
	     (dev->mc_count != lp->mc_count))
#endif
	    {
	      lp->promiscuous = 0;
	      lp->allmulticast = 0;
	      lp->mc_count = dev->mc_count;

	      wv_82593_reconfig(dev);
	    }
	}
      else
	{
	  
	  if(lp->promiscuous || lp->mc_count == 0)
	    {
	      lp->promiscuous = 0;
	      lp->allmulticast = 0;
	      lp->mc_count = 0;

	      wv_82593_reconfig(dev);
	    }
	}
#ifdef DEBUG_IOCTL_TRACE
  printk(KERN_DEBUG "%s: <-wavelan_set_multicast_list()\n", dev->name);
#endif
}



#ifdef SET_MAC_ADDRESS
static int
wavelan_set_mac_address(struct net_device *	dev,
			void *		addr)
{
  struct sockaddr *	mac = addr;

  
  memcpy(dev->dev_addr, mac->sa_data, WAVELAN_ADDR_SIZE);

  
  wv_82593_reconfig(dev);

  return 0;
}
#endif	




static int
wv_set_frequency(u_long		base,	
		 iw_freq *	frequency)
{
  const int	BAND_NUM = 10;	
  long		freq = 0L;	
#ifdef DEBUG_IOCTL_INFO
  int		i;
#endif

  
  
  if((frequency->e == 1) &&
     (frequency->m >= (int) 2.412e8) && (frequency->m <= (int) 2.487e8))
    {
      freq = ((frequency->m / 10000) - 24000L) / 5;
    }

  
  
  if((frequency->e == 0) &&
     (frequency->m >= 0) && (frequency->m < BAND_NUM))
    {
      
      freq = channel_bands[frequency->m] >> 1;
    }

  
  if(freq != 0L)
    {
      u_short	table[10];	

      
      fee_read(base, 0x71 ,
	       table, 10);

#ifdef DEBUG_IOCTL_INFO
      printk(KERN_DEBUG "Frequency table :");
      for(i = 0; i < 10; i++)
	{
	  printk(" %04X",
		 table[i]);
	}
      printk("\n");
#endif

      
      if(!(table[9 - ((freq - 24) / 16)] &
	   (1 << ((freq - 24) % 16))))
	return -EINVAL;		
    }
  else
    return -EINVAL;

  
  if(freq != 0L)
    {
      unsigned short	area[16];
      unsigned short	dac[2];
      unsigned short	area_verify[16];
      unsigned short	dac_verify[2];
      
      unsigned short	power_limit[] = { 40, 80, 120, 160, 0 };
      int		power_band = 0;		
      unsigned short	power_adjust;		

      
      power_band = 0;
      while((freq > power_limit[power_band]) &&
	    (power_limit[++power_band] != 0))
	;

      
      fee_read(base, 0x00,
	       area, 16);

      
      fee_read(base, 0x60,
	       dac, 2);

      
      fee_read(base, 0x6B - (power_band >> 1),
	       &power_adjust, 1);
      if(power_band & 0x1)
	power_adjust >>= 8;
      else
	power_adjust &= 0xFF;

#ifdef DEBUG_IOCTL_INFO
      printk(KERN_DEBUG "Wavelan EEprom Area 1 :");
      for(i = 0; i < 16; i++)
	{
	  printk(" %04X",
		 area[i]);
	}
      printk("\n");

      printk(KERN_DEBUG "Wavelan EEprom DAC : %04X %04X\n",
	     dac[0], dac[1]);
#endif

      
      area[0] = ((freq << 5) & 0xFFE0) | (area[0] & 0x1F);

      
      area[3] = (freq >> 1) + 2400L - 352L;
      area[2] = ((freq & 0x1) << 4) | (area[2] & 0xFFEF);

      
      area[13] = (freq >> 1) + 2400L;
      area[12] = ((freq & 0x1) << 4) | (area[2] & 0xFFEF);

      

      
      dac[1] = ((power_adjust >> 1) & 0x7F) | (dac[1] & 0xFF80);
      dac[0] = ((power_adjust & 0x1) << 4) | (dac[0] & 0xFFEF);

      
      fee_write(base, 0x00,
		area, 16);

      
      fee_write(base, 0x60,
		dac, 2);

      

      
      fee_read(base, 0x00,
	       area_verify, 16);

      
      fee_read(base, 0x60,
	       dac_verify, 2);

      
      if(memcmp(area, area_verify, 16 * 2) ||
	 memcmp(dac, dac_verify, 2 * 2))
	{
#ifdef DEBUG_IOCTL_ERROR
	  printk(KERN_INFO "Wavelan: wv_set_frequency : unable to write new frequency to EEprom (?)\n");
#endif
	  return -EOPNOTSUPP;
	}

      
      mmc_out(base, mmwoff(0, mmw_fee_addr), 0x0F);
      mmc_out(base, mmwoff(0, mmw_fee_ctrl),
	      MMW_FEE_CTRL_READ | MMW_FEE_CTRL_DWLD);

      
      fee_wait(base, 100, 100);

      
      mmc_out(base, mmwoff(0, mmw_fee_addr), 0x61);
      mmc_out(base, mmwoff(0, mmw_fee_ctrl),
	      MMW_FEE_CTRL_READ | MMW_FEE_CTRL_DWLD);

      
      fee_wait(base, 100, 100);

#ifdef DEBUG_IOCTL_INFO
      

      printk(KERN_DEBUG "Wavelan EEprom Area 1 :");
      for(i = 0; i < 16; i++)
	{
	  printk(" %04X",
		 area_verify[i]);
	}
      printk("\n");

      printk(KERN_DEBUG "Wavelan EEprom DAC : %04X %04X\n",
	     dac_verify[0], dac_verify[1]);
#endif

      return 0;
    }
  else
    return -EINVAL;		
}



static int
wv_frequency_list(u_long	base,	
		  iw_freq *	list,	
		  int		max)	
{
  u_short	table[10];	
  long		freq = 0L;	
  int		i;		
  const int	BAND_NUM = 10;	
  int		c = 0;		

  
  fee_read(base, 0x71 ,
	   table, 10);

  
  i = 0;
  for(freq = 0; freq < 150; freq++)
    
    if(table[9 - (freq / 16)] & (1 << (freq % 16)))
      {
	
	while((((channel_bands[c] >> 1) - 24) < freq) &&
	      (c < BAND_NUM))
	  c++;
	list[i].i = c;	

	
	list[i].m = (((freq + 24) * 5) + 24000L) * 10000;
	list[i++].e = 1;

	
	if(i >= max)
	  return(i);
      }

  return(i);
}

#ifdef IW_WIRELESS_SPY


static inline void
wl_spy_gather(struct net_device *	dev,
	      u_char *	mac,		
	      u_char *	stats)		
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


static inline void
wl_his_gather(struct net_device *	dev,
	      u_char *	stats)		
{
  net_local *	lp = netdev_priv(dev);
  u_char	level = stats[0] & MMR_SIGNAL_LVL;
  int		i;

  
  i = 0;
  while((i < (lp->his_number - 1)) && (level >= lp->his_range[i++]))
    ;

  
  (lp->his_sum[i])++;
}
#endif	

static void wl_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	strncpy(info->driver, "wavelan_cs", sizeof(info->driver)-1);
}

static const struct ethtool_ops ops = {
	.get_drvinfo = wl_get_drvinfo
};



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
	unsigned int base = dev->base_addr;
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
		psa_write(dev,
			  (char *) psa.psa_nwid - (char *) &psa,
			  (unsigned char *) psa.psa_nwid, 3);

		
		m.w.mmw_netw_id_l = psa.psa_nwid[1];
		m.w.mmw_netw_id_h = psa.psa_nwid[0];
		mmc_write(base,
			  (char *) &m.w.mmw_netw_id_l -
			  (char *) &m,
			  (unsigned char *) &m.w.mmw_netw_id_l, 2);
		mmc_out(base, mmwoff(0, mmw_loopt_sel), 0x00);
	} else {
		
		psa.psa_nwid_select = 0x00;
		psa_write(dev,
			  (char *) &psa.psa_nwid_select -
			  (char *) &psa,
			  (unsigned char *) &psa.psa_nwid_select,
			  1);

		
		mmc_out(base, mmwoff(0, mmw_loopt_sel),
			MMW_LOOPT_SEL_DIS_NWID);
	}
	
	update_psa_checksum(dev);

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_nwid(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	net_local *lp = netdev_priv(dev);
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	psa_read(dev,
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
	unsigned int base = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	unsigned long flags;
	int ret;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if (!(mmc_in(base, mmroff(0, mmr_fee_status)) &
	      (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY)))
		ret = wv_set_frequency(base, &(wrqu->freq));
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
	unsigned int base = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if (!(mmc_in(base, mmroff(0, mmr_fee_status)) &
	      (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY))) {
		unsigned short freq;

		
		fee_read(base, 0x00, &freq, 1);
		wrqu->freq.m = ((freq >> 5) * 5 + 24000L) * 10000;
		wrqu->freq.e = 1;
	} else {
		psa_read(dev,
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
	unsigned int base = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	
	psa.psa_thr_pre_set = wrqu->sens.value & 0x3F;
	psa_write(dev,
		  (char *) &psa.psa_thr_pre_set - (char *) &psa,
		  (unsigned char *) &psa.psa_thr_pre_set, 1);
	
	update_psa_checksum(dev);
	mmc_out(base, mmwoff(0, mmw_thr_pre_set),
		psa.psa_thr_pre_set);

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_sens(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	net_local *lp = netdev_priv(dev);
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	psa_read(dev,
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
	unsigned int base = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	unsigned long flags;
	psa_t psa;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);

	
	if (!mmc_encr(base)) {
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

			psa_write(dev,
				  (char *) &psa.psa_encryption_select -
				  (char *) &psa,
				  (unsigned char *) &psa.
				  psa_encryption_select, 8 + 1);

			mmc_out(base, mmwoff(0, mmw_encr_enable),
				MMW_ENCR_ENABLE_EN | MMW_ENCR_ENABLE_MODE);
			mmc_write(base, mmwoff(0, mmw_encr_key),
				  (unsigned char *) &psa.
				  psa_encryption_key, 8);
		}

		
		if (wrqu->encoding.flags & IW_ENCODE_DISABLED) {
			psa.psa_encryption_select = 0;
			psa_write(dev,
				  (char *) &psa.psa_encryption_select -
				  (char *) &psa,
				  (unsigned char *) &psa.
				  psa_encryption_select, 1);

			mmc_out(base, mmwoff(0, mmw_encr_enable), 0);
		}
		
		update_psa_checksum(dev);
	}

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_encode(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu,
			      char *extra)
{
	unsigned int base = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	psa_t psa;
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if (!mmc_encr(base)) {
		ret = -EOPNOTSUPP;
	} else {
		
		psa_read(dev,
			 (char *) &psa.psa_encryption_select -
			 (char *) &psa,
			 (unsigned char *) &psa.
			 psa_encryption_select, 1 + 8);

		
		if (psa.psa_encryption_select)
			wrqu->encoding.flags = IW_ENCODE_ENABLED;
		else
			wrqu->encoding.flags = IW_ENCODE_DISABLED;
		wrqu->encoding.flags |= mmc_encr(base);

		
		wrqu->encoding.length = 8;
		memcpy(extra, psa.psa_encryption_key, wrqu->encoding.length);
	}

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}

#ifdef WAVELAN_ROAMING_EXT


static int wavelan_set_essid(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	net_local *lp = netdev_priv(dev);
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if(wrqu->data.flags == 0)
		lp->filter_domains = 0;
	else {
		char	essid[IW_ESSID_MAX_SIZE + 1];
		char *	endp;

		
		memcpy(essid, extra, wrqu->data.length);
		essid[IW_ESSID_MAX_SIZE] = '\0';

#ifdef DEBUG_IOCTL_INFO
		printk(KERN_DEBUG "SetEssid : ``%s''\n", essid);
#endif	

		
		lp->domain_id = simple_strtoul(essid, &endp, 16);
		
		if(endp > essid)
			lp->filter_domains = 1;
		else {
			lp->filter_domains = 0;
			ret = -EINVAL;
		}
	}

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_essid(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	net_local *lp = netdev_priv(dev);

	
	wrqu->data.flags = lp->filter_domains;

	
	
	sprintf(extra, "%lX", lp->domain_id);
	extra[IW_ESSID_MAX_SIZE] = '\0';

	
	wrqu->data.length = strlen(extra);

	return 0;
}



static int wavelan_set_wap(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu,
			   char *extra)
{
#ifdef DEBUG_IOCTL_INFO
	printk(KERN_DEBUG "Set AP to : %pM\n", wrqu->ap_addr.sa_data);
#endif	

	return -EOPNOTSUPP;
}



static int wavelan_get_wap(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu,
			   char *extra)
{
	
	memcpy(wrqu->ap_addr.sa_data, dev->dev_addr, WAVELAN_ADDR_SIZE);
	wrqu->ap_addr.sa_family = ARPHRD_ETHER;

	return -EOPNOTSUPP;
}
#endif	

#ifdef WAVELAN_ROAMING


static int wavelan_set_mode(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	net_local *lp = netdev_priv(dev);
	unsigned long flags;
	int ret = 0;

	
	spin_lock_irqsave(&lp->spinlock, flags);

	
	switch(wrqu->mode) {
	case IW_MODE_ADHOC:
		if(do_roaming) {
			wv_roam_cleanup(dev);
			do_roaming = 0;
		}
		break;
	case IW_MODE_INFRA:
		if(!do_roaming) {
			wv_roam_init(dev);
			do_roaming = 1;
		}
		break;
	default:
		ret = -EINVAL;
	}

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return ret;
}



static int wavelan_get_mode(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	if(do_roaming)
		wrqu->mode = IW_MODE_INFRA;
	else
		wrqu->mode = IW_MODE_ADHOC;

	return 0;
}
#endif	



static int wavelan_get_range(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	unsigned int base = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	struct iw_range *range = (struct iw_range *) extra;
	unsigned long flags;
	int ret = 0;

	
	wrqu->data.length = sizeof(struct iw_range);

	
	memset(range, 0, sizeof(struct iw_range));

	
	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 9;

	
	range->throughput = 1.4 * 1000 * 1000;	
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
				IW_EVENT_CAPA_MASK(0x8B04) |
				IW_EVENT_CAPA_MASK(0x8B06));
	range->event_capa[1] = IW_EVENT_CAPA_K_1;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if (!(mmc_in(base, mmroff(0, mmr_fee_status)) &
	      (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY))) {
		range->num_channels = 10;
		range->num_frequency = wv_frequency_list(base, range->freq,
							IW_MAX_FREQUENCIES);
	} else
		range->num_channels = range->num_frequency = 0;

	
	if (mmc_encr(base)) {
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
	unsigned int base = dev->base_addr;
	net_local *lp = netdev_priv(dev);
	psa_t psa;
	unsigned long flags;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	psa.psa_quality_thr = *(extra) & 0x0F;
	psa_write(dev,
		  (char *) &psa.psa_quality_thr - (char *) &psa,
		  (unsigned char *) &psa.psa_quality_thr, 1);
	
	update_psa_checksum(dev);
	mmc_out(base, mmwoff(0, mmw_quality_thr),
		psa.psa_quality_thr);

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return 0;
}



static int wavelan_get_qthr(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	net_local *lp = netdev_priv(dev);
	psa_t psa;
	unsigned long flags;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	psa_read(dev,
		 (char *) &psa.psa_quality_thr - (char *) &psa,
		 (unsigned char *) &psa.psa_quality_thr, 1);
	*(extra) = psa.psa_quality_thr & 0x0F;

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return 0;
}

#ifdef WAVELAN_ROAMING


static int wavelan_set_roam(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	net_local *lp = netdev_priv(dev);
	unsigned long flags;

	
	spin_lock_irqsave(&lp->spinlock, flags);
	
	
	if(do_roaming && (*extra)==0)
		wv_roam_cleanup(dev);
	else if(do_roaming==0 && (*extra)!=0)
		wv_roam_init(dev);

	do_roaming = (*extra);

	
	spin_unlock_irqrestore(&lp->spinlock, flags);

	return 0;
}



static int wavelan_get_roam(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu,
			    char *extra)
{
	*(extra) = do_roaming;

	return 0;
}
#endif	

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




static const struct iw_priv_args wavelan_private_args[] = {

  { SIOCSIPQTHR, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, 0, "setqualthr" },
  { SIOCGIPQTHR, 0, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, "getqualthr" },
  { SIOCSIPROAM, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, 0, "setroam" },
  { SIOCGIPROAM, 0, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, "getroam" },
  { SIOCSIPHISTO, IW_PRIV_TYPE_BYTE | 16,                    0, "sethisto" },
  { SIOCGIPHISTO, 0,                     IW_PRIV_TYPE_INT | 16, "gethisto" },
};

static const iw_handler		wavelan_handler[] =
{
	NULL,				
	wavelan_get_name,		
	wavelan_set_nwid,		
	wavelan_get_nwid,		
	wavelan_set_freq,		
	wavelan_get_freq,		
#ifdef WAVELAN_ROAMING
	wavelan_set_mode,		
	wavelan_get_mode,		
#else	
	NULL,				
	NULL,				
#endif	
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
#ifdef WAVELAN_ROAMING_EXT
	wavelan_set_wap,		
	wavelan_get_wap,		
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	wavelan_set_essid,		
	wavelan_get_essid,		
#else	
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
	NULL,				
#endif	
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
#ifdef WAVELAN_ROAMING
	wavelan_set_roam,		
	wavelan_get_roam,		
#else	
	NULL,				
	NULL,				
#endif	
#ifdef HISTOGRAM
	wavelan_set_histo,		
	wavelan_get_histo,		
#endif	
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



static iw_stats *
wavelan_get_wireless_stats(struct net_device *	dev)
{
  unsigned int		base = dev->base_addr;
  net_local *		lp = netdev_priv(dev);
  mmr_t			m;
  iw_stats *		wstats;
  unsigned long		flags;

#ifdef DEBUG_IOCTL_TRACE
  printk(KERN_DEBUG "%s: ->wavelan_get_wireless_stats()\n", dev->name);
#endif

  
  spin_lock_irqsave(&lp->spinlock, flags);

  wstats = &lp->wstats;

  
  mmc_out(base, mmwoff(0, mmw_freeze), 1);

  mmc_read(base, mmroff(0, mmr_dce_status), &m.mmr_dce_status, 1);
  mmc_read(base, mmroff(0, mmr_wrong_nwid_l), &m.mmr_wrong_nwid_l, 2);
  mmc_read(base, mmroff(0, mmr_thr_pre_set), &m.mmr_thr_pre_set, 4);

  mmc_out(base, mmwoff(0, mmw_freeze), 0);

  
  wstats->status = m.mmr_dce_status & MMR_DCE_STATUS;
  wstats->qual.qual = m.mmr_sgnl_qual & MMR_SGNL_QUAL;
  wstats->qual.level = m.mmr_signal_lvl & MMR_SIGNAL_LVL;
  wstats->qual.noise = m.mmr_silence_lvl & MMR_SILENCE_LVL;
  wstats->qual.updated = (((m.mmr_signal_lvl & MMR_SIGNAL_LVL_VALID) >> 7) |
			  ((m.mmr_signal_lvl & MMR_SIGNAL_LVL_VALID) >> 6) |
			  ((m.mmr_silence_lvl & MMR_SILENCE_LVL_VALID) >> 5));
  wstats->discard.nwid += (m.mmr_wrong_nwid_h << 8) | m.mmr_wrong_nwid_l;
  wstats->discard.code = 0L;
  wstats->discard.misc = 0L;

  
  spin_unlock_irqrestore(&lp->spinlock, flags);

#ifdef DEBUG_IOCTL_TRACE
  printk(KERN_DEBUG "%s: <-wavelan_get_wireless_stats()\n", dev->name);
#endif
  return &lp->wstats;
}






static int
wv_start_of_frame(struct net_device *	dev,
		  int		rfp,	
		  int		wrap)	
{
  unsigned int	base = dev->base_addr;
  int		rp;
  int		len;

  rp = (rfp - 5 + RX_SIZE) % RX_SIZE;
  outb(rp & 0xff, PIORL(base));
  outb(((rp >> 8) & PIORH_MASK), PIORH(base));
  len = inb(PIOP(base));
  len |= inb(PIOP(base)) << 8;

  
  
  if(len > MAXDATAZ + 100)
    {
#ifdef DEBUG_RX_ERROR
      printk(KERN_INFO "%s: wv_start_of_frame: Received frame too large, rfp %d len 0x%x\n",
	     dev->name, rfp, len);
#endif
      return(-1);
    }
  
  
  if(len < 7)
    {
#ifdef DEBUG_RX_ERROR
      printk(KERN_INFO "%s: wv_start_of_frame: Received null frame, rfp %d len 0x%x\n",
	     dev->name, rfp, len);
#endif
      return(-1);
    }
  
  
  if(len > ((wrap - (rfp - len) + RX_SIZE) % RX_SIZE))	
    {
#ifdef DEBUG_RX_ERROR
      printk(KERN_INFO "%s: wv_start_of_frame: wrap around buffer, wrap %d rfp %d len 0x%x\n",
	     dev->name, wrap, rfp, len);
#endif
      return(-1);
    }

  return((rp - len + RX_SIZE) % RX_SIZE);
} 



static void
wv_packet_read(struct net_device *		dev,
	       int		fd_p,
	       int		sksize)
{
  net_local *		lp = netdev_priv(dev);
  struct sk_buff *	skb;

#ifdef DEBUG_RX_TRACE
  printk(KERN_DEBUG "%s: ->wv_packet_read(0x%X, %d)\n",
	 dev->name, fd_p, sksize);
#endif

  
  if((skb = dev_alloc_skb(sksize+2)) == (struct sk_buff *) NULL)
    {
#ifdef DEBUG_RX_ERROR
      printk(KERN_INFO "%s: wv_packet_read(): could not alloc_skb(%d, GFP_ATOMIC)\n",
	     dev->name, sksize);
#endif
      dev->stats.rx_dropped++;
      
      return;
    }

  skb_reserve(skb, 2);
  fd_p = read_ringbuf(dev, fd_p, (char *) skb_put(skb, sksize), sksize);
  skb->protocol = eth_type_trans(skb, dev);

#ifdef DEBUG_RX_INFO
  wv_packet_info(skb_mac_header(skb), sksize, dev->name, "wv_packet_read");
#endif	
     
  
  if(
#ifdef IW_WIRELESS_SPY
     (lp->spy_data.spy_number > 0) ||
#endif	
#ifdef HISTOGRAM
     (lp->his_number > 0) ||
#endif	
#ifdef WAVELAN_ROAMING
     (do_roaming) ||
#endif	
     0)
    {
      u_char	stats[3];	

      
      fd_p = read_ringbuf(dev, (fd_p + 4) % RX_SIZE + RX_BASE,
			  stats, 3);
#ifdef DEBUG_RX_INFO
      printk(KERN_DEBUG "%s: wv_packet_read(): Signal level %d/63, Silence level %d/63, signal quality %d/16\n",
	     dev->name, stats[0] & 0x3F, stats[1] & 0x3F, stats[2] & 0x0F);
#endif

#ifdef WAVELAN_ROAMING
      if(do_roaming)
	if(WAVELAN_BEACON(skb->data))
	  wl_roam_gather(dev, skb->data, stats);
#endif	
	  
#ifdef WIRELESS_SPY
      wl_spy_gather(dev, skb_mac_header(skb) + WAVELAN_ADDR_SIZE, stats);
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
  return;
}



static void
wv_packet_rcv(struct net_device *	dev)
{
  unsigned int	base = dev->base_addr;
  net_local *	lp = netdev_priv(dev);
  int		newrfp;
  int		rp;
  int		len;
  int		f_start;
  int		status;
  int		i593_rfp;
  int		stat_ptr;
  u_char	c[4];

#ifdef DEBUG_RX_TRACE
  printk(KERN_DEBUG "%s: ->wv_packet_rcv()\n", dev->name);
#endif

  
  outb(CR0_STATUS_2 | OP0_NOP, LCCR(base));
  i593_rfp = inb(LCSR(base));
  i593_rfp |= inb(LCSR(base)) << 8;
  i593_rfp %= RX_SIZE;

  
  newrfp = inb(RPLL(base));
  newrfp |= inb(RPLH(base)) << 8;
  newrfp %= RX_SIZE;

#ifdef DEBUG_RX_INFO
  printk(KERN_DEBUG "%s: wv_packet_rcv(): i593_rfp %d stop %d newrfp %d lp->rfp %d\n",
	 dev->name, i593_rfp, lp->stop, newrfp, lp->rfp);
#endif

#ifdef DEBUG_RX_ERROR
  
  if(lp->overrunning || newrfp == lp->rfp)
    printk(KERN_INFO "%s: wv_packet_rcv(): no new frame: i593_rfp %d stop %d newrfp %d lp->rfp %d\n",
	   dev->name, i593_rfp, lp->stop, newrfp, lp->rfp);
#endif

  
  while(newrfp != lp->rfp)
    {
      

      
      rp = newrfp;	
      while(((f_start = wv_start_of_frame(dev, rp, newrfp)) != lp->rfp) &&
	    (f_start != -1))
	  rp = f_start;

      
      if(f_start == -1)
	{
#ifdef DEBUG_RX_ERROR
	  printk(KERN_INFO "wavelan_cs: cannot find start of frame ");
	  printk(" i593_rfp %d stop %d newrfp %d lp->rfp %d\n",
		 i593_rfp, lp->stop, newrfp, lp->rfp);
#endif
	  lp->rfp = rp;		
	  continue;
	}

      

      
      stat_ptr = (rp - 7 + RX_SIZE) % RX_SIZE;
      stat_ptr = read_ringbuf(dev, stat_ptr, c, 4);
      status = c[0] | (c[1] << 8);
      len = c[2] | (c[3] << 8);

      
      if((status & RX_RCV_OK) != RX_RCV_OK)
	{
	  dev->stats.rx_errors++;
	  if(status & RX_NO_SFD)
	    dev->stats.rx_frame_errors++;
	  if(status & RX_CRC_ERR)
	    dev->stats.rx_crc_errors++;
	  if(status & RX_OVRRUN)
	    dev->stats.rx_over_errors++;

#ifdef DEBUG_RX_FAIL
	  printk(KERN_DEBUG "%s: wv_packet_rcv(): packet not received ok, status = 0x%x\n",
		 dev->name, status);
#endif
	}
      else
	
	wv_packet_read(dev, f_start, len - 2);

      
      lp->rfp = rp;
    }

  
  lp->stop = (i593_rfp + RX_SIZE - ((RX_SIZE / 64) * 3)) % RX_SIZE;
  outb(OP0_SWIT_TO_PORT_1 | CR0_CHNL, LCCR(base));
  outb(CR1_STOP_REG_UPDATE | (lp->stop >> RX_SIZE_SHIFT), LCCR(base));
  outb(OP1_SWIT_TO_PORT_0, LCCR(base));

#ifdef DEBUG_RX_TRACE
  printk(KERN_DEBUG "%s: <-wv_packet_rcv()\n", dev->name);
#endif
}






static void
wv_packet_write(struct net_device *	dev,
		void *		buf,
		short		length)
{
  net_local *		lp = netdev_priv(dev);
  unsigned int		base = dev->base_addr;
  unsigned long		flags;
  int			clen = length;
  register u_short	xmtdata_base = TX_BASE;

#ifdef DEBUG_TX_TRACE
  printk(KERN_DEBUG "%s: ->wv_packet_write(%d)\n", dev->name, length);
#endif

  spin_lock_irqsave(&lp->spinlock, flags);

  
  outb(xmtdata_base & 0xff, PIORL(base));
  outb(((xmtdata_base >> 8) & PIORH_MASK) | PIORH_SEL_TX, PIORH(base));
  outb(clen & 0xff, PIOP(base));	
  outb(clen >> 8, PIOP(base));  	

  
  outsb(PIOP(base), buf, clen);

  
  outb(OP0_NOP, PIOP(base));
  
  outb(OP0_NOP, PIOP(base));

  
  hacr_write_slow(base, HACR_PWR_STAT | HACR_TX_DMA_RESET);
  hacr_write(base, HACR_DEFAULT);
  
  wv_82593_cmd(dev, "wv_packet_write(): transmit",
	       OP0_TRANSMIT, SR0_NO_RESULT);

  
  dev->trans_start = jiffies;

  
  dev->stats.tx_bytes += length;

  spin_unlock_irqrestore(&lp->spinlock, flags);

#ifdef DEBUG_TX_INFO
  wv_packet_info((u_char *) buf, length, dev->name, "wv_packet_write");
#endif	

#ifdef DEBUG_TX_TRACE
  printk(KERN_DEBUG "%s: <-wv_packet_write()\n", dev->name);
#endif
}



static netdev_tx_t
wavelan_packet_xmit(struct sk_buff *	skb,
		    struct net_device *		dev)
{
  net_local *		lp = netdev_priv(dev);
  unsigned long		flags;

#ifdef DEBUG_TX_TRACE
  printk(KERN_DEBUG "%s: ->wavelan_packet_xmit(0x%X)\n", dev->name,
	 (unsigned) skb);
#endif

  
  netif_stop_queue(dev);

  
  if(lp->reconfig_82593)
    {
      spin_lock_irqsave(&lp->spinlock, flags);	
      wv_82593_config(dev);
      spin_unlock_irqrestore(&lp->spinlock, flags);	
      
    }

	
	
	if (skb_padto(skb, ETH_ZLEN))
		return NETDEV_TX_OK;

  wv_packet_write(dev, skb->data, skb->len);

  dev_kfree_skb(skb);

#ifdef DEBUG_TX_TRACE
  printk(KERN_DEBUG "%s: <-wavelan_packet_xmit()\n", dev->name);
#endif
  return NETDEV_TX_OK;
}






static int
wv_mmc_init(struct net_device *	dev)
{
  unsigned int	base = dev->base_addr;
  psa_t		psa;
  mmw_t		m;
  int		configured;
  int		i;		

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: ->wv_mmc_init()\n", dev->name);
#endif

  
  psa_read(dev, 0, (unsigned char *) &psa, sizeof(psa));

  
  for (i = 0; i < ARRAY_SIZE(MAC_ADDRESSES); i++)
    if ((psa.psa_univ_mac_addr[0] == MAC_ADDRESSES[i][0]) &&
        (psa.psa_univ_mac_addr[1] == MAC_ADDRESSES[i][1]) &&
        (psa.psa_univ_mac_addr[2] == MAC_ADDRESSES[i][2]))
      break;

  
  if (i == ARRAY_SIZE(MAC_ADDRESSES))
    {
#ifdef DEBUG_CONFIG_ERRORS
      printk(KERN_WARNING "%s: wv_mmc_init(): Invalid MAC address: %02X:%02X:%02X:...\n",
	     dev->name, psa.psa_univ_mac_addr[0],
	     psa.psa_univ_mac_addr[1], psa.psa_univ_mac_addr[2]);
#endif
      return FALSE;
    }

  
  memcpy(&dev->dev_addr[0], &psa.psa_univ_mac_addr[0], WAVELAN_ADDR_SIZE);

#ifdef USE_PSA_CONFIG
  configured = psa.psa_conf_status & 1;
#else
  configured = 0;
#endif

  
  if(!configured)
    {
      
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
      
      psa_write(dev, (char *)psa.psa_nwid - (char *)&psa,
		(unsigned char *)psa.psa_nwid, 4);
      psa_write(dev, (char *)&psa.psa_thr_pre_set - (char *)&psa,
		(unsigned char *)&psa.psa_thr_pre_set, 1);
      psa_write(dev, (char *)&psa.psa_quality_thr - (char *)&psa,
		(unsigned char *)&psa.psa_quality_thr, 1);
      psa_write(dev, (char *)&psa.psa_conf_status - (char *)&psa,
		(unsigned char *)&psa.psa_conf_status, 1);
      
      update_psa_checksum(dev);
#endif	
    }

  
  memset(&m, 0x00, sizeof(m));

  
  m.mmw_netw_id_l = psa.psa_nwid[1];
  m.mmw_netw_id_h = psa.psa_nwid[0];
  
  if(psa.psa_nwid_select & 1)
    m.mmw_loopt_sel = 0x00;
  else
    m.mmw_loopt_sel = MMW_LOOPT_SEL_DIS_NWID;

  memcpy(&m.mmw_encr_key, &psa.psa_encryption_key, 
	 sizeof(m.mmw_encr_key));

  if(psa.psa_encryption_select)
    m.mmw_encr_enable = MMW_ENCR_ENABLE_EN | MMW_ENCR_ENABLE_MODE;
  else
    m.mmw_encr_enable = 0;

  m.mmw_thr_pre_set = psa.psa_thr_pre_set & 0x3F;
  m.mmw_quality_thr = psa.psa_quality_thr & 0x0F;

  
  m.mmw_jabber_enable = 0x01;
  m.mmw_anten_sel = MMW_ANTEN_SEL_ALG_EN;
  m.mmw_ifs = 0x20;
  m.mmw_mod_delay = 0x04;
  m.mmw_jam_time = 0x38;

  m.mmw_des_io_invert = 0;
  m.mmw_freeze = 0;
  m.mmw_decay_prm = 0;
  m.mmw_decay_updat_prm = 0;

  
  mmc_write(base, 0, (u_char *)&m, sizeof(m));

  

  
  
  if(!(mmc_in(base, mmroff(0, mmr_fee_status)) &
       (MMR_FEE_STATUS_DWLD | MMR_FEE_STATUS_BUSY)))
    {
      
      m.mmw_fee_addr = 0x0F;
      m.mmw_fee_ctrl = MMW_FEE_CTRL_READ | MMW_FEE_CTRL_DWLD;
      mmc_write(base, (char *)&m.mmw_fee_ctrl - (char *)&m,
		(unsigned char *)&m.mmw_fee_ctrl, 2);

      
      fee_wait(base, 100, 100);

#ifdef DEBUG_CONFIG_INFO
      
      mmc_read(base, (char *)&m.mmw_fee_data_l - (char *)&m,
	       (unsigned char *)&m.mmw_fee_data_l, 2);

      
      printk(KERN_DEBUG "%s: Wavelan 2.00 recognised (frequency select) : Current frequency = %ld\n",
	     dev->name,
	     ((m.mmw_fee_data_h << 4) |
	      (m.mmw_fee_data_l >> 4)) * 5 / 2 + 24000L);
#endif

      
      m.mmw_fee_addr = 0x61;
      m.mmw_fee_ctrl = MMW_FEE_CTRL_READ | MMW_FEE_CTRL_DWLD;
      mmc_write(base, (char *)&m.mmw_fee_ctrl - (char *)&m,
		(unsigned char *)&m.mmw_fee_ctrl, 2);

      
    }	

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: <-wv_mmc_init()\n", dev->name);
#endif
  return TRUE;
}



static int
wv_ru_stop(struct net_device *	dev)
{
  unsigned int	base = dev->base_addr;
  net_local *	lp = netdev_priv(dev);
  unsigned long	flags;
  int		status;
  int		spin;

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: ->wv_ru_stop()\n", dev->name);
#endif

  spin_lock_irqsave(&lp->spinlock, flags);

  
  wv_82593_cmd(dev, "wv_graceful_shutdown(): stop-rcv",
	       OP0_STOP_RCV, SR0_NO_RESULT);

  
  spin = 300;
  do
    {
      udelay(10);
      outb(OP0_NOP | CR0_STATUS_3, LCCR(base));
      status = inb(LCSR(base));
    }
  while(((status & SR3_RCV_STATE_MASK) != SR3_RCV_IDLE) && (spin-- > 0));

  
  do
    {
      udelay(10);
      outb(OP0_NOP | CR0_STATUS_3, LCCR(base));
      status = inb(LCSR(base));
    }
  while(((status & SR3_EXEC_STATE_MASK) != SR3_EXEC_IDLE) && (spin-- > 0));

  spin_unlock_irqrestore(&lp->spinlock, flags);

  
  if(spin <= 0)
    {
#ifdef DEBUG_CONFIG_ERRORS
      printk(KERN_INFO "%s: wv_ru_stop(): The chip doesn't want to stop...\n",
	     dev->name);
#endif
      return FALSE;
    }

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: <-wv_ru_stop()\n", dev->name);
#endif
  return TRUE;
} 



static int
wv_ru_start(struct net_device *	dev)
{
  unsigned int	base = dev->base_addr;
  net_local *	lp = netdev_priv(dev);
  unsigned long	flags;

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: ->wv_ru_start()\n", dev->name);
#endif

  
  if(!wv_ru_stop(dev))
    return FALSE;

  spin_lock_irqsave(&lp->spinlock, flags);

  

  
  lp->rfp = 0;
  outb(OP0_SWIT_TO_PORT_1 | CR0_CHNL, LCCR(base));

  
  outb(OP1_RESET_RING_MNGMT, LCCR(base));

#if 0
  
  
#elif 0
  
  lp->stop = 0;
#else
  
  lp->stop = (0 + RX_SIZE - ((RX_SIZE / 64) * 3)) % RX_SIZE;
#endif
  outb(CR1_STOP_REG_UPDATE | (lp->stop >> RX_SIZE_SHIFT), LCCR(base));
  outb(OP1_INT_ENABLE, LCCR(base));
  outb(OP1_SWIT_TO_PORT_0, LCCR(base));

  
  hacr_write_slow(base, HACR_PWR_STAT | HACR_TX_DMA_RESET);
  hacr_write_slow(base, HACR_DEFAULT);

  
  wv_82593_cmd(dev, "wv_ru_start(): rcv-enable",
	       CR0_CHNL | OP0_RCV_ENABLE, SR0_NO_RESULT);

#ifdef DEBUG_I82593_SHOW
  {
    int	status;
    int	opri;
    int	spin = 10000;

    
    do
      {
	outb(OP0_NOP | CR0_STATUS_3, LCCR(base));
	status = inb(LCSR(base));
	if(spin-- <= 0)
	  break;
      }
    while(((status & SR3_RCV_STATE_MASK) != SR3_RCV_ACTIVE) &&
	  ((status & SR3_RCV_STATE_MASK) != SR3_RCV_READY));
    printk(KERN_DEBUG "rcv status is 0x%x [i:%d]\n",
	   (status & SR3_RCV_STATE_MASK), i);
  }
#endif

  spin_unlock_irqrestore(&lp->spinlock, flags);

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: <-wv_ru_start()\n", dev->name);
#endif
  return TRUE;
}



static int
wv_82593_config(struct net_device *	dev)
{
  unsigned int			base = dev->base_addr;
  net_local *			lp = netdev_priv(dev);
  struct i82593_conf_block	cfblk;
  int				ret = TRUE;

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: ->wv_82593_config()\n", dev->name);
#endif

  
  memset(&cfblk, 0x00, sizeof(struct i82593_conf_block));
  cfblk.d6mod = FALSE;  	
  cfblk.fifo_limit = 5;         
  cfblk.forgnesi = FALSE;       
  cfblk.fifo_32 = 1;
  cfblk.throttle_enb = FALSE;
  cfblk.contin = TRUE;          
  cfblk.cntrxint = FALSE;       
  cfblk.addr_len = WAVELAN_ADDR_SIZE;
  cfblk.acloc = TRUE;           
  cfblk.preamb_len = 0;         
  cfblk.loopback = FALSE;
  cfblk.lin_prio = 0;   	
  cfblk.exp_prio = 5;	        
  cfblk.bof_met = 1;	        
  cfblk.ifrm_spc = 0x20 >> 4;	
  cfblk.slottim_low = 0x20 >> 5;	
  cfblk.slottim_hi = 0x0;
  cfblk.max_retr = 15;
  cfblk.prmisc = ((lp->promiscuous) ? TRUE: FALSE);	
  cfblk.bc_dis = FALSE;         
  cfblk.crs_1 = TRUE;		
  cfblk.nocrc_ins = FALSE;		
  cfblk.crc_1632 = FALSE;	
  cfblk.crs_cdt = FALSE;	
  cfblk.cs_filter = 0;  	
  cfblk.crs_src = FALSE;	
  cfblk.cd_filter = 0;  	
  cfblk.min_fr_len = ETH_ZLEN >> 2;     
  cfblk.lng_typ = FALSE;	
  cfblk.lng_fld = TRUE; 	
  cfblk.rxcrc_xf = TRUE;	
  cfblk.artx = TRUE;		
  cfblk.sarec = TRUE;		
  cfblk.tx_jabber = TRUE;	
  cfblk.hash_1 = FALSE; 	
  cfblk.lbpkpol = TRUE; 	
  cfblk.fdx = FALSE;		
  cfblk.dummy_6 = 0x3f; 	
  cfblk.mult_ia = FALSE;	
  cfblk.dis_bof = FALSE;	
  cfblk.dummy_1 = TRUE; 	
  cfblk.tx_ifs_retrig = 3;	
#ifdef MULTICAST_ALL
  cfblk.mc_all = (lp->allmulticast ? TRUE: FALSE);	
#else
  cfblk.mc_all = FALSE;		
#endif
  cfblk.rcv_mon = 0;		
  cfblk.frag_acpt = TRUE;	
  cfblk.tstrttrs = FALSE;	
  cfblk.fretx = TRUE;		
  cfblk.syncrqs = FALSE; 	
  cfblk.sttlen = TRUE;  	
  cfblk.rx_eop = TRUE;  	
  cfblk.tx_eop = TRUE;  	
  cfblk.rbuf_size = RX_SIZE>>11;	
  cfblk.rcvstop = TRUE; 	

#ifdef DEBUG_I82593_SHOW
  print_hex_dump(KERN_DEBUG, "wavelan_cs: config block: ", DUMP_PREFIX_NONE,
		 16, 1, &cfblk, sizeof(struct i82593_conf_block), false);
#endif

  
  outb(TX_BASE & 0xff, PIORL(base));
  outb(((TX_BASE >> 8) & PIORH_MASK) | PIORH_SEL_TX, PIORH(base));
  outb(sizeof(struct i82593_conf_block) & 0xff, PIOP(base));    
  outb(sizeof(struct i82593_conf_block) >> 8, PIOP(base));	
  outsb(PIOP(base), (char *) &cfblk, sizeof(struct i82593_conf_block));

  
  hacr_write_slow(base, HACR_PWR_STAT | HACR_TX_DMA_RESET);
  hacr_write(base, HACR_DEFAULT);
  if(!wv_82593_cmd(dev, "wv_82593_config(): configure",
		   OP0_CONFIGURE, SR0_CONFIGURE_DONE))
    ret = FALSE;

  
  outb(TX_BASE & 0xff, PIORL(base));
  outb(((TX_BASE >> 8) & PIORH_MASK) | PIORH_SEL_TX, PIORH(base));
  outb(WAVELAN_ADDR_SIZE, PIOP(base));	
  outb(0, PIOP(base));			
  outsb(PIOP(base), &dev->dev_addr[0], WAVELAN_ADDR_SIZE);

  
  hacr_write_slow(base, HACR_PWR_STAT | HACR_TX_DMA_RESET);
  hacr_write(base, HACR_DEFAULT);
  if(!wv_82593_cmd(dev, "wv_82593_config(): ia-setup",
		   OP0_IA_SETUP, SR0_IA_SETUP_DONE))
    ret = FALSE;

#ifdef WAVELAN_ROAMING
    
    
  if(do_roaming)
    dev_mc_add(dev,WAVELAN_BEACON_ADDRESS, WAVELAN_ADDR_SIZE, 1);
#endif	

  
  if(lp->mc_count)
    {
      struct dev_mc_list *	dmi;
      int			addrs_len = WAVELAN_ADDR_SIZE * lp->mc_count;

#ifdef DEBUG_CONFIG_INFO
      printk(KERN_DEBUG "%s: wv_hw_config(): set %d multicast addresses:\n",
	     dev->name, lp->mc_count);
      for(dmi=dev->mc_list; dmi; dmi=dmi->next)
	printk(KERN_DEBUG " %pM\n", dmi->dmi_addr);
#endif

      
      outb(TX_BASE & 0xff, PIORL(base));
      outb(((TX_BASE >> 8) & PIORH_MASK) | PIORH_SEL_TX, PIORH(base));
      outb(addrs_len & 0xff, PIOP(base));	
      outb((addrs_len >> 8), PIOP(base));	
      for(dmi=dev->mc_list; dmi; dmi=dmi->next)
	outsb(PIOP(base), dmi->dmi_addr, dmi->dmi_addrlen);

      
      hacr_write_slow(base, HACR_PWR_STAT | HACR_TX_DMA_RESET);
      hacr_write(base, HACR_DEFAULT);
      if(!wv_82593_cmd(dev, "wv_82593_config(): mc-setup",
		       OP0_MC_SETUP, SR0_MC_SETUP_DONE))
	ret = FALSE;
      lp->mc_count = dev->mc_count;	
    }

  
  lp->reconfig_82593 = FALSE;

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: <-wv_82593_config()\n", dev->name);
#endif
  return(ret);
}



static int
wv_pcmcia_reset(struct net_device *	dev)
{
  int		i;
  conf_reg_t	reg = { 0, CS_READ, CISREG_COR, 0 };
  struct pcmcia_device *	link = ((net_local *)netdev_priv(dev))->link;

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: ->wv_pcmcia_reset()\n", dev->name);
#endif

  i = pcmcia_access_configuration_register(link, &reg);
  if (i != 0)
    {
      cs_error(link, AccessConfigurationRegister, i);
      return FALSE;
    }
      
#ifdef DEBUG_CONFIG_INFO
  printk(KERN_DEBUG "%s: wavelan_pcmcia_reset(): Config reg is 0x%x\n",
	 dev->name, (u_int) reg.Value);
#endif

  reg.Action = CS_WRITE;
  reg.Value = reg.Value | COR_SW_RESET;
  i = pcmcia_access_configuration_register(link, &reg);
  if (i != 0)
    {
      cs_error(link, AccessConfigurationRegister, i);
      return FALSE;
    }
      
  reg.Action = CS_WRITE;
  reg.Value = COR_LEVEL_IRQ | COR_CONFIG;
  i = pcmcia_access_configuration_register(link, &reg);
  if (i != 0)
    {
      cs_error(link, AccessConfigurationRegister, i);
      return FALSE;
    }

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: <-wv_pcmcia_reset()\n", dev->name);
#endif
  return TRUE;
}



static int
wv_hw_config(struct net_device *	dev)
{
  net_local *		lp = netdev_priv(dev);
  unsigned int		base = dev->base_addr;
  unsigned long		flags;
  int			ret = FALSE;

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: ->wv_hw_config()\n", dev->name);
#endif

  
  BUILD_BUG_ON(sizeof(psa_t) != PSA_SIZE);
  BUILD_BUG_ON(sizeof(mmw_t) != MMW_SIZE);
  BUILD_BUG_ON(sizeof(mmr_t) != MMR_SIZE);

  
  if(wv_pcmcia_reset(dev) == FALSE)
    return FALSE;

  
  spin_lock_irqsave(&lp->spinlock, flags);

  
  do
    {
      
      hacr_write_slow(base, HACR_RESET);
      hacr_write(base, HACR_DEFAULT);

      
      if(hasr_read(base) & HASR_NO_CLK)
	{
#ifdef DEBUG_CONFIG_ERRORS
	  printk(KERN_WARNING "%s: wv_hw_config(): modem not connected or not a wavelan card\n",
		 dev->name);
#endif
	  break;
	}

      
      if(wv_mmc_init(dev) == FALSE)
	{
#ifdef DEBUG_CONFIG_ERRORS
	  printk(KERN_WARNING "%s: wv_hw_config(): Can't configure the modem\n",
		 dev->name);
#endif
	  break;
	}

      
      outb(OP0_RESET, LCCR(base));
      mdelay(1);	

      
      if(wv_82593_config(dev) == FALSE)
	{
#ifdef DEBUG_CONFIG_ERRORS
	  printk(KERN_INFO "%s: wv_hw_config(): i82593 init failed\n",
		 dev->name);
#endif
	  break;
	}

      
      if(wv_diag(dev) == FALSE)
	{
#ifdef DEBUG_CONFIG_ERRORS
	  printk(KERN_INFO "%s: wv_hw_config(): i82593 diagnostic failed\n",
		 dev->name);
#endif
	  break;
	}

      

      
      lp->configured = 1;
      ret = TRUE;
    }
  while(0);

  
  spin_unlock_irqrestore(&lp->spinlock, flags);

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: <-wv_hw_config()\n", dev->name);
#endif
  return(ret);
}



static void
wv_hw_reset(struct net_device *	dev)
{
  net_local *	lp = netdev_priv(dev);

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: ->wv_hw_reset()\n", dev->name);
#endif

  lp->nresets++;
  lp->configured = 0;
  
  
  if(wv_hw_config(dev) == FALSE)
    return;

  
  wv_ru_start(dev);

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "%s: <-wv_hw_reset()\n", dev->name);
#endif
}



static int
wv_pcmcia_config(struct pcmcia_device *	link)
{
  struct net_device *	dev = (struct net_device *) link->priv;
  int			i;
  win_req_t		req;
  memreq_t		mem;
  net_local *		lp = netdev_priv(dev);


#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "->wv_pcmcia_config(0x%p)\n", link);
#endif

  do
    {
      i = pcmcia_request_io(link, &link->io);
      if (i != 0)
	{
	  cs_error(link, RequestIO, i);
	  break;
	}

      
      i = pcmcia_request_irq(link, &link->irq);
      if (i != 0)
	{
	  cs_error(link, RequestIRQ, i);
	  break;
	}

      
      link->conf.ConfigIndex = 1;
      i = pcmcia_request_configuration(link, &link->conf);
      if (i != 0)
	{
	  cs_error(link, RequestConfiguration, i);
	  break;
	}

      
      req.Attributes = WIN_DATA_WIDTH_8|WIN_MEMORY_TYPE_AM|WIN_ENABLE;
      req.Base = req.Size = 0;
      req.AccessSpeed = mem_speed;
      i = pcmcia_request_window(&link, &req, &link->win);
      if (i != 0)
	{
	  cs_error(link, RequestWindow, i);
	  break;
	}

      lp->mem = ioremap(req.Base, req.Size);
      dev->mem_start = (u_long)lp->mem;
      dev->mem_end = dev->mem_start + req.Size;

      mem.CardOffset = 0; mem.Page = 0;
      i = pcmcia_map_mem_page(link->win, &mem);
      if (i != 0)
	{
	  cs_error(link, MapMemPage, i);
	  break;
	}

      
      dev->irq = link->irq.AssignedIRQ;
      dev->base_addr = link->io.BasePort1;
      netif_start_queue(dev);

#ifdef DEBUG_CONFIG_INFO
      printk(KERN_DEBUG "wv_pcmcia_config: MEMSTART %p IRQ %d IOPORT 0x%x\n",
	     lp->mem, dev->irq, (u_int) dev->base_addr);
#endif

      SET_NETDEV_DEV(dev, &handle_to_dev(link));
      i = register_netdev(dev);
      if(i != 0)
	{
#ifdef DEBUG_CONFIG_ERRORS
	  printk(KERN_INFO "wv_pcmcia_config(): register_netdev() failed\n");
#endif
	  break;
	}
    }
  while(0);		

  
  if(i != 0)
    {
      wv_pcmcia_release(link);
      return FALSE;
    }

  strcpy(((net_local *) netdev_priv(dev))->node.dev_name, dev->name);
  link->dev_node = &((net_local *) netdev_priv(dev))->node;

#ifdef DEBUG_CONFIG_TRACE
  printk(KERN_DEBUG "<-wv_pcmcia_config()\n");
#endif
  return TRUE;
}



static void
wv_pcmcia_release(struct pcmcia_device *link)
{
	struct net_device *	dev = (struct net_device *) link->priv;
	net_local *		lp = netdev_priv(dev);

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: -> wv_pcmcia_release(0x%p)\n", dev->name, link);
#endif

	iounmap(lp->mem);
	pcmcia_disable_device(link);

#ifdef DEBUG_CONFIG_TRACE
	printk(KERN_DEBUG "%s: <- wv_pcmcia_release()\n", dev->name);
#endif
}




static irqreturn_t
wavelan_interrupt(int		irq,
		  void *	dev_id)
{
  struct net_device *	dev = dev_id;
  net_local *	lp;
  unsigned int	base;
  int		status0;
  u_int		tx_status;

#ifdef DEBUG_INTERRUPT_TRACE
  printk(KERN_DEBUG "%s: ->wavelan_interrupt()\n", dev->name);
#endif

  lp = netdev_priv(dev);
  base = dev->base_addr;

#ifdef DEBUG_INTERRUPT_INFO
  
  if(spin_is_locked(&lp->spinlock))
    printk(KERN_DEBUG
	   "%s: wavelan_interrupt(): spinlock is already locked !!!\n",
	   dev->name);
#endif

  
  spin_lock(&lp->spinlock);

  
  while(1)
    {
      
      
      outb(CR0_STATUS_0 | OP0_NOP, LCCR(base));
      status0 = inb(LCSR(base));

#ifdef DEBUG_INTERRUPT_INFO
      printk(KERN_DEBUG "status0 0x%x [%s => 0x%x]", status0, 
	     (status0&SR0_INTERRUPT)?"int":"no int",status0&~SR0_INTERRUPT);
      if(status0&SR0_INTERRUPT)
	{
	  printk(" [%s => %d]\n", (status0 & SR0_CHNL) ? "chnl" :
		 ((status0 & SR0_EXECUTION) ? "cmd" :
		  ((status0 & SR0_RECEPTION) ? "recv" : "unknown")),
		 (status0 & SR0_EVENT_MASK));
	}
      else
	printk("\n");
#endif

      
      if(!(status0 & SR0_INTERRUPT))
	break;

      
      if(((status0 & SR0_BOTH_RX_TX) == SR0_BOTH_RX_TX) ||
	 ((status0 & SR0_BOTH_RX_TX) == 0x0))
	{
#ifdef DEBUG_INTERRUPT_INFO
	  printk(KERN_INFO "%s: wv_interrupt(): bogus interrupt (or from dead card) : %X\n",
		 dev->name, status0);
#endif
	  
	  outb(CR0_INT_ACK | OP0_NOP, LCCR(base));
	  break;
	}

      
      
      if(status0 & SR0_RECEPTION)
	{
#ifdef DEBUG_INTERRUPT_INFO
	  printk(KERN_DEBUG "%s: wv_interrupt(): receive\n", dev->name);
#endif

	  if((status0 & SR0_EVENT_MASK) == SR0_STOP_REG_HIT)
	    {
#ifdef DEBUG_INTERRUPT_ERROR
	      printk(KERN_INFO "%s: wv_interrupt(): receive buffer overflow\n",
		     dev->name);
#endif
	      dev->stats.rx_over_errors++;
	      lp->overrunning = 1;
      	    }

	  
	  wv_packet_rcv(dev);
	  lp->overrunning = 0;

	  
	  outb(CR0_INT_ACK | OP0_NOP, LCCR(base));
	  continue;
    	}

      
      

      
      if((status0 & SR0_EVENT_MASK) == SR0_TRANSMIT_DONE ||
	 (status0 & SR0_EVENT_MASK) == SR0_RETRANSMIT_DONE ||
	 (status0 & SR0_EVENT_MASK) == SR0_TRANSMIT_NO_CRC_DONE)
	{
#ifdef DEBUG_TX_ERROR
	  if((status0 & SR0_EVENT_MASK) == SR0_TRANSMIT_NO_CRC_DONE)
	    printk(KERN_INFO "%s: wv_interrupt(): packet transmitted without CRC.\n",
		   dev->name);
#endif

	  
	  tx_status = inb(LCSR(base));
	  tx_status |= (inb(LCSR(base)) << 8);
#ifdef DEBUG_INTERRUPT_INFO
	  printk(KERN_DEBUG "%s: wv_interrupt(): transmission done\n",
		 dev->name);
	  {
	    u_int	rcv_bytes;
	    u_char	status3;
	    rcv_bytes = inb(LCSR(base));
	    rcv_bytes |= (inb(LCSR(base)) << 8);
	    status3 = inb(LCSR(base));
	    printk(KERN_DEBUG "tx_status 0x%02x rcv_bytes 0x%02x status3 0x%x\n",
		   tx_status, rcv_bytes, (u_int) status3);
	  }
#endif
	  
	  if((tx_status & TX_OK) != TX_OK)
	    {
	      dev->stats.tx_errors++;

	      if(tx_status & TX_FRTL)
		{
#ifdef DEBUG_TX_ERROR
		  printk(KERN_INFO "%s: wv_interrupt(): frame too long\n",
			 dev->name);
#endif
		}
	      if(tx_status & TX_UND_RUN)
		{
#ifdef DEBUG_TX_FAIL
		  printk(KERN_DEBUG "%s: wv_interrupt(): DMA underrun\n",
			 dev->name);
#endif
		  dev->stats.tx_aborted_errors++;
		}
	      if(tx_status & TX_LOST_CTS)
		{
#ifdef DEBUG_TX_FAIL
		  printk(KERN_DEBUG "%s: wv_interrupt(): no CTS\n", dev->name);
#endif
		  dev->stats.tx_carrier_errors++;
		}
	      if(tx_status & TX_LOST_CRS)
		{
#ifdef DEBUG_TX_FAIL
		  printk(KERN_DEBUG "%s: wv_interrupt(): no carrier\n",
			 dev->name);
#endif
		  dev->stats.tx_carrier_errors++;
		}
	      if(tx_status & TX_HRT_BEAT)
		{
#ifdef DEBUG_TX_FAIL
		  printk(KERN_DEBUG "%s: wv_interrupt(): heart beat\n", dev->name);
#endif
		  dev->stats.tx_heartbeat_errors++;
		}
	      if(tx_status & TX_DEFER)
		{
#ifdef DEBUG_TX_FAIL
		  printk(KERN_DEBUG "%s: wv_interrupt(): channel jammed\n",
			 dev->name);
#endif
		}
	      
	      if(tx_status & TX_COLL)
		{
		  if(tx_status & TX_MAX_COL)
		    {
#ifdef DEBUG_TX_FAIL
		      printk(KERN_DEBUG "%s: wv_interrupt(): channel congestion\n",
			     dev->name);
#endif
		      if(!(tx_status & TX_NCOL_MASK))
			{
			  dev->stats.collisions += 0x10;
			}
		    }
		}
	    }	

	  dev->stats.collisions += (tx_status & TX_NCOL_MASK);
	  dev->stats.tx_packets++;

	  netif_wake_queue(dev);
	  outb(CR0_INT_ACK | OP0_NOP, LCCR(base));	
    	} 
      else	
	{
#ifdef DEBUG_INTERRUPT_ERROR
	  printk(KERN_INFO "wavelan_cs: unknown interrupt, status0 = %02x\n",
		 status0);
#endif
	  outb(CR0_INT_ACK | OP0_NOP, LCCR(base));	
    	}
    }	

  spin_unlock(&lp->spinlock);

#ifdef DEBUG_INTERRUPT_TRACE
  printk(KERN_DEBUG "%s: <-wavelan_interrupt()\n", dev->name);
#endif

  
  return IRQ_HANDLED;
} 



static void
wavelan_watchdog(struct net_device *	dev)
{
  net_local *		lp = netdev_priv(dev);
  unsigned int		base = dev->base_addr;
  unsigned long		flags;
  int			aborted = FALSE;

#ifdef DEBUG_INTERRUPT_TRACE
  printk(KERN_DEBUG "%s: ->wavelan_watchdog()\n", dev->name);
#endif

#ifdef DEBUG_INTERRUPT_ERROR
  printk(KERN_INFO "%s: wavelan_watchdog: watchdog timer expired\n",
	 dev->name);
#endif

  spin_lock_irqsave(&lp->spinlock, flags);

  
  outb(OP0_ABORT, LCCR(base));

  
  if(wv_82593_cmd(dev, "wavelan_watchdog(): abort",
		  OP0_NOP | CR0_STATUS_3, SR0_EXECUTION_ABORTED))
    aborted = TRUE;

  
  spin_unlock_irqrestore(&lp->spinlock, flags);

  
  if(!aborted)
    {
      
#ifdef DEBUG_INTERRUPT_ERROR
      printk(KERN_INFO "%s: wavelan_watchdog: abort failed, trying reset\n",
	     dev->name);
#endif
      wv_hw_reset(dev);
    }

#ifdef DEBUG_PSA_SHOW
  {
    psa_t		psa;
    psa_read(dev, 0, (unsigned char *) &psa, sizeof(psa));
    wv_psa_show(&psa);
  }
#endif
#ifdef DEBUG_MMC_SHOW
  wv_mmc_show(dev);
#endif
#ifdef DEBUG_I82593_SHOW
  wv_ru_show(dev);
#endif

  
  netif_wake_queue(dev);

#ifdef DEBUG_INTERRUPT_TRACE
  printk(KERN_DEBUG "%s: <-wavelan_watchdog()\n", dev->name);
#endif
}






static int
wavelan_open(struct net_device *	dev)
{
  net_local *	lp = netdev_priv(dev);
  struct pcmcia_device *	link = lp->link;
  unsigned int	base = dev->base_addr;

#ifdef DEBUG_CALLBACK_TRACE
  printk(KERN_DEBUG "%s: ->wavelan_open(dev=0x%x)\n", dev->name,
	 (unsigned int) dev);
#endif

  
  if(hasr_read(base) & HASR_NO_CLK)
    {
      
      hacr_write(base, HACR_DEFAULT);

      
      if(hasr_read(base) & HASR_NO_CLK)
	{
#ifdef DEBUG_CONFIG_ERRORS
	  printk(KERN_WARNING "%s: wavelan_open(): modem not connected\n",
		 dev->name);
#endif
	  return FALSE;
	}
    }

  
  if(!lp->configured)
    return FALSE;
  if(!wv_ru_start(dev))
    wv_hw_reset(dev);		
  netif_start_queue(dev);

  
  link->open++;

#ifdef WAVELAN_ROAMING
  if(do_roaming)
    wv_roam_init(dev);
#endif	

#ifdef DEBUG_CALLBACK_TRACE
  printk(KERN_DEBUG "%s: <-wavelan_open()\n", dev->name);
#endif
  return 0;
}



static int
wavelan_close(struct net_device *	dev)
{
  struct pcmcia_device *	link = ((net_local *)netdev_priv(dev))->link;
  unsigned int	base = dev->base_addr;

#ifdef DEBUG_CALLBACK_TRACE
  printk(KERN_DEBUG "%s: ->wavelan_close(dev=0x%x)\n", dev->name,
	 (unsigned int) dev);
#endif

  
  if(!link->open)
    {
#ifdef DEBUG_CONFIG_INFO
      printk(KERN_DEBUG "%s: wavelan_close(): device not open\n", dev->name);
#endif
      return 0;
    }

#ifdef WAVELAN_ROAMING
  
  if(do_roaming)
    wv_roam_cleanup(dev);
#endif	

  link->open--;

  
  if(netif_running(dev))
    {
      netif_stop_queue(dev);

      
      wv_ru_stop(dev);

      
      hacr_write(base, HACR_DEFAULT & (~HACR_PWR_STAT));
    }

#ifdef DEBUG_CALLBACK_TRACE
  printk(KERN_DEBUG "%s: <-wavelan_close()\n", dev->name);
#endif
  return 0;
}

static const struct net_device_ops wavelan_netdev_ops = {
	.ndo_open 		= wavelan_open,
	.ndo_stop		= wavelan_close,
	.ndo_start_xmit		= wavelan_packet_xmit,
	.ndo_set_multicast_list = wavelan_set_multicast_list,
#ifdef SET_MAC_ADDRESS
	.ndo_set_mac_address	= wavelan_set_mac_address,
#endif
	.ndo_tx_timeout		= wavelan_watchdog,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
};



static int
wavelan_probe(struct pcmcia_device *p_dev)
{
  struct net_device *	dev;		
  net_local *	lp;		
  int ret;

#ifdef DEBUG_CALLBACK_TRACE
  printk(KERN_DEBUG "-> wavelan_attach()\n");
#endif

  
  p_dev->io.NumPorts1 = 8;
  p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
  p_dev->io.IOAddrLines = 3;

  
  p_dev->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING | IRQ_HANDLE_PRESENT;
  p_dev->irq.IRQInfo1 = IRQ_LEVEL_ID;
  p_dev->irq.Handler = wavelan_interrupt;

  
  p_dev->conf.Attributes = CONF_ENABLE_IRQ;
  p_dev->conf.IntType = INT_MEMORY_AND_IO;

  
  dev = alloc_etherdev(sizeof(net_local));
  if (!dev)
      return -ENOMEM;

  p_dev->priv = p_dev->irq.Instance = dev;

  lp = netdev_priv(dev);

  
  lp->configured = 0;
  lp->reconfig_82593 = FALSE;
  lp->nresets = 0;
  
  lp->promiscuous = 0;
  lp->allmulticast = 0;
  lp->mc_count = 0;

  
  spin_lock_init(&lp->spinlock);

  
  lp->dev = dev;

  
  dev->netdev_ops = &wavelan_netdev_ops;
  dev->watchdog_timeo	= WATCHDOG_JIFFIES;
  SET_ETHTOOL_OPS(dev, &ops);

  dev->wireless_handlers = &wavelan_handler_def;
  lp->wireless_data.spy_data = &lp->spy_data;
  dev->wireless_data = &lp->wireless_data;

  
  dev->mtu = WAVELAN_MTU;

  ret = wv_pcmcia_config(p_dev);
  if (ret)
	  return ret;

  ret = wv_hw_config(dev);
  if (ret) {
	  dev->irq = 0;
	  pcmcia_disable_device(p_dev);
	  return ret;
  }

  wv_init_info(dev);

#ifdef DEBUG_CALLBACK_TRACE
  printk(KERN_DEBUG "<- wavelan_attach()\n");
#endif

  return 0;
}



static void
wavelan_detach(struct pcmcia_device *link)
{
#ifdef DEBUG_CALLBACK_TRACE
  printk(KERN_DEBUG "-> wavelan_detach(0x%p)\n", link);
#endif

  
  wv_pcmcia_release(link);

  
  if(link->priv)
    {
      struct net_device *	dev = (struct net_device *) link->priv;

      
      
      if (link->dev_node)
	unregister_netdev(dev);
      link->dev_node = NULL;
      ((net_local *)netdev_priv(dev))->link = NULL;
      ((net_local *)netdev_priv(dev))->dev = NULL;
      free_netdev(dev);
    }

#ifdef DEBUG_CALLBACK_TRACE
  printk(KERN_DEBUG "<- wavelan_detach()\n");
#endif
}

static int wavelan_suspend(struct pcmcia_device *link)
{
	struct net_device *	dev = (struct net_device *) link->priv;

	

	
	wv_ru_stop(dev);

	if (link->open)
		netif_device_detach(dev);

	
	hacr_write(dev->base_addr, HACR_DEFAULT & (~HACR_PWR_STAT));

	return 0;
}

static int wavelan_resume(struct pcmcia_device *link)
{
	struct net_device *	dev = (struct net_device *) link->priv;

	if (link->open) {
		wv_hw_reset(dev);
		netif_device_attach(dev);
	}

	return 0;
}


static struct pcmcia_device_id wavelan_ids[] = {
	PCMCIA_DEVICE_PROD_ID12("AT&T","WaveLAN/PCMCIA", 0xe7c5affd, 0x1bc50975),
	PCMCIA_DEVICE_PROD_ID12("Digital", "RoamAbout/DS", 0x9999ab35, 0x00d05e06),
	PCMCIA_DEVICE_PROD_ID12("Lucent Technologies", "WaveLAN/PCMCIA", 0x23eb9949, 0x1bc50975),
	PCMCIA_DEVICE_PROD_ID12("NCR", "WaveLAN/PCMCIA", 0x24358cd4, 0x1bc50975),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, wavelan_ids);

static struct pcmcia_driver wavelan_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "wavelan_cs",
	},
	.probe		= wavelan_probe,
	.remove		= wavelan_detach,
	.id_table       = wavelan_ids,
	.suspend	= wavelan_suspend,
	.resume		= wavelan_resume,
};

static int __init
init_wavelan_cs(void)
{
	return pcmcia_register_driver(&wavelan_driver);
}

static void __exit
exit_wavelan_cs(void)
{
	pcmcia_unregister_driver(&wavelan_driver);
}

module_init(init_wavelan_cs);
module_exit(exit_wavelan_cs);
