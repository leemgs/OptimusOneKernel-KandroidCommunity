

#ifndef _STG4000REG_H
#define _STG4000REG_H

#define DWFILL unsigned long :32
#define WFILL unsigned short :16


#if defined(__KERNEL__)
#include <asm/page.h>
#include <asm/io.h>
#define STG_WRITE_REG(reg,data) (writel(data,&pSTGReg->reg))
#define STG_READ_REG(reg)      (readl(&pSTGReg->reg))
#else
#define STG_WRITE_REG(reg,data) (pSTGReg->reg = data)
#define STG_READ_REG(reg)      (pSTGReg->reg)
#endif 

#define SET_BIT(n) (1<<(n))
#define CLEAR_BIT(n) (tmp &= ~(1<<n))
#define CLEAR_BITS_FRM_TO(frm, to) \
{\
int i; \
    for(i = frm; i<= to; i++) \
	{ \
	    tmp &= ~(1<<i); \
	} \
}

#define CLEAR_BIT_2(n) (usTemp &= ~(1<<n))
#define CLEAR_BITS_FRM_TO_2(frm, to) \
{\
int i; \
    for(i = frm; i<= to; i++) \
	{ \
	    usTemp &= ~(1<<i); \
	} \
}


typedef enum _LUT_USES {
	NO_LUT = 0, RESERVED, GRAPHICS, OVERLAY
} LUT_USES;


typedef enum _PIXEL_FORMAT {
	_8BPP = 0, _15BPP, _16BPP, _24BPP, _32BPP
} PIXEL_FORMAT;


typedef enum _BLEND_MODE {
	GRAPHICS_MODE = 0, COLOR_KEY, PER_PIXEL_ALPHA, GLOBAL_ALPHA,
	CK_PIXEL_ALPHA, CK_GLOBAL_ALPHA
} OVRL_BLEND_MODE;


typedef enum _OVRL_PIX_FORMAT {
	UYVY, VYUY, YUYV, YVYU
} OVRL_PIX_FORMAT;


