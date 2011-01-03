

#include <linux/vmalloc.h>

#undef LOOP_TEST
#undef DUMP_RX
#undef DUMP_TX
#undef DEBUG_TX_DESC2
#undef RX_DONT_PASS_UL
#undef DEBUG_EPROM
#undef DEBUG_RX_VERBOSE
#undef DUMMY_RX
#undef DEBUG_ZERO_RX
#undef DEBUG_RX_SKB
#undef DEBUG_TX_FRAG
#undef DEBUG_RX_FRAG
#undef DEBUG_TX_FILLDESC
#undef DEBUG_TX
#undef DEBUG_IRQ
#undef DEBUG_RX
#undef DEBUG_RXALLOC
#undef DEBUG_REGISTERS
#undef DEBUG_RING
#undef DEBUG_IRQ_TASKLET
#undef DEBUG_TX_ALLOC
#undef DEBUG_TX_DESC

#define CONFIG_RTL8192_IO_MAP

#include <asm/uaccess.h>
#include "r8192U.h"
#include "r8180_93cx6.h"   
#include "r8192U_wx.h"

#include "r8192S_rtl8225.h"
#include "r8192S_hw.h"
#include "r8192S_phy.h"
#include "r8192S_phyreg.h"
#include "r8192S_Efuse.h"

#include "r819xU_cmdpkt.h"
#include "r8192U_dm.h"

#include <linux/usb.h>

#include "r8192U_pm.h"

#include "ieee80211/dot11d.h"



u32 rt_global_debug_component = \






				COMP_POWER	|

				COMP_SWBW	|
				COMP_POWER_TRACKING |
				COMP_TURBO	|
				COMP_QOS	|


				COMP_DIG	|



                              	COMP_HIPWR	|

				COMP_SEC	|
				COMP_LED	|


				COMP_FIRMWARE	|
				COMP_HT		|
				COMP_AMSDU	|
				COMP_SCAN	|

				COMP_DOWN	|
				COMP_RESET	|
				COMP_ERR; 

#define TOTAL_CAM_ENTRY 32
#define CAM_CONTENT_COUNT 8

static struct usb_device_id rtl8192_usb_id_tbl[] = {
	
	{USB_DEVICE(0x0bda, 0x8192)},
	{USB_DEVICE(0x0bda, 0x8709)},
	
	{USB_DEVICE(0x07aa, 0x0043)},
	
	{USB_DEVICE(0x050d, 0x805E)},
	
	{USB_DEVICE(0x0df6, 0x0031)},
	
	{USB_DEVICE(0x1740, 0x9201)},
	
	{USB_DEVICE(0x2001, 0x3301)},
	
	{USB_DEVICE(0x5a57, 0x0290)},
	
	{USB_DEVICE(0x0bda, 0x8172)},
	{}
};

MODULE_LICENSE("GPL");
MODULE_VERSION("V 1.1");
MODULE_DEVICE_TABLE(usb, rtl8192_usb_id_tbl);
MODULE_DESCRIPTION("Linux driver for Realtek RTL8192 USB WiFi cards");

static char* ifname = "wlan%d";
static int hwwep = 1;  
static int channels = 0x3fff;



module_param(ifname, charp, S_IRUGO|S_IWUSR );

module_param(hwwep,int, S_IRUGO|S_IWUSR);
module_param(channels,int, S_IRUGO|S_IWUSR);

MODULE_PARM_DESC(ifname," Net interface name, wlan%d=default");

MODULE_PARM_DESC(hwwep," Try to use hardware security support. ");
MODULE_PARM_DESC(channels," Channel bitmask for specific locales. NYI");

static int __devinit rtl8192_usb_probe(struct usb_interface *intf,
			 const struct usb_device_id *id);
static void __devexit rtl8192_usb_disconnect(struct usb_interface *intf);

static struct usb_driver rtl8192_usb_driver = {
	.name		= RTL819xU_MODULE_NAME,	          
	.id_table	= rtl8192_usb_id_tbl,	          
	.probe		= rtl8192_usb_probe,	          
	.disconnect	= rtl8192_usb_disconnect,	  
	.suspend	= rtl8192U_suspend,	          
	.resume		= rtl8192U_resume,                 
	.reset_resume   = rtl8192U_resume,                 
};


static void 	rtl8192SU_read_eeprom_info(struct net_device *dev);
short 	rtl8192SU_tx(struct net_device *dev, struct sk_buff* skb);
void 	rtl8192SU_rx_nomal(struct sk_buff* skb);
void 	rtl8192SU_rx_cmd(struct sk_buff *skb);
bool 	rtl8192SU_adapter_start(struct net_device *dev);
short	rtl8192SU_tx_cmd(struct net_device *dev, struct sk_buff *skb);
void 	rtl8192SU_link_change(struct net_device *dev);
void 	InitialGain8192S(struct net_device *dev,u8 Operation);
void 	rtl8192SU_query_rxdesc_status(struct sk_buff *skb, struct ieee80211_rx_stats *stats, bool bIsRxAggrSubframe);

struct rtl819x_ops rtl8192su_ops = {
	.nic_type = NIC_8192SU,
	.rtl819x_read_eeprom_info = rtl8192SU_read_eeprom_info,
	.rtl819x_tx = rtl8192SU_tx,
	.rtl819x_tx_cmd = rtl8192SU_tx_cmd,
	.rtl819x_rx_nomal = rtl8192SU_rx_nomal,
	.rtl819x_rx_cmd = rtl8192SU_rx_cmd,
	.rtl819x_adapter_start = rtl8192SU_adapter_start,
	.rtl819x_link_change = rtl8192SU_link_change,
	.rtl819x_initial_gain = InitialGain8192S,
	.rtl819x_query_rxdesc_status = rtl8192SU_query_rxdesc_status,
};


typedef struct _CHANNEL_LIST
{
	u8	Channel[32];
	u8	Len;
}CHANNEL_LIST, *PCHANNEL_LIST;

static CHANNEL_LIST ChannelPlan[] = {
	{{1,2,3,4,5,6,7,8,9,10,11,36,40,44,48,52,56,60,64,149,153,157,161,165},24},  		
	{{1,2,3,4,5,6,7,8,9,10,11},11},                    				
	{{1,2,3,4,5,6,7,8,9,10,11,12,13,36,40,44,48,52,56,60,64},21},  	
	{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},    
	{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},  	
	{{1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,40,44,48,52,56,60,64},22},	
	{{1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,40,44,48,52,56,60,64},22},
	{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},	
	{{1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,40,44,48,52,56,60,64},22},			
	{{1,2,3,4,5,6,7,8,9,10,11,12,13,14,36,40,44,48,52,56,60,64}, 22},    
	{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14}					
};

static void rtl819x_set_channel_map(u8 channel_plan, struct r8192_priv* priv)
{
	int i, max_chan=-1, min_chan=-1;
	struct ieee80211_device* ieee = priv->ieee80211;
	switch (channel_plan)
	{
		case COUNTRY_CODE_FCC:
		case COUNTRY_CODE_IC:
		case COUNTRY_CODE_ETSI:
		case COUNTRY_CODE_SPAIN:
		case COUNTRY_CODE_FRANCE:
		case COUNTRY_CODE_MKK:
		case COUNTRY_CODE_MKK1:
		case COUNTRY_CODE_ISRAEL:
		case COUNTRY_CODE_TELEC:
		case COUNTRY_CODE_MIC:
		{
			Dot11d_Init(ieee);
			ieee->bGlobalDomain = false;
			
                        if ((priv->rf_chip == RF_8225) || (priv->rf_chip == RF_8256) || (priv->rf_chip == RF_6052))
			{
				min_chan = 1;
				max_chan = 14;
			}
			else
			{
				RT_TRACE(COMP_ERR, "unknown rf chip, can't set channel map in function:%s()\n", __FUNCTION__);
			}
			if (ChannelPlan[channel_plan].Len != 0){
				
				memset(GET_DOT11D_INFO(ieee)->channel_map, 0, sizeof(GET_DOT11D_INFO(ieee)->channel_map));
				
				for (i=0;i<ChannelPlan[channel_plan].Len;i++)
				{
					if (ChannelPlan[channel_plan].Channel[i] < min_chan || ChannelPlan[channel_plan].Channel[i] > max_chan)
					break;
					GET_DOT11D_INFO(ieee)->channel_map[ChannelPlan[channel_plan].Channel[i]] = 1;
				}
			}
			break;
		}
		case COUNTRY_CODE_GLOBAL_DOMAIN:
		{
			GET_DOT11D_INFO(ieee)->bEnabled = 0;
			Dot11d_Reset(ieee);
			ieee->bGlobalDomain = true;
			break;
		}
		default:
			break;
	}
	return;
}

#define eqMacAddr(a,b) ( ((a)[0]==(b)[0] && (a)[1]==(b)[1] && (a)[2]==(b)[2] && (a)[3]==(b)[3] && (a)[4]==(b)[4] && (a)[5]==(b)[5]) ? 1:0 )

#define		rx_hal_is_cck_rate(_pDesc)\
			((_pDesc->RxMCS  == DESC92S_RATE1M ||\
			_pDesc->RxMCS == DESC92S_RATE2M ||\
			_pDesc->RxMCS == DESC92S_RATE5_5M ||\
			_pDesc->RxMCS == DESC92S_RATE11M) &&\
			!_pDesc->RxHT)

#define 	tx_hal_is_cck_rate(_DataRate)\
			( _DataRate == MGN_1M ||\
			 _DataRate == MGN_2M ||\
			 _DataRate == MGN_5_5M ||\
			 _DataRate == MGN_11M )




void CamResetAllEntry(struct net_device *dev)
{
#if 1
	u32 ulcommand = 0;
        
        
        
        
        

        
        
        
	ulcommand |= BIT31|BIT30;
	write_nic_dword(dev, RWCAM, ulcommand);
#else
        for(ucIndex=0;ucIndex<TOTAL_CAM_ENTRY;ucIndex++)
                CAM_mark_invalid(dev, ucIndex);
        for(ucIndex=0;ucIndex<TOTAL_CAM_ENTRY;ucIndex++)
                CAM_empty_entry(dev, ucIndex);
#endif

}


void write_cam(struct net_device *dev, u8 addr, u32 data)
{
        write_nic_dword(dev, WCAMI, data);
        write_nic_dword(dev, RWCAM, BIT31|BIT16|(addr&0xff) );
}

u32 read_cam(struct net_device *dev, u8 addr)
{
        write_nic_dword(dev, RWCAM, 0x80000000|(addr&0xff) );
        return read_nic_dword(dev, 0xa8);
}

void write_nic_byte_E(struct net_device *dev, int indx, u8 data)
{
	int status;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;

	status = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			       RTL8187_REQ_SET_REGS, RTL8187_REQT_WRITE,
			       indx|0xfe00, 0, &data, 1, HZ / 2);

	if (status < 0)
	{
		printk("write_nic_byte_E TimeOut! status:%d\n", status);
	}
}

u8 read_nic_byte_E(struct net_device *dev, int indx)
{
	int status;
	u8 data;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;

	status = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       indx|0xfe00, 0, &data, 1, HZ / 2);

        if (status < 0)
        {
                printk("read_nic_byte_E TimeOut! status:%d\n", status);
        }

	return data;
}

void write_nic_byte(struct net_device *dev, int indx, u8 data)
{
	int status;

	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;

	status = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			       RTL8187_REQ_SET_REGS, RTL8187_REQT_WRITE,
			       indx, 0, &data, 1, HZ / 2);

        if (status < 0)
        {
                printk("write_nic_byte TimeOut! status:%d\n", status);
        }


}


void write_nic_word(struct net_device *dev, int indx, u16 data)
{

	int status;

	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;

	status = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			       RTL8187_REQ_SET_REGS, RTL8187_REQT_WRITE,
			       indx, 0, &data, 2, HZ / 2);

        if (status < 0)
        {
                printk("write_nic_word TimeOut! status:%d\n", status);
        }

}


void write_nic_dword(struct net_device *dev, int indx, u32 data)
{

	int status;

	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;

	status = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			       RTL8187_REQ_SET_REGS, RTL8187_REQT_WRITE,
			       indx, 0, &data, 4, HZ / 2);


        if (status < 0)
        {
                printk("write_nic_dword TimeOut! status:%d\n", status);
        }

}



u8 read_nic_byte(struct net_device *dev, int indx)
{
	u8 data;
	int status;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;

	status = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       indx, 0, &data, 1, HZ / 2);

        if (status < 0)
        {
                printk("read_nic_byte TimeOut! status:%d\n", status);
        }

	return data;
}



u16 read_nic_word(struct net_device *dev, int indx)
{
	u16 data;
	int status;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;

	status = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       indx, 0, &data, 2, HZ / 2);

        if (status < 0)
        {
                printk("read_nic_word TimeOut! status:%d\n", status);
        }


	return data;
}

u16 read_nic_word_E(struct net_device *dev, int indx)
{
	u16 data;
	int status;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;

	status = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       indx|0xfe00, 0, &data, 2, HZ / 2);

        if (status < 0)
        {
                printk("read_nic_word TimeOut! status:%d\n", status);
        }


	return data;
}

u32 read_nic_dword(struct net_device *dev, int indx)
{
	u32 data;
	int status;


	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct usb_device *udev = priv->udev;

	status = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			       RTL8187_REQ_GET_REGS, RTL8187_REQT_READ,
			       indx, 0, &data, 4, HZ / 2);




        if (status < 0)
        {
                printk("read_nic_dword TimeOut! status:%d\n", status);
		if(status == -ENODEV) {
			priv->usb_error = true;
		}
        }



	return data;
}





inline void force_pci_posting(struct net_device *dev)
{
}


static struct net_device_stats *rtl8192_stats(struct net_device *dev);
void rtl8192_commit(struct net_device *dev);

void rtl8192_restart(struct work_struct *work);


void watch_dog_timer_callback(unsigned long data);



static struct proc_dir_entry *rtl8192_proc = NULL;



static int proc_get_stats_ap(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct ieee80211_device *ieee = priv->ieee80211;
	struct ieee80211_network *target;

	int len = 0;

        list_for_each_entry(target, &ieee->network_list, list) {

		len += snprintf(page + len, count - len,
                "%s ", target->ssid);

		if(target->wpa_ie_len>0 || target->rsn_ie_len>0){
	                len += snprintf(page + len, count - len,
        	        "WPA\n");
		}
		else{
                        len += snprintf(page + len, count - len,
                        "non_WPA\n");
                }

        }

	*eof = 1;
	return len;
}

static int proc_get_registers(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;


	int len = 0;
	int i,n,page0,page1,page2;

	int max=0xff;
	page0 = 0x000;
	page1 = 0x100;
	page2 = 0x800;

	
	if(!IS_BB_REG_OFFSET_92S(page0)){
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len,
					"\nD:  %2x > ",n);
			for(i=0;i<16 && n<=max;i++,n++)
				len += snprintf(page + len, count - len,
						"%2.2x ",read_nic_byte(dev,(page0|n)));
		}
	}else{
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len, "\nD:  %2x > ",n);
			for(i=0;i<4 && n<=max;n+=4,i++)
				len += snprintf(page + len, count - len,
						"%8.8x ",rtl8192_QueryBBReg(dev,(page0|n), bMaskDWord));
		}
	}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;

}
static int proc_get_registers_1(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;


	int len = 0;
	int i,n,page0;

	int max=0xff;
	page0 = 0x100;

	
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len,
					"\nD:  %2x > ",n);
			for(i=0;i<16 && n<=max;i++,n++)
				len += snprintf(page + len, count - len,
						"%2.2x ",read_nic_byte(dev,(page0|n)));
		}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;

}
static int proc_get_registers_2(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;


	int len = 0;
	int i,n,page0;

	int max=0xff;
	page0 = 0x200;

	
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len,
					"\nD:  %2x > ",n);
			for(i=0;i<16 && n<=max;i++,n++)
				len += snprintf(page + len, count - len,
						"%2.2x ",read_nic_byte(dev,(page0|n)));
		}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;

}
static int proc_get_registers_8(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;

	int len = 0;
	int i,n,page0;

	int max=0xff;
	page0 = 0x800;

	
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len, "\nD:  %2x > ",n);
			for(i=0;i<4 && n<=max;n+=4,i++)
				len += snprintf(page + len, count - len,
						"%8.8x ",rtl8192_QueryBBReg(dev,(page0|n), bMaskDWord));
		}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;

	}
static int proc_get_registers_9(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;


	int len = 0;
	int i,n,page0;

	int max=0xff;
	page0 = 0x900;

	
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len, "\nD:  %2x > ",n);
			for(i=0;i<4 && n<=max;n+=4,i++)
			len += snprintf(page + len, count - len,
						"%8.8x ",rtl8192_QueryBBReg(dev,(page0|n), bMaskDWord));
		}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;
}
static int proc_get_registers_a(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;


	int len = 0;
	int i,n,page0;

	int max=0xff;
	page0 = 0xa00;

	
				len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len, "\nD:  %2x > ",n);
			for(i=0;i<4 && n<=max;n+=4,i++)
				len += snprintf(page + len, count - len,
						"%8.8x ",rtl8192_QueryBBReg(dev,(page0|n), bMaskDWord));
		}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;
}
static int proc_get_registers_b(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;


	int len = 0;
	int i,n,page0;

	int max=0xff;
	page0 = 0xb00;

	
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len, "\nD:  %2x > ",n);
			for(i=0;i<4 && n<=max;n+=4,i++)
				len += snprintf(page + len, count - len,
						"%8.8x ",rtl8192_QueryBBReg(dev,(page0|n), bMaskDWord));
		}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;
	}
static int proc_get_registers_c(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;


	int len = 0;
	int i,n,page0;

	int max=0xff;
	page0 = 0xc00;

	
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len, "\nD:  %2x > ",n);
			for(i=0;i<4 && n<=max;n+=4,i++)
				len += snprintf(page + len, count - len,
						"%8.8x ",rtl8192_QueryBBReg(dev,(page0|n), bMaskDWord));
		}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;
}
static int proc_get_registers_d(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;


	int len = 0;
	int i,n,page0;

	int max=0xff;
	page0 = 0xd00;

	
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len, "\nD:  %2x > ",n);
			for(i=0;i<4 && n<=max;n+=4,i++)
				len += snprintf(page + len, count - len,
						"%8.8x ",rtl8192_QueryBBReg(dev,(page0|n), bMaskDWord));
		}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;
}
static int proc_get_registers_e(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;


	int len = 0;
	int i,n,page0;

	int max=0xff;
	page0 = 0xe00;

	
		len += snprintf(page + len, count - len,
				"\n####################page %x##################\n ", (page0>>8));
		for(n=0;n<=max;)
		{
			len += snprintf(page + len, count - len, "\nD:  %2x > ",n);
			for(i=0;i<4 && n<=max;n+=4,i++)
				len += snprintf(page + len, count - len,
						"%8.8x ",rtl8192_QueryBBReg(dev,(page0|n), bMaskDWord));
		}
	len += snprintf(page + len, count - len,"\n");
	*eof = 1;
	return len;
}

static int proc_get_stats_tx(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);

	int len = 0;

	len += snprintf(page + len, count - len,
		"TX VI priority ok int: %lu\n"
		"TX VI priority error int: %lu\n"
		"TX VO priority ok int: %lu\n"
		"TX VO priority error int: %lu\n"
		"TX BE priority ok int: %lu\n"
		"TX BE priority error int: %lu\n"
		"TX BK priority ok int: %lu\n"
		"TX BK priority error int: %lu\n"
		"TX MANAGE priority ok int: %lu\n"
		"TX MANAGE priority error int: %lu\n"
		"TX BEACON priority ok int: %lu\n"
		"TX BEACON priority error int: %lu\n"


		"TX queue resume: %lu\n"
		"TX queue stopped?: %d\n"
		"TX fifo overflow: %lu\n"

		"TX VI queue: %d\n"
		"TX VO queue: %d\n"
		"TX BE queue: %d\n"
		"TX BK queue: %d\n"

		"TX VI dropped: %lu\n"
		"TX VO dropped: %lu\n"
		"TX BE dropped: %lu\n"
		"TX BK dropped: %lu\n"
		"TX total data packets %lu\n",

		priv->stats.txviokint,
		priv->stats.txvierr,
		priv->stats.txvookint,
		priv->stats.txvoerr,
		priv->stats.txbeokint,
		priv->stats.txbeerr,
		priv->stats.txbkokint,
		priv->stats.txbkerr,
		priv->stats.txmanageokint,
		priv->stats.txmanageerr,
		priv->stats.txbeaconokint,
		priv->stats.txbeaconerr,


		priv->stats.txresumed,
		netif_queue_stopped(dev),
		priv->stats.txoverflow,

		atomic_read(&(priv->tx_pending[VI_PRIORITY])),
		atomic_read(&(priv->tx_pending[VO_PRIORITY])),
		atomic_read(&(priv->tx_pending[BE_PRIORITY])),
		atomic_read(&(priv->tx_pending[BK_PRIORITY])),

		priv->stats.txvidrop,
		priv->stats.txvodrop,
		priv->stats.txbedrop,
		priv->stats.txbkdrop,
		priv->stats.txdatapkt

		);

	*eof = 1;
	return len;
}



static int proc_get_stats_rx(char *page, char **start,
			  off_t offset, int count,
			  int *eof, void *data)
{
	struct net_device *dev = data;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);

	int len = 0;

	len += snprintf(page + len, count - len,
		"RX packets: %lu\n"
		"RX urb status error: %lu\n"
		"RX invalid urb error: %lu\n",
		priv->stats.rxoktotal,
		priv->stats.rxstaterr,
		priv->stats.rxurberr);

	*eof = 1;
	return len;
}

void rtl8192_proc_module_init(void)
{
	RT_TRACE(COMP_INIT, "Initializing proc filesystem");
	rtl8192_proc=create_proc_entry(RTL819xU_MODULE_NAME, S_IFDIR, init_net.proc_net);
}


void rtl8192_proc_module_remove(void)
{
	remove_proc_entry(RTL819xU_MODULE_NAME, init_net.proc_net);
}


void rtl8192_proc_remove_one(struct net_device *dev)
{
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);


	if (priv->dir_dev) {
	
		remove_proc_entry("stats-tx", priv->dir_dev);
		remove_proc_entry("stats-rx", priv->dir_dev);
	
		remove_proc_entry("stats-ap", priv->dir_dev);
		remove_proc_entry("registers", priv->dir_dev);
		remove_proc_entry("registers-1", priv->dir_dev);
		remove_proc_entry("registers-2", priv->dir_dev);
		remove_proc_entry("registers-8", priv->dir_dev);
		remove_proc_entry("registers-9", priv->dir_dev);
		remove_proc_entry("registers-a", priv->dir_dev);
		remove_proc_entry("registers-b", priv->dir_dev);
		remove_proc_entry("registers-c", priv->dir_dev);
		remove_proc_entry("registers-d", priv->dir_dev);
		remove_proc_entry("registers-e", priv->dir_dev);
	
	
		
		remove_proc_entry("wlan0", rtl8192_proc);
		priv->dir_dev = NULL;
	}
}


void rtl8192_proc_init_one(struct net_device *dev)
{
	struct proc_dir_entry *e;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	priv->dir_dev = create_proc_entry(dev->name,
					  S_IFDIR | S_IRUGO | S_IXUGO,
					  rtl8192_proc);
	if (!priv->dir_dev) {
		RT_TRACE(COMP_ERR, "Unable to initialize /proc/net/rtl8192/%s\n",
		      dev->name);
		return;
	}
	e = create_proc_read_entry("stats-rx", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_stats_rx, dev);

	if (!e) {
		RT_TRACE(COMP_ERR,"Unable to initialize "
		      "/proc/net/rtl8192/%s/stats-rx\n",
		      dev->name);
	}


	e = create_proc_read_entry("stats-tx", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_stats_tx, dev);

	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/stats-tx\n",
		      dev->name);
	}

	e = create_proc_read_entry("stats-ap", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_stats_ap, dev);

	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/stats-ap\n",
		      dev->name);
	}

	e = create_proc_read_entry("registers", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers\n",
		      dev->name);
	}
	e = create_proc_read_entry("registers-1", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers_1, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers-1\n",
		      dev->name);
	}
	e = create_proc_read_entry("registers-2", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers_2, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers-2\n",
		      dev->name);
	}
	e = create_proc_read_entry("registers-8", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers_8, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers-8\n",
		      dev->name);
	}
	e = create_proc_read_entry("registers-9", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers_9, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers-9\n",
		      dev->name);
	}
	e = create_proc_read_entry("registers-a", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers_a, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers-a\n",
		      dev->name);
	}
	e = create_proc_read_entry("registers-b", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers_b, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers-b\n",
		      dev->name);
	}
	e = create_proc_read_entry("registers-c", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers_c, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers-c\n",
		      dev->name);
	}
	e = create_proc_read_entry("registers-d", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers_d, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers-d\n",
		      dev->name);
	}
	e = create_proc_read_entry("registers-e", S_IFREG | S_IRUGO,
				   priv->dir_dev, proc_get_registers_e, dev);
	if (!e) {
		RT_TRACE(COMP_ERR, "Unable to initialize "
		      "/proc/net/rtl8192/%s/registers-e\n",
		      dev->name);
	}
}



void print_buffer(u32 *buffer, int len)
{
	int i;
	u8 *buf =(u8*)buffer;

	printk("ASCII BUFFER DUMP (len: %x):\n",len);

	for(i=0;i<len;i++)
		printk("%c",buf[i]);

	printk("\nBINARY BUFFER DUMP (len: %x):\n",len);

	for(i=0;i<len;i++)
		printk("%x",buf[i]);

	printk("\n");
}


short check_nic_enough_desc(struct net_device *dev,int queue_index)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	int used = atomic_read(&priv->tx_pending[queue_index]);

	return (used < MAX_TX_URB);
}

void tx_timeout(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	

	schedule_work(&priv->reset_wq);
	
}



void dump_eprom(struct net_device *dev)
{
	int i;
	for(i=0; i<63; i++)
		RT_TRACE(COMP_EPROM, "EEPROM addr %x : %x", i, eprom_read(dev,i));
}


void rtl8192_dump_reg(struct net_device *dev)
{
	int i;
	int n;
	int max=0x1ff;

	RT_TRACE(COMP_PHY, "Dumping NIC register map");

	for(n=0;n<=max;)
	{
		printk( "\nD: %2x> ", n);
		for(i=0;i<16 && n<=max;i++,n++)
			printk("%2x ",read_nic_byte(dev,n));
	}
	printk("\n");
}



void rtl8192_set_mode(struct net_device *dev,int mode)
{
	u8 ecmd;
	ecmd=read_nic_byte(dev, EPROM_CMD);
	ecmd=ecmd &~ EPROM_CMD_OPERATING_MODE_MASK;
	ecmd=ecmd | (mode<<EPROM_CMD_OPERATING_MODE_SHIFT);
	ecmd=ecmd &~ (1<<EPROM_CS_SHIFT);
	ecmd=ecmd &~ (1<<EPROM_CK_SHIFT);
	write_nic_byte(dev, EPROM_CMD, ecmd);
}


void rtl8192_update_msr(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	u8 msr;

	msr  = read_nic_byte(dev, MSR);
	msr &= ~ MSR_LINK_MASK;

	
	if (priv->ieee80211->state == IEEE80211_LINKED){

		if (priv->ieee80211->iw_mode == IW_MODE_INFRA)
			msr |= (MSR_LINK_MANAGED<<MSR_LINK_SHIFT);
		else if (priv->ieee80211->iw_mode == IW_MODE_ADHOC)
			msr |= (MSR_LINK_ADHOC<<MSR_LINK_SHIFT);
		else if (priv->ieee80211->iw_mode == IW_MODE_MASTER)
			msr |= (MSR_LINK_MASTER<<MSR_LINK_SHIFT);

	}else
		msr |= (MSR_LINK_NONE<<MSR_LINK_SHIFT);

	write_nic_byte(dev, MSR, msr);
}

void rtl8192_set_chan(struct net_device *dev,short ch)
{
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);

	RT_TRACE(COMP_CH, "=====>%s()====ch:%d\n", __FUNCTION__, ch);
	
	priv->chan=ch;

	





#ifndef LOOP_TEST


	

	if (priv->rf_set_chan)
	priv->rf_set_chan(dev,priv->chan);
	mdelay(10);

#endif
}

static void rtl8192_rx_isr(struct urb *urb);

