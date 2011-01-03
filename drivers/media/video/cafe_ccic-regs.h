
#define REG_Y0BAR	0x00
#define REG_Y1BAR	0x04
#define REG_Y2BAR	0x08


#define REG_IMGPITCH	0x24	
#define   IMGP_YP_SHFT	  2		
#define   IMGP_YP_MASK	  0x00003ffc	
#define	  IMGP_UVP_SHFT	  18		
#define   IMGP_UVP_MASK   0x3ffc0000
#define REG_IRQSTATRAW	0x28	
#define   IRQ_EOF0	  0x00000001	
#define   IRQ_EOF1	  0x00000002	
#define   IRQ_EOF2	  0x00000004	
#define   IRQ_SOF0	  0x00000008	
#define   IRQ_SOF1	  0x00000010	
#define   IRQ_SOF2	  0x00000020	
#define   IRQ_OVERFLOW	  0x00000040	
#define   IRQ_TWSIW	  0x00010000	
#define   IRQ_TWSIR	  0x00020000	
#define   IRQ_TWSIE	  0x00040000	
#define   TWSIIRQS (IRQ_TWSIW|IRQ_TWSIR|IRQ_TWSIE)
#define   FRAMEIRQS (IRQ_EOF0|IRQ_EOF1|IRQ_EOF2|IRQ_SOF0|IRQ_SOF1|IRQ_SOF2)
#define   ALLIRQS (TWSIIRQS|FRAMEIRQS|IRQ_OVERFLOW)
#define REG_IRQMASK	0x2c	
#define REG_IRQSTAT	0x30	

#define REG_IMGSIZE	0x34	
#define  IMGSZ_V_MASK	  0x1fff0000
#define  IMGSZ_V_SHIFT	  16
#define	 IMGSZ_H_MASK	  0x00003fff
#define REG_IMGOFFSET	0x38	

#define REG_CTRL0	0x3c	
#define   C0_ENABLE	  0x00000001	


#define   C0_DF_MASK	  0x00fffffc    


#define   C0_RGB4_RGBX	  0x00000000
#define	  C0_RGB4_XRGB	  0x00000004
#define	  C0_RGB4_BGRX	  0x00000008
#define   C0_RGB4_XBGR	  0x0000000c
#define   C0_RGB5_RGGB	  0x00000000
#define	  C0_RGB5_GRBG	  0x00000004
#define	  C0_RGB5_GBRG	  0x00000008
#define   C0_RGB5_BGGR	  0x0000000c


#define   C0_DF_YUV	  0x00000000    
#define   C0_DF_RGB	  0x000000a0	
#define   C0_DF_BAYER     0x00000140	

#define   C0_RGBF_565	  0x00000000
#define   C0_RGBF_444	  0x00000800
#define   C0_RGB_BGR	  0x00001000	
#define   C0_YUV_PLANAR	  0x00000000	
#define   C0_YUV_PACKED	  0x00008000	
#define   C0_YUV_420PL	  0x0000a000	

#define	  C0_YUVE_YUYV	  0x00000000	
#define	  C0_YUVE_YVYU	  0x00010000	
#define	  C0_YUVE_VYUY	  0x00020000	
#define	  C0_YUVE_UYVY	  0x00030000	
#define   C0_YUVE_XYUV	  0x00000000    
#define	  C0_YUVE_XYVU	  0x00010000	
#define	  C0_YUVE_XUVY	  0x00020000	
#define	  C0_YUVE_XVUY	  0x00030000	

#define   C0_HPOL_LOW	  0x01000000	
#define   C0_VPOL_LOW	  0x02000000	
#define   C0_VCLK_LOW	  0x04000000	
#define   C0_DOWNSCALE	  0x08000000	
#define	  C0_SIFM_MASK	  0xc0000000	
#define   C0_SIF_HVSYNC	  0x00000000	
#define   CO_SOF_NOSYNC	  0x40000000	


#define REG_CTRL1	0x40	
#define   C1_444ALPHA	  0x00f00000	
#define   C1_ALPHA_SHFT	  20
#define   C1_DMAB32	  0x00000000	
#define   C1_DMAB16	  0x02000000	
#define	  C1_DMAB64	  0x04000000	
#define	  C1_DMAB_MASK	  0x06000000
#define   C1_TWOBUFS	  0x08000000	
#define   C1_PWRDWN	  0x10000000	

#define REG_CLKCTRL	0x88	
#define   CLK_DIV_MASK	  0x0000ffff	

#define REG_GPR		0xb4	
#define   GPR_C1EN	  0x00000020	
#define   GPR_C0EN	  0x00000010	
#define	  GPR_C1	  0x00000002	

#define   GPR_C0	  0x00000001	

#define REG_TWSIC0	0xb8	
#define   TWSIC0_EN       0x00000001	
#define   TWSIC0_MODE	  0x00000002	
#define   TWSIC0_SID	  0x000003fc	
#define   TWSIC0_SID_SHIFT 2
#define   TWSIC0_CLKDIV   0x0007fc00	
#define   TWSIC0_MASKACK  0x00400000	
#define   TWSIC0_OVMAGIC  0x00800000	

#define REG_TWSIC1	0xbc	
#define   TWSIC1_DATA	  0x0000ffff	
#define   TWSIC1_ADDR	  0x00ff0000	
#define   TWSIC1_ADDR_SHIFT 16
#define   TWSIC1_READ	  0x01000000	
#define   TWSIC1_WSTAT	  0x02000000	
#define   TWSIC1_RVALID	  0x04000000	
#define   TWSIC1_ERROR	  0x08000000	


#define REG_UBAR	0xc4	


#define REG_GL_CSR     0x3004  
#define   GCSR_SRS	 0x00000001	
#define   GCSR_SRC  	 0x00000002	
#define	  GCSR_MRS	 0x00000004	
#define	  GCSR_MRC	 0x00000008	
#define   GCSR_CCIC_EN   0x00004000    
#define REG_GL_IMASK   0x300c  
#define   GIMSK_CCIC_EN          0x00000004    

#define REG_GL_FCR	0x3038  
#define	  GFCR_GPIO_ON	  0x08		
#define REG_GL_GPIOR	0x315c	
#define   GGPIO_OUT  		0x80000	
#define   GGPIO_VAL  		0x00008	

#define REG_LEN                REG_GL_IMASK + 4



#define VGA_WIDTH	640
#define VGA_HEIGHT	480
