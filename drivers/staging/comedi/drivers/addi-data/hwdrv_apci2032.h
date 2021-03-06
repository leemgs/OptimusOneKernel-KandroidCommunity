




#define APCI2032_BOARD_VENDOR_ID                 0x15B8
#define APCI2032_ADDRESS_RANGE                   63



#define APCI2032_DIGITAL_OP                 	0
#define APCI2032_DIGITAL_OP_RW                 	0
#define APCI2032_DIGITAL_OP_INTERRUPT           4
#define APCI2032_DIGITAL_OP_IRQ                 12


#define APCI2032_DIGITAL_OP_INTERRUPT_STATUS    8


#define APCI2032_DIGITAL_OP_VCC_INTERRUPT_ENABLE   0x1
#define APCI2032_DIGITAL_OP_VCC_INTERRUPT_DISABLE  0xFFFFFFFE
#define APCI2032_DIGITAL_OP_CC_INTERRUPT_ENABLE    0x2
#define APCI2032_DIGITAL_OP_CC_INTERRUPT_DISABLE   0xFFFFFFFD



#define ADDIDATA_ENABLE                            1
#define ADDIDATA_DISABLE                           0



#define ADDIDATA_WATCHDOG                          2
#define APCI2032_DIGITAL_OP_WATCHDOG               16
#define APCI2032_TCW_RELOAD_VALUE                  4
#define APCI2032_TCW_TIMEBASE                      8
#define APCI2032_TCW_PROG                          12
#define APCI2032_TCW_TRIG_STATUS                   16
#define APCI2032_TCW_IRQ                           20




int i_APCI2032_ConfigDigitalOutput(struct comedi_device *dev, struct comedi_subdevice *s,
				   struct comedi_insn *insn, unsigned int *data);
int i_APCI2032_WriteDigitalOutput(struct comedi_device *dev, struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data);
int i_APCI2032_ReadDigitalOutput(struct comedi_device *dev, struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data);
int i_APCI2032_ReadInterruptStatus(struct comedi_device *dev, struct comedi_subdevice *s,
				   struct comedi_insn *insn, unsigned int *data);



int i_APCI2032_ConfigWatchdog(struct comedi_device *dev, struct comedi_subdevice *s,
			      struct comedi_insn *insn, unsigned int *data);
int i_APCI2032_StartStopWriteWatchdog(struct comedi_device *dev, struct comedi_subdevice *s,
				      struct comedi_insn *insn, unsigned int *data);
int i_APCI2032_ReadWatchdog(struct comedi_device *dev, struct comedi_subdevice *s,
			    struct comedi_insn *insn, unsigned int *data);



void v_APCI2032_Interrupt(int irq, void *d);


int i_APCI2032_Reset(struct comedi_device *dev);