u32 get_rxpacket_shiftbytes_819xusb(struct ieee80211_rx_stats *pstats)
{

		return (sizeof(rx_desc_819x_usb) + pstats->RxDrvInfoSize
				+ pstats->RxBufShift);

}
static int rtl8192_rx_initiate(struct net_device*dev)
{
        struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
        struct urb *entry;
        struct sk_buff *skb;
        struct rtl8192_rx_info *info;

	
        while (skb_queue_len(&priv->rx_queue) < MAX_RX_URB) {
                skb = __dev_alloc_skb(RX_URB_SIZE, GFP_KERNEL);
                if (!skb)
                        break;
	        entry = usb_alloc_urb(0, GFP_KERNEL);
                if (!entry) {
                        kfree_skb(skb);
                        break;
                }

                usb_fill_bulk_urb(entry, priv->udev,
                                  usb_rcvbulkpipe(priv->udev, 3), skb_tail_pointer(skb),
                                  RX_URB_SIZE, rtl8192_rx_isr, skb);
                info = (struct rtl8192_rx_info *) skb->cb;
                info->urb = entry;
                info->dev = dev;
		info->out_pipe = 3; 
                skb_queue_tail(&priv->rx_queue, skb);
                usb_submit_urb(entry, GFP_KERNEL);
        }

	
        while (skb_queue_len(&priv->rx_queue) < MAX_RX_URB + 3) {

                skb = __dev_alloc_skb(RX_URB_SIZE ,GFP_KERNEL);
                if (!skb)
                        break;
                entry = usb_alloc_urb(0, GFP_KERNEL);
                if (!entry) {
                        kfree_skb(skb);
                        break;
                }
                usb_fill_bulk_urb(entry, priv->udev,
                                  usb_rcvbulkpipe(priv->udev, 9), skb_tail_pointer(skb),
                                  RX_URB_SIZE, rtl8192_rx_isr, skb);
                info = (struct rtl8192_rx_info *) skb->cb;
                info->urb = entry;
                info->dev = dev;
		   info->out_pipe = 9; 
                skb_queue_tail(&priv->rx_queue, skb);
		usb_submit_urb(entry, GFP_KERNEL);
        }

        return 0;
}

void rtl8192_set_rxconf(struct net_device *dev)
{
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	u32 rxconf;

	rxconf=read_nic_dword(dev,RCR);
	rxconf = rxconf &~ MAC_FILTER_MASK;
	rxconf = rxconf | RCR_AMF;
	rxconf = rxconf | RCR_ADF;
	rxconf = rxconf | RCR_AB;
	rxconf = rxconf | RCR_AM;
	

	if (dev->flags & IFF_PROMISC) {DMESG ("NIC in promisc mode");}

	if(priv->ieee80211->iw_mode == IW_MODE_MONITOR || \
	   dev->flags & IFF_PROMISC){
		rxconf = rxconf | RCR_AAP;
	} else{
		rxconf = rxconf | RCR_APM;
		rxconf = rxconf | RCR_CBSSID;
	}


	if(priv->ieee80211->iw_mode == IW_MODE_MONITOR){
		rxconf = rxconf | RCR_AICV;
		rxconf = rxconf | RCR_APWRMGT;
	}

	if( priv->crcmon == 1 && priv->ieee80211->iw_mode == IW_MODE_MONITOR)
		rxconf = rxconf | RCR_ACRC32;


	rxconf = rxconf &~ RX_FIFO_THRESHOLD_MASK;
	rxconf = rxconf | (RX_FIFO_THRESHOLD_NONE<<RX_FIFO_THRESHOLD_SHIFT);
	rxconf = rxconf &~ MAX_RX_DMA_MASK;
	rxconf = rxconf | ((u32)7<<RCR_MXDMA_OFFSET);


	rxconf = rxconf | RCR_ONLYERLPKT;




	write_nic_dword(dev, RCR, rxconf);

	#ifdef DEBUG_RX
	DMESG("rxconf: %x %x",rxconf ,read_nic_dword(dev,RCR));
	#endif
}

void rtl8192_rx_enable(struct net_device *dev)
{
	

	

	rtl8192_rx_initiate(dev);


}


void rtl8192_tx_enable(struct net_device *dev)
{
}

void rtl8192_rtx_disable(struct net_device *dev)
{
	u8 cmd;
	struct r8192_priv *priv = ieee80211_priv(dev);
	struct sk_buff *skb;
	struct rtl8192_rx_info *info;

	cmd=read_nic_byte(dev,CMDR);
	write_nic_byte(dev, CMDR, cmd &~ \
		(CR_TE|CR_RE));
	force_pci_posting(dev);
	mdelay(10);

	while ((skb = __skb_dequeue(&priv->rx_queue))) {
		info = (struct rtl8192_rx_info *) skb->cb;
		if (!info->urb)
			continue;

		usb_kill_urb(info->urb);
		kfree_skb(skb);
	}

	if (skb_queue_len(&priv->skb_queue)) {
		printk(KERN_WARNING "skb_queue not empty\n");
	}

	skb_queue_purge(&priv->skb_queue);
	return;
}


int alloc_tx_beacon_desc_ring(struct net_device *dev, int count)
{
	return 0;
}

inline u16 ieeerate2rtlrate(int rate)
{
	switch(rate){
	case 10:
	return 0;
	case 20:
	return 1;
	case 55:
	return 2;
	case 110:
	return 3;
	case 60:
	return 4;
	case 90:
	return 5;
	case 120:
	return 6;
	case 180:
	return 7;
	case 240:
	return 8;
	case 360:
	return 9;
	case 480:
	return 10;
	case 540:
	return 11;
	default:
	return 3;

	}
}
static u16 rtl_rate[] = {10,20,55,110,60,90,120,180,240,360,480,540};
inline u16 rtl8192_rate2rate(short rate)
{
	if (rate >11) return 0;
	return rtl_rate[rate];
}

static void rtl8192_rx_isr(struct urb *urb)
{
        struct sk_buff *skb = (struct sk_buff *) urb->context;
        struct rtl8192_rx_info *info = (struct rtl8192_rx_info *)skb->cb;
        struct net_device *dev = info->dev;
	struct r8192_priv *priv = ieee80211_priv(dev);
	int out_pipe = info->out_pipe;
	int err;
	if(!priv->up)
		return;
        if (unlikely(urb->status)) {
                info->urb = NULL;
                priv->stats.rxstaterr++;
                priv->ieee80211->stats.rx_errors++;
                usb_free_urb(urb);
	
                return;
        }

        skb_unlink(skb, &priv->rx_queue);
        skb_put(skb, urb->actual_length);

	skb_queue_tail(&priv->skb_queue, skb);
	tasklet_schedule(&priv->irq_rx_tasklet);

        skb = dev_alloc_skb(RX_URB_SIZE);
        if (unlikely(!skb)) {
                usb_free_urb(urb);
		printk("%s():can,t alloc skb\n",__FUNCTION__);
                
                return;
        }

	usb_fill_bulk_urb(urb, priv->udev,
			usb_rcvbulkpipe(priv->udev, out_pipe),
			skb_tail_pointer(skb),
			RX_URB_SIZE, rtl8192_rx_isr, skb);

        info = (struct rtl8192_rx_info *) skb->cb;
        info->urb = urb;
        info->dev = dev;
	info->out_pipe = out_pipe;

        urb->transfer_buffer = skb_tail_pointer(skb);
        urb->context = skb;
        skb_queue_tail(&priv->rx_queue, skb);
        err = usb_submit_urb(urb, GFP_ATOMIC);
	if(err && err != EPERM)
		printk("can not submit rxurb, err is %x,URB status is %x\n",err,urb->status);
}

u32
rtl819xusb_rx_command_packet(
	struct net_device *dev,
	struct ieee80211_rx_stats *pstats
	)
{
	u32	status;

	

	status = cmpk_message_handle_rx(dev, pstats);
	if (status)
	{
		DMESG("rxcommandpackethandle819xusb: It is a command packet\n");
	}
	else
	{
		
	}

	
	return status;
}

void rtl8192_data_hard_stop(struct net_device *dev)
{
	
}


void rtl8192_data_hard_resume(struct net_device *dev)
{
	
}


void rtl8192_hard_data_xmit(struct sk_buff *skb, struct net_device *dev, int rate)
{
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	int ret;
	unsigned long flags;
	cb_desc *tcb_desc = (cb_desc *)(skb->cb + MAX_DEV_ADDR_SIZE);
	u8 queue_index = tcb_desc->queue_index;

	
	assert(queue_index != TXCMD_QUEUE);

	spin_lock_irqsave(&priv->tx_lock,flags);

        memcpy((unsigned char *)(skb->cb),&dev,sizeof(dev));



	tcb_desc->bTxEnableFwCalcDur = 1;
	skb_push(skb, priv->ieee80211->tx_headroom);
	ret = priv->ops->rtl819x_tx(dev, skb);

	
	

	spin_unlock_irqrestore(&priv->tx_lock,flags);


	return;
}


int rtl8192_hard_start_xmit(struct sk_buff *skb,struct net_device *dev)
{
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	int ret;
	unsigned long flags;
        cb_desc *tcb_desc = (cb_desc *)(skb->cb + MAX_DEV_ADDR_SIZE);
        u8 queue_index = tcb_desc->queue_index;


	spin_lock_irqsave(&priv->tx_lock,flags);

        memcpy((unsigned char *)(skb->cb),&dev,sizeof(dev));
	if(queue_index == TXCMD_QUEUE) {
		skb_push(skb, USB_HWDESC_HEADER_LEN);
		priv->ops->rtl819x_tx_cmd(dev, skb);
		ret = 1;
	        spin_unlock_irqrestore(&priv->tx_lock,flags);
		return ret;
	} else {
		skb_push(skb, priv->ieee80211->tx_headroom);
		ret = priv->ops->rtl819x_tx(dev, skb);
	}

	spin_unlock_irqrestore(&priv->tx_lock,flags);

	return ret;
}


void rtl8192_try_wake_queue(struct net_device *dev, int pri);


static void rtl8192_tx_isr(struct urb *tx_urb)
{
	struct sk_buff *skb = (struct sk_buff*)tx_urb->context;
	struct net_device *dev = NULL;
	struct r8192_priv *priv = NULL;
	cb_desc *tcb_desc = (cb_desc *)(skb->cb + MAX_DEV_ADDR_SIZE);
	u8  queue_index = tcb_desc->queue_index;



	memcpy(&dev,(struct net_device*)(skb->cb),sizeof(struct net_device*));
	priv = ieee80211_priv(dev);

	if(tcb_desc->queue_index != TXCMD_QUEUE) {
		if(tx_urb->status == 0) {
		
			
			
			
			priv->stats.txoktotal++;
			priv->ieee80211->LinkDetectInfo.NumTxOkInPeriod++;
			priv->stats.txbytesunicast += (skb->len - priv->ieee80211->tx_headroom);
		} else {
			priv->ieee80211->stats.tx_errors++;
			
			
		}
	}

	
	if(skb != NULL) {
		dev_kfree_skb_any(skb);
		usb_free_urb(tx_urb);
		atomic_dec(&priv->tx_pending[queue_index]);
	}

	{
		
		
		
		
		
		
		
		
		
		
	if (queue_index == MGNT_QUEUE){
        if (priv->ieee80211->ack_tx_to_ieee){
            if (rtl8192_is_tx_queue_empty(dev)){
                priv->ieee80211->ack_tx_to_ieee = 0;
                ieee80211_ps_tx_ack(priv->ieee80211, 1);
            }
        }
    }
		
		if(queue_index != BEACON_QUEUE) {
			
			if((skb_queue_len(&priv->ieee80211->skb_waitQ[queue_index]) != 0)&&\
					(!(priv->ieee80211->queue_stop))) {
				if(NULL != (skb = skb_dequeue(&(priv->ieee80211->skb_waitQ[queue_index]))))
					priv->ieee80211->softmac_hard_start_xmit(skb, dev);

				return; 
			}
		}
	}
}

void rtl8192_beacon_stop(struct net_device *dev)
{
	u8 msr, msrm, msr2;
	struct r8192_priv *priv = ieee80211_priv(dev);

	msr  = read_nic_byte(dev, MSR);
	msrm = msr & MSR_LINK_MASK;
	msr2 = msr & ~MSR_LINK_MASK;

	if(NIC_8192U == priv->card_8192) {
		usb_kill_urb(priv->rx_urb[MAX_RX_URB]);
	}
	if ((msrm == (MSR_LINK_ADHOC<<MSR_LINK_SHIFT) ||
		(msrm == (MSR_LINK_MASTER<<MSR_LINK_SHIFT)))){
		write_nic_byte(dev, MSR, msr2 | MSR_LINK_NONE);
		write_nic_byte(dev, MSR, msr);
	}
}

void rtl8192_config_rate(struct net_device* dev, u16* rate_config)
{
	 struct r8192_priv *priv = ieee80211_priv(dev);
	 struct ieee80211_network *net;
	 u8 i=0, basic_rate = 0;
	 net = & priv->ieee80211->current_network;

	 for (i=0; i<net->rates_len; i++)
	 {
		 basic_rate = net->rates[i]&0x7f;
		 switch(basic_rate)
		 {
			 case MGN_1M:	*rate_config |= RRSR_1M;	break;
			 case MGN_2M:	*rate_config |= RRSR_2M;	break;
			 case MGN_5_5M:	*rate_config |= RRSR_5_5M;	break;
			 case MGN_11M:	*rate_config |= RRSR_11M;	break;
			 case MGN_6M:	*rate_config |= RRSR_6M;	break;
			 case MGN_9M:	*rate_config |= RRSR_9M;	break;
			 case MGN_12M:	*rate_config |= RRSR_12M;	break;
			 case MGN_18M:	*rate_config |= RRSR_18M;	break;
			 case MGN_24M:	*rate_config |= RRSR_24M;	break;
			 case MGN_36M:	*rate_config |= RRSR_36M;	break;
			 case MGN_48M:	*rate_config |= RRSR_48M;	break;
			 case MGN_54M:	*rate_config |= RRSR_54M;	break;
		 }
	 }
	 for (i=0; i<net->rates_ex_len; i++)
	 {
		 basic_rate = net->rates_ex[i]&0x7f;
		 switch(basic_rate)
		 {
			 case MGN_1M:	*rate_config |= RRSR_1M;	break;
			 case MGN_2M:	*rate_config |= RRSR_2M;	break;
			 case MGN_5_5M:	*rate_config |= RRSR_5_5M;	break;
			 case MGN_11M:	*rate_config |= RRSR_11M;	break;
			 case MGN_6M:	*rate_config |= RRSR_6M;	break;
			 case MGN_9M:	*rate_config |= RRSR_9M;	break;
			 case MGN_12M:	*rate_config |= RRSR_12M;	break;
			 case MGN_18M:	*rate_config |= RRSR_18M;	break;
			 case MGN_24M:	*rate_config |= RRSR_24M;	break;
			 case MGN_36M:	*rate_config |= RRSR_36M;	break;
			 case MGN_48M:	*rate_config |= RRSR_48M;	break;
			 case MGN_54M:	*rate_config |= RRSR_54M;	break;
		 }
	 }
}


#define SHORT_SLOT_TIME 9
#define NON_SHORT_SLOT_TIME 20

void rtl8192_update_cap(struct net_device* dev, u16 cap)
{
	
	struct r8192_priv *priv = ieee80211_priv(dev);
	struct ieee80211_network *net = &priv->ieee80211->current_network;
	priv->short_preamble = cap & WLAN_CAPABILITY_SHORT_PREAMBLE;

	
	if(0)
	{
		u8 tmp = 0;
		tmp = ((priv->nCur40MhzPrimeSC) << 5);
		if (priv->short_preamble)
			tmp |= 0x80;
		write_nic_byte(dev, RRSR+2, tmp);
	}

	if (net->mode & (IEEE_G|IEEE_N_24G))
	{
		u8 slot_time = 0;
		if ((cap & WLAN_CAPABILITY_SHORT_SLOT)&&(!priv->ieee80211->pHTInfo->bCurrentRT2RTLongSlotTime))
		{
			slot_time = SHORT_SLOT_TIME;
		}
		else 
			slot_time = NON_SHORT_SLOT_TIME;
		priv->slot_time = slot_time;
		write_nic_byte(dev, SLOT_TIME, slot_time);
	}

}
void rtl8192_net_update(struct net_device *dev)
{

	struct r8192_priv *priv = ieee80211_priv(dev);
	struct ieee80211_network *net;
	u16 BcnTimeCfg = 0, BcnCW = 6, BcnIFS = 0xf;
	u16 rate_config = 0;
	net = & priv->ieee80211->current_network;

	rtl8192_config_rate(dev, &rate_config);
	priv->basic_rate = rate_config &= 0x15f;

	write_nic_dword(dev,BSSIDR,((u32*)net->bssid)[0]);
	write_nic_word(dev,BSSIDR+4,((u16*)net->bssid)[2]);
	
	

	rtl8192_update_msr(dev);

	if (priv->ieee80211->iw_mode == IW_MODE_ADHOC)
	{
	write_nic_word(dev, ATIMWND, 2);
	write_nic_word(dev, BCN_DMATIME, 1023);
	write_nic_word(dev, BCN_INTERVAL, net->beacon_interval);

	write_nic_word(dev, BCN_DRV_EARLY_INT, 1);
	write_nic_byte(dev, BCN_ERR_THRESH, 100);
		BcnTimeCfg |= (BcnCW<<BCN_TCFG_CW_SHIFT);
	
	 	BcnTimeCfg |= BcnIFS<<BCN_TCFG_IFS;

	write_nic_word(dev, BCN_TCFG, BcnTimeCfg);
	}



}



#if 1
void rtl819xusb_beacon_tx(struct net_device *dev,u16  tx_rate)
{
}
#endif
inline u8 rtl8192_IsWirelessBMode(u16 rate)
{
	if( ((rate <= 110) && (rate != 60) && (rate != 90)) || (rate == 220) )
		return 1;
	else return 0;
}

u16 N_DBPSOfRate(u16 DataRate);

u16 ComputeTxTime(
	u16		FrameLength,
	u16		DataRate,
	u8		bManagementFrame,
	u8		bShortPreamble
)
{
	u16	FrameTime;
	u16	N_DBPS;
	u16	Ceiling;

	if( rtl8192_IsWirelessBMode(DataRate) )
	{
		if( bManagementFrame || !bShortPreamble || DataRate == 10 )
		{	
			FrameTime = (u16)(144+48+(FrameLength*8/(DataRate/10)));
		}
		else
		{	
			FrameTime = (u16)(72+24+(FrameLength*8/(DataRate/10)));
		}
		if( ( FrameLength*8 % (DataRate/10) ) != 0 ) 
				FrameTime ++;
	} else {	
		N_DBPS = N_DBPSOfRate(DataRate);
		Ceiling = (16 + 8*FrameLength + 6) / N_DBPS
				+ (((16 + 8*FrameLength + 6) % N_DBPS) ? 1 : 0);
		FrameTime = (u16)(16 + 4 + 4*Ceiling + 6);
	}
	return FrameTime;
}

u16 N_DBPSOfRate(u16 DataRate)
{
	 u16 N_DBPS = 24;

	 switch(DataRate)
	 {
	 case 60:
	  N_DBPS = 24;
	  break;

	 case 90:
	  N_DBPS = 36;
	  break;

	 case 120:
	  N_DBPS = 48;
	  break;

	 case 180:
	  N_DBPS = 72;
	  break;

	 case 240:
	  N_DBPS = 96;
	  break;

	 case 360:
	  N_DBPS = 144;
	  break;

	 case 480:
	  N_DBPS = 192;
	  break;

	 case 540:
	  N_DBPS = 216;
	  break;

	 default:
	  break;
	 }

	 return N_DBPS;
}

void rtl819xU_cmd_isr(struct urb *tx_cmd_urb, struct pt_regs *regs)
{
	usb_free_urb(tx_cmd_urb);
}

unsigned int txqueue2outpipe(struct r8192_priv* priv,unsigned int tx_queue) {

	if(tx_queue >= 9)
	{
		RT_TRACE(COMP_ERR,"%s():Unknown queue ID!!!\n",__FUNCTION__);
		return 0x04;
	}
	return priv->txqueue_to_outpipemap[tx_queue];
}

short rtl8192SU_tx_cmd(struct net_device *dev, struct sk_buff *skb)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	int			status;
	struct urb		*tx_urb;
	unsigned int 		idx_pipe;
	tx_desc_cmd_819x_usb *pdesc = (tx_desc_cmd_819x_usb *)skb->data;
	cb_desc *tcb_desc = (cb_desc *)(skb->cb + MAX_DEV_ADDR_SIZE);
	u8 queue_index = tcb_desc->queue_index;
	u32			PktSize = 0;

	
	atomic_inc(&priv->tx_pending[queue_index]);

	tx_urb = usb_alloc_urb(0,GFP_ATOMIC);
	if(!tx_urb){
		dev_kfree_skb(skb);
		return -ENOMEM;
	}

	memset(pdesc, 0, USB_HWDESC_HEADER_LEN);

	
	pdesc->LINIP = tcb_desc->bLastIniPkt;
	PktSize = (u16)(skb->len - USB_HWDESC_HEADER_LEN);
	pdesc->PktSize = PktSize;
	
	
	
	
	
	idx_pipe = txqueue2outpipe(priv,queue_index);
	

	usb_fill_bulk_urb(tx_urb,
	                            priv->udev,
	                            usb_sndbulkpipe(priv->udev,priv->RtOutPipes[idx_pipe]),
	                            skb->data,
	                            skb->len,
	                            rtl8192_tx_isr,
	                            skb);

	status = usb_submit_urb(tx_urb, GFP_ATOMIC);
	if (!status){
		return 0;
	}else{
		printk("Error TX CMD URB, error %d",
				status);
		return -1;
	}
}


u8 MapHwQueueToFirmwareQueue(u8 QueueID)
{
	u8 QueueSelect = 0x0;       

	switch(QueueID) {
		case BE_QUEUE:
			QueueSelect = QSLT_BE;  
			break;

		case BK_QUEUE:
			QueueSelect = QSLT_BK;  
			break;

		case VO_QUEUE:
			QueueSelect = QSLT_VO;  
			break;

		case VI_QUEUE:
			QueueSelect = QSLT_VI;  
			break;
		case MGNT_QUEUE:
			QueueSelect = QSLT_MGNT;
			break;

		case BEACON_QUEUE:
			QueueSelect = QSLT_BEACON;
			break;

			
			

		case TXCMD_QUEUE:
			QueueSelect = QSLT_CMD;
			break;

		case HIGH_QUEUE:
			QueueSelect = QSLT_HIGH;
			break;

		default:
			RT_TRACE(COMP_ERR, "TransmitTCB(): Impossible Queue Selection: %d \n", QueueID);
			break;
	}
	return QueueSelect;
}

u8 MRateToHwRate8190Pci(u8 rate)
{
	u8	ret = DESC92S_RATE1M;

	switch(rate)
	{
		
	case MGN_1M:		ret = DESC92S_RATE1M;	break;
	case MGN_2M:		ret = DESC92S_RATE2M;	break;
	case MGN_5_5M:		ret = DESC92S_RATE5_5M;	break;
	case MGN_11M:		ret = DESC92S_RATE11M;	break;
	case MGN_6M:		ret = DESC92S_RATE6M;	break;
	case MGN_9M:		ret = DESC92S_RATE9M;	break;
	case MGN_12M:		ret = DESC92S_RATE12M;	break;
	case MGN_18M:		ret = DESC92S_RATE18M;	break;
	case MGN_24M:		ret = DESC92S_RATE24M;	break;
	case MGN_36M:		ret = DESC92S_RATE36M;	break;
	case MGN_48M:		ret = DESC92S_RATE48M;	break;
	case MGN_54M:		ret = DESC92S_RATE54M;	break;

		
	case MGN_MCS0:		ret = DESC92S_RATEMCS0;	break;
	case MGN_MCS1:		ret = DESC92S_RATEMCS1;	break;
	case MGN_MCS2:		ret = DESC92S_RATEMCS2;	break;
	case MGN_MCS3:		ret = DESC92S_RATEMCS3;	break;
	case MGN_MCS4:		ret = DESC92S_RATEMCS4;	break;
	case MGN_MCS5:		ret = DESC92S_RATEMCS5;	break;
	case MGN_MCS6:		ret = DESC92S_RATEMCS6;	break;
	case MGN_MCS7:		ret = DESC92S_RATEMCS7;	break;
	case MGN_MCS8:		ret = DESC92S_RATEMCS8;	break;
	case MGN_MCS9:		ret = DESC92S_RATEMCS9;	break;
	case MGN_MCS10:	ret = DESC92S_RATEMCS10;	break;
	case MGN_MCS11:	ret = DESC92S_RATEMCS11;	break;
	case MGN_MCS12:	ret = DESC92S_RATEMCS12;	break;
	case MGN_MCS13:	ret = DESC92S_RATEMCS13;	break;
	case MGN_MCS14:	ret = DESC92S_RATEMCS14;	break;
	case MGN_MCS15:	ret = DESC92S_RATEMCS15;	break;

	
	case MGN_MCS0_SG:
	case MGN_MCS1_SG:
	case MGN_MCS2_SG:
	case MGN_MCS3_SG:
	case MGN_MCS4_SG:
	case MGN_MCS5_SG:
	case MGN_MCS6_SG:
	case MGN_MCS7_SG:
	case MGN_MCS8_SG:
	case MGN_MCS9_SG:
	case MGN_MCS10_SG:
	case MGN_MCS11_SG:
	case MGN_MCS12_SG:
	case MGN_MCS13_SG:
	case MGN_MCS14_SG:
	case MGN_MCS15_SG:
	{
		ret = DESC92S_RATEMCS15_SG;
		break;
	}

	default:		break;
	}
	return ret;
}

u8 QueryIsShort(u8 TxHT, u8 TxRate, cb_desc *tcb_desc)
{
	u8   tmp_Short;

	tmp_Short = (TxHT==1)?((tcb_desc->bUseShortGI)?1:0):((tcb_desc->bUseShortPreamble)?1:0);

	if(TxHT==1 && TxRate != DESC90_RATEMCS15)
		tmp_Short = 0;

	return tmp_Short;
}

static void tx_zero_isr(struct urb *tx_urb)
{
	return;
}



 








