

#define APCI1710_PCI_BUS_CLOCK 			0
#define APCI1710_FRONT_CONNECTOR_INPUT 		1
#define APCI1710_TIMER_READVALUE		0
#define APCI1710_TIMER_GETOUTPUTLEVEL		1
#define APCI1710_TIMER_GETPROGRESSSTATUS	2
#define APCI1710_TIMER_WRITEVALUE		3

#define APCI1710_TIMER_READINTERRUPT		1
#define APCI1710_TIMER_READALLTIMER		2


#ifndef APCI1710_10MHZ
#define APCI1710_10MHZ	10
#endif



int i_APCI1710_InsnConfigInitTimer(struct comedi_device *dev, struct comedi_subdevice *s,
				   struct comedi_insn *insn, unsigned int *data);

int i_APCI1710_InsnWriteEnableDisableTimer(struct comedi_device *dev,
					   struct comedi_subdevice *s,
					   struct comedi_insn *insn, unsigned int *data);


int i_APCI1710_InsnReadAllTimerValue(struct comedi_device *dev, struct comedi_subdevice *s,
				     struct comedi_insn *insn, unsigned int *data);

int i_APCI1710_InsnBitsTimer(struct comedi_device *dev, struct comedi_subdevice *s,
			     struct comedi_insn *insn, unsigned int *data);


int i_APCI1710_ReadTimerValue(struct comedi_device *dev,
			      unsigned char b_ModulNbr, unsigned char b_TimerNbr,
			      unsigned int *pul_TimerValue);

int i_APCI1710_GetTimerOutputLevel(struct comedi_device *dev,
				   unsigned char b_ModulNbr, unsigned char b_TimerNbr,
				   unsigned char *pb_OutputLevel);

int i_APCI1710_GetTimerProgressStatus(struct comedi_device *dev,
				      unsigned char b_ModulNbr, unsigned char b_TimerNbr,
				      unsigned char *pb_TimerStatus);


int i_APCI1710_WriteTimerValue(struct comedi_device *dev,
			       unsigned char b_ModulNbr, unsigned char b_TimerNbr,
			       unsigned int ul_WriteValue);
