

#ifndef _FALC_LH_H
#define _FALC_LH_H

#define NUM_OF_T1_CHANNELS	24
#define NUM_OF_E1_CHANNELS	32




#define CMDR_RMC	0x80
#define CMDR_RRES	0x40
#define CMDR_XREP	0x20
#define CMDR_XRES	0x10
#define CMDR_XHF	0x08
#define CMDR_XTF	0x04
#define CMDR_XME	0x02
#define CMDR_SRES	0x01


#define MODE_MDS2	0x80
#define MODE_MDS1	0x40
#define MODE_MDS0	0x20
#define MODE_BRAC	0x10
#define MODE_HRAC	0x08


#define IPC_VIS		0x80
#define IPC_SCI		0x04
#define IPC_IC1		0x02
#define IPC_IC0		0x01


#define CCR1_SFLG       0x80
#define CCR1_XTS16RA    0x40
#define CCR1_BRM        0x40
#define CCR1_CASSYM     0x20
#define CCR1_EDLX       0x20
#define CCR1_EITS       0x10
#define CCR1_ITF        0x08
#define CCR1_RFT1       0x02
#define CCR1_RFT0       0x01



#define CCR3_PRE1       0x80
#define CCR3_PRE0       0x40
#define CCR3_EPT        0x20
#define CCR3_RADD       0x10
#define CCR3_RCRC       0x04
#define CCR3_XCRC       0x02




#define RTR1_TS0        0x80
#define RTR1_TS1        0x40
#define RTR1_TS2        0x20
#define RTR1_TS3        0x10
#define RTR1_TS4        0x08
#define RTR1_TS5        0x04
#define RTR1_TS6        0x02
#define RTR1_TS7        0x01

#define RTR2_TS8        0x80
#define RTR2_TS9        0x40
#define RTR2_TS10       0x20
#define RTR2_TS11       0x10
#define RTR2_TS12       0x08
#define RTR2_TS13       0x04
#define RTR2_TS14       0x02
#define RTR2_TS15       0x01

#define RTR3_TS16       0x80
#define RTR3_TS17       0x40
#define RTR3_TS18       0x20
#define RTR3_TS19       0x10
#define RTR3_TS20       0x08
#define RTR3_TS21       0x04
#define RTR3_TS22       0x02
#define RTR3_TS23       0x01

#define RTR4_TS24       0x80
#define RTR4_TS25       0x40
#define RTR4_TS26       0x20
#define RTR4_TS27       0x10
#define RTR4_TS28       0x08
#define RTR4_TS29       0x04
#define RTR4_TS30       0x02
#define RTR4_TS31       0x01




#define TTR1_TS0        0x80
#define TTR1_TS1        0x40
#define TTR1_TS2        0x20
#define TTR1_TS3        0x10
#define TTR1_TS4        0x08
#define TTR1_TS5        0x04
#define TTR1_TS6        0x02
#define TTR1_TS7        0x01

#define TTR2_TS8        0x80
#define TTR2_TS9        0x40
#define TTR2_TS10       0x20
#define TTR2_TS11       0x10
#define TTR2_TS12       0x08
#define TTR2_TS13       0x04
#define TTR2_TS14       0x02
#define TTR2_TS15       0x01

#define TTR3_TS16       0x80
#define TTR3_TS17       0x40
#define TTR3_TS18       0x20
#define TTR3_TS19       0x10
#define TTR3_TS20       0x08
#define TTR3_TS21       0x04
#define TTR3_TS22       0x02
#define TTR3_TS23       0x01

#define TTR4_TS24       0x80
#define TTR4_TS25       0x40
#define TTR4_TS26       0x20
#define TTR4_TS27       0x10
#define TTR4_TS28       0x08
#define TTR4_TS29       0x04
#define TTR4_TS30       0x02
#define TTR4_TS31       0x01





#define IMR0_RME        0x80
#define IMR0_RFS        0x40
#define IMR0_T8MS       0x20
#define IMR0_ISF        0x20
#define IMR0_RMB        0x10
#define IMR0_CASC       0x08
#define IMR0_RSC        0x08
#define IMR0_CRC6       0x04
#define IMR0_CRC4       0x04
#define IMR0_PDEN	0x02
#define IMR0_RPF        0x01