short rtl8192SU_tx(struct net_device *dev, struct sk_buff* skb)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	cb_desc *tcb_desc = (cb_desc *)(skb->cb + MAX_DEV_ADDR_SIZE);
	tx_desc_819x_usb *tx_desc = (tx_desc_819x_usb *)skb->data;
	
	struct usb_device *udev = priv->udev;
	int pend;
	int status;
	struct urb *tx_urb = NULL, *tx_urb_zero = NULL;
	
	unsigned int idx_pipe;
	u16		MPDUOverhead = 0;
 	

	pend = atomic_read(&priv->tx_pending[tcb_desc->queue_index]);
	
	if( pend > MAX_TX_URB){
		switch (tcb_desc->queue_index) {
			case VO_PRIORITY:
				priv->stats.txvodrop++;
				break;
			case VI_PRIORITY:
				priv->stats.txvidrop++;
				break;
			case BE_PRIORITY:
				priv->stats.txbedrop++;
				break;
			default:
				priv->stats.txbkdrop++;
				break;
		}
		printk("To discard skb packet!\n");
		dev_kfree_skb_any(skb);
		return -1;
	}

	tx_urb = usb_alloc_urb(0,GFP_ATOMIC);
	if(!tx_urb){
		dev_kfree_skb_any(skb);
		return -ENOMEM;
	}

	memset(tx_desc, 0, sizeof(tx_desc_819x_usb));


		tx_desc->NonQos = (IsQoSDataFrame(skb->data)==TRUE)? 0:1;

	
	

	
	
	tx_desc->TxHT = (tcb_desc->data_rate&0x80)?1:0;


	tx_desc->TxRate = MRateToHwRate8190Pci(tcb_desc->data_rate);
	
	tx_desc->TxShort = QueryIsShort(tx_desc->TxHT, tx_desc->TxRate, tcb_desc);


	
	if(tcb_desc->bAMPDUEnable) {
		tx_desc->AllowAggregation = 1;
		
		
		
	} else {
		tx_desc->AllowAggregation = 0;
		
		
		
	}

	
	
	
	
	
	{
		u8	*pSeq;
		u16	Temp;
		
		pSeq = (u8 *)(skb->data+USB_HWDESC_HEADER_LEN + 22);
		Temp = pSeq[0];
		Temp <<= 12;
		Temp |= (*(u16 *)pSeq)>>4;
		tx_desc->Seq = Temp;
	}

	
	tx_desc->RTSEn = (tcb_desc->bRTSEnable)?1:0;
	tx_desc->CTS2Self = (tcb_desc->bCTSEnable)?1:0;
	tx_desc->RTSSTBC = (tcb_desc->bRTSSTBC)?1:0;
	tx_desc->RTSHT = (tcb_desc->rts_rate&0x80)?1:0;
	tx_desc->RTSRate =  MRateToHwRate8190Pci((u8)tcb_desc->rts_rate);
	tx_desc->RTSSubcarrier = (tx_desc->RTSHT==0)?(tcb_desc->RTSSC):0;
	tx_desc->RTSBW = (tx_desc->RTSHT==1)?((tcb_desc->bRTSBW)?1:0):0;
	tx_desc->RTSShort = (tx_desc->RTSHT==0)?(tcb_desc->bRTSUseShortPreamble?1:0):\
				(tcb_desc->bRTSUseShortGI?1:0);
	
	tx_desc->DisRTSFB = 0;
	tx_desc->RTSRateFBLmt = 0xf;

	
	

	
	if(priv->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40)
	{
		if(tcb_desc->bPacketBW) {
			tx_desc->TxBandwidth = 1;
			tx_desc->TxSubCarrier = 0;    
		} else {
			tx_desc->TxBandwidth = 0;
			tx_desc->TxSubCarrier = priv->nCur40MhzPrimeSC;
		}
	} else {
		tx_desc->TxBandwidth = 0;
		tx_desc->TxSubCarrier = 0;
	}


	
	
        tx_desc->LINIP = 0;
        
        tx_desc->Offset =  USB_HWDESC_HEADER_LEN;

	{
		tx_desc->PktSize = (skb->len - USB_HWDESC_HEADER_LEN) & 0xffff;
	}

	
	
	tx_desc->RaBRSRID= tcb_desc->RATRIndex;


	{
		MPDUOverhead = 0;
		
	}

	tx_desc->SecType = 0x0;

		if (tcb_desc->bHwSec)
			{
				switch (priv->ieee80211->pairwise_key_type)
				{
					case KEY_TYPE_WEP40:
					case KEY_TYPE_WEP104:
						 tx_desc->SecType = 0x1;
						 
						 break;
					case KEY_TYPE_TKIP:
						 tx_desc->SecType = 0x2;
						 
						 break;
					case KEY_TYPE_CCMP:
						 tx_desc->SecType = 0x3;
						 
						 break;
					case KEY_TYPE_NA:
						 tx_desc->SecType = 0x0;
						 
						 break;
					default:
						 tx_desc->SecType = 0x0;
						 
						 break;
				}
			}

	


	tx_desc->USERATE = tcb_desc->bTxUseDriverAssingedRate;
	tx_desc->DISFB = tcb_desc->bTxDisableRateFallBack;
	tx_desc->DataRateFBLmt = 0x1F;

	tx_desc->QueueSelect = MapHwQueueToFirmwareQueue(tcb_desc->queue_index);


        
        
        tx_desc->FirstSeg = 1;
        tx_desc->LastSeg = 1;
        tx_desc->OWN = 1;

	{
		
		
		tx_desc->TxBufferSize = (u32)(skb->len);
	}

	
	idx_pipe = txqueue2outpipe(priv,tcb_desc->queue_index);
	

	
	

	
	usb_fill_bulk_urb(tx_urb,
				    udev,
				    usb_sndbulkpipe(udev,priv->RtOutPipes[idx_pipe]),
				    skb->data,
				    skb->len, rtl8192_tx_isr, skb);

	status = usb_submit_urb(tx_urb, GFP_ATOMIC);
	if (!status){

		bool bSend0Byte = false;
		u8 zero = 0;
		if(udev->speed == USB_SPEED_HIGH)
		{
			if (skb->len > 0 && skb->len % 512 == 0)
				bSend0Byte = true;
		}
		else
		{
			if (skb->len > 0 && skb->len % 64 == 0)
				bSend0Byte = true;
		}
		if (bSend0Byte)
		{
#if 1
			tx_urb_zero = usb_alloc_urb(0,GFP_ATOMIC);
			if(!tx_urb_zero){
				RT_TRACE(COMP_ERR, "can't alloc urb for zero byte\n");
				return -ENOMEM;
			}
			usb_fill_bulk_urb(tx_urb_zero,udev,
					usb_sndbulkpipe(udev,idx_pipe), &zero,
					0, tx_zero_isr, dev);
			status = usb_submit_urb(tx_urb_zero, GFP_ATOMIC);
			if (status){
			RT_TRACE(COMP_ERR, "Error TX URB for zero byte %d, error %d", atomic_read(&priv->tx_pending[tcb_desc->queue_index]), status);
			return -1;
			}
#endif
		}
		dev->trans_start = jiffies;
		atomic_inc(&priv->tx_pending[tcb_desc->queue_index]);
		return 0;
	}else{
		RT_TRACE(COMP_ERR, "Error TX URB %d, error %d", atomic_read(&priv->tx_pending[tcb_desc->queue_index]),
				status);
		return -1;
	}
}

void rtl8192SU_net_update(struct net_device *dev)
{

	struct r8192_priv *priv = ieee80211_priv(dev);
	struct ieee80211_device* ieee = priv->ieee80211;
	struct ieee80211_network *net = &priv->ieee80211->current_network;
	
	u16 rate_config = 0;
	u32 regTmp = 0;
	u8 rateIndex = 0;
	u8	retrylimit = 0x30;
	u16 cap = net->capability;

	priv->short_preamble = cap & WLAN_CAPABILITY_SHORT_PREAMBLE;


	
	rtl8192_config_rate(dev, &rate_config);	

	priv->basic_rate = rate_config  = rate_config & 0x15f;

	
	write_nic_byte(dev, RRSR, rate_config&0xff);
	write_nic_byte(dev, RRSR+1, (rate_config>>8)&0xff);

	
	while(rate_config > 0x1)
	{
		rate_config = (rate_config>> 1);
		rateIndex++;
	}
	write_nic_byte(dev, INIRTSMCS_SEL, rateIndex);


	
	regTmp = (priv->nCur40MhzPrimeSC) << 5;
	if (priv->short_preamble)
		regTmp |= 0x80;
	write_nic_byte(dev, RRSR+2, regTmp);

	write_nic_dword(dev,BSSIDR,((u32*)net->bssid)[0]);
	write_nic_word(dev,BSSIDR+4,((u16*)net->bssid)[2]);

	write_nic_word(dev, BCN_INTERVAL, net->beacon_interval);
	
	PHY_SetBeaconHwReg( dev, net->beacon_interval);

	rtl8192_update_cap(dev, cap);

	if (ieee->iw_mode == IW_MODE_ADHOC){
		retrylimit = 7;
		
		if (0){
			priv->irq_mask |= (IMR_BcnInt | IMR_BcnInt | IMR_TBDOK | IMR_TBDER);
			
			
		}
	}
	else{
		if (0){
			priv->irq_mask &= ~(IMR_BcnInt | IMR_BcnInt | IMR_TBDOK | IMR_TBDER);
			
			
		}
	}

	priv->ShortRetryLimit = priv->LongRetryLimit = retrylimit;

	write_nic_word(dev, 	RETRY_LIMIT,
				retrylimit << RETRY_LIMIT_SHORT_SHIFT | \
				retrylimit << RETRY_LIMIT_LONG_SHIFT);
}

void rtl8192SU_update_ratr_table(struct net_device* dev)
{
		struct r8192_priv* priv = ieee80211_priv(dev);
	struct ieee80211_device* ieee = priv->ieee80211;
	u8* pMcsRate = ieee->dot11HTOperationalRateSet;
	
	u32 ratr_value = 0;

	u8 rate_index = 0;
	int WirelessMode = ieee->mode;
	u8 MimoPs = ieee->pHTInfo->PeerMimoPs;

	u8 bNMode = 0;

	rtl8192_config_rate(dev, (u16*)(&ratr_value));
	ratr_value |= (*(u16*)(pMcsRate)) << 12;

	
	switch (WirelessMode)
	{
		case IEEE_A:
			ratr_value &= 0x00000FF0;
			break;
		case IEEE_B:
			ratr_value &= 0x0000000D;
			break;
		case IEEE_G:
			ratr_value &= 0x00000FF5;
			break;
		case IEEE_N_24G:
		case IEEE_N_5G:
		{
			bNMode = 1;

			if (MimoPs == 0) 
					{
				ratr_value &= 0x0007F005;
			}
			else
			{	
				u32	ratr_mask;

				
				if (	priv->rf_type == RF_1T2R ||
					priv->rf_type == RF_1T1R ||
					(ieee->pHTInfo->IOTAction & HT_IOT_ACT_DISABLE_TX_2SS) )
						ratr_mask = 0x000ff005;
					else
						ratr_mask = 0x0f0ff005;

				if((ieee->pHTInfo->bCurTxBW40MHz) &&
				    !(ieee->pHTInfo->IOTAction & HT_IOT_ACT_DISABLE_TX_40_MHZ))
					ratr_mask |= 0x00000010; 

				
					ratr_value &= ratr_mask;
					}
			}
			break;
		default:
			if(0)
			{
				if(priv->rf_type == RF_1T2R)	
				{
				ratr_value &= 0x000ff0f5;
				}
				else
				{
				ratr_value &= 0x0f0ff0f5;
				}
			}
			
			break;
	}

	ratr_value &= 0x0FFFFFFF;

	
	if (   (bNMode && ((ieee->pHTInfo->IOTAction & HT_IOT_ACT_DISABLE_SHORT_GI)==0)) &&
		((ieee->pHTInfo->bCurBW40MHz && ieee->pHTInfo->bCurShortGI40MHz) ||
	        (!ieee->pHTInfo->bCurBW40MHz && ieee->pHTInfo->bCurShortGI20MHz)))
	{
		u8 shortGI_rate = 0;
		u32 tmp_ratr_value = 0;
		ratr_value |= 0x10000000;
		tmp_ratr_value = (ratr_value>>12);
		for(shortGI_rate=15; shortGI_rate>0; shortGI_rate--)
		{
			if((1<<shortGI_rate) & tmp_ratr_value)
				break;
		}
		shortGI_rate = (shortGI_rate<<12)|(shortGI_rate<<8)|(shortGI_rate<<4)|(shortGI_rate);
		write_nic_byte(dev, SG_RATE, shortGI_rate);
		
	}
	write_nic_dword(dev, ARFR0+rate_index*4, ratr_value);
	printk("=============>ARFR0+rate_index*4:%#x\n", ratr_value);

	
	if (ratr_value & 0xfffff000){
		
		HalSetFwCmd8192S(dev, FW_CMD_RA_REFRESH_N);
	}
	else	{
		
		HalSetFwCmd8192S(dev, FW_CMD_RA_REFRESH_BG);
	}
}

void rtl8192SU_link_change(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	struct ieee80211_device* ieee = priv->ieee80211;
	
	u32 reg = 0;

	printk("=====>%s 1\n", __func__);
	reg = read_nic_dword(dev, RCR);

	if (ieee->state == IEEE80211_LINKED)
	{

		rtl8192SU_net_update(dev);
		rtl8192SU_update_ratr_table(dev);
		ieee->SetFwCmdHandler(dev, FW_CMD_HIGH_PWR_ENABLE);
		priv->ReceiveConfig = reg |= RCR_CBSSID;

	}else{
		priv->ReceiveConfig = reg &= ~RCR_CBSSID;

	}

	write_nic_dword(dev, RCR, reg);
	rtl8192_update_msr(dev);

	printk("<=====%s 2\n", __func__);
}

static struct ieee80211_qos_parameters def_qos_parameters = {
        {3,3,3,3},
        {7,7,7,7},
        {2,2,2,2},
        {0,0,0,0},
        {0,0,0,0} 
};


void rtl8192_update_beacon(struct work_struct * work)
{
        struct r8192_priv *priv = container_of(work, struct r8192_priv, update_beacon_wq.work);
        struct net_device *dev = priv->ieee80211->dev;
 	struct ieee80211_device* ieee = priv->ieee80211;
	struct ieee80211_network* net = &ieee->current_network;

	if (ieee->pHTInfo->bCurrentHTSupport)
		HTUpdateSelfAndPeerSetting(ieee, net);
	ieee->pHTInfo->bCurrentRT2RTLongSlotTime = net->bssht.bdRT2RTLongSlotTime;
	
	ieee->pHTInfo->RT2RT_HT_Mode = net->bssht.RT2RT_HT_Mode;
	rtl8192_update_cap(dev, net->capability);
}

int WDCAPARA_ADD[] = {EDCAPARA_BE,EDCAPARA_BK,EDCAPARA_VI,EDCAPARA_VO};

void rtl8192_qos_activate(struct work_struct * work)
{
        struct r8192_priv *priv = container_of(work, struct r8192_priv, qos_activate);
        struct net_device *dev = priv->ieee80211->dev;
        struct ieee80211_qos_parameters *qos_parameters = &priv->ieee80211->current_network.qos_data.parameters;
        u8 mode = priv->ieee80211->current_network.mode;
        
	u8  u1bAIFS;
	u32 u4bAcParam;
        int i;

        if (priv == NULL)
                return;

       mutex_lock(&priv->mutex);

        if(priv->ieee80211->state != IEEE80211_LINKED)
		goto success;
	RT_TRACE(COMP_QOS,"qos active process with associate response received\n");
	
	
	
	for(i = 0; i <  QOS_QUEUE_NUM; i++) {
		
		u1bAIFS = qos_parameters->aifs[i] * ((mode&(IEEE_G|IEEE_N_24G)) ?9:20) + aSifsTime;
		u4bAcParam = ((((u32)(qos_parameters->tx_op_limit[i]))<< AC_PARAM_TXOP_LIMIT_OFFSET)|
				(((u32)(qos_parameters->cw_max[i]))<< AC_PARAM_ECW_MAX_OFFSET)|
				(((u32)(qos_parameters->cw_min[i]))<< AC_PARAM_ECW_MIN_OFFSET)|
				((u32)u1bAIFS << AC_PARAM_AIFS_OFFSET));

		write_nic_dword(dev, WDCAPARA_ADD[i], u4bAcParam);
		
	}

success:
       mutex_unlock(&priv->mutex);
}

static int rtl8192_qos_handle_probe_response(struct r8192_priv *priv,
		int active_network,
		struct ieee80211_network *network)
{
	int ret = 0;
	u32 size = sizeof(struct ieee80211_qos_parameters);

	if(priv->ieee80211->state !=IEEE80211_LINKED)
                return ret;

        if ((priv->ieee80211->iw_mode != IW_MODE_INFRA))
                return ret;

	if (network->flags & NETWORK_HAS_QOS_MASK) {
		if (active_network &&
				(network->flags & NETWORK_HAS_QOS_PARAMETERS))
			network->qos_data.active = network->qos_data.supported;

		if ((network->qos_data.active == 1) && (active_network == 1) &&
				(network->flags & NETWORK_HAS_QOS_PARAMETERS) &&
				(network->qos_data.old_param_count !=
				 network->qos_data.param_count)) {
			network->qos_data.old_param_count =
				network->qos_data.param_count;
			queue_work(priv->priv_wq, &priv->qos_activate);
			RT_TRACE (COMP_QOS, "QoS parameters change call "
					"qos_activate\n");
		}
	} else {
		memcpy(&priv->ieee80211->current_network.qos_data.parameters,\
		       &def_qos_parameters, size);

		if ((network->qos_data.active == 1) && (active_network == 1)) {
			queue_work(priv->priv_wq, &priv->qos_activate);
			RT_TRACE(COMP_QOS, "QoS was disabled call qos_activate \n");
		}
		network->qos_data.active = 0;
		network->qos_data.supported = 0;
	}

	return 0;
}


static int rtl8192_handle_beacon(struct net_device * dev,
				 struct ieee80211_probe_response *beacon,
				 struct ieee80211_network *network)
{
	struct r8192_priv *priv = ieee80211_priv(dev);

	rtl8192_qos_handle_probe_response(priv,1,network);
	queue_delayed_work(priv->priv_wq, &priv->update_beacon_wq, 0);

	return 0;

}


static int rtl8192_qos_association_resp(struct r8192_priv *priv,
                                    struct ieee80211_network *network)
{
        int ret = 0;
        unsigned long flags;
        u32 size = sizeof(struct ieee80211_qos_parameters);
        int set_qos_param = 0;

        if ((priv == NULL) || (network == NULL))
                return ret;

	if(priv->ieee80211->state !=IEEE80211_LINKED)
                return ret;

        if ((priv->ieee80211->iw_mode != IW_MODE_INFRA))
                return ret;

        spin_lock_irqsave(&priv->ieee80211->lock, flags);
	if(network->flags & NETWORK_HAS_QOS_PARAMETERS) {
		memcpy(&priv->ieee80211->current_network.qos_data.parameters,\
			 &network->qos_data.parameters,\
			sizeof(struct ieee80211_qos_parameters));
		priv->ieee80211->current_network.qos_data.active = 1;
		 {
                        set_qos_param = 1;
			
			priv->ieee80211->current_network.qos_data.old_param_count = \
				 priv->ieee80211->current_network.qos_data.param_count;
			priv->ieee80211->current_network.qos_data.param_count = \
			     	 network->qos_data.param_count;
		}
        } else {
		memcpy(&priv->ieee80211->current_network.qos_data.parameters,\
		       &def_qos_parameters, size);
		priv->ieee80211->current_network.qos_data.active = 0;
		priv->ieee80211->current_network.qos_data.supported = 0;
                set_qos_param = 1;
        }

        spin_unlock_irqrestore(&priv->ieee80211->lock, flags);

	RT_TRACE(COMP_QOS, "%s: network->flags = %d,%d\n",__FUNCTION__,network->flags ,priv->ieee80211->current_network.qos_data.active);
	if (set_qos_param == 1)
		queue_work(priv->priv_wq, &priv->qos_activate);

        return ret;
}


static int rtl8192_handle_assoc_response(struct net_device *dev,
                                     struct ieee80211_assoc_response_frame *resp,
                                     struct ieee80211_network *network)
{
        struct r8192_priv *priv = ieee80211_priv(dev);
        rtl8192_qos_association_resp(priv, network);
        return 0;
}


void rtl8192_update_ratr_table(struct net_device* dev)
	
	
	
{
	struct r8192_priv* priv = ieee80211_priv(dev);
	struct ieee80211_device* ieee = priv->ieee80211;
	u8* pMcsRate = ieee->dot11HTOperationalRateSet;
	
	u32 ratr_value = 0;
	u8 rate_index = 0;
	rtl8192_config_rate(dev, (u16*)(&ratr_value));
	ratr_value |= (*(u16*)(pMcsRate)) << 12;

	switch (ieee->mode)
	{
		case IEEE_A:
			ratr_value &= 0x00000FF0;
			break;
		case IEEE_B:
			ratr_value &= 0x0000000F;
			break;
		case IEEE_G:
			ratr_value &= 0x00000FF7;
			break;
		case IEEE_N_24G:
		case IEEE_N_5G:
			if (ieee->pHTInfo->PeerMimoPs == 0) 
				ratr_value &= 0x0007F007;
			else{
				if (priv->rf_type == RF_1T2R)
					ratr_value &= 0x000FF007;
				else
					ratr_value &= 0x0F81F007;
			}
			break;
		default:
			break;
	}
	ratr_value &= 0x0FFFFFFF;
	if(ieee->pHTInfo->bCurTxBW40MHz && ieee->pHTInfo->bCurShortGI40MHz){
		ratr_value |= 0x80000000;
	}else if(!ieee->pHTInfo->bCurTxBW40MHz && ieee->pHTInfo->bCurShortGI20MHz){
		ratr_value |= 0x80000000;
	}
	write_nic_dword(dev, RATR0+rate_index*4, ratr_value);
	write_nic_byte(dev, UFWP, 1);
}

static u8 ccmp_ie[4] = {0x00,0x50,0xf2,0x04};
static u8 ccmp_rsn_ie[4] = {0x00, 0x0f, 0xac, 0x04};
bool GetNmodeSupportBySecCfg8192(struct net_device*dev)
{
#if 1
	struct r8192_priv* priv = ieee80211_priv(dev);
	struct ieee80211_device* ieee = priv->ieee80211;
	struct ieee80211_network * network = &ieee->current_network;
        int wpa_ie_len= ieee->wpa_ie_len;
        struct ieee80211_crypt_data* crypt;
        int encrypt;
	return TRUE;

        crypt = ieee->crypt[ieee->tx_keyidx];
	
        encrypt = (network->capability & WLAN_CAPABILITY_PRIVACY) || (ieee->host_encrypt && crypt && crypt->ops && (0 == strcmp(crypt->ops->name,"WEP")));

	
	if(encrypt && (wpa_ie_len == 0)) {
		
		return false;

	} else if((wpa_ie_len != 0)) {
		
		
		if (((ieee->wpa_ie[0] == 0xdd) && (!memcmp(&(ieee->wpa_ie[14]),ccmp_ie,4))) || ((ieee->wpa_ie[0] == 0x30) && (!memcmp(&ieee->wpa_ie[10],ccmp_rsn_ie, 4))))
			return true;
		else
			return false;
	} else {
		return true;
	}

	return true;
#endif
}

bool GetHalfNmodeSupportByAPs819xUsb(struct net_device* dev)
{
	bool			Reval;
	struct r8192_priv* priv = ieee80211_priv(dev);
	struct ieee80211_device* ieee = priv->ieee80211;


	return false;

	if(ieee->bHalfWirelessN24GMode == true)
		Reval = true;
	else
		Reval =  false;

	return Reval;
}

void rtl8192_refresh_supportrate(struct r8192_priv* priv)
{
	struct ieee80211_device* ieee = priv->ieee80211;
	
	if (ieee->mode == WIRELESS_MODE_N_24G || ieee->mode == WIRELESS_MODE_N_5G)
	{
		memcpy(ieee->Regdot11HTOperationalRateSet, ieee->RegHTSuppRateSet, 16);
		
		
	}
	else
		memset(ieee->Regdot11HTOperationalRateSet, 0, 16);
	return;
}

u8 rtl8192_getSupportedWireleeMode(struct net_device*dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	u8 ret = 0;
	switch(priv->rf_chip)
	{
		case RF_8225:
		case RF_8256:
		case RF_PSEUDO_11N:
		case RF_6052:
			ret = (WIRELESS_MODE_N_24G|WIRELESS_MODE_G|WIRELESS_MODE_B);
			break;
		case RF_8258:
			ret = (WIRELESS_MODE_A|WIRELESS_MODE_N_5G);
			break;
		default:
			ret = WIRELESS_MODE_B;
			break;
	}
	return ret;
}
void rtl8192_SetWirelessMode(struct net_device* dev, u8 wireless_mode)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	u8 bSupportMode = rtl8192_getSupportedWireleeMode(dev);

#if 1
	if ((wireless_mode == WIRELESS_MODE_AUTO) || ((wireless_mode&bSupportMode)==0))
	{
		if(bSupportMode & WIRELESS_MODE_N_24G)
		{
			wireless_mode = WIRELESS_MODE_N_24G;
		}
		else if(bSupportMode & WIRELESS_MODE_N_5G)
		{
			wireless_mode = WIRELESS_MODE_N_5G;
		}
		else if((bSupportMode & WIRELESS_MODE_A))
		{
			wireless_mode = WIRELESS_MODE_A;
		}
		else if((bSupportMode & WIRELESS_MODE_G))
		{
			wireless_mode = WIRELESS_MODE_G;
		}
		else if((bSupportMode & WIRELESS_MODE_B))
		{
			wireless_mode = WIRELESS_MODE_B;
		}
		else{
			RT_TRACE(COMP_ERR, "%s(), No valid wireless mode supported, SupportedWirelessMode(%x)!!!\n", __FUNCTION__,bSupportMode);
			wireless_mode = WIRELESS_MODE_B;
		}
	}
#ifdef TO_DO_LIST 
	ActUpdateChannelAccessSetting( pAdapter, pHalData->CurrentWirelessMode, &pAdapter->MgntInfo.Info8185.ChannelAccessSetting );
#endif
	
	
	priv->ieee80211->mode = wireless_mode;

	if ((wireless_mode == WIRELESS_MODE_N_24G) ||  (wireless_mode == WIRELESS_MODE_N_5G))
		priv->ieee80211->pHTInfo->bEnableHT = 1;
	else
		priv->ieee80211->pHTInfo->bEnableHT = 0;
	RT_TRACE(COMP_INIT, "Current Wireless Mode is %x\n", wireless_mode);
	rtl8192_refresh_supportrate(priv);
#endif

}


short rtl8192_is_tx_queue_empty(struct net_device *dev)
{
	int i=0;
	struct r8192_priv *priv = ieee80211_priv(dev);
	
	for (i=0; i<=MGNT_QUEUE; i++)
	{
		if ((i== TXCMD_QUEUE) || (i == HCCA_QUEUE) )
			continue;
		if (atomic_read(&priv->tx_pending[i]))
		{
			printk("===>tx queue is not empty:%d, %d\n", i, atomic_read(&priv->tx_pending[i]));
			return 0;
		}
	}
	return 1;
}

void rtl8192_hw_sleep_down(struct net_device *dev)
{
	RT_TRACE(COMP_POWER, "%s()============>come to sleep down\n", __FUNCTION__);
#ifdef TODO

#endif
}

void rtl8192_hw_sleep_wq (struct work_struct *work)
{



        struct delayed_work *dwork = container_of(work,struct delayed_work,work);
        struct ieee80211_device *ieee = container_of(dwork,struct ieee80211_device,hw_sleep_wq);
        struct net_device *dev = ieee->dev;

	
        rtl8192_hw_sleep_down(dev);
}


void rtl8192_hw_wakeup(struct net_device* dev)
{



	RT_TRACE(COMP_POWER, "%s()============>come to wake up\n", __FUNCTION__);
#ifdef TODO

#endif
	

}

void rtl8192_hw_wakeup_wq (struct work_struct *work)
{



	struct delayed_work *dwork = container_of(work,struct delayed_work,work);
	struct ieee80211_device *ieee = container_of(dwork,struct ieee80211_device,hw_wakeup_wq);
	struct net_device *dev = ieee->dev;

	rtl8192_hw_wakeup(dev);
}

#define MIN_SLEEP_TIME 50
#define MAX_SLEEP_TIME 10000
void rtl8192_hw_to_sleep(struct net_device *dev, u32 th, u32 tl)
{

	struct r8192_priv *priv = ieee80211_priv(dev);

	u32 rb = jiffies;
	unsigned long flags;

	spin_lock_irqsave(&priv->ps_lock,flags);

	
	tl -= MSECS(4+16+7);

	

	

	



	
	if(((tl>=rb)&& (tl-rb) <= MSECS(MIN_SLEEP_TIME))
		||((rb>tl)&& (rb-tl) < MSECS(MIN_SLEEP_TIME))) {
		spin_unlock_irqrestore(&priv->ps_lock,flags);
		printk("too short to sleep\n");
		return;
	}



	{
		u32 tmp = (tl>rb)?(tl-rb):(rb-tl);
	

		queue_delayed_work(priv->ieee80211->wq, &priv->ieee80211->hw_wakeup_wq, tmp); 
	}
	
#if 1
	if(((tl > rb) && ((tl-rb) > MSECS(MAX_SLEEP_TIME)))||
		((tl < rb) && ((rb-tl) > MSECS(MAX_SLEEP_TIME)))) {
		printk("========>too long to sleep:%x, %x, %lx\n", tl, rb,  MSECS(MAX_SLEEP_TIME));
		spin_unlock_irqrestore(&priv->ps_lock,flags);
		return;
	}
#endif



	
	queue_delayed_work(priv->ieee80211->wq, (void *)&priv->ieee80211->hw_sleep_wq,0);

	spin_unlock_irqrestore(&priv->ps_lock,flags);
}

