

#ifndef _LINUX_TOPIC_H
#define _LINUX_TOPIC_H



#define TOPIC_SOCKET_CONTROL		0x0090	
#define  TOPIC_SCR_IRQSEL		0x00000001

#define TOPIC_SLOT_CONTROL		0x00a0	
#define  TOPIC_SLOT_SLOTON		0x80
#define  TOPIC_SLOT_SLOTEN		0x40
#define  TOPIC_SLOT_ID_LOCK		0x20
#define  TOPIC_SLOT_ID_WP		0x10
#define  TOPIC_SLOT_PORT_MASK		0x0c
#define  TOPIC_SLOT_PORT_SHIFT		2
#define  TOPIC_SLOT_OFS_MASK		0x03

#define TOPIC_CARD_CONTROL		0x00a1	
#define  TOPIC_CCR_INTB			0x20
#define  TOPIC_CCR_INTA			0x10
#define  TOPIC_CCR_CLOCK		0x0c
#define  TOPIC_CCR_PCICLK		0x0c
#define  TOPIC_CCR_PCICLK_2		0x08
#define  TOPIC_CCR_CCLK			0x04

#define TOPIC97_INT_CONTROL		0x00a1	
#define  TOPIC97_ICR_INTB		0x20
#define  TOPIC97_ICR_INTA		0x10
#define  TOPIC97_ICR_STSIRQNP		0x04
#define  TOPIC97_ICR_IRQNP		0x02
#define  TOPIC97_ICR_IRQSEL		0x01

#define TOPIC_CARD_DETECT		0x00a3	
#define  TOPIC_CDR_MODE_PC32		0x80
#define  TOPIC_CDR_VS1			0x04
#define  TOPIC_CDR_VS2			0x02
#define  TOPIC_CDR_SW_DETECT		0x01

#define TOPIC_REGISTER_CONTROL		0x00a4	
#define  TOPIC_RCR_RESUME_RESET		0x80000000
#define  TOPIC_RCR_REMOVE_RESET		0x40000000
#define  TOPIC97_RCR_CLKRUN_ENA		0x20000000
#define  TOPIC97_RCR_TESTMODE		0x10000000
#define  TOPIC97_RCR_IOPLUP		0x08000000
#define  TOPIC_RCR_BUFOFF_PWROFF	0x02000000
#define  TOPIC_RCR_BUFOFF_SIGOFF	0x01000000
#define  TOPIC97_RCR_CB_DEV_MASK	0x0000f800
#define  TOPIC97_RCR_CB_DEV_SHIFT	11
#define  TOPIC97_RCR_RI_DISABLE		0x00000004
#define  TOPIC97_RCR_CAUDIO_OFF		0x00000002
#define  TOPIC_RCR_CAUDIO_INVERT	0x00000001

#define TOPIC97_MISC1			0x00ad  
#define  TOPIC97_MISC1_CLOCKRUN_ENABLE	0x80
#define  TOPIC97_MISC1_CLOCKRUN_MODE	0x40
#define  TOPIC97_MISC1_DETECT_REQ_ENA	0x10
#define  TOPIC97_MISC1_SCK_CLEAR_DIS	0x04
#define  TOPIC97_MISC1_R2_LOW_ENABLE	0x10

#define TOPIC97_MISC2			0x00ae  
#define  TOPIC97_MISC2_SPWRCLK_MASK	0x70
#define  TOPIC97_MISC2_SPWRMOD		0x08
#define  TOPIC97_MISC2_SPWR_ENABLE	0x04
#define  TOPIC97_MISC2_ZV_MODE		0x02
#define  TOPIC97_MISC2_ZV_ENABLE	0x01

#define TOPIC97_ZOOM_VIDEO_CONTROL	0x009c  
#define  TOPIC97_ZV_CONTROL_ENABLE	0x01

#define TOPIC97_AUDIO_VIDEO_SWITCH	0x003c  
#define  TOPIC97_AVS_AUDIO_CONTROL	0x02
#define  TOPIC97_AVS_VIDEO_CONTROL	0x01

#define TOPIC_EXCA_IF_CONTROL		0x3e	
#define TOPIC_EXCA_IFC_33V_ENA		0x01

static void topic97_zoom_video(struct pcmcia_socket *sock, int onoff)
{
	struct yenta_socket *socket = container_of(sock, struct yenta_socket, socket);
	u8 reg_zv, reg;

	reg_zv = config_readb(socket, TOPIC97_ZOOM_VIDEO_CONTROL);
	if (onoff) {
		reg_zv |= TOPIC97_ZV_CONTROL_ENABLE;
		config_writeb(socket, TOPIC97_ZOOM_VIDEO_CONTROL, reg_zv);

		reg = config_readb(socket, TOPIC97_MISC2);
		reg |= TOPIC97_MISC2_ZV_ENABLE;
		config_writeb(socket, TOPIC97_MISC2, reg);

		
#if 0
		reg = config_readb(socket, TOPIC97_AUDIO_VIDEO_SWITCH);
		reg |= TOPIC97_AVS_AUDIO_CONTROL | TOPIC97_AVS_VIDEO_CONTROL;
		config_writeb(socket, TOPIC97_AUDIO_VIDEO_SWITCH, reg);
#endif
	}
	else {
		reg_zv &= ~TOPIC97_ZV_CONTROL_ENABLE;
		config_writeb(socket, TOPIC97_ZOOM_VIDEO_CONTROL, reg_zv);
	}

}

static int topic97_override(struct yenta_socket *socket)
{
	
	socket->socket.zoom_video = topic97_zoom_video;
	return 0;
}


static int topic95_override(struct yenta_socket *socket)
{
	u8 fctrl;

	
	fctrl = exca_readb(socket, TOPIC_EXCA_IF_CONTROL);
	exca_writeb(socket, TOPIC_EXCA_IF_CONTROL, fctrl | TOPIC_EXCA_IFC_33V_ENA);

	
	socket->flags |= YENTA_16BIT_POWER_EXCA | YENTA_16BIT_POWER_DF;

	return 0;
}

#endif 
