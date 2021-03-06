

#ifndef _WM8903_H
#define _WM8903_H

#include <linux/i2c.h>

extern struct snd_soc_dai wm8903_dai;
extern struct snd_soc_codec_device soc_codec_dev_wm8903;

#define WM8903_MCLK_DIV_2 1
#define WM8903_CLK_SYS    2
#define WM8903_BCLK       3
#define WM8903_LRCLK      4


#define WM8903_SW_RESET_AND_ID                  0x00
#define WM8903_REVISION_NUMBER                  0x01
#define WM8903_BIAS_CONTROL_0                   0x04
#define WM8903_VMID_CONTROL_0                   0x05
#define WM8903_MIC_BIAS_CONTROL_0               0x06
#define WM8903_ANALOGUE_DAC_0                   0x08
#define WM8903_ANALOGUE_ADC_0                   0x0A
#define WM8903_POWER_MANAGEMENT_0               0x0C
#define WM8903_POWER_MANAGEMENT_1               0x0D
#define WM8903_POWER_MANAGEMENT_2               0x0E
#define WM8903_POWER_MANAGEMENT_3               0x0F
#define WM8903_POWER_MANAGEMENT_4               0x10
#define WM8903_POWER_MANAGEMENT_5               0x11
#define WM8903_POWER_MANAGEMENT_6               0x12
#define WM8903_CLOCK_RATES_0                    0x14
#define WM8903_CLOCK_RATES_1                    0x15
#define WM8903_CLOCK_RATES_2                    0x16
#define WM8903_AUDIO_INTERFACE_0                0x18
#define WM8903_AUDIO_INTERFACE_1                0x19
#define WM8903_AUDIO_INTERFACE_2                0x1A
#define WM8903_AUDIO_INTERFACE_3                0x1B
#define WM8903_DAC_DIGITAL_VOLUME_LEFT          0x1E
#define WM8903_DAC_DIGITAL_VOLUME_RIGHT         0x1F
#define WM8903_DAC_DIGITAL_0                    0x20
#define WM8903_DAC_DIGITAL_1                    0x21
#define WM8903_ADC_DIGITAL_VOLUME_LEFT          0x24
#define WM8903_ADC_DIGITAL_VOLUME_RIGHT         0x25
#define WM8903_ADC_DIGITAL_0                    0x26
#define WM8903_DIGITAL_MICROPHONE_0             0x27
#define WM8903_DRC_0                            0x28
#define WM8903_DRC_1                            0x29
#define WM8903_DRC_2                            0x2A
#define WM8903_DRC_3                            0x2B
#define WM8903_ANALOGUE_LEFT_INPUT_0            0x2C
#define WM8903_ANALOGUE_RIGHT_INPUT_0           0x2D
#define WM8903_ANALOGUE_LEFT_INPUT_1            0x2E
#define WM8903_ANALOGUE_RIGHT_INPUT_1           0x2F
#define WM8903_ANALOGUE_LEFT_MIX_0              0x32
#define WM8903_ANALOGUE_RIGHT_MIX_0             0x33
#define WM8903_ANALOGUE_SPK_MIX_LEFT_0          0x34
#define WM8903_ANALOGUE_SPK_MIX_LEFT_1          0x35
#define WM8903_ANALOGUE_SPK_MIX_RIGHT_0         0x36
#define WM8903_ANALOGUE_SPK_MIX_RIGHT_1         0x37
#define WM8903_ANALOGUE_OUT1_LEFT               0x39
#define WM8903_ANALOGUE_OUT1_RIGHT              0x3A
#define WM8903_ANALOGUE_OUT2_LEFT               0x3B
#define WM8903_ANALOGUE_OUT2_RIGHT              0x3C
#define WM8903_ANALOGUE_OUT3_LEFT               0x3E
#define WM8903_ANALOGUE_OUT3_RIGHT              0x3F
#define WM8903_ANALOGUE_SPK_OUTPUT_CONTROL_0    0x41
#define WM8903_DC_SERVO_0                       0x43
#define WM8903_DC_SERVO_2                       0x45
#define WM8903_ANALOGUE_HP_0                    0x5A
#define WM8903_ANALOGUE_LINEOUT_0               0x5E
#define WM8903_CHARGE_PUMP_0                    0x62
#define WM8903_CLASS_W_0                        0x68
#define WM8903_WRITE_SEQUENCER_0                0x6C
#define WM8903_WRITE_SEQUENCER_1                0x6D
#define WM8903_WRITE_SEQUENCER_2                0x6E
#define WM8903_WRITE_SEQUENCER_3                0x6F
#define WM8903_WRITE_SEQUENCER_4                0x70
#define WM8903_CONTROL_INTERFACE                0x72
#define WM8903_GPIO_CONTROL_1                   0x74
#define WM8903_GPIO_CONTROL_2                   0x75
#define WM8903_GPIO_CONTROL_3                   0x76
#define WM8903_GPIO_CONTROL_4                   0x77
#define WM8903_GPIO_CONTROL_5                   0x78
#define WM8903_INTERRUPT_STATUS_1               0x79
#define WM8903_INTERRUPT_STATUS_1_MASK          0x7A
#define WM8903_INTERRUPT_POLARITY_1             0x7B
#define WM8903_INTERRUPT_CONTROL                0x7E
#define WM8903_CONTROL_INTERFACE_TEST_1         0x81
#define WM8903_CHARGE_PUMP_TEST_1               0x95
#define WM8903_CLOCK_RATE_TEST_4                0xA4
#define WM8903_ANALOGUE_OUTPUT_BIAS_0           0xAC

#define WM8903_REGISTER_COUNT                   75
#define WM8903_MAX_REGISTER                     0xAC




#define WM8903_SW_RESET_DEV_ID1_MASK            0xFFFF  
#define WM8903_SW_RESET_DEV_ID1_SHIFT                0  
#define WM8903_SW_RESET_DEV_ID1_WIDTH               16  


#define WM8903_CHIP_REV_MASK                    0x000F  
#define WM8903_CHIP_REV_SHIFT                        0  
#define WM8903_CHIP_REV_WIDTH                        4  


#define WM8903_POBCTRL                          0x0010  
#define WM8903_POBCTRL_MASK                     0x0010  
#define WM8903_POBCTRL_SHIFT                         4  
#define WM8903_POBCTRL_WIDTH                         1  
#define WM8903_ISEL_MASK                        0x000C  
#define WM8903_ISEL_SHIFT                            2  
#define WM8903_ISEL_WIDTH                            2  
#define WM8903_STARTUP_BIAS_ENA                 0x0002  
#define WM8903_STARTUP_BIAS_ENA_MASK            0x0002  
#define WM8903_STARTUP_BIAS_ENA_SHIFT                1  
#define WM8903_STARTUP_BIAS_ENA_WIDTH                1  
#define WM8903_BIAS_ENA                         0x0001  
#define WM8903_BIAS_ENA_MASK                    0x0001  
#define WM8903_BIAS_ENA_SHIFT                        0  
#define WM8903_BIAS_ENA_WIDTH                        1  


#define WM8903_VMID_TIE_ENA                     0x0080  
#define WM8903_VMID_TIE_ENA_MASK                0x0080  
#define WM8903_VMID_TIE_ENA_SHIFT                    7  
#define WM8903_VMID_TIE_ENA_WIDTH                    1  
#define WM8903_BUFIO_ENA                        0x0040  
#define WM8903_BUFIO_ENA_MASK                   0x0040  
#define WM8903_BUFIO_ENA_SHIFT                       6  
#define WM8903_BUFIO_ENA_WIDTH                       1  
#define WM8903_VMID_IO_ENA                      0x0020  
#define WM8903_VMID_IO_ENA_MASK                 0x0020  
#define WM8903_VMID_IO_ENA_SHIFT                     5  
#define WM8903_VMID_IO_ENA_WIDTH                     1  
#define WM8903_VMID_SOFT_MASK                   0x0018  
#define WM8903_VMID_SOFT_SHIFT                       3  
#define WM8903_VMID_SOFT_WIDTH                       2  
#define WM8903_VMID_RES_MASK                    0x0006  
#define WM8903_VMID_RES_SHIFT                        1  
#define WM8903_VMID_RES_WIDTH                        2  
#define WM8903_VMID_BUF_ENA                     0x0001  
#define WM8903_VMID_BUF_ENA_MASK                0x0001  
#define WM8903_VMID_BUF_ENA_SHIFT                    0  
#define WM8903_VMID_BUF_ENA_WIDTH                    1  

#define WM8903_VMID_RES_50K                          2
#define WM8903_VMID_RES_250K                         3
#define WM8903_VMID_RES_5K                           4


#define WM8903_MICDET_HYST_ENA                  0x0080  
#define WM8903_MICDET_HYST_ENA_MASK             0x0080  
#define WM8903_MICDET_HYST_ENA_SHIFT                 7  
#define WM8903_MICDET_HYST_ENA_WIDTH                 1  
#define WM8903_MICDET_THR_MASK                  0x0070  
#define WM8903_MICDET_THR_SHIFT                      4  
#define WM8903_MICDET_THR_WIDTH                      3  
#define WM8903_MICSHORT_THR_MASK                0x000C  
#define WM8903_MICSHORT_THR_SHIFT                    2  
#define WM8903_MICSHORT_THR_WIDTH                    2  
#define WM8903_MICDET_ENA                       0x0002  
#define WM8903_MICDET_ENA_MASK                  0x0002  
#define WM8903_MICDET_ENA_SHIFT                      1  
#define WM8903_MICDET_ENA_WIDTH                      1  
#define WM8903_MICBIAS_ENA                      0x0001  
#define WM8903_MICBIAS_ENA_MASK                 0x0001  
#define WM8903_MICBIAS_ENA_SHIFT                     0  
#define WM8903_MICBIAS_ENA_WIDTH                     1  