static void rtl8192_init_priv_variable(struct net_device* dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	u8 i;
	priv->card_8192 = NIC_8192U;
	priv->chan = 1; 
	priv->ieee80211->mode = WIRELESS_MODE_AUTO; 
	priv->ieee80211->iw_mode = IW_MODE_INFRA;
	priv->ieee80211->ieee_up=0;
	priv->retry_rts = DEFAULT_RETRY_RTS;
	priv->retry_data = DEFAULT_RETRY_DATA;
	priv->ieee80211->rts = DEFAULT_RTS_THRESHOLD;
	priv->ieee80211->rate = 110; 
	priv->ieee80211->short_slot = 1;
	priv->promisc = (dev->flags & IFF_PROMISC) ? 1:0;
	priv->CckPwEnl = 6;
	
	priv->IrpPendingCount = 1;
	priv->ResetProgress = RESET_TYPE_NORESET;
	priv->bForcedSilentReset = 0;
	priv->bDisableNormalResetCheck = false;
	priv->force_reset = false;

	priv->ieee80211->FwRWRF = 0; 	
	priv->ieee80211->current_network.beacon_interval = DEFAULT_BEACONINTERVAL;
	priv->ieee80211->iw_mode = IW_MODE_INFRA;
	priv->ieee80211->softmac_features  = IEEE_SOFTMAC_SCAN |
		IEEE_SOFTMAC_ASSOCIATE | IEEE_SOFTMAC_PROBERQ |
		IEEE_SOFTMAC_PROBERS | IEEE_SOFTMAC_TX_QUEUE |
		IEEE_SOFTMAC_BEACONS;

	priv->ieee80211->active_scan = 1;
	priv->ieee80211->modulation = IEEE80211_CCK_MODULATION | IEEE80211_OFDM_MODULATION;
	priv->ieee80211->host_encrypt = 1;
	priv->ieee80211->host_decrypt = 1;
	priv->ieee80211->start_send_beacons = NULL;
	priv->ieee80211->stop_send_beacons = NULL;
	priv->ieee80211->softmac_hard_start_xmit = rtl8192_hard_start_xmit;
	priv->ieee80211->set_chan = rtl8192_set_chan;
	priv->ieee80211->link_change = priv->ops->rtl819x_link_change;
	priv->ieee80211->softmac_data_hard_start_xmit = rtl8192_hard_data_xmit;
	priv->ieee80211->data_hard_stop = rtl8192_data_hard_stop;
	priv->ieee80211->data_hard_resume = rtl8192_data_hard_resume;
	priv->ieee80211->init_wmmparam_flag = 0;
	priv->ieee80211->fts = DEFAULT_FRAG_THRESHOLD;
	priv->ieee80211->check_nic_enough_desc = check_nic_enough_desc;
	priv->ieee80211->tx_headroom = TX_PACKET_SHIFT_BYTES;
	priv->ieee80211->qos_support = 1;

	

	priv->ieee80211->SetBWModeHandler = rtl8192_SetBWMode;
	priv->ieee80211->handle_assoc_response = rtl8192_handle_assoc_response;
	priv->ieee80211->handle_beacon = rtl8192_handle_beacon;
	
	priv->ieee80211->sta_wake_up = rtl8192_hw_wakeup;

	priv->ieee80211->enter_sleep_state = rtl8192_hw_to_sleep;
	priv->ieee80211->ps_is_queue_empty = rtl8192_is_tx_queue_empty;
	
	priv->ieee80211->GetNmodeSupportBySecCfg = GetNmodeSupportBySecCfg8192;
	priv->ieee80211->GetHalfNmodeSupportByAPsHandler = GetHalfNmodeSupportByAPs819xUsb;
	priv->ieee80211->SetWirelessMode = rtl8192_SetWirelessMode;
	
	priv->ieee80211->InitialGainHandler = priv->ops->rtl819x_initial_gain;
	priv->card_type = USB;


	priv->ieee80211->current_network.beacon_interval = DEFAULT_BEACONINTERVAL;
	priv->ieee80211->SetFwCmdHandler = HalSetFwCmd8192S;
	priv->bRFSiOrPi = 0;
	
	priv->bInHctTest = false;

	priv->MidHighPwrTHR_L1 = 0x3B;
	priv->MidHighPwrTHR_L2 = 0x40;

	if(priv->bInHctTest)
  	{
		priv->ShortRetryLimit = HAL_RETRY_LIMIT_AP_ADHOC;
		priv->LongRetryLimit = HAL_RETRY_LIMIT_AP_ADHOC;
  	}
	else
	{
		priv->ShortRetryLimit = HAL_RETRY_LIMIT_INFRA;
		priv->LongRetryLimit = HAL_RETRY_LIMIT_INFRA;
	}

	priv->SetFwCmdInProgress = false; 
	priv->CurrentFwCmdIO = 0;

	priv->MinSpaceCfg = 0;

	priv->EarlyRxThreshold = 7;
	priv->enable_gpio0 = 0;
	priv->TransmitConfig	=
				((u32)TCR_MXDMA_2048<<TCR_MXDMA_OFFSET) |	
				(priv->ShortRetryLimit<<TCR_SRL_OFFSET) |	
				(priv->LongRetryLimit<<TCR_LRL_OFFSET) |	
				(false ? TCR_SAT : 0);	
	if(priv->bInHctTest)
		priv->ReceiveConfig	=	
								RCR_AMF | RCR_ADF |	
									RCR_ACF |RCR_APPFCS|						
								RCR_AB | RCR_AM | RCR_APM |		
								RCR_AICV | RCR_ACRC32 | 		
								RCR_APP_PHYST_STAFF | RCR_APP_PHYST_RXFF |	
								((u32)7<<RCR_MXDMA_OFFSET) | 
								(priv->EarlyRxThreshold<<RCR_FIFO_OFFSET) | 
								(priv->EarlyRxThreshold == 7 ? RCR_OnlyErlPkt:0);
	else
		priv->ReceiveConfig	=	
									RCR_AMF | RCR_ADF | RCR_AB |
									RCR_AM | RCR_APM |RCR_AAP |RCR_ADD3|RCR_APP_ICV|
								RCR_APP_PHYST_STAFF | RCR_APP_PHYST_RXFF |	
									RCR_APP_MIC | RCR_APPFCS;

	
	priv->IntrMask 		= 	(u16)(IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
								IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
								IMR_BDOK | IMR_RXCMDOK |  IMR_RDU | IMR_RXFOVW	|			\
								IMR_TXFOVW | IMR_BcnInt | IMR_TBDOK | IMR_TBDER);




	priv->AcmControl = 0;
	priv->pFirmware = (rt_firmware*)vmalloc(sizeof(rt_firmware));
	if (priv->pFirmware)
	memset(priv->pFirmware, 0, sizeof(rt_firmware));

	
        skb_queue_head_init(&priv->rx_queue);
	skb_queue_head_init(&priv->skb_queue);

	
	for(i = 0; i < MAX_QUEUE_SIZE; i++) {
		skb_queue_head_init(&priv->ieee80211->skb_waitQ [i]);
	}
	for(i = 0; i < MAX_QUEUE_SIZE; i++) {
		skb_queue_head_init(&priv->ieee80211->skb_aggQ [i]);
	}
	for(i = 0; i < MAX_QUEUE_SIZE; i++) {
		skb_queue_head_init(&priv->ieee80211->skb_drv_aggQ [i]);
	}
	priv->rf_set_chan = rtl8192_phy_SwChnl;
}


static void rtl8192_init_priv_lock(struct r8192_priv* priv)
{
	spin_lock_init(&priv->tx_lock);
	spin_lock_init(&priv->irq_lock);
	
	sema_init(&priv->wx_sem,1);
	sema_init(&priv->rf_sem,1);
	spin_lock_init(&priv->ps_lock);
	mutex_init(&priv->mutex);
}

extern  void    rtl819x_watchdog_wqcallback(struct work_struct *work);

void rtl8192_irq_rx_tasklet(struct r8192_priv *priv);

#define DRV_NAME "wlan0"
static void rtl8192_init_priv_task(struct net_device* dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);

#ifdef PF_SYNCTHREAD
	priv->priv_wq = create_workqueue(DRV_NAME,0);
#else
	priv->priv_wq = create_workqueue(DRV_NAME);
#endif

	INIT_WORK(&priv->reset_wq, rtl8192_restart);

	
	INIT_DELAYED_WORK(&priv->watch_dog_wq, rtl819x_watchdog_wqcallback);
	INIT_DELAYED_WORK(&priv->txpower_tracking_wq,  dm_txpower_trackingcallback);

	INIT_DELAYED_WORK(&priv->rfpath_check_wq,  dm_rf_pathcheck_workitemcallback);
	INIT_DELAYED_WORK(&priv->update_beacon_wq, rtl8192_update_beacon);
	INIT_DELAYED_WORK(&priv->initialgain_operate_wq, InitialGainOperateWorkItemCallBack);
	
	
	INIT_WORK(&priv->qos_activate, rtl8192_qos_activate);
	INIT_DELAYED_WORK(&priv->ieee80211->hw_wakeup_wq,(void*) rtl8192_hw_wakeup_wq);
	INIT_DELAYED_WORK(&priv->ieee80211->hw_sleep_wq,(void*) rtl8192_hw_sleep_wq);

	tasklet_init(&priv->irq_rx_tasklet,
	     (void(*)(unsigned long))rtl8192_irq_rx_tasklet,
	     (unsigned long)priv);
}

static void rtl8192_get_eeprom_size(struct net_device* dev)
{
	u16 curCR = 0;
	struct r8192_priv *priv = ieee80211_priv(dev);
	RT_TRACE(COMP_EPROM, "===========>%s()\n", __FUNCTION__);
	curCR = read_nic_word_E(dev,EPROM_CMD);
	RT_TRACE(COMP_EPROM, "read from Reg EPROM_CMD(%x):%x\n", EPROM_CMD, curCR);
	
	priv->epromtype = (curCR & Cmd9346CR_9356SEL) ? EPROM_93c56 : EPROM_93c46;
	RT_TRACE(COMP_EPROM, "<===========%s(), epromtype:%d\n", __FUNCTION__, priv->epromtype);
}


static inline u16 endian_swap(u16* data)
{
	u16 tmp = *data;
	*data = (tmp >> 8) | (tmp << 8);
	return *data;
}

u8 rtl8192SU_UsbOptionToEndPointNumber(u8 UsbOption)
{
	u8	nEndPoint = 0;
	switch(UsbOption)
	{
		case 0:
			nEndPoint = 6;
			break;
		case 1:
			nEndPoint = 11;
			break;
		case 2:
			nEndPoint = 4;
			break;
		default:
			RT_TRACE(COMP_INIT, "UsbOptionToEndPointNumber(): Invalid UsbOption(%#x)\n", UsbOption);
			break;
	}
	return nEndPoint;
}

u8 rtl8192SU_BoardTypeToRFtype(struct net_device* dev,  u8 Boardtype)
{
	u8	RFtype = RF_1T2R;

	switch(Boardtype)
	{
		case 0:
			RFtype = RF_1T1R;
			break;
		case 1:
			RFtype = RF_1T2R;
			break;
		case 2:
			RFtype = RF_2T2R;
			break;
		case 3:
			RFtype = RF_2T2R_GREEN;
			break;
		default:
			break;
	}

	return RFtype;
}










void
rtl8192SU_ConfigAdapterInfo8192SForAutoLoadFail(struct net_device* dev)
{
	struct r8192_priv 	*priv = ieee80211_priv(dev);
	
	
	u8		rf_path, index;	
	int	i;

	RT_TRACE(COMP_INIT, "====> ConfigAdapterInfo8192SForAutoLoadFail\n");

	write_nic_byte(dev, SYS_ISO_CTRL+1, 0xE8); 
	
	mdelay(10);
	write_nic_byte(dev, PMC_FSM, 0x02); 

	

	
	priv->eeprom_vid = 0;
	priv->eeprom_pid = 0;
	priv->card_8192_version = 0;
	priv->eeprom_ChannelPlan = 0;
	priv->eeprom_CustomerID = 0;
	priv->eeprom_SubCustomerID = 0;
	priv->bIgnoreDiffRateTxPowerOffset = false;

	RT_TRACE(COMP_INIT, "EEPROM VID = 0x%4x\n", priv->eeprom_vid);
	RT_TRACE(COMP_INIT, "EEPROM PID = 0x%4x\n", priv->eeprom_pid);
	RT_TRACE(COMP_INIT, "EEPROM Customer ID: 0x%2x\n", priv->eeprom_CustomerID);
	RT_TRACE(COMP_INIT, "EEPROM SubCustomer ID: 0x%2x\n", priv->eeprom_SubCustomerID);
	RT_TRACE(COMP_INIT, "EEPROM ChannelPlan = 0x%4x\n", priv->eeprom_ChannelPlan);
	RT_TRACE(COMP_INIT, "IgnoreDiffRateTxPowerOffset = %d\n", priv->bIgnoreDiffRateTxPowerOffset);



	priv->EEPROMUsbOption = EEPROM_USB_Default_OPTIONAL_FUNC;
	RT_TRACE(COMP_INIT, "USB Option = %#x\n", priv->EEPROMUsbOption);

	for(i=0; i<5; i++)
		priv->EEPROMUsbPhyParam[i] = EEPROM_USB_Default_PHY_PARAM;

	

	{
	
		static u8 sMacAddr[6] = {0x00, 0xE0, 0x4C, 0x81, 0x92, 0x00};
		u8	i;

        	

		for(i = 0; i < 6; i++)
			dev->dev_addr[i] = sMacAddr[i];
	}
	
	write_nic_dword(dev, IDR0, ((u32*)dev->dev_addr)[0]);
	write_nic_word(dev, IDR4, ((u16*)(dev->dev_addr + 4))[0]);

	RT_TRACE(COMP_INIT, "ReadAdapterInfo8192SEFuse(), Permanent Address = %02x-%02x-%02x-%02x-%02x-%02x\n",
			dev->dev_addr[0], dev->dev_addr[1],
			dev->dev_addr[2], dev->dev_addr[3],
			dev->dev_addr[4], dev->dev_addr[5]);

	priv->EEPROMBoardType = EEPROM_Default_BoardType;
	priv->rf_type = RF_1T2R; 
	priv->EEPROMTxPowerDiff = EEPROM_Default_PwDiff;
	priv->EEPROMThermalMeter = EEPROM_Default_ThermalMeter;
	priv->EEPROMCrystalCap = EEPROM_Default_CrystalCap;
	priv->EEPROMTxPwrBase = EEPROM_Default_TxPowerBase;
	priv->EEPROMTSSI_A = EEPROM_Default_TSSI;
	priv->EEPROMTSSI_B = EEPROM_Default_TSSI;
	priv->EEPROMTxPwrTkMode = EEPROM_Default_TxPwrTkMode;



	for (rf_path = 0; rf_path < 2; rf_path++)
	{
		for (i = 0; i < 3; i++)
		{
			
			priv->RfCckChnlAreaTxPwr[rf_path][i] =
			priv->RfOfdmChnlAreaTxPwr1T[rf_path][i] =
			priv->RfOfdmChnlAreaTxPwr2T[rf_path][i] =
			(u8)(EEPROM_Default_TxPower & 0xff);
		}
	}

	for (i = 0; i < 3; i++)
	{
		
		
		
		
		
		
	}

	
	for(i=0; i<14; i++)	
		{
		if (i < 3)			
			index = 0;
		else if (i < 9)		
			index = 1;
		else				
			index = 2;

		
		priv->RfTxPwrLevelCck[rf_path][i]  =
		priv->RfCckChnlAreaTxPwr[rf_path][index];
		priv->RfTxPwrLevelOfdm1T[rf_path][i]  =
		priv->RfOfdmChnlAreaTxPwr1T[rf_path][index];
		priv->RfTxPwrLevelOfdm2T[rf_path][i]  =
		priv->RfOfdmChnlAreaTxPwr2T[rf_path][index];
		}

		for(i=0; i<14; i++)
		{
		
		
		
		
		}

	
	
	
	priv->TSSI_13dBm = priv->EEPROMThermalMeter *100;
	priv->LegacyHTTxPowerDiff = priv->EEPROMTxPowerDiff;
	priv->TxPowerDiff = priv->EEPROMTxPowerDiff;
	
	
	priv->CrystalCap = priv->EEPROMCrystalCap;	
	priv->ThermalMeter[0] = priv->EEPROMThermalMeter;
	priv->LedStrategy = SW_LED_MODE0;

	init_rate_adaptive(dev);

	RT_TRACE(COMP_INIT, "<==== ConfigAdapterInfo8192SForAutoLoadFail\n");

}











void
rtl8192SU_ReadAdapterInfo8192SUsb(struct net_device* dev)
{
	struct r8192_priv 	*priv = ieee80211_priv(dev);
	u16			i,usValue;
	u8			tmpU1b, tempval;
	u16			EEPROMId;
	u8			hwinfo[HWSET_MAX_SIZE_92S];
	u8			rf_path, index;	


	RT_TRACE(COMP_INIT, "====> ReadAdapterInfo8192SUsb\n");

	
	
	
	
	tmpU1b = read_nic_byte(dev, EFUSE_TEST+3);
	write_nic_byte(dev, EFUSE_TEST+3, tmpU1b|0x80);
	
	mdelay(10);
	write_nic_byte(dev, EFUSE_TEST+3, (tmpU1b&(~BIT7)));

	
	priv->card_8192_version = (VERSION_8192S)((read_nic_dword(dev, PMC_FSM)>>16)&0xF);
	RT_TRACE(COMP_INIT, "Chip Version ID: 0x%2x\n", priv->card_8192_version);

	switch(priv->card_8192_version)
	{
		case 0:
			RT_TRACE(COMP_INIT, "Chip Version ID: VERSION_8192S_ACUT.\n");
			break;
		case 1:
			RT_TRACE(COMP_INIT, "Chip Version ID: VERSION_8192S_BCUT.\n");
			break;
		case 2:
			RT_TRACE(COMP_INIT, "Chip Version ID: VERSION_8192S_CCUT.\n");
			break;
		default:
			RT_TRACE(COMP_INIT, "Unknown Chip Version!!\n");
			priv->card_8192_version = VERSION_8192S_BCUT;
			break;
	}

	
	if(priv->EepromOrEfuse)
	{	
		write_nic_byte(dev, SYS_ISO_CTRL+1, 0xE8); 
		
		mdelay(10);
		write_nic_byte(dev, PMC_FSM, 0x02); 
		
		for(i = 0; i < HWSET_MAX_SIZE_92S; i += 2)
		{
			usValue = eprom_read(dev, (u16) (i>>1));
			*((u16*)(&hwinfo[i])) = usValue;
		}
	}
	else if (!(priv->EepromOrEfuse))
	{	

		
		
		
		
		
		
		
		
		

		
		
		
		

		
		EFUSE_ShadowMapUpdate(dev);
		memcpy(hwinfo, &priv->EfuseMap[EFUSE_INIT_MAP][0], HWSET_MAX_SIZE_92S);
	}
	else
	{
		RT_TRACE(COMP_INIT, "ReadAdapterInfo8192SUsb(): Invalid boot type!!\n");
	}

	
	
	
	
	
	

	
	
	
	
	
	EEPROMId = *((u16 *)&hwinfo[0]);

	if( EEPROMId != RTL8190_EEPROM_ID )
	{
		RT_TRACE(COMP_INIT, "ID(%#x) is invalid!!\n", EEPROMId);
		priv->bTXPowerDataReadFromEEPORM = FALSE;
		priv->AutoloadFailFlag=TRUE;
	}
	else
	{
		priv->AutoloadFailFlag=FALSE;
		priv->bTXPowerDataReadFromEEPORM = TRUE;
	}
       
	if(!priv->AutoloadFailFlag)
	{
        	
	    	priv->eeprom_vid = *(u16 *)&hwinfo[EEPROM_VID];
		priv->eeprom_pid = *(u16 *)&hwinfo[EEPROM_PID];
		priv->bIgnoreDiffRateTxPowerOffset = false;	


		
		priv->EEPROMVersion = *(u8 *)&hwinfo[EEPROM_Version];
		priv->eeprom_ChannelPlan = *(u8 *)&hwinfo[EEPROM_ChannelPlan];

		
		priv->eeprom_CustomerID = *(u8 *)&hwinfo[EEPROM_CustomID];
		priv->eeprom_SubCustomerID = *(u8 *)&hwinfo[EEPROM_SubCustomID];
	}
	else
	{
		
		
		
		
		
		

		rtl8192SU_ConfigAdapterInfo8192SForAutoLoadFail(dev);
		return;
	}


	RT_TRACE(COMP_INIT, "EEPROM Id = 0x%4x\n", EEPROMId);
	RT_TRACE(COMP_INIT, "EEPROM VID = 0x%4x\n", priv->eeprom_vid);
	RT_TRACE(COMP_INIT, "EEPROM PID = 0x%4x\n", priv->eeprom_pid);
	RT_TRACE(COMP_INIT, "EEPROM Version ID: 0x%2x\n", priv->EEPROMVersion);
	RT_TRACE(COMP_INIT, "EEPROM Customer ID: 0x%2x\n", priv->eeprom_CustomerID);
	RT_TRACE(COMP_INIT, "EEPROM SubCustomer ID: 0x%2x\n", priv->eeprom_SubCustomerID);
	RT_TRACE(COMP_INIT, "EEPROM ChannelPlan = 0x%4x\n", priv->eeprom_ChannelPlan);
	RT_TRACE(COMP_INIT, "bIgnoreDiffRateTxPowerOffset = %d\n", priv->bIgnoreDiffRateTxPowerOffset);


	
	if(!priv->AutoloadFailFlag)
	{
		priv->EEPROMUsbOption = *(u8 *)&hwinfo[EEPROM_USB_OPTIONAL];
	}
	else
	{
		priv->EEPROMUsbOption = EEPROM_USB_Default_OPTIONAL_FUNC;
	}


	priv->EEPROMUsbEndPointNumber = rtl8192SU_UsbOptionToEndPointNumber((priv->EEPROMUsbOption&EEPROM_EP_NUMBER)>>3);

	RT_TRACE(COMP_INIT, "USB Option = %#x\n", priv->EEPROMUsbOption);
	RT_TRACE(COMP_INIT, "EndPoint Number = %#x\n", priv->EEPROMUsbEndPointNumber);

#ifdef TO_DO_LIST
	
	
	
	switch(pHalData->EEPROMCustomerID)
	{
		case EEPROM_CID_ALPHA:
			pMgntInfo->CustomerID = RT_CID_819x_ALPHA;
			break;

		case EEPROM_CID_CAMEO:
			pMgntInfo->CustomerID = RT_CID_819x_CAMEO;
			break;

		case EEPROM_CID_SITECOM:
			pMgntInfo->CustomerID = RT_CID_819x_Sitecom;
			RT_TRACE(COMP_INIT, DBG_LOUD, ("CustomerID = 0x%4x\n", pMgntInfo->CustomerID));

			break;

		case EEPROM_CID_WHQL:
			Adapter->bInHctTest = TRUE;

			pMgntInfo->bSupportTurboMode = FALSE;
			pMgntInfo->bAutoTurboBy8186 = FALSE;

			pMgntInfo->PowerSaveControl.bInactivePs = FALSE;
			pMgntInfo->PowerSaveControl.bIPSModeBackup = FALSE;
			pMgntInfo->PowerSaveControl.bLeisurePs = FALSE;
			pMgntInfo->keepAliveLevel = 0;
			break;

		default:
			pMgntInfo->CustomerID = RT_CID_DEFAULT;
			break;

	}

	
	
	
	switch(pMgntInfo->CustomerID)
	{
		case RT_CID_DEFAULT:
		case RT_CID_819x_ALPHA:
			pHalData->LedStrategy = SW_LED_MODE1;
			pHalData->bRegUseLed = TRUE;
			pHalData->SwLed1.bLedOn = TRUE;
			break;
		case RT_CID_819x_CAMEO:
			pHalData->LedStrategy = SW_LED_MODE1;
			pHalData->bRegUseLed = TRUE;
			break;

		case RT_CID_819x_Sitecom:
			pHalData->LedStrategy = SW_LED_MODE2;
			pHalData->bRegUseLed = TRUE;
			break;

		default:
			pHalData->LedStrategy = SW_LED_MODE0;
			break;
	}
#endif

	
	for(i=0; i<5; i++)
		priv->EEPROMUsbPhyParam[i] = *(u8 *)&hwinfo[EEPROM_USB_PHY_PARA1+i];

	


       
	for(i=0; i<6; i++)
		dev->dev_addr[i] =  *(u8 *)&hwinfo[EEPROM_NODE_ADDRESS_BYTE_0+i];

	
	write_nic_dword(dev, IDR0, ((u32*)dev->dev_addr)[0]);
	write_nic_word(dev, IDR4, ((u16*)(dev->dev_addr + 4))[0]);

	RT_TRACE(COMP_INIT, "ReadAdapterInfo8192SEFuse(), Permanent Address = %02x-%02x-%02x-%02x-%02x-%02x\n",
			dev->dev_addr[0], dev->dev_addr[1],
			dev->dev_addr[2], dev->dev_addr[3],
			dev->dev_addr[4], dev->dev_addr[5]);

	
	
	
	
	
	
	
		priv->EEPROMBoardType = *(u8 *)&hwinfo[EEPROM_BoardType];
		priv->rf_type = rtl8192SU_BoardTypeToRFtype(dev, priv->EEPROMBoardType);
	
	
	
	
	
	

	priv->rf_chip = RF_6052;

	priv->rf_chip = RF_6052;
	RT_TRACE(COMP_INIT, "BoardType = 0x%2x\n", priv->EEPROMBoardType);
	RT_TRACE(COMP_INIT, "RF_Type = 0x%2x\n", priv->rf_type);

	
	
	
	
	
	{
		priv->EEPROMTxPowerDiff = *(u8 *)&hwinfo[EEPROM_PwDiff];
		priv->EEPROMThermalMeter = *(u8 *)&hwinfo[EEPROM_ThermalMeter];
	}
	
	
	
	
	

	RT_TRACE(COMP_INIT, "PwDiff = %#x\n", priv->EEPROMTxPowerDiff);
	RT_TRACE(COMP_INIT, "ThermalMeter = %#x\n", priv->EEPROMThermalMeter);

	
	
	
	
	
	{
		priv->EEPROMCrystalCap = *(u8 *)&hwinfo[EEPROM_CrystalCap];
	}
	
	
	
	

	RT_TRACE(COMP_INIT, "CrystalCap = %#x\n", priv->EEPROMCrystalCap);

	
	
	
	
	{
		priv->EEPROMTxPwrBase = *(u8 *)&hwinfo[EEPROM_TxPowerBase];
	}
	
	
	
	

	RT_TRACE(COMP_INIT, "TxPwrBase = %#x\n", priv->EEPROMTxPwrBase);


	
	
	
	
	{
		priv->EEPROMTSSI_A = *(u8 *)&hwinfo[EEPROM_TSSI_A];
		priv->EEPROMTSSI_B = *(u8 *)&hwinfo[EEPROM_TSSI_B];
	}
	
	
	
	
	

	RT_TRACE(COMP_INIT, "TSSI_A = %#x, TSSI_B = %#x\n", priv->EEPROMTSSI_A, priv->EEPROMTSSI_B);

	
	
	
	
	{
		priv->EEPROMTxPwrTkMode = *(u8 *)&hwinfo[EEPROM_TxPwTkMode];
	}

	RT_TRACE(COMP_INIT, "TxPwrTkMod = %#x\n", priv->EEPROMTxPwrTkMode);


	{
		
		
		
		

		
		
		
		
		{
			
			
			for (rf_path = 0; rf_path < 2; rf_path++)
			{
				for (i = 0; i < 3; i++)
				{
					
					priv->RfCckChnlAreaTxPwr[rf_path][i] =
					hwinfo[EEPROM_TxPwIndex+rf_path*3+i];

					
					priv->RfOfdmChnlAreaTxPwr1T[rf_path][i] =
					hwinfo[EEPROM_TxPwIndex+6+rf_path*3+i];

					
					priv->RfOfdmChnlAreaTxPwr2T[rf_path][i] =
					hwinfo[EEPROM_TxPwIndex+12+rf_path*3+i];
				}
			}

		}

		

		for (rf_path = 0; rf_path < 2; rf_path++)
		{
			for (i = 0; i < 3; i++)
			{
				RT_TRACE((COMP_INIT),  "CCK RF-%d CHan_Area-%d = 0x%x\n",  rf_path, i,
				priv->RfCckChnlAreaTxPwr[rf_path][i]);
				RT_TRACE((COMP_INIT), "OFDM-1T RF-%d CHan_Area-%d = 0x%x\n",  rf_path, i,
				priv->RfOfdmChnlAreaTxPwr1T[rf_path][i]);
				RT_TRACE((COMP_INIT), "OFDM-2T RF-%d CHan_Area-%d = 0x%x\n",  rf_path, i, priv->RfOfdmChnlAreaTxPwr2T[rf_path][i]);
			}

			
			for(i=0; i<14; i++)	
			{
				if (i < 3)			
					index = 0;
				else if (i < 9)		
					index = 1;
				else				
					index = 2;

				
				priv->RfTxPwrLevelCck[rf_path][i]  =
				priv->RfCckChnlAreaTxPwr[rf_path][index];
				priv->RfTxPwrLevelOfdm1T[rf_path][i]  =
				priv->RfOfdmChnlAreaTxPwr1T[rf_path][index];
				priv->RfTxPwrLevelOfdm2T[rf_path][i]  =
				priv->RfOfdmChnlAreaTxPwr2T[rf_path][index];
				if (rf_path == 0)
				{
					priv->TxPowerLevelOFDM24G[i] = priv->RfTxPwrLevelOfdm1T[rf_path][i] ;
					priv->TxPowerLevelCCK[i] = priv->RfTxPwrLevelCck[rf_path][i];
				}
			}

			for(i=0; i<14; i++)
			{
				RT_TRACE((COMP_INIT),
				"Rf-%d TxPwr CH-%d CCK OFDM_1T OFDM_2T= 0x%x/0x%x/0x%x\n",
				rf_path, i, priv->RfTxPwrLevelCck[rf_path][i],
				priv->RfTxPwrLevelOfdm1T[rf_path][i] ,
				priv->RfTxPwrLevelOfdm2T[rf_path][i] );
			}
		}
	}

	
	
	
	for(i=0; i<14; i++)	
	{
		
		if (i < 3)			
			index = 0;
		else if (i < 9)		
			index = 1;
		else				
			index = 2;

		tempval = (*(u8 *)&hwinfo[EEPROM_TX_PWR_HT20_DIFF+index])&0xff;
		priv->TxPwrHt20Diff[RF90_PATH_A][i] = (tempval&0xF);
		priv->TxPwrHt20Diff[RF90_PATH_B][i] = ((tempval>>4)&0xF);

		
		if (i < 3)			
			tempval = (*(u8 *)&hwinfo[EEPROM_TX_PWR_OFDM_DIFF])&0xff;
		else if (i < 9)		
			tempval = (*(u8 *)&hwinfo[EEPROM_PwDiff])&0xff;
		else				
			tempval = (*(u8 *)&hwinfo[EEPROM_TX_PWR_OFDM_DIFF+1])&0xff;

		
		priv->TxPwrLegacyHtDiff[RF90_PATH_A][i] = (tempval&0xF);
		priv->TxPwrLegacyHtDiff[RF90_PATH_B][i] = ((tempval>>4)&0xF);

		
		
		
		
		tempval = (*(u8 *)&hwinfo[EEPROM_TX_PWR_BAND_EDGE])&0xff;
		priv->TxPwrbandEdgeHt40[RF90_PATH_A][0] = (tempval&0xF); 		
		priv->TxPwrbandEdgeHt40[RF90_PATH_A][1] =  ((tempval>>4)&0xF);	
		tempval = (*(u8 *)&hwinfo[EEPROM_TX_PWR_BAND_EDGE+1])&0xff;
		priv->TxPwrbandEdgeHt40[RF90_PATH_B][0] = (tempval&0xF); 		
		priv->TxPwrbandEdgeHt40[RF90_PATH_B][1] =  ((tempval>>4)&0xF);	
		
		tempval = (*(u8 *)&hwinfo[EEPROM_TX_PWR_BAND_EDGE+2])&0xff;
		priv->TxPwrbandEdgeHt20[RF90_PATH_A][0] = (tempval&0xF); 		
		priv->TxPwrbandEdgeHt20[RF90_PATH_A][1] =  ((tempval>>4)&0xF);	
		tempval = (*(u8 *)&hwinfo[EEPROM_TX_PWR_BAND_EDGE+3])&0xff;
		priv->TxPwrbandEdgeHt20[RF90_PATH_B][0] = (tempval&0xF); 		
		priv->TxPwrbandEdgeHt20[RF90_PATH_B][1] =  ((tempval>>4)&0xF);	
		
		tempval = (*(u8 *)&hwinfo[EEPROM_TX_PWR_BAND_EDGE+4])&0xff;
		priv->TxPwrbandEdgeLegacyOfdm[RF90_PATH_A][0] = (tempval&0xF); 		
		priv->TxPwrbandEdgeLegacyOfdm[RF90_PATH_A][1] =  ((tempval>>4)&0xF);	
		tempval = (*(u8 *)&hwinfo[EEPROM_TX_PWR_BAND_EDGE+5])&0xff;
		priv->TxPwrbandEdgeLegacyOfdm[RF90_PATH_B][0] = (tempval&0xF); 		
		priv->TxPwrbandEdgeLegacyOfdm[RF90_PATH_B][1] =  ((tempval>>4)&0xF);	

		priv->TxPwrbandEdgeFlag = (*(u8 *)&hwinfo[TX_PWR_BAND_EDGE_CHK]);
	}

	for(i=0; i<14; i++)
		RT_TRACE(COMP_INIT, "RF-A Ht20 to HT40 Diff[%d] = 0x%x\n", i, priv->TxPwrHt20Diff[RF90_PATH_A][i]);
	for(i=0; i<14; i++)
		RT_TRACE(COMP_INIT,  "RF-A Legacy to Ht40 Diff[%d] = 0x%x\n", i, priv->TxPwrLegacyHtDiff[RF90_PATH_A][i]);
	for(i=0; i<14; i++)
		RT_TRACE(COMP_INIT,  "RF-B Ht20 to HT40 Diff[%d] = 0x%x\n", i, priv->TxPwrHt20Diff[RF90_PATH_B][i]);
	for(i=0; i<14; i++)
		RT_TRACE(COMP_INIT,  "RF-B Legacy to HT40 Diff[%d] = 0x%x\n", i, priv->TxPwrLegacyHtDiff[RF90_PATH_B][i]);
	RT_TRACE(COMP_INIT, "RF-A HT40 band-edge low/high power diff = 0x%x/0x%x\n",
		priv->TxPwrbandEdgeHt40[RF90_PATH_A][0],
		priv->TxPwrbandEdgeHt40[RF90_PATH_A][1]);
	RT_TRACE((COMP_INIT&COMP_DBG), "RF-B HT40 band-edge low/high power diff = 0x%x/0x%x\n",
		priv->TxPwrbandEdgeHt40[RF90_PATH_B][0],
		priv->TxPwrbandEdgeHt40[RF90_PATH_B][1]);

	RT_TRACE((COMP_INIT&COMP_DBG), "RF-A HT20 band-edge low/high power diff = 0x%x/0x%x\n",
		priv->TxPwrbandEdgeHt20[RF90_PATH_A][0],
		priv->TxPwrbandEdgeHt20[RF90_PATH_A][1]);
	RT_TRACE((COMP_INIT&COMP_DBG), "RF-B HT20 band-edge low/high power diff = 0x%x/0x%x\n",
		priv->TxPwrbandEdgeHt20[RF90_PATH_B][0],
		priv->TxPwrbandEdgeHt20[RF90_PATH_B][1]);

	RT_TRACE((COMP_INIT&COMP_DBG), "RF-A OFDM band-edge low/high power diff = 0x%x/0x%x\n",
		priv->TxPwrbandEdgeLegacyOfdm[RF90_PATH_A][0],
		priv->TxPwrbandEdgeLegacyOfdm[RF90_PATH_A][1]);
	RT_TRACE((COMP_INIT&COMP_DBG), "RF-B OFDM band-edge low/high power diff = 0x%x/0x%x\n",
		priv->TxPwrbandEdgeLegacyOfdm[RF90_PATH_B][0],
		priv->TxPwrbandEdgeLegacyOfdm[RF90_PATH_B][1]);
	RT_TRACE((COMP_INIT&COMP_DBG), "Band-edge enable flag = %d\n", priv->TxPwrbandEdgeFlag);

	
	
	
	priv->TSSI_13dBm = priv->EEPROMThermalMeter *100;
	priv->LegacyHTTxPowerDiff = priv->EEPROMTxPowerDiff;
	priv->TxPowerDiff = priv->EEPROMTxPowerDiff;
	
	
	priv->CrystalCap = priv->EEPROMCrystalCap;	
	priv->ThermalMeter[0] = (priv->EEPROMThermalMeter&0x1f);
	priv->LedStrategy = SW_LED_MODE0;

	init_rate_adaptive(dev);

	RT_TRACE(COMP_INIT, "<==== ReadAdapterInfo8192SUsb\n");

	
}












