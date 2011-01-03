

#ifndef __EICON_MDM_MSG_H__
#define __EICON_MDM_MSG_H__
#define DSP_UDATA_INDICATION_DCD_OFF  0x01
#define DSP_UDATA_INDICATION_DCD_ON  0x02
#define DSP_UDATA_INDICATION_CTS_OFF  0x03
#define DSP_UDATA_INDICATION_CTS_ON  0x04

#define DSP_CONNECTED_NORM_UNSPECIFIED      0
#define DSP_CONNECTED_NORM_V21              1
#define DSP_CONNECTED_NORM_V23              2
#define DSP_CONNECTED_NORM_V22              3
#define DSP_CONNECTED_NORM_V22_BIS          4
#define DSP_CONNECTED_NORM_V32_BIS          5
#define DSP_CONNECTED_NORM_V34              6
#define DSP_CONNECTED_NORM_V8               7
#define DSP_CONNECTED_NORM_BELL_212A        8
#define DSP_CONNECTED_NORM_BELL_103         9
#define DSP_CONNECTED_NORM_V29_LEASED_LINE  10
#define DSP_CONNECTED_NORM_V33_LEASED_LINE  11
#define DSP_CONNECTED_NORM_V90              12
#define DSP_CONNECTED_NORM_V21_CH2          13
#define DSP_CONNECTED_NORM_V27_TER          14
#define DSP_CONNECTED_NORM_V29              15
#define DSP_CONNECTED_NORM_V33              16
#define DSP_CONNECTED_NORM_V17              17
#define DSP_CONNECTED_NORM_V32              18
#define DSP_CONNECTED_NORM_K56_FLEX         19
#define DSP_CONNECTED_NORM_X2               20
#define DSP_CONNECTED_NORM_V18              21
#define DSP_CONNECTED_NORM_V18_LOW_HIGH     22
#define DSP_CONNECTED_NORM_V18_HIGH_LOW     23
#define DSP_CONNECTED_NORM_V21_LOW_HIGH     24
#define DSP_CONNECTED_NORM_V21_HIGH_LOW     25
#define DSP_CONNECTED_NORM_BELL103_LOW_HIGH 26
#define DSP_CONNECTED_NORM_BELL103_HIGH_LOW 27
#define DSP_CONNECTED_NORM_V23_75_1200      28
#define DSP_CONNECTED_NORM_V23_1200_75      29
#define DSP_CONNECTED_NORM_EDT_110          30
#define DSP_CONNECTED_NORM_BAUDOT_45        31
#define DSP_CONNECTED_NORM_BAUDOT_47        32
#define DSP_CONNECTED_NORM_BAUDOT_50        33
#define DSP_CONNECTED_NORM_DTMF             34
#define DSP_CONNECTED_NORM_V18_RESERVED_13  35
#define DSP_CONNECTED_NORM_V18_RESERVED_14  36
#define DSP_CONNECTED_NORM_V18_RESERVED_15  37
#define DSP_CONNECTED_NORM_VOWN             38
#define DSP_CONNECTED_NORM_V23_OFF_HOOK     39
#define DSP_CONNECTED_NORM_V23_ON_HOOK      40
#define DSP_CONNECTED_NORM_VOWN_RESERVED_3  41
#define DSP_CONNECTED_NORM_VOWN_RESERVED_4  42
#define DSP_CONNECTED_NORM_VOWN_RESERVED_5  43
#define DSP_CONNECTED_NORM_VOWN_RESERVED_6  44
#define DSP_CONNECTED_NORM_VOWN_RESERVED_7  45
#define DSP_CONNECTED_NORM_VOWN_RESERVED_8  46
#define DSP_CONNECTED_NORM_VOWN_RESERVED_9  47
#define DSP_CONNECTED_NORM_VOWN_RESERVED_10 48
#define DSP_CONNECTED_NORM_VOWN_RESERVED_11 49
#define DSP_CONNECTED_NORM_VOWN_RESERVED_12 50
#define DSP_CONNECTED_NORM_VOWN_RESERVED_13 51
#define DSP_CONNECTED_NORM_VOWN_RESERVED_14 52
#define DSP_CONNECTED_NORM_VOWN_RESERVED_15 53
#define DSP_CONNECTED_NORM_VOWN_RESERVED_16 54
#define DSP_CONNECTED_NORM_VOWN_RESERVED_17 55
#define DSP_CONNECTED_NORM_VOWN_RESERVED_18 56
#define DSP_CONNECTED_NORM_VOWN_RESERVED_19 57
#define DSP_CONNECTED_NORM_VOWN_RESERVED_20 58
#define DSP_CONNECTED_NORM_VOWN_RESERVED_21 59
#define DSP_CONNECTED_NORM_VOWN_RESERVED_22 60
#define DSP_CONNECTED_NORM_VOWN_RESERVED_23 61
#define DSP_CONNECTED_NORM_VOWN_RESERVED_24 62
#define DSP_CONNECTED_NORM_VOWN_RESERVED_25 63
#define DSP_CONNECTED_NORM_VOWN_RESERVED_26 64
#define DSP_CONNECTED_NORM_VOWN_RESERVED_27 65
#define DSP_CONNECTED_NORM_VOWN_RESERVED_28 66
#define DSP_CONNECTED_NORM_VOWN_RESERVED_29 67
#define DSP_CONNECTED_NORM_VOWN_RESERVED_30 68
#define DSP_CONNECTED_NORM_VOWN_RESERVED_31 69
#define DSP_CONNECTED_OPTION_TRELLIS             0x0001
#define DSP_CONNECTED_OPTION_V42_TRANS           0x0002
#define DSP_CONNECTED_OPTION_V42_LAPM            0x0004
#define DSP_CONNECTED_OPTION_SHORT_TRAIN         0x0008
#define DSP_CONNECTED_OPTION_TALKER_ECHO_PROTECT 0x0010
#define DSP_CONNECTED_OPTION_V42BIS              0x0020
#define DSP_CONNECTED_OPTION_MNP2                0x0040
#define DSP_CONNECTED_OPTION_MNP3                0x0080
#define DSP_CONNECTED_OPTION_MNP4                0x00c0
#define DSP_CONNECTED_OPTION_MNP5                0x0100
#define DSP_CONNECTED_OPTION_MNP10               0x0200
#define DSP_CONNECTED_OPTION_MASK_V42            0x0024
#define DSP_CONNECTED_OPTION_MASK_MNP            0x03c0
#define DSP_CONNECTED_OPTION_MASK_ERROR_CORRECT  0x03e4
#define DSP_CONNECTED_OPTION_MASK_COMPRESSION    0x0320
#define DSP_UDATA_INDICATION_DISCONNECT         5



