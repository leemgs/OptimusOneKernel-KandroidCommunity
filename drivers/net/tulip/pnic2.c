






#include "tulip.h"
#include <linux/delay.h>


void pnic2_timer(unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	struct tulip_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->base_addr;
	int next_tick = 60*HZ;

	if (tulip_debug > 3)
		printk(KERN_INFO"%s: PNIC2 negotiation status %8.8x.\n",
                    dev->name,ioread32(ioaddr + CSR12));

	if (next_tick) {
		mod_timer(&tp->timer, RUN_AT(next_tick));
	}
}


void pnic2_start_nway(struct net_device *dev)
{
	struct tulip_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->base_addr;
        int csr14;
        int csr12;

        

        
	csr14 = (ioread32(ioaddr + CSR14) & 0xfff0ee39);

        
        if (tp->sym_advertise & 0x0100) csr14 |= 0x00020000;

        
        if (tp->sym_advertise & 0x0080) csr14 |= 0x00010000;

        
        if (tp->sym_advertise & 0x0020) csr14 |= 0x00000040;

        
        csr14 |= 0x00001184;

	if (tulip_debug > 1)
		printk(KERN_DEBUG "%s: Restarting PNIC2 autonegotiation, "
                      "csr14=%8.8x.\n", dev->name, csr14);

        
	dev->if_port = 0;
	tp->nway = tp->mediasense = 1;
	tp->nwayset = tp->lpar = 0;

        

	tp->csr6 = ioread32(ioaddr + CSR6);
	if (tulip_debug > 1)
		printk(KERN_DEBUG "%s: On Entry to Nway, "
                      "csr6=%8.8x.\n", dev->name, tp->csr6);

        
	tp->csr6 = tp->csr6 & 0xfe3bd1fd;

        
        
        if (tp->sym_advertise & 0x0040) tp->csr6 |= 0x00000200;

        
        tp->csr6 |= 0x01000000;
	iowrite32(csr14, ioaddr + CSR14);
	iowrite32(tp->csr6, ioaddr + CSR6);
        udelay(100);

        

        
        csr12 = (ioread32(ioaddr + CSR12) & 0xffff8fff);
        csr12 |= 0x1000;
	iowrite32(csr12, ioaddr + CSR12);
}