#define IMR1_CASE       0x80
#define IMR1_RDO        0x40
#define IMR1_ALLS       0x20
#define IMR1_XDU        0x10
#define IMR1_XMB        0x08
#define IMR1_XLSC       0x02
#define IMR1_XPR        0x01
#define IMR1_LLBSC	0x80

#define IMR2_FAR        0x80
#define IMR2_LFA        0x40
#define IMR2_MFAR       0x20
#define IMR2_T400MS     0x10
#define IMR2_LMFA       0x10
#define IMR2_AIS        0x08
#define IMR2_LOS        0x04
#define IMR2_RAR        0x02
#define IMR2_RA         0x01

#define IMR3_ES         0x80
#define IMR3_SEC        0x40
#define IMR3_LMFA16     0x20
#define IMR3_AIS16      0x10
#define IMR3_RA16       0x08
#define IMR3_API        0x04
#define IMR3_XSLP       0x20
#define IMR3_XSLN       0x10
#define IMR3_LLBSC      0x08
#define IMR3_XRS        0x04
#define IMR3_SLN        0x02
#define IMR3_SLP        0x01

#define IMR4_LFA        0x80
#define IMR4_FER        0x40
#define IMR4_CER        0x20
#define IMR4_AIS        0x10
#define IMR4_LOS        0x08
#define IMR4_CVE        0x04
#define IMR4_SLIP       0x02
#define IMR4_EBE        0x01



#define FMR0_XC1        0x80
#define FMR0_XC0        0x40
#define FMR0_RC1        0x20
#define FMR0_RC0        0x10
#define FMR0_EXTD       0x08
#define FMR0_ALM        0x04
#define E1_FMR0_FRS     0x02
#define T1_FMR0_FRS     0x08
#define FMR0_SRAF       0x04
#define FMR0_EXLS       0x02
#define FMR0_SIM        0x01

#define FMR1_MFCS       0x80
#define FMR1_AFR        0x40
#define FMR1_ENSA       0x20
#define FMR1_CTM        0x80
#define FMR1_SIGM       0x40
#define FMR1_EDL        0x20
#define FMR1_PMOD       0x10
#define FMR1_XFS        0x08
#define FMR1_CRC        0x08
#define FMR1_ECM        0x04
#define FMR1_IMOD       0x02
#define FMR1_XAIS       0x01

#define FMR2_RFS1       0x80
#define FMR2_RFS0       0x40
#define FMR2_MCSP	0x40
#define FMR2_RTM        0x20
#define FMR2_SSP        0x20
#define FMR2_DAIS       0x10
#define FMR2_SAIS       0x08
#define FMR2_PLB        0x04
#define FMR2_AXRA       0x02
#define FMR2_ALMF       0x01
#define FMR2_EXZE       0x01

#define LOOP_RTM	0x40
#define LOOP_SFM	0x40
#define LOOP_ECLB	0x20
#define LOOP_CLA	0x1f


#define FMR3_XLD	0x20
#define FMR3_XLU	0x10


#define FMR4_AIS3       0x80
#define FMR4_TM         0x40
#define FMR4_XRA        0x20
#define FMR4_SSC1       0x10
#define FMR4_SSC0       0x08
#define FMR4_AUTO       0x04
#define FMR4_FM1        0x02
#define FMR4_FM0        0x01

#define FMR5_SRS        0x80
#define FMR5_EIBR       0x40
#define FMR5_XLD        0x20
#define FMR5_XLU        0x10




#define LOOP_SFM        0x40
#define LOOP_ECLB       0x20
#define LOOP_CLA4       0x10
#define LOOP_CLA3       0x08
#define LOOP_CLA2       0x04
#define LOOP_CLA1       0x02
#define LOOP_CLA0       0x01





#define XSW_XSIS        0x80
#define XSW_XTM         0x40
#define XSW_XRA         0x20
#define XSW_XY0         0x10
#define XSW_XY1         0x08
#define XSW_XY2         0x04
#define XSW_XY3         0x02
#define XSW_XY4         0x01




#define XSP_XAP         0x80
#define XSP_CASEN       0x40
#define XSP_TT0         0x20
#define XSP_EBP         0x10
#define XSP_AXS         0x08
#define XSP_XSIF        0x04
#define XSP_XS13        0x02
#define XSP_XS15        0x01




