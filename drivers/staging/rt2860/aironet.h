

#ifndef	__AIRONET_H__
#define	__AIRONET_H__


#define	MSRN_TYPE_UNUSED				0
#define	MSRN_TYPE_CHANNEL_LOAD_REQ		1
#define	MSRN_TYPE_NOISE_HIST_REQ		2
#define	MSRN_TYPE_BEACON_REQ			3
#define	MSRN_TYPE_FRAME_REQ				4


#define	MSRN_SCAN_MODE_PASSIVE			0
#define	MSRN_SCAN_MODE_ACTIVE			1
#define	MSRN_SCAN_MODE_BEACON_TABLE		2


#define	PHY_FH							1
#define	PHY_DSS							2
#define	PHY_UNUSED						3
#define	PHY_OFDM						4
#define	PHY_HR_DSS						5
#define	PHY_ERP							6


#define	RPI_0			0			
#define	RPI_1			1			
#define	RPI_2			2			
#define	RPI_3			3			
#define	RPI_4			4			
#define	RPI_5			5			
#define	RPI_6			6			
#define	RPI_7			7			


#define	AIRONET_IAPP_TYPE					0x32
#define	AIRONET_IAPP_SUBTYPE_REQUEST		0x01
#define	AIRONET_IAPP_SUBTYPE_REPORT			0x81


typedef	struct	_MEASUREMENT_REQUEST	{
	UCHAR	Channel;
	UCHAR	ScanMode;			
	USHORT	Duration;
}	MEASUREMENT_REQUEST, *PMEASUREMENT_REQUEST;




typedef	struct	_BEACON_REPORT	{
	UCHAR	Channel;
	UCHAR	Spare;
	USHORT	Duration;
	UCHAR	PhyType;			
	UCHAR	RxPower;
	UCHAR	BSSID[6];
	UCHAR	ParentTSF[4];
	UCHAR	TargetTSF[8];
	USHORT	BeaconInterval;
	USHORT	CapabilityInfo;
}	BEACON_REPORT, *PBEACON_REPORT;


typedef	struct	_FRAME_REPORT	{
	UCHAR	Channel;
	UCHAR	Spare;
	USHORT	Duration;
	UCHAR	TA;
	UCHAR	BSSID[6];
	UCHAR	RSSI;
	UCHAR	Count;
}	FRAME_REPORT, *PFRAME_REPORT;

#pragma pack(1)

typedef	struct	_CHANNEL_LOAD_REPORT	{
	UCHAR	Channel;
	UCHAR	Spare;
	USHORT	Duration;
	UCHAR	CCABusy;
}	CHANNEL_LOAD_REPORT, *PCHANNEL_LOAD_REPORT;
#pragma pack()


typedef	struct	_NOISE_HIST_REPORT	{
	UCHAR	Channel;
	UCHAR	Spare;
	USHORT	Duration;
	UCHAR	Density[8];
}	NOISE_HIST_REPORT, *PNOISE_HIST_REPORT;


typedef	struct	_RADIO_MANAGEMENT_CAPABILITY	{
	UCHAR	Eid;				
	UCHAR	Length;
	UCHAR	AironetOui[3];		
	UCHAR	Type;				
	USHORT	Status;				
}	RADIO_MANAGEMENT_CAPABILITY, *PRADIO_MANAGEMENT_CAPABILITY;


typedef	struct	_MEASUREMENT_MODE	{
	UCHAR	Rsvd:4;
	UCHAR	Report:1;
	UCHAR	NotUsed:1;
	UCHAR	Enable:1;
	UCHAR	Parallel:1;
}	MEASUREMENT_MODE, *PMEASUREMENT_MODE;


typedef	struct	_MEASUREMENT_REQUEST_ELEMENT	{
	USHORT				Eid;
	USHORT				Length;				
	USHORT				Token;				
	UCHAR				Mode;				
	UCHAR				Type;				
}	MEASUREMENT_REQUEST_ELEMENT, *PMEASUREMENT_REQUEST_ELEMENT;


typedef	struct	_MEASUREMENT_REPORT_ELEMENT	{
	USHORT				Eid;
	USHORT				Length;				
	USHORT				Token;				
	UCHAR				Mode;				
	UCHAR				Type;				
}	MEASUREMENT_REPORT_ELEMENT, *PMEASUREMENT_REPORT_ELEMENT;


typedef	struct	_AIRONET_IAPP_HEADER {
	UCHAR	CiscoSnapHeader[8];	
	USHORT	Length;				
	UCHAR	Type;				
	UCHAR	SubType;			
	UCHAR	DA[6];				
	UCHAR	SA[6];				
	USHORT	Token;				
}	AIRONET_IAPP_HEADER, *PAIRONET_IAPP_HEADER;


typedef	struct	_AIRONET_RM_REQUEST_FRAME	{
    AIRONET_IAPP_HEADER	IAPP;			
	UCHAR				Delay;			
	UCHAR				Offset;			
}	AIRONET_RM_REQUEST_FRAME, *PAIRONET_RM_REQUEST_FRAME;


typedef	struct	_AIRONET_RM_REPORT_FRAME	{
    AIRONET_IAPP_HEADER	IAPP;			
}	AIRONET_RM_REPORT_FRAME, *PAIRONET_RM_REPORT_FRAME;


typedef	struct	_RM_REQUEST_ACTION	{
	MEASUREMENT_REQUEST_ELEMENT	ReqElem;		
	MEASUREMENT_REQUEST			Measurement;	
}	RM_REQUEST_ACTION, *PRM_REQUEST_ACTION;


typedef	union	_CCX_CONTROL	{
	struct	{
		UINT32		Enable:1;			
		UINT32		LeapEnable:1;		
		UINT32		RMEnable:1;			
		UINT32		DCRMEnable:1;		
		UINT32		QOSEnable:1;		
		UINT32		FastRoamEnable:1;	
		UINT32		Rsvd:2;				
		UINT32		dBmToRoam:8;		
		UINT32		TuLimit:16;			
	}	field;
	UINT32			word;
}	CCX_CONTROL, *PCCX_CONTROL;

#endif	