void pnic2_lnk_change(struct net_device *dev, int csr5)
{
	struct tulip_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->base_addr;
        int csr14;

        
	int csr12 = ioread32(ioaddr + CSR12);

	if (tulip_debug > 1)
		printk(KERN_INFO"%s: PNIC2 link status interrupt %8.8x, "
                       " CSR5 %x, %8.8x.\n", dev->name, csr12,
                       csr5, ioread32(ioaddr + CSR14));

	
	if (tp->nway  &&  !tp->nwayset) {

	        

                if ((csr12 & 0x7000) == 0x5000) {

	               

	               

		        int negotiated = ((csr12 >> 16) & 0x01E0) & tp->sym_advertise;
		        tp->lpar = (csr12 >> 16);
		        tp->nwayset = 1;

                        if (negotiated & 0x0100)        dev->if_port = 5;
		        else if (negotiated & 0x0080)	dev->if_port = 3;
		        else if (negotiated & 0x0040)	dev->if_port = 4;
			else if (negotiated & 0x0020)	dev->if_port = 0;
			else {
			     if (tulip_debug > 1)
		                   printk(KERN_INFO "%s: funny autonegotiate result "
                                        "csr12 %8.8x advertising %4.4x\n",
			                 dev->name, csr12, tp->sym_advertise);
			     tp->nwayset = 0;
			     
			     if ((csr12 & 2) == 0  &&  (tp->sym_advertise & 0x0180))
			       dev->if_port = 3;
			}

			
			tp->full_duplex = 0;
			if ((dev->if_port == 4) || (dev->if_port == 5))
			       tp->full_duplex = 1;

			if (tulip_debug > 1) {
			       if (tp->nwayset)
			             printk(KERN_INFO "%s: Switching to %s based on link "
				    "negotiation %4.4x & %4.4x = %4.4x.\n",
				     dev->name, medianame[dev->if_port],
                                     tp->sym_advertise, tp->lpar, negotiated);
			}

                        
	                csr14 = (ioread32(ioaddr + CSR14) & 0xffffff7f);
                        iowrite32(csr14,ioaddr + CSR14);


                        

			
			

			tp->csr6 = (ioread32(ioaddr + CSR6) & 0xfe3bd1fd);

			
			if (dev->if_port & 1) tp->csr6 |= 0x01840000;
			else tp->csr6 |= 0x00400000;

			
			if (tp->full_duplex) tp->csr6 |= 0x00000200;

			iowrite32(1, ioaddr + CSR13);

			if (tulip_debug > 2)
			        printk(KERN_DEBUG "%s:  Setting CSR6 %8.8x/%x CSR12 "
                                      "%8.8x.\n", dev->name, tp->csr6,
                                      ioread32(ioaddr + CSR6), ioread32(ioaddr + CSR12));

			
			tulip_start_rxtx(tp);

                        return;

	        } else {
	                printk(KERN_INFO "%s: Autonegotiation failed, "
                                    "using %s, link beat status %4.4x.\n",
				     dev->name, medianame[dev->if_port], csr12);

                        
	                csr14 = (ioread32(ioaddr + CSR14) & 0xffffff7f);
                        iowrite32(csr14,ioaddr + CSR14);

                        

	                 dev->if_port = 0;
                         tp->nway = 0;
                         tp->nwayset = 1;

                         
	                 tp->csr6 = (ioread32(ioaddr + CSR6) & 0xfe3bd1fd);
                         tp->csr6 |= 0x00400000;

	                 tulip_restart_rxtx(tp);

                         return;

		}
	}

	if ((tp->nwayset  &&  (csr5 & 0x08000000)
			  && (dev->if_port == 3  ||  dev->if_port == 5)
			  && (csr12 & 2) == 2) || (tp->nway && (csr5 & (TPLnkFail)))) {

		

		if (tulip_debug > 2)
			printk(KERN_DEBUG "%s: Ugh! Link blew?\n", dev->name);

		del_timer_sync(&tp->timer);
		pnic2_start_nway(dev);
		tp->timer.expires = RUN_AT(3*HZ);
		add_timer(&tp->timer);

                return;
	}


        if (dev->if_port == 3  ||  dev->if_port == 5) {

	        

		if (tulip_debug > 1)
			printk(KERN_INFO"%s: PNIC2 %s link beat %s.\n",
				   dev->name, medianame[dev->if_port],
				   (csr12 & 2) ? "failed" : "good");

                

                tp->nway = 0;
                tp->nwayset = 1;

                
		if ((csr12 & 2)  &&  ! tp->medialock) {
			del_timer_sync(&tp->timer);
			pnic2_start_nway(dev);
			tp->timer.expires = RUN_AT(3*HZ);
       			add_timer(&tp->timer);
                }

                return;
        }

	if (dev->if_port == 0  ||  dev->if_port == 4) {

	        

		if (tulip_debug > 1)
			printk(KERN_INFO"%s: PNIC2 %s link beat %s.\n",
				   dev->name, medianame[dev->if_port],
				   (csr12 & 4) ? "failed" : "good");


                tp->nway = 0;
                tp->nwayset = 1;

                
		if ((csr12 & 4)  &&  ! tp->medialock) {
			del_timer_sync(&tp->timer);
			pnic2_start_nway(dev);
			tp->timer.expires = RUN_AT(3*HZ);
       			add_timer(&tp->timer);
                }

                return;
        }


	if (tulip_debug > 1)
		printk(KERN_INFO"%s: PNIC2 Link Change Default?\n",dev->name);

        
	dev->if_port = 0;

        
	csr14 = (ioread32(ioaddr + CSR14) & 0xffffff7f);
        iowrite32(csr14,ioaddr + CSR14);

        
	tp->csr6 = (ioread32(ioaddr + CSR6) & 0xfe3bd1fd);
        tp->csr6 |= 0x00400000;

	tulip_restart_rxtx(tp);
}