#define WM8903_DACBIAS_SEL_MASK                 0x0018  
#define WM8903_DACBIAS_SEL_SHIFT                     3  
#define WM8903_DACBIAS_SEL_WIDTH                     2  
#define WM8903_DACVMID_BIAS_SEL_MASK            0x0006  
#define WM8903_DACVMID_BIAS_SEL_SHIFT                1  
#define WM8903_DACVMID_BIAS_SEL_WIDTH                2  


#define WM8903_ADC_OSR128                       0x0001  
#define WM8903_ADC_OSR128_MASK                  0x0001  
#define WM8903_ADC_OSR128_SHIFT                      0  
#define WM8903_ADC_OSR128_WIDTH                      1  


#define WM8903_INL_ENA                          0x0002  
#define WM8903_INL_ENA_MASK                     0x0002  
#define WM8903_INL_ENA_SHIFT                         1  
#define WM8903_INL_ENA_WIDTH                         1  
#define WM8903_INR_ENA                          0x0001  
#define WM8903_INR_ENA_MASK                     0x0001  
#define WM8903_INR_ENA_SHIFT                         0  
#define WM8903_INR_ENA_WIDTH                         1  


#define WM8903_MIXOUTL_ENA                      0x0002  
#define WM8903_MIXOUTL_ENA_MASK                 0x0002  
#define WM8903_MIXOUTL_ENA_SHIFT                     1  
#define WM8903_MIXOUTL_ENA_WIDTH                     1  
#define WM8903_MIXOUTR_ENA                      0x0001  
#define WM8903_MIXOUTR_ENA_MASK                 0x0001  
#define WM8903_MIXOUTR_ENA_SHIFT                     0  
#define WM8903_MIXOUTR_ENA_WIDTH                     1  


#define WM8903_HPL_PGA_ENA                      0x0002  
#define WM8903_HPL_PGA_ENA_MASK                 0x0002  
#define WM8903_HPL_PGA_ENA_SHIFT                     1  
#define WM8903_HPL_PGA_ENA_WIDTH                     1  
#define WM8903_HPR_PGA_ENA                      0x0001  
#define WM8903_HPR_PGA_ENA_MASK                 0x0001  
#define WM8903_HPR_PGA_ENA_SHIFT                     0  
#define WM8903_HPR_PGA_ENA_WIDTH                     1  


#define WM8903_LINEOUTL_PGA_ENA                 0x0002  
#define WM8903_LINEOUTL_PGA_ENA_MASK            0x0002  
#define WM8903_LINEOUTL_PGA_ENA_SHIFT                1  
#define WM8903_LINEOUTL_PGA_ENA_WIDTH                1  
#define WM8903_LINEOUTR_PGA_ENA                 0x0001  
#define WM8903_LINEOUTR_PGA_ENA_MASK            0x0001  
#define WM8903_LINEOUTR_PGA_ENA_SHIFT                0  
#define WM8903_LINEOUTR_PGA_ENA_WIDTH                1  


#define WM8903_MIXSPKL_ENA                      0x0002  
#define WM8903_MIXSPKL_ENA_MASK                 0x0002  
#define WM8903_MIXSPKL_ENA_SHIFT                     1  
#define WM8903_MIXSPKL_ENA_WIDTH                     1  
#define WM8903_MIXSPKR_ENA                      0x0001  
#define WM8903_MIXSPKR_ENA_MASK                 0x0001  
#define WM8903_MIXSPKR_ENA_SHIFT                     0  
#define WM8903_MIXSPKR_ENA_WIDTH                     1  


#define WM8903_SPKL_ENA                         0x0002  
#define WM8903_SPKL_ENA_MASK                    0x0002  
#define WM8903_SPKL_ENA_SHIFT                        1  
#define WM8903_SPKL_ENA_WIDTH                        1  
#define WM8903_SPKR_ENA                         0x0001  
#define WM8903_SPKR_ENA_MASK                    0x0001  
#define WM8903_SPKR_ENA_SHIFT                        0  
#define WM8903_SPKR_ENA_WIDTH                        1  


#define WM8903_DACL_ENA                         0x0008  
#define WM8903_DACL_ENA_MASK                    0x0008  
#define WM8903_DACL_ENA_SHIFT                        3  
#define WM8903_DACL_ENA_WIDTH                        1  
#define WM8903_DACR_ENA                         0x0004  
#define WM8903_DACR_ENA_MASK                    0x0004  
#define WM8903_DACR_ENA_SHIFT                        2  
#define WM8903_DACR_ENA_WIDTH                        1  
#define WM8903_ADCL_ENA                         0x0002  
#define WM8903_ADCL_ENA_MASK                    0x0002  
#define WM8903_ADCL_ENA_SHIFT                        1  
#define WM8903_ADCL_ENA_WIDTH                        1  
#define WM8903_ADCR_ENA                         0x0001  
#define WM8903_ADCR_ENA_MASK                    0x0001  
#define WM8903_ADCR_ENA_SHIFT                        0  
#define WM8903_ADCR_ENA_WIDTH                        1  


#define WM8903_MCLKDIV2                         0x0001  
#define WM8903_MCLKDIV2_MASK                    0x0001  
#define WM8903_MCLKDIV2_SHIFT                        0  
#define WM8903_MCLKDIV2_WIDTH                        1  


#define WM8903_CLK_SYS_RATE_MASK                0x3C00  
#define WM8903_CLK_SYS_RATE_SHIFT                   10  
#define WM8903_CLK_SYS_RATE_WIDTH                    4  
#define WM8903_CLK_SYS_MODE_MASK                0x0300  
#define WM8903_CLK_SYS_MODE_SHIFT                    8  
#define WM8903_CLK_SYS_MODE_WIDTH                    2  
#define WM8903_SAMPLE_RATE_MASK                 0x000F  
#define WM8903_SAMPLE_RATE_SHIFT                     0  
#define WM8903_SAMPLE_RATE_WIDTH                     4  


#define WM8903_CLK_SYS_ENA                      0x0004  
#define WM8903_CLK_SYS_ENA_MASK                 0x0004  
#define WM8903_CLK_SYS_ENA_SHIFT                     2  
#define WM8903_CLK_SYS_ENA_WIDTH                     1  
#define WM8903_CLK_DSP_ENA                      0x0002  
#define WM8903_CLK_DSP_ENA_MASK                 0x0002  
#define WM8903_CLK_DSP_ENA_SHIFT                     1  
#define WM8903_CLK_DSP_ENA_WIDTH                     1  
#define WM8903_TO_ENA                           0x0001  
#define WM8903_TO_ENA_MASK                      0x0001  
#define WM8903_TO_ENA_SHIFT                          0  
#define WM8903_TO_ENA_WIDTH                          1  


#define WM8903_DACL_DATINV                      0x1000  
#define WM8903_DACL_DATINV_MASK                 0x1000  
#define WM8903_DACL_DATINV_SHIFT                    12  
#define WM8903_DACL_DATINV_WIDTH                     1  
#define WM8903_DACR_DATINV                      0x0800  
#define WM8903_DACR_DATINV_MASK                 0x0800  
#define WM8903_DACR_DATINV_SHIFT                    11  
#define WM8903_DACR_DATINV_WIDTH                     1  
#define WM8903_DAC_BOOST_MASK                   0x0600  
#define WM8903_DAC_BOOST_SHIFT                       9  
#define WM8903_DAC_BOOST_WIDTH                       2  
#define WM8903_LOOPBACK                         0x0100  
#define WM8903_LOOPBACK_MASK                    0x0100  
#define WM8903_LOOPBACK_SHIFT                        8  
#define WM8903_LOOPBACK_WIDTH                        1  
#define WM8903_AIFADCL_SRC                      0x0080  
#define WM8903_AIFADCL_SRC_MASK                 0x0080  
#define WM8903_AIFADCL_SRC_SHIFT                     7  
#define WM8903_AIFADCL_SRC_WIDTH                     1  
#define WM8903_AIFADCR_SRC                      0x0040  
#define WM8903_AIFADCR_SRC_MASK                 0x0040  
#define WM8903_AIFADCR_SRC_SHIFT                     6  
#define WM8903_AIFADCR_SRC_WIDTH                     1  
#define WM8903_AIFDACL_SRC                      0x0020  
#define WM8903_AIFDACL_SRC_MASK                 0x0020  
#define WM8903_AIFDACL_SRC_SHIFT                     5  
#define WM8903_AIFDACL_SRC_WIDTH                     1  
#define WM8903_AIFDACR_SRC                      0x0010  
#define WM8903_AIFDACR_SRC_MASK                 0x0010  
#define WM8903_AIFDACR_SRC_SHIFT                     4  
#define WM8903_AIFDACR_SRC_WIDTH                     1  
#define WM8903_ADC_COMP                         0x0008  
#define WM8903_ADC_COMP_MASK                    0x0008  
#define WM8903_ADC_COMP_SHIFT                        3  
#define WM8903_ADC_COMP_WIDTH                        1  
#define WM8903_ADC_COMPMODE                     0x0004  
#define WM8903_ADC_COMPMODE_MASK                0x0004  
#define WM8903_ADC_COMPMODE_SHIFT                    2  
#define WM8903_ADC_COMPMODE_WIDTH                    1  
#define WM8903_DAC_COMP                         0x0002  
#define WM8903_DAC_COMP_MASK                    0x0002  
#define WM8903_DAC_COMP_SHIFT                        1  
#define WM8903_DAC_COMP_WIDTH                        1  
#define WM8903_DAC_COMPMODE                     0x0001  
#define WM8903_DAC_COMPMODE_MASK                0x0001  
#define WM8903_DAC_COMPMODE_SHIFT                    0  
#define WM8903_DAC_COMPMODE_WIDTH                    1  


