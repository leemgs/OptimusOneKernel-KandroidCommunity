

#if defined(CONFIG_USB_STORAGE_SDDR55) || \
		defined(CONFIG_USB_STORAGE_SDDR55_MODULE)


UNUSUAL_DEV( 0x07c4, 0xa103, 0x0000, 0x9999,
		"Datafab",
		"MDSM-B reader",
		US_SC_SCSI, US_PR_SDDR55, NULL,
		US_FL_FIX_INQUIRY),


UNUSUAL_DEV(  0x07c4, 0xa109, 0x0000, 0xffff,
		"Datafab Systems, Inc.",
		"USB to CF + SM Combo (LC1)",
		US_SC_SCSI, US_PR_SDDR55, NULL, 0),

UNUSUAL_DEV( 0x0c0b, 0xa109, 0x0000, 0xffff,
		"Acomdata",
		"SM",
		US_SC_SCSI, US_PR_SDDR55, NULL, 0),

UNUSUAL_DEV(  0x55aa, 0xa103, 0x0000, 0x9999,
		"Sandisk",
		"ImageMate SDDR55",
		US_SC_SCSI, US_PR_SDDR55, NULL, 0),

#endif 