static void rtl8192SU_read_eeprom_info(struct net_device *dev)
{
	struct r8192_priv 	*priv = ieee80211_priv(dev);
	u8			tmpU1b;

	RT_TRACE(COMP_INIT, "====> ReadAdapterInfo8192SUsb\n");

	
	priv->card_8192_version = (VERSION_8192S)((read_nic_dword(dev, PMC_FSM)>>16)&0xF);
	RT_TRACE(COMP_INIT, "Chip Version ID: 0x%2x\n", priv->card_8192_version);

	tmpU1b = read_nic_byte(dev, EPROM_CMD);

	
	if (tmpU1b & CmdEERPOMSEL)
	{
		RT_TRACE(COMP_INIT, "Boot from EEPROM\n");
		priv->EepromOrEfuse = TRUE;
	}
	else
	{
		RT_TRACE(COMP_INIT, "Boot from EFUSE\n");
		priv->EepromOrEfuse = FALSE;
	}

	
	if (tmpU1b & CmdEEPROM_En)
	{
		RT_TRACE(COMP_INIT, "Autoload OK!!\n");
		priv->AutoloadFailFlag=FALSE;
		rtl8192SU_ReadAdapterInfo8192SUsb(dev);
	}
	else
	{ 
		RT_TRACE(COMP_INIT, "AutoLoad Fail reported from CR9346!!\n");
		priv->AutoloadFailFlag=TRUE;
		rtl8192SU_ConfigAdapterInfo8192SForAutoLoadFail(dev);

		
		if(!priv->EepromOrEfuse)
		{
			RT_TRACE(COMP_INIT, "Update shadow map for EFuse future use!!\n");
			EFUSE_ShadowMapUpdate(dev);
		}
	}
#ifdef TO_DO_LIST
	if((priv->RegChannelPlan >= RT_CHANNEL_DOMAIN_MAX) || (pHalData->EEPROMChannelPlan & EEPROM_CHANNEL_PLAN_BY_HW_MASK))
	{
		pMgntInfo->ChannelPlan = HalMapChannelPlan8192S(Adapter, (pHalData->EEPROMChannelPlan & (~(EEPROM_CHANNEL_PLAN_BY_HW_MASK))));
		pMgntInfo->bChnlPlanFromHW = (pHalData->EEPROMChannelPlan & EEPROM_CHANNEL_PLAN_BY_HW_MASK) ? TRUE : FALSE; 
	}
	else
	{
		pMgntInfo->ChannelPlan = (RT_CHANNEL_DOMAIN)pMgntInfo->RegChannelPlan;
	}

	switch(pMgntInfo->ChannelPlan)
	{
		case RT_CHANNEL_DOMAIN_GLOBAL_DOAMIN:
		{
			PRT_DOT11D_INFO	pDot11dInfo = GET_DOT11D_INFO(pMgntInfo);

			pDot11dInfo->bEnabled = TRUE;
		}
		RT_TRACE(COMP_INIT, DBG_LOUD, ("ReadAdapterInfo8187(): Enable dot11d when RT_CHANNEL_DOMAIN_GLOBAL_DOAMIN!\n"));
		break;
	}

	RT_TRACE(COMP_INIT, DBG_LOUD, ("RegChannelPlan(%d) EEPROMChannelPlan(%d)", pMgntInfo->RegChannelPlan, pHalData->EEPROMChannelPlan));
	RT_TRACE(COMP_INIT, DBG_LOUD, ("ChannelPlan = %d\n" , pMgntInfo->ChannelPlan));

	RT_TRACE(COMP_INIT, DBG_LOUD, ("<==== ReadAdapterInfo8192S\n"));
#endif

	RT_TRACE(COMP_INIT, "<==== ReadAdapterInfo8192SUsb\n");

	
}

short rtl8192_get_channel_map(struct net_device * dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	if(priv->ChannelPlan > COUNTRY_CODE_GLOBAL_DOMAIN){
		printk("rtl8180_init:Error channel plan! Set to default.\n");
		priv->ChannelPlan= 0;
	}
	RT_TRACE(COMP_INIT, "Channel plan is %d\n",priv->ChannelPlan);

	rtl819x_set_channel_map(priv->ChannelPlan, priv);
	return 0;
}

short rtl8192_init(struct net_device *dev)
{

	struct r8192_priv *priv = ieee80211_priv(dev);

	rtl8192_init_priv_variable(dev);
	rtl8192_init_priv_lock(priv);
	rtl8192_init_priv_task(dev);
	rtl8192_get_eeprom_size(dev);
	priv->ops->rtl819x_read_eeprom_info(dev);
	rtl8192_get_channel_map(dev);
	init_hal_dm(dev);
	init_timer(&priv->watch_dog_timer);
	priv->watch_dog_timer.data = (unsigned long)dev;
	priv->watch_dog_timer.function = watch_dog_timer_callback;

	
#ifdef DEBUG_EPROM
	dump_eprom(dev);
#endif
	return 0;
}


void rtl8192_hwconfig(struct net_device* dev)
{
	u32 regRATR = 0, regRRSR = 0;
	u8 regBwOpMode = 0, regTmp = 0;
	struct r8192_priv *priv = ieee80211_priv(dev);


	
	switch(priv->ieee80211->mode)
	{
	case WIRELESS_MODE_B:
		regBwOpMode = BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_CCK;
		regRRSR = RATE_ALL_CCK;
		break;
	case WIRELESS_MODE_A:
		regBwOpMode = BW_OPMODE_5G |BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_OFDM_AG;
		regRRSR = RATE_ALL_OFDM_AG;
		break;
	case WIRELESS_MODE_G:
		regBwOpMode = BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		break;
	case WIRELESS_MODE_AUTO:
#ifdef TO_DO_LIST
		if (Adapter->bInHctTest)
		{
		    regBwOpMode = BW_OPMODE_20MHZ;
		    regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		    regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		}
		else
#endif
		{
		    regBwOpMode = BW_OPMODE_20MHZ;
		    regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG | RATE_ALL_OFDM_1SS | RATE_ALL_OFDM_2SS;
		    regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		}
		break;
	case WIRELESS_MODE_N_24G:
		
		
		regBwOpMode = BW_OPMODE_20MHZ;
			regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG | RATE_ALL_OFDM_1SS | RATE_ALL_OFDM_2SS;
			regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		break;
	case WIRELESS_MODE_N_5G:
		regBwOpMode = BW_OPMODE_5G;
		regRATR = RATE_ALL_OFDM_AG | RATE_ALL_OFDM_1SS | RATE_ALL_OFDM_2SS;
		regRRSR = RATE_ALL_OFDM_AG;
		break;
	}

	write_nic_byte(dev, BW_OPMODE, regBwOpMode);
	{
		u32 ratr_value = 0;
		ratr_value = regRATR;
		if (priv->rf_type == RF_1T2R)
		{
			ratr_value &= ~(RATE_ALL_OFDM_2SS);
		}
		write_nic_dword(dev, RATR0, ratr_value);
		write_nic_byte(dev, UFWP, 1);
	}
	regTmp = read_nic_byte(dev, 0x313);
	regRRSR = ((regTmp) << 24) | (regRRSR & 0x00ffffff);
	write_nic_dword(dev, RRSR, regRRSR);

	
	
	
	write_nic_word(dev, RETRY_LIMIT,
			priv->ShortRetryLimit << RETRY_LIMIT_SHORT_SHIFT | \
			priv->LongRetryLimit << RETRY_LIMIT_LONG_SHIFT);
	

	

	

	


}











static void rtl8192SU_MacConfigBeforeFwDownloadASIC(struct net_device *dev)
{
	u8				tmpU1b;


	u8				PollingCnt = 20;

	RT_TRACE(COMP_INIT, "--->MacConfigBeforeFwDownloadASIC()\n");

	

	
	
	
	
	
       tmpU1b = read_nic_byte(dev, SYS_CLKR+1);
       if(tmpU1b & 0x80)
	{
       	tmpU1b &= 0x3f;
              write_nic_byte(dev, SYS_CLKR+1, tmpU1b);
       }
	
	write_nic_byte(dev, RPWM, 0x0);

       tmpU1b = read_nic_byte(dev, SYS_FUNC_EN+1);
       tmpU1b &= 0x73;
       write_nic_byte(dev, SYS_FUNC_EN+1, tmpU1b);
       udelay(1000);

	
	write_nic_byte(dev, SPS0_CTRL+1, 0x53);
	write_nic_byte(dev, SPS0_CTRL, 0x57);

	
	tmpU1b = read_nic_byte(dev, AFE_MISC);
	write_nic_byte(dev, AFE_MISC, (tmpU1b|AFE_BGEN|AFE_MBEN));

	
	tmpU1b = read_nic_byte(dev, LDOA15_CTRL);
	write_nic_byte(dev, LDOA15_CTRL, (tmpU1b|LDA15_EN));

	
	tmpU1b = read_nic_byte(dev, LDOV12D_CTRL);
	write_nic_byte(dev, LDOV12D_CTRL, (tmpU1b|LDV12_EN));

	
	

	

	
	
	

	

	tmpU1b = read_nic_byte(dev, SYS_ISO_CTRL+1);
	write_nic_byte(dev, SYS_ISO_CTRL+1, (tmpU1b|0x08));

	
	tmpU1b = read_nic_byte(dev, SYS_FUNC_EN+1);
	write_nic_byte(dev, SYS_FUNC_EN+1, (tmpU1b|0x20));

	
	tmpU1b = read_nic_byte(dev, SYS_ISO_CTRL+1);
	write_nic_byte(dev, SYS_ISO_CTRL+1, (tmpU1b& 0x68));

	
	tmpU1b = read_nic_byte(dev, AFE_XTAL_CTRL+1);
	write_nic_byte(dev, AFE_XTAL_CTRL+1, (tmpU1b& 0xfb));

	
	tmpU1b = read_nic_byte(dev, AFE_PLL_CTRL);
	write_nic_byte(dev, AFE_PLL_CTRL, (tmpU1b|0x11));

	
	tmpU1b = read_nic_byte(dev, SYS_ISO_CTRL);
	write_nic_byte(dev, SYS_ISO_CTRL, (tmpU1b&0xEE));

	
	write_nic_byte(dev, SYS_CLKR, 0x00);

	
	tmpU1b = read_nic_byte(dev, SYS_CLKR);
	
	write_nic_byte(dev, SYS_CLKR, (tmpU1b|0xa0));

	
	tmpU1b = read_nic_byte(dev, SYS_CLKR+1);
	write_nic_byte(dev, SYS_CLKR+1, (tmpU1b|0x18));

	
	write_nic_byte(dev, PMC_FSM, 0x02);

	
	tmpU1b = read_nic_byte(dev, SYS_FUNC_EN+1);
	write_nic_byte(dev, SYS_FUNC_EN+1, (tmpU1b|0x08));

	
	tmpU1b = read_nic_byte(dev, SYS_FUNC_EN+1);
	write_nic_byte(dev, SYS_FUNC_EN+1, (tmpU1b|0x80));

	
	tmpU1b = read_nic_byte(dev, SYS_CLKR+1);
	write_nic_byte(dev, SYS_CLKR+1, (tmpU1b|0x80)& 0xBF);

	write_nic_byte(dev, CMDR, 0xFC);
	write_nic_byte(dev, CMDR+1, 0x37);

	
	tmpU1b = read_nic_byte_E(dev, 0x5c);
	write_nic_byte_E(dev, 0x5c, (tmpU1b|BIT7));

	 
	tmpU1b = read_nic_byte(dev, SYS_CLKR);
	write_nic_byte(dev, SYS_CLKR, tmpU1b&(~SYS_CPU_CLKSEL));

	
	write_nic_byte_E(dev, 0x1c, 0x80);

	
	
	
	
	
	do
	{
		tmpU1b = read_nic_byte(dev, TCR);
		if((tmpU1b & TXDMA_INIT_VALUE) == TXDMA_INIT_VALUE)
			break;
		
		udelay(5);
	}while(PollingCnt--);	

	if(PollingCnt <= 0 )
	{
		RT_TRACE(COMP_INIT, "MacConfigBeforeFwDownloadASIC(): Polling TXDMA_INIT_VALUE timeout!! Current TCR(%#x)\n", tmpU1b);
		tmpU1b = read_nic_byte(dev, CMDR);
		write_nic_byte(dev, CMDR, tmpU1b&(~TXDMA_EN));
		udelay(2);
		write_nic_byte(dev, CMDR, tmpU1b|TXDMA_EN);
	}


	RT_TRACE(COMP_INIT, "<---MacConfigBeforeFwDownloadASIC()\n");
}











static void rtl8192SU_MacConfigAfterFwDownload(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv((struct net_device *)dev);
	
	
	u16	tmpU2b;
	u8	tmpU1b;


	RT_TRACE(COMP_INIT, "--->MacConfigAfterFwDownload()\n");

	
	tmpU2b = (BBRSTn|BB_GLB_RSTn|SCHEDULE_EN|MACRXEN|MACTXEN|DDMA_EN|
			 FW2HW_EN|RXDMA_EN|TXDMA_EN|HCI_RXDMA_EN|HCI_TXDMA_EN);		
	
	write_nic_word(dev, CMDR, tmpU2b); 

	
	priv->LoopbackMode = RTL8192SU_NO_LOOPBACK; 
	if(priv->LoopbackMode == RTL8192SU_NO_LOOPBACK)
		tmpU1b = LBK_NORMAL;
	else if (priv->LoopbackMode == RTL8192SU_MAC_LOOPBACK )
		tmpU1b = LBK_MAC_DLB;
	else
		RT_TRACE(COMP_INIT, "Serious error: wrong loopback mode setting\n");

	
	write_nic_byte(dev, LBKMD_SEL, tmpU1b);

	
	write_nic_dword(dev, RCR, priv->ReceiveConfig);
	RT_TRACE(COMP_INIT, "MacConfigAfterFwDownload(): Current RCR settings(%#x)\n", priv->ReceiveConfig);


	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

	
	tmpU1b = read_nic_byte_E(dev, 0x5C);
	write_nic_byte_E(dev, 0x5C, tmpU1b|BIT7);

	
	
	if (priv->bBootFromEfuse)
	{
		u8	tempval;

		tempval = read_nic_byte(dev, SYS_ISO_CTRL+1);
		tempval &= 0xFE;
		write_nic_byte(dev, SYS_ISO_CTRL+1, tempval);

		
		
		

		
		

		
		write_nic_byte(dev, EFUSE_CTRL+3, 0x72);
		
		RT_TRACE(COMP_INIT, "EFUSE CONFIG OK\n");
	}


	RT_TRACE(COMP_INIT, "<---MacConfigAfterFwDownload()\n");
}

void rtl8192SU_HwConfigureRTL8192SUsb(struct net_device *dev)
{

	struct r8192_priv *priv = ieee80211_priv(dev);
	u8			regBwOpMode = 0;
	u32			regRATR = 0, regRRSR = 0;
	u8			regTmp = 0;
	u32 			i = 0;

	
	
	
	
	switch(priv->ieee80211->mode)
	{
	case WIRELESS_MODE_B:
		regBwOpMode = BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_CCK;
		regRRSR = RATE_ALL_CCK;
		break;
	case WIRELESS_MODE_A:
		regBwOpMode = BW_OPMODE_5G |BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_OFDM_AG;
		regRRSR = RATE_ALL_OFDM_AG;
		break;
	case WIRELESS_MODE_G:
		regBwOpMode = BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		break;
	case WIRELESS_MODE_AUTO:
		if (priv->bInHctTest)
		{
		    regBwOpMode = BW_OPMODE_20MHZ;
		    regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		    regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		}
		else
		{
		    regBwOpMode = BW_OPMODE_20MHZ;
		    regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG | RATE_ALL_OFDM_1SS | RATE_ALL_OFDM_2SS;
		    regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		}
		break;
	case WIRELESS_MODE_N_24G:
		
		
		regBwOpMode = BW_OPMODE_20MHZ;
			regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG | RATE_ALL_OFDM_1SS | RATE_ALL_OFDM_2SS;
			regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		break;
	case WIRELESS_MODE_N_5G:
		regBwOpMode = BW_OPMODE_5G;
		regRATR = RATE_ALL_OFDM_AG | RATE_ALL_OFDM_1SS | RATE_ALL_OFDM_2SS;
		regRRSR = RATE_ALL_OFDM_AG;
		break;
	}

	
	
	
	
	regTmp = read_nic_byte(dev, INIRTSMCS_SEL);
	regRRSR = ((regRRSR & 0x000fffff)<<8) | regTmp;

	
	
	
	
	
	{	u8 val[4] = {0x0e, 0x0e, 0x0a, 0x0a};
		
		write_nic_byte(dev, SIFS_CCK, val[0]);
		
		write_nic_byte(dev, SIFS_CCK+1, val[1]);

		
		write_nic_byte(dev, SIFS_OFDM, val[2]);
		
		write_nic_byte(dev, SIFS_OFDM+1, val[3]);
	}

	write_nic_dword(dev, INIRTSMCS_SEL, regRRSR);
	write_nic_byte(dev, BW_OPMODE, regBwOpMode);

	
	
	
	

	
	
	
	write_nic_dword(dev, DARFRC, 0x02010000);
	write_nic_dword(dev, DARFRC+4, 0x06050403);
	write_nic_dword(dev, RARFRC, 0x02010000);
	write_nic_dword(dev, RARFRC+4, 0x06050403);

	
	for (i = 0; i < 8; i++)
		write_nic_dword(dev, ARFR0+i*4, 0x1f0ffff0);

	
	
	
	write_nic_byte(dev, AGGLEN_LMT_H, 0x0f);	
	write_nic_dword(dev, AGGLEN_LMT_L, 0xddd77442); 
	write_nic_dword(dev, AGGLEN_LMT_L+4, 0xfffdd772);

	
	write_nic_word(dev, NAV_PROT_LEN, 0x0080);

	
	write_nic_byte(dev, TXOP_STALL_CTRL, 0x00); 

	
	write_nic_byte(dev, MLT, 0x8f);

	
	write_nic_word(dev, SIFS_CCK, 0x0a0a); 
	write_nic_word(dev, SIFS_OFDM, 0x0e0e);

	write_nic_byte(dev, ACK_TIMEOUT, 0x40);

	
	write_nic_byte(dev, CFEND_TH, 0xFF);

	
	
	
	switch(priv->rf_type)
	{
		case RF_1T2R:
		case RF_1T1R:
			RT_TRACE(COMP_INIT, "Initializeadapter: RF_Type%s\n", (priv->rf_type==RF_1T1R? "(1T1R)":"(1T2R)"));
			priv->MinSpaceCfg = (MAX_MSS_DENSITY_1T<<3);
			break;
		case RF_2T2R:
		case RF_2T2R_GREEN:
			RT_TRACE(COMP_INIT, "Initializeadapter:RF_Type(2T2R)\n");
			priv->MinSpaceCfg = (MAX_MSS_DENSITY_2T<<3);
			break;
	}
	write_nic_byte(dev, AMPDU_MIN_SPACE, priv->MinSpaceCfg);

	
	
	
	
	
	
}







