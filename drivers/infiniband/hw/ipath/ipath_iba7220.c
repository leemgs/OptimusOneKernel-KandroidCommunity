


#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <rdma/ib_verbs.h>

#include "ipath_kernel.h"
#include "ipath_registers.h"
#include "ipath_7220.h"

static void ipath_setup_7220_setextled(struct ipath_devdata *, u64, u64);

static unsigned ipath_compat_ddr_negotiate = 1;

module_param_named(compat_ddr_negotiate, ipath_compat_ddr_negotiate, uint,
			S_IWUSR | S_IRUGO);
MODULE_PARM_DESC(compat_ddr_negotiate,
		"Attempt pre-IBTA 1.2 DDR speed negotiation");

static unsigned ipath_sdma_fetch_arb = 1;
module_param_named(fetch_arb, ipath_sdma_fetch_arb, uint, S_IRUGO);
MODULE_PARM_DESC(fetch_arb, "IBA7220: change SDMA descriptor arbitration");


struct _infinipath_do_not_use_kernel_regs {
	unsigned long long Revision;
	unsigned long long Control;
	unsigned long long PageAlign;
	unsigned long long PortCnt;
	unsigned long long DebugPortSelect;
	unsigned long long DebugSigsIntSel; 
	unsigned long long SendRegBase;
	unsigned long long UserRegBase;
	unsigned long long CounterRegBase;
	unsigned long long Scratch;
	unsigned long long EEPROMAddrCmd; 
	unsigned long long EEPROMData; 
	unsigned long long IntBlocked;
	unsigned long long IntMask;
	unsigned long long IntStatus;
	unsigned long long IntClear;
	unsigned long long ErrorMask;
	unsigned long long ErrorStatus;
	unsigned long long ErrorClear;
	unsigned long long HwErrMask;
	unsigned long long HwErrStatus;
	unsigned long long HwErrClear;
	unsigned long long HwDiagCtrl;
	unsigned long long MDIO;
	unsigned long long IBCStatus;
	unsigned long long IBCCtrl;
	unsigned long long ExtStatus;
	unsigned long long ExtCtrl;
	unsigned long long GPIOOut;
	unsigned long long GPIOMask;
	unsigned long long GPIOStatus;
	unsigned long long GPIOClear;
	unsigned long long RcvCtrl;
	unsigned long long RcvBTHQP;
	unsigned long long RcvHdrSize;
	unsigned long long RcvHdrCnt;
	unsigned long long RcvHdrEntSize;
	unsigned long long RcvTIDBase;
	unsigned long long RcvTIDCnt;
	unsigned long long RcvEgrBase;
	unsigned long long RcvEgrCnt;
	unsigned long long RcvBufBase;
	unsigned long long RcvBufSize;
	unsigned long long RxIntMemBase;
	unsigned long long RxIntMemSize;
	unsigned long long RcvPartitionKey;
	unsigned long long RcvQPMulticastPort;
	unsigned long long RcvPktLEDCnt;
	unsigned long long IBCDDRCtrl;
	unsigned long long HRTBT_GUID;
	unsigned long long IB_SDTEST_IF_TX;
	unsigned long long IB_SDTEST_IF_RX;
	unsigned long long IBCDDRCtrl2;
	unsigned long long IBCDDRStatus;
	unsigned long long JIntReload;
	unsigned long long IBNCModeCtrl;
	unsigned long long SendCtrl;
	unsigned long long SendBufBase;
	unsigned long long SendBufSize;
	unsigned long long SendBufCnt;
	unsigned long long SendAvailAddr;
	unsigned long long TxIntMemBase;
	unsigned long long TxIntMemSize;
	unsigned long long SendDmaBase;
	unsigned long long SendDmaLenGen;
	unsigned long long SendDmaTail;
	unsigned long long SendDmaHead;
	unsigned long long SendDmaHeadAddr;
	unsigned long long SendDmaBufMask0;
	unsigned long long SendDmaBufMask1;
	unsigned long long SendDmaBufMask2;
	unsigned long long SendDmaStatus;
	unsigned long long SendBufferError;
	unsigned long long SendBufferErrorCONT1;
	unsigned long long SendBufErr2; 
	unsigned long long Reserved6L[2];
	unsigned long long AvailUpdCount;
	unsigned long long RcvHdrAddr0;
	unsigned long long RcvHdrAddrs[16]; 
	unsigned long long Reserved7hdtl; 
	unsigned long long RcvHdrTailAddr0; 
	unsigned long long RcvHdrTailAddrs[16];
	unsigned long long Reserved9SW[7]; 
	unsigned long long IbsdEpbAccCtl; 
	unsigned long long IbsdEpbTransReg; 
	unsigned long long Reserved10sds; 
	unsigned long long XGXSConfig;
	unsigned long long IBSerDesCtrl; 
	unsigned long long EEPCtlStat; 
	unsigned long long EEPAddrCmd;
	unsigned long long EEPData;
	unsigned long long PcieEpbAccCtl;
	unsigned long long PcieEpbTransCtl;
	unsigned long long EfuseCtl; 
	unsigned long long EfuseData[4];
	unsigned long long ProcMon;
	
	unsigned long long PCIeRBufTestReg0;
	unsigned long long PCIeRBufTestReg1;
	
	unsigned long long PCIeRBufTestReg2;
	unsigned long long PCIeRBufTestReg3;
	
	unsigned long long SPC_JTAG_ACCESS_REG;
	unsigned long long LAControlReg;
	unsigned long long GPIODebugSelReg;
	unsigned long long DebugPortValueReg;
	
	unsigned long long SendDmaBufUsed[3];
	unsigned long long SendDmaReqTagUsed;
	
	unsigned long long efuse_pgm_data[2];
	unsigned long long Reserved11LAalign[10]; 
	
	unsigned long long SerDesDDSRXEQ[30];
	unsigned long long Reserved12LAalign[2]; 
	
	unsigned long long LAMemory[32];
};

struct _infinipath_do_not_use_counters {
	__u64 LBIntCnt;
	__u64 LBFlowStallCnt;
	__u64 TxSDmaDescCnt;	
	__u64 TxUnsupVLErrCnt;
	__u64 TxDataPktCnt;
	__u64 TxFlowPktCnt;
	__u64 TxDwordCnt;
	__u64 TxLenErrCnt;
	__u64 TxMaxMinLenErrCnt;
	__u64 TxUnderrunCnt;
	__u64 TxFlowStallCnt;
	__u64 TxDroppedPktCnt;
	__u64 RxDroppedPktCnt;
	__u64 RxDataPktCnt;
	__u64 RxFlowPktCnt;
	__u64 RxDwordCnt;
	__u64 RxLenErrCnt;
	__u64 RxMaxMinLenErrCnt;
	__u64 RxICRCErrCnt;
	__u64 RxVCRCErrCnt;
	__u64 RxFlowCtrlErrCnt;
	__u64 RxBadFormatCnt;
	__u64 RxLinkProblemCnt;
	__u64 RxEBPCnt;
	__u64 RxLPCRCErrCnt;
	__u64 RxBufOvflCnt;
	__u64 RxTIDFullErrCnt;
	__u64 RxTIDValidErrCnt;
	__u64 RxPKeyMismatchCnt;
	__u64 RxP0HdrEgrOvflCnt;
	__u64 RxP1HdrEgrOvflCnt;
	__u64 RxP2HdrEgrOvflCnt;
	__u64 RxP3HdrEgrOvflCnt;
	__u64 RxP4HdrEgrOvflCnt;
	__u64 RxP5HdrEgrOvflCnt;
	__u64 RxP6HdrEgrOvflCnt;
	__u64 RxP7HdrEgrOvflCnt;
	__u64 RxP8HdrEgrOvflCnt;
	__u64 RxP9HdrEgrOvflCnt;	
	__u64 RxP10HdrEgrOvflCnt;	
	__u64 RxP11HdrEgrOvflCnt;	
	__u64 RxP12HdrEgrOvflCnt;	
	__u64 RxP13HdrEgrOvflCnt;	
	__u64 RxP14HdrEgrOvflCnt;	
	__u64 RxP15HdrEgrOvflCnt;	
	__u64 RxP16HdrEgrOvflCnt;	
	__u64 IBStatusChangeCnt;
	__u64 IBLinkErrRecoveryCnt;
	__u64 IBLinkDownedCnt;
	__u64 IBSymbolErrCnt;
	
	__u64 RxVL15DroppedPktCnt;
	__u64 RxOtherLocalPhyErrCnt;
	__u64 PcieRetryBufDiagQwordCnt;
	__u64 ExcessBufferOvflCnt;
	__u64 LocalLinkIntegrityErrCnt;
	__u64 RxVlErrCnt;
	__u64 RxDlidFltrCnt;
	__u64 Reserved8[7];
	__u64 PSStat;
	__u64 PSStart;
	__u64 PSInterval;
	__u64 PSRcvDataCount;
	__u64 PSRcvPktsCount;
	__u64 PSXmitDataCount;
	__u64 PSXmitPktsCount;
	__u64 PSXmitWaitCount;
};

#define IPATH_KREG_OFFSET(field) (offsetof( \
	struct _infinipath_do_not_use_kernel_regs, field) / sizeof(u64))
#define IPATH_CREG_OFFSET(field) (offsetof( \
	struct _infinipath_do_not_use_counters, field) / sizeof(u64))