#define DLC_MODEMPROT_DISABLE_V42_V42BIS     0x01
#define DLC_MODEMPROT_DISABLE_MNP_MNP5       0x02
#define DLC_MODEMPROT_REQUIRE_PROTOCOL       0x04
#define DLC_MODEMPROT_DISABLE_V42_DETECT     0x08
#define DLC_MODEMPROT_DISABLE_COMPRESSION    0x10
#define DLC_MODEMPROT_REQUIRE_PROTOCOL_V34UP 0x20
#define DLC_MODEMPROT_NO_PROTOCOL_IF_1200    0x01
#define DLC_MODEMPROT_BUFFER_IN_V42_DETECT   0x02
#define DLC_MODEMPROT_DISABLE_V42_SREJ       0x04
#define DLC_MODEMPROT_DISABLE_MNP3           0x08
#define DLC_MODEMPROT_DISABLE_MNP4           0x10
#define DLC_MODEMPROT_DISABLE_MNP10          0x20
#define DLC_MODEMPROT_NO_PROTOCOL_IF_V22BIS  0x40
#define DLC_MODEMPROT_NO_PROTOCOL_IF_V32BIS  0x80
#define DLC_MODEMPROT_BREAK_DISABLED         0x00
#define DLC_MODEMPROT_BREAK_NORMAL           0x01
#define DLC_MODEMPROT_BREAK_EXPEDITED        0x02
#define DLC_MODEMPROT_BREAK_DESTRUCTIVE      0x03
#define DLC_MODEMPROT_BREAK_CONFIG_MASK      0x03
#define DLC_MODEMPROT_APPL_EARLY_CONNECT     0x01
#define DLC_MODEMPROT_APPL_PASS_INDICATIONS  0x02