bool rtl8192SU_adapter_start(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	
	
	
	bool             				rtStatus = true;
	
	
	u8 fw_download_times = 1;


	RT_TRACE(COMP_INIT, "--->InitializeAdapter8192SUsb()\n");

	


	
	
	
	
	
	
	
	
	
	
	

	
	
	
start:
	rtl8192SU_MacConfigBeforeFwDownloadASIC(dev);

	
	
	
	rtStatus = FirmwareDownload92S(dev);
	if(rtStatus != true)
	{
		if(fw_download_times == 1){
			RT_TRACE(COMP_INIT, "InitializeAdapter8192SUsb(): Download Firmware failed once, Download again!!\n");
			fw_download_times = fw_download_times + 1;
			goto start;
		}else{
			RT_TRACE(COMP_INIT, "InitializeAdapter8192SUsb(): Download Firmware failed twice, end!!\n");
		goto end;
	}
	}
	
	
	
	rtl8192SU_MacConfigAfterFwDownload(dev);

	
	
	

	
	
	
	rtStatus = PHY_MACConfig8192S(dev);
	if(rtStatus != true)
	{
		RT_TRACE(COMP_INIT, "InitializeAdapter8192SUsb(): Fail to configure MAC!!\n");
		goto end;
	}
	if (1){
		int i;
		for (i=0; i<4; i++)
			write_nic_dword(dev,WDCAPARA_ADD[i], 0x5e4322);
		write_nic_byte(dev,AcmHwCtrl, 0x01);
	}


	
	
	

	rtStatus = PHY_BBConfig8192S(dev);
	if(rtStatus != true)
	{
		RT_TRACE(COMP_INIT, "InitializeAdapter8192SUsb(): Fail to configure BB!!\n");
		goto end;
	}

	rtl8192_setBBreg(dev, rFPGA0_AnalogParameter2, 0xff, 0x58);

	
	
	
	
	priv->Rf_Mode = RF_OP_By_SW_3wire;

	
	
	


	write_nic_byte(dev, AFE_XTAL_CTRL+1, 0xDB);

	
	
	if(priv->card_8192_version == VERSION_8192S_ACUT)
		write_nic_byte(dev, SPS1_CTRL+3, (u8)(RF_EN|RF_RSTB|RF_SDMRSTB)); 
	else
		write_nic_byte(dev, RF_CTRL, (u8)(RF_EN|RF_RSTB|RF_SDMRSTB));

	rtStatus = PHY_RFConfig8192S(dev);
	if(rtStatus != true)
	{
		RT_TRACE(COMP_INIT, "InitializeAdapter8192SUsb(): Fail to configure RF!!\n");
		goto end;
	}


	
	rtl8192_setBBreg(dev, rFPGA0_RFMOD, bCCKEn, 0x1);
	rtl8192_setBBreg(dev, rFPGA0_RFMOD, bOFDMEn, 0x1);

	
	
	
	
	if(priv->rf_type == RF_1T1R)
	{
		
		rtl8192_setBBreg(dev, rFPGA0_RFMOD, 0xff000000, 0x03);
		
		
	}


	
	
	
	


	
	

	
	
	
	rtl8192SU_HwConfigureRTL8192SUsb(dev);

	
	
	
	
	write_nic_dword(dev, IDR0, ((u32*)dev->dev_addr)[0]);
	write_nic_word(dev, IDR4, ((u16*)(dev->dev_addr + 4))[0]);
	if(!priv->bInHctTest)
	{
		if(priv->ResetProgress == RESET_TYPE_NORESET)
		{
			
			
			rtl8192_SetWirelessMode(dev, priv->ieee80211->mode);
	        }
	}
	else
	{
		priv->ieee80211->mode = WIRELESS_MODE_G;
	 	rtl8192_SetWirelessMode(dev, WIRELESS_MODE_G);
	}

	
	
	
	
	
	
	
	

	
	CamResetAllEntry(dev);
	
	{
		u8 SECR_value = 0x0;
		SECR_value |= SCR_TxEncEnable;
		SECR_value |= SCR_RxDecEnable;
		SECR_value |= SCR_NoSKMC;
		write_nic_byte(dev, SECR, SECR_value);
	}

#ifdef TO_DO_LIST

	

	if(priv->RegRfOff == true)
	{ 
		u8 eRFPath = 0;

		RT_TRACE((COMP_INIT|COMP_RF), "InitializeAdapter8192SUsb(): Turn off RF for RegRfOff ----------\n");
		MgntActSet_RF_State(dev, eRfOff, RF_CHANGE_BY_SW);
		
		for(eRFPath = 0; eRFPath <priv->NumTotalRFPath; eRFPath++)
			rtl8192_setBBreg(dev, (RF90_RADIO_PATH_E)eRFPath, 0x4, 0xC00, 0x0);
	}
	else if(priv->RfOffReason > RF_CHANGE_BY_PS)
	{ 
		RT_TRACE((COMP_INIT|COMP_RF), "InitializeAdapter8192SUsb(): Turn off RF for RfOffReason(%d) ----------\n", priv->RfOffReason);
		MgntActSet_RF_State(dev, eRfOff, priv->RfOffReason);
	}
	else
	{
		priv->eRFPowerState = eRfOn;
		priv->RfOffReason = 0;
		RT_TRACE((COMP_INIT|COMP_RF), "InitializeAdapter8192SUsb(): RF is on ----------\n");
	}

#endif





#ifdef TO_DO_LIST

#ifndef UNDER_VISTA
	{
		u8	i;
		PlatformAcquireSpinLock(Adapter, RT_RX_SPINLOCK);

		for(PipeIndex=0; PipeIndex < MAX_RX_QUEUE; PipeIndex++)
		{
			if (PipeIndex == 0)
			{
				for(i=0; i<32; i++)
				HalUsbInMpdu(Adapter, PipeIndex);
			}
			else
			{
				
				
				
			}
		}
		PlatformReleaseSpinLock(Adapter, RT_RX_SPINLOCK);
	}
#else
		
		
	        PlatformUsbEnableInPipes(Adapter);
#endif

	RT_TRACE(COMP_INIT, "HighestOperaRate = %x\n", Adapter->MgntInfo.HighestOperaRate);

	PlatformStartWorkItem( &(pHalData->RtUsbCheckForHangWorkItem) );

	
	
	
	

#endif

	
	
	
	
	
	{
		
		PHY_GetHWRegOriginalValue(dev);

		
		PHY_SetTxPowerLevel8192S(dev, priv->chan);
	}

	{
	u8  tmpU1b = 0;
	
	tmpU1b = read_nic_byte(dev, MAC_PINMUX_CFG);
	write_nic_byte(dev, MAC_PINMUX_CFG, tmpU1b&(~GPIOMUX_EN));
	}





#ifdef RTL8192SU_FW_IQK
	write_nic_dword(dev, WFM5, FW_IQK_ENABLE);
	ChkFwCmdIoDone(dev);
#endif

	
	
	
	
	write_nic_dword(dev, WFM5, FW_RA_RESET);
	ChkFwCmdIoDone(dev);
	write_nic_dword(dev, WFM5, FW_RA_ACTIVE);
	ChkFwCmdIoDone(dev);
	write_nic_dword(dev, WFM5, FW_RA_REFRESH);
	ChkFwCmdIoDone(dev);
	write_nic_dword(dev, WFM5, FW_BB_RESET_ENABLE);




end:
return rtStatus;
}



static struct net_device_stats *rtl8192_stats(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);

	return &priv->ieee80211->stats;
}

bool
HalTxCheckStuck819xUsb(
	struct net_device *dev
	)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	u16 		RegTxCounter = read_nic_word(dev, 0x128);
	bool		bStuck = FALSE;
	RT_TRACE(COMP_RESET,"%s():RegTxCounter is %d,TxCounter is %d\n",__FUNCTION__,RegTxCounter,priv->TxCounter);
	if(priv->TxCounter==RegTxCounter)
		bStuck = TRUE;

	priv->TxCounter = RegTxCounter;

	return bStuck;
}


RESET_TYPE
TxCheckStuck(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	u8			QueueID;


	bool			bCheckFwTxCnt = false;
	

	
	
	




	     for (QueueID = 0; QueueID<=BEACON_QUEUE;QueueID ++)
	     {
	     		if(QueueID == TXCMD_QUEUE)
		         continue;
#if 1
		     	if((skb_queue_len(&priv->ieee80211->skb_waitQ[QueueID]) == 0)  && (skb_queue_len(&priv->ieee80211->skb_aggQ[QueueID]) == 0))
			 	continue;
#endif

	             bCheckFwTxCnt = true;
	     }



#if 1
	if(bCheckFwTxCnt)
	{
		if(HalTxCheckStuck819xUsb(dev))
		{
			RT_TRACE(COMP_RESET, "TxCheckStuck(): Fw indicates no Tx condition! \n");
			return RESET_TYPE_SILENT;
		}
	}
#endif
	return RESET_TYPE_NORESET;
}

bool
HalRxCheckStuck819xUsb(struct net_device *dev)
{
	u16 	RegRxCounter = read_nic_word(dev, 0x130);
	struct r8192_priv *priv = ieee80211_priv(dev);
	bool bStuck = FALSE;



	static u8	rx_chk_cnt = 0;
	RT_TRACE(COMP_RESET,"%s(): RegRxCounter is %d,RxCounter is %d\n",__FUNCTION__,RegRxCounter,priv->RxCounter);
	
	
	rx_chk_cnt++;
	if(priv->undecorated_smoothed_pwdb >= (RateAdaptiveTH_High+5))
	{
		rx_chk_cnt = 0;	
	}
	else if(priv->undecorated_smoothed_pwdb < (RateAdaptiveTH_High+5) &&
		((priv->CurrentChannelBW!=HT_CHANNEL_WIDTH_20&&priv->undecorated_smoothed_pwdb>=RateAdaptiveTH_Low_40M) ||
		(priv->CurrentChannelBW==HT_CHANNEL_WIDTH_20&&priv->undecorated_smoothed_pwdb>=RateAdaptiveTH_Low_20M)) )
	{
		if(rx_chk_cnt < 2)
		{
			return bStuck;
		}
		else
		{
			rx_chk_cnt = 0;
		}
	}
	else if(((priv->CurrentChannelBW!=HT_CHANNEL_WIDTH_20&&priv->undecorated_smoothed_pwdb<RateAdaptiveTH_Low_40M) ||
		(priv->CurrentChannelBW==HT_CHANNEL_WIDTH_20&&priv->undecorated_smoothed_pwdb<RateAdaptiveTH_Low_20M)) &&
		priv->undecorated_smoothed_pwdb >= VeryLowRSSI)
	{
		if(rx_chk_cnt < 4)
		{
			
			return bStuck;
		}
		else
		{
			rx_chk_cnt = 0;
			
		}
	}
	else
	{
		if(rx_chk_cnt < 8)
		{
			
			return bStuck;
		}
		else
		{
			rx_chk_cnt = 0;
			
		}
	}


	if(priv->RxCounter==RegRxCounter)
		bStuck = TRUE;

	priv->RxCounter = RegRxCounter;

	return bStuck;
}

RESET_TYPE
RxCheckStuck(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	
	bool        bRxCheck = FALSE;


	

	 if(priv->IrpPendingCount > 1)
	 	bRxCheck = TRUE;
       


	if(bRxCheck)
	{
		if(HalRxCheckStuck819xUsb(dev))
		{
			RT_TRACE(COMP_RESET, "RxStuck Condition\n");
			return RESET_TYPE_SILENT;
		}
	}
	return RESET_TYPE_NORESET;
}



RESET_TYPE
rtl819x_ifcheck_resetornot(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	RESET_TYPE	TxResetType = RESET_TYPE_NORESET;
	RESET_TYPE	RxResetType = RESET_TYPE_NORESET;
	RT_RF_POWER_STATE 	rfState;

	return RESET_TYPE_NORESET;

	rfState = priv->ieee80211->eRFPowerState;

	TxResetType = TxCheckStuck(dev);
#if 1
	if( rfState != eRfOff ||
		
		(priv->ieee80211->iw_mode != IW_MODE_ADHOC))
	{
		
		
		
		

		
		
		
		RxResetType = RxCheckStuck(dev);
	}
#endif
	if(TxResetType==RESET_TYPE_NORMAL || RxResetType==RESET_TYPE_NORMAL)
		return RESET_TYPE_NORMAL;
	else if(TxResetType==RESET_TYPE_SILENT || RxResetType==RESET_TYPE_SILENT){
		RT_TRACE(COMP_RESET,"%s():silent reset\n",__FUNCTION__);
		return RESET_TYPE_SILENT;
	}
	else
		return RESET_TYPE_NORESET;

}

void rtl8192_cancel_deferred_work(struct r8192_priv* priv);
int _rtl8192_up(struct net_device *dev);
int rtl8192_close(struct net_device *dev);



void
CamRestoreAllEntry(	struct net_device *dev)
{
	u8 EntryId = 0;
	struct r8192_priv *priv = ieee80211_priv(dev);
	u8*	MacAddr = priv->ieee80211->current_network.bssid;

	static u8	CAM_CONST_ADDR[4][6] = {
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x03}};
	static u8	CAM_CONST_BROAD[] =
		{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	RT_TRACE(COMP_SEC, "CamRestoreAllEntry: \n");


	if ((priv->ieee80211->pairwise_key_type == KEY_TYPE_WEP40)||
	    (priv->ieee80211->pairwise_key_type == KEY_TYPE_WEP104))
	{

		for(EntryId=0; EntryId<4; EntryId++)
		{
			{
				MacAddr = CAM_CONST_ADDR[EntryId];
				setKey(dev,
						EntryId ,
						EntryId,
						priv->ieee80211->pairwise_key_type,
						MacAddr,
						0,
						NULL);
			}
		}

	}
	else if(priv->ieee80211->pairwise_key_type == KEY_TYPE_TKIP)
	{

		{
			if(priv->ieee80211->iw_mode == IW_MODE_ADHOC)
				setKey(dev,
						4,
						0,
						priv->ieee80211->pairwise_key_type,
						(u8*)dev->dev_addr,
						0,
						NULL);
			else
				setKey(dev,
						4,
						0,
						priv->ieee80211->pairwise_key_type,
						MacAddr,
						0,
						NULL);
		}
	}
	else if(priv->ieee80211->pairwise_key_type == KEY_TYPE_CCMP)
	{

		{
			if(priv->ieee80211->iw_mode == IW_MODE_ADHOC)
				setKey(dev,
						4,
						0,
						priv->ieee80211->pairwise_key_type,
						(u8*)dev->dev_addr,
						0,
						NULL);
			else
				setKey(dev,
						4,
						0,
						priv->ieee80211->pairwise_key_type,
						MacAddr,
						0,
						NULL);
		}
	}



	if(priv->ieee80211->group_key_type == KEY_TYPE_TKIP)
	{
		MacAddr = CAM_CONST_BROAD;
		for(EntryId=1 ; EntryId<4 ; EntryId++)
		{
			{
				setKey(dev,
						EntryId,
						EntryId,
						priv->ieee80211->group_key_type,
						MacAddr,
						0,
						NULL);
			}
		}
		if(priv->ieee80211->iw_mode == IW_MODE_ADHOC)
				setKey(dev,
						0,
						0,
						priv->ieee80211->group_key_type,
						CAM_CONST_ADDR[0],
						0,
						NULL);
	}
	else if(priv->ieee80211->group_key_type == KEY_TYPE_CCMP)
	{
		MacAddr = CAM_CONST_BROAD;
		for(EntryId=1; EntryId<4 ; EntryId++)
		{
			{
				setKey(dev,
						EntryId ,
						EntryId,
						priv->ieee80211->group_key_type,
						MacAddr,
						0,
						NULL);
			}
		}

		if(priv->ieee80211->iw_mode == IW_MODE_ADHOC)
				setKey(dev,
						0 ,
						0,
						priv->ieee80211->group_key_type,
						CAM_CONST_ADDR[0],
						0,
						NULL);
	}
}






void
rtl819x_ifsilentreset(struct net_device *dev)
{
	
	struct r8192_priv *priv = ieee80211_priv(dev);
	u8	reset_times = 0;
	int reset_status = 0;
	struct ieee80211_device *ieee = priv->ieee80211;


	
	

	if(priv->ResetProgress==RESET_TYPE_NORESET)
	{
RESET_START:

		RT_TRACE(COMP_RESET,"=========>Reset progress!! \n");

		
		priv->ResetProgress = RESET_TYPE_SILENT;

#if 1
		down(&priv->wx_sem);
		if(priv->up == 0)
		{
			RT_TRACE(COMP_ERR,"%s():the driver is not up! return\n",__FUNCTION__);
			up(&priv->wx_sem);
			return ;
		}
		priv->up = 0;
		RT_TRACE(COMP_RESET,"%s():======>start to down the driver\n",__FUNCTION__);



		rtl8192_rtx_disable(dev);
		rtl8192_cancel_deferred_work(priv);
		deinit_hal_dm(dev);
		del_timer_sync(&priv->watch_dog_timer);

		ieee->sync_scan_hurryup = 1;
		if(ieee->state == IEEE80211_LINKED)
		{
			down(&ieee->wx_sem);
			printk("ieee->state is IEEE80211_LINKED\n");
			ieee80211_stop_send_beacons(priv->ieee80211);
			del_timer_sync(&ieee->associate_timer);
			cancel_delayed_work(&ieee->associate_retry_wq);
			ieee80211_stop_scan(ieee);
			netif_carrier_off(dev);
			up(&ieee->wx_sem);
		}
		else{
			printk("ieee->state is NOT LINKED\n");
			ieee80211_softmac_stop_protocol(priv->ieee80211);			}
		up(&priv->wx_sem);
		RT_TRACE(COMP_RESET,"%s():<==========down process is finished\n",__FUNCTION__);
	
		RT_TRACE(COMP_RESET,"%s():===========>start to up the driver\n",__FUNCTION__);
		reset_status = _rtl8192_up(dev);

		RT_TRACE(COMP_RESET,"%s():<===========up process is finished\n",__FUNCTION__);
		if(reset_status == -EAGAIN)
		{
			if(reset_times < 3)
			{
				reset_times++;
				goto RESET_START;
			}
			else
			{
				RT_TRACE(COMP_ERR," ERR!!! %s():  Reset Failed!!\n", __FUNCTION__);
			}
		}
#endif
		ieee->is_silent_reset = 1;
#if 1
		EnableHWSecurityConfig8192(dev);
#if 1
		if(ieee->state == IEEE80211_LINKED && ieee->iw_mode == IW_MODE_INFRA)
		{
			ieee->set_chan(ieee->dev, ieee->current_network.channel);

#if 1
			queue_work(ieee->wq, &ieee->associate_complete_wq);
#endif

		}
		else if(ieee->state == IEEE80211_LINKED && ieee->iw_mode == IW_MODE_ADHOC)
		{
			ieee->set_chan(ieee->dev, ieee->current_network.channel);
			ieee->link_change(ieee->dev);

		

			ieee80211_start_send_beacons(ieee);

			if (ieee->data_hard_resume)
				ieee->data_hard_resume(ieee->dev);
			netif_carrier_on(ieee->dev);
		}
#endif

		CamRestoreAllEntry(dev);

		priv->ResetProgress = RESET_TYPE_NORESET;
		priv->reset_count++;

		priv->bForcedSilentReset =false;
		priv->bResetInProgress = false;

		
		write_nic_byte(dev, UFWP, 1);
		RT_TRACE(COMP_RESET, "Reset finished!! ====>[%d]\n", priv->reset_count);
#endif
	}
}

void CAM_read_entry(
	struct net_device *dev,
	u32	 		iIndex
)
{
 	u32 target_command=0;
	 u32 target_content=0;
	 u8 entry_i=0;
	 u32 ulStatus;
	s32 i=100;

 	for(entry_i=0;entry_i<CAM_CONTENT_COUNT;entry_i++)
 	{
   	
		target_command= entry_i+CAM_CONTENT_COUNT*iIndex;
		target_command= target_command | BIT31;

	

#if 1
		while((i--)>=0)
		{
			ulStatus = read_nic_dword(dev, RWCAM);
			if(ulStatus & BIT31){
				continue;
			}
			else{
				break;
			}
		}
#endif
  		write_nic_dword(dev, RWCAM, target_command);
   	 	RT_TRACE(COMP_SEC,"CAM_read_entry(): WRITE A0: %x \n",target_command);
   	 
  	 	target_content = read_nic_dword(dev, RCAMO);
  	 	RT_TRACE(COMP_SEC, "CAM_read_entry(): WRITE A8: %x \n",target_content);
  	 
 	}
	printk("\n");
}

void rtl819x_update_rxcounts(
	struct r8192_priv *priv,
	u32* TotalRxBcnNum,
	u32* TotalRxDataNum
)
{
	u16 			SlotIndex;
	u8			i;

	*TotalRxBcnNum = 0;
	*TotalRxDataNum = 0;

	SlotIndex = (priv->ieee80211->LinkDetectInfo.SlotIndex++)%(priv->ieee80211->LinkDetectInfo.SlotNum);
	priv->ieee80211->LinkDetectInfo.RxBcnNum[SlotIndex] = priv->ieee80211->LinkDetectInfo.NumRecvBcnInPeriod;
	priv->ieee80211->LinkDetectInfo.RxDataNum[SlotIndex] = priv->ieee80211->LinkDetectInfo.NumRecvDataInPeriod;
	for( i=0; i<priv->ieee80211->LinkDetectInfo.SlotNum; i++ ){
		*TotalRxBcnNum += priv->ieee80211->LinkDetectInfo.RxBcnNum[i];
		*TotalRxDataNum += priv->ieee80211->LinkDetectInfo.RxDataNum[i];
	}
}

extern	void	rtl819x_watchdog_wqcallback(struct work_struct *work)
{
	struct delayed_work *dwork = container_of(work,struct delayed_work,work);
       struct r8192_priv *priv = container_of(dwork,struct r8192_priv,watch_dog_wq);
       struct net_device *dev = priv->ieee80211->dev;
	struct ieee80211_device* ieee = priv->ieee80211;
	RESET_TYPE	ResetType = RESET_TYPE_NORESET;
      	static u8	check_reset_cnt=0;
	bool bBusyTraffic = false;

	if(!priv->up)
		return;
	hal_dm_watchdog(dev);

	{
		if(ieee->state == IEEE80211_LINKED)
		{
			
			
			
			if(	ieee->LinkDetectInfo.NumRxOkInPeriod> 100 ||
				ieee->LinkDetectInfo.NumTxOkInPeriod> 100 ) {
				bBusyTraffic = true;
			}
			ieee->LinkDetectInfo.NumRxOkInPeriod = 0;
			ieee->LinkDetectInfo.NumTxOkInPeriod = 0;
			ieee->LinkDetectInfo.bBusyTraffic = bBusyTraffic;
		}
	}
	
	{
		if(priv->ieee80211->state == IEEE80211_LINKED && priv->ieee80211->iw_mode == IW_MODE_INFRA)
		{
			u32	TotalRxBcnNum = 0;
			u32	TotalRxDataNum = 0;

			rtl819x_update_rxcounts(priv, &TotalRxBcnNum, &TotalRxDataNum);
			if((TotalRxBcnNum+TotalRxDataNum) == 0)
			{
				#ifdef TODO
				if(rfState == eRfOff)
					RT_TRACE(COMP_ERR,"========>%s()\n",__FUNCTION__);
				#endif
				printk("===>%s(): AP is power off,connect another one\n",__FUNCTION__);
			
				priv->ieee80211->state = IEEE80211_ASSOCIATING;
				notify_wx_assoc_event(priv->ieee80211);
				RemovePeerTS(priv->ieee80211,priv->ieee80211->current_network.bssid);
				ieee->is_roaming = true;
				priv->ieee80211->link_change(dev);
                                queue_work(priv->ieee80211->wq, &priv->ieee80211->associate_procedure_wq);
			}
		}
		priv->ieee80211->LinkDetectInfo.NumRecvBcnInPeriod=0;
		priv->ieee80211->LinkDetectInfo.NumRecvDataInPeriod=0;
	}

	
	if(check_reset_cnt++ >= 3 && !ieee->is_roaming)
	{
    		ResetType = rtl819x_ifcheck_resetornot(dev);
		check_reset_cnt = 3;
		
	}
	
#if 1
	if( (priv->force_reset) || (priv->ResetProgress==RESET_TYPE_NORESET &&
		(priv->bForcedSilentReset ||
		(!priv->bDisableNormalResetCheck && ResetType==RESET_TYPE_SILENT)))) 
	{
		RT_TRACE(COMP_RESET,"%s():priv->force_reset is %d,priv->ResetProgress is %d, priv->bForcedSilentReset is %d,priv->bDisableNormalResetCheck is %d,ResetType is %d\n",__FUNCTION__,priv->force_reset,priv->ResetProgress,priv->bForcedSilentReset,priv->bDisableNormalResetCheck,ResetType);
		rtl819x_ifsilentreset(dev);
	}
#endif
	priv->force_reset = false;
	priv->bForcedSilentReset = false;
	priv->bResetInProgress = false;
	RT_TRACE(COMP_TRACE, " <==RtUsbCheckForHangWorkItemCallback()\n");

}

void watch_dog_timer_callback(unsigned long data)
{
	struct r8192_priv *priv = ieee80211_priv((struct net_device *) data);
	
	queue_delayed_work(priv->priv_wq,&priv->watch_dog_wq, 0);
	mod_timer(&priv->watch_dog_timer, jiffies + MSECS(IEEE80211_WATCH_DOG_TIME));
}
int _rtl8192_up(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	
	int init_status = 0;
	priv->up=1;
	priv->ieee80211->ieee_up=1;
	RT_TRACE(COMP_INIT, "Bringing up iface");
	init_status = priv->ops->rtl819x_adapter_start(dev);
	if(!init_status)
	{
		RT_TRACE(COMP_ERR,"ERR!!! %s(): initialization is failed!\n", __FUNCTION__);
		priv->up=priv->ieee80211->ieee_up = 0;
		return -EAGAIN;
	}
	RT_TRACE(COMP_INIT, "start adapter finished\n");
	rtl8192_rx_enable(dev);

	if(priv->ieee80211->state != IEEE80211_LINKED)
	ieee80211_softmac_start_protocol(priv->ieee80211);
	ieee80211_reset_queue(priv->ieee80211);
	watch_dog_timer_callback((unsigned long) dev);
	if(!netif_queue_stopped(dev))
		netif_start_queue(dev);
	else
		netif_wake_queue(dev);

	
	priv->ieee80211->drop_unencrypted = 0;

	return 0;
}


int rtl8192_open(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	int ret;
	down(&priv->wx_sem);
	ret = rtl8192_up(dev);
	up(&priv->wx_sem);
	return ret;

}


int rtl8192_up(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);

	if (priv->up == 1) return -1;

	return _rtl8192_up(dev);
}


int rtl8192_close(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	int ret;

	down(&priv->wx_sem);

	ret = rtl8192_down(dev);

	up(&priv->wx_sem);

	return ret;

}

int rtl8192_down(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	int i;

	if (priv->up == 0) return -1;

	priv->up=0;
	priv->ieee80211->ieee_up = 0;
	RT_TRACE(COMP_DOWN, "==========>%s()\n", __FUNCTION__);

	if (!netif_queue_stopped(dev))
		netif_stop_queue(dev);

	rtl8192_rtx_disable(dev);
	

 
        for(i = 0; i < MAX_QUEUE_SIZE; i++) {
                skb_queue_purge(&priv->ieee80211->skb_waitQ [i]);
        }
        for(i = 0; i < MAX_QUEUE_SIZE; i++) {
                skb_queue_purge(&priv->ieee80211->skb_aggQ [i]);
        }

        for(i = 0; i < MAX_QUEUE_SIZE; i++) {
                skb_queue_purge(&priv->ieee80211->skb_drv_aggQ [i]);
        }

        

	rtl8192_cancel_deferred_work(priv);
	deinit_hal_dm(dev);
	del_timer_sync(&priv->watch_dog_timer);


	ieee80211_softmac_stop_protocol(priv->ieee80211);
	memset(&priv->ieee80211->current_network, 0 , offsetof(struct ieee80211_network, list));
	RT_TRACE(COMP_DOWN, "<==========%s()\n", __FUNCTION__);

		return 0;
}


void rtl8192_commit(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	int reset_status = 0;
	
	if (priv->up == 0) return ;
	priv->up = 0;

	rtl8192_cancel_deferred_work(priv);
	del_timer_sync(&priv->watch_dog_timer);
	

	ieee80211_softmac_stop_protocol(priv->ieee80211);

	
	rtl8192_rtx_disable(dev);
	reset_status = _rtl8192_up(dev);

}


void rtl8192_restart(struct work_struct *work)
{
        struct r8192_priv *priv = container_of(work, struct r8192_priv, reset_wq);
        struct net_device *dev = priv->ieee80211->dev;

	down(&priv->wx_sem);

	rtl8192_commit(dev);

	up(&priv->wx_sem);
}

static void r8192_set_multicast(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	short promisc;

	

	

	promisc = (dev->flags & IFF_PROMISC) ? 1:0;

	if (promisc != priv->promisc)
	

	priv->promisc = promisc;

	
	
}


int r8192_set_mac_adr(struct net_device *dev, void *mac)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	struct sockaddr *addr = mac;

	down(&priv->wx_sem);

	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);

	schedule_work(&priv->reset_wq);

	up(&priv->wx_sem);

	return 0;
}


int rtl8192_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct iwreq *wrq = (struct iwreq *)rq;
	int ret=-1;
	struct ieee80211_device *ieee = priv->ieee80211;
	u32 key[4];
	u8 broadcast_addr[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	u8 zero_addr[6] = {0};
	struct iw_point *p = &wrq->u.data;
	struct ieee_param *ipw = NULL;

	down(&priv->wx_sem);


     if (p->length < sizeof(struct ieee_param) || !p->pointer){
             ret = -EINVAL;
             goto out;
	}

     ipw = (struct ieee_param *)kmalloc(p->length, GFP_KERNEL);
     if (ipw == NULL){
             ret = -ENOMEM;
             goto out;
     }
     if (copy_from_user(ipw, p->pointer, p->length)) {
		kfree(ipw);
            ret = -EFAULT;
            goto out;
	}

	switch (cmd) {
	    case RTL_IOCTL_WPA_SUPPLICANT:
	
			if (ipw->cmd == IEEE_CMD_SET_ENCRYPTION)
			{
				if (ipw->u.crypt.set_tx)
				{
					if (strcmp(ipw->u.crypt.alg, "CCMP") == 0)
						ieee->pairwise_key_type = KEY_TYPE_CCMP;
					else if (strcmp(ipw->u.crypt.alg, "TKIP") == 0)
						ieee->pairwise_key_type = KEY_TYPE_TKIP;
					else if (strcmp(ipw->u.crypt.alg, "WEP") == 0)
					{
						if (ipw->u.crypt.key_len == 13)
							ieee->pairwise_key_type = KEY_TYPE_WEP104;
						else if (ipw->u.crypt.key_len == 5)
							ieee->pairwise_key_type = KEY_TYPE_WEP40;
					}
					else
						ieee->pairwise_key_type = KEY_TYPE_NA;

					if (ieee->pairwise_key_type)
					{
				
						if (memcmp(ieee->ap_mac_addr, zero_addr, 6) == 0)
							ieee->iw_mode = IW_MODE_ADHOC;
						memcpy((u8*)key, ipw->u.crypt.key, 16);
						EnableHWSecurityConfig8192(dev);
					
					
						setKey(dev, 4, ipw->u.crypt.idx, ieee->pairwise_key_type, (u8*)ieee->ap_mac_addr, 0, key);
						if (ieee->iw_mode == IW_MODE_ADHOC)
						setKey(dev, ipw->u.crypt.idx, ipw->u.crypt.idx, ieee->pairwise_key_type, (u8*)ieee->ap_mac_addr, 0, key);
					}
				}
				else 
				{
					memcpy((u8*)key, ipw->u.crypt.key, 16);
					if (strcmp(ipw->u.crypt.alg, "CCMP") == 0)
						ieee->group_key_type= KEY_TYPE_CCMP;
					else if (strcmp(ipw->u.crypt.alg, "TKIP") == 0)
						ieee->group_key_type = KEY_TYPE_TKIP;
					else if (strcmp(ipw->u.crypt.alg, "WEP") == 0)
					{
						if (ipw->u.crypt.key_len == 13)
							ieee->group_key_type = KEY_TYPE_WEP104;
						else if (ipw->u.crypt.key_len == 5)
							ieee->group_key_type = KEY_TYPE_WEP40;
					}
					else
						ieee->group_key_type = KEY_TYPE_NA;

					if (ieee->group_key_type)
					{
							setKey(	dev,
								ipw->u.crypt.idx,
								ipw->u.crypt.idx,		
						     		ieee->group_key_type,	
						            	broadcast_addr,	
								0,		
							      	key);		
					}
				}
			}
#ifdef JOHN_HWSEC_DEBUG
		
		printk("@@ wrq->u pointer = ");
		for(i=0;i<wrq->u.data.length;i++){
			if(i%10==0) printk("\n");
			printk( "%8x|", ((u32*)wrq->u.data.pointer)[i] );
		}
		printk("\n");
#endif 
		ret = ieee80211_wpa_supplicant_ioctl(priv->ieee80211, &wrq->u.data);
		break;

	    default:
		ret = -EOPNOTSUPP;
		break;
	}
	kfree(ipw);
        ipw = NULL;
out:
	up(&priv->wx_sem);
	return ret;
}