static const struct ipath_kregs ipath_7220_kregs = {
	.kr_control = IPATH_KREG_OFFSET(Control),
	.kr_counterregbase = IPATH_KREG_OFFSET(CounterRegBase),
	.kr_debugportselect = IPATH_KREG_OFFSET(DebugPortSelect),
	.kr_errorclear = IPATH_KREG_OFFSET(ErrorClear),
	.kr_errormask = IPATH_KREG_OFFSET(ErrorMask),
	.kr_errorstatus = IPATH_KREG_OFFSET(ErrorStatus),
	.kr_extctrl = IPATH_KREG_OFFSET(ExtCtrl),
	.kr_extstatus = IPATH_KREG_OFFSET(ExtStatus),
	.kr_gpio_clear = IPATH_KREG_OFFSET(GPIOClear),
	.kr_gpio_mask = IPATH_KREG_OFFSET(GPIOMask),
	.kr_gpio_out = IPATH_KREG_OFFSET(GPIOOut),
	.kr_gpio_status = IPATH_KREG_OFFSET(GPIOStatus),
	.kr_hwdiagctrl = IPATH_KREG_OFFSET(HwDiagCtrl),
	.kr_hwerrclear = IPATH_KREG_OFFSET(HwErrClear),
	.kr_hwerrmask = IPATH_KREG_OFFSET(HwErrMask),
	.kr_hwerrstatus = IPATH_KREG_OFFSET(HwErrStatus),
	.kr_ibcctrl = IPATH_KREG_OFFSET(IBCCtrl),
	.kr_ibcstatus = IPATH_KREG_OFFSET(IBCStatus),
	.kr_intblocked = IPATH_KREG_OFFSET(IntBlocked),
	.kr_intclear = IPATH_KREG_OFFSET(IntClear),
	.kr_intmask = IPATH_KREG_OFFSET(IntMask),
	.kr_intstatus = IPATH_KREG_OFFSET(IntStatus),
	.kr_mdio = IPATH_KREG_OFFSET(MDIO),
	.kr_pagealign = IPATH_KREG_OFFSET(PageAlign),
	.kr_partitionkey = IPATH_KREG_OFFSET(RcvPartitionKey),
	.kr_portcnt = IPATH_KREG_OFFSET(PortCnt),
	.kr_rcvbthqp = IPATH_KREG_OFFSET(RcvBTHQP),
	.kr_rcvbufbase = IPATH_KREG_OFFSET(RcvBufBase),
	.kr_rcvbufsize = IPATH_KREG_OFFSET(RcvBufSize),
	.kr_rcvctrl = IPATH_KREG_OFFSET(RcvCtrl),
	.kr_rcvegrbase = IPATH_KREG_OFFSET(RcvEgrBase),
	.kr_rcvegrcnt = IPATH_KREG_OFFSET(RcvEgrCnt),
	.kr_rcvhdrcnt = IPATH_KREG_OFFSET(RcvHdrCnt),
	.kr_rcvhdrentsize = IPATH_KREG_OFFSET(RcvHdrEntSize),
	.kr_rcvhdrsize = IPATH_KREG_OFFSET(RcvHdrSize),
	.kr_rcvintmembase = IPATH_KREG_OFFSET(RxIntMemBase),
	.kr_rcvintmemsize = IPATH_KREG_OFFSET(RxIntMemSize),
	.kr_rcvtidbase = IPATH_KREG_OFFSET(RcvTIDBase),
	.kr_rcvtidcnt = IPATH_KREG_OFFSET(RcvTIDCnt),
	.kr_revision = IPATH_KREG_OFFSET(Revision),
	.kr_scratch = IPATH_KREG_OFFSET(Scratch),
	.kr_sendbuffererror = IPATH_KREG_OFFSET(SendBufferError),
	.kr_sendctrl = IPATH_KREG_OFFSET(SendCtrl),
	.kr_sendpioavailaddr = IPATH_KREG_OFFSET(SendAvailAddr),
	.kr_sendpiobufbase = IPATH_KREG_OFFSET(SendBufBase),
	.kr_sendpiobufcnt = IPATH_KREG_OFFSET(SendBufCnt),
	.kr_sendpiosize = IPATH_KREG_OFFSET(SendBufSize),
	.kr_sendregbase = IPATH_KREG_OFFSET(SendRegBase),
	.kr_txintmembase = IPATH_KREG_OFFSET(TxIntMemBase),
	.kr_txintmemsize = IPATH_KREG_OFFSET(TxIntMemSize),
	.kr_userregbase = IPATH_KREG_OFFSET(UserRegBase),

	.kr_xgxsconfig = IPATH_KREG_OFFSET(XGXSConfig),

	
	.kr_senddmabase = IPATH_KREG_OFFSET(SendDmaBase),
	.kr_senddmalengen = IPATH_KREG_OFFSET(SendDmaLenGen),
	.kr_senddmatail = IPATH_KREG_OFFSET(SendDmaTail),
	.kr_senddmahead = IPATH_KREG_OFFSET(SendDmaHead),
	.kr_senddmaheadaddr = IPATH_KREG_OFFSET(SendDmaHeadAddr),
	.kr_senddmabufmask0 = IPATH_KREG_OFFSET(SendDmaBufMask0),
	.kr_senddmabufmask1 = IPATH_KREG_OFFSET(SendDmaBufMask1),
	.kr_senddmabufmask2 = IPATH_KREG_OFFSET(SendDmaBufMask2),
	.kr_senddmastatus = IPATH_KREG_OFFSET(SendDmaStatus),

	
	.kr_ibserdesctrl = IPATH_KREG_OFFSET(IBSerDesCtrl),
	.kr_ib_epbacc = IPATH_KREG_OFFSET(IbsdEpbAccCtl),
	.kr_ib_epbtrans = IPATH_KREG_OFFSET(IbsdEpbTransReg),
	.kr_pcie_epbacc = IPATH_KREG_OFFSET(PcieEpbAccCtl),
	.kr_pcie_epbtrans = IPATH_KREG_OFFSET(PcieEpbTransCtl),
	.kr_ib_ddsrxeq = IPATH_KREG_OFFSET(SerDesDDSRXEQ),

	
	.kr_rcvhdraddr = IPATH_KREG_OFFSET(RcvHdrAddr0),
	.kr_rcvhdrtailaddr = IPATH_KREG_OFFSET(RcvHdrTailAddr0),

	
	.kr_rcvpktledcnt = IPATH_KREG_OFFSET(RcvPktLEDCnt),
	.kr_pcierbuftestreg0 = IPATH_KREG_OFFSET(PCIeRBufTestReg0),
	.kr_pcierbuftestreg1 = IPATH_KREG_OFFSET(PCIeRBufTestReg1),

	.kr_hrtbt_guid = IPATH_KREG_OFFSET(HRTBT_GUID),
	.kr_ibcddrctrl = IPATH_KREG_OFFSET(IBCDDRCtrl),
	.kr_ibcddrstatus = IPATH_KREG_OFFSET(IBCDDRStatus),
	.kr_jintreload = IPATH_KREG_OFFSET(JIntReload)
};

static const struct ipath_cregs ipath_7220_cregs = {
	.cr_badformatcnt = IPATH_CREG_OFFSET(RxBadFormatCnt),
	.cr_erricrccnt = IPATH_CREG_OFFSET(RxICRCErrCnt),
	.cr_errlinkcnt = IPATH_CREG_OFFSET(RxLinkProblemCnt),
	.cr_errlpcrccnt = IPATH_CREG_OFFSET(RxLPCRCErrCnt),
	.cr_errpkey = IPATH_CREG_OFFSET(RxPKeyMismatchCnt),
	.cr_errrcvflowctrlcnt = IPATH_CREG_OFFSET(RxFlowCtrlErrCnt),
	.cr_err_rlencnt = IPATH_CREG_OFFSET(RxLenErrCnt),
	.cr_errslencnt = IPATH_CREG_OFFSET(TxLenErrCnt),
	.cr_errtidfull = IPATH_CREG_OFFSET(RxTIDFullErrCnt),
	.cr_errtidvalid = IPATH_CREG_OFFSET(RxTIDValidErrCnt),
	.cr_errvcrccnt = IPATH_CREG_OFFSET(RxVCRCErrCnt),
	.cr_ibstatuschange = IPATH_CREG_OFFSET(IBStatusChangeCnt),
	.cr_intcnt = IPATH_CREG_OFFSET(LBIntCnt),
	.cr_invalidrlencnt = IPATH_CREG_OFFSET(RxMaxMinLenErrCnt),
	.cr_invalidslencnt = IPATH_CREG_OFFSET(TxMaxMinLenErrCnt),
	.cr_lbflowstallcnt = IPATH_CREG_OFFSET(LBFlowStallCnt),
	.cr_pktrcvcnt = IPATH_CREG_OFFSET(RxDataPktCnt),
	.cr_pktrcvflowctrlcnt = IPATH_CREG_OFFSET(RxFlowPktCnt),
	.cr_pktsendcnt = IPATH_CREG_OFFSET(TxDataPktCnt),
	.cr_pktsendflowcnt = IPATH_CREG_OFFSET(TxFlowPktCnt),
	.cr_portovflcnt = IPATH_CREG_OFFSET(RxP0HdrEgrOvflCnt),
	.cr_rcvebpcnt = IPATH_CREG_OFFSET(RxEBPCnt),
	.cr_rcvovflcnt = IPATH_CREG_OFFSET(RxBufOvflCnt),
	.cr_senddropped = IPATH_CREG_OFFSET(TxDroppedPktCnt),
	.cr_sendstallcnt = IPATH_CREG_OFFSET(TxFlowStallCnt),
	.cr_sendunderruncnt = IPATH_CREG_OFFSET(TxUnderrunCnt),
	.cr_wordrcvcnt = IPATH_CREG_OFFSET(RxDwordCnt),
	.cr_wordsendcnt = IPATH_CREG_OFFSET(TxDwordCnt),
	.cr_unsupvlcnt = IPATH_CREG_OFFSET(TxUnsupVLErrCnt),
	.cr_rxdroppktcnt = IPATH_CREG_OFFSET(RxDroppedPktCnt),
	.cr_iblinkerrrecovcnt = IPATH_CREG_OFFSET(IBLinkErrRecoveryCnt),
	.cr_iblinkdowncnt = IPATH_CREG_OFFSET(IBLinkDownedCnt),
	.cr_ibsymbolerrcnt = IPATH_CREG_OFFSET(IBSymbolErrCnt),
	.cr_vl15droppedpktcnt = IPATH_CREG_OFFSET(RxVL15DroppedPktCnt),
	.cr_rxotherlocalphyerrcnt =
		IPATH_CREG_OFFSET(RxOtherLocalPhyErrCnt),
	.cr_excessbufferovflcnt = IPATH_CREG_OFFSET(ExcessBufferOvflCnt),
	.cr_locallinkintegrityerrcnt =
		IPATH_CREG_OFFSET(LocalLinkIntegrityErrCnt),
	.cr_rxvlerrcnt = IPATH_CREG_OFFSET(RxVlErrCnt),
	.cr_rxdlidfltrcnt = IPATH_CREG_OFFSET(RxDlidFltrCnt),
	.cr_psstat = IPATH_CREG_OFFSET(PSStat),
	.cr_psstart = IPATH_CREG_OFFSET(PSStart),
	.cr_psinterval = IPATH_CREG_OFFSET(PSInterval),
	.cr_psrcvdatacount = IPATH_CREG_OFFSET(PSRcvDataCount),
	.cr_psrcvpktscount = IPATH_CREG_OFFSET(PSRcvPktsCount),
	.cr_psxmitdatacount = IPATH_CREG_OFFSET(PSXmitDataCount),
	.cr_psxmitpktscount = IPATH_CREG_OFFSET(PSXmitPktsCount),
	.cr_psxmitwaitcount = IPATH_CREG_OFFSET(PSXmitWaitCount),
};


#define INFINIPATH_C_RESET (1U<<7)


#define INFINIPATH_I_RCVURG_MASK ((1ULL<<17)-1)
#define INFINIPATH_I_RCVURG_SHIFT 32
#define INFINIPATH_I_RCVAVAIL_MASK ((1ULL<<17)-1)
#define INFINIPATH_I_RCVAVAIL_SHIFT 0
#define INFINIPATH_I_SERDESTRIMDONE (1ULL<<27)


#define INFINIPATH_HWE_PCIEMEMPARITYERR_MASK  0x00000000000000ffULL
#define INFINIPATH_HWE_PCIEMEMPARITYERR_SHIFT 0
#define INFINIPATH_HWE_PCIEPOISONEDTLP      0x0000000010000000ULL
#define INFINIPATH_HWE_PCIECPLTIMEOUT       0x0000000020000000ULL
#define INFINIPATH_HWE_PCIEBUSPARITYXTLH    0x0000000040000000ULL
#define INFINIPATH_HWE_PCIEBUSPARITYXADM    0x0000000080000000ULL
#define INFINIPATH_HWE_PCIEBUSPARITYRADM    0x0000000100000000ULL
#define INFINIPATH_HWE_COREPLL_FBSLIP       0x0080000000000000ULL
#define INFINIPATH_HWE_COREPLL_RFSLIP       0x0100000000000000ULL
#define INFINIPATH_HWE_PCIE1PLLFAILED       0x0400000000000000ULL
#define INFINIPATH_HWE_PCIE0PLLFAILED       0x0800000000000000ULL
#define INFINIPATH_HWE_SERDESPLLFAILED      0x1000000000000000ULL

#define INFINIPATH_HWE_PCIECPLDATAQUEUEERR         0x0000000000000040ULL
#define INFINIPATH_HWE_PCIECPLHDRQUEUEERR          0x0000000000000080ULL
#define INFINIPATH_HWE_SDMAMEMREADERR              0x0000000010000000ULL
#define INFINIPATH_HWE_CLK_UC_PLLNOTLOCKED	   0x2000000000000000ULL
#define INFINIPATH_HWE_PCIESERDESQ0PCLKNOTDETECT   0x0100000000000000ULL
#define INFINIPATH_HWE_PCIESERDESQ1PCLKNOTDETECT   0x0200000000000000ULL
#define INFINIPATH_HWE_PCIESERDESQ2PCLKNOTDETECT   0x0400000000000000ULL
#define INFINIPATH_HWE_PCIESERDESQ3PCLKNOTDETECT   0x0800000000000000ULL
#define INFINIPATH_HWE_DDSRXEQMEMORYPARITYERR	   0x0000008000000000ULL
#define INFINIPATH_HWE_IB_UC_MEMORYPARITYERR	   0x0000004000000000ULL
#define INFINIPATH_HWE_PCIE_UC_OCT0MEMORYPARITYERR 0x0000001000000000ULL
#define INFINIPATH_HWE_PCIE_UC_OCT1MEMORYPARITYERR 0x0000002000000000ULL

#define IBA7220_IBCS_LINKTRAININGSTATE_MASK 0x1F
#define IBA7220_IBCS_LINKSTATE_SHIFT 5
#define IBA7220_IBCS_LINKSPEED_SHIFT 8
#define IBA7220_IBCS_LINKWIDTH_SHIFT 9

#define IBA7220_IBCC_LINKINITCMD_MASK 0x7ULL
#define IBA7220_IBCC_LINKCMD_SHIFT 19
#define IBA7220_IBCC_MAXPKTLEN_SHIFT 21


