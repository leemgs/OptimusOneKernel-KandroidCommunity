



#define APCI1564_BOARD_VENDOR_ID                0x15B8
#define APCI1564_ADDRESS_RANGE                  128



#define APCI1564_DIGITAL_IP                     0x04
#define APCI1564_DIGITAL_IP_INTERRUPT_MODE1     4
#define APCI1564_DIGITAL_IP_INTERRUPT_MODE2     8
#define APCI1564_DIGITAL_IP_IRQ                 16


#define APCI1564_DIGITAL_OP                 	0x18
#define APCI1564_DIGITAL_OP_RW               	0
#define APCI1564_DIGITAL_OP_INTERRUPT           4
#define APCI1564_DIGITAL_OP_IRQ                 12


#define ADDIDATA_OR                             0
#define ADDIDATA_AND                            1


#define APCI1564_DIGITAL_IP_INTERRUPT_STATUS    12


#define APCI1564_DIGITAL_OP_INTERRUPT_STATUS    8


#define APCI1564_DIGITAL_IP_INTERRUPT_ENABLE    0x4
#define APCI1564_DIGITAL_IP_INTERRUPT_DISABLE   0xFFFFFFFB


#define APCI1564_DIGITAL_OP_VCC_INTERRUPT_ENABLE   0x1
#define APCI1564_DIGITAL_OP_VCC_INTERRUPT_DISABLE  0xFFFFFFFE
#define APCI1564_DIGITAL_OP_CC_INTERRUPT_ENABLE    0x2
#define APCI1564_DIGITAL_OP_CC_INTERRUPT_DISABLE   0xFFFFFFFD



#define ADDIDATA_ENABLE                            1
#define ADDIDATA_DISABLE                           0



#define ADDIDATA_TIMER                             0
#define ADDIDATA_COUNTER                           1
#define ADDIDATA_WATCHDOG                          2
#define APCI1564_DIGITAL_OP_WATCHDOG               0x28
#define APCI1564_TIMER                             0x48
#define APCI1564_COUNTER1                          0x0
#define APCI1564_COUNTER2                          0x20
#define APCI1564_COUNTER3                          0x40
#define APCI1564_COUNTER4                          0x60
#define APCI1564_TCW_SYNC_ENABLEDISABLE            0
#define APCI1564_TCW_RELOAD_VALUE                  4
#define APCI1564_TCW_TIMEBASE                      8
#define APCI1564_TCW_PROG                          12
#define APCI1564_TCW_TRIG_STATUS                   16
#define APCI1564_TCW_IRQ                           20
#define APCI1564_TCW_WARN_TIMEVAL                  24
#define APCI1564_TCW_WARN_TIMEBASE                 28




int i_APCI1564_ConfigDigitalInput(struct comedi_device *dev, struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data);
int i_APCI1564_Read1DigitalInput(struct comedi_device *dev, struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data);
int i_APCI1564_ReadMoreDigitalInput(struct comedi_device *dev, struct comedi_subdevice *s,
				    struct comedi_insn *insn, unsigned int *data);


int i_APCI1564_ConfigDigitalOutput(struct comedi_device *dev, struct comedi_subdevice *s,
				   struct comedi_insn *insn, unsigned int *data);
int i_APCI1564_WriteDigitalOutput(struct comedi_device *dev, struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data);
int i_APCI1564_ReadDigitalOutput(struct comedi_device *dev, struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data);
int i_APCI1564_ReadInterruptStatus(struct comedi_device *dev, struct comedi_subdevice *s,
				   struct comedi_insn *insn, unsigned int *data);


int i_APCI1564_ConfigTimerCounterWatchdog(struct comedi_device *dev,
					  struct comedi_subdevice *s,
					  struct comedi_insn *insn, unsigned int *data);
int i_APCI1564_StartStopWriteTimerCounterWatchdog(struct comedi_device *dev,
						  struct comedi_subdevice *s,
						  struct comedi_insn *insn,
						  unsigned int *data);
int i_APCI1564_ReadTimerCounterWatchdog(struct comedi_device *dev,
					struct comedi_subdevice *s,
					struct comedi_insn *insn, unsigned int *data);


static void v_APCI1564_Interrupt(int irq, void *d);


int i_APCI1564_Reset(struct comedi_device *dev);