#define DSP_CAI_HARDWARE_HDLC_64K          0x05
#define DSP_CAI_HARDWARE_HDLC_56K          0x08
#define DSP_CAI_HARDWARE_TRANSP            0x09
#define DSP_CAI_HARDWARE_V110_SYNC         0x0c
#define DSP_CAI_HARDWARE_V110_ASYNC        0x0d
#define DSP_CAI_HARDWARE_HDLC_128K         0x0f
#define DSP_CAI_HARDWARE_FAX               0x10
#define DSP_CAI_HARDWARE_MODEM_ASYNC       0x11
#define DSP_CAI_HARDWARE_MODEM_SYNC        0x12
#define DSP_CAI_HARDWARE_V110_HDLCA        0x13
#define DSP_CAI_HARDWARE_ADVANCED_VOICE    0x14
#define DSP_CAI_HARDWARE_TRANSP_DTMF       0x16
#define DSP_CAI_HARDWARE_DTMF_VOICE_ISDN   0x17
#define DSP_CAI_HARDWARE_DTMF_VOICE_LOCAL  0x18
#define DSP_CAI_HARDWARE_MASK              0x3f
#define DSP_CAI_ENABLE_INFO_INDICATIONS    0x80
#define DSP_CAI_RATE_ADAPTATION_300        0x00
#define DSP_CAI_RATE_ADAPTATION_600        0x01
#define DSP_CAI_RATE_ADAPTATION_1200       0x02
#define DSP_CAI_RATE_ADAPTATION_2400       0x03
#define DSP_CAI_RATE_ADAPTATION_4800       0x04
#define DSP_CAI_RATE_ADAPTATION_9600       0x05
#define DSP_CAI_RATE_ADAPTATION_19200      0x06
#define DSP_CAI_RATE_ADAPTATION_38400      0x07
#define DSP_CAI_RATE_ADAPTATION_48000      0x08
#define DSP_CAI_RATE_ADAPTATION_56000      0x09
#define DSP_CAI_RATE_ADAPTATION_7200       0x0a
#define DSP_CAI_RATE_ADAPTATION_14400      0x0b
#define DSP_CAI_RATE_ADAPTATION_28800      0x0c
#define DSP_CAI_RATE_ADAPTATION_12000      0x0d
#define DSP_CAI_RATE_ADAPTATION_1200_75    0x0e
#define DSP_CAI_RATE_ADAPTATION_75_1200    0x0f
#define DSP_CAI_RATE_ADAPTATION_MASK       0x0f
#define DSP_CAI_ASYNC_PARITY_ENABLE        0x01
#define DSP_CAI_ASYNC_PARITY_SPACE         0x00
#define DSP_CAI_ASYNC_PARITY_ODD           0x02
#define DSP_CAI_ASYNC_PARITY_EVEN          0x04
#define DSP_CAI_ASYNC_PARITY_MARK          0x06
#define DSP_CAI_ASYNC_PARITY_MASK          0x06
#define DSP_CAI_ASYNC_ONE_STOP_BIT         0x00
#define DSP_CAI_ASYNC_TWO_STOP_BITS        0x20
#define DSP_CAI_ASYNC_CHAR_LENGTH_8        0x00
#define DSP_CAI_ASYNC_CHAR_LENGTH_7        0x40
#define DSP_CAI_ASYNC_CHAR_LENGTH_6        0x80
#define DSP_CAI_ASYNC_CHAR_LENGTH_5        0xc0
#define DSP_CAI_ASYNC_CHAR_LENGTH_MASK     0xc0
#define DSP_CAI_MODEM_LEASED_LINE_MODE     0x01
#define DSP_CAI_MODEM_4_WIRE_OPERATION     0x02
#define DSP_CAI_MODEM_DISABLE_BUSY_DETECT  0x04
#define DSP_CAI_MODEM_DISABLE_CALLING_TONE 0x08
#define DSP_CAI_MODEM_DISABLE_ANSWER_TONE  0x10
#define DSP_CAI_MODEM_ENABLE_DIAL_TONE_DET 0x20
#define DSP_CAI_MODEM_USE_POTS_INTERFACE   0x40
#define DSP_CAI_MODEM_FORCE_RAY_TAYLOR_FAX 0x80
#define DSP_CAI_MODEM_NEGOTIATE_HIGHEST    0x00
#define DSP_CAI_MODEM_NEGOTIATE_DISABLED   0x01
#define DSP_CAI_MODEM_NEGOTIATE_IN_CLASS   0x02
#define DSP_CAI_MODEM_NEGOTIATE_V100       0x03
#define DSP_CAI_MODEM_NEGOTIATE_V8         0x04
#define DSP_CAI_MODEM_NEGOTIATE_V8BIS      0x05
#define DSP_CAI_MODEM_NEGOTIATE_MASK       0x07
#define DSP_CAI_MODEM_GUARD_TONE_NONE      0x00
#define DSP_CAI_MODEM_GUARD_TONE_550HZ     0x40
#define DSP_CAI_MODEM_GUARD_TONE_1800HZ    0x80
#define DSP_CAI_MODEM_GUARD_TONE_MASK      0xc0
#define DSP_CAI_MODEM_DISABLE_RETRAIN      0x01
#define DSP_CAI_MODEM_DISABLE_STEPUPDOWN   0x02
#define DSP_CAI_MODEM_DISABLE_SPLIT_SPEED  0x04
#define DSP_CAI_MODEM_DISABLE_TRELLIS      0x08
#define DSP_CAI_MODEM_ALLOW_RDL_TEST_LOOP  0x10
#define DSP_CAI_MODEM_DISABLE_FLUSH_TIMER  0x40
#define DSP_CAI_MODEM_REVERSE_DIRECTION    0x80
#define DSP_CAI_MODEM_DISABLE_V21          0x01
#define DSP_CAI_MODEM_DISABLE_V23          0x02
#define DSP_CAI_MODEM_DISABLE_V22          0x04
#define DSP_CAI_MODEM_DISABLE_V22BIS       0x08
#define DSP_CAI_MODEM_DISABLE_V32          0x10
#define DSP_CAI_MODEM_DISABLE_V32BIS       0x20
#define DSP_CAI_MODEM_DISABLE_V34          0x40
#define DSP_CAI_MODEM_DISABLE_V90          0x80
#define DSP_CAI_MODEM_DISABLE_BELL103      0x01
#define DSP_CAI_MODEM_DISABLE_BELL212A     0x02
#define DSP_CAI_MODEM_DISABLE_VFC          0x04
#define DSP_CAI_MODEM_DISABLE_K56FLEX      0x08
#define DSP_CAI_MODEM_DISABLE_X2           0x10
#define DSP_CAI_MODEM_ENABLE_V29FDX        0x01
#define DSP_CAI_MODEM_ENABLE_V33           0x02
#define DSP_CAI_MODEM_DISABLE_2400_SYMBOLS 0x01
#define DSP_CAI_MODEM_DISABLE_2743_SYMBOLS 0x02
#define DSP_CAI_MODEM_DISABLE_2800_SYMBOLS 0x04
#define DSP_CAI_MODEM_DISABLE_3000_SYMBOLS 0x08
#define DSP_CAI_MODEM_DISABLE_3200_SYMBOLS 0x10
#define DSP_CAI_MODEM_DISABLE_3429_SYMBOLS 0x20
#define DSP_CAI_MODEM_DISABLE_TX_REDUCTION 0x01
#define DSP_CAI_MODEM_DISABLE_PRECODING    0x02
#define DSP_CAI_MODEM_DISABLE_PREEMPHASIS  0x04
#define DSP_CAI_MODEM_DISABLE_SHAPING      0x08
#define DSP_CAI_MODEM_DISABLE_NONLINEAR_EN 0x10
#define DSP_CAI_MODEM_SPEAKER_OFF          0x00
#define DSP_CAI_MODEM_SPEAKER_DURING_TRAIN 0x01
#define DSP_CAI_MODEM_SPEAKER_TIL_CONNECT  0x02
#define DSP_CAI_MODEM_SPEAKER_ALWAYS_ON    0x03
#define DSP_CAI_MODEM_SPEAKER_CONTROL_MASK 0x03
#define DSP_CAI_MODEM_SPEAKER_VOLUME_MIN   0x00
#define DSP_CAI_MODEM_SPEAKER_VOLUME_LOW   0x04
#define DSP_CAI_MODEM_SPEAKER_VOLUME_HIGH  0x08
#define DSP_CAI_MODEM_SPEAKER_VOLUME_MAX   0x0c
#define DSP_CAI_MODEM_SPEAKER_VOLUME_MASK  0x0c