#define IBA7220_IBC_DLIDLMC_MASK	0xFFFFFFFFUL
#define IBA7220_IBC_DLIDLMC_SHIFT	32
#define IBA7220_IBC_HRTBT_MASK	3
#define IBA7220_IBC_HRTBT_SHIFT	16
#define IBA7220_IBC_HRTBT_ENB	0x10000UL
#define IBA7220_IBC_LANE_REV_SUPPORTED (1<<8)
#define IBA7220_IBC_LREV_MASK	1
#define IBA7220_IBC_LREV_SHIFT	8
#define IBA7220_IBC_RXPOL_MASK	1
#define IBA7220_IBC_RXPOL_SHIFT	7
#define IBA7220_IBC_WIDTH_SHIFT	5
#define IBA7220_IBC_WIDTH_MASK	0x3
#define IBA7220_IBC_WIDTH_1X_ONLY	(0<<IBA7220_IBC_WIDTH_SHIFT)
#define IBA7220_IBC_WIDTH_4X_ONLY	(1<<IBA7220_IBC_WIDTH_SHIFT)
#define IBA7220_IBC_WIDTH_AUTONEG	(2<<IBA7220_IBC_WIDTH_SHIFT)
#define IBA7220_IBC_SPEED_AUTONEG	(1<<1)
#define IBA7220_IBC_SPEED_SDR		(1<<2)
#define IBA7220_IBC_SPEED_DDR		(1<<3)
#define IBA7220_IBC_SPEED_AUTONEG_MASK  (0x7<<1)
#define IBA7220_IBC_IBTA_1_2_MASK	(1)



#define IBA7220_DDRSTAT_LINKLAT_MASK    0x3ffffff


#define INFINIPATH_EXTS_FREQSEL 0x2
#define INFINIPATH_EXTS_SERDESSEL 0x4
#define INFINIPATH_EXTS_MEMBIST_ENDTEST     0x0000000000004000
#define INFINIPATH_EXTS_MEMBIST_DISABLED    0x0000000000008000


#define INFINIPATH_XGXS_RESET          0x5ULL
#define INFINIPATH_XGXS_FC_SAFE        (1ULL<<63)


#define IBA7220_LEDBLINK_ON_SHIFT 32 
#define IBA7220_LEDBLINK_OFF_SHIFT 0 

#define _IPATH_GPIO_SDA_NUM 1
#define _IPATH_GPIO_SCL_NUM 0

#define IPATH_GPIO_SDA (1ULL << \
	(_IPATH_GPIO_SDA_NUM+INFINIPATH_EXTC_GPIOOE_SHIFT))
#define IPATH_GPIO_SCL (1ULL << \
	(_IPATH_GPIO_SCL_NUM+INFINIPATH_EXTC_GPIOOE_SHIFT))

#define IBA7220_R_INTRAVAIL_SHIFT 17
#define IBA7220_R_TAILUPD_SHIFT 35
#define IBA7220_R_PORTCFG_SHIFT 36

#define INFINIPATH_JINT_PACKETSHIFT 16
#define INFINIPATH_JINT_DEFAULT_IDLE_TICKS  0
#define INFINIPATH_JINT_DEFAULT_MAX_PACKETS 0

#define IBA7220_HDRHEAD_PKTINT_SHIFT 32 


#define IBA7220_TID_SZ_SHIFT 37 
#define IBA7220_TID_SZ_2K (1UL<<IBA7220_TID_SZ_SHIFT) 
#define IBA7220_TID_SZ_4K (2UL<<IBA7220_TID_SZ_SHIFT) 
#define IBA7220_TID_PA_SHIFT 11U 

#define IPATH_AUTONEG_TRIES 5 

static char int_type[16] = "auto";
module_param_string(interrupt_type, int_type, sizeof(int_type), 0444);
MODULE_PARM_DESC(int_type, " interrupt_type=auto|force_msi|force_intx");


static u8 rate_to_delay[2][2] = {
	
	{   8, 2 }, 
	{   4, 1 }  
};


static const struct ipath_hwerror_msgs ipath_7220_hwerror_msgs[] = {
	INFINIPATH_HWE_MSG(PCIEPOISONEDTLP, "PCIe Poisoned TLP"),
	INFINIPATH_HWE_MSG(PCIECPLTIMEOUT, "PCIe completion timeout"),
	
	INFINIPATH_HWE_MSG(PCIE1PLLFAILED, "PCIePLL1"),
	INFINIPATH_HWE_MSG(PCIE0PLLFAILED, "PCIePLL0"),
	INFINIPATH_HWE_MSG(PCIEBUSPARITYXTLH, "PCIe XTLH core parity"),
	INFINIPATH_HWE_MSG(PCIEBUSPARITYXADM, "PCIe ADM TX core parity"),
	INFINIPATH_HWE_MSG(PCIEBUSPARITYRADM, "PCIe ADM RX core parity"),
	INFINIPATH_HWE_MSG(RXDSYNCMEMPARITYERR, "Rx Dsync"),
	INFINIPATH_HWE_MSG(SERDESPLLFAILED, "SerDes PLL"),
	INFINIPATH_HWE_MSG(PCIECPLDATAQUEUEERR, "PCIe cpl header queue"),
	INFINIPATH_HWE_MSG(PCIECPLHDRQUEUEERR, "PCIe cpl data queue"),
	INFINIPATH_HWE_MSG(SDMAMEMREADERR, "Send DMA memory read"),
	INFINIPATH_HWE_MSG(CLK_UC_PLLNOTLOCKED, "uC PLL clock not locked"),
	INFINIPATH_HWE_MSG(PCIESERDESQ0PCLKNOTDETECT,
		"PCIe serdes Q0 no clock"),
	INFINIPATH_HWE_MSG(PCIESERDESQ1PCLKNOTDETECT,
		"PCIe serdes Q1 no clock"),
	INFINIPATH_HWE_MSG(PCIESERDESQ2PCLKNOTDETECT,
		"PCIe serdes Q2 no clock"),
	INFINIPATH_HWE_MSG(PCIESERDESQ3PCLKNOTDETECT,
		"PCIe serdes Q3 no clock"),
	INFINIPATH_HWE_MSG(DDSRXEQMEMORYPARITYERR,
		"DDS RXEQ memory parity"),
	INFINIPATH_HWE_MSG(IB_UC_MEMORYPARITYERR, "IB uC memory parity"),
	INFINIPATH_HWE_MSG(PCIE_UC_OCT0MEMORYPARITYERR,
		"PCIe uC oct0 memory parity"),
	INFINIPATH_HWE_MSG(PCIE_UC_OCT1MEMORYPARITYERR,
		"PCIe uC oct1 memory parity"),
};

static void autoneg_work(struct work_struct *);


static inline u32 port_egrtid_idx(struct ipath_devdata *dd, unsigned port)
{
	 return port ? dd->ipath_p0_rcvegrcnt +
		 (port-1) * dd->ipath_rcvegrcnt : 0;
}

static void ipath_7220_txe_recover(struct ipath_devdata *dd)
{
	++ipath_stats.sps_txeparity;

	dev_info(&dd->pcidev->dev,
		"Recovering from TXE PIO parity error\n");
	ipath_disarm_senderrbufs(dd);
}



static void ipath_7220_handle_hwerrors(struct ipath_devdata *dd, char *msg,
				       size_t msgl)
{
	ipath_err_t hwerrs;
	u32 bits, ctrl;
	int isfatal = 0;
	char bitsmsg[64];
	int log_idx;

	hwerrs = ipath_read_kreg64(dd, dd->ipath_kregs->kr_hwerrstatus);
	if (!hwerrs) {
		
		ipath_cdbg(VERBOSE, "Called but no hardware errors set\n");
		goto bail;
	} else if (hwerrs == ~0ULL) {
		ipath_dev_err(dd, "Read of hardware error status failed "
			      "(all bits set); ignoring\n");
		goto bail;
	}
	ipath_stats.sps_hwerrs++;

	
	ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrclear,
			 hwerrs&~INFINIPATH_HWE_MEMBISTFAILED);

	hwerrs &= dd->ipath_hwerrmask;

	
	for (log_idx = 0; log_idx < IPATH_EEP_LOG_CNT; ++log_idx)
		if (hwerrs & dd->ipath_eep_st_masks[log_idx].hwerrs_to_log)
			ipath_inc_eeprom_err(dd, log_idx, 1);
	
	if ((hwerrs & ~(dd->ipath_lasthwerror |
			((INFINIPATH_HWE_TXEMEMPARITYERR_PIOBUF |
			  INFINIPATH_HWE_TXEMEMPARITYERR_PIOPBC)
			 << INFINIPATH_HWE_TXEMEMPARITYERR_SHIFT))) ||
	    (ipath_debug & __IPATH_VERBDBG))
		dev_info(&dd->pcidev->dev, "Hardware error: hwerr=0x%llx "
			 "(cleared)\n", (unsigned long long) hwerrs);
	dd->ipath_lasthwerror |= hwerrs;

	if (hwerrs & ~dd->ipath_hwe_bitsextant)
		ipath_dev_err(dd, "hwerror interrupt with unknown errors "
			      "%llx set\n", (unsigned long long)
			      (hwerrs & ~dd->ipath_hwe_bitsextant));

	if (hwerrs & INFINIPATH_HWE_IB_UC_MEMORYPARITYERR)
		ipath_sd7220_clr_ibpar(dd);

	ctrl = ipath_read_kreg32(dd, dd->ipath_kregs->kr_control);
	if ((ctrl & INFINIPATH_C_FREEZEMODE) && !ipath_diag_inuse) {
		
		if (hwerrs & ((INFINIPATH_HWE_TXEMEMPARITYERR_PIOBUF |
			       INFINIPATH_HWE_TXEMEMPARITYERR_PIOPBC)
			      << INFINIPATH_HWE_TXEMEMPARITYERR_SHIFT)) {
			ipath_7220_txe_recover(dd);
			hwerrs &= ~((INFINIPATH_HWE_TXEMEMPARITYERR_PIOBUF |
				     INFINIPATH_HWE_TXEMEMPARITYERR_PIOPBC)
				    << INFINIPATH_HWE_TXEMEMPARITYERR_SHIFT);
		}
		if (hwerrs) {
			
			if (dd->ipath_flags & IPATH_INITTED) {
				ipath_set_linkstate(dd, IPATH_IB_LINKDOWN);
				ipath_setup_7220_setextled(dd,
					INFINIPATH_IBCS_L_STATE_DOWN,
					INFINIPATH_IBCS_LT_STATE_DISABLED);
				ipath_dev_err(dd, "Fatal Hardware Error "
					      "(freeze mode), no longer"
					      " usable, SN %.16s\n",
						  dd->ipath_serial);
				isfatal = 1;
			}
			
			*dd->ipath_statusp &= ~IPATH_STATUS_IB_READY;
			*dd->ipath_statusp |= IPATH_STATUS_HWERROR;
			dd->ipath_flags &= ~IPATH_INITTED;
		} else {
			ipath_dbg("Clearing freezemode on ignored or "
				"recovered hardware error\n");
			ipath_clear_freeze(dd);
		}
	}

	*msg = '\0';

	if (hwerrs & INFINIPATH_HWE_MEMBISTFAILED) {
		strlcat(msg, "[Memory BIST test failed, "
			"InfiniPath hardware unusable]", msgl);
		
		*dd->ipath_statusp |= IPATH_STATUS_HWERROR;
		dd->ipath_hwerrmask &= ~INFINIPATH_HWE_MEMBISTFAILED;
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrmask,
				 dd->ipath_hwerrmask);
	}

	ipath_format_hwerrors(hwerrs,
			      ipath_7220_hwerror_msgs,
			      ARRAY_SIZE(ipath_7220_hwerror_msgs),
			      msg, msgl);

	if (hwerrs & (INFINIPATH_HWE_PCIEMEMPARITYERR_MASK
		      << INFINIPATH_HWE_PCIEMEMPARITYERR_SHIFT)) {
		bits = (u32) ((hwerrs >>
			       INFINIPATH_HWE_PCIEMEMPARITYERR_SHIFT) &
			      INFINIPATH_HWE_PCIEMEMPARITYERR_MASK);
		snprintf(bitsmsg, sizeof bitsmsg,
			 "[PCIe Mem Parity Errs %x] ", bits);
		strlcat(msg, bitsmsg, msgl);
	}

#define _IPATH_PLL_FAIL (INFINIPATH_HWE_COREPLL_FBSLIP |	\
			 INFINIPATH_HWE_COREPLL_RFSLIP)

	if (hwerrs & _IPATH_PLL_FAIL) {
		snprintf(bitsmsg, sizeof bitsmsg,
			 "[PLL failed (%llx), InfiniPath hardware unusable]",
			 (unsigned long long) hwerrs & _IPATH_PLL_FAIL);
		strlcat(msg, bitsmsg, msgl);
		
		dd->ipath_hwerrmask &= ~(hwerrs & _IPATH_PLL_FAIL);
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrmask,
				 dd->ipath_hwerrmask);
	}

	if (hwerrs & INFINIPATH_HWE_SERDESPLLFAILED) {
		
		dd->ipath_hwerrmask &= ~INFINIPATH_HWE_SERDESPLLFAILED;
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrmask,
				 dd->ipath_hwerrmask);
	}

	ipath_dev_err(dd, "%s hardware error\n", msg);
	
	if (isfatal && !ipath_diag_inuse && dd->ipath_freezemsg)
		snprintf(dd->ipath_freezemsg, dd->ipath_freezelen,
			 "{%s}", msg);