#define XC0_SA8E        0x80
#define XC0_SA7E        0x40
#define XC0_SA6E        0x20
#define XC0_SA5E        0x10
#define XC0_SA4E        0x08
#define XC0_BRM         0x80
#define XC0_MFBS        0x40
#define XC0_SFRZ        0x10
#define XC0_XCO2        0x04
#define XC0_XCO1        0x02
#define XC0_XCO0        0x01

#define XC1_XTO5        0x20
#define XC1_XTO4        0x10
#define XC1_XTO3        0x08
#define XC1_XTO2        0x04
#define XC1_XTO1        0x02
#define XC1_XTO0        0x01




#define RC0_SICS        0x40
#define RC0_CRCI        0x20
#define RC0_XCRCI       0x10
#define RC0_RDIS        0x08
#define RC0_RCO2        0x04
#define RC0_RCO1        0x02
#define RC0_RCO0        0x01

#define RC1_SWD         0x80
#define RC1_ASY4        0x40
#define RC1_RRAM        0x40
#define RC1_RTO5        0x20
#define RC1_RTO4        0x10
#define RC1_RTO3        0x08
#define RC1_RTO2        0x04
#define RC1_RTO1        0x02
#define RC1_RTO0        0x01





#define XPM0_XP12       0x80
#define XPM0_XP11       0x40
#define XPM0_XP10       0x20
#define XPM0_XP04       0x10
#define XPM0_XP03       0x08
#define XPM0_XP02       0x04
#define XPM0_XP01       0x02
#define XPM0_XP00       0x01

#define XPM1_XP30       0x80
#define XPM1_XP24       0x40
#define XPM1_XP23       0x20
#define XPM1_XP22       0x10
#define XPM1_XP21       0x08
#define XPM1_XP20       0x04
#define XPM1_XP14       0x02
#define XPM1_XP13       0x01

#define XPM2_XLHP       0x80
#define XPM2_XLT        0x40
#define XPM2_DAXLT      0x20
#define XPM2_XP34       0x08
#define XPM2_XP33       0x04
#define XPM2_XP32       0x02
#define XPM2_XP31       0x01




#define TSWM_TSIS       0x80
#define TSWM_TSIF       0x40
#define TSWM_TRA        0x20
#define TSWM_TSA4       0x10
#define TSWM_TSA5       0x08
#define TSWM_TSA6       0x04
#define TSWM_TSA7       0x02
#define TSWM_TSA8       0x01



#define IDLE_IDL7       0x80
#define IDLE_IDL6       0x40
#define IDLE_IDL5       0x20
#define IDLE_IDL4       0x10
#define IDLE_IDL3       0x08
#define IDLE_IDL2       0x04
#define IDLE_IDL1       0x02
#define IDLE_IDL0       0x01




#define XSA4_XS47       0x80
#define XSA4_XS46       0x40
#define XSA4_XS45       0x20
#define XSA4_XS44       0x10
#define XSA4_XS43       0x08
#define XSA4_XS42       0x04
#define XSA4_XS41       0x02
#define XSA4_XS40       0x01

#define XSA5_XS57       0x80
#define XSA5_XS56       0x40
#define XSA5_XS55       0x20
#define XSA5_XS54       0x10
#define XSA5_XS53       0x08
#define XSA5_XS52       0x04
#define XSA5_XS51       0x02
#define XSA5_XS50       0x01

#define XSA6_XS67       0x80
#define XSA6_XS66       0x40
#define XSA6_XS65       0x20
#define XSA6_XS64       0x10
#define XSA6_XS63       0x08
#define XSA6_XS62       0x04
#define XSA6_XS61       0x02
#define XSA6_XS60       0x01

#define XSA7_XS77       0x80
#define XSA7_XS76       0x40
#define XSA7_XS75       0x20
#define XSA7_XS74       0x10
#define XSA7_XS73       0x08
#define XSA7_XS72       0x04
#define XSA7_XS71       0x02
#define XSA7_XS70       0x01

#define XSA8_XS87       0x80
#define XSA8_XS86       0x40
#define XSA8_XS85       0x20
#define XSA8_XS84       0x10
#define XSA8_XS83       0x08
#define XSA8_XS82       0x04
#define XSA8_XS81       0x02
#define XSA8_XS80       0x01




