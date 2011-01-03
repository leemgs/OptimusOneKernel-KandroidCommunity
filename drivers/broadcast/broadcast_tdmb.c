#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>        

#include <linux/io.h>
#include <mach/hardware.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/memory.h>
#include <mach/gpio.h>
#include <mach/board_lge.h>
#include <linux/wakelock.h> 		
#include <linux/workqueue.h>		


#include <linux/pm_qos_params.h>
#include <mach/camera.h>

#include <linux/broadcast/broadcast_tdmb_typedef.h>
#include <linux/broadcast/broadcast_tdmb.h>
#include <linux/broadcast/broadcast_lg2102_ioctrl.h>


#define TDMB_EBI2_ISR




#define inp(port)		  (*((volatile byte *) (port)))
#define inpw(port)		  (*((volatile word *) (port)))
#define inpdw(port) 	  (*((volatile dword *)(port)))
	
#define outp(port, val)   (*((volatile byte *) (port)) = ((byte) (val)))
#define outpw(port, val)  (*((volatile word *) (port)) = ((word) (val)))
#define outpdw(port, val) (*((volatile dword *) (port)) = ((dword) (val)))


#define TDMB_CLASS_NAME "broadcast_tdmb"

#define DMB_POWER_GPIO    124
#define DMB_RESET_GPIO    100
#define DMB_INT_GPIO       99

#define BROADCAST_INT_SIZE (188*16)

#define BROADCAST_DATA_CHUNK_NUM  48
#define BROADCAST_DATA_CHUNK_SIZE (BROADCAST_DATA_CHUNK_NUM*BROADCAST_INT_SIZE)
#define BROADCAST_TDMB_NUM_DEVS 1 


#define BROADCAST_AXI_QOS_RATE 200000
#define MSM_DMB_AXI_QOS_NAME  "broadcast_dmb"


#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_SU370) || defined(CONFIG_MACH_MSM7X27_KU3700) || defined(CONFIG_MACH_MSM7X27_LU3700) || defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
#define  CONFIG_BROADCAST_USE_LUT
#endif

struct broadcast_tdmb_chdevice {
	struct cdev cdev;
	struct device *dev;
	resource_size_t ebi2_phys_base;
	void __iomem *ebi2_virt_base;
	resource_size_t ebi2_cr_phys_base;	
	void __iomem *ebi2_cr_virt_base;
	resource_size_t ebi2_xm_phys_base;	
	void __iomem *ebi2_xm_virt_base;
	uint8    *gp_buffer;
	uint32   ts_isr_count;
	uint8    ts_buffer_widx;
	uint8    ts_buffer_ridx;
	unsigned int irq;
	unsigned int power_enable;
	unsigned int reset;
	unsigned int isr_gpio;
	struct wake_lock wake_lock;	
#ifdef READ_USE_WORKQUEUE	
	struct work_struct			ebi_r_work;
	struct workqueue_struct* 	ebi_r_wq;
#endif	

};


static unsigned int buf_idx_limit =  2;
static unsigned int buf_len_limit = BROADCAST_INT_SIZE*2;
static unsigned int packet_cnt_limit = 32;
static uint32	      isr_setup_flag = FALSE;





static  uint8			*gpDMB_DATA_Buffer = NULL;
static uint8			*gDMB_BBBuffer[BROADCAST_DATA_CHUNK_NUM];
static uint8 g_ts_buffer[BROADCAST_DATA_CHUNK_SIZE];



static struct class *broadcast_tdmb_class;
static dev_t broadcast_tdmb_dev;
static struct broadcast_tdmb_chdevice *pbroadcast;


#ifdef CONFIG_BROADCAST_USE_LUT
static int dmb_lut_table = -1;
#endif

static int broadcast_tdmb_probe(struct platform_device *pdev);
static irqreturn_t broadcast_tdmb_isr(int irq, void *handle);

static struct platform_driver  broadcast_tdmb_driver = {
	.probe = broadcast_tdmb_probe,
	.driver = {
		.name = "tdmb_lg2102",
		.owner = THIS_MODULE,
	},
};