typedef struct {
	
	volatile unsigned long Thread0Enable;	
	volatile unsigned long Thread1Enable;	
	volatile unsigned long Thread0Recover;	
	volatile unsigned long Thread1Recover;	
	volatile unsigned long Thread0Step;	
	volatile unsigned long Thread1Step;	
	volatile unsigned long VideoInStatus;	
	volatile unsigned long Core2InSignStart;	
	volatile unsigned long Core1ResetVector;	
	volatile unsigned long Core1ROMOffset;	
	volatile unsigned long Core1ArbiterPriority;	
	volatile unsigned long VideoInControl;	
	volatile unsigned long VideoInReg0CtrlA;	
	volatile unsigned long VideoInReg0CtrlB;	
	volatile unsigned long VideoInReg1CtrlA;	
	volatile unsigned long VideoInReg1CtrlB;	
	volatile unsigned long Thread0Kicker;	
	volatile unsigned long Core2InputSign;	
	volatile unsigned long Thread0ProgCtr;	
	volatile unsigned long Thread1ProgCtr;	
	volatile unsigned long Thread1Kicker;	
	volatile unsigned long GPRegister1;	
	volatile unsigned long GPRegister2;	
	volatile unsigned long GPRegister3;	
	volatile unsigned long GPRegister4;	
	volatile unsigned long SerialIntA;	

	volatile unsigned long Fill0[6];	

	volatile unsigned long SoftwareReset;	
	volatile unsigned long SerialIntB;	

	volatile unsigned long Fill1[37];	

	volatile unsigned long ROMELQV;	
	volatile unsigned long WLWH;	
	volatile unsigned long ROMELWL;	

	volatile unsigned long dwFill_1;	

	volatile unsigned long IntStatus;	
	volatile unsigned long IntMask;	
	volatile unsigned long IntClear;	

	volatile unsigned long Fill2[6];	

	volatile unsigned long ROMGPIOA;	
	volatile unsigned long ROMGPIOB;	
	volatile unsigned long ROMGPIOC;	
	volatile unsigned long ROMGPIOD;	

	volatile unsigned long Fill3[2];	

	volatile unsigned long AGPIntID;	
	volatile unsigned long AGPIntClassCode;	
	volatile unsigned long AGPIntBIST;	
	volatile unsigned long AGPIntSSID;	
	volatile unsigned long AGPIntPMCSR;	
	volatile unsigned long VGAFrameBufBase;	
	volatile unsigned long VGANotify;	
	volatile unsigned long DACPLLMode;	
	volatile unsigned long Core1VideoClockDiv;	
	volatile unsigned long AGPIntStat;	

	
	volatile unsigned long Fill4[412];	

	volatile unsigned long TACtrlStreamBase;	
	volatile unsigned long TAObjDataBase;	
	volatile unsigned long TAPtrDataBase;	
	volatile unsigned long TARegionDataBase;	
	volatile unsigned long TATailPtrBase;	
	volatile unsigned long TAPtrRegionSize;	
	volatile unsigned long TAConfiguration;	
	volatile unsigned long TAObjDataStartAddr;	
	volatile unsigned long TAObjDataEndAddr;	
	volatile unsigned long TAXScreenClip;	
	volatile unsigned long TAYScreenClip;	
	volatile unsigned long TARHWClamp;	
	volatile unsigned long TARHWCompare;	
	volatile unsigned long TAStart;	
	volatile unsigned long TAObjReStart;	
	volatile unsigned long TAPtrReStart;	
	volatile unsigned long TAStatus1;	
	volatile unsigned long TAStatus2;	
	volatile unsigned long TAIntStatus;	
	volatile unsigned long TAIntMask;	

	volatile unsigned long Fill5[235];	

	volatile unsigned long TextureAddrThresh;	
	volatile unsigned long Core1Translation;	
	volatile unsigned long TextureAddrReMap;	
	volatile unsigned long RenderOutAGPRemap;	
	volatile unsigned long _3DRegionReadTrans;	
	volatile unsigned long _3DPtrReadTrans;	
	volatile unsigned long _3DParamReadTrans;	
	volatile unsigned long _3DRegionReadThresh;	
	volatile unsigned long _3DPtrReadThresh;	
	volatile unsigned long _3DParamReadThresh;	
	volatile unsigned long _3DRegionReadAGPRemap;	
	volatile unsigned long _3DPtrReadAGPRemap;	
	volatile unsigned long _3DParamReadAGPRemap;	
	volatile unsigned long ZBufferAGPRemap;	
	volatile unsigned long TAIndexAGPRemap;	
	volatile unsigned long TAVertexAGPRemap;	
	volatile unsigned long TAUVAddrTrans;	
	volatile unsigned long TATailPtrCacheTrans;	
	volatile unsigned long TAParamWriteTrans;	
	volatile unsigned long TAPtrWriteTrans;	
	volatile unsigned long TAParamWriteThresh;	
	volatile unsigned long TAPtrWriteThresh;	
	volatile unsigned long TATailPtrCacheAGPRe;	
	volatile unsigned long TAParamWriteAGPRe;	
	volatile unsigned long TAPtrWriteAGPRe;	
	volatile unsigned long SDRAMArbiterConf;	
	volatile unsigned long SDRAMConf0;	
	volatile unsigned long SDRAMConf1;	
	volatile unsigned long SDRAMConf2;	
	volatile unsigned long SDRAMRefresh;	
	volatile unsigned long SDRAMPowerStat;	

	volatile unsigned long Fill6[2];	

	volatile unsigned long RAMBistData;	
	volatile unsigned long RAMBistCtrl;	
	volatile unsigned long FIFOBistKey;	
	volatile unsigned long RAMBistResult;	
	volatile unsigned long FIFOBistResult;	

	

	volatile unsigned long Fill7[16];	

	volatile unsigned long SDRAMAddrSign;	
	volatile unsigned long SDRAMDataSign;	
	volatile unsigned long SDRAMSignConf;	

	
	volatile unsigned long dwFill_2;

	volatile unsigned long ISPSignature;	

	volatile unsigned long Fill8[454];	

	volatile unsigned long DACPrimAddress;	
	volatile unsigned long DACPrimSize;	
	volatile unsigned long DACCursorAddr;	
	volatile unsigned long DACCursorCtrl;	
	volatile unsigned long DACOverlayAddr;	
	volatile unsigned long DACOverlayUAddr;	
	volatile unsigned long DACOverlayVAddr;	
	volatile unsigned long DACOverlaySize;	
	volatile unsigned long DACOverlayVtDec;	

	volatile unsigned long Fill9[9];	

	volatile unsigned long DACVerticalScal;	
	volatile unsigned long DACPixelFormat;	
	volatile unsigned long DACHorizontalScal;	
	volatile unsigned long DACVidWinStart;	
	volatile unsigned long DACVidWinEnd;	
	volatile unsigned long DACBlendCtrl;	
	volatile unsigned long DACHorTim1;	
	volatile unsigned long DACHorTim2;	
	volatile unsigned long DACHorTim3;	
	volatile unsigned long DACVerTim1;	
	volatile unsigned long DACVerTim2;	
	volatile unsigned long DACVerTim3;	
	volatile unsigned long DACBorderColor;	
	volatile unsigned long DACSyncCtrl;	
	volatile unsigned long DACStreamCtrl;	
	volatile unsigned long DACLUTAddress;	
	volatile unsigned long DACLUTData;	
	volatile unsigned long DACBurstCtrl;	
	volatile unsigned long DACCrcTrigger;	
	volatile unsigned long DACCrcDone;	
	volatile unsigned long DACCrcResult1;	
	volatile unsigned long DACCrcResult2;	
	volatile unsigned long DACLinecount;	

	volatile unsigned long Fill10[151];	

	volatile unsigned long DigVidPortCtrl;	
	volatile unsigned long DigVidPortStat;	

	

	volatile unsigned long Fill11[1598];

	
	volatile unsigned long Fill_3;

} STG4000REG;

#endif 