#define WM8903_AIFDAC_TDM                       0x2000  
#define WM8903_AIFDAC_TDM_MASK                  0x2000  
#define WM8903_AIFDAC_TDM_SHIFT                     13  
#define WM8903_AIFDAC_TDM_WIDTH                      1  
#define WM8903_AIFDAC_TDM_CHAN                  0x1000  
#define WM8903_AIFDAC_TDM_CHAN_MASK             0x1000  
#define WM8903_AIFDAC_TDM_CHAN_SHIFT                12  
#define WM8903_AIFDAC_TDM_CHAN_WIDTH                 1  
#define WM8903_AIFADC_TDM                       0x0800  
#define WM8903_AIFADC_TDM_MASK                  0x0800  
#define WM8903_AIFADC_TDM_SHIFT                     11  
#define WM8903_AIFADC_TDM_WIDTH                      1  
#define WM8903_AIFADC_TDM_CHAN                  0x0400  
#define WM8903_AIFADC_TDM_CHAN_MASK             0x0400  
#define WM8903_AIFADC_TDM_CHAN_SHIFT                10  
#define WM8903_AIFADC_TDM_CHAN_WIDTH                 1  
#define WM8903_LRCLK_DIR                        0x0200  
#define WM8903_LRCLK_DIR_MASK                   0x0200  
#define WM8903_LRCLK_DIR_SHIFT                       9  
#define WM8903_LRCLK_DIR_WIDTH                       1  
#define WM8903_AIF_BCLK_INV                     0x0080  
#define WM8903_AIF_BCLK_INV_MASK                0x0080  
#define WM8903_AIF_BCLK_INV_SHIFT                    7  
#define WM8903_AIF_BCLK_INV_WIDTH                    1  
#define WM8903_BCLK_DIR                         0x0040  
#define WM8903_BCLK_DIR_MASK                    0x0040  
#define WM8903_BCLK_DIR_SHIFT                        6  
#define WM8903_BCLK_DIR_WIDTH                        1  
#define WM8903_AIF_LRCLK_INV                    0x0010  
#define WM8903_AIF_LRCLK_INV_MASK               0x0010  
#define WM8903_AIF_LRCLK_INV_SHIFT                   4  
#define WM8903_AIF_LRCLK_INV_WIDTH                   1  
#define WM8903_AIF_WL_MASK                      0x000C  
#define WM8903_AIF_WL_SHIFT                          2  
#define WM8903_AIF_WL_WIDTH                          2  
#define WM8903_AIF_FMT_MASK                     0x0003  
#define WM8903_AIF_FMT_SHIFT                         0  
#define WM8903_AIF_FMT_WIDTH                         2  


#define WM8903_BCLK_DIV_MASK                    0x001F  
#define WM8903_BCLK_DIV_SHIFT                        0  
#define WM8903_BCLK_DIV_WIDTH                        5  


#define WM8903_LRCLK_RATE_MASK                  0x07FF  
#define WM8903_LRCLK_RATE_SHIFT                      0  
#define WM8903_LRCLK_RATE_WIDTH                     11  


#define WM8903_DACVU                            0x0100  
#define WM8903_DACVU_MASK                       0x0100  
#define WM8903_DACVU_SHIFT                           8  
#define WM8903_DACVU_WIDTH                           1  
#define WM8903_DACL_VOL_MASK                    0x00FF  
#define WM8903_DACL_VOL_SHIFT                        0  
#define WM8903_DACL_VOL_WIDTH                        8  


#define WM8903_DACVU                            0x0100  
#define WM8903_DACVU_MASK                       0x0100  
#define WM8903_DACVU_SHIFT                           8  
#define WM8903_DACVU_WIDTH                           1  
#define WM8903_DACR_VOL_MASK                    0x00FF  
#define WM8903_DACR_VOL_SHIFT                        0  
#define WM8903_DACR_VOL_WIDTH                        8  


#define WM8903_ADCL_DAC_SVOL_MASK               0x0F00  
#define WM8903_ADCL_DAC_SVOL_SHIFT                   8  
#define WM8903_ADCL_DAC_SVOL_WIDTH                   4  
#define WM8903_ADCR_DAC_SVOL_MASK               0x00F0  
#define WM8903_ADCR_DAC_SVOL_SHIFT                   4  
#define WM8903_ADCR_DAC_SVOL_WIDTH                   4  
#define WM8903_ADC_TO_DACL_MASK                 0x000C  
#define WM8903_ADC_TO_DACL_SHIFT                     2  
#define WM8903_ADC_TO_DACL_WIDTH                     2  
#define WM8903_ADC_TO_DACR_MASK                 0x0003  
#define WM8903_ADC_TO_DACR_SHIFT                     0  
#define WM8903_ADC_TO_DACR_WIDTH                     2  


#define WM8903_DAC_MONO                         0x1000  
#define WM8903_DAC_MONO_MASK                    0x1000  
#define WM8903_DAC_MONO_SHIFT                       12  
#define WM8903_DAC_MONO_WIDTH                        1  
#define WM8903_DAC_SB_FILT                      0x0800  
#define WM8903_DAC_SB_FILT_MASK                 0x0800  
#define WM8903_DAC_SB_FILT_SHIFT                    11  
#define WM8903_DAC_SB_FILT_WIDTH                     1  
#define WM8903_DAC_MUTERATE                     0x0400  
#define WM8903_DAC_MUTERATE_MASK                0x0400  
#define WM8903_DAC_MUTERATE_SHIFT                   10  
#define WM8903_DAC_MUTERATE_WIDTH                    1  
#define WM8903_DAC_MUTEMODE                     0x0200  
#define WM8903_DAC_MUTEMODE_MASK                0x0200  
#define WM8903_DAC_MUTEMODE_SHIFT                    9  
#define WM8903_DAC_MUTEMODE_WIDTH                    1  
#define WM8903_DAC_MUTE                         0x0008  
#define WM8903_DAC_MUTE_MASK                    0x0008  
#define WM8903_DAC_MUTE_SHIFT                        3  
#define WM8903_DAC_MUTE_WIDTH                        1  
#define WM8903_DEEMPH_MASK                      0x0006  
#define WM8903_DEEMPH_SHIFT                          1  
#define WM8903_DEEMPH_WIDTH                          2  


#define WM8903_ADCVU                            0x0100  
#define WM8903_ADCVU_MASK                       0x0100  
#define WM8903_ADCVU_SHIFT                           8  
#define WM8903_ADCVU_WIDTH                           1  
#define WM8903_ADCL_VOL_MASK                    0x00FF  
#define WM8903_ADCL_VOL_SHIFT                        0  
#define WM8903_ADCL_VOL_WIDTH                        8  


#define WM8903_ADCVU                            0x0100  
#define WM8903_ADCVU_MASK                       0x0100  
#define WM8903_ADCVU_SHIFT                           8  
#define WM8903_ADCVU_WIDTH                           1  
#define WM8903_ADCR_VOL_MASK                    0x00FF  
#define WM8903_ADCR_VOL_SHIFT                        0  
#define WM8903_ADCR_VOL_WIDTH                        8  


#define WM8903_ADC_HPF_CUT_MASK                 0x0060  
#define WM8903_ADC_HPF_CUT_SHIFT                     5  
#define WM8903_ADC_HPF_CUT_WIDTH                     2  
#define WM8903_ADC_HPF_ENA                      0x0010  
#define WM8903_ADC_HPF_ENA_MASK                 0x0010  
#define WM8903_ADC_HPF_ENA_SHIFT                     4  
#define WM8903_ADC_HPF_ENA_WIDTH                     1  
#define WM8903_ADCL_DATINV                      0x0002  
#define WM8903_ADCL_DATINV_MASK                 0x0002  
#define WM8903_ADCL_DATINV_SHIFT                     1  
#define WM8903_ADCL_DATINV_WIDTH                     1  
#define WM8903_ADCR_DATINV                      0x0001  
#define WM8903_ADCR_DATINV_MASK                 0x0001  
#define WM8903_ADCR_DATINV_SHIFT                     0  
#define WM8903_ADCR_DATINV_WIDTH                     1  


#define WM8903_DIGMIC_MODE_SEL                  0x0100  
#define WM8903_DIGMIC_MODE_SEL_MASK             0x0100  
#define WM8903_DIGMIC_MODE_SEL_SHIFT                 8  
#define WM8903_DIGMIC_MODE_SEL_WIDTH                 1  
#define WM8903_DIGMIC_CLK_SEL_L_MASK            0x00C0  
#define WM8903_DIGMIC_CLK_SEL_L_SHIFT                6  
#define WM8903_DIGMIC_CLK_SEL_L_WIDTH                2  
#define WM8903_DIGMIC_CLK_SEL_R_MASK            0x0030  
#define WM8903_DIGMIC_CLK_SEL_R_SHIFT                4  
#define WM8903_DIGMIC_CLK_SEL_R_WIDTH                2  
#define WM8903_DIGMIC_CLK_SEL_RT_MASK           0x000C  
#define WM8903_DIGMIC_CLK_SEL_RT_SHIFT               2  
#define WM8903_DIGMIC_CLK_SEL_RT_WIDTH               2  
#define WM8903_DIGMIC_CLK_SEL_MASK              0x0003  
#define WM8903_DIGMIC_CLK_SEL_SHIFT                  0  
#define WM8903_DIGMIC_CLK_SEL_WIDTH                  2  