bail:;
}


static int ipath_7220_boardname(struct ipath_devdata *dd, char *name,
	size_t namelen)
{
	char *n = NULL;
	u8 boardrev = dd->ipath_boardrev;
	int ret;

	if (boardrev == 15) {
		
		boardrev = 0;
		dd->ipath_boardrev = boardrev;
	}
	switch (boardrev) {
	case 0:
		n = "InfiniPath_7220_Emulation";
		break;
	case 1:
		n = "InfiniPath_QLE7240";
		break;
	case 2:
		n = "InfiniPath_QLE7280";
		break;
	case 3:
		n = "InfiniPath_QLE7242";
		break;
	case 4:
		n = "InfiniPath_QEM7240";
		break;
	case 5:
		n = "InfiniPath_QMI7240";
		break;
	case 6:
		n = "InfiniPath_QMI7264";
		break;
	case 7:
		n = "InfiniPath_QMH7240";
		break;
	case 8:
		n = "InfiniPath_QME7240";
		break;
	case 9:
		n = "InfiniPath_QLE7250";
		break;
	case 10:
		n = "InfiniPath_QLE7290";
		break;
	case 11:
		n = "InfiniPath_QEM7250";
		break;
	case 12:
		n = "InfiniPath_QLE-Bringup";
		break;
	default:
		ipath_dev_err(dd,
			      "Don't yet know about board with ID %u\n",
			      boardrev);
		snprintf(name, namelen, "Unknown_InfiniPath_PCIe_%u",
			 boardrev);
		break;
	}
	if (n)
		snprintf(name, namelen, "%s", n);

	if (dd->ipath_majrev != 5 || !dd->ipath_minrev ||
		dd->ipath_minrev > 2) {
		ipath_dev_err(dd, "Unsupported InfiniPath hardware "
			      "revision %u.%u!\n",
			      dd->ipath_majrev, dd->ipath_minrev);
		ret = 1;
	} else if (dd->ipath_minrev == 1 &&
		!(dd->ipath_flags & IPATH_INITTED)) {
		
		ipath_dev_err(dd, "Unsupported hardware "
			      "revision %u.%u, Contact support@qlogic.com\n",
			      dd->ipath_majrev, dd->ipath_minrev);
		ret = 0;
	} else
		ret = 0;

	
	dd->ipath_ureg_align = 0x10000;  

	return ret;
}


static void ipath_7220_init_hwerrors(struct ipath_devdata *dd)
{
	ipath_err_t val;
	u64 extsval;

	extsval = ipath_read_kreg64(dd, dd->ipath_kregs->kr_extstatus);

	if (!(extsval & (INFINIPATH_EXTS_MEMBIST_ENDTEST |
			INFINIPATH_EXTS_MEMBIST_DISABLED)))
		ipath_dev_err(dd, "MemBIST did not complete!\n");
	if (extsval & INFINIPATH_EXTS_MEMBIST_DISABLED)
		dev_info(&dd->pcidev->dev, "MemBIST is disabled.\n");

	val = ~0ULL;	

	if (!dd->ipath_boardrev)	
		val &= ~INFINIPATH_HWE_SERDESPLLFAILED;

	if (dd->ipath_minrev == 1)
		val &= ~(1ULL << 42); 

	val &= ~INFINIPATH_HWE_IB_UC_MEMORYPARITYERR;
	dd->ipath_hwerrmask = val;

	
	if (!dd->ipath_boardrev)
		dd->ipath_maskederrs = INFINIPATH_E_SENDSPECIALTRIGGER;
}




static int ipath_7220_bringup_serdes(struct ipath_devdata *dd)
{
	int ret = 0;
	u64 val, prev_val, guid;
	int was_reset;		

	ipath_dbg("Trying to bringup serdes\n");

	if (ipath_read_kreg64(dd, dd->ipath_kregs->kr_hwerrstatus) &
	    INFINIPATH_HWE_SERDESPLLFAILED) {
		ipath_dbg("At start, serdes PLL failed bit set "
			  "in hwerrstatus, clearing and continuing\n");
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwerrclear,
				 INFINIPATH_HWE_SERDESPLLFAILED);
	}

	dd->ibdeltainprog = 1;
	dd->ibsymsnap =
	     ipath_read_creg32(dd, dd->ipath_cregs->cr_ibsymbolerrcnt);
	dd->iblnkerrsnap =
	     ipath_read_creg32(dd, dd->ipath_cregs->cr_iblinkerrrecovcnt);

	if (!dd->ipath_ibcddrctrl) {
		
		dd->ipath_ibcddrctrl =
			ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibcddrctrl);

		if (dd->ipath_link_speed_enabled ==
			(IPATH_IB_SDR | IPATH_IB_DDR))
			dd->ipath_ibcddrctrl |=
				IBA7220_IBC_SPEED_AUTONEG_MASK |
				IBA7220_IBC_IBTA_1_2_MASK;
		else
			dd->ipath_ibcddrctrl |=
				dd->ipath_link_speed_enabled == IPATH_IB_DDR
				?  IBA7220_IBC_SPEED_DDR :
				IBA7220_IBC_SPEED_SDR;
		if ((dd->ipath_link_width_enabled & (IB_WIDTH_1X |
			IB_WIDTH_4X)) == (IB_WIDTH_1X | IB_WIDTH_4X))
			dd->ipath_ibcddrctrl |= IBA7220_IBC_WIDTH_AUTONEG;
		else
			dd->ipath_ibcddrctrl |=
				dd->ipath_link_width_enabled == IB_WIDTH_4X
				? IBA7220_IBC_WIDTH_4X_ONLY :
				IBA7220_IBC_WIDTH_1X_ONLY;

		
		dd->ipath_ibcddrctrl |=
			IBA7220_IBC_RXPOL_MASK << IBA7220_IBC_RXPOL_SHIFT;
		dd->ipath_ibcddrctrl |=
			IBA7220_IBC_HRTBT_MASK << IBA7220_IBC_HRTBT_SHIFT;
		
		if (dd->ipath_minrev == 1)
			dd->ipath_ibcddrctrl &=
			~IBA7220_IBC_LANE_REV_SUPPORTED;
		else
			dd->ipath_ibcddrctrl |=
				IBA7220_IBC_LANE_REV_SUPPORTED;
	}

	ipath_write_kreg(dd, dd->ipath_kregs->kr_ibcddrctrl,
			dd->ipath_ibcddrctrl);

	ipath_write_kreg(dd, IPATH_KREG_OFFSET(IBNCModeCtrl), 0Ull);

	
	val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibserdesctrl);
	
	was_reset = (val & 1);
	ipath_cdbg(VERBOSE, "IBReset %s xgxsconfig %llx\n",
		   was_reset ? "Asserted" : "Negated", (unsigned long long)
		   ipath_read_kreg64(dd, dd->ipath_kregs->kr_xgxsconfig));

	if (dd->ipath_boardrev) {
		
		ret = ipath_sd7220_init(dd, was_reset);
	}

	val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_xgxsconfig);
	prev_val = val;
	val |= INFINIPATH_XGXS_FC_SAFE;
	if (val != prev_val) {
		ipath_write_kreg(dd, dd->ipath_kregs->kr_xgxsconfig, val);
		ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
	}
	if (val & INFINIPATH_XGXS_RESET)
		val &= ~INFINIPATH_XGXS_RESET;
	if (val != prev_val)
		ipath_write_kreg(dd, dd->ipath_kregs->kr_xgxsconfig, val);

	ipath_cdbg(VERBOSE, "done: xgxs=%llx from %llx\n",
		   (unsigned long long)
		   ipath_read_kreg64(dd, dd->ipath_kregs->kr_xgxsconfig),
		   (unsigned long long) prev_val);

	guid = be64_to_cpu(dd->ipath_guid);

	if (!guid) {
		
		guid = get_cycles();
		ipath_dbg("No GUID for heartbeat, faking %llx\n",
			(unsigned long long)guid);
	} else
		ipath_cdbg(VERBOSE, "Wrote %llX to HRTBT_GUID\n",
			(unsigned long long) guid);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_hrtbt_guid, guid);
	return ret;
}

static void ipath_7220_config_jint(struct ipath_devdata *dd,
				   u16 idle_ticks, u16 max_packets)
{

	
	if (idle_ticks == 0 || max_packets == 0)
		
		dd->ipath_rhdrhead_intr_off =
			1ULL << IBA7220_HDRHEAD_PKTINT_SHIFT;
	else
		
		dd->ipath_rhdrhead_intr_off = 0ULL;

	
	ipath_write_ureg(dd, ur_rcvhdrhead,
			 dd->ipath_rhdrhead_intr_off |
			 dd->ipath_pd[0]->port_head, 0);

	dd->ipath_jint_max_packets = max_packets;
	dd->ipath_jint_idle_ticks = idle_ticks;
	ipath_write_kreg(dd, dd->ipath_kregs->kr_jintreload,
			 ((u64) max_packets << INFINIPATH_JINT_PACKETSHIFT) |
			 idle_ticks);
}


static void ipath_7220_quiet_serdes(struct ipath_devdata *dd)
{
	u64 val;
	if (dd->ibsymdelta || dd->iblnkerrdelta ||
	    dd->ibdeltainprog) {
		u64 diagc;
		
		diagc = ipath_read_kreg64(dd, dd->ipath_kregs->kr_hwdiagctrl);
		ipath_write_kreg(dd, dd->ipath_kregs->kr_hwdiagctrl,
				 diagc | INFINIPATH_DC_COUNTERWREN);

		if (dd->ibsymdelta || dd->ibdeltainprog) {
			val = ipath_read_creg32(dd,
					dd->ipath_cregs->cr_ibsymbolerrcnt);
			if (dd->ibdeltainprog)
				val -= val - dd->ibsymsnap;
			val -= dd->ibsymdelta;
			ipath_write_creg(dd,
				  dd->ipath_cregs->cr_ibsymbolerrcnt, val);
		}
		if (dd->iblnkerrdelta || dd->ibdeltainprog) {
			val = ipath_read_creg32(dd,
					dd->ipath_cregs->cr_iblinkerrrecovcnt);
			if (dd->ibdeltainprog)
				val -= val - dd->iblnkerrsnap;
			val -= dd->iblnkerrdelta;
			ipath_write_creg(dd,
				   dd->ipath_cregs->cr_iblinkerrrecovcnt, val);
	     }

	     
	     ipath_write_kreg(dd, dd->ipath_kregs->kr_hwdiagctrl, diagc);
	}

	dd->ipath_flags &= ~IPATH_IB_AUTONEG_INPROG;
	wake_up(&dd->ipath_autoneg_wait);
	cancel_delayed_work(&dd->ipath_autoneg_work);
	flush_scheduled_work();
	ipath_shutdown_relock_poll(dd);
	val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_xgxsconfig);
	val |= INFINIPATH_XGXS_RESET;
	ipath_write_kreg(dd, dd->ipath_kregs->kr_xgxsconfig, val);
}

static int ipath_7220_intconfig(struct ipath_devdata *dd)
{
	ipath_7220_config_jint(dd, dd->ipath_jint_idle_ticks,
			       dd->ipath_jint_max_packets);
	return 0;
}


static void ipath_setup_7220_setextled(struct ipath_devdata *dd, u64 lst,
				       u64 ltst)
{
	u64 extctl, ledblink = 0;
	unsigned long flags = 0;

	
	if (ipath_diag_inuse)
		return;

	
	if (dd->ipath_led_override) {
		ltst = (dd->ipath_led_override & IPATH_LED_PHYS)
			? INFINIPATH_IBCS_LT_STATE_LINKUP
			: INFINIPATH_IBCS_LT_STATE_DISABLED;
		lst = (dd->ipath_led_override & IPATH_LED_LOG)
			? INFINIPATH_IBCS_L_STATE_ACTIVE
			: INFINIPATH_IBCS_L_STATE_DOWN;
	}

	spin_lock_irqsave(&dd->ipath_gpio_lock, flags);
	extctl = dd->ipath_extctrl & ~(INFINIPATH_EXTC_LED1PRIPORT_ON |
				       INFINIPATH_EXTC_LED2PRIPORT_ON);
	if (ltst == INFINIPATH_IBCS_LT_STATE_LINKUP) {
		extctl |= INFINIPATH_EXTC_LED1PRIPORT_ON;
		
		ledblink = ((66600*1000UL/4) << IBA7220_LEDBLINK_ON_SHIFT)
			| ((187500*1000UL/4) << IBA7220_LEDBLINK_OFF_SHIFT);
	}
	if (lst == INFINIPATH_IBCS_L_STATE_ACTIVE)
		extctl |= INFINIPATH_EXTC_LED2PRIPORT_ON;
	dd->ipath_extctrl = extctl;
	ipath_write_kreg(dd, dd->ipath_kregs->kr_extctrl, extctl);
	spin_unlock_irqrestore(&dd->ipath_gpio_lock, flags);

	if (ledblink) 
		ipath_write_kreg(dd, dd->ipath_kregs->kr_rcvpktledcnt,
			ledblink);
}