#define MDM_WANT_CONNECT_B3_ACTIVE_I  0x01
#define MDM_NCPI_VALID                0x02
#define MDM_NCPI_CTS_ON_RECEIVED      0x04
#define MDM_NCPI_DCD_ON_RECEIVED      0x08

#define MDM_NCPI_ECM_V42              0x0001
#define MDM_NCPI_ECM_MNP              0x0002
#define MDM_NCPI_TRANSPARENT          0x0004
#define MDM_NCPI_COMPRESSED           0x0010

#define MDM_B2_DISABLE_V42bis         0x0001
#define MDM_B2_DISABLE_MNP            0x0002
#define MDM_B2_DISABLE_TRANS          0x0004
#define MDM_B2_DISABLE_V42            0x0008
#define MDM_B2_DISABLE_COMP           0x0010

#define MDM_CAPI_DISABLE_RETRAIN      0x0001
#define MDM_CAPI_DISABLE_RING_TONE    0x0002
#define MDM_CAPI_GUARD_1800           0x0004
#define MDM_CAPI_GUARD_550            0x0008
#define MDM_CAPI_NEG_V8               0x0003
#define MDM_CAPI_NEG_V100             0x0002
#define MDM_CAPI_NEG_MOD_CLASS        0x0001
#define MDM_CAPI_NEG_DISABLED         0x0000
#endif