#define WM8903_DRC_ENA                          0x8000  
#define WM8903_DRC_ENA_MASK                     0x8000  
#define WM8903_DRC_ENA_SHIFT                        15  
#define WM8903_DRC_ENA_WIDTH                         1  
#define WM8903_DRC_THRESH_HYST_MASK             0x1800  
#define WM8903_DRC_THRESH_HYST_SHIFT                11  
#define WM8903_DRC_THRESH_HYST_WIDTH                 2  
#define WM8903_DRC_STARTUP_GAIN_MASK            0x07C0  
#define WM8903_DRC_STARTUP_GAIN_SHIFT                6  
#define WM8903_DRC_STARTUP_GAIN_WIDTH                5  
#define WM8903_DRC_FF_DELAY                     0x0020  
#define WM8903_DRC_FF_DELAY_MASK                0x0020  
#define WM8903_DRC_FF_DELAY_SHIFT                    5  
#define WM8903_DRC_FF_DELAY_WIDTH                    1  
#define WM8903_DRC_SMOOTH_ENA                   0x0008  
#define WM8903_DRC_SMOOTH_ENA_MASK              0x0008  
#define WM8903_DRC_SMOOTH_ENA_SHIFT                  3  
#define WM8903_DRC_SMOOTH_ENA_WIDTH                  1  
#define WM8903_DRC_QR_ENA                       0x0004  
#define WM8903_DRC_QR_ENA_MASK                  0x0004  
#define WM8903_DRC_QR_ENA_SHIFT                      2  
#define WM8903_DRC_QR_ENA_WIDTH                      1  
#define WM8903_DRC_ANTICLIP_ENA                 0x0002  
#define WM8903_DRC_ANTICLIP_ENA_MASK            0x0002  
#define WM8903_DRC_ANTICLIP_ENA_SHIFT                1  
#define WM8903_DRC_ANTICLIP_ENA_WIDTH                1  
#define WM8903_DRC_HYST_ENA                     0x0001  
#define WM8903_DRC_HYST_ENA_MASK                0x0001  
#define WM8903_DRC_HYST_ENA_SHIFT                    0  
#define WM8903_DRC_HYST_ENA_WIDTH                    1  


#define WM8903_DRC_ATTACK_RATE_MASK             0xF000  
#define WM8903_DRC_ATTACK_RATE_SHIFT                12  
#define WM8903_DRC_ATTACK_RATE_WIDTH                 4  
#define WM8903_DRC_DECAY_RATE_MASK              0x0F00  
#define WM8903_DRC_DECAY_RATE_SHIFT                  8  
#define WM8903_DRC_DECAY_RATE_WIDTH                  4  
#define WM8903_DRC_THRESH_QR_MASK               0x00C0  
#define WM8903_DRC_THRESH_QR_SHIFT                   6  
#define WM8903_DRC_THRESH_QR_WIDTH                   2  
#define WM8903_DRC_RATE_QR_MASK                 0x0030  
#define WM8903_DRC_RATE_QR_SHIFT                     4  
#define WM8903_DRC_RATE_QR_WIDTH                     2  
#define WM8903_DRC_MINGAIN_MASK                 0x000C  
#define WM8903_DRC_MINGAIN_SHIFT                     2  
#define WM8903_DRC_MINGAIN_WIDTH                     2  
#define WM8903_DRC_MAXGAIN_MASK                 0x0003  
#define WM8903_DRC_MAXGAIN_SHIFT                     0  
#define WM8903_DRC_MAXGAIN_WIDTH                     2  


#define WM8903_DRC_R0_SLOPE_COMP_MASK           0x0038  
#define WM8903_DRC_R0_SLOPE_COMP_SHIFT               3  
#define WM8903_DRC_R0_SLOPE_COMP_WIDTH               3  
#define WM8903_DRC_R1_SLOPE_COMP_MASK           0x0007  
#define WM8903_DRC_R1_SLOPE_COMP_SHIFT               0  
#define WM8903_DRC_R1_SLOPE_COMP_WIDTH               3  


#define WM8903_DRC_THRESH_COMP_MASK             0x07E0  
#define WM8903_DRC_THRESH_COMP_SHIFT                 5  
#define WM8903_DRC_THRESH_COMP_WIDTH                 6  
#define WM8903_DRC_AMP_COMP_MASK                0x001F  
#define WM8903_DRC_AMP_COMP_SHIFT                    0  
#define WM8903_DRC_AMP_COMP_WIDTH                    5  


#define WM8903_LINMUTE                          0x0080  
#define WM8903_LINMUTE_MASK                     0x0080  
#define WM8903_LINMUTE_SHIFT                         7  
#define WM8903_LINMUTE_WIDTH                         1  
#define WM8903_LIN_VOL_MASK                     0x001F  
#define WM8903_LIN_VOL_SHIFT                         0  
#define WM8903_LIN_VOL_WIDTH                         5  


#define WM8903_RINMUTE                          0x0080  
#define WM8903_RINMUTE_MASK                     0x0080  
#define WM8903_RINMUTE_SHIFT                         7  
#define WM8903_RINMUTE_WIDTH                         1  
#define WM8903_RIN_VOL_MASK                     0x001F  
#define WM8903_RIN_VOL_SHIFT                         0  
#define WM8903_RIN_VOL_WIDTH                         5  


#define WM8903_INL_CM_ENA                       0x0040  
#define WM8903_INL_CM_ENA_MASK                  0x0040  
#define WM8903_INL_CM_ENA_SHIFT                      6  
#define WM8903_INL_CM_ENA_WIDTH                      1  
#define WM8903_L_IP_SEL_N_MASK                  0x0030  
#define WM8903_L_IP_SEL_N_SHIFT                      4  
#define WM8903_L_IP_SEL_N_WIDTH                      2  
#define WM8903_L_IP_SEL_P_MASK                  0x000C  
#define WM8903_L_IP_SEL_P_SHIFT                      2  
#define WM8903_L_IP_SEL_P_WIDTH                      2  
#define WM8903_L_MODE_MASK                      0x0003  
#define WM8903_L_MODE_SHIFT                          0  
#define WM8903_L_MODE_WIDTH                          2  


#define WM8903_INR_CM_ENA                       0x0040  
#define WM8903_INR_CM_ENA_MASK                  0x0040  
#define WM8903_INR_CM_ENA_SHIFT                      6  
#define WM8903_INR_CM_ENA_WIDTH                      1  
#define WM8903_R_IP_SEL_N_MASK                  0x0030  
#define WM8903_R_IP_SEL_N_SHIFT                      4  
#define WM8903_R_IP_SEL_N_WIDTH                      2  
#define WM8903_R_IP_SEL_P_MASK                  0x000C  
#define WM8903_R_IP_SEL_P_SHIFT                      2  
#define WM8903_R_IP_SEL_P_WIDTH                      2  
#define WM8903_R_MODE_MASK                      0x0003  
#define WM8903_R_MODE_SHIFT                          0  
#define WM8903_R_MODE_WIDTH                          2  


#define WM8903_DACL_TO_MIXOUTL                  0x0008  
#define WM8903_DACL_TO_MIXOUTL_MASK             0x0008  
#define WM8903_DACL_TO_MIXOUTL_SHIFT                 3  
#define WM8903_DACL_TO_MIXOUTL_WIDTH                 1  
#define WM8903_DACR_TO_MIXOUTL                  0x0004  
#define WM8903_DACR_TO_MIXOUTL_MASK             0x0004  
#define WM8903_DACR_TO_MIXOUTL_SHIFT                 2  
#define WM8903_DACR_TO_MIXOUTL_WIDTH                 1  
#define WM8903_BYPASSL_TO_MIXOUTL               0x0002  
#define WM8903_BYPASSL_TO_MIXOUTL_MASK          0x0002  
#define WM8903_BYPASSL_TO_MIXOUTL_SHIFT              1  
#define WM8903_BYPASSL_TO_MIXOUTL_WIDTH              1  
#define WM8903_BYPASSR_TO_MIXOUTL               0x0001  
#define WM8903_BYPASSR_TO_MIXOUTL_MASK          0x0001  
#define WM8903_BYPASSR_TO_MIXOUTL_SHIFT              0  
#define WM8903_BYPASSR_TO_MIXOUTL_WIDTH              1  


#define WM8903_DACL_TO_MIXOUTR                  0x0008  
#define WM8903_DACL_TO_MIXOUTR_MASK             0x0008  
#define WM8903_DACL_TO_MIXOUTR_SHIFT                 3  
#define WM8903_DACL_TO_MIXOUTR_WIDTH                 1  
#define WM8903_DACR_TO_MIXOUTR                  0x0004  
#define WM8903_DACR_TO_MIXOUTR_MASK             0x0004  
#define WM8903_DACR_TO_MIXOUTR_SHIFT                 2  
#define WM8903_DACR_TO_MIXOUTR_WIDTH                 1  
#define WM8903_BYPASSL_TO_MIXOUTR               0x0002  
#define WM8903_BYPASSL_TO_MIXOUTR_MASK          0x0002  
#define WM8903_BYPASSL_TO_MIXOUTR_SHIFT              1  
#define WM8903_BYPASSL_TO_MIXOUTR_WIDTH              1  
#define WM8903_BYPASSR_TO_MIXOUTR               0x0001  
#define WM8903_BYPASSR_TO_MIXOUTR_MASK          0x0001  
#define WM8903_BYPASSR_TO_MIXOUTR_SHIFT              0  
#define WM8903_BYPASSR_TO_MIXOUTR_WIDTH              1  