#define XDL1_XDL17      0x80
#define XDL1_XDL16      0x40
#define XDL1_XDL15      0x20
#define XDL1_XDL14      0x10
#define XDL1_XDL13      0x08
#define XDL1_XDL12      0x04
#define XDL1_XDL11      0x02
#define XDL1_XDL10      0x01

#define XDL2_XDL27      0x80
#define XDL2_XDL26      0x40
#define XDL2_XDL25      0x20
#define XDL2_XDL24      0x10
#define XDL2_XDL23      0x08
#define XDL2_XDL22      0x04
#define XDL2_XDL21      0x02
#define XDL2_XDL20      0x01

#define XDL3_XDL37      0x80
#define XDL3_XDL36      0x40
#define XDL3_XDL35      0x20
#define XDL3_XDL34      0x10
#define XDL3_XDL33      0x08
#define XDL3_XDL32      0x04
#define XDL3_XDL31      0x02
#define XDL3_XDL30      0x01




#define E1_ICB1_IC0	0x80
#define E1_ICB1_IC1	0x40
#define E1_ICB1_IC2	0x20
#define E1_ICB1_IC3	0x10
#define E1_ICB1_IC4	0x08
#define E1_ICB1_IC5	0x04
#define E1_ICB1_IC6	0x02
#define E1_ICB1_IC7	0x01

#define E1_ICB2_IC8	0x80
#define E1_ICB2_IC9	0x40
#define E1_ICB2_IC10	0x20
#define E1_ICB2_IC11	0x10
#define E1_ICB2_IC12	0x08
#define E1_ICB2_IC13	0x04
#define E1_ICB2_IC14	0x02
#define E1_ICB2_IC15	0x01

#define E1_ICB3_IC16	0x80
#define E1_ICB3_IC17	0x40
#define E1_ICB3_IC18	0x20
#define E1_ICB3_IC19	0x10
#define E1_ICB3_IC20	0x08
#define E1_ICB3_IC21	0x04
#define E1_ICB3_IC22	0x02
#define E1_ICB3_IC23	0x01

#define E1_ICB4_IC24	0x80
#define E1_ICB4_IC25	0x40
#define E1_ICB4_IC26	0x20
#define E1_ICB4_IC27	0x10
#define E1_ICB4_IC28	0x08
#define E1_ICB4_IC29	0x04
#define E1_ICB4_IC30	0x02
#define E1_ICB4_IC31	0x01



#define T1_ICB1_IC1	0x80
#define T1_ICB1_IC2	0x40
#define T1_ICB1_IC3	0x20
#define T1_ICB1_IC4	0x10
#define T1_ICB1_IC5	0x08
#define T1_ICB1_IC6	0x04
#define T1_ICB1_IC7	0x02
#define T1_ICB1_IC8	0x01

#define T1_ICB2_IC9	0x80
#define T1_ICB2_IC10	0x40
#define T1_ICB2_IC11	0x20
#define T1_ICB2_IC12	0x10
#define T1_ICB2_IC13	0x08
#define T1_ICB2_IC14	0x04
#define T1_ICB2_IC15	0x02
#define T1_ICB2_IC16	0x01

#define T1_ICB3_IC17	0x80
#define T1_ICB3_IC18	0x40
#define T1_ICB3_IC19	0x20
#define T1_ICB3_IC20	0x10
#define T1_ICB3_IC21	0x08
#define T1_ICB3_IC22	0x04
#define T1_ICB3_IC23	0x02
#define T1_ICB3_IC24	0x01



#define FMR3_CMI        0x08
#define FMR3_SYNSA      0x04
#define FMR3_CFRZ       0x02
#define FMR3_EXTIW      0x01





#define CCB1_CH1        0x80
#define CCB1_CH2        0x40
#define CCB1_CH3        0x20
#define CCB1_CH4        0x10
#define CCB1_CH5        0x08
#define CCB1_CH6        0x04
#define CCB1_CH7        0x02
#define CCB1_CH8        0x01

#define CCB2_CH9        0x80
#define CCB2_CH10       0x40
#define CCB2_CH11       0x20
#define CCB2_CH12       0x10
#define CCB2_CH13       0x08
#define CCB2_CH14       0x04
#define CCB2_CH15       0x02
#define CCB2_CH16       0x01

#define CCB3_CH17       0x80
#define CCB3_CH18       0x40
#define CCB3_CH19       0x20
#define CCB3_CH20       0x10
#define CCB3_CH21       0x08
#define CCB3_CH22       0x04
#define CCB3_CH23       0x02
#define CCB3_CH24       0x01




