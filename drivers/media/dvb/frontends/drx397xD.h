

#ifndef _DRX397XD_H_INCLUDED
#define _DRX397XD_H_INCLUDED

#include <linux/dvb/frontend.h>

#define DRX_F_STEPSIZE	166667
#define DRX_F_OFFSET	36000000

#define I2C_ADR_C0(x) \
(	cpu_to_le32( \
		(u32)( \
			(((u32)(x) & (u32)0x000000ffUL)      ) | \
			(((u32)(x) & (u32)0x0000ff00UL) << 16) | \
			(((u32)(x) & (u32)0x0fff0000UL) >>  8) | \
			 (	     (u32)0x00c00000UL)          \
		      )) \
)

#define I2C_ADR_E0(x) \
(	cpu_to_le32( \
		(u32)( \
			(((u32)(x) & (u32)0x000000ffUL)      ) | \
			(((u32)(x) & (u32)0x0000ff00UL) << 16) | \
			(((u32)(x) & (u32)0x0fff0000UL) >>  8) | \
			 (	     (u32)0x00e00000UL)          \
		      )) \
)

struct drx397xD_CfgRfAgc	
{
	int d00;	
	u16 w04;
	u16 w06;
};

struct drx397xD_CfgIfAgc	
{
	int d00;	
	u16 w04;	
	u16 w06;
	u16 w08;
	u16 w0A;
	u16 w0C;
};

struct drx397xD_s20 {
	int d04;
	u32 d18;
	u32 d1C;
	u32 d20;
	u32 d14;
	u32 d24;
	u32 d0C;
	u32 d08;
};

struct drx397xD_config
{
	
	u8	demod_address;		

	struct drx397xD_CfgIfAgc  ifagc;  
	struct drx397xD_CfgRfAgc  rfagc;  
	u32	s20d24;

	
	u16	w50, w52,  w56;

	int	d5C;
	int	d60;
	int	d48;
	int	d28;

	u32	f_if;	
			
			

	u16	f_osc;	

	u16	w92;	

	u16	wA0;
	u16	w98;
	u16	w9A;

	u16	w9C;	
	u16	w9E;	

	
	u16	ss78;	
	u16	ss7A;	
	u16	ss76;	
};

#if defined(CONFIG_DVB_DRX397XD) || (defined(CONFIG_DVB_DRX397XD_MODULE) && defined(MODULE))
extern struct dvb_frontend* drx397xD_attach(const struct drx397xD_config *config,
					   struct i2c_adapter *i2c);
#else
static inline struct dvb_frontend* drx397xD_attach(const struct drx397xD_config *config,
					   struct i2c_adapter *i2c)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif 

#endif 