static void ipath_enable_intx(struct pci_dev *pdev)
{
	u16 cw, new;
	int pos;

	
	pci_read_config_word(pdev, PCI_COMMAND, &cw);
	new = cw & ~PCI_COMMAND_INTX_DISABLE;
	if (new != cw)
		pci_write_config_word(pdev, PCI_COMMAND, new);

	
	pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
	if (pos) {
		pci_read_config_word(pdev, pos + PCI_MSI_FLAGS, &cw);
		new = cw & ~PCI_MSI_FLAGS_ENABLE;
		if (new != cw)
			pci_write_config_word(pdev, pos + PCI_MSI_FLAGS, new);
	}
}

static int ipath_msi_enabled(struct pci_dev *pdev)
{
	int pos, ret = 0;

	pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
	if (pos) {
		u16 cw;

		pci_read_config_word(pdev, pos + PCI_MSI_FLAGS, &cw);
		ret = !!(cw & PCI_MSI_FLAGS_ENABLE);
	}
	return ret;
}


static void ipath_7220_nomsi(struct ipath_devdata *dd)
{
	dd->ipath_msi_lo = 0;

	if (ipath_msi_enabled(dd->pcidev)) {
		
		 if (dd->ipath_irq)
			free_irq(dd->ipath_irq, dd);
		pci_disable_msi(dd->pcidev);
	}
}


static void ipath_setup_7220_cleanup(struct ipath_devdata *dd)
{
	ipath_7220_nomsi(dd);
}


static void ipath_7220_pcie_params(struct ipath_devdata *dd, u32 boardrev)
{
	u16 linkstat, minwidth, speed;
	int pos;

	pos = pci_find_capability(dd->pcidev, PCI_CAP_ID_EXP);
	if (!pos) {
		ipath_dev_err(dd, "Can't find PCI Express capability!\n");
		goto bail;
	}

	pci_read_config_word(dd->pcidev, pos + PCI_EXP_LNKSTA,
			     &linkstat);
	
	speed = linkstat & 0xf;
	linkstat >>= 4;
	linkstat &= 0x1f;
	dd->ipath_lbus_width = linkstat;
	switch (boardrev) {
	case 0:
	case 2:
	case 10:
	case 12:
		minwidth = 16; 
		break;
	default:
		minwidth = 8; 
		break;
	}

	switch (speed) {
	case 1:
		dd->ipath_lbus_speed = 2500; 
		break;
	case 2:
		dd->ipath_lbus_speed = 5000; 
		break;
	default: 
		dd->ipath_lbus_speed = 2500;
		break;
	}

	if (linkstat < minwidth)
		ipath_dev_err(dd,
			"PCIe width %u (x%u HCA), performance "
			"reduced\n", linkstat, minwidth);
	else
		ipath_cdbg(VERBOSE, "PCIe speed %u width %u (x%u HCA)\n",
			dd->ipath_lbus_speed, linkstat, minwidth);

	if (speed != 1)
		ipath_dev_err(dd,
			"PCIe linkspeed %u is incorrect; "
			"should be 1 (2500)!\n", speed);

bail:
	
	snprintf(dd->ipath_lbus_info, sizeof(dd->ipath_lbus_info),
		"PCIe,%uMHz,x%u\n",
		dd->ipath_lbus_speed,
		dd->ipath_lbus_width);
	return;
}



static int ipath_setup_7220_config(struct ipath_devdata *dd,
				   struct pci_dev *pdev)
{
	int pos, ret = -1;
	u32 boardrev;

	dd->ipath_msi_lo = 0;	

	pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
	if (!strcmp(int_type, "force_msi") || !strcmp(int_type, "auto"))
		ret = pci_enable_msi(pdev);
	if (ret) {
		if (!strcmp(int_type, "force_msi")) {
			ipath_dev_err(dd, "pci_enable_msi failed: %d, "
				      "force_msi is on, so not continuing.\n",
				      ret);
			return ret;
		}

		ipath_enable_intx(pdev);
		if (!strcmp(int_type, "auto"))
			ipath_dev_err(dd, "pci_enable_msi failed: %d, "
				      "falling back to INTx\n", ret);
	} else if (pos) {
		u16 control;
		pci_read_config_dword(pdev, pos + PCI_MSI_ADDRESS_LO,
				      &dd->ipath_msi_lo);
		pci_read_config_dword(pdev, pos + PCI_MSI_ADDRESS_HI,
				      &dd->ipath_msi_hi);
		pci_read_config_word(pdev, pos + PCI_MSI_FLAGS,
				     &control);
		
		pci_read_config_word(pdev,
				     pos + ((control & PCI_MSI_FLAGS_64BIT)
					    ? PCI_MSI_DATA_64 :
					    PCI_MSI_DATA_32),
				     &dd->ipath_msi_data);
	} else
		ipath_dev_err(dd, "Can't find MSI capability, "
			      "can't save MSI settings for reset\n");

	dd->ipath_irq = pdev->irq;

	
	pci_read_config_byte(pdev, PCI_CACHE_LINE_SIZE,
			     &dd->ipath_pci_cacheline);

	
	boardrev = (readq(&dd->ipath_kregbase[dd->ipath_kregs->kr_revision])
		 >> INFINIPATH_R_BOARDID_SHIFT) & INFINIPATH_R_BOARDID_MASK;

	ipath_7220_pcie_params(dd, boardrev);

	dd->ipath_flags |= IPATH_NODMA_RTAIL | IPATH_HAS_SEND_DMA |
		IPATH_HAS_PBC_CNT | IPATH_HAS_THRESH_UPDATE;
	dd->ipath_pioupd_thresh = 4U; 
	return 0;
}

static void ipath_init_7220_variables(struct ipath_devdata *dd)
{
	
	dd->ipath_kregs = &ipath_7220_kregs;
	dd->ipath_cregs = &ipath_7220_cregs;

	
	dd->ipath_gpio_sda_num = _IPATH_GPIO_SDA_NUM;
	dd->ipath_gpio_scl_num = _IPATH_GPIO_SCL_NUM;
	dd->ipath_gpio_sda = IPATH_GPIO_SDA;
	dd->ipath_gpio_scl = IPATH_GPIO_SCL;

	
	dd->ibcs_ls_shift = IBA7220_IBCS_LINKSTATE_SHIFT;
	dd->ibcs_lts_mask = IBA7220_IBCS_LINKTRAININGSTATE_MASK;
	dd->ibcs_mask = (INFINIPATH_IBCS_LINKSTATE_MASK <<
		dd->ibcs_ls_shift) | dd->ibcs_lts_mask;
	dd->ib_init = (INFINIPATH_IBCS_LT_STATE_LINKUP <<
		INFINIPATH_IBCS_LINKTRAININGSTATE_SHIFT) |
		(INFINIPATH_IBCS_L_STATE_INIT << dd->ibcs_ls_shift);
	dd->ib_arm = (INFINIPATH_IBCS_LT_STATE_LINKUP <<
		INFINIPATH_IBCS_LINKTRAININGSTATE_SHIFT) |
		(INFINIPATH_IBCS_L_STATE_ARM << dd->ibcs_ls_shift);
	dd->ib_active = (INFINIPATH_IBCS_LT_STATE_LINKUP <<
		INFINIPATH_IBCS_LINKTRAININGSTATE_SHIFT) |
		(INFINIPATH_IBCS_L_STATE_ACTIVE << dd->ibcs_ls_shift);

	
	dd->ibcc_lic_mask = IBA7220_IBCC_LINKINITCMD_MASK;
	dd->ibcc_lc_shift = IBA7220_IBCC_LINKCMD_SHIFT;
	dd->ibcc_mpl_shift = IBA7220_IBCC_MAXPKTLEN_SHIFT;

	
	dd->ipath_r_portenable_shift = INFINIPATH_R_PORTENABLE_SHIFT;
	dd->ipath_r_intravail_shift = IBA7220_R_INTRAVAIL_SHIFT;
	dd->ipath_r_tailupd_shift = IBA7220_R_TAILUPD_SHIFT;
	dd->ipath_r_portcfg_shift = IBA7220_R_PORTCFG_SHIFT;

	
	dd->ipath_hwe_bitsextant =
		(INFINIPATH_HWE_RXEMEMPARITYERR_MASK <<
		 INFINIPATH_HWE_RXEMEMPARITYERR_SHIFT) |
		(INFINIPATH_HWE_TXEMEMPARITYERR_MASK <<
		 INFINIPATH_HWE_TXEMEMPARITYERR_SHIFT) |
		(INFINIPATH_HWE_PCIEMEMPARITYERR_MASK <<
		 INFINIPATH_HWE_PCIEMEMPARITYERR_SHIFT) |
		INFINIPATH_HWE_PCIE1PLLFAILED |
		INFINIPATH_HWE_PCIE0PLLFAILED |
		INFINIPATH_HWE_PCIEPOISONEDTLP |
		INFINIPATH_HWE_PCIECPLTIMEOUT |
		INFINIPATH_HWE_PCIEBUSPARITYXTLH |
		INFINIPATH_HWE_PCIEBUSPARITYXADM |
		INFINIPATH_HWE_PCIEBUSPARITYRADM |
		INFINIPATH_HWE_MEMBISTFAILED |
		INFINIPATH_HWE_COREPLL_FBSLIP |
		INFINIPATH_HWE_COREPLL_RFSLIP |
		INFINIPATH_HWE_SERDESPLLFAILED |
		INFINIPATH_HWE_IBCBUSTOSPCPARITYERR |
		INFINIPATH_HWE_IBCBUSFRSPCPARITYERR |
		INFINIPATH_HWE_PCIECPLDATAQUEUEERR |
		INFINIPATH_HWE_PCIECPLHDRQUEUEERR |
		INFINIPATH_HWE_SDMAMEMREADERR |
		INFINIPATH_HWE_CLK_UC_PLLNOTLOCKED |
		INFINIPATH_HWE_PCIESERDESQ0PCLKNOTDETECT |
		INFINIPATH_HWE_PCIESERDESQ1PCLKNOTDETECT |
		INFINIPATH_HWE_PCIESERDESQ2PCLKNOTDETECT |
		INFINIPATH_HWE_PCIESERDESQ3PCLKNOTDETECT |
		INFINIPATH_HWE_DDSRXEQMEMORYPARITYERR |
		INFINIPATH_HWE_IB_UC_MEMORYPARITYERR |
		INFINIPATH_HWE_PCIE_UC_OCT0MEMORYPARITYERR |
		INFINIPATH_HWE_PCIE_UC_OCT1MEMORYPARITYERR;
	dd->ipath_i_bitsextant =
		INFINIPATH_I_SDMAINT | INFINIPATH_I_SDMADISABLED |
		(INFINIPATH_I_RCVURG_MASK << INFINIPATH_I_RCVURG_SHIFT) |
		(INFINIPATH_I_RCVAVAIL_MASK <<
		 INFINIPATH_I_RCVAVAIL_SHIFT) |
		INFINIPATH_I_ERROR | INFINIPATH_I_SPIOSENT |
		INFINIPATH_I_SPIOBUFAVAIL | INFINIPATH_I_GPIO |
		INFINIPATH_I_JINT | INFINIPATH_I_SERDESTRIMDONE;
	dd->ipath_e_bitsextant =
		INFINIPATH_E_RFORMATERR | INFINIPATH_E_RVCRC |
		INFINIPATH_E_RICRC | INFINIPATH_E_RMINPKTLEN |
		INFINIPATH_E_RMAXPKTLEN | INFINIPATH_E_RLONGPKTLEN |
		INFINIPATH_E_RSHORTPKTLEN | INFINIPATH_E_RUNEXPCHAR |
		INFINIPATH_E_RUNSUPVL | INFINIPATH_E_REBP |
		INFINIPATH_E_RIBFLOW | INFINIPATH_E_RBADVERSION |
		INFINIPATH_E_RRCVEGRFULL | INFINIPATH_E_RRCVHDRFULL |
		INFINIPATH_E_RBADTID | INFINIPATH_E_RHDRLEN |
		INFINIPATH_E_RHDR | INFINIPATH_E_RIBLOSTLINK |
		INFINIPATH_E_SENDSPECIALTRIGGER |
		INFINIPATH_E_SDMADISABLED | INFINIPATH_E_SMINPKTLEN |
		INFINIPATH_E_SMAXPKTLEN | INFINIPATH_E_SUNDERRUN |
		INFINIPATH_E_SPKTLEN | INFINIPATH_E_SDROPPEDSMPPKT |
		INFINIPATH_E_SDROPPEDDATAPKT |
		INFINIPATH_E_SPIOARMLAUNCH | INFINIPATH_E_SUNEXPERRPKTNUM |
		INFINIPATH_E_SUNSUPVL | INFINIPATH_E_SENDBUFMISUSE |
		INFINIPATH_E_SDMAGENMISMATCH | INFINIPATH_E_SDMAOUTOFBOUND |
		INFINIPATH_E_SDMATAILOUTOFBOUND | INFINIPATH_E_SDMABASE |
		INFINIPATH_E_SDMA1STDESC | INFINIPATH_E_SDMARPYTAG |
		INFINIPATH_E_SDMADWEN | INFINIPATH_E_SDMAMISSINGDW |
		INFINIPATH_E_SDMAUNEXPDATA |
		INFINIPATH_E_IBSTATUSCHANGED | INFINIPATH_E_INVALIDADDR |
		INFINIPATH_E_RESET | INFINIPATH_E_HARDWARE |
		INFINIPATH_E_SDMADESCADDRMISALIGN |
		INFINIPATH_E_INVALIDEEPCMD;

	dd->ipath_i_rcvavail_mask = INFINIPATH_I_RCVAVAIL_MASK;
	dd->ipath_i_rcvurg_mask = INFINIPATH_I_RCVURG_MASK;
	dd->ipath_i_rcvavail_shift = INFINIPATH_I_RCVAVAIL_SHIFT;
	dd->ipath_i_rcvurg_shift = INFINIPATH_I_RCVURG_SHIFT;
	dd->ipath_flags |= IPATH_INTREG_64 | IPATH_HAS_MULT_IB_SPEED
		| IPATH_HAS_LINK_LATENCY;

	
	dd->ipath_eep_st_masks[0].hwerrs_to_log =
		INFINIPATH_HWE_TXEMEMPARITYERR_MASK <<
		INFINIPATH_HWE_TXEMEMPARITYERR_SHIFT;

	dd->ipath_eep_st_masks[1].hwerrs_to_log =
		INFINIPATH_HWE_RXEMEMPARITYERR_MASK <<
		INFINIPATH_HWE_RXEMEMPARITYERR_SHIFT;

	dd->ipath_eep_st_masks[2].errs_to_log = INFINIPATH_E_RESET;

	ipath_linkrecovery = 0;

	init_waitqueue_head(&dd->ipath_autoneg_wait);
	INIT_DELAYED_WORK(&dd->ipath_autoneg_work,  autoneg_work);

	dd->ipath_link_width_supported = IB_WIDTH_1X | IB_WIDTH_4X;
	dd->ipath_link_speed_supported = IPATH_IB_SDR | IPATH_IB_DDR;

	dd->ipath_link_width_enabled = dd->ipath_link_width_supported;
	dd->ipath_link_speed_enabled = dd->ipath_link_speed_supported;
	
	dd->ipath_link_width_active = IB_WIDTH_4X;
	dd->ipath_link_speed_active = IPATH_IB_SDR;
	dd->delay_mult = rate_to_delay[0][1];
}