#define WM8903_DACL_TO_MIXSPKL                  0x0008  
#define WM8903_DACL_TO_MIXSPKL_MASK             0x0008  
#define WM8903_DACL_TO_MIXSPKL_SHIFT                 3  
#define WM8903_DACL_TO_MIXSPKL_WIDTH                 1  
#define WM8903_DACR_TO_MIXSPKL                  0x0004  
#define WM8903_DACR_TO_MIXSPKL_MASK             0x0004  
#define WM8903_DACR_TO_MIXSPKL_SHIFT                 2  
#define WM8903_DACR_TO_MIXSPKL_WIDTH                 1  
#define WM8903_BYPASSL_TO_MIXSPKL               0x0002  
#define WM8903_BYPASSL_TO_MIXSPKL_MASK          0x0002  
#define WM8903_BYPASSL_TO_MIXSPKL_SHIFT              1  
#define WM8903_BYPASSL_TO_MIXSPKL_WIDTH              1  
#define WM8903_BYPASSR_TO_MIXSPKL               0x0001  
#define WM8903_BYPASSR_TO_MIXSPKL_MASK          0x0001  
#define WM8903_BYPASSR_TO_MIXSPKL_SHIFT              0  
#define WM8903_BYPASSR_TO_MIXSPKL_WIDTH              1  


#define WM8903_DACL_MIXSPKL_VOL                 0x0008  
#define WM8903_DACL_MIXSPKL_VOL_MASK            0x0008  
#define WM8903_DACL_MIXSPKL_VOL_SHIFT                3  
#define WM8903_DACL_MIXSPKL_VOL_WIDTH                1  
#define WM8903_DACR_MIXSPKL_VOL                 0x0004  
#define WM8903_DACR_MIXSPKL_VOL_MASK            0x0004  
#define WM8903_DACR_MIXSPKL_VOL_SHIFT                2  
#define WM8903_DACR_MIXSPKL_VOL_WIDTH                1  
#define WM8903_BYPASSL_MIXSPKL_VOL              0x0002  
#define WM8903_BYPASSL_MIXSPKL_VOL_MASK         0x0002  
#define WM8903_BYPASSL_MIXSPKL_VOL_SHIFT             1  
#define WM8903_BYPASSL_MIXSPKL_VOL_WIDTH             1  
#define WM8903_BYPASSR_MIXSPKL_VOL              0x0001  
#define WM8903_BYPASSR_MIXSPKL_VOL_MASK         0x0001  
#define WM8903_BYPASSR_MIXSPKL_VOL_SHIFT             0  
#define WM8903_BYPASSR_MIXSPKL_VOL_WIDTH             1  


#define WM8903_DACL_TO_MIXSPKR                  0x0008  
#define WM8903_DACL_TO_MIXSPKR_MASK             0x0008  
#define WM8903_DACL_TO_MIXSPKR_SHIFT                 3  
#define WM8903_DACL_TO_MIXSPKR_WIDTH                 1  
#define WM8903_DACR_TO_MIXSPKR                  0x0004  
#define WM8903_DACR_TO_MIXSPKR_MASK             0x0004  
#define WM8903_DACR_TO_MIXSPKR_SHIFT                 2  
#define WM8903_DACR_TO_MIXSPKR_WIDTH                 1  
#define WM8903_BYPASSL_TO_MIXSPKR               0x0002  
#define WM8903_BYPASSL_TO_MIXSPKR_MASK          0x0002  
#define WM8903_BYPASSL_TO_MIXSPKR_SHIFT              1  
#define WM8903_BYPASSL_TO_MIXSPKR_WIDTH              1  
#define WM8903_BYPASSR_TO_MIXSPKR               0x0001  
#define WM8903_BYPASSR_TO_MIXSPKR_MASK          0x0001  
#define WM8903_BYPASSR_TO_MIXSPKR_SHIFT              0  
#define WM8903_BYPASSR_TO_MIXSPKR_WIDTH              1  


#define WM8903_DACL_MIXSPKR_VOL                 0x0008  
#define WM8903_DACL_MIXSPKR_VOL_MASK            0x0008  
#define WM8903_DACL_MIXSPKR_VOL_SHIFT                3  
#define WM8903_DACL_MIXSPKR_VOL_WIDTH                1  
#define WM8903_DACR_MIXSPKR_VOL                 0x0004  
#define WM8903_DACR_MIXSPKR_VOL_MASK            0x0004  
#define WM8903_DACR_MIXSPKR_VOL_SHIFT                2  
#define WM8903_DACR_MIXSPKR_VOL_WIDTH                1  
#define WM8903_BYPASSL_MIXSPKR_VOL              0x0002  
#define WM8903_BYPASSL_MIXSPKR_VOL_MASK         0x0002  
#define WM8903_BYPASSL_MIXSPKR_VOL_SHIFT             1  
#define WM8903_BYPASSL_MIXSPKR_VOL_WIDTH             1  
#define WM8903_BYPASSR_MIXSPKR_VOL              0x0001  
#define WM8903_BYPASSR_MIXSPKR_VOL_MASK         0x0001  
#define WM8903_BYPASSR_MIXSPKR_VOL_SHIFT             0  
#define WM8903_BYPASSR_MIXSPKR_VOL_WIDTH             1  


#define WM8903_HPL_MUTE                         0x0100  
#define WM8903_HPL_MUTE_MASK                    0x0100  
#define WM8903_HPL_MUTE_SHIFT                        8  
#define WM8903_HPL_MUTE_WIDTH                        1  
#define WM8903_HPOUTVU                          0x0080  
#define WM8903_HPOUTVU_MASK                     0x0080  
#define WM8903_HPOUTVU_SHIFT                         7  
#define WM8903_HPOUTVU_WIDTH                         1  
#define WM8903_HPOUTLZC                         0x0040  
#define WM8903_HPOUTLZC_MASK                    0x0040  
#define WM8903_HPOUTLZC_SHIFT                        6  
#define WM8903_HPOUTLZC_WIDTH                        1  
#define WM8903_HPOUTL_VOL_MASK                  0x003F  
#define WM8903_HPOUTL_VOL_SHIFT                      0  
#define WM8903_HPOUTL_VOL_WIDTH                      6  


#define WM8903_HPR_MUTE                         0x0100  
#define WM8903_HPR_MUTE_MASK                    0x0100  
#define WM8903_HPR_MUTE_SHIFT                        8  
#define WM8903_HPR_MUTE_WIDTH                        1  
#define WM8903_HPOUTVU                          0x0080  
#define WM8903_HPOUTVU_MASK                     0x0080  
#define WM8903_HPOUTVU_SHIFT                         7  
#define WM8903_HPOUTVU_WIDTH                         1  
#define WM8903_HPOUTRZC                         0x0040  
#define WM8903_HPOUTRZC_MASK                    0x0040  
#define WM8903_HPOUTRZC_SHIFT                        6  
#define WM8903_HPOUTRZC_WIDTH                        1  
#define WM8903_HPOUTR_VOL_MASK                  0x003F  
#define WM8903_HPOUTR_VOL_SHIFT                      0  
#define WM8903_HPOUTR_VOL_WIDTH                      6  


#define WM8903_LINEOUTL_MUTE                    0x0100  
#define WM8903_LINEOUTL_MUTE_MASK               0x0100  
#define WM8903_LINEOUTL_MUTE_SHIFT                   8  
#define WM8903_LINEOUTL_MUTE_WIDTH                   1  
#define WM8903_LINEOUTVU                        0x0080  
#define WM8903_LINEOUTVU_MASK                   0x0080  
#define WM8903_LINEOUTVU_SHIFT                       7  
#define WM8903_LINEOUTVU_WIDTH                       1  
#define WM8903_LINEOUTLZC                       0x0040  
#define WM8903_LINEOUTLZC_MASK                  0x0040  
#define WM8903_LINEOUTLZC_SHIFT                      6  
#define WM8903_LINEOUTLZC_WIDTH                      1  
#define WM8903_LINEOUTL_VOL_MASK                0x003F  
#define WM8903_LINEOUTL_VOL_SHIFT                    0  
#define WM8903_LINEOUTL_VOL_WIDTH                    6  


#define WM8903_LINEOUTR_MUTE                    0x0100  
#define WM8903_LINEOUTR_MUTE_MASK               0x0100  
#define WM8903_LINEOUTR_MUTE_SHIFT                   8  
#define WM8903_LINEOUTR_MUTE_WIDTH                   1  
#define WM8903_LINEOUTVU                        0x0080  
#define WM8903_LINEOUTVU_MASK                   0x0080  
#define WM8903_LINEOUTVU_SHIFT                       7  
#define WM8903_LINEOUTVU_WIDTH                       1  
#define WM8903_LINEOUTRZC                       0x0040  
#define WM8903_LINEOUTRZC_MASK                  0x0040  
#define WM8903_LINEOUTRZC_SHIFT                      6  
#define WM8903_LINEOUTRZC_WIDTH                      1  
#define WM8903_LINEOUTR_VOL_MASK                0x003F  
#define WM8903_LINEOUTR_VOL_SHIFT                    0  
#define WM8903_LINEOUTR_VOL_WIDTH                    6  


