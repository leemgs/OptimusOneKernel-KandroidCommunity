

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <asm/setup.h>


#define ATAG_MSM_WIFI	0x57494649 

#define MAX_NVS_SIZE	0x800U
static unsigned char wifi_nvs_ram[MAX_NVS_SIZE];

unsigned char *get_wifi_nvs_ram( void )
{
	return( wifi_nvs_ram );
}
EXPORT_SYMBOL(get_wifi_nvs_ram);

static int __init parse_tag_msm_wifi(const struct tag *tag)
{
	unsigned char *dptr = (unsigned char *)(&tag->u);
	unsigned size;
	
	size = min((tag->hdr.size - 2) * sizeof(__u32), MAX_NVS_SIZE);
#ifdef ATAG_MSM_WIFI_DEBUG	
	unsigned i;
	
	printk("WiFi Data size = %d , 0x%x\n", tag->hdr.size, tag->hdr.tag);
	for(i=0;( i < size );i++) {
		printk("%02x ", *dptr++);
	}
#endif	
	memcpy( (void *)wifi_nvs_ram, (void *)dptr, size );
	return 0;
}

__tagtable(ATAG_MSM_WIFI, parse_tag_msm_wifi);