#define LIM0_XFB        0x80
#define LIM0_XDOS       0x40
#define LIM0_SCL1       0x20
#define LIM0_SCL0       0x10
#define LIM0_EQON       0x08
#define LIM0_ELOS       0x04
#define LIM0_LL         0x02
#define LIM0_MAS        0x01

#define LIM1_EFSC       0x80
#define LIM1_RIL2       0x40
#define LIM1_RIL1       0x20
#define LIM1_RIL0       0x10
#define LIM1_DCOC       0x08
#define LIM1_JATT       0x04
#define LIM1_RL         0x02
#define LIM1_DRS        0x01




#define PCDR_PCD7	0x80
#define PCDR_PCD6	0x40
#define PCDR_PCD5	0x20
#define PCDR_PCD4	0x10
#define PCDR_PCD3	0x08
#define PCDR_PCD2	0x04
#define PCDR_PCD1	0x02
#define PCDR_PCD0	0x01

#define PCRR_PCR7	0x80
#define PCRR_PCR6	0x40
#define PCRR_PCR5	0x20
#define PCRR_PCR4	0x10
#define PCRR_PCR3	0x08
#define PCRR_PCR2	0x04
#define PCRR_PCR1	0x02
#define PCRR_PCR0	0x01




#define LIM2_DJA2	0x20
#define LIM2_DJA1	0x10
#define LIM2_LOS2	0x02
#define LIM2_LOS1	0x01



#define LCR1_EPRM	0x80
#define	LCR1_XPRBS	0x40


#define SIC1_SRSC	0x80
#define SIC1_RBS1	0x20
#define SIC1_RBS0	0x10
#define SIC1_SXSC	0x08
#define SIC1_XBS1	0x02
#define SIC1_XBS0	0x01



#define DEC_DCEC3       0x20
#define DEC_DBEC        0x10
#define DEC_DCEC1       0x08
#define DEC_DCEC        0x08
#define DEC_DEBC        0x04
#define DEC_DCVC        0x02
#define DEC_DFEC        0x01







#define FRS0_LOS        0x80
#define FRS0_AIS        0x40
#define FRS0_LFA        0x20
#define FRS0_RRA        0x10
#define FRS0_API        0x08
#define FRS0_NMF        0x04
#define FRS0_LMFA       0x02
#define FRS0_FSRF       0x01

#define FRS1_TS16RA     0x40
#define FRS1_TS16LOS    0x20
#define FRS1_TS16AIS    0x10
#define FRS1_TS16LFA    0x08
#define FRS1_EXZD       0x80
#define FRS1_LLBDD      0x10
#define FRS1_LLBAD      0x08
#define FRS1_XLS        0x02
#define FRS1_XLO        0x01
#define FRS1_PDEN	0x40



#define FRS2_ESC2       0x80
#define FRS2_ESC1       0x40
#define FRS2_ESC0       0x20

#define FRS3_FEH5       0x20
#define FRS3_FEH4       0x10
#define FRS3_FEH3       0x08
#define FRS3_FEH2       0x04
#define FRS3_FEH1       0x02
#define FRS3_FEH0       0x01




#define RSW_RSI         0x80
#define RSW_RRA         0x20
#define RSW_RYO         0x10
#define RSW_RY1         0x08
#define RSW_RY2         0x04
#define RSW_RY3         0x02
#define RSW_RY4         0x01




#define RSP_SI1         0x80
#define RSP_SI2         0x40
#define RSP_LLBDD	0x10
#define RSP_LLBAD	0x08
#define RSP_RSIF        0x04
#define RSP_RS13        0x02
#define RSP_RS15        0x01




#define FECL_FE7        0x80
#define FECL_FE6        0x40
#define FECL_FE5        0x20
#define FECL_FE4        0x10
#define FECL_FE3        0x08
#define FECL_FE2        0x04
#define FECL_FE1        0x02
#define FECL_FE0        0x01

#define FECH_FE15       0x80
#define FECH_FE14       0x40
#define FECH_FE13       0x20
#define FECH_FE12       0x10
#define FECH_FE11       0x08
#define FECH_FE10       0x04
#define FECH_FE9        0x02
#define FECH_FE8        0x01