static int ipath_reinit_msi(struct ipath_devdata *dd)
{
	int ret = 0;

	int pos;
	u16 control;
	if (!dd->ipath_msi_lo) 
		goto bail;

	pos = pci_find_capability(dd->pcidev, PCI_CAP_ID_MSI);
	if (!pos) {
		ipath_dev_err(dd, "Can't find MSI capability, "
			      "can't restore MSI settings\n");
		goto bail;
	}
	ipath_cdbg(VERBOSE, "Writing msi_lo 0x%x to config offset 0x%x\n",
		   dd->ipath_msi_lo, pos + PCI_MSI_ADDRESS_LO);
	pci_write_config_dword(dd->pcidev, pos + PCI_MSI_ADDRESS_LO,
			       dd->ipath_msi_lo);
	ipath_cdbg(VERBOSE, "Writing msi_lo 0x%x to config offset 0x%x\n",
		   dd->ipath_msi_hi, pos + PCI_MSI_ADDRESS_HI);
	pci_write_config_dword(dd->pcidev, pos + PCI_MSI_ADDRESS_HI,
			       dd->ipath_msi_hi);
	pci_read_config_word(dd->pcidev, pos + PCI_MSI_FLAGS, &control);
	if (!(control & PCI_MSI_FLAGS_ENABLE)) {
		ipath_cdbg(VERBOSE, "MSI control at off %x was %x, "
			   "setting MSI enable (%x)\n", pos + PCI_MSI_FLAGS,
			   control, control | PCI_MSI_FLAGS_ENABLE);
		control |= PCI_MSI_FLAGS_ENABLE;
		pci_write_config_word(dd->pcidev, pos + PCI_MSI_FLAGS,
				      control);
	}
	
	pci_write_config_word(dd->pcidev, pos +
			      ((control & PCI_MSI_FLAGS_64BIT) ? 12 : 8),
			      dd->ipath_msi_data);
	ret = 1;

bail:
	if (!ret) {
		ipath_dbg("Using INTx, MSI disabled or not configured\n");
		ipath_enable_intx(dd->pcidev);
		ret = 1;
	}
	
	pci_write_config_byte(dd->pcidev, PCI_CACHE_LINE_SIZE,
			      dd->ipath_pci_cacheline);
	
	pci_set_master(dd->pcidev);

	return ret;
}


static int ipath_setup_7220_reset(struct ipath_devdata *dd)
{
	u64 val;
	int i;
	int ret;
	u16 cmdval;

	pci_read_config_word(dd->pcidev, PCI_COMMAND, &cmdval);

	
	ipath_dev_err(dd, "Resetting InfiniPath unit %u\n", dd->ipath_unit);

	
	dd->ipath_flags &= ~(IPATH_INITTED | IPATH_PRESENT);
	val = dd->ipath_control | INFINIPATH_C_RESET;
	ipath_write_kreg(dd, dd->ipath_kregs->kr_control, val);
	mb();

	for (i = 1; i <= 5; i++) {
		int r;

		
		msleep(1000 + (1 + i) * 2000);
		r = pci_write_config_dword(dd->pcidev, PCI_BASE_ADDRESS_0,
					   dd->ipath_pcibar0);
		if (r)
			ipath_dev_err(dd, "rewrite of BAR0 failed: %d\n", r);
		r = pci_write_config_dword(dd->pcidev, PCI_BASE_ADDRESS_1,
					   dd->ipath_pcibar1);
		if (r)
			ipath_dev_err(dd, "rewrite of BAR1 failed: %d\n", r);
		
		pci_write_config_word(dd->pcidev, PCI_COMMAND, cmdval);
		r = pci_enable_device(dd->pcidev);
		if (r)
			ipath_dev_err(dd, "pci_enable_device failed after "
				      "reset: %d\n", r);
		
		dd->ipath_flags |= IPATH_PRESENT;
		val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_revision);
		if (val == dd->ipath_revision) {
			ipath_cdbg(VERBOSE, "Got matching revision "
				   "register %llx on try %d\n",
				   (unsigned long long) val, i);
			ret = ipath_reinit_msi(dd);
			goto bail;
		}
		
		ipath_dbg("Didn't get expected revision register, "
			  "got %llx, try %d\n", (unsigned long long) val,
			  i + 1);
	}
	ret = 0; 

bail:
	if (ret)
		ipath_7220_pcie_params(dd, dd->ipath_boardrev);

	return ret;
}


static void ipath_7220_put_tid(struct ipath_devdata *dd, u64 __iomem *tidptr,
			     u32 type, unsigned long pa)
{
	if (pa != dd->ipath_tidinvalid) {
		u64 chippa = pa >> IBA7220_TID_PA_SHIFT;

		
		if (pa != (chippa << IBA7220_TID_PA_SHIFT)) {
			dev_info(&dd->pcidev->dev, "BUG: physaddr %lx "
				 "not 2KB aligned!\n", pa);
			return;
		}
		if (chippa >= (1UL << IBA7220_TID_SZ_SHIFT)) {
			ipath_dev_err(dd,
				      "BUG: Physical page address 0x%lx "
				      "larger than supported\n", pa);
			return;
		}

		if (type == RCVHQ_RCV_TYPE_EAGER)
			chippa |= dd->ipath_tidtemplate;
		else 
			chippa |= IBA7220_TID_SZ_4K;
		writeq(chippa, tidptr);
	} else
		writeq(pa, tidptr);
	mmiowb();
}


static void ipath_7220_clear_tids(struct ipath_devdata *dd, unsigned port)
{
	u64 __iomem *tidbase;
	unsigned long tidinv;
	int i;

	if (!dd->ipath_kregbase)
		return;

	ipath_cdbg(VERBOSE, "Invalidate TIDs for port %u\n", port);

	tidinv = dd->ipath_tidinvalid;
	tidbase = (u64 __iomem *)
		((char __iomem *)(dd->ipath_kregbase) +
		 dd->ipath_rcvtidbase +
		 port * dd->ipath_rcvtidcnt * sizeof(*tidbase));

	for (i = 0; i < dd->ipath_rcvtidcnt; i++)
		ipath_7220_put_tid(dd, &tidbase[i], RCVHQ_RCV_TYPE_EXPECTED,
				   tidinv);

	tidbase = (u64 __iomem *)
		((char __iomem *)(dd->ipath_kregbase) +
		 dd->ipath_rcvegrbase + port_egrtid_idx(dd, port)
		 * sizeof(*tidbase));

	for (i = port ? dd->ipath_rcvegrcnt : dd->ipath_p0_rcvegrcnt; i; i--)
		ipath_7220_put_tid(dd, &tidbase[i-1], RCVHQ_RCV_TYPE_EAGER,
			tidinv);
}


static void ipath_7220_tidtemplate(struct ipath_devdata *dd)
{
	
	if (dd->ipath_rcvegrbufsize == 2048)
		dd->ipath_tidtemplate = IBA7220_TID_SZ_2K;
	else if (dd->ipath_rcvegrbufsize == 4096)
		dd->ipath_tidtemplate = IBA7220_TID_SZ_4K;
	else {
		dev_info(&dd->pcidev->dev, "BUG: unsupported egrbufsize "
			 "%u, using %u\n", dd->ipath_rcvegrbufsize,
			 4096);
		dd->ipath_tidtemplate = IBA7220_TID_SZ_4K;
	}
	dd->ipath_tidinvalid = 0;
}

static int ipath_7220_early_init(struct ipath_devdata *dd)
{
	u32 i, s;

	if (strcmp(int_type, "auto") &&
	    strcmp(int_type, "force_msi") &&
	    strcmp(int_type, "force_intx")) {
		ipath_dev_err(dd, "Invalid interrupt_type: '%s', expecting "
			      "auto, force_msi or force_intx\n", int_type);
		return -EINVAL;
	}

	
	if (ipath_sdma_fetch_arb && (dd->ipath_minrev > 1))
		dd->ipath_control |= 1<<4;

	dd->ipath_flags |= IPATH_4BYTE_TID;

	
	dd->ipath_rcvhdrentsize = 24;
	dd->ipath_rcvhdrsize = IPATH_DFLT_RCVHDRSIZE;
	dd->ipath_rhf_offset =
		dd->ipath_rcvhdrentsize - sizeof(u64) / sizeof(u32);

	dd->ipath_rcvegrbufsize = ipath_mtu4096 ? 4096 : 2048;
	
	dd->ipath_ibmaxlen = min(ipath_mtu4096 ? dd->ipath_piosize4k :
				 dd->ipath_piosize2k,
				 dd->ipath_rcvegrbufsize +
				 (dd->ipath_rcvhdrentsize << 2));
	dd->ipath_init_ibmaxlen = dd->ipath_ibmaxlen;

	ipath_7220_config_jint(dd, INFINIPATH_JINT_DEFAULT_IDLE_TICKS,
			       INFINIPATH_JINT_DEFAULT_MAX_PACKETS);

	if (dd->ipath_boardrev) 
		ipath_get_eeprom_info(dd);

	
	s = ipath_read_kreg32(dd, IPATH_KREG_OFFSET(ProcMon));
	s &= ~(1U<<31); 
	s |= 1U<<14; 
	ipath_write_kreg(dd, IPATH_KREG_OFFSET(ProcMon), s);
	
	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);

	s &= ~(1U<<14); 
	ipath_write_kreg(dd, IPATH_KREG_OFFSET(ProcMon), s);
	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
	s = ipath_read_kreg32(dd, IPATH_KREG_OFFSET(ProcMon));

	s |= 1U<<15; 
	s &= ~(1U<<31); 
	s &= ~0x7ffU; 
	s |= 0xe29; 
	ipath_write_kreg(dd, IPATH_KREG_OFFSET(ProcMon), s);

	s = 0;
	for (i = 500; i > 0 && !(s&(1ULL<<31)); i--) {
		ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
		s = ipath_read_kreg32(dd, IPATH_KREG_OFFSET(ProcMon));
	}
	if (!(s&(1U<<31)))
		ipath_dev_err(dd, "ProcMon register not valid: 0x%x\n", s);
	else
		ipath_dbg("ProcMon=0x%x, count=0x%x\n", s, (s>>16)&0x1ff);

	return 0;
}


