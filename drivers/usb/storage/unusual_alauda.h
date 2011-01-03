

#if defined(CONFIG_USB_STORAGE_ALAUDA) || \
		defined(CONFIG_USB_STORAGE_ALAUDA_MODULE)

UNUSUAL_DEV(  0x0584, 0x0008, 0x0102, 0x0102,
		"Fujifilm",
		"DPC-R1 (Alauda)",
		US_SC_SCSI, US_PR_ALAUDA, init_alauda, 0),

UNUSUAL_DEV(  0x07b4, 0x010a, 0x0102, 0x0102,
		"Olympus",
		"MAUSB-10 (Alauda)",
		US_SC_SCSI, US_PR_ALAUDA, init_alauda, 0),

#endif 