#define CVCL_CV7        0x80
#define CVCL_CV6        0x40
#define CVCL_CV5        0x20
#define CVCL_CV4        0x10
#define CVCL_CV3        0x08
#define CVCL_CV2        0x04
#define CVCL_CV1        0x02
#define CVCL_CV0        0x01

#define CVCH_CV15       0x80
#define CVCH_CV14       0x40
#define CVCH_CV13       0x20
#define CVCH_CV12       0x10
#define CVCH_CV11       0x08
#define CVCH_CV10       0x04
#define CVCH_CV9        0x02
#define CVCH_CV8        0x01




#define CEC1L_CR7       0x80
#define CEC1L_CR6       0x40
#define CEC1L_CR5       0x20
#define CEC1L_CR4       0x10
#define CEC1L_CR3       0x08
#define CEC1L_CR2       0x04
#define CEC1L_CR1       0x02
#define CEC1L_CR0       0x01

#define CEC1H_CR15      0x80
#define CEC1H_CR14      0x40
#define CEC1H_CR13      0x20
#define CEC1H_CR12      0x10
#define CEC1H_CR11      0x08
#define CEC1H_CR10      0x04
#define CEC1H_CR9       0x02
#define CEC1H_CR8       0x01

#define CEC2L_CR7       0x80
#define CEC2L_CR6       0x40
#define CEC2L_CR5       0x20
#define CEC2L_CR4       0x10
#define CEC2L_CR3       0x08
#define CEC2L_CR2       0x04
#define CEC2L_CR1       0x02
#define CEC2L_CR0       0x01

#define CEC2H_CR15      0x80
#define CEC2H_CR14      0x40
#define CEC2H_CR13      0x20
#define CEC2H_CR12      0x10
#define CEC2H_CR11      0x08
#define CEC2H_CR10      0x04
#define CEC2H_CR9       0x02
#define CEC2H_CR8       0x01

#define CEC3L_CR7       0x80
#define CEC3L_CR6       0x40
#define CEC3L_CR5       0x20
#define CEC3L_CR4       0x10
#define CEC3L_CR3       0x08
#define CEC3L_CR2       0x04
#define CEC3L_CR1       0x02
#define CEC3L_CR0       0x01

#define CEC3H_CR15      0x80
#define CEC3H_CR14      0x40
#define CEC3H_CR13      0x20
#define CEC3H_CR12      0x10
#define CEC3H_CR11      0x08
#define CEC3H_CR10      0x04
#define CEC3H_CR9       0x02
#define CEC3H_CR8       0x01




#define CECL_CR7        0x80
#define CECL_CR6        0x40
#define CECL_CR5        0x20
#define CECL_CR4        0x10
#define CECL_CR3        0x08
#define CECL_CR2        0x04
#define CECL_CR1        0x02
#define CECL_CR0        0x01

#define CECH_CR15       0x80
#define CECH_CR14       0x40
#define CECH_CR13       0x20
#define CECH_CR12       0x10
#define CECH_CR11       0x08
#define CECH_CR10       0x04
#define CECH_CR9        0x02
#define CECH_CR8        0x01



#define EBCL_EB7        0x80
#define EBCL_EB6        0x40
#define EBCL_EB5        0x20
#define EBCL_EB4        0x10
#define EBCL_EB3        0x08
#define EBCL_EB2        0x04
#define EBCL_EB1        0x02
#define EBCL_EB0        0x01

#define EBCH_EB15       0x80
#define EBCH_EB14       0x40
#define EBCH_EB13       0x20
#define EBCH_EB12       0x10
#define EBCH_EB11       0x08
#define EBCH_EB10       0x04
#define EBCH_EB9        0x02
#define EBCH_EB8        0x01




#define RSA4_RS47       0x80
#define RSA4_RS46       0x40
#define RSA4_RS45       0x20
#define RSA4_RS44       0x10
#define RSA4_RS43       0x08
#define RSA4_RS42       0x04
#define RSA4_RS41       0x02
#define RSA4_RS40       0x01

#define RSA5_RS57       0x80
#define RSA5_RS56       0x40
#define RSA5_RS55       0x20
#define RSA5_RS54       0x10
#define RSA5_RS53       0x08
#define RSA5_RS52       0x04
#define RSA5_RS51       0x02
#define RSA5_RS50       0x01