static int ipath_7220_get_base_info(struct ipath_portdata *pd, void *kbase)
{
	struct ipath_base_info *kinfo = kbase;

	kinfo->spi_runtime_flags |=
		IPATH_RUNTIME_PCIE | IPATH_RUNTIME_NODMA_RTAIL |
		IPATH_RUNTIME_SDMA;

	return 0;
}

static void ipath_7220_free_irq(struct ipath_devdata *dd)
{
	free_irq(dd->ipath_irq, dd);
	dd->ipath_irq = 0;
}

static struct ipath_message_header *
ipath_7220_get_msgheader(struct ipath_devdata *dd, __le32 *rhf_addr)
{
	u32 offset = ipath_hdrget_offset(rhf_addr);

	return (struct ipath_message_header *)
		(rhf_addr - dd->ipath_rhf_offset + offset);
}

static void ipath_7220_config_ports(struct ipath_devdata *dd, ushort cfgports)
{
	u32 nchipports;

	nchipports = ipath_read_kreg32(dd, dd->ipath_kregs->kr_portcnt);
	if (!cfgports) {
		int ncpus = num_online_cpus();

		if (ncpus <= 4)
			dd->ipath_portcnt = 5;
		else if (ncpus <= 8)
			dd->ipath_portcnt = 9;
		if (dd->ipath_portcnt)
			ipath_dbg("Auto-configured for %u ports, %d cpus "
				"online\n", dd->ipath_portcnt, ncpus);
	} else if (cfgports <= nchipports)
		dd->ipath_portcnt = cfgports;
	if (!dd->ipath_portcnt) 
		dd->ipath_portcnt = nchipports;
	
	if (dd->ipath_portcnt > 9)
		dd->ipath_rcvctrl |= 2ULL << IBA7220_R_PORTCFG_SHIFT;
	else if (dd->ipath_portcnt > 5)
		dd->ipath_rcvctrl |= 1ULL << IBA7220_R_PORTCFG_SHIFT;
	
	ipath_write_kreg(dd, dd->ipath_kregs->kr_rcvctrl,
			 dd->ipath_rcvctrl);
	dd->ipath_p0_rcvegrcnt = 2048; 
	if (dd->ipath_flags & IPATH_HAS_SEND_DMA)
		dd->ipath_pioreserved = 3; 
}


static int ipath_7220_get_ib_cfg(struct ipath_devdata *dd, int which)
{
	int lsb, ret = 0;
	u64 maskr; 

	switch (which) {
	case IPATH_IB_CFG_HRTBT: 
		lsb = IBA7220_IBC_HRTBT_SHIFT;
		maskr = IBA7220_IBC_HRTBT_MASK;
		break;

	case IPATH_IB_CFG_LWID_ENB: 
		ret = dd->ipath_link_width_enabled;
		goto done;

	case IPATH_IB_CFG_LWID: 
		ret = dd->ipath_link_width_active;
		goto done;

	case IPATH_IB_CFG_SPD_ENB: 
		ret = dd->ipath_link_speed_enabled;
		goto done;

	case IPATH_IB_CFG_SPD: 
		ret = dd->ipath_link_speed_active;
		goto done;

	case IPATH_IB_CFG_RXPOL_ENB: 
		lsb = IBA7220_IBC_RXPOL_SHIFT;
		maskr = IBA7220_IBC_RXPOL_MASK;
		break;

	case IPATH_IB_CFG_LREV_ENB: 
		lsb = IBA7220_IBC_LREV_SHIFT;
		maskr = IBA7220_IBC_LREV_MASK;
		break;

	case IPATH_IB_CFG_LINKLATENCY:
		ret = ipath_read_kreg64(dd, dd->ipath_kregs->kr_ibcddrstatus)
			& IBA7220_DDRSTAT_LINKLAT_MASK;
		goto done;

	default:
		ret = -ENOTSUPP;
		goto done;
	}
	ret = (int)((dd->ipath_ibcddrctrl >> lsb) & maskr);
done:
	return ret;
}

static int ipath_7220_set_ib_cfg(struct ipath_devdata *dd, int which, u32 val)
{
	int lsb, ret = 0, setforce = 0;
	u64 maskr; 

	switch (which) {
	case IPATH_IB_CFG_LIDLMC:
		
		lsb = IBA7220_IBC_DLIDLMC_SHIFT;
		maskr = IBA7220_IBC_DLIDLMC_MASK;
		break;

	case IPATH_IB_CFG_HRTBT: 
		if (val & IPATH_IB_HRTBT_ON &&
			(dd->ipath_flags & IPATH_NO_HRTBT))
			goto bail;
		lsb = IBA7220_IBC_HRTBT_SHIFT;
		maskr = IBA7220_IBC_HRTBT_MASK;
		break;

	case IPATH_IB_CFG_LWID_ENB: 
		
		dd->ipath_link_width_enabled = val;
		if ((dd->ipath_flags & (IPATH_LINKDOWN|IPATH_LINKINIT)) !=
			IPATH_LINKDOWN)
			goto bail;
		
		val--; 
		maskr = IBA7220_IBC_WIDTH_MASK;
		lsb = IBA7220_IBC_WIDTH_SHIFT;
		setforce = 1;
		dd->ipath_flags |= IPATH_IB_FORCE_NOTIFY;
		break;

	case IPATH_IB_CFG_SPD_ENB: 
		
		dd->ipath_link_speed_enabled = val;
		if (dd->ipath_ibcddrctrl & IBA7220_IBC_IBTA_1_2_MASK &&
		    !(val & (val - 1)))
			dd->ipath_presets_needed = 1;
		if ((dd->ipath_flags & (IPATH_LINKDOWN|IPATH_LINKINIT)) !=
			IPATH_LINKDOWN)
			goto bail;
		
		if (val == (IPATH_IB_SDR | IPATH_IB_DDR)) {
			val = IBA7220_IBC_SPEED_AUTONEG_MASK |
				IBA7220_IBC_IBTA_1_2_MASK;
			dd->ipath_flags &= ~IPATH_IB_AUTONEG_FAILED;
		} else
			val = val == IPATH_IB_DDR ?  IBA7220_IBC_SPEED_DDR
				: IBA7220_IBC_SPEED_SDR;
		maskr = IBA7220_IBC_SPEED_AUTONEG_MASK |
			IBA7220_IBC_IBTA_1_2_MASK;
		lsb = 0; 
		setforce = 1;
		break;

	case IPATH_IB_CFG_RXPOL_ENB: 
		lsb = IBA7220_IBC_RXPOL_SHIFT;
		maskr = IBA7220_IBC_RXPOL_MASK;
		break;

	case IPATH_IB_CFG_LREV_ENB: 
		lsb = IBA7220_IBC_LREV_SHIFT;
		maskr = IBA7220_IBC_LREV_MASK;
		break;

	default:
		ret = -ENOTSUPP;
		goto bail;
	}
	dd->ipath_ibcddrctrl &= ~(maskr << lsb);
	dd->ipath_ibcddrctrl |= (((u64) val & maskr) << lsb);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_ibcddrctrl,
			 dd->ipath_ibcddrctrl);
	if (setforce)
		dd->ipath_flags |= IPATH_IB_FORCE_NOTIFY;
bail:
	return ret;
}

static void ipath_7220_read_counters(struct ipath_devdata *dd,
				     struct infinipath_counters *cntrs)
{
	u64 *counters = (u64 *) cntrs;
	int i;

	for (i = 0; i < sizeof(*cntrs) / sizeof(u64); i++)
		counters[i] = ipath_snap_cntr(dd, i);
}


static int ipath_7220_intr_fallback(struct ipath_devdata *dd)
{
	if (dd->ipath_msi_lo) {
		dev_info(&dd->pcidev->dev, "MSI interrupt not detected,"
			" trying INTx interrupts\n");
		ipath_7220_nomsi(dd);
		ipath_enable_intx(dd->pcidev);
		
		dd->ipath_irq = dd->pcidev->irq;
		if (request_irq(dd->ipath_irq, ipath_intr, IRQF_SHARED,
			IPATH_DRV_NAME, dd))
			ipath_dev_err(dd,
				"Could not re-request_irq for INTx\n");
		return 1;
	}
	return 0;
}


static void ipath_7220_xgxs_reset(struct ipath_devdata *dd)
{
	u64 val, prev_val;

	prev_val = ipath_read_kreg64(dd, dd->ipath_kregs->kr_xgxsconfig);
	val = prev_val | INFINIPATH_XGXS_RESET;
	prev_val &= ~INFINIPATH_XGXS_RESET; 
	ipath_write_kreg(dd, dd->ipath_kregs->kr_control,
			 dd->ipath_control & ~INFINIPATH_C_LINKENABLE);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_xgxsconfig, val);
	ipath_read_kreg32(dd, dd->ipath_kregs->kr_scratch);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_xgxsconfig, prev_val);
	ipath_write_kreg(dd, dd->ipath_kregs->kr_control,
			 dd->ipath_control);
}



static void autoneg_send(struct ipath_devdata *dd,
	u32 *hdr, u32 dcnt, u32 *data)
{
	int i;
	u64 cnt;
	u32 __iomem *piobuf;
	u32 pnum;

	i = 0;
	cnt = 7 + dcnt + 1; 
	while (!(piobuf = ipath_getpiobuf(dd, cnt, &pnum))) {
		if (i++ > 15) {
			ipath_dbg("Couldn't get pio buffer for send\n");
			return;
		}
		udelay(2);
	}
	if (dd->ipath_flags&IPATH_HAS_PBC_CNT)
		cnt |= 0x80000000UL<<32; 
	writeq(cnt, piobuf);
	ipath_flush_wc();
	__iowrite32_copy(piobuf + 2, hdr, 7);
	__iowrite32_copy(piobuf + 9, data, dcnt);
	ipath_flush_wc();
}


static void ipath_autoneg_send(struct ipath_devdata *dd, int which)
{
	static u32 swapped;
	u32 dw, i, hcnt, dcnt, *data;
	static u32 hdr[7] = { 0xf002ffff, 0x48ffff, 0x6400abba };
	static u32 madpayload_start[0x40] = {
		0x1810103, 0x1, 0x0, 0x0, 0x2c90000, 0x2c9, 0x0, 0x0,
		0xffffffff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x1, 0x1388, 0x15e, 0x1, 
		};
	static u32 madpayload_done[0x40] = {
		0x1810103, 0x1, 0x0, 0x0, 0x2c90000, 0x2c9, 0x0, 0x0,
		0xffffffff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x40000001, 0x1388, 0x15e, 
		};
	dcnt = ARRAY_SIZE(madpayload_start);
	hcnt = ARRAY_SIZE(hdr);
	if (!swapped) {
		
		for (i = 0; i < hcnt; i++) {
			dw = (__force u32) cpu_to_be32(hdr[i]);
			hdr[i] = dw;
		}
		for (i = 0; i < dcnt; i++) {
			dw = (__force u32) cpu_to_be32(madpayload_start[i]);
			madpayload_start[i] = dw;
			dw = (__force u32) cpu_to_be32(madpayload_done[i]);
			madpayload_done[i] = dw;
		}
		swapped = 1;
	}

	data = which ? madpayload_done : madpayload_start;
	ipath_cdbg(PKT, "Sending %s special MADs\n", which?"done":"start");

	autoneg_send(dd, hdr, dcnt, data);
	ipath_read_kreg64(dd, dd->ipath_kregs->kr_scratch);
	udelay(2);
	autoneg_send(dd, hdr, dcnt, data);
	ipath_read_kreg64(dd, dd->ipath_kregs->kr_scratch);
	udelay(2);
}