static uint32_t tdmb_off_gpio_table[] = {
	GPIO_CFG(DMB_POWER_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(DMB_RESET_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	
};

static uint32_t tdmb_on_gpio_table[] = {
	GPIO_CFG(DMB_POWER_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(DMB_RESET_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	
};

#ifdef CONFIG_BROADCAST_USE_LUT

extern mdp_load_thunder_lut(int lut_type);
#endif

static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_ENABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

void config_tdmb_on_gpios(void)
{
	config_gpio_table(tdmb_on_gpio_table,
		ARRAY_SIZE(tdmb_on_gpio_table));
}

void config_tdmb_off_gpios(void)
{
	config_gpio_table(tdmb_off_gpio_table,
		ARRAY_SIZE(tdmb_off_gpio_table));
}




static int broadcast_tdmb_add_axi_qos(void)
{
	int rc = 0;

	rc = pm_qos_add_requirement(PM_QOS_SYSTEM_BUS_FREQ,
		MSM_DMB_AXI_QOS_NAME, PM_QOS_DEFAULT_VALUE);
	if (rc < 0)
	{
		printk("request DMB AXI bus QOS fails. rc = %d\n", rc);
	}
	return rc;
}

static void broadcast_tdmb_release_axi_qos(void)
{
	pm_qos_remove_requirement(PM_QOS_SYSTEM_BUS_FREQ,
		MSM_DMB_AXI_QOS_NAME);
}


static int broadcast_tdmb_axi_qos_on(int  rate)
{
	int rc = 0;
	
	rc = pm_qos_update_requirement(PM_QOS_SYSTEM_BUS_FREQ,
		MSM_DMB_AXI_QOS_NAME, rate);
	if (rc < 0)
	{
		printk("update AXI bus QOS fails. rc = %d\n", rc);
	}
	return rc;
}

#ifdef READ_USE_WORKQUEUE
static void broadcast_tdmb_work_handler(struct work_struct *pdmb_work)
{
	uint8* 	read_p 	= NULL;
	uint32 	read_size 	= 0;
	int		w_i = 0;
	int		r_i = 0;

	struct broadcast_tdmb_chdevice *the_dev =
		container_of(pdmb_work, struct broadcast_tdmb_chdevice, ebi_r_work);

	if((the_dev == NULL) ||(gpDMB_DATA_Buffer == NULL))
	{
		printk("tdmb_irq broadcast_tdmb_chdevice pointer is NULL\n");	
		gpio_clear_detect_status(DMB_INT_GPIO);
		enable_irq(the_dev->irq);
		return;
	}

	w_i = the_dev->ts_buffer_widx;
	r_i = the_dev->ts_buffer_ridx;

	if(r_i == ((w_i + 1)%BROADCAST_DATA_CHUNK_NUM))
	{
		printk("tdmb_irq read & write index + 1 is equql ridx:%x widx:%x \n", 
			the_dev->ts_buffer_ridx,the_dev->ts_buffer_widx);	
		gpio_clear_detect_status(DMB_INT_GPIO);
		enable_irq(the_dev->irq);
		return;
	}

	read_p = gpDMB_DATA_Buffer + w_i*BROADCAST_INT_SIZE;
	gDMB_BBBuffer[w_i] = read_p;
	
	the_dev->ts_isr_count++;

	tunerbb_drv_lg2102_read_data(read_p, &read_size);
	
	
 	the_dev->ts_buffer_widx = ((w_i + 1)% BROADCAST_DATA_CHUNK_NUM );
	gpio_clear_detect_status(DMB_INT_GPIO);
	enable_irq(the_dev->irq);
}
#endif


static irqreturn_t broadcast_tdmb_isr(int irq, void *handle)
{
#ifdef READ_USE_WORKQUEUE
	struct broadcast_tdmb_chdevice *the_dev =	(struct broadcast_tdmb_chdevice*)handle;
	disable_irq_nosync(the_dev->irq);
	queue_work(the_dev->ebi_r_wq, &the_dev->ebi_r_work);
#else
	uint8* 	read_p 	= NULL;
	uint32 	read_size 	= 0;
	int		w_i = 0;
	int		r_i = 0;


	struct broadcast_tdmb_chdevice *p_tdmbdev = handle;

	if((p_tdmbdev == NULL)||(gpDMB_DATA_Buffer == NULL))
	{
		printk("tdmb_irq broadcast_tdmb_chdevice pointer is NULL\n");	
		gpio_clear_detect_status(DMB_INT_GPIO);

		return IRQ_HANDLED;
	}

	w_i = p_tdmbdev->ts_buffer_widx;
	r_i = p_tdmbdev->ts_buffer_ridx;

	if(r_i == ((w_i + 1)%BROADCAST_DATA_CHUNK_NUM))
	{
		printk("tdmb_irq read & write index + 1 is equql ridx:%x widx:%x \n", 
			p_tdmbdev->ts_buffer_ridx,p_tdmbdev->ts_buffer_widx);	
		gpio_clear_detect_status(DMB_INT_GPIO);
		return IRQ_HANDLED;
	}

	read_p = gpDMB_DATA_Buffer + w_i*BROADCAST_INT_SIZE;
	
	gDMB_BBBuffer[w_i] = read_p;
	
	p_tdmbdev->ts_isr_count++;
	
	tunerbb_drv_lg2102_read_data(read_p, &read_size);

 	p_tdmbdev->ts_buffer_widx = ((w_i + 1)% BROADCAST_DATA_CHUNK_NUM );

	gpio_clear_detect_status(DMB_INT_GPIO);
#endif
	return IRQ_HANDLED;

}


static int broadcast_tdmb_probe(struct platform_device *pdev)
{
	int ret;
	

	struct resource *res;	

	gpio_tlmm_config( GPIO_CFG(DMB_POWER_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), GPIO_ENABLE ) ;
	gpio_configure( DMB_POWER_GPIO, GPIOF_DRIVE_OUTPUT);

	gpio_tlmm_config( GPIO_CFG(DMB_RESET_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), GPIO_ENABLE ) ;
	gpio_configure( DMB_RESET_GPIO, GPIOF_DRIVE_OUTPUT);

	

	pbroadcast->isr_gpio = DMB_INT_GPIO ;

	pbroadcast->irq = platform_get_irq_byname(pdev, "dmb_ebi2_int");
	if (unlikely(pbroadcast->irq < 0)) {
		printk(KERN_ERR "%s(): Invalid irq = %d\n", __func__,
					 pbroadcast->irq);
		return -ENOMEM;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
				"dmb_ebi2_phys_memory");
	if (!res) {
		printk("couldn't find dmb_ebi2_phys_memory\n");
		ret = -ENODEV;
		return ret;
	}
	
	printk("ebi2_base start = %x, end = %x, name = %s!\n", res->start, res->end, res->name);
	
	pbroadcast->ebi2_phys_base = res->start;
	
	pbroadcast->ebi2_virt_base = ioremap(res->start , res->end - res->start + 1);

	if (!pbroadcast->ebi2_virt_base)
	{
		printk(KERN_ERR
			"ebi2_base ioremap failed!\n");
		return -ENOMEM;
	}

	printk("pbroadcast->ebi2_virt_base = %x\n", pbroadcast->ebi2_virt_base);


	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
				"dmb_ebi2_xm_base");
	if (!res) {
		printk("couldn't find dmb_ebi2_phys_memory\n");
		ret = -ENODEV;
		return ret;
	}
	
	pbroadcast->ebi2_xm_phys_base = res->start;
	
	pbroadcast->ebi2_xm_virt_base = ioremap(res->start , res->end - res->start + 1);

	if (!pbroadcast->ebi2_xm_virt_base)
	{
		printk(KERN_ERR
			"dmb_ebi2_xm_base failed!\n");
		return -ENOMEM;
	}

	
	 outpdw(pbroadcast->ebi2_xm_virt_base + 0x0008 + 0x10, 0xCC070744);
	 outpdw(pbroadcast->ebi2_xm_virt_base + 0x0028 + 0x10, 0x01010020);
	
		
	(void) tunerbb_drv_lg2102_set_ebi2_address(pbroadcast->ebi2_virt_base);
	
	wake_lock_init(&pbroadcast->wake_lock,  WAKE_LOCK_SUSPEND,
		       dev_name(&pdev->dev));

#ifdef READ_USE_WORKQUEUE
	INIT_WORK(&pbroadcast->ebi_r_work, broadcast_tdmb_work_handler);
	pbroadcast->ebi_r_wq = create_singlethread_workqueue("dmb_r_workqueue");
	if(pbroadcast->ebi_r_wq == NULL)
	{
		printk("Fail to setup DMB ebi read workqueue\n");
	}
#endif
	printk("broadcast_tdmb_lg2102_probe\n");
      
	return 0;

}

static int8 broadcast_tdmb_setup_isr(void)
{
	int ret;
	
	if(isr_setup_flag == 1)
	{
		printk("[broadcast_irq]broadcast_tdmb_setup_isr immediately done\n");
		return OK;
	}
	
	ret= gpio_tlmm_config(GPIO_CFG(DMB_INT_GPIO, 0, GPIO_INPUT,GPIO_PULL_UP, GPIO_2MA), GPIO_ENABLE);
	if (ret < 0) {
		printk(KERN_ERR
			"broadcast_tdmb_power_on for  %s"
			"(rc = %d)\n", "gpio_tlmm_config", ret);
		ret = -EIO;
	}
	
	
	ret = gpio_request(DMB_INT_GPIO, "dmb_ebi2_int");
	if (ret < 0) {
		printk(KERN_ERR
			"broadcast_tdmb_power_on for  %s "
			"(rc = %d)\n", "gpio_request", ret);
		ret = -EIO;
	}
	
	pbroadcast->irq = MSM_GPIO_TO_INT(DMB_INT_GPIO);
	gpio_clear_detect_status(DMB_INT_GPIO);	
	ret = request_irq(pbroadcast->irq , &broadcast_tdmb_isr ,
				(IRQF_DISABLED | IRQF_TRIGGER_FALLING) , "tdmb_lg2102", pbroadcast);
		
	if (ret < 0) {
		printk(KERN_ERR
			"Could not request_irq	for  %s interrupt "
			"(rc = %d)\n", "tdmb_lg2102", ret);
		ret = -EIO;
	}
	

	printk("[broadcast_irq]broadcast_tdmb_setup_isr = (0x%X)\n", pbroadcast->irq);
	isr_setup_flag = 1;
	return OK;
}

static int8 broadcast_tdmb_release_isr(void)
{
	
	if(isr_setup_flag == 0)
	{
		printk("[broadcast_irq]broadcast_tdmb_release_isr immediately done\n");
		return OK;
	}

	free_irq(pbroadcast->irq, pbroadcast);
	isr_setup_flag = 0;
	printk("[broadcast_irq]broadcast_tdmb_release_isr irq =(0x%X)\n", pbroadcast->irq);
	return OK;
}


static int8 broadcast_tdmb_power_on(void)
{
	int8 res = ERROR;
	int ret;
	boolean retval = FALSE;

	broadcast_tdmb_axi_qos_on(BROADCAST_AXI_QOS_RATE);
	
	wake_lock(&pbroadcast->wake_lock);
	retval = tunerbb_drv_lg2102_power_on();
	
	if(retval == TRUE)
	{
		res = OK;
	}
	tunerbb_drv_lg2102_set_userstop( );
	

	printk("broadcast_tdmb_power_on \n");

	return res;
}

static int8 broadcast_tdmb_power_off(void)
{
	int8 res = ERROR;
	boolean retval = FALSE;

	retval = tunerbb_drv_lg2102_power_off();
	wake_unlock(&pbroadcast->wake_lock);
	broadcast_tdmb_axi_qos_on(PM_QOS_DEFAULT_VALUE );
	
	if(retval == TRUE)
	{
		res = OK;
	}
	tunerbb_drv_lg2102_set_userstop( );
	

	return res;
}

static int8 broadcast_tdmb_open(void)
{
	int8 res = ERROR;
	boolean retval = FALSE;

	printk("broadcast_tdmb_open inini\n");

	
	retval = tunerbb_drv_lg2102_init();

	
	if(retval == FALSE)
	{
		tunerbb_drv_lg2102_power_off( );
		tunerbb_drv_lg2102_power_on( );
		retval = tunerbb_drv_lg2102_init( );
	}
	
	
	
	
	if(retval == TRUE)
	{
		res = OK;
	}
	printk("broadcast_tdmb_open out\n");

#ifdef CONFIG_BROADCAST_USE_LUT
	dmb_lut_table = -1;
#endif
	return res;
}

static int8 broadcast_tdmb_close(void)
{
	int8 res = ERROR;
	boolean retval = FALSE;

	
	broadcast_tdmb_release_isr( );

	
	retval = tunerbb_drv_lg2102_stop();

	if(gpDMB_DATA_Buffer)
	{
		gpDMB_DATA_Buffer = NULL;
	}
	
	if(retval == TRUE)
	{
		res = OK;
	}

#ifdef CONFIG_BROADCAST_USE_LUT
	if(dmb_lut_table == 0)
	{
		mdp_load_thunder_lut(1);
		dmb_lut_table = -1;
	}
#endif
	return res;
}

static int8 broadcast_tdmb_tune(void __user *arg)
{
	int8 rc = ERROR;
	boolean retval = FALSE;
	int udata;
	
	if(copy_from_user(&udata, arg, sizeof(int)))
	{	
		
		printk("broadcast_tdmb_tune fail!!! udata = %d\n", udata);
		rc = ERROR;
	}
	else
	{
		
		tunerbb_drv_lg2102_stop();
		retval = tunerbb_drv_lg2102_tune(udata);
		if(retval == TRUE)
		{
			rc = OK;
		}
	}

	return rc;

}

static int broadcast_tdmb_tune_set_ch(void __user *arg)
{
	int8 ret = ERROR;
	boolean retval = FALSE;
	struct broadcast_tdmb_set_ch_info udata;

	if(copy_from_user(&udata, arg, sizeof(struct broadcast_tdmb_set_ch_info)))
	{	
		
		printk("broadcast_tdmb_set_ch fail!!! \n");
		ret = ERROR;
	}
	else
	{
		printk("broadcast_tdmb_set_ch ch_num = %d, mode = %d, sub_ch_id = %d \n", udata.ch_num, udata.mode, udata.sub_ch_id);
		
		

		
		
		if(udata.mode == 1) 
		{
			buf_idx_limit = 2;
		}
		else if(udata.mode == 2) 
		{
			buf_idx_limit = 4;
		}
		else if(udata.mode ==3)
		{
			buf_idx_limit = 1;
		}
		else
		{
			buf_idx_limit = 4;
		}
		
		buf_len_limit = BROADCAST_INT_SIZE*buf_idx_limit;
		packet_cnt_limit = 16*buf_idx_limit;

		
		broadcast_tdmb_release_isr( );
		
		retval = tunerbb_drv_lg2102_set_channel(udata.ch_num, udata.sub_ch_id, udata.mode);
#ifdef TDMB_EBI2_ISR
		if ( pbroadcast != NULL)
		{
			pbroadcast->ts_buffer_ridx = 0;
			pbroadcast->ts_buffer_widx = 0;
			pbroadcast->ts_isr_count = 0;
			gpDMB_DATA_Buffer = &g_ts_buffer[0];
		}

		
		broadcast_tdmb_setup_isr( );
#endif		
		if(retval == TRUE)
		{
			ret = OK;
		}

#ifdef CONFIG_BROADCAST_USE_LUT
		if((dmb_lut_table == -1) 
			&& ((udata.mode == 2)||(udata.mode == 3)))
		{
			mdp_load_thunder_lut(3);
			dmb_lut_table = 0;
		}
#endif
	}

	return ret;

}

static int broadcast_tdmb_resync(void __user *arg)
{
	#if 0
	int rc;
	int udata;
	
	copy_from_user(&udata, arg, sizeof(int));
	#endif
	return 0;

}

static int broadcast_tdmb_detect_sync(void __user *arg)
{
	int8 rc = ERROR;
	boolean retval = FALSE;
	int udata;
	int __user* puser = (int __user*)arg;
	udata = *puser;

	retval = tunerbb_drv_lg2102_re_syncdetector(udata);

	if(retval == TRUE)
	{
		rc = OK;
	}
	return rc;
#if 0	
	if(copy_from_user(&udata, arg, sizeof(int)))
	{	
		
		printk("broadcast_tdmb_tune fail!!! udata = %d\n", udata);
		rc = ERROR;
	}
	else
	{
		retval = tunerbb_drv_lg2102_re_syncdetector(udata);
		if(retval == TRUE)
		{
			rc = OK;
		}
	}
#endif
	return rc;
}


static int broadcast_tdmb_get_sig_info(void __user *arg)
{
	int rc = ERROR;
	boolean retval = FALSE;
	
	struct broadcast_tdmb_sig_info udata;

	memset((void*)&udata, 0x00, sizeof(struct broadcast_tdmb_sig_info));

	disable_irq_nosync(pbroadcast->irq);
	retval = tunerbb_drv_lg2102_get_ber(&udata);
	enable_irq(pbroadcast->irq);

	if(retval == TRUE)
	{
		rc = OK;
	}

	if(copy_to_user((void*)arg, (void*)&udata, sizeof(struct broadcast_tdmb_sig_info)))
	{
		printk("broadcast_tdmb_get_sig_info copy_to_user error!!\n");
		rc = ERROR;
	}
	else
	{
		rc= OK;
	}

	return rc;
}

static int broadcast_tdmb_get_ch_info(void __user *arg)
{
	int rc = ERROR;
	boolean retval = FALSE;
	uint8 fic_kernel_buffer[400];
	uint32 fic_len = 0;

	struct broadcast_tdmb_get_ch_info __user* puserdata = (struct broadcast_tdmb_get_ch_info __user*)arg;

	if((puserdata == NULL)||( puserdata->ch_buf == NULL))
	{
		printk("broadcast_tdmb_get_ch_info argument error\n");
		return rc;
	}

	memset(fic_kernel_buffer, 0x00, sizeof(fic_kernel_buffer));

	retval = tunerbb_drv_lg2102_get_fic(fic_kernel_buffer, &fic_len ,TRUE);
	
	if(retval == TRUE)
	{
		if(copy_to_user((void __user*)puserdata->ch_buf, (void*)fic_kernel_buffer, fic_len))
		{
			fic_len = 0;
			rc = ERROR;
		}
		else
		{
			rc = OK;
		}
	}
	else
	{
		fic_len = 0;
		rc = ERROR;
	}
	puserdata->buf_len = fic_len;
	
	return rc;
}

static int broadcast_tdmb_get_dmb_data(void __user *arg)
{	
	int rc = ERROR;
	int index_gap = 0 ;
	int w_i = pbroadcast->ts_buffer_widx;
	int r_i = pbroadcast->ts_buffer_ridx;
	uint8* read_p = gDMB_BBBuffer[r_i];
	
	struct broadcast_tdmb_get_dmb_data_info __user* puserdata = (struct broadcast_tdmb_get_dmb_data_info  __user*)arg;

	if((NULL == puserdata) ||(NULL == puserdata->data_buf))
	{
		printk("broadcast_tdmb_get_dmb_data arg error\n");
		return rc;
	}

	index_gap = ((w_i >= r_i) ?( w_i - r_i) : (BROADCAST_DATA_CHUNK_NUM -r_i + w_i));
	if((index_gap < buf_idx_limit) ||(read_p == NULL))
	{
		rc = ERROR;
		
		return rc;
	}

	
	if(index_gap > buf_idx_limit)
	{
		printk("index_gap is greater than the buf_idx_limit index_gap = (%d), buf_idx_limit = (%d)\n", index_gap, buf_idx_limit);
	}
	
	if(copy_to_user((void __user*)puserdata->data_buf, (void*)read_p, buf_len_limit))
	{
		puserdata->buf_len = 0;
		puserdata->packet_cnt = 0;
		rc = ERROR;
	}
	else
	{
		puserdata->buf_len = buf_len_limit;
		puserdata->packet_cnt = packet_cnt_limit;
		rc = OK;
	}
	pbroadcast->ts_buffer_ridx = ((r_i + buf_idx_limit)%BROADCAST_DATA_CHUNK_NUM);

	return rc;
}

static int8 broadcast_tdmb_reset_ch(void)
{
	int8 res = ERROR;
	boolean retval = FALSE;
	
	retval = tunerbb_drv_lg2102_reset_ch();

	if(gpDMB_DATA_Buffer)
	{
		gpDMB_DATA_Buffer = NULL;
	}

	if(retval == TRUE)
	{
		res = OK;
	}

	return res;
}

static int8 broadcast_tdmb_user_stop(void __user *arg)
{
	int8 rc = ERROR;
	
	int udata;
	int __user* puser = (int __user*)arg;

	udata = *puser;

#if 1
	printk("broadcast_tdmb_user_stop data =(%d) (IN)\n", udata);
	tunerbb_drv_lg2102_set_userstop( );
	rc = OK;
#else
	printk("broadcast_tdmb_user_stop - mode udata = %d\n", udata);
	
	if(udata == 1)
	{
		user_stop_mode_cnt++;
	}
	else
	{
		if(user_stop_mode_cnt > 0)
		{
			user_stop_mode_cnt--;
		}
		else
		{
			user_stop_mode_cnt = 0;
		}
	}
	rc = OK;
#endif	
	return rc;	
}


static ssize_t broadcast_tdmb_read_control(struct file* filp, char __user* buf, size_t count, 
												loff_t* f_pos)
{
	int avail_size = 0;
	int w_i = 0;
	int r_i = 0;
	int i_g = 0;
	uint8* read_p = NULL;
	struct broadcast_tdmb_chdevice *the_dev = filp->private_data;

	w_i = the_dev->ts_buffer_widx;
	r_i = the_dev->ts_buffer_ridx;
	read_p = gDMB_BBBuffer[r_i];

	i_g = ((w_i >= r_i) ? (w_i - r_i) : (BROADCAST_DATA_CHUNK_NUM -r_i + w_i));

	if(((i_g*BROADCAST_INT_SIZE)< count) ||(read_p == NULL))
	{
		return 0;
	}

	if(copy_to_user((void __user*)buf, (void*)read_p, count))
	{
		return 0;
	}

	avail_size = count;
	the_dev->ts_buffer_ridx = (r_i + buf_idx_limit)%BROADCAST_DATA_CHUNK_NUM;

	return avail_size;
}


static ssize_t broadcast_tdmb_open_control(struct inode *inode, struct file *file)
{

	 struct broadcast_tdmb_chdevice *the_dev =
	       container_of(inode->i_cdev, struct broadcast_tdmb_chdevice, cdev);

	printk("broadcast_tdmb_open_control start \n");
	 
	file->private_data = the_dev;
	
	printk("broadcast_tdmb_open_control OK \n");
	
	return nonseekable_open(inode, file);
}

static long broadcast_tdmb_ioctl_control(struct file *filep, unsigned int cmd,	unsigned long arg)
{
	int rc = -EINVAL;
	void __user *argp = (void __user *)arg;
	
	
	
	switch (cmd) 
	{
	case LGE_BROADCAST_TDMB_IOCTL_ON:
		rc = broadcast_tdmb_power_on();
		printk("LGE_BROADCAST_TDMB_IOCTL_ON OK %d \n", rc);
		break;
	case LGE_BROADCAST_TDMB_IOCTL_OFF:		
		rc = broadcast_tdmb_power_off();
		printk("LGE_BROADCAST_TDMB_IOCTL_OFF OK %d \n", rc);
		break;
	case LGE_BROADCAST_TDMB_IOCTL_OPEN:
		rc = broadcast_tdmb_open();
		printk("LGE_BROADCAST_TDMB_IOCTL_OPEN OK %d \n", rc);
		break;
	case LGE_BROADCAST_TDMB_IOCTL_CLOSE:
		broadcast_tdmb_close();
		printk("LGE_BROADCAST_TDMB_IOCTL_CLOSE OK \n");
		rc = 0;
		break;
	case LGE_BROADCAST_TDMB_IOCTL_TUNE:
		rc = broadcast_tdmb_tune(argp);
		printk("LGE_BROADCAST_TDMB_IOCTL_TUNE result = %d \n", rc);
		break;
	case LGE_BROADCAST_TDMB_IOCTL_SET_CH:
		rc = broadcast_tdmb_tune_set_ch(argp);
		printk("LGE_BROADCAST_TDMB_IOCTL_SET_CH result = %d \n", rc);
		break;
	case LGE_BROADCAST_TDMB_IOCTL_RESYNC:
		rc = broadcast_tdmb_resync(argp);
		printk("LGE_BROADCAST_TDMB_IOCTL_RESYNC result = %d \n", rc);
		break;
	case LGE_BROADCAST_TDMB_IOCTL_DETECT_SYNC:
		rc = broadcast_tdmb_detect_sync(argp);
		printk("LGE_BROADCAST_TDMB_IOCTL_DETECT_SYNC result = %d \n", rc);
		break;
	case LGE_BROADCAST_TDMB_IOCTL_GET_SIG_INFO:
		rc = broadcast_tdmb_get_sig_info(argp);
		
		break;
	case LGE_BROADCAST_TDMB_IOCTL_GET_CH_INFO:
		rc = broadcast_tdmb_get_ch_info(argp);
		printk("LGE_BROADCAST_TDMB_IOCTL_GET_CH_INFO result = %d \n", rc);
		break;

	case LGE_BROADCAST_TDMB_IOCTL_RESET_CH:
		rc = broadcast_tdmb_reset_ch();
		printk("LGE_BROADCAST_TDMB_IOCTL_RESET_CH result = %d \n", rc);
		break;
		
	case LGE_BROADCAST_TDMB_IOCTL_USER_STOP:
		rc = broadcast_tdmb_user_stop(argp);
		printk("LGE_BROADCAST_TDMB_IOCTL_USER_STOP !!! \n");
		break;

	case LGE_BROADCAST_TDMB_IOCTL_GET_DMB_DATA:
		rc = broadcast_tdmb_get_dmb_data(argp);
		
		break;
	
	default:
		printk("broadcast_tdmb_ioctl_control OK \n");
		rc = -EINVAL;
		break;
	}

	return rc;
}

static ssize_t broadcast_tdmb_release_control(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations broadcast_tdmb_fops_control = 
{
	.owner = THIS_MODULE,
	.read = broadcast_tdmb_read_control,
	.open = broadcast_tdmb_open_control,
	.unlocked_ioctl = broadcast_tdmb_ioctl_control,
	.release = broadcast_tdmb_release_control,
};

static int broadcast_tdmb_device_init(struct broadcast_tdmb_chdevice *pbroadcast, int index)
{
	int rc;

	cdev_init(&pbroadcast->cdev, &broadcast_tdmb_fops_control);

	pbroadcast->cdev.owner = THIS_MODULE;
	

	rc = cdev_add(&pbroadcast->cdev, broadcast_tdmb_dev, 1);

	pbroadcast->dev = device_create(broadcast_tdmb_class, NULL, MKDEV(MAJOR(broadcast_tdmb_dev), 0),
					 NULL, "broadcast%d", index);

	printk("broadcast_tdmb_device_add add add%d broadcast_tdmb_dev = %d \n", rc, MKDEV(MAJOR(broadcast_tdmb_dev), 0));

	
	if (IS_ERR(pbroadcast->dev)) {
		rc = PTR_ERR(pbroadcast->dev);
		pr_err("device_create failed: %d\n", rc);
		rc = -1;
	}
	
	printk("broadcast_tdmb_device_init start %d\n", rc);

	return rc;
}


int broadcast_tdmb_drv_start(void)
{

	int rc = -ENODEV;
	
	if (!broadcast_tdmb_class) {

		broadcast_tdmb_class = class_create(THIS_MODULE, "broadcast_tdmb");
		if (IS_ERR(broadcast_tdmb_class)) {
			rc = PTR_ERR(broadcast_tdmb_class);
			pr_err("broadcast_tdmb_class: create device class failed: %d\n",
				rc);
			return rc;
		}

		rc = alloc_chrdev_region(&broadcast_tdmb_dev, 0, BROADCAST_TDMB_NUM_DEVS, "broadcast_tdmb");
		printk("broadcast_tdmb_drv_start add add%d broadcast_tdmb_dev = %d \n", rc, broadcast_tdmb_dev);
		if (rc < 0) {
			pr_err("broadcast_class: failed to allocate chrdev: %d\n",
				rc);
			return rc;
		}
	}

	pbroadcast = kzalloc(sizeof(struct broadcast_tdmb_chdevice), GFP_KERNEL);
#ifdef TDMB_EBI2_ISR	
	
#endif
	
	
	
	

	rc = broadcast_tdmb_device_init(pbroadcast, 0);
	if (rc < 0) {
		kfree(pbroadcast);
		return rc;
	}
	
	printk("broadcast_tdmb_drv_start start %d\n", rc);

	return rc;
}



int broadcast_tdmb_get_stop_mode(void)
{
	return 0;
#if 0
	if(user_stop_mode_cnt > 0 )
	{
		printk("broadcast_tdmb_get_stop_mode cnt = %d\n", user_stop_mode_cnt);
		return 1;
	}
	else
	{
		return 0;
	}
#endif	
}

EXPORT_SYMBOL(broadcast_tdmb_get_stop_mode);


int __devinit broadcast_tdmb_drv_init(void)
{

	int rc = 0 ;
	
	rc = broadcast_tdmb_drv_start();

	broadcast_tdmb_add_axi_qos( );		       

	rc = platform_driver_register(&broadcast_tdmb_driver);
	if (rc) {
		printk("broadcast_tdmb_drv_init %s failed to load\n", __func__);
		return rc;
	}

	return rc;
}

static void __exit broadcast_tdmb_drv_exit(void)
{
	broadcast_tdmb_release_axi_qos( );
	platform_driver_unregister(&broadcast_tdmb_driver);
}

module_init(broadcast_tdmb_drv_init);
module_exit(broadcast_tdmb_drv_exit);

MODULE_DESCRIPTION("broadcast_tdmb_drv_init");
MODULE_LICENSE("INC");