u8 rtl8192SU_HwRateToMRate(bool bIsHT, u8 rate,bool bFirstAMPDU)
{

	u8	ret_rate = 0x02;

	if( bFirstAMPDU )
	{
	if(!bIsHT)
	{
		switch(rate)
		{

			case DESC92S_RATE1M:		ret_rate = MGN_1M;		break;
			case DESC92S_RATE2M:		ret_rate = MGN_2M;		break;
			case DESC92S_RATE5_5M:		ret_rate = MGN_5_5M;		break;
			case DESC92S_RATE11M:		ret_rate = MGN_11M;		break;
			case DESC92S_RATE6M:		ret_rate = MGN_6M;		break;
			case DESC92S_RATE9M:		ret_rate = MGN_9M;		break;
			case DESC92S_RATE12M:		ret_rate = MGN_12M;		break;
			case DESC92S_RATE18M:		ret_rate = MGN_18M;		break;
			case DESC92S_RATE24M:		ret_rate = MGN_24M;		break;
			case DESC92S_RATE36M:		ret_rate = MGN_36M;		break;
			case DESC92S_RATE48M:		ret_rate = MGN_48M;		break;
			case DESC92S_RATE54M:		ret_rate = MGN_54M;		break;

			default:
				RT_TRACE(COMP_RECV, "HwRateToMRate90(): Non supported Rate [%x], bIsHT = %d!!!\n", rate, bIsHT);
					break;
	}
		}
		else
	{
		switch(rate)
		{

			case DESC92S_RATEMCS0:	ret_rate = MGN_MCS0;		break;
			case DESC92S_RATEMCS1:	ret_rate = MGN_MCS1;		break;
			case DESC92S_RATEMCS2:	ret_rate = MGN_MCS2;		break;
			case DESC92S_RATEMCS3:	ret_rate = MGN_MCS3;		break;
			case DESC92S_RATEMCS4:	ret_rate = MGN_MCS4;		break;
			case DESC92S_RATEMCS5:	ret_rate = MGN_MCS5;		break;
			case DESC92S_RATEMCS6:	ret_rate = MGN_MCS6;		break;
			case DESC92S_RATEMCS7:	ret_rate = MGN_MCS7;		break;
			case DESC92S_RATEMCS8:	ret_rate = MGN_MCS8;		break;
			case DESC92S_RATEMCS9:	ret_rate = MGN_MCS9;		break;
			case DESC92S_RATEMCS10:	ret_rate = MGN_MCS10;	break;
			case DESC92S_RATEMCS11:	ret_rate = MGN_MCS11;	break;
			case DESC92S_RATEMCS12:	ret_rate = MGN_MCS12;	break;
			case DESC92S_RATEMCS13:	ret_rate = MGN_MCS13;	break;
			case DESC92S_RATEMCS14:	ret_rate = MGN_MCS14;	break;
			case DESC92S_RATEMCS15:	ret_rate = MGN_MCS15;	break;
			case DESC92S_RATEMCS32:	ret_rate = (0x80|0x20);	break;

			default:
					RT_TRACE(COMP_RECV, "HwRateToMRate92S(): Non supported Rate [%x], bIsHT = %d!!!\n",rate, bIsHT );
				break;
		}

	}
	}
	else
	{
		switch(rate)
		{

			case DESC92S_RATE1M:	ret_rate = MGN_1M;		break;
			case DESC92S_RATE2M:	ret_rate = MGN_2M;		break;
			case DESC92S_RATE5_5M:	ret_rate = MGN_5_5M;		break;
			case DESC92S_RATE11M:	ret_rate = MGN_11M;		break;
			case DESC92S_RATE6M:	ret_rate = MGN_6M;		break;
			case DESC92S_RATE9M:	ret_rate = MGN_9M;		break;
			case DESC92S_RATE12M:	ret_rate = MGN_12M;		break;
			case DESC92S_RATE18M:	ret_rate = MGN_18M;		break;
			case DESC92S_RATE24M:	ret_rate = MGN_24M;		break;
			case DESC92S_RATE36M:	ret_rate = MGN_36M;		break;
			case DESC92S_RATE48M:	ret_rate = MGN_48M;		break;
			case DESC92S_RATE54M:	ret_rate = MGN_54M;		break;
			case DESC92S_RATEMCS0:	ret_rate = MGN_MCS0;		break;
			case DESC92S_RATEMCS1:	ret_rate = MGN_MCS1;		break;
			case DESC92S_RATEMCS2:	ret_rate = MGN_MCS2;		break;
			case DESC92S_RATEMCS3:	ret_rate = MGN_MCS3;		break;
			case DESC92S_RATEMCS4:	ret_rate = MGN_MCS4;		break;
			case DESC92S_RATEMCS5:	ret_rate = MGN_MCS5;		break;
			case DESC92S_RATEMCS6:	ret_rate = MGN_MCS6;		break;
			case DESC92S_RATEMCS7:	ret_rate = MGN_MCS7;		break;
			case DESC92S_RATEMCS8:	ret_rate = MGN_MCS8;		break;
			case DESC92S_RATEMCS9:	ret_rate = MGN_MCS9;		break;
			case DESC92S_RATEMCS10:	ret_rate = MGN_MCS10;	break;
			case DESC92S_RATEMCS11:	ret_rate = MGN_MCS11;	break;
			case DESC92S_RATEMCS12:	ret_rate = MGN_MCS12;	break;
			case DESC92S_RATEMCS13:	ret_rate = MGN_MCS13;	break;
			case DESC92S_RATEMCS14:	ret_rate = MGN_MCS14;	break;
			case DESC92S_RATEMCS15:	ret_rate = MGN_MCS15;	break;
			case DESC92S_RATEMCS32:	ret_rate = (0x80|0x20);	break;

			default:
				RT_TRACE(COMP_RECV, "HwRateToMRate92S(): Non supported Rate [%x], bIsHT = %d!!!\n",rate, bIsHT );
				break;
			}
	}
	return ret_rate;
}

u8 HwRateToMRate90(bool bIsHT, u8 rate)
{
	u8  ret_rate = 0xff;

	if(!bIsHT) {
		switch(rate) {
			case DESC90_RATE1M:   ret_rate = MGN_1M;         break;
			case DESC90_RATE2M:   ret_rate = MGN_2M;         break;
			case DESC90_RATE5_5M: ret_rate = MGN_5_5M;       break;
			case DESC90_RATE11M:  ret_rate = MGN_11M;        break;
			case DESC90_RATE6M:   ret_rate = MGN_6M;         break;
			case DESC90_RATE9M:   ret_rate = MGN_9M;         break;
			case DESC90_RATE12M:  ret_rate = MGN_12M;        break;
			case DESC90_RATE18M:  ret_rate = MGN_18M;        break;
			case DESC90_RATE24M:  ret_rate = MGN_24M;        break;
			case DESC90_RATE36M:  ret_rate = MGN_36M;        break;
			case DESC90_RATE48M:  ret_rate = MGN_48M;        break;
			case DESC90_RATE54M:  ret_rate = MGN_54M;        break;

			default:
				ret_rate = 0xff;
				RT_TRACE(COMP_RECV, "HwRateToMRate90(): Non supported Rate [%x], bIsHT = %d!!!\n", rate, bIsHT);
				break;
		}

	} else {
		switch(rate) {
			case DESC90_RATEMCS0:   ret_rate = MGN_MCS0;    break;
			case DESC90_RATEMCS1:   ret_rate = MGN_MCS1;    break;
			case DESC90_RATEMCS2:   ret_rate = MGN_MCS2;    break;
			case DESC90_RATEMCS3:   ret_rate = MGN_MCS3;    break;
			case DESC90_RATEMCS4:   ret_rate = MGN_MCS4;    break;
			case DESC90_RATEMCS5:   ret_rate = MGN_MCS5;    break;
			case DESC90_RATEMCS6:   ret_rate = MGN_MCS6;    break;
			case DESC90_RATEMCS7:   ret_rate = MGN_MCS7;    break;
			case DESC90_RATEMCS8:   ret_rate = MGN_MCS8;    break;
			case DESC90_RATEMCS9:   ret_rate = MGN_MCS9;    break;
			case DESC90_RATEMCS10:  ret_rate = MGN_MCS10;   break;
			case DESC90_RATEMCS11:  ret_rate = MGN_MCS11;   break;
			case DESC90_RATEMCS12:  ret_rate = MGN_MCS12;   break;
			case DESC90_RATEMCS13:  ret_rate = MGN_MCS13;   break;
			case DESC90_RATEMCS14:  ret_rate = MGN_MCS14;   break;
			case DESC90_RATEMCS15:  ret_rate = MGN_MCS15;   break;
			case DESC90_RATEMCS32:  ret_rate = (0x80|0x20); break;

			default:
				ret_rate = 0xff;
				RT_TRACE(COMP_RECV, "HwRateToMRate90(): Non supported Rate [%x], bIsHT = %d!!!\n",rate, bIsHT);
				break;
		}
	}

	return ret_rate;
}


void UpdateRxPktTimeStamp8190 (struct net_device *dev, struct ieee80211_rx_stats *stats)
{
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);

	if(stats->bIsAMPDU && !stats->bFirstMPDU) {
		stats->mac_time[0] = priv->LastRxDescTSFLow;
		stats->mac_time[1] = priv->LastRxDescTSFHigh;
	} else {
		priv->LastRxDescTSFLow = stats->mac_time[0];
		priv->LastRxDescTSFHigh = stats->mac_time[1];
	}
}



long rtl819x_translate_todbm(u8 signal_strength_index	)
{
	long	signal_power; 

	
	signal_power = (long)((signal_strength_index + 1) >> 1);
	signal_power -= 95;

	return signal_power;
}



void rtl8192_process_phyinfo(struct r8192_priv * priv,u8* buffer, struct ieee80211_rx_stats * pprevious_stats, struct ieee80211_rx_stats * pcurrent_stats)
{
	bool bcheck = false;
	u8	rfpath;
	u32	nspatial_stream, tmp_val;
	
	static u32 slide_rssi_index=0, slide_rssi_statistics=0;
	static u32 slide_evm_index=0, slide_evm_statistics=0;
	static u32 last_rssi=0, last_evm=0;

	static u32 slide_beacon_adc_pwdb_index=0, slide_beacon_adc_pwdb_statistics=0;
	static u32 last_beacon_adc_pwdb=0;

	struct ieee80211_hdr_3addr *hdr;
	u16 sc ;
	unsigned int frag,seq;
	hdr = (struct ieee80211_hdr_3addr *)buffer;
	sc = le16_to_cpu(hdr->seq_ctl);
	frag = WLAN_GET_SEQ_FRAG(sc);
	seq = WLAN_GET_SEQ_SEQ(sc);
	
	pcurrent_stats->Seq_Num = seq;
	
	
	
	if(!pprevious_stats->bIsAMPDU)
	{
		
		bcheck = true;
	}else
	{
	}


	if(slide_rssi_statistics++ >= PHY_RSSI_SLID_WIN_MAX)
	{
		slide_rssi_statistics = PHY_RSSI_SLID_WIN_MAX;
		last_rssi = priv->stats.slide_signal_strength[slide_rssi_index];
		priv->stats.slide_rssi_total -= last_rssi;
	}
	priv->stats.slide_rssi_total += pprevious_stats->SignalStrength;

	priv->stats.slide_signal_strength[slide_rssi_index++] = pprevious_stats->SignalStrength;
	if(slide_rssi_index >= PHY_RSSI_SLID_WIN_MAX)
		slide_rssi_index = 0;

	
	tmp_val = priv->stats.slide_rssi_total/slide_rssi_statistics;
	priv->stats.signal_strength = rtl819x_translate_todbm((u8)tmp_val);
	pcurrent_stats->rssi = priv->stats.signal_strength;
	
	
	
	if(!pprevious_stats->bPacketMatchBSSID)
	{
		if(!pprevious_stats->bToSelfBA)
			return;
	}

	if(!bcheck)
		return;


	

	
	
	
	priv->stats.num_process_phyinfo++;

	


	
	
	if(!pprevious_stats->bIsCCK && (pprevious_stats->bPacketToSelf || pprevious_stats->bToSelfBA))
	{
		for (rfpath = RF90_PATH_A; rfpath < priv->NumTotalRFPath; rfpath++)
		{
                     if (!rtl8192_phy_CheckIsLegalRFPath(priv->ieee80211->dev, rfpath))
			         continue;

			
			if(priv->stats.rx_rssi_percentage[rfpath] == 0)
			{
				priv->stats.rx_rssi_percentage[rfpath] = pprevious_stats->RxMIMOSignalStrength[rfpath];
				
			}
			if(pprevious_stats->RxMIMOSignalStrength[rfpath]  > priv->stats.rx_rssi_percentage[rfpath])
			{
				priv->stats.rx_rssi_percentage[rfpath] =
					( (priv->stats.rx_rssi_percentage[rfpath]*(Rx_Smooth_Factor-1)) +
					(pprevious_stats->RxMIMOSignalStrength[rfpath])) /(Rx_Smooth_Factor);
				priv->stats.rx_rssi_percentage[rfpath] = priv->stats.rx_rssi_percentage[rfpath]  + 1;
			}
			else
			{
				priv->stats.rx_rssi_percentage[rfpath] =
					( (priv->stats.rx_rssi_percentage[rfpath]*(Rx_Smooth_Factor-1)) +
					(pprevious_stats->RxMIMOSignalStrength[rfpath])) /(Rx_Smooth_Factor);
			}
			RT_TRACE(COMP_DBG,"priv->stats.rx_rssi_percentage[rfPath]  = %d \n" ,priv->stats.rx_rssi_percentage[rfpath] );
		}
	}


	
	
	
	RT_TRACE(COMP_RXDESC, "Smooth %s PWDB = %d\n",
				pprevious_stats->bIsCCK? "CCK": "OFDM",
				pprevious_stats->RxPWDBAll);

	if(pprevious_stats->bPacketBeacon)
	{

		if(slide_beacon_adc_pwdb_statistics++ >= PHY_Beacon_RSSI_SLID_WIN_MAX)
		{
			slide_beacon_adc_pwdb_statistics = PHY_Beacon_RSSI_SLID_WIN_MAX;
			last_beacon_adc_pwdb = priv->stats.Slide_Beacon_pwdb[slide_beacon_adc_pwdb_index];
			priv->stats.Slide_Beacon_Total -= last_beacon_adc_pwdb;
			
			
		}
		priv->stats.Slide_Beacon_Total += pprevious_stats->RxPWDBAll;
		priv->stats.Slide_Beacon_pwdb[slide_beacon_adc_pwdb_index] = pprevious_stats->RxPWDBAll;
		
		slide_beacon_adc_pwdb_index++;
		if(slide_beacon_adc_pwdb_index >= PHY_Beacon_RSSI_SLID_WIN_MAX)
			slide_beacon_adc_pwdb_index = 0;
		pprevious_stats->RxPWDBAll = priv->stats.Slide_Beacon_Total/slide_beacon_adc_pwdb_statistics;
		if(pprevious_stats->RxPWDBAll >= 3)
			pprevious_stats->RxPWDBAll -= 3;
	}

	RT_TRACE(COMP_RXDESC, "Smooth %s PWDB = %d\n",
				pprevious_stats->bIsCCK? "CCK": "OFDM",
				pprevious_stats->RxPWDBAll);


	if(pprevious_stats->bPacketToSelf || pprevious_stats->bPacketBeacon || pprevious_stats->bToSelfBA)
	{
		if(priv->undecorated_smoothed_pwdb < 0)	
		{
			priv->undecorated_smoothed_pwdb = pprevious_stats->RxPWDBAll;
			
		}
#if 1
		if(pprevious_stats->RxPWDBAll > (u32)priv->undecorated_smoothed_pwdb)
		{
			priv->undecorated_smoothed_pwdb =
					( ((priv->undecorated_smoothed_pwdb)*(Rx_Smooth_Factor-1)) +
					(pprevious_stats->RxPWDBAll)) /(Rx_Smooth_Factor);
			priv->undecorated_smoothed_pwdb = priv->undecorated_smoothed_pwdb + 1;
		}
		else
		{
			priv->undecorated_smoothed_pwdb =
					( ((priv->undecorated_smoothed_pwdb)*(Rx_Smooth_Factor-1)) +
					(pprevious_stats->RxPWDBAll)) /(Rx_Smooth_Factor);
		}
#else
		
		if(pPreviousRfd->Status.RxPWDBAll > (u32)pHalData->UndecoratedSmoothedPWDB)
		{
			pHalData->UndecoratedSmoothedPWDB =
					( ((pHalData->UndecoratedSmoothedPWDB)* 5) + (pPreviousRfd->Status.RxPWDBAll)) / 6;
			pHalData->UndecoratedSmoothedPWDB = pHalData->UndecoratedSmoothedPWDB + 1;
		}
		else
		{
			pHalData->UndecoratedSmoothedPWDB =
					( ((pHalData->UndecoratedSmoothedPWDB)* 5) + (pPreviousRfd->Status.RxPWDBAll)) / 6;
		}
#endif

	}

	
	
	
	
	if(pprevious_stats->SignalQuality == 0)
	{
	}
	else
	{
		if(pprevious_stats->bPacketToSelf || pprevious_stats->bPacketBeacon || pprevious_stats->bToSelfBA){
			if(slide_evm_statistics++ >= PHY_RSSI_SLID_WIN_MAX){
				slide_evm_statistics = PHY_RSSI_SLID_WIN_MAX;
				last_evm = priv->stats.slide_evm[slide_evm_index];
				priv->stats.slide_evm_total -= last_evm;
			}

			priv->stats.slide_evm_total += pprevious_stats->SignalQuality;

			priv->stats.slide_evm[slide_evm_index++] = pprevious_stats->SignalQuality;
			if(slide_evm_index >= PHY_RSSI_SLID_WIN_MAX)
				slide_evm_index = 0;

			
			tmp_val = priv->stats.slide_evm_total/slide_evm_statistics;
			priv->stats.signal_quality = tmp_val;
			
			priv->stats.last_signal_strength_inpercent = tmp_val;
		}

		
		if(pprevious_stats->bPacketToSelf || pprevious_stats->bPacketBeacon || pprevious_stats->bToSelfBA)
		{
			for(nspatial_stream = 0; nspatial_stream<2 ; nspatial_stream++) 
			{
				if(pprevious_stats->RxMIMOSignalQuality[nspatial_stream] != -1)
				{
					if(priv->stats.rx_evm_percentage[nspatial_stream] == 0)	
					{
						priv->stats.rx_evm_percentage[nspatial_stream] = pprevious_stats->RxMIMOSignalQuality[nspatial_stream];
					}
					priv->stats.rx_evm_percentage[nspatial_stream] =
						( (priv->stats.rx_evm_percentage[nspatial_stream]* (Rx_Smooth_Factor-1)) +
						(pprevious_stats->RxMIMOSignalQuality[nspatial_stream]* 1)) / (Rx_Smooth_Factor);
				}
			}
		}
	}


}


static u8 rtl819x_query_rxpwrpercentage(
	char		antpower
	)
{
	if ((antpower <= -100) || (antpower >= 20))
	{
		return	0;
	}
	else if (antpower >= 0)
	{
		return	100;
	}
	else
	{
		return	(100+antpower);
	}

}	

static u8
rtl819x_evm_dbtopercentage(
    char value
    )
{
    char ret_val;

    ret_val = value;

    if(ret_val >= 0)
        ret_val = 0;
    if(ret_val <= -33)
        ret_val = -33;
    ret_val = 0 - ret_val;
    ret_val*=3;
	if(ret_val == 99)
		ret_val = 100;
    return(ret_val);
}





long
rtl819x_signal_scale_mapping(
	long currsig
	)
{
	long retsig;

	
	if(currsig >= 61 && currsig <= 100)
	{
		retsig = 90 + ((currsig - 60) / 4);
	}
	else if(currsig >= 41 && currsig <= 60)
	{
		retsig = 78 + ((currsig - 40) / 2);
	}
	else if(currsig >= 31 && currsig <= 40)
	{
		retsig = 66 + (currsig - 30);
	}
	else if(currsig >= 21 && currsig <= 30)
	{
		retsig = 54 + (currsig - 20);
	}
	else if(currsig >= 5 && currsig <= 20)
	{
		retsig = 42 + (((currsig - 5) * 2) / 3);
	}
	else if(currsig == 4)
	{
		retsig = 36;
	}
	else if(currsig == 3)
	{
		retsig = 27;
	}
	else if(currsig == 2)
	{
		retsig = 18;
	}
	else if(currsig == 1)
	{
		retsig = 9;
	}
	else
	{
		retsig = currsig;
	}

	return retsig;
}


static void rtl8192SU_query_rxphystatus(
	struct r8192_priv * priv,
	struct ieee80211_rx_stats * pstats,
	rx_desc_819x_usb	*pDesc,
	rx_drvinfo_819x_usb  * pdrvinfo,
	struct ieee80211_rx_stats * precord_stats,
	bool bpacket_match_bssid,
	bool bpacket_toself,
	bool bPacketBeacon,
	bool bToSelfBA
	)
{
	
	
	phy_sts_cck_819xusb_t	*	pcck_buf;
	phy_ofdm_rx_status_rxsc_sgien_exintfflag* prxsc;
	
	
	u8				i, max_spatial_stream, rxsc_sgien_exflg;
	char				rx_pwr[4], rx_pwr_all=0;
	
	
	u8				evm, pwdb_all;
	u32				RSSI, total_rssi=0;

	u8				is_cck_rate=0;
	u8				rf_rx_num = 0;



	priv->stats.numqry_phystatus++;

	is_cck_rate = rx_hal_is_cck_rate(pDesc);

	
	memset(precord_stats, 0, sizeof(struct ieee80211_rx_stats));
	pstats->bPacketMatchBSSID = precord_stats->bPacketMatchBSSID = bpacket_match_bssid;
	pstats->bPacketToSelf = precord_stats->bPacketToSelf = bpacket_toself;
	pstats->bIsCCK = precord_stats->bIsCCK = is_cck_rate;
	pstats->bPacketBeacon = precord_stats->bPacketBeacon = bPacketBeacon;
	pstats->bToSelfBA = precord_stats->bToSelfBA = bToSelfBA;


	pstats->RxMIMOSignalQuality[0] = -1;
	pstats->RxMIMOSignalQuality[1] = -1;
	precord_stats->RxMIMOSignalQuality[0] = -1;
	precord_stats->RxMIMOSignalQuality[1] = -1;

	if(is_cck_rate)
	{
		u8 report;
		

		
		pcck_buf = (phy_sts_cck_819xusb_t *)pdrvinfo;

		
		
		

		
		
		

		priv->stats.numqry_phystatusCCK++;

		if(!priv->bCckHighPower)
		{
			report = pcck_buf->cck_agc_rpt & 0xc0;
			report = report>>6;
			switch(report)
			{
				
				
				
				case 0x3:
					rx_pwr_all = -35 - (pcck_buf->cck_agc_rpt & 0x3e);
					break;
				case 0x2:
					rx_pwr_all = -23 - (pcck_buf->cck_agc_rpt & 0x3e);
					break;
				case 0x1:
					rx_pwr_all = -11 - (pcck_buf->cck_agc_rpt & 0x3e);
					break;
				case 0x0:
					rx_pwr_all = 8 - (pcck_buf->cck_agc_rpt & 0x3e);
					break;
			}
		}
		else
		{
			report = pdrvinfo->cfosho[0] & 0x60;
			report = report>>5;
			switch(report)
			{
				case 0x3:
					rx_pwr_all = -35 - ((pcck_buf->cck_agc_rpt & 0x1f)<<1) ;
					break;
				case 0x2:
					rx_pwr_all = -23 - ((pcck_buf->cck_agc_rpt & 0x1f)<<1);
					break;
				case 0x1:
					rx_pwr_all = -11 - ((pcck_buf->cck_agc_rpt & 0x1f)<<1) ;
					break;
				case 0x0:
					rx_pwr_all = -8 - ((pcck_buf->cck_agc_rpt & 0x1f)<<1) ;
					break;
			}
		}

		pwdb_all = rtl819x_query_rxpwrpercentage(rx_pwr_all);
		pstats->RxPWDBAll = precord_stats->RxPWDBAll = pwdb_all;
		
		pstats->RecvSignalPower = rx_pwr_all;

		
		
		
	
	{
			u8	sq;

			if(pstats->RxPWDBAll > 40)
			{
				sq = 100;
			}else
			{
				sq = pcck_buf->sq_rpt;

				if(pcck_buf->sq_rpt > 64)
					sq = 0;
				else if (pcck_buf->sq_rpt < 20)
					sq = 100;
				else
					sq = ((64-sq) * 100) / 44;
			}
			pstats->SignalQuality = precord_stats->SignalQuality = sq;
			pstats->RxMIMOSignalQuality[0] = precord_stats->RxMIMOSignalQuality[0] = sq;
			pstats->RxMIMOSignalQuality[1] = precord_stats->RxMIMOSignalQuality[1] = -1;
		}
	}
	else
	{
		priv->stats.numqry_phystatusHT++;

		
		priv->brfpath_rxenable[0] = priv->brfpath_rxenable[1] = TRUE;

		
		
		
		
		for(i=RF90_PATH_A; i<RF90_PATH_MAX; i++)
		{
			
			if (priv->brfpath_rxenable[i])
				rf_rx_num++;
			
			

		
		

			
			
			
			rx_pwr[i] = ((pdrvinfo->gain_trsw[i]&0x3F)*2) - 110;

			
			RSSI = rtl819x_query_rxpwrpercentage(rx_pwr[i]);	
			total_rssi += RSSI;
			RT_TRACE(COMP_RF, "RF-%d RXPWR=%x RSSI=%d\n", i, rx_pwr[i], RSSI);

			
			
			
			
			
			priv->stats.rxSNRdB[i] = (long)(pdrvinfo->rxsnr[i]/2);

			
			
			

			
			
			{
				pstats->RxMIMOSignalStrength[i] =(u8) RSSI;
				precord_stats->RxMIMOSignalStrength[i] =(u8) RSSI;
			}
		}


		
		
		
		
		
		
		rx_pwr_all = (((pdrvinfo->pwdb_all ) >> 1 )& 0x7f) -106;
		pwdb_all = rtl819x_query_rxpwrpercentage(rx_pwr_all);

		pstats->RxPWDBAll = precord_stats->RxPWDBAll = pwdb_all;
		pstats->RxPower = precord_stats->RxPower =  rx_pwr_all;
		pstats->RecvSignalPower = rx_pwr_all;

		
		
		
		
		 
		 if(pDesc->RxHT && pDesc->RxMCS>=DESC92S_RATEMCS8 &&
		 	pDesc->RxMCS<=DESC92S_RATEMCS15)
			max_spatial_stream = 2; 
		else
			max_spatial_stream = 1; 

		for(i=0; i<max_spatial_stream; i++)
		{
			
			

			
			
			
			

			
			evm = rtl819x_evm_dbtopercentage( (pdrvinfo->rxevm[i] ));	
			RT_TRACE(COMP_RF, "RXRATE=%x RXEVM=%x EVM=%s%d\n", pDesc->RxMCS, pdrvinfo->rxevm[i], "%", evm);

			
			{
				if(i==0) 
					pstats->SignalQuality = precord_stats->SignalQuality = (u8)(evm & 0xff);
				pstats->RxMIMOSignalQuality[i] = precord_stats->RxMIMOSignalQuality[i] = (u8)(evm & 0xff);
			}
		}


		
		
		prxsc =	(phy_ofdm_rx_status_rxsc_sgien_exintfflag *)&rxsc_sgien_exflg;
		
		if(pDesc->BW)	
			priv->stats.received_bwtype[1+pdrvinfo->rxsc]++;
		else				
			priv->stats.received_bwtype[0]++;
	}

	
	
	if(is_cck_rate)
	{
		pstats->SignalStrength = precord_stats->SignalStrength = (u8)(rtl819x_signal_scale_mapping((long)pwdb_all));

	}
	else
	{
		
		
		if (rf_rx_num != 0)
			pstats->SignalStrength = precord_stats->SignalStrength = (u8)(rtl819x_signal_scale_mapping((long)(total_rssi/=rf_rx_num)));
	}
}