#define WM8903_SPKL_MUTE                        0x0100  
#define WM8903_SPKL_MUTE_MASK                   0x0100  
#define WM8903_SPKL_MUTE_SHIFT                       8  
#define WM8903_SPKL_MUTE_WIDTH                       1  
#define WM8903_SPKVU                            0x0080  
#define WM8903_SPKVU_MASK                       0x0080  
#define WM8903_SPKVU_SHIFT                           7  
#define WM8903_SPKVU_WIDTH                           1  
#define WM8903_SPKLZC                           0x0040  
#define WM8903_SPKLZC_MASK                      0x0040  
#define WM8903_SPKLZC_SHIFT                          6  
#define WM8903_SPKLZC_WIDTH                          1  
#define WM8903_SPKL_VOL_MASK                    0x003F  
#define WM8903_SPKL_VOL_SHIFT                        0  
#define WM8903_SPKL_VOL_WIDTH                        6  


#define WM8903_SPKR_MUTE                        0x0100  
#define WM8903_SPKR_MUTE_MASK                   0x0100  
#define WM8903_SPKR_MUTE_SHIFT                       8  
#define WM8903_SPKR_MUTE_WIDTH                       1  
#define WM8903_SPKVU                            0x0080  
#define WM8903_SPKVU_MASK                       0x0080  
#define WM8903_SPKVU_SHIFT                           7  
#define WM8903_SPKVU_WIDTH                           1  
#define WM8903_SPKRZC                           0x0040  
#define WM8903_SPKRZC_MASK                      0x0040  
#define WM8903_SPKRZC_SHIFT                          6  
#define WM8903_SPKRZC_WIDTH                          1  
#define WM8903_SPKR_VOL_MASK                    0x003F  
#define WM8903_SPKR_VOL_SHIFT                        0  
#define WM8903_SPKR_VOL_WIDTH                        6  


#define WM8903_SPK_DISCHARGE                    0x0002  
#define WM8903_SPK_DISCHARGE_MASK               0x0002  
#define WM8903_SPK_DISCHARGE_SHIFT                   1  
#define WM8903_SPK_DISCHARGE_WIDTH                   1  
#define WM8903_VROI                             0x0001  
#define WM8903_VROI_MASK                        0x0001  
#define WM8903_VROI_SHIFT                            0  
#define WM8903_VROI_WIDTH                            1  


#define WM8903_DCS_MASTER_ENA                   0x0010  
#define WM8903_DCS_MASTER_ENA_MASK              0x0010  
#define WM8903_DCS_MASTER_ENA_SHIFT                  4  
#define WM8903_DCS_MASTER_ENA_WIDTH                  1  
#define WM8903_DCS_ENA_MASK                     0x000F  
#define WM8903_DCS_ENA_SHIFT                         0  
#define WM8903_DCS_ENA_WIDTH                         4  


#define WM8903_DCS_MODE_MASK                    0x0003  
#define WM8903_DCS_MODE_SHIFT                        0  
#define WM8903_DCS_MODE_WIDTH                        2  


#define WM8903_HPL_RMV_SHORT                    0x0080  
#define WM8903_HPL_RMV_SHORT_MASK               0x0080  
#define WM8903_HPL_RMV_SHORT_SHIFT                   7  
#define WM8903_HPL_RMV_SHORT_WIDTH                   1  
#define WM8903_HPL_ENA_OUTP                     0x0040  
#define WM8903_HPL_ENA_OUTP_MASK                0x0040  
#define WM8903_HPL_ENA_OUTP_SHIFT                    6  
#define WM8903_HPL_ENA_OUTP_WIDTH                    1  
#define WM8903_HPL_ENA_DLY                      0x0020  
#define WM8903_HPL_ENA_DLY_MASK                 0x0020  
#define WM8903_HPL_ENA_DLY_SHIFT                     5  
#define WM8903_HPL_ENA_DLY_WIDTH                     1  
#define WM8903_HPL_ENA                          0x0010  
#define WM8903_HPL_ENA_MASK                     0x0010  
#define WM8903_HPL_ENA_SHIFT                         4  
#define WM8903_HPL_ENA_WIDTH                         1  
#define WM8903_HPR_RMV_SHORT                    0x0008  
#define WM8903_HPR_RMV_SHORT_MASK               0x0008  
#define WM8903_HPR_RMV_SHORT_SHIFT                   3  
#define WM8903_HPR_RMV_SHORT_WIDTH                   1  
#define WM8903_HPR_ENA_OUTP                     0x0004  
#define WM8903_HPR_ENA_OUTP_MASK                0x0004  
#define WM8903_HPR_ENA_OUTP_SHIFT                    2  
#define WM8903_HPR_ENA_OUTP_WIDTH                    1  
#define WM8903_HPR_ENA_DLY                      0x0002  
#define WM8903_HPR_ENA_DLY_MASK                 0x0002  
#define WM8903_HPR_ENA_DLY_SHIFT                     1  
#define WM8903_HPR_ENA_DLY_WIDTH                     1  
#define WM8903_HPR_ENA                          0x0001  
#define WM8903_HPR_ENA_MASK                     0x0001  
#define WM8903_HPR_ENA_SHIFT                         0  
#define WM8903_HPR_ENA_WIDTH                         1  


#define WM8903_LINEOUTL_RMV_SHORT               0x0080  
#define WM8903_LINEOUTL_RMV_SHORT_MASK          0x0080  
#define WM8903_LINEOUTL_RMV_SHORT_SHIFT              7  
#define WM8903_LINEOUTL_RMV_SHORT_WIDTH              1  
#define WM8903_LINEOUTL_ENA_OUTP                0x0040  
#define WM8903_LINEOUTL_ENA_OUTP_MASK           0x0040  
#define WM8903_LINEOUTL_ENA_OUTP_SHIFT               6  
#define WM8903_LINEOUTL_ENA_OUTP_WIDTH               1  
#define WM8903_LINEOUTL_ENA_DLY                 0x0020  
#define WM8903_LINEOUTL_ENA_DLY_MASK            0x0020  
#define WM8903_LINEOUTL_ENA_DLY_SHIFT                5  
#define WM8903_LINEOUTL_ENA_DLY_WIDTH                1  
#define WM8903_LINEOUTL_ENA                     0x0010  
#define WM8903_LINEOUTL_ENA_MASK                0x0010  
#define WM8903_LINEOUTL_ENA_SHIFT                    4  
#define WM8903_LINEOUTL_ENA_WIDTH                    1  
#define WM8903_LINEOUTR_RMV_SHORT               0x0008  
#define WM8903_LINEOUTR_RMV_SHORT_MASK          0x0008  
#define WM8903_LINEOUTR_RMV_SHORT_SHIFT              3  
#define WM8903_LINEOUTR_RMV_SHORT_WIDTH              1  
#define WM8903_LINEOUTR_ENA_OUTP                0x0004  
#define WM8903_LINEOUTR_ENA_OUTP_MASK           0x0004  
#define WM8903_LINEOUTR_ENA_OUTP_SHIFT               2  
#define WM8903_LINEOUTR_ENA_OUTP_WIDTH               1  
#define WM8903_LINEOUTR_ENA_DLY                 0x0002  
#define WM8903_LINEOUTR_ENA_DLY_MASK            0x0002  
#define WM8903_LINEOUTR_ENA_DLY_SHIFT                1  
#define WM8903_LINEOUTR_ENA_DLY_WIDTH                1  
#define WM8903_LINEOUTR_ENA                     0x0001  
#define WM8903_LINEOUTR_ENA_MASK                0x0001  
#define WM8903_LINEOUTR_ENA_SHIFT                    0  
#define WM8903_LINEOUTR_ENA_WIDTH                    1  


#define WM8903_CP_ENA                           0x0001  
#define WM8903_CP_ENA_MASK                      0x0001  
#define WM8903_CP_ENA_SHIFT                          0  
#define WM8903_CP_ENA_WIDTH                          1  


#define WM8903_CP_DYN_FREQ                      0x0002  
#define WM8903_CP_DYN_FREQ_MASK                 0x0002  
#define WM8903_CP_DYN_FREQ_SHIFT                     1  
#define WM8903_CP_DYN_FREQ_WIDTH                     1  
#define WM8903_CP_DYN_V                         0x0001  
#define WM8903_CP_DYN_V_MASK                    0x0001  
#define WM8903_CP_DYN_V_SHIFT                        0  
#define WM8903_CP_DYN_V_WIDTH                        1  


#define WM8903_WSEQ_ENA                         0x0100  
#define WM8903_WSEQ_ENA_MASK                    0x0100  
#define WM8903_WSEQ_ENA_SHIFT                        8  
#define WM8903_WSEQ_ENA_WIDTH                        1  
#define WM8903_WSEQ_WRITE_INDEX_MASK            0x001F  
#define WM8903_WSEQ_WRITE_INDEX_SHIFT                0  
#define WM8903_WSEQ_WRITE_INDEX_WIDTH                5  


#define WM8903_WSEQ_DATA_WIDTH_MASK             0x7000  
#define WM8903_WSEQ_DATA_WIDTH_SHIFT                12  
#define WM8903_WSEQ_DATA_WIDTH_WIDTH                 3  
#define WM8903_WSEQ_DATA_START_MASK             0x0F00  
#define WM8903_WSEQ_DATA_START_SHIFT                 8  
#define WM8903_WSEQ_DATA_START_WIDTH                 4  
#define WM8903_WSEQ_ADDR_MASK                   0x00FF  
#define WM8903_WSEQ_ADDR_SHIFT                       0  
#define WM8903_WSEQ_ADDR_WIDTH                       8  


