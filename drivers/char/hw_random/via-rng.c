

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hw_random.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/msr.h>
#include <asm/cpufeature.h>
#include <asm/i387.h>


#define PFX	KBUILD_MODNAME ": "


enum {
	VIA_STRFILT_CNT_SHIFT	= 16,
	VIA_STRFILT_FAIL	= (1 << 15),
	VIA_STRFILT_ENABLE	= (1 << 14),
	VIA_RAWBITS_ENABLE	= (1 << 13),
	VIA_RNG_ENABLE		= (1 << 6),
	VIA_NOISESRC1		= (1 << 8),
	VIA_NOISESRC2		= (1 << 9),
	VIA_XSTORE_CNT_MASK	= 0x0F,

	VIA_RNG_CHUNK_8		= 0x00,	
	VIA_RNG_CHUNK_4		= 0x01,	
	VIA_RNG_CHUNK_4_MASK	= 0xFFFFFFFF,
	VIA_RNG_CHUNK_2		= 0x02,	
	VIA_RNG_CHUNK_2_MASK	= 0xFFFF,
	VIA_RNG_CHUNK_1		= 0x03,	
	VIA_RNG_CHUNK_1_MASK	= 0xFF,
};



static inline u32 xstore(u32 *addr, u32 edx_in)
{
	u32 eax_out;
	int ts_state;

	ts_state = irq_ts_save();

	asm(".byte 0x0F,0xA7,0xC0 /* xstore %%edi (addr=%0) */"
		:"=m"(*addr), "=a"(eax_out)
		:"D"(addr), "d"(edx_in));

	irq_ts_restore(ts_state);
	return eax_out;
}

static int via_rng_data_present(struct hwrng *rng, int wait)
{
	u32 bytes_out;
	u32 *via_rng_datum = (u32 *)(&rng->priv);
	int i;

	

	for (i = 0; i < 20; i++) {
		*via_rng_datum = 0; 
		bytes_out = xstore(via_rng_datum, VIA_RNG_CHUNK_1);
		bytes_out &= VIA_XSTORE_CNT_MASK;
		if (bytes_out || !wait)
			break;
		udelay(10);
	}
	return bytes_out ? 1 : 0;
}

static int via_rng_data_read(struct hwrng *rng, u32 *data)
{
	u32 via_rng_datum = (u32)rng->priv;

	*data = via_rng_datum;

	return 1;
}

static int via_rng_init(struct hwrng *rng)
{
	struct cpuinfo_x86 *c = &cpu_data(0);
	u32 lo, hi, old_lo;

	
	if ((c->x86 == 6) && (c->x86_model >= 0x0f)) {
		if (!cpu_has_xstore_enabled) {
			printk(KERN_ERR PFX "can't enable hardware RNG "
				"if XSTORE is not enabled\n");
			return -ENODEV;
		}
		return 0;
	}

	
	rdmsr(MSR_VIA_RNG, lo, hi);

	old_lo = lo;
	lo &= ~(0x7f << VIA_STRFILT_CNT_SHIFT);
	lo &= ~VIA_XSTORE_CNT_MASK;
	lo &= ~(VIA_STRFILT_ENABLE | VIA_STRFILT_FAIL | VIA_RAWBITS_ENABLE);
	lo |= VIA_RNG_ENABLE;
	lo |= VIA_NOISESRC1;

	

	
	if ((c->x86_model == 9) && (c->x86_mask > 7))
		lo |= VIA_NOISESRC2;

	
	if (c->x86_model >= 10)
		lo |= VIA_NOISESRC2;

	if (lo != old_lo)
		wrmsr(MSR_VIA_RNG, lo, hi);

	
	rdmsr(MSR_VIA_RNG, lo, hi);
	if ((lo & VIA_RNG_ENABLE) == 0) {
		printk(KERN_ERR PFX "cannot enable VIA C3 RNG, aborting\n");
		return -ENODEV;
	}

	return 0;
}


static struct hwrng via_rng = {
	.name		= "via",
	.init		= via_rng_init,
	.data_present	= via_rng_data_present,
	.data_read	= via_rng_data_read,
};


static int __init mod_init(void)
{
	int err;

	if (!cpu_has_xstore)
		return -ENODEV;
	printk(KERN_INFO "VIA RNG detected\n");
	err = hwrng_register(&via_rng);
	if (err) {
		printk(KERN_ERR PFX "RNG registering failed (%d)\n",
		       err);
		goto out;
	}
out:
	return err;
}

static void __exit mod_exit(void)
{
	hwrng_unregister(&via_rng);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_DESCRIPTION("H/W RNG driver for VIA CPU with PadLock");
MODULE_LICENSE("GPL");