#define RSA6_RS67       0x80
#define RSA6_RS66       0x40
#define RSA6_RS65       0x20
#define RSA6_RS64       0x10
#define RSA6_RS63       0x08
#define RSA6_RS62       0x04
#define RSA6_RS61       0x02
#define RSA6_RS60       0x01

#define RSA7_RS77       0x80
#define RSA7_RS76       0x40
#define RSA7_RS75       0x20
#define RSA7_RS74       0x10
#define RSA7_RS73       0x08
#define RSA7_RS72       0x04
#define RSA7_RS71       0x02
#define RSA7_RS70       0x01

#define RSA8_RS87       0x80
#define RSA8_RS86       0x40
#define RSA8_RS85       0x20
#define RSA8_RS84       0x10
#define RSA8_RS83       0x08
#define RSA8_RS82       0x04
#define RSA8_RS81       0x02
#define RSA8_RS80       0x01



#define RSA6S_SX        0x20
#define RSA6S_SF        0x10
#define RSA6S_SE        0x08
#define RSA6S_SC        0x04
#define RSA6S_SA        0x02
#define RSA6S_S8        0x01




#define RDL1_RDL17      0x80
#define RDL1_RDL16      0x40
#define RDL1_RDL15      0x20
#define RDL1_RDL14      0x10
#define RDL1_RDL13      0x08
#define RDL1_RDL12      0x04
#define RDL1_RDL11      0x02
#define RDL1_RDL10      0x01

#define RDL2_RDL27      0x80
#define RDL2_RDL26      0x40
#define RDL2_RDL25      0x20
#define RDL2_RDL24      0x10
#define RDL2_RDL23      0x08
#define RDL2_RDL22      0x04
#define RDL2_RDL21      0x02
#define RDL2_RDL20      0x01

#define RDL3_RDL37      0x80
#define RDL3_RDL36      0x40
#define RDL3_RDL35      0x20
#define RDL3_RDL34      0x10
#define RDL3_RDL33      0x08
#define RDL3_RDL32      0x04
#define RDL3_RDL31      0x02
#define RDL3_RDL30      0x01




#define SIS_XDOV        0x80
#define SIS_XFW         0x40
#define SIS_XREP        0x20
#define SIS_RLI         0x08
#define SIS_CEC         0x04
#define SIS_BOM         0x01




#define RSIS_VFR        0x80
#define RSIS_RDO        0x40
#define RSIS_CRC16      0x20
#define RSIS_RAB        0x10
#define RSIS_HA1        0x08
#define RSIS_HA0        0x04
#define RSIS_HFR        0x02
#define RSIS_LA         0x01




#define RBCL_RBC7       0x80
#define RBCL_RBC6       0x40
#define RBCL_RBC5       0x20
#define RBCL_RBC4       0x10
#define RBCL_RBC3       0x08
#define RBCL_RBC2       0x04
#define RBCL_RBC1       0x02
#define RBCL_RBC0       0x01

#define RBCH_OV         0x10
#define RBCH_RBC11      0x08
#define RBCH_RBC10      0x04
#define RBCH_RBC9       0x02
#define RBCH_RBC8       0x01




#define  FISR0_RME	0x80
#define  FISR0_RFS	0x40
#define  FISR0_T8MS	0x20
#define  FISR0_ISF	0x20
#define  FISR0_RMB	0x10
#define  FISR0_CASC	0x08
#define  FISR0_RSC	0x08
#define  FISR0_CRC6	0x04
#define  FISR0_CRC4	0x04
#define  FISR0_PDEN	0x02
#define  FISR0_RPF	0x01

#define  FISR1_CASE	0x80
#define  FISR1_LLBSC	0x80
#define  FISR1_RDO	0x40
#define  FISR1_ALLS	0x20
#define  FISR1_XDU	0x10
#define  FISR1_XMB	0x08
#define  FISR1_XLSC	0x02
#define  FISR1_XPR	0x01

#define  FISR2_FAR	0x80
#define  FISR2_LFA	0x40
#define  FISR2_MFAR	0x20
#define  FISR2_T400MS	0x10
#define  FISR2_LMFA	0x10
#define  FISR2_AIS	0x08
#define  FISR2_LOS	0x04
#define  FISR2_RAR	0x02
#define  FISR2_RA	0x01