#define WM8903_WSEQ_EOS                         0x4000  
#define WM8903_WSEQ_EOS_MASK                    0x4000  
#define WM8903_WSEQ_EOS_SHIFT                       14  
#define WM8903_WSEQ_EOS_WIDTH                        1  
#define WM8903_WSEQ_DELAY_MASK                  0x0F00  
#define WM8903_WSEQ_DELAY_SHIFT                      8  
#define WM8903_WSEQ_DELAY_WIDTH                      4  
#define WM8903_WSEQ_DATA_MASK                   0x00FF  
#define WM8903_WSEQ_DATA_SHIFT                       0  
#define WM8903_WSEQ_DATA_WIDTH                       8  


#define WM8903_WSEQ_ABORT                       0x0200  
#define WM8903_WSEQ_ABORT_MASK                  0x0200  
#define WM8903_WSEQ_ABORT_SHIFT                      9  
#define WM8903_WSEQ_ABORT_WIDTH                      1  
#define WM8903_WSEQ_START                       0x0100  
#define WM8903_WSEQ_START_MASK                  0x0100  
#define WM8903_WSEQ_START_SHIFT                      8  
#define WM8903_WSEQ_START_WIDTH                      1  
#define WM8903_WSEQ_START_INDEX_MASK            0x003F  
#define WM8903_WSEQ_START_INDEX_SHIFT                0  
#define WM8903_WSEQ_START_INDEX_WIDTH                6  


#define WM8903_WSEQ_CURRENT_INDEX_MASK          0x03F0  
#define WM8903_WSEQ_CURRENT_INDEX_SHIFT              4  
#define WM8903_WSEQ_CURRENT_INDEX_WIDTH              6  
#define WM8903_WSEQ_BUSY                        0x0001  
#define WM8903_WSEQ_BUSY_MASK                   0x0001  
#define WM8903_WSEQ_BUSY_SHIFT                       0  
#define WM8903_WSEQ_BUSY_WIDTH                       1  


#define WM8903_MASK_WRITE_ENA                   0x0001  
#define WM8903_MASK_WRITE_ENA_MASK              0x0001  
#define WM8903_MASK_WRITE_ENA_SHIFT                  0  
#define WM8903_MASK_WRITE_ENA_WIDTH                  1  


#define WM8903_GP1_FN_MASK                      0x1F00  
#define WM8903_GP1_FN_SHIFT                          8  
#define WM8903_GP1_FN_WIDTH                          5  
#define WM8903_GP1_DIR                          0x0080  
#define WM8903_GP1_DIR_MASK                     0x0080  
#define WM8903_GP1_DIR_SHIFT                         7  
#define WM8903_GP1_DIR_WIDTH                         1  
#define WM8903_GP1_OP_CFG                       0x0040  
#define WM8903_GP1_OP_CFG_MASK                  0x0040  
#define WM8903_GP1_OP_CFG_SHIFT                      6  
#define WM8903_GP1_OP_CFG_WIDTH                      1  
#define WM8903_GP1_IP_CFG                       0x0020  
#define WM8903_GP1_IP_CFG_MASK                  0x0020  
#define WM8903_GP1_IP_CFG_SHIFT                      5  
#define WM8903_GP1_IP_CFG_WIDTH                      1  
#define WM8903_GP1_LVL                          0x0010  
#define WM8903_GP1_LVL_MASK                     0x0010  
#define WM8903_GP1_LVL_SHIFT                         4  
#define WM8903_GP1_LVL_WIDTH                         1  
#define WM8903_GP1_PD                           0x0008  
#define WM8903_GP1_PD_MASK                      0x0008  
#define WM8903_GP1_PD_SHIFT                          3  
#define WM8903_GP1_PD_WIDTH                          1  
#define WM8903_GP1_PU                           0x0004  
#define WM8903_GP1_PU_MASK                      0x0004  
#define WM8903_GP1_PU_SHIFT                          2  
#define WM8903_GP1_PU_WIDTH                          1  
#define WM8903_GP1_INTMODE                      0x0002  
#define WM8903_GP1_INTMODE_MASK                 0x0002  
#define WM8903_GP1_INTMODE_SHIFT                     1  
#define WM8903_GP1_INTMODE_WIDTH                     1  
#define WM8903_GP1_DB                           0x0001  
#define WM8903_GP1_DB_MASK                      0x0001  
#define WM8903_GP1_DB_SHIFT                          0  
#define WM8903_GP1_DB_WIDTH                          1  


#define WM8903_GP2_FN_MASK                      0x1F00  
#define WM8903_GP2_FN_SHIFT                          8  
#define WM8903_GP2_FN_WIDTH                          5  
#define WM8903_GP2_DIR                          0x0080  
#define WM8903_GP2_DIR_MASK                     0x0080  
#define WM8903_GP2_DIR_SHIFT                         7  
#define WM8903_GP2_DIR_WIDTH                         1  
#define WM8903_GP2_OP_CFG                       0x0040  
#define WM8903_GP2_OP_CFG_MASK                  0x0040  
#define WM8903_GP2_OP_CFG_SHIFT                      6  
#define WM8903_GP2_OP_CFG_WIDTH                      1  
#define WM8903_GP2_IP_CFG                       0x0020  
#define WM8903_GP2_IP_CFG_MASK                  0x0020  
#define WM8903_GP2_IP_CFG_SHIFT                      5  
#define WM8903_GP2_IP_CFG_WIDTH                      1  
#define WM8903_GP2_LVL                          0x0010  
#define WM8903_GP2_LVL_MASK                     0x0010  
#define WM8903_GP2_LVL_SHIFT                         4  
#define WM8903_GP2_LVL_WIDTH                         1  
#define WM8903_GP2_PD                           0x0008  
#define WM8903_GP2_PD_MASK                      0x0008  
#define WM8903_GP2_PD_SHIFT                          3  
#define WM8903_GP2_PD_WIDTH                          1  
#define WM8903_GP2_PU                           0x0004  
#define WM8903_GP2_PU_MASK                      0x0004  
#define WM8903_GP2_PU_SHIFT                          2  
#define WM8903_GP2_PU_WIDTH                          1  
#define WM8903_GP2_INTMODE                      0x0002  
#define WM8903_GP2_INTMODE_MASK                 0x0002  
#define WM8903_GP2_INTMODE_SHIFT                     1  
#define WM8903_GP2_INTMODE_WIDTH                     1  
#define WM8903_GP2_DB                           0x0001  
#define WM8903_GP2_DB_MASK                      0x0001  
#define WM8903_GP2_DB_SHIFT                          0  
#define WM8903_GP2_DB_WIDTH                          1  


#define WM8903_GP3_FN_MASK                      0x1F00  
#define WM8903_GP3_FN_SHIFT                          8  
#define WM8903_GP3_FN_WIDTH                          5  
#define WM8903_GP3_DIR                          0x0080  
#define WM8903_GP3_DIR_MASK                     0x0080  
#define WM8903_GP3_DIR_SHIFT                         7  
#define WM8903_GP3_DIR_WIDTH                         1  
#define WM8903_GP3_OP_CFG                       0x0040  
#define WM8903_GP3_OP_CFG_MASK                  0x0040  
#define WM8903_GP3_OP_CFG_SHIFT                      6  
#define WM8903_GP3_OP_CFG_WIDTH                      1  
#define WM8903_GP3_IP_CFG                       0x0020  
#define WM8903_GP3_IP_CFG_MASK                  0x0020  
#define WM8903_GP3_IP_CFG_SHIFT                      5  
#define WM8903_GP3_IP_CFG_WIDTH                      1  
#define WM8903_GP3_LVL                          0x0010  
#define WM8903_GP3_LVL_MASK                     0x0010  
#define WM8903_GP3_LVL_SHIFT                         4  
#define WM8903_GP3_LVL_WIDTH                         1  
#define WM8903_GP3_PD                           0x0008  
#define WM8903_GP3_PD_MASK                      0x0008  
#define WM8903_GP3_PD_SHIFT                          3  
#define WM8903_GP3_PD_WIDTH                          1  
#define WM8903_GP3_PU                           0x0004  
#define WM8903_GP3_PU_MASK                      0x0004  
#define WM8903_GP3_PU_SHIFT                          2  
#define WM8903_GP3_PU_WIDTH                          1  
#define WM8903_GP3_INTMODE                      0x0002  
#define WM8903_GP3_INTMODE_MASK                 0x0002  
#define WM8903_GP3_INTMODE_SHIFT                     1  
#define WM8903_GP3_INTMODE_WIDTH                     1  
#define WM8903_GP3_DB                           0x0001  
#define WM8903_GP3_DB_MASK                      0x0001  
#define WM8903_GP3_DB_SHIFT                          0  
#define WM8903_GP3_DB_WIDTH                          1  


