

#ifndef __SPECTRUM_H__
#define __SPECTRUM_H__

#include "rtmp_type.h"
#include "spectrum_def.h"


CHAR RTMP_GetTxPwr(
	IN PRTMP_ADAPTER pAd,
	IN HTTRANSMIT_SETTING HTTxMode);


VOID MakeMeasurementReqFrame(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pOutBuffer,
	OUT PULONG pFrameLen,
	IN UINT8 TotalLen,
	IN UINT8 Category,
	IN UINT8 Action,
	IN UINT8 MeasureToken,
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT8 NumOfRepetitions);


VOID EnqueueMeasurementRep(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDA,
	IN UINT8 DialogToken,
	IN UINT8 MeasureToken,
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT8 ReportInfoLen,
	IN PUINT8 pReportInfo);


VOID EnqueueTPCReq(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDA,
	IN UCHAR DialogToken);


VOID EnqueueTPCRep(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDA,
	IN UINT8 DialogToken,
	IN UINT8 TxPwr,
	IN UINT8 LinkMargin);


VOID EnqueueChSwAnn(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDA,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh);


VOID PeerSpectrumAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);


INT Set_MeasureReq_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_TpcReq_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_PwrConstraint(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);


VOID MeasureReqTabInit(
	IN PRTMP_ADAPTER pAd);

VOID MeasureReqTabExit(
	IN PRTMP_ADAPTER pAd);

PMEASURE_REQ_ENTRY MeasureReqLookUp(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8			DialogToken);

PMEASURE_REQ_ENTRY MeasureReqInsert(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8			DialogToken);

VOID MeasureReqDelete(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8			DialogToken);

VOID InsertChannelRepIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PSTRING pCountry,
	IN UINT8 RegulatoryClass);

VOID InsertTpcReportIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 TxPwr,
	IN UINT8 LinkMargin);

VOID InsertDialogToken(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 DialogToken);

VOID TpcReqTabInit(
	IN PRTMP_ADAPTER pAd);

VOID TpcReqTabExit(
	IN PRTMP_ADAPTER pAd);

VOID NotifyChSwAnnToPeerAPs(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pRA,
	IN PUCHAR pTA,
	IN UINT8 ChSwMode,
	IN UINT8 Channel);

VOID RguClass_BuildBcnChList(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pBuf,
	OUT	PULONG pBufLen);
#endif 
