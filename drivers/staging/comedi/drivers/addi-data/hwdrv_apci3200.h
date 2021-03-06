


#define APCI3200_BOARD_VENDOR_ID                 0x15B8


int MODULE_NO;
struct {
	int i_Gain;
	int i_Polarity;
	int i_OffsetRange;
	int i_Coupling;
	int i_SingleDiff;
	int i_AutoCalibration;
	unsigned int ui_ReloadValue;
	unsigned int ui_TimeUnitReloadVal;
	int i_Interrupt;
	int i_ModuleSelection;
} Config_Parameters_Module1, Config_Parameters_Module2,
    Config_Parameters_Module3, Config_Parameters_Module4;


static const struct comedi_lrange range_apci3200_ai = { 8, {
						     BIP_RANGE(10),
						     BIP_RANGE(5),
						     BIP_RANGE(2),
						     BIP_RANGE(1),
						     UNI_RANGE(10),
						     UNI_RANGE(5),
						     UNI_RANGE(2),
						     UNI_RANGE(1)
						     }
};

static const struct comedi_lrange range_apci3300_ai = { 4, {
						     UNI_RANGE(10),
						     UNI_RANGE(5),
						     UNI_RANGE(2),
						     UNI_RANGE(1)
						     }
};


#define APCI3200_AI_OFFSET_GAIN                  0
#define APCI3200_AI_SC_TEST                      4
#define APCI3200_AI_IRQ                          8
#define APCI3200_AI_AUTOCAL                      12
#define APCI3200_RELOAD_CONV_TIME_VAL            32
#define APCI3200_CONV_TIME_TIME_BASE             36
#define APCI3200_RELOAD_DELAY_TIME_VAL           40
#define APCI3200_DELAY_TIME_TIME_BASE            44
#define APCI3200_AI_MODULE1                      0
#define APCI3200_AI_MODULE2                      64
#define APCI3200_AI_MODULE3                      128
#define APCI3200_AI_MODULE4                      192
#define TRUE                                     1
#define FALSE                                    0
#define APCI3200_AI_EOSIRQ                       16
#define APCI3200_AI_EOS                          20
#define APCI3200_AI_CHAN_ID                      24
#define APCI3200_AI_CHAN_VAL                     28
#define ANALOG_INPUT                             0
#define TEMPERATURE                              1
#define RESISTANCE                               2

#define ENABLE_EXT_TRIG                          1
#define ENABLE_EXT_GATE                          2
#define ENABLE_EXT_TRIG_GATE                     3

#define APCI3200_MAXVOLT                         2.5
#define ADDIDATA_GREATER_THAN_TEST               0
#define ADDIDATA_LESS_THAN_TEST                  1

#define ADDIDATA_UNIPOLAR                        1
#define ADDIDATA_BIPOLAR                         2


#define MAX_MODULE				4


struct str_ADDIDATA_RTDStruct {
	unsigned int ul_NumberOfValue;
	unsigned int *pul_ResistanceValue;
	unsigned int *pul_TemperatureValue;
};


struct str_Module {

	
	unsigned long ul_CurrentSourceCJC;
	unsigned long ul_CurrentSource[5];
	

	
	unsigned long ul_GainFactor[8];	
	unsigned int w_GainValue[10];
	
};




struct str_BoardInfos {

	int i_CJCAvailable;
	int i_CJCPolarity;
	int i_CJCGain;
	int i_InterruptFlag;
	int i_ADDIDATAPolarity;
	int i_ADDIDATAGain;
	int i_AutoCalibration;
	int i_ADDIDATAConversionTime;
	int i_ADDIDATAConversionTimeUnit;
	int i_ADDIDATAType;
	int i_ChannelNo;
	int i_ChannelCount;
	int i_ScanType;
	int i_FirstChannel;
	int i_LastChannel;
	int i_Sum;
	int i_Offset;
	unsigned int ui_Channel_num;
	int i_Count;
	int i_Initialised;
	
	unsigned int ui_InterruptChannelValue[144];	
	unsigned char b_StructInitialized;
	
	unsigned int ui_ScanValueArray[7 + 12];	
	

	
	int i_ConnectionType;
	int i_NbrOfModule;
	struct str_Module s_Module[MAX_MODULE];
	
};







int i_APCI3200_ConfigAnalogInput(struct comedi_device *dev, struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data);
int i_APCI3200_ReadAnalogInput(struct comedi_device *dev, struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data);
int i_APCI3200_InsnWriteReleaseAnalogInput(struct comedi_device *dev,
					   struct comedi_subdevice *s,
					   struct comedi_insn *insn, unsigned int *data);
int i_APCI3200_InsnBits_AnalogInput_Test(struct comedi_device *dev,
					 struct comedi_subdevice *s,
					 struct comedi_insn *insn, unsigned int *data);
int i_APCI3200_StopCyclicAcquisition(struct comedi_device *dev, struct comedi_subdevice *s);
int i_APCI3200_InterruptHandleEos(struct comedi_device *dev);
int i_APCI3200_CommandTestAnalogInput(struct comedi_device *dev, struct comedi_subdevice *s,
				      struct comedi_cmd *cmd);
int i_APCI3200_CommandAnalogInput(struct comedi_device *dev, struct comedi_subdevice *s);
int i_APCI3200_ReadDigitalInput(struct comedi_device *dev, struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data);

void v_APCI3200_Interrupt(int irq, void *d);
int i_APCI3200_InterruptHandleEos(struct comedi_device *dev);

int i_APCI3200_Reset(struct comedi_device *dev);

int i_APCI3200_ReadCJCCalOffset(struct comedi_device *dev, unsigned int *data);
int i_APCI3200_ReadCJCValue(struct comedi_device *dev, unsigned int *data);
int i_APCI3200_ReadCalibrationGainValue(struct comedi_device *dev, unsigned int *data);
int i_APCI3200_ReadCalibrationOffsetValue(struct comedi_device *dev, unsigned int *data);
int i_APCI3200_Read1AnalogInputChannel(struct comedi_device *dev,
				       struct comedi_subdevice *s, struct comedi_insn *insn,
				       unsigned int *data);
int i_APCI3200_ReadCJCCalGain(struct comedi_device *dev, unsigned int *data);
