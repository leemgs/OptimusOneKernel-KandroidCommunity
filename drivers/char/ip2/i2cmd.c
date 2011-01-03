


















#include "i2cmd.h"   













static UCHAR ct02[] = { 1, BTH,     0x02                     }; 
static UCHAR ct03[] = { 1, BTH,     0x03                     }; 
static UCHAR ct04[] = { 1, BTH,     0x04                     }; 
static UCHAR ct05[] = { 1, BTH,     0x05                     }; 
static UCHAR ct06[] = { 1, BYP,     0x06                     }; 
static UCHAR ct07[] = { 2, BTH,     0x07,0                   }; 
static UCHAR ct08[] = { 2, BTH,     0x08,0                   }; 
static UCHAR ct09[] = { 2, BTH,     0x09,0                   }; 
static UCHAR ct10[] = { 2, BTH,     0x0A,0                   }; 
static UCHAR ct11[] = { 2, BTH,     0x0B,0                   }; 
static UCHAR ct12[] = { 2, BTH,     0x0C,0                   }; 
static UCHAR ct13[] = { 1, BTH,     0x0D                     }; 
static UCHAR ct14[] = { 1, BYP|VIP, 0x0E                     }; 

static UCHAR ct16[] = { 2, INL,     0x10,0                   }; 
static UCHAR ct17[] = { 2, INL,     0x11,0                   }; 
static UCHAR ct18[] = { 1, INL,     0x12                     }; 
static UCHAR ct19[] = { 1, BTH,     0x13                     }; 
static UCHAR ct20[] = { 1, INL,     0x14                     }; 
static UCHAR ct21[] = { 1, BTH,     0x15                     }; 
static UCHAR ct22[] = { 1, BTH,     0x16                     }; 
static UCHAR ct23[] = { 1, BTH,     0x17                     }; 
static UCHAR ct24[] = { 1, BTH,     0x18                     }; 
static UCHAR ct25[] = { 1, BTH,     0x19                     }; 
static UCHAR ct26[] = { 2, BTH,     0x1A,0                   }; 
static UCHAR ct27[] = { 1, BTH,     0x1B                     }; 


static UCHAR ct30[] = { 1, INL,     0x1E                     }; 
static UCHAR ct31[] = { 1, INL,     0x1F                     }; 
static UCHAR ct32[] = { 1, INL,     0x20                     }; 
static UCHAR ct33[] = { 1, INL,     0x21                     }; 
static UCHAR ct34[] = { 2, BTH,     0x22,0                   }; 
static UCHAR ct35[] = { 2, BTH|END, 0x23,0                   }; 
static UCHAR ct36[] = { 2, BTH,     0x24,0                   }; 











static UCHAR ct41[] = { 1, BYP,     0x29                     }; 












static UCHAR ct54[] = { 3, BTH,     0x36,0,0                 }; 
static UCHAR ct55[] = { 3, BTH,     0x37,0,0                 }; 
static UCHAR ct56[] = { 2, BTH|END, 0x38,0                   }; 
static UCHAR ct57[] = { 1, BYP,     0x39                     }; 
static UCHAR ct58[] = { 1, BYP,     0x3A                     }; 
static UCHAR ct59[] = { 2, BTH,     0x3B,0                   }; 
static UCHAR ct60[] = { 1, INL|VIP, 0x3C                     }; 


static UCHAR ct63[] = { 2, INL,     0x3F,0                   }; 
static UCHAR ct64[] = { 2, INL,     0x40,0                   }; 














static UCHAR ct79[] = { 2, BYP,     0x4F,0                   }; 







static UCHAR ct87[] = { 1, BYP,     0x57                     }; 























#if 0
cmdSyntaxPtr
i2cmdUnixFlags(unsigned short iflag,unsigned short cflag,unsigned short lflag)
{
	cmdSyntaxPtr pCM = (cmdSyntaxPtr) ct47;

	pCM->cmd[1] = (unsigned char)  iflag;
	pCM->cmd[2] = (unsigned char) (iflag >> 8);
	pCM->cmd[3] = (unsigned char)  cflag;
	pCM->cmd[4] = (unsigned char) (cflag >> 8);
	pCM->cmd[5] = (unsigned char)  lflag;
	pCM->cmd[6] = (unsigned char) (lflag >> 8);
	return pCM;
}
#endif  












static cmdSyntaxPtr
i2cmdBaudDef(int which, unsigned short rate)
{
	cmdSyntaxPtr pCM;

	switch(which)
	{
	case 1:
		pCM = (cmdSyntaxPtr) ct54;
		break;
	default:
	case 2:
		pCM = (cmdSyntaxPtr) ct55;
		break;
	}
	pCM->cmd[1] = (unsigned char) rate;
	pCM->cmd[2] = (unsigned char) (rate >> 8);
	return pCM;
}