#define  FISR3_ES	0x80
#define  FISR3_SEC	0x40
#define  FISR3_LMFA16	0x20
#define  FISR3_AIS16	0x10
#define  FISR3_RA16	0x08
#define  FISR3_API	0x04
#define  FISR3_XSLP	0x20
#define  FISR3_XSLN	0x10
#define  FISR3_LLBSC	0x08
#define  FISR3_XRS	0x04
#define  FISR3_SLN	0x02
#define  FISR3_SLP	0x01




#define  GIS_ISR3	0x08
#define  GIS_ISR2	0x04
#define  GIS_ISR1	0x02
#define  GIS_ISR0	0x01




#define  VSTR_VN3	0x08
#define  VSTR_VN2	0x04
#define  VSTR_VN1	0x02
#define  VSTR_VN0	0x01





#define XFIFOH	0x00		
#define XFIFOL	0x01		
#define CMDR	0x02		
#define DEC	0x60		
#define TEST2	0x62		
#define XS(nbr)	(0x70 + (nbr))	


#define MODE	0x03	
#define RAH1	0x04	
#define RAH2	0x05	
#define RAL1	0x06	
#define RAL2	0x07	
#define IPC	0x08	
#define CCR1	0x09	
#define CCR3	0x0A	
#define PRE	0x0B	
#define RTR1	0x0C	
#define RTR2	0x0D	
#define RTR3	0x0E	
#define RTR4	0x0F	
#define TTR1	0x10	
#define TTR2	0x11	
#define TTR3	0x12	
#define TTR4	0x13	
#define IMR0	0x14	
#define IMR1	0x15	
#define IMR2	0x16	
#define IMR3	0x17	
#define IMR4	0x18	
#define IMR5	0x19	
#define FMR0	0x1A	
#define FMR1	0x1B	
#define FMR2	0x1C	
#define LOOP	0x1D	
#define XSW	0x1E	
#define FMR4	0x1E	
#define XSP	0x1F	
#define FMR5	0x1F	
#define XC0	0x20	
#define XC1	0x21	
#define RC0	0x22	
#define RC1	0x23	
#define XPM0	0x24	
#define XPM1	0x25	
#define XPM2	0x26	
#define TSWM	0x27	
#define TEST1	0x28	
#define IDLE	0x29	
#define XSA4    0x2A	
#define XDL1	0x2A	
#define XSA5    0x2B	
#define XDL2	0x2B	
#define XSA6    0x2C	
#define XDL3	0x2C	
#define XSA7    0x2D	
#define CCB1	0x2D	
#define XSA8    0x2E	
#define CCB2	0x2E	
#define FMR3	0x2F	
#define CCB3	0x2F	
#define ICB1	0x30	
#define ICB2	0x31	
#define ICB3	0x32	
#define ICB4	0x33	
#define LIM0	0x34	
#define LIM1	0x35	
#define PCDR	0x36	
#define PCRR	0x37	
#define LIM2	0x38	
#define LCR1	0x39	
#define LCR2	0x3A	
#define LCR3	0x3B	
#define SIC1	0x3C	


#define RFIFOH	0x00		
#define RFIFOL	0x01		
#define FRS0	0x4C		
#define FRS1	0x4D		
#define RSW	0x4E		
#define FRS2	0x4E		
#define RSP	0x4F		
#define FRS3	0x4F		
#define FECL	0x50		
#define FECH	0x51		
#define CVCL	0x52		
#define CVCH	0x53		
#define CECL	0x54		
#define CECH	0x55		
#define EBCL	0x56		
#define EBCH	0x57		
#define BECL	0x58		
#define BECH	0x59		
#define CEC3	0x5A		
#define RSA4	0x5C		
#define RDL1	0x5C		
#define RSA5	0x5D		
#define RDL2	0x5D		
#define RSA6	0x5E		
#define RDL3	0x5E		
#define RSA7	0x5F		
#define RSA8	0x60		
#define RSA6S	0x61		
#define TSR0	0x62		
#define TSR1	0x63		
#define SIS	0x64		
#define RSIS	0x65		
#define RBCL	0x66		
#define RBCH	0x67		
#define FISR0	0x68		
#define FISR1	0x69		
#define FISR2	0x6A		
#define FISR3	0x6B		
#define GIS	0x6E		
#define VSTR	0x6F		
#define RS(nbr)	(0x70 + (nbr))	

#endif	

