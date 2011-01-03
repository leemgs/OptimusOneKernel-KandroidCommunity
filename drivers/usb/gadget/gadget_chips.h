

#ifndef __GADGET_CHIPS_H
#define __GADGET_CHIPS_H

#ifdef CONFIG_USB_GADGET_NET2280
#define	gadget_is_net2280(g)	!strcmp("net2280", (g)->name)
#else
#define	gadget_is_net2280(g)	0
#endif

#ifdef CONFIG_USB_GADGET_AMD5536UDC
#define	gadget_is_amd5536udc(g)	!strcmp("amd5536udc", (g)->name)
#else
#define	gadget_is_amd5536udc(g)	0
#endif

#ifdef CONFIG_USB_GADGET_DUMMY_HCD
#define	gadget_is_dummy(g)	!strcmp("dummy_udc", (g)->name)
#else
#define	gadget_is_dummy(g)	0
#endif

#ifdef CONFIG_USB_GADGET_PXA25X
#define	gadget_is_pxa(g)	!strcmp("pxa25x_udc", (g)->name)
#else
#define	gadget_is_pxa(g)	0
#endif

#ifdef CONFIG_USB_GADGET_GOKU
#define	gadget_is_goku(g)	!strcmp("goku_udc", (g)->name)
#else
#define	gadget_is_goku(g)	0
#endif


#ifdef CONFIG_USB_GADGET_SUPERH
#define	gadget_is_sh(g)		!strcmp("sh_udc", (g)->name)
#else
#define	gadget_is_sh(g)		0
#endif


#ifdef CONFIG_USB_GADGET_SA1100
#define	gadget_is_sa1100(g)	!strcmp("sa1100_udc", (g)->name)
#else
#define	gadget_is_sa1100(g)	0
#endif

#ifdef CONFIG_USB_GADGET_LH7A40X
#define	gadget_is_lh7a40x(g)	!strcmp("lh7a40x_udc", (g)->name)
#else
#define	gadget_is_lh7a40x(g)	0
#endif


#ifdef CONFIG_USB_GADGET_MQ11XX
#define	gadget_is_mq11xx(g)	!strcmp("mq11xx_udc", (g)->name)
#else
#define	gadget_is_mq11xx(g)	0
#endif

#ifdef CONFIG_USB_GADGET_OMAP
#define	gadget_is_omap(g)	!strcmp("omap_udc", (g)->name)
#else
#define	gadget_is_omap(g)	0
#endif


#ifdef CONFIG_USB_GADGET_N9604
#define	gadget_is_n9604(g)	!strcmp("n9604_udc", (g)->name)
#else
#define	gadget_is_n9604(g)	0
#endif


#ifdef CONFIG_USB_GADGET_PXA27X
#define	gadget_is_pxa27x(g)	!strcmp("pxa27x_udc", (g)->name)
#else
#define	gadget_is_pxa27x(g)	0
#endif

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
#define gadget_is_atmel_usba(g)	!strcmp("atmel_usba_udc", (g)->name)
#else
#define gadget_is_atmel_usba(g)	0
#endif

#ifdef CONFIG_USB_GADGET_S3C2410
#define gadget_is_s3c2410(g)    !strcmp("s3c2410_udc", (g)->name)
#else
#define gadget_is_s3c2410(g)    0
#endif

#ifdef CONFIG_USB_GADGET_AT91
#define gadget_is_at91(g)	!strcmp("at91_udc", (g)->name)
#else
#define gadget_is_at91(g)	0
#endif

#ifdef CONFIG_USB_GADGET_IMX
#define gadget_is_imx(g)	!strcmp("imx_udc", (g)->name)
#else
#define gadget_is_imx(g)	0
#endif

#ifdef CONFIG_USB_GADGET_FSL_USB2
#define gadget_is_fsl_usb2(g)	!strcmp("fsl-usb2-udc", (g)->name)
#else
#define gadget_is_fsl_usb2(g)	0
#endif



#ifdef CONFIG_USB_GADGET_MUSBHSFC
#define gadget_is_musbhsfc(g)	!strcmp("musbhsfc_udc", (g)->name)
#else
#define gadget_is_musbhsfc(g)	0
#endif