#define WM8903_GP4_FN_MASK                      0x1F00  
#define WM8903_GP4_FN_SHIFT                          8  
#define WM8903_GP4_FN_WIDTH                          5  
#define WM8903_GP4_DIR                          0x0080  
#define WM8903_GP4_DIR_MASK                     0x0080  
#define WM8903_GP4_DIR_SHIFT                         7  
#define WM8903_GP4_DIR_WIDTH                         1  
#define WM8903_GP4_OP_CFG                       0x0040  
#define WM8903_GP4_OP_CFG_MASK                  0x0040  
#define WM8903_GP4_OP_CFG_SHIFT                      6  
#define WM8903_GP4_OP_CFG_WIDTH                      1  
#define WM8903_GP4_IP_CFG                       0x0020  
#define WM8903_GP4_IP_CFG_MASK                  0x0020  
#define WM8903_GP4_IP_CFG_SHIFT                      5  
#define WM8903_GP4_IP_CFG_WIDTH                      1  
#define WM8903_GP4_LVL                          0x0010  
#define WM8903_GP4_LVL_MASK                     0x0010  
#define WM8903_GP4_LVL_SHIFT                         4  
#define WM8903_GP4_LVL_WIDTH                         1  
#define WM8903_GP4_PD                           0x0008  
#define WM8903_GP4_PD_MASK                      0x0008  
#define WM8903_GP4_PD_SHIFT                          3  
#define WM8903_GP4_PD_WIDTH                          1  
#define WM8903_GP4_PU                           0x0004  
#define WM8903_GP4_PU_MASK                      0x0004  
#define WM8903_GP4_PU_SHIFT                          2  
#define WM8903_GP4_PU_WIDTH                          1  
#define WM8903_GP4_INTMODE                      0x0002  
#define WM8903_GP4_INTMODE_MASK                 0x0002  
#define WM8903_GP4_INTMODE_SHIFT                     1  
#define WM8903_GP4_INTMODE_WIDTH                     1  
#define WM8903_GP4_DB                           0x0001  
#define WM8903_GP4_DB_MASK                      0x0001  
#define WM8903_GP4_DB_SHIFT                          0  
#define WM8903_GP4_DB_WIDTH                          1  


#define WM8903_GP5_FN_MASK                      0x1F00  
#define WM8903_GP5_FN_SHIFT                          8  
#define WM8903_GP5_FN_WIDTH                          5  
#define WM8903_GP5_DIR                          0x0080  
#define WM8903_GP5_DIR_MASK                     0x0080  
#define WM8903_GP5_DIR_SHIFT                         7  
#define WM8903_GP5_DIR_WIDTH                         1  
#define WM8903_GP5_OP_CFG                       0x0040  
#define WM8903_GP5_OP_CFG_MASK                  0x0040  
#define WM8903_GP5_OP_CFG_SHIFT                      6  
#define WM8903_GP5_OP_CFG_WIDTH                      1  
#define WM8903_GP5_IP_CFG                       0x0020  
#define WM8903_GP5_IP_CFG_MASK                  0x0020  
#define WM8903_GP5_IP_CFG_SHIFT                      5  
#define WM8903_GP5_IP_CFG_WIDTH                      1  
#define WM8903_GP5_LVL                          0x0010  
#define WM8903_GP5_LVL_MASK                     0x0010  
#define WM8903_GP5_LVL_SHIFT                         4  
#define WM8903_GP5_LVL_WIDTH                         1  
#define WM8903_GP5_PD                           0x0008  
#define WM8903_GP5_PD_MASK                      0x0008  
#define WM8903_GP5_PD_SHIFT                          3  
#define WM8903_GP5_PD_WIDTH                          1  
#define WM8903_GP5_PU                           0x0004  
#define WM8903_GP5_PU_MASK                      0x0004  
#define WM8903_GP5_PU_SHIFT                          2  
#define WM8903_GP5_PU_WIDTH                          1  
#define WM8903_GP5_INTMODE                      0x0002  
#define WM8903_GP5_INTMODE_MASK                 0x0002  
#define WM8903_GP5_INTMODE_SHIFT                     1  
#define WM8903_GP5_INTMODE_WIDTH                     1  
#define WM8903_GP5_DB                           0x0001  
#define WM8903_GP5_DB_MASK                      0x0001  
#define WM8903_GP5_DB_SHIFT                          0  
#define WM8903_GP5_DB_WIDTH                          1  


#define WM8903_MICSHRT_EINT                     0x8000  
#define WM8903_MICSHRT_EINT_MASK                0x8000  
#define WM8903_MICSHRT_EINT_SHIFT                   15  
#define WM8903_MICSHRT_EINT_WIDTH                    1  
#define WM8903_MICDET_EINT                      0x4000  
#define WM8903_MICDET_EINT_MASK                 0x4000  
#define WM8903_MICDET_EINT_SHIFT                    14  
#define WM8903_MICDET_EINT_WIDTH                     1  
#define WM8903_WSEQ_BUSY_EINT                   0x2000  
#define WM8903_WSEQ_BUSY_EINT_MASK              0x2000  
#define WM8903_WSEQ_BUSY_EINT_SHIFT                 13  
#define WM8903_WSEQ_BUSY_EINT_WIDTH                  1  
#define WM8903_GP5_EINT                         0x0010  
#define WM8903_GP5_EINT_MASK                    0x0010  
#define WM8903_GP5_EINT_SHIFT                        4  
#define WM8903_GP5_EINT_WIDTH                        1  
#define WM8903_GP4_EINT                         0x0008  
#define WM8903_GP4_EINT_MASK                    0x0008  
#define WM8903_GP4_EINT_SHIFT                        3  
#define WM8903_GP4_EINT_WIDTH                        1  
#define WM8903_GP3_EINT                         0x0004  
#define WM8903_GP3_EINT_MASK                    0x0004  
#define WM8903_GP3_EINT_SHIFT                        2  
#define WM8903_GP3_EINT_WIDTH                        1  
#define WM8903_GP2_EINT                         0x0002  
#define WM8903_GP2_EINT_MASK                    0x0002  
#define WM8903_GP2_EINT_SHIFT                        1  
#define WM8903_GP2_EINT_WIDTH                        1  
#define WM8903_GP1_EINT                         0x0001  
#define WM8903_GP1_EINT_MASK                    0x0001  
#define WM8903_GP1_EINT_SHIFT                        0  
#define WM8903_GP1_EINT_WIDTH                        1  


#define WM8903_IM_MICSHRT_EINT                  0x8000  
#define WM8903_IM_MICSHRT_EINT_MASK             0x8000  
#define WM8903_IM_MICSHRT_EINT_SHIFT                15  
#define WM8903_IM_MICSHRT_EINT_WIDTH                 1  
#define WM8903_IM_MICDET_EINT                   0x4000  
#define WM8903_IM_MICDET_EINT_MASK              0x4000  
#define WM8903_IM_MICDET_EINT_SHIFT                 14  
#define WM8903_IM_MICDET_EINT_WIDTH                  1  
#define WM8903_IM_WSEQ_BUSY_EINT                0x2000  
#define WM8903_IM_WSEQ_BUSY_EINT_MASK           0x2000  
#define WM8903_IM_WSEQ_BUSY_EINT_SHIFT              13  
#define WM8903_IM_WSEQ_BUSY_EINT_WIDTH               1  
#define WM8903_IM_GP5_EINT                      0x0010  
#define WM8903_IM_GP5_EINT_MASK                 0x0010  
#define WM8903_IM_GP5_EINT_SHIFT                     4  
#define WM8903_IM_GP5_EINT_WIDTH                     1  
#define WM8903_IM_GP4_EINT                      0x0008  
#define WM8903_IM_GP4_EINT_MASK                 0x0008  
#define WM8903_IM_GP4_EINT_SHIFT                     3  
#define WM8903_IM_GP4_EINT_WIDTH                     1  
#define WM8903_IM_GP3_EINT                      0x0004  
#define WM8903_IM_GP3_EINT_MASK                 0x0004  
#define WM8903_IM_GP3_EINT_SHIFT                     2  
#define WM8903_IM_GP3_EINT_WIDTH                     1  
#define WM8903_IM_GP2_EINT                      0x0002  
#define WM8903_IM_GP2_EINT_MASK                 0x0002  
#define WM8903_IM_GP2_EINT_SHIFT                     1  
#define WM8903_IM_GP2_EINT_WIDTH                     1  
#define WM8903_IM_GP1_EINT                      0x0001  
#define WM8903_IM_GP1_EINT_MASK                 0x0001  
#define WM8903_IM_GP1_EINT_SHIFT                     0  
#define WM8903_IM_GP1_EINT_WIDTH                     1  


#define WM8903_MICSHRT_INV                      0x8000  
#define WM8903_MICSHRT_INV_MASK                 0x8000  
#define WM8903_MICSHRT_INV_SHIFT                    15  
#define WM8903_MICSHRT_INV_WIDTH                     1  
#define WM8903_MICDET_INV                       0x4000  
#define WM8903_MICDET_INV_MASK                  0x4000  
#define WM8903_MICDET_INV_SHIFT                     14  
#define WM8903_MICDET_INV_WIDTH                      1  


#define WM8903_IRQ_POL                          0x0001  
#define WM8903_IRQ_POL_MASK                     0x0001  
#define WM8903_IRQ_POL_SHIFT                         0  
#define WM8903_IRQ_POL_WIDTH                         1  


#define WM8903_USER_KEY                         0x0002  
#define WM8903_USER_KEY_MASK                    0x0002  
#define WM8903_USER_KEY_SHIFT                        1  
#define WM8903_USER_KEY_WIDTH                        1  
#define WM8903_TEST_KEY                         0x0001  
#define WM8903_TEST_KEY_MASK                    0x0001  
#define WM8903_TEST_KEY_SHIFT                        0  
#define WM8903_TEST_KEY_WIDTH                        1  


#define WM8903_CP_SW_KELVIN_MODE_MASK           0x0006  
#define WM8903_CP_SW_KELVIN_MODE_SHIFT               1  
#define WM8903_CP_SW_KELVIN_MODE_WIDTH               2  


#define WM8903_ADC_DIG_MIC                      0x0200  
#define WM8903_ADC_DIG_MIC_MASK                 0x0200  
#define WM8903_ADC_DIG_MIC_SHIFT                     9  
#define WM8903_ADC_DIG_MIC_WIDTH                     1  


#define WM8903_PGA_BIAS_MASK                    0x0070  
#define WM8903_PGA_BIAS_SHIFT                        4  
#define WM8903_PGA_BIAS_WIDTH                        3  

#endif