static void set_speed_fast(struct ipath_devdata *dd, u32 speed)
{
	dd->ipath_ibcddrctrl &= ~(IBA7220_IBC_SPEED_AUTONEG_MASK |
		IBA7220_IBC_IBTA_1_2_MASK |
		(IBA7220_IBC_WIDTH_MASK << IBA7220_IBC_WIDTH_SHIFT));

	if (speed == (IPATH_IB_SDR | IPATH_IB_DDR))
		dd->ipath_ibcddrctrl |= IBA7220_IBC_SPEED_AUTONEG_MASK |
			IBA7220_IBC_IBTA_1_2_MASK;
	else
		dd->ipath_ibcddrctrl |= speed == IPATH_IB_DDR ?
			IBA7220_IBC_SPEED_DDR : IBA7220_IBC_SPEED_SDR;

	
	dd->ipath_ibcddrctrl |= (u64)(dd->ipath_link_width_enabled - 1) <<
		IBA7220_IBC_WIDTH_SHIFT;
	ipath_write_kreg(dd, dd->ipath_kregs->kr_ibcddrctrl,
			dd->ipath_ibcddrctrl);
	ipath_cdbg(VERBOSE, "setup for IB speed (%x) done\n", speed);
}



static void try_auto_neg(struct ipath_devdata *dd)
{
	
	ipath_write_kreg(dd, IPATH_KREG_OFFSET(IBNCModeCtrl),
		0x3b9dc07);
	dd->ipath_flags |= IPATH_IB_AUTONEG_INPROG;
	ipath_autoneg_send(dd, 0);
	set_speed_fast(dd, IPATH_IB_DDR);
	ipath_toggle_rclkrls(dd);
	
	schedule_delayed_work(&dd->ipath_autoneg_work,
		msecs_to_jiffies(2));
}


static int ipath_7220_ib_updown(struct ipath_devdata *dd, int ibup, u64 ibcs)
{
	int ret = 0, symadj = 0;
	u32 ltstate = ipath_ib_linkstate(dd, ibcs);

	dd->ipath_link_width_active =
		((ibcs >> IBA7220_IBCS_LINKWIDTH_SHIFT) & 1) ?
		    IB_WIDTH_4X : IB_WIDTH_1X;
	dd->ipath_link_speed_active =
		((ibcs >> IBA7220_IBCS_LINKSPEED_SHIFT) & 1) ?
		    IPATH_IB_DDR : IPATH_IB_SDR;

	if (!ibup) {
		
		if (dd->ipath_x1_fix_tries &&
			 ltstate <= INFINIPATH_IBCS_LT_STATE_SLEEPQUIET &&
			ltstate != INFINIPATH_IBCS_LT_STATE_LINKUP)
			dd->ipath_x1_fix_tries = 0;
		if (!(dd->ipath_flags & (IPATH_IB_AUTONEG_FAILED |
			IPATH_IB_AUTONEG_INPROG)))
			set_speed_fast(dd, dd->ipath_link_speed_enabled);
		if (!(dd->ipath_flags & IPATH_IB_AUTONEG_INPROG)) {
			ipath_cdbg(VERBOSE, "Setting RXEQ defaults\n");
			ipath_sd7220_presets(dd);
		}
		
		ipath_set_relock_poll(dd, ibup);
	} else {
		if (ipath_compat_ddr_negotiate &&
		    !(dd->ipath_flags & (IPATH_IB_AUTONEG_FAILED |
			IPATH_IB_AUTONEG_INPROG)) &&
			dd->ipath_link_speed_active == IPATH_IB_SDR &&
			(dd->ipath_link_speed_enabled &
			    (IPATH_IB_DDR | IPATH_IB_SDR)) ==
			    (IPATH_IB_DDR | IPATH_IB_SDR) &&
			dd->ipath_autoneg_tries < IPATH_AUTONEG_TRIES) {
			
			++dd->ipath_autoneg_tries;
			ipath_dbg("DDR negotiation try, %u/%u\n",
				dd->ipath_autoneg_tries,
				IPATH_AUTONEG_TRIES);
			if (!dd->ibdeltainprog) {
				dd->ibdeltainprog = 1;
				dd->ibsymsnap = ipath_read_creg32(dd,
					dd->ipath_cregs->cr_ibsymbolerrcnt);
				dd->iblnkerrsnap = ipath_read_creg32(dd,
					dd->ipath_cregs->cr_iblinkerrrecovcnt);
			}
			try_auto_neg(dd);
			ret = 1; 
		} else if ((dd->ipath_flags & IPATH_IB_AUTONEG_INPROG)
			&& dd->ipath_link_speed_active == IPATH_IB_SDR) {
			ipath_autoneg_send(dd, 1);
			set_speed_fast(dd, IPATH_IB_DDR);
			udelay(2);
			ipath_toggle_rclkrls(dd);
			ret = 1; 
		} else {
			if ((dd->ipath_flags & IPATH_IB_AUTONEG_INPROG) &&
				(dd->ipath_link_speed_active & IPATH_IB_DDR)) {
				ipath_dbg("Got to INIT with DDR autoneg\n");
				dd->ipath_flags &= ~(IPATH_IB_AUTONEG_INPROG
					| IPATH_IB_AUTONEG_FAILED);
				dd->ipath_autoneg_tries = 0;
				
				set_speed_fast(dd,
					dd->ipath_link_speed_enabled);
				wake_up(&dd->ipath_autoneg_wait);
				symadj = 1;
			} else if (dd->ipath_flags & IPATH_IB_AUTONEG_FAILED) {
				
				ipath_dbg("INIT %sDR after autoneg failure\n",
					(dd->ipath_link_speed_active &
					  IPATH_IB_DDR) ? "D" : "S");
				dd->ipath_flags &= ~IPATH_IB_AUTONEG_FAILED;
				dd->ipath_ibcddrctrl |=
					IBA7220_IBC_IBTA_1_2_MASK;
				ipath_write_kreg(dd,
					IPATH_KREG_OFFSET(IBNCModeCtrl), 0);
				symadj = 1;
			}
		}
		
		if (dd->ipath_minrev == 1 &&
		    (dd->ipath_link_width_enabled & (IB_WIDTH_1X |
			IB_WIDTH_4X)) == (IB_WIDTH_1X | IB_WIDTH_4X)
			&& dd->ipath_link_width_active == IB_WIDTH_1X
			&& dd->ipath_x1_fix_tries < 3) {
		     if (++dd->ipath_x1_fix_tries == 3) {
				dev_info(&dd->pcidev->dev,
					"IB link is in 1X mode\n");
				if (!(dd->ipath_flags &
				      IPATH_IB_AUTONEG_INPROG))
					symadj = 1;
		     }
			else {
				ipath_cdbg(VERBOSE, "IB 1X in "
					"auto-width, try %u to be "
					"sure it's really 1X; "
					"ltstate %u\n",
					 dd->ipath_x1_fix_tries,
					 ltstate);
				dd->ipath_f_xgxs_reset(dd);
				ret = 1; 
			}
		} else if (!(dd->ipath_flags & IPATH_IB_AUTONEG_INPROG))
			symadj = 1;

		if (!ret) {
			dd->delay_mult = rate_to_delay
			    [(ibcs >> IBA7220_IBCS_LINKSPEED_SHIFT) & 1]
			    [(ibcs >> IBA7220_IBCS_LINKWIDTH_SHIFT) & 1];

			ipath_set_relock_poll(dd, ibup);
		}
	}

	if (symadj) {
		if (dd->ibdeltainprog) {
			dd->ibdeltainprog = 0;
			dd->ibsymdelta += ipath_read_creg32(dd,
				dd->ipath_cregs->cr_ibsymbolerrcnt) -
				dd->ibsymsnap;
			dd->iblnkerrdelta += ipath_read_creg32(dd,
				dd->ipath_cregs->cr_iblinkerrrecovcnt) -
				dd->iblnkerrsnap;
		}
	} else if (!ibup && !dd->ibdeltainprog
		   && !(dd->ipath_flags & IPATH_IB_AUTONEG_INPROG)) {
		dd->ibdeltainprog = 1;
		dd->ibsymsnap =	ipath_read_creg32(dd,
				     dd->ipath_cregs->cr_ibsymbolerrcnt);
		dd->iblnkerrsnap = ipath_read_creg32(dd,
				     dd->ipath_cregs->cr_iblinkerrrecovcnt);
	}

	if (!ret)
		ipath_setup_7220_setextled(dd, ipath_ib_linkstate(dd, ibcs),
			ltstate);
	return ret;
}



static void autoneg_work(struct work_struct *work)
{
	struct ipath_devdata *dd;
	u64 startms;
	u32 lastlts, i;

	dd = container_of(work, struct ipath_devdata,
		ipath_autoneg_work.work);

	startms = jiffies_to_msecs(jiffies);

	
	for (i = 0; i < 25; i++) {
		lastlts = ipath_ib_linktrstate(dd, dd->ipath_lastibcstat);
		if (lastlts == INFINIPATH_IBCS_LT_STATE_POLLQUIET) {
			ipath_set_linkstate(dd, IPATH_IB_LINKDOWN_DISABLE);
			break;
		}
		udelay(100);
	}

	if (!(dd->ipath_flags & IPATH_IB_AUTONEG_INPROG))
		goto done; 

	
	if (wait_event_timeout(dd->ipath_autoneg_wait,
		!(dd->ipath_flags & IPATH_IB_AUTONEG_INPROG),
		msecs_to_jiffies(90)))
		goto done;

	ipath_toggle_rclkrls(dd);

	
	if (wait_event_timeout(dd->ipath_autoneg_wait,
		!(dd->ipath_flags & IPATH_IB_AUTONEG_INPROG),
		msecs_to_jiffies(1700)))
		goto done;

	set_speed_fast(dd, IPATH_IB_SDR);
	ipath_toggle_rclkrls(dd);

	
	wait_event_timeout(dd->ipath_autoneg_wait,
		!(dd->ipath_flags & IPATH_IB_AUTONEG_INPROG),
		msecs_to_jiffies(250));
done:
	if (dd->ipath_flags & IPATH_IB_AUTONEG_INPROG) {
		ipath_dbg("Did not get to DDR INIT (%x) after %Lu msecs\n",
			ipath_ib_state(dd, dd->ipath_lastibcstat),
			(unsigned long long) jiffies_to_msecs(jiffies)-startms);
		dd->ipath_flags &= ~IPATH_IB_AUTONEG_INPROG;
		if (dd->ipath_autoneg_tries == IPATH_AUTONEG_TRIES) {
			dd->ipath_flags |= IPATH_IB_AUTONEG_FAILED;
			ipath_dbg("Giving up on DDR until next IB "
				"link Down\n");
			dd->ipath_autoneg_tries = 0;
		}
		set_speed_fast(dd, dd->ipath_link_speed_enabled);
	}
}



void ipath_init_iba7220_funcs(struct ipath_devdata *dd)
{
	dd->ipath_f_intrsetup = ipath_7220_intconfig;
	dd->ipath_f_bus = ipath_setup_7220_config;
	dd->ipath_f_reset = ipath_setup_7220_reset;
	dd->ipath_f_get_boardname = ipath_7220_boardname;
	dd->ipath_f_init_hwerrors = ipath_7220_init_hwerrors;
	dd->ipath_f_early_init = ipath_7220_early_init;
	dd->ipath_f_handle_hwerrors = ipath_7220_handle_hwerrors;
	dd->ipath_f_quiet_serdes = ipath_7220_quiet_serdes;
	dd->ipath_f_bringup_serdes = ipath_7220_bringup_serdes;
	dd->ipath_f_clear_tids = ipath_7220_clear_tids;
	dd->ipath_f_put_tid = ipath_7220_put_tid;
	dd->ipath_f_cleanup = ipath_setup_7220_cleanup;
	dd->ipath_f_setextled = ipath_setup_7220_setextled;
	dd->ipath_f_get_base_info = ipath_7220_get_base_info;
	dd->ipath_f_free_irq = ipath_7220_free_irq;
	dd->ipath_f_tidtemplate = ipath_7220_tidtemplate;
	dd->ipath_f_intr_fallback = ipath_7220_intr_fallback;
	dd->ipath_f_xgxs_reset = ipath_7220_xgxs_reset;
	dd->ipath_f_get_ib_cfg = ipath_7220_get_ib_cfg;
	dd->ipath_f_set_ib_cfg = ipath_7220_set_ib_cfg;
	dd->ipath_f_config_jint = ipath_7220_config_jint;
	dd->ipath_f_config_ports = ipath_7220_config_ports;
	dd->ipath_f_read_counters = ipath_7220_read_counters;
	dd->ipath_f_get_msgheader = ipath_7220_get_msgheader;
	dd->ipath_f_ib_updown = ipath_7220_ib_updown;

	
	ipath_init_7220_variables(dd);
}