#ifdef CONFIG_USB_GADGET_MUSB_HDRC
#define gadget_is_musbhdrc(g)	!strcmp("musb_hdrc", (g)->name)
#else
#define gadget_is_musbhdrc(g)	0
#endif

#ifdef CONFIG_USB_GADGET_LANGWELL
#define gadget_is_langwell(g)	(!strcmp("langwell_udc", (g)->name))
#else
#define gadget_is_langwell(g)	0
#endif


#ifdef CONFIG_USB_GADGET_MPC8272
#define gadget_is_mpc8272(g)	!strcmp("mpc8272_udc", (g)->name)
#else
#define gadget_is_mpc8272(g)	0
#endif

#ifdef CONFIG_USB_GADGET_M66592
#define	gadget_is_m66592(g)	!strcmp("m66592_udc", (g)->name)
#else
#define	gadget_is_m66592(g)	0
#endif


#ifdef CONFIG_USB_GADGET_FSL_QE
#define gadget_is_fsl_qe(g)	!strcmp("fsl_qe_udc", (g)->name)
#else
#define gadget_is_fsl_qe(g)	0
#endif

#ifdef CONFIG_USB_GADGET_CI13XXX
#define gadget_is_ci13xxx(g)	(!strcmp("ci13xxx_udc", (g)->name))
#else
#define gadget_is_ci13xxx(g)	0
#endif

#ifdef CONFIG_USB_GADGET_MSM_72K
#define	gadget_is_msm72k(g)	!strcmp("msm72k_udc", (g)->name)
#else
#define	gadget_is_msm72k(g)	0
#endif





#ifdef CONFIG_USB_GADGET_R8A66597
#define	gadget_is_r8a66597(g)	!strcmp("r8a66597_udc", (g)->name)
#else
#define	gadget_is_r8a66597(g)	0
#endif



static inline int usb_gadget_controller_number(struct usb_gadget *gadget)
{
	if (gadget_is_net2280(gadget))
		return 0x01;
	else if (gadget_is_dummy(gadget))
		return 0x02;
	else if (gadget_is_pxa(gadget))
		return 0x03;
	else if (gadget_is_sh(gadget))
		return 0x04;
	else if (gadget_is_sa1100(gadget))
		return 0x05;
	else if (gadget_is_goku(gadget))
		return 0x06;
	else if (gadget_is_mq11xx(gadget))
		return 0x07;
	else if (gadget_is_omap(gadget))
		return 0x08;
	else if (gadget_is_lh7a40x(gadget))
		return 0x09;
	else if (gadget_is_n9604(gadget))
		return 0x10;
	else if (gadget_is_pxa27x(gadget))
		return 0x11;
	else if (gadget_is_s3c2410(gadget))
		return 0x12;
	else if (gadget_is_at91(gadget))
		return 0x13;
	else if (gadget_is_imx(gadget))
		return 0x14;
	else if (gadget_is_musbhsfc(gadget))
		return 0x15;
	else if (gadget_is_musbhdrc(gadget))
		return 0x16;
	else if (gadget_is_mpc8272(gadget))
		return 0x17;
	else if (gadget_is_atmel_usba(gadget))
		return 0x18;
	else if (gadget_is_fsl_usb2(gadget))
		return 0x19;
	else if (gadget_is_amd5536udc(gadget))
		return 0x20;
	else if (gadget_is_m66592(gadget))
		return 0x21;
	else if (gadget_is_fsl_qe(gadget))
		return 0x22;
	else if (gadget_is_ci13xxx(gadget))
		return 0x23;
	else if (gadget_is_langwell(gadget))
		return 0x24;
	else if (gadget_is_r8a66597(gadget))
		return 0x25;
	else if (gadget_is_msm72k(gadget))
		return 0x26;
	return -ENOENT;
}



static inline bool gadget_supports_altsettings(struct usb_gadget *gadget)
{
	
	if (gadget_is_pxa(gadget))
		return false;

	
	if (gadget_is_pxa27x(gadget))
		return false;

	
	if (gadget_is_sh(gadget))
		return false;

	
	return true;
}

#endif 