void
rtl8192_record_rxdesc_forlateruse(
	struct ieee80211_rx_stats *	psrc_stats,
	struct ieee80211_rx_stats *	ptarget_stats
)
{
	ptarget_stats->bIsAMPDU = psrc_stats->bIsAMPDU;
	ptarget_stats->bFirstMPDU = psrc_stats->bFirstMPDU;
	ptarget_stats->Seq_Num = psrc_stats->Seq_Num;
}

static void rtl8192SU_query_rxphystatus(
	struct r8192_priv * priv,
	struct ieee80211_rx_stats * pstats,
	rx_desc_819x_usb	*pDesc,
	rx_drvinfo_819x_usb  * pdrvinfo,
	struct ieee80211_rx_stats * precord_stats,
	bool bpacket_match_bssid,
	bool bpacket_toself,
	bool bPacketBeacon,
	bool bToSelfBA
	);
void rtl8192SU_TranslateRxSignalStuff(struct sk_buff *skb,
				   struct ieee80211_rx_stats * pstats,
				   rx_desc_819x_usb	*pDesc,
                                   rx_drvinfo_819x_usb  *pdrvinfo)
{
	
	rtl8192_rx_info *info = (struct rtl8192_rx_info *)skb->cb;
	struct net_device *dev=info->dev;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	bool bpacket_match_bssid, bpacket_toself;
	bool bPacketBeacon=FALSE, bToSelfBA=FALSE;
	static struct ieee80211_rx_stats  previous_stats;
	struct ieee80211_hdr_3addr *hdr;
       u16 fc,type;

	

	u8* tmp_buf;
	
	u8  *praddr;

	
	tmp_buf = (u8*)skb->data;

	hdr = (struct ieee80211_hdr_3addr *)tmp_buf;
	fc = le16_to_cpu(hdr->frame_ctl);
	type = WLAN_FC_GET_TYPE(fc);
	praddr = hdr->addr1;

	
	bpacket_match_bssid = ((IEEE80211_FTYPE_CTL != type) &&
                                			(eqMacAddr(priv->ieee80211->current_network.bssid,  (fc & IEEE80211_FCTL_TODS)? hdr->addr1 : (fc & IEEE80211_FCTL_FROMDS )? hdr->addr2 : hdr->addr3))
                                				 && (!pstats->bHwError) && (!pstats->bCRC)&& (!pstats->bICV));
	bpacket_toself =  bpacket_match_bssid & (eqMacAddr(praddr, priv->ieee80211->dev->dev_addr));

#if 1
		if(WLAN_FC_GET_FRAMETYPE(fc)== IEEE80211_STYPE_BEACON)
		{
			bPacketBeacon = true;
			
		}
		if(WLAN_FC_GET_FRAMETYPE(fc) == IEEE80211_STYPE_BLOCKACK)
		{
			if((eqMacAddr(praddr,dev->dev_addr)))
				bToSelfBA = true;
				
		}

#endif


	if(bpacket_match_bssid)
	{
		priv->stats.numpacket_matchbssid++;
	}
	if(bpacket_toself){
		priv->stats.numpacket_toself++;
	}
	
	
	
	
	
	rtl8192_process_phyinfo(priv, tmp_buf, &previous_stats, pstats);
	rtl8192SU_query_rxphystatus(priv, pstats, pDesc, pdrvinfo, &previous_stats, bpacket_match_bssid,bpacket_toself,bPacketBeacon,bToSelfBA);
	rtl8192_record_rxdesc_forlateruse(pstats, &previous_stats);

}


void
UpdateReceivedRateHistogramStatistics8190(
	struct net_device *dev,
	struct ieee80211_rx_stats *stats
	)
{
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
    	u32 rcvType=1;   
    	u32 rateIndex;
    	u32 preamble_guardinterval;  


    	if(stats->bCRC)
       	rcvType = 2;
   	else if(stats->bICV)
       	rcvType = 3;

   	if(stats->bShortPreamble)
       	preamble_guardinterval = 1;
    	else
       	preamble_guardinterval = 0;

	switch(stats->rate)
	{
		
		
		
		case MGN_1M:    rateIndex = 0;  break;
		case MGN_2M:    rateIndex = 1;  break;
		case MGN_5_5M:  rateIndex = 2;  break;
		case MGN_11M:   rateIndex = 3;  break;
		
		
		
		case MGN_6M:    rateIndex = 4;  break;
		case MGN_9M:    rateIndex = 5;  break;
		case MGN_12M:   rateIndex = 6;  break;
		case MGN_18M:   rateIndex = 7;  break;
		case MGN_24M:   rateIndex = 8;  break;
		case MGN_36M:   rateIndex = 9;  break;
		case MGN_48M:   rateIndex = 10; break;
		case MGN_54M:   rateIndex = 11; break;
		
		
		
		case MGN_MCS0:  rateIndex = 12; break;
		case MGN_MCS1:  rateIndex = 13; break;
		case MGN_MCS2:  rateIndex = 14; break;
		case MGN_MCS3:  rateIndex = 15; break;
		case MGN_MCS4:  rateIndex = 16; break;
		case MGN_MCS5:  rateIndex = 17; break;
		case MGN_MCS6:  rateIndex = 18; break;
		case MGN_MCS7:  rateIndex = 19; break;
		case MGN_MCS8:  rateIndex = 20; break;
		case MGN_MCS9:  rateIndex = 21; break;
		case MGN_MCS10: rateIndex = 22; break;
		case MGN_MCS11: rateIndex = 23; break;
		case MGN_MCS12: rateIndex = 24; break;
		case MGN_MCS13: rateIndex = 25; break;
		case MGN_MCS14: rateIndex = 26; break;
		case MGN_MCS15: rateIndex = 27; break;
		default:        rateIndex = 28; break;
	}
    priv->stats.received_preamble_GI[preamble_guardinterval][rateIndex]++;
    priv->stats.received_rate_histogram[0][rateIndex]++; 
    priv->stats.received_rate_histogram[rcvType][rateIndex]++;
}

void rtl8192SU_query_rxdesc_status(struct sk_buff *skb, struct ieee80211_rx_stats *stats, bool bIsRxAggrSubframe)
{
	rtl8192_rx_info *info = (struct rtl8192_rx_info *)skb->cb;
	struct net_device *dev=info->dev;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	
	rx_drvinfo_819x_usb  *driver_info = NULL;

	
	
	
	

	rx_desc_819x_usb *desc = (rx_desc_819x_usb *)skb->data;

	if(0)
	{
		int m = 0;
		printk("========================");
		for(m=0; m<skb->len; m++){
			if((m%32) == 0)
				printk("\n");
			printk("%2x ",((u8*)skb->data)[m]);
		}
		printk("\n========================\n");

	}


	
	
	
	stats->Length = desc->Length ;
	stats->RxDrvInfoSize = desc->RxDrvInfoSize*RX_DRV_INFO_SIZE_UNIT;
	stats->RxBufShift = (desc->Shift)&0x03;
	stats->bICV = desc->ICV;
	stats->bCRC = desc->CRC32;
	stats->bHwError = stats->bCRC|stats->bICV;
	stats->Decrypted = !desc->SWDec;
	stats->bIsAMPDU = (desc->AMSDU==1);
	stats->bFirstMPDU = (desc->PAGGR==1) && (desc->FAGGR==1);
	stats->bShortPreamble = desc->SPLCP;
	stats->RxIs40MHzPacket = (desc->BW==1);
	stats->TimeStampLow = desc->TSFL;

	if((desc->FAGGR==1) || (desc->PAGGR==1))
	{
		RT_TRACE(COMP_RXDESC, "FirstAGGR = %d, PartAggr = %d\n", desc->FAGGR, desc->PAGGR);
	}

if(stats->bHwError)
{
	if(stats->bICV)
		printk("%s: Receive ICV error!!!!!!!!!!!!!!!!!!!!!!\n", __FUNCTION__);
	if(stats->bCRC)
		printk("%s: Receive CRC error!!!!!!!!!!!!!!!!!!!!!!\n", __FUNCTION__);
}

	if(IS_UNDER_11N_AES_MODE(priv->ieee80211))
	{
		
		
		if(stats->bICV && !stats->bCRC)
		{
			stats->bICV = FALSE;
			stats->bHwError = FALSE;
		}
	}

	
	if(!stats->bHwError)
		
		
		
		
		stats->rate = rtl8192SU_HwRateToMRate(desc->RxHT, desc->RxMCS, desc->PAGGR);
	else
		stats->rate = MGN_1M;

	
	
	
	
	
	
	UpdateReceivedRateHistogramStatistics8190(dev, stats);
	
	UpdateRxPktTimeStamp8190(dev, stats);

	
	
	
	
	if (desc->PHYStatus)
	{
		
		driver_info = (rx_drvinfo_819x_usb *)(skb->data + sizeof(rx_desc_819x_usb) + \
				stats->RxBufShift);
		if(0)
		{
			int m = 0;
			printk("========================\n");
			printk("RX_DESC_SIZE:%d, RxBufShift:%d, RxDrvInfoSize:%d\n",
					RX_DESC_SIZE, stats->RxBufShift, stats->RxDrvInfoSize);
			for(m=0; m<32; m++){
			       printk("%2x ",((u8*)driver_info)[m]);
			}
			printk("\n========================\n");

		}

	}

	
	skb_pull(skb, sizeof(rx_desc_819x_usb));
	

	
	
	
	if((stats->RxBufShift + stats->RxDrvInfoSize) > 0)
	{
		stats->bShift = 1;
		
		skb_pull(skb, stats->RxBufShift + stats->RxDrvInfoSize);
		
	}

	
	
	
	
	if (desc->PHYStatus)
	{
		rtl8192SU_TranslateRxSignalStuff(skb, stats, desc, driver_info);
	}
}


















u32 GetRxPacketShiftBytes8192SU(struct ieee80211_rx_stats  *Status, bool bIsRxAggrSubframe)
{
	

	return (sizeof(rx_desc_819x_usb) + Status->RxDrvInfoSize + Status->RxBufShift);
}

void rtl8192SU_rx_nomal(struct sk_buff* skb)
{
	rtl8192_rx_info *info = (struct rtl8192_rx_info *)skb->cb;
	struct net_device *dev=info->dev;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	struct ieee80211_rx_stats stats = {
		.signal = 0,
		.noise = -98,
		.rate = 0,
		
		.freq = IEEE80211_24GHZ_BAND,
	};
	u32 rx_pkt_len = 0;
	struct ieee80211_hdr_1addr *ieee80211_hdr = NULL;
	bool unicast_packet = false;

	
	
	if((skb->len >=(20 + sizeof(rx_desc_819x_usb))) && (skb->len < RX_URB_SIZE)) {

		
		rtl8192SU_query_rxdesc_status(skb, &stats, false);
		

		
		priv->stats.rxoktotal++;  

		
		skb_trim(skb, skb->len - 4);

		rx_pkt_len = skb->len;
		ieee80211_hdr = (struct ieee80211_hdr_1addr *)skb->data;
		unicast_packet = false;
		if(is_broadcast_ether_addr(ieee80211_hdr->addr1)) {
			
		}else if(is_multicast_ether_addr(ieee80211_hdr->addr1)){
			
		}else {
			
			unicast_packet = true;
		}

		if(!ieee80211_rx(priv->ieee80211,skb, &stats)) {
			dev_kfree_skb_any(skb);
		} else {
		
			if(unicast_packet) {
				priv->stats.rxbytesunicast += rx_pkt_len;
			}
		}

		
	}
	else
	{
		priv->stats.rxurberr++;
		printk("actual_length:%d\n", skb->len);
		dev_kfree_skb_any(skb);
	}

}

void
rtl819xusb_process_received_packet(
	struct net_device *dev,
	struct ieee80211_rx_stats *pstats
	)
{

	u8* 	frame;
	u16     frame_len=0;
	struct r8192_priv *priv = ieee80211_priv(dev);


	
	

	
	
	pstats->virtual_address += get_rxpacket_shiftbytes_819xusb(pstats);
	frame = pstats->virtual_address;
	frame_len = pstats->packetlength;
#ifdef TODO	
	if(!Adapter->bInHctTest)
		CountRxErrStatistics(Adapter, pRfd);
#endif
	{
	#ifdef ENABLE_PS  
		RT_RF_POWER_STATE rtState;
		
		
		
		Adapter->HalFunc.GetHwRegHandler(Adapter, HW_VAR_RF_STATE, (u8* )(&rtState));
		if (rtState == eRfOff)
		{
			return;
		}
	#endif
	priv->stats.rxframgment++;

	}
#ifdef TODO
	RmMonitorSignalStrength(Adapter, pRfd);
#endif
	
	
	if (rtl819xusb_rx_command_packet(dev, pstats))
	{
		return;
	}

#ifdef SW_CRC_CHECK
	SwCrcCheck();
#endif


}

void query_rx_cmdpkt_desc_status(struct sk_buff *skb, struct ieee80211_rx_stats *stats)
{



	rx_desc_819x_usb *desc = (rx_desc_819x_usb *)skb->data;


	
	
	
	stats->virtual_address = (u8*)skb->data;
	stats->Length = desc->Length;
	stats->RxDrvInfoSize = 0;
	stats->RxBufShift = 0;
	stats->packetlength = stats->Length-scrclng;
	stats->fraglength = stats->packetlength;
	stats->fragoffset = 0;
	stats->ntotalfrag = 1;
}

void rtl8192SU_rx_cmd(struct sk_buff *skb)
{
	struct rtl8192_rx_info *info = (struct rtl8192_rx_info *)skb->cb;
	struct net_device *dev = info->dev;

	
	struct ieee80211_rx_stats stats = {
		.signal = 0,
		.noise = -98,
		.rate = 0,
		
		.freq = IEEE80211_24GHZ_BAND,
	};

	
	
	
	if( (skb->len >= sizeof(rx_desc_819x_usb)) && (skb->len <= RX_URB_SIZE) )
		
	{
		
		
		
		query_rx_cmdpkt_desc_status(skb,&stats);
		

		
		
		
		rtl819xusb_process_received_packet(dev,&stats);

		dev_kfree_skb_any(skb);
	}
	else
	{
		
		
		
			
	}

	
	
	
	
	

	
	
	
	

	RT_TRACE(COMP_RECV, "<--- HalUsbInCommandComplete8192SUsb()\n");

}

void rtl8192_irq_rx_tasklet(struct r8192_priv *priv)
{
        struct sk_buff *skb;
	struct rtl8192_rx_info *info;

        while (NULL != (skb = skb_dequeue(&priv->skb_queue))) {
		info = (struct rtl8192_rx_info *)skb->cb;
                switch (info->out_pipe) {
		
			case 3:
				
				priv->IrpPendingCount--;
				priv->ops->rtl819x_rx_nomal(skb);
				break;

				
			case 9:
				RT_TRACE(COMP_RECV, "command in-pipe index(%d)\n",\
						info->out_pipe);
				priv->ops->rtl819x_rx_cmd(skb);
				break;

			default: 
				RT_TRACE(COMP_ERR, "Unknown in-pipe index(%d)\n",\
						info->out_pipe);
				dev_kfree_skb(skb);
				break;

		}
        }
}





static void HalUsbSetQueuePipeMapping8192SUsb(struct usb_interface *intf, struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	u8 i = 0;

	priv->ep_in_num = 0;
	priv->ep_out_num = 0;
	memset(priv->RtOutPipes,0,16);
	memset(priv->RtInPipes,0,16);

	iface_desc = intf->cur_altsetting;
	priv->ep_num = iface_desc->desc.bNumEndpoints;

	for (i = 0; i < priv->ep_num; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;
		if (usb_endpoint_is_bulk_in(endpoint)) {
			priv->RtInPipes[priv->ep_in_num] = usb_endpoint_num(endpoint);
			priv->ep_in_num ++;
			
		} else if (usb_endpoint_is_bulk_out(endpoint)) {
			priv->RtOutPipes[priv->ep_out_num] = usb_endpoint_num(endpoint);
			priv->ep_out_num ++;
			
		}
	}
	{
		memset(priv->txqueue_to_outpipemap,0,9);
		if (priv->ep_num == 6) {
			
			u8 queuetopipe[] = {3, 2, 1, 0, 4, 4, 4, 4, 4};

			memcpy(priv->txqueue_to_outpipemap,queuetopipe,9);
		} else if (priv->ep_num == 4) {
			
			u8 queuetopipe[] = {1, 1, 0, 0, 2, 2, 2, 2, 2};

			memcpy(priv->txqueue_to_outpipemap,queuetopipe,9);
		} else if (priv->ep_num > 9) {
			
			u8 queuetopipe[] = {3, 2, 1, 0, 4, 8, 7, 6, 5};

			memcpy(priv->txqueue_to_outpipemap,queuetopipe,9);
		} else {
			
			u8 queuetopipe[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
			memcpy(priv->txqueue_to_outpipemap,queuetopipe,9);
		}
	}
	printk("==>ep_num:%d, in_ep_num:%d, out_ep_num:%d\n", priv->ep_num, priv->ep_in_num, priv->ep_out_num);

	printk("==>RtInPipes:");
	for(i=0; i < priv->ep_in_num; i++)
		printk("%d  ", priv->RtInPipes[i]);
	printk("\n");

	printk("==>RtOutPipes:");
	for(i=0; i < priv->ep_out_num; i++)
		printk("%d  ", priv->RtOutPipes[i]);
	printk("\n");

	printk("==>txqueue_to_outpipemap for BK, BE, VI, VO, HCCA, TXCMD, MGNT, HIGH, BEACON:\n");
	for(i=0; i < 9; i++)
		printk("%d  ", priv->txqueue_to_outpipemap[i]);
	printk("\n");

	return;
}

static const struct net_device_ops rtl8192_netdev_ops = {
	.ndo_open		= rtl8192_open,
	.ndo_stop		= rtl8192_close,
	.ndo_get_stats		= rtl8192_stats,
	.ndo_tx_timeout		= tx_timeout,
	.ndo_do_ioctl		= rtl8192_ioctl,
	.ndo_set_multicast_list	= r8192_set_multicast,
	.ndo_set_mac_address	= r8192_set_mac_adr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_start_xmit		= rtl8192_ieee80211_xmit,
};

static int __devinit rtl8192_usb_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{

	struct net_device *dev = NULL;
	struct r8192_priv *priv= NULL;
	struct usb_device *udev = interface_to_usbdev(intf);

        RT_TRACE(COMP_INIT, "Oops: i'm coming\n");

	dev = alloc_ieee80211(sizeof(struct r8192_priv));

	usb_set_intfdata(intf, dev);
	SET_NETDEV_DEV(dev, &intf->dev);
	priv = ieee80211_priv(dev);
	priv->ieee80211 = netdev_priv(dev);
	priv->udev=udev;

	HalUsbSetQueuePipeMapping8192SUsb(intf, dev);

	
	priv->ops = &rtl8192su_ops;

	dev->netdev_ops = &rtl8192_netdev_ops;

         
        dev->wireless_handlers = (struct iw_handler_def *) &r8192_wx_handlers_def;

	dev->type=ARPHRD_ETHER;

	dev->watchdog_timeo = HZ*3;	

	if (dev_alloc_name(dev, ifname) < 0){
                RT_TRACE(COMP_INIT, "Oops: devname already taken! Trying wlan%%d...\n");
		ifname = "wlan%d";
		dev_alloc_name(dev, ifname);
        }

	RT_TRACE(COMP_INIT, "Driver probe completed1\n");
#if 1
	if(rtl8192_init(dev)!=0){
		RT_TRACE(COMP_ERR, "Initialization failed");
		goto fail;
	}
#endif
	netif_carrier_off(dev);
	netif_stop_queue(dev);

	register_netdev(dev);
	RT_TRACE(COMP_INIT, "dev name=======> %s\n",dev->name);
	rtl8192_proc_init_one(dev);


	RT_TRACE(COMP_INIT, "Driver probe completed\n");
	return 0;
fail:
	free_ieee80211(dev);

	RT_TRACE(COMP_ERR, "wlan driver load failed\n");
	return -ENODEV;
}


void rtl8192_cancel_deferred_work(struct r8192_priv* priv)
{
	cancel_work_sync(&priv->reset_wq);
	cancel_work_sync(&priv->qos_activate);
	cancel_delayed_work(&priv->watch_dog_wq);
	cancel_delayed_work(&priv->update_beacon_wq);
	cancel_delayed_work(&priv->ieee80211->hw_wakeup_wq);
	cancel_delayed_work(&priv->ieee80211->hw_sleep_wq);
	
	
}

static void __devexit rtl8192_usb_disconnect(struct usb_interface *intf)
{
	struct net_device *dev = usb_get_intfdata(intf);
	struct r8192_priv *priv = ieee80211_priv(dev);
 	if(dev){

		unregister_netdev(dev);

		RT_TRACE(COMP_DOWN, "=============>wlan driver to be removed\n");
		rtl8192_proc_remove_one(dev);

			rtl8192_down(dev);
		if (priv->pFirmware)
		{
			vfree(priv->pFirmware);
			priv->pFirmware = NULL;
		}
	

		destroy_workqueue(priv->priv_wq);
		
		
		mdelay(10);

	}
	free_ieee80211(dev);
	RT_TRACE(COMP_DOWN, "wlan driver removed\n");
}


extern int ieee80211_debug_init(void);
extern void ieee80211_debug_exit(void);
extern int ieee80211_crypto_init(void);
extern void ieee80211_crypto_deinit(void);
extern int ieee80211_crypto_tkip_init(void);
extern void ieee80211_crypto_tkip_exit(void);
extern int ieee80211_crypto_ccmp_init(void);
extern void ieee80211_crypto_ccmp_exit(void);
extern int ieee80211_crypto_wep_init(void);
extern void ieee80211_crypto_wep_exit(void);

static int __init rtl8192_usb_module_init(void)
{
	int ret;

#ifdef CONFIG_IEEE80211_DEBUG
	ret = ieee80211_debug_init();
	if (ret) {
		printk(KERN_ERR "ieee80211_debug_init() failed %d\n", ret);
		return ret;
	}
#endif
	ret = ieee80211_crypto_init();
	if (ret) {
		printk(KERN_ERR "ieee80211_crypto_init() failed %d\n", ret);
		return ret;
	}

	ret = ieee80211_crypto_tkip_init();
	if (ret) {
		printk(KERN_ERR "ieee80211_crypto_tkip_init() failed %d\n",
			ret);
		return ret;
	}

	ret = ieee80211_crypto_ccmp_init();
	if (ret) {
		printk(KERN_ERR "ieee80211_crypto_ccmp_init() failed %d\n",
			ret);
		return ret;
	}

	ret = ieee80211_crypto_wep_init();
	if (ret) {
		printk(KERN_ERR "ieee80211_crypto_wep_init() failed %d\n", ret);
		return ret;
	}

	printk(KERN_INFO "\nLinux kernel driver for RTL8192 based WLAN cards\n");
	printk(KERN_INFO "Copyright (c) 2007-2008, Realsil Wlan\n");
	RT_TRACE(COMP_INIT, "Initializing module");
	RT_TRACE(COMP_INIT, "Wireless extensions version %d", WIRELESS_EXT);
	rtl8192_proc_module_init();
	return usb_register(&rtl8192_usb_driver);
}


static void __exit rtl8192_usb_module_exit(void)
{
	usb_deregister(&rtl8192_usb_driver);

	RT_TRACE(COMP_DOWN, "Exiting");
	rtl8192_proc_module_remove();

	ieee80211_crypto_tkip_exit();
	ieee80211_crypto_ccmp_exit();
	ieee80211_crypto_wep_exit();
	ieee80211_crypto_deinit();
#ifdef CONFIG_IEEE80211_DEBUG
	ieee80211_debug_exit();
#endif
}


void rtl8192_try_wake_queue(struct net_device *dev, int pri)
{
	unsigned long flags;
	short enough_desc;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);

	spin_lock_irqsave(&priv->tx_lock,flags);
	enough_desc = check_nic_enough_desc(dev,pri);
        spin_unlock_irqrestore(&priv->tx_lock,flags);

	if(enough_desc)
		ieee80211_wake_queue(priv->ieee80211);
}

void EnableHWSecurityConfig8192(struct net_device *dev)
{
        u8 SECR_value = 0x0;
	struct r8192_priv *priv = (struct r8192_priv *)ieee80211_priv(dev);
	 struct ieee80211_device* ieee = priv->ieee80211;

	SECR_value = SCR_TxEncEnable | SCR_RxDecEnable;
#if 1
	if (((KEY_TYPE_WEP40 == ieee->pairwise_key_type) || (KEY_TYPE_WEP104 == ieee->pairwise_key_type)) && (priv->ieee80211->auth_mode != 2))
	{
		SECR_value |= SCR_RxUseDK;
		SECR_value |= SCR_TxUseDK;
	}
	else if ((ieee->iw_mode == IW_MODE_ADHOC) && (ieee->pairwise_key_type & (KEY_TYPE_CCMP | KEY_TYPE_TKIP)))
	{
		SECR_value |= SCR_RxUseDK;
		SECR_value |= SCR_TxUseDK;
	}
#endif
        


	ieee->hwsec_active = 1;

	if ((ieee->pHTInfo->IOTAction&HT_IOT_ACT_PURE_N_MODE) || !hwwep)
	{
		ieee->hwsec_active = 0;
		SECR_value &= ~SCR_RxDecEnable;
	}

	RT_TRACE(COMP_SEC,"%s:, hwsec:%d, pairwise_key:%d, SECR_value:%x\n", __FUNCTION__, \
			ieee->hwsec_active, ieee->pairwise_key_type, SECR_value);
	{
                write_nic_byte(dev, SECR,  SECR_value);
        }
}


void setKey(	struct net_device *dev,
		u8 EntryNo,
		u8 KeyIndex,
		u16 KeyType,
		u8 *MacAddr,
		u8 DefaultKey,
		u32 *KeyContent )
{
	u32 TargetCommand = 0;
	u32 TargetContent = 0;
	u16 usConfig = 0;
	u8 i;
	if (EntryNo >= TOTAL_CAM_ENTRY)
		RT_TRACE(COMP_ERR, "cam entry exceeds in setKey()\n");

	RT_TRACE(COMP_SEC, "====>to setKey(), dev:%p, EntryNo:%d, KeyIndex:%d, KeyType:%d, MacAddr"MAC_FMT"\n", dev,EntryNo, KeyIndex, KeyType, MAC_ARG(MacAddr));

	if (DefaultKey)
		usConfig |= BIT15 | (KeyType<<2);
	else
		usConfig |= BIT15 | (KeyType<<2) | KeyIndex;



	for(i=0 ; i<CAM_CONTENT_COUNT; i++){
		TargetCommand  = i+CAM_CONTENT_COUNT*EntryNo;
		TargetCommand |= BIT31|BIT16;

		if(i==0){
			TargetContent = (u32)(*(MacAddr+0)) << 16|
					(u32)(*(MacAddr+1)) << 24|
					(u32)usConfig;

			write_nic_dword(dev, WCAMI, TargetContent);
			write_nic_dword(dev, RWCAM, TargetCommand);
	
		}
		else if(i==1){
                        TargetContent = (u32)(*(MacAddr+2)) 	 |
                                        (u32)(*(MacAddr+3)) <<  8|
                                        (u32)(*(MacAddr+4)) << 16|
                                        (u32)(*(MacAddr+5)) << 24;
			write_nic_dword(dev, WCAMI, TargetContent);
			write_nic_dword(dev, RWCAM, TargetCommand);
		}
		else {
			
			if(KeyContent !=NULL){
			write_nic_dword(dev, WCAMI, (u32)(*(KeyContent+i-2)) );
			write_nic_dword(dev, RWCAM, TargetCommand);
		}
	}
	}

}


module_init(rtl8192_usb_module_init);
module_exit(rtl8192_usb_module_exit);
