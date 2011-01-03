

#ifndef __DOT11_BASE_H__
#define __DOT11_BASE_H__

#include "rtmp_type.h"



typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
    UINT32		RDG:1;	
    UINT32		ACConstraint:1;	
    UINT32		rsv:5;  
    UINT32		ZLFAnnouce:1;	
    UINT32		CSISTEERING:2;	
    UINT32		FBKReq:2;	
    UINT32		CalSeq:2;  
    UINT32		CalPos:2;	
    UINT32		MFBorASC:7;	
    UINT32		MFS:3;	
    UINT32		MRSorASI:3;	
    UINT32		MRQ:1;	
    UINT32		TRQ:1;	
    UINT32		MA:1;	
#else
    UINT32		MA:1;	
    UINT32		TRQ:1;	
    UINT32		MRQ:1;	
    UINT32		MRSorASI:3;	
    UINT32		MFS:3;	
    UINT32		MFBorASC:7;	
    UINT32		CalPos:2;	
    UINT32		CalSeq:2;  
    UINT32		FBKReq:2;	
    UINT32		CSISTEERING:2;	
    UINT32		ZLFAnnouce:1;	
    UINT32		rsv:5;  
    UINT32		ACConstraint:1;	
    UINT32		RDG:1;	
#endif 
} HT_CONTROL, *PHT_CONTROL;


typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
    USHORT      Txop_QueueSize:8;
    USHORT      AMsduPresent:1;
    USHORT      AckPolicy:2;  
    USHORT      EOSP:1;
    USHORT      TID:4;
#else
    USHORT      TID:4;
    USHORT      EOSP:1;
    USHORT      AckPolicy:2;  
    USHORT      AMsduPresent:1;
    USHORT      Txop_QueueSize:8;
#endif 
} QOS_CONTROL, *PQOS_CONTROL;



typedef	struct	PACKED {
#ifdef RT_BIG_ENDIAN
	USHORT		Order:1;			
	USHORT		Wep:1;				
	USHORT		MoreData:1;			
	USHORT		PwrMgmt:1;			
	USHORT		Retry:1;			
	USHORT		MoreFrag:1;			
	USHORT		FrDs:1;				
	USHORT		ToDs:1;				
	USHORT		SubType:4;			
	USHORT		Type:2;				
	USHORT		Ver:2;				
#else
	USHORT		Ver:2;				
	USHORT		Type:2;				
	USHORT		SubType:4;			
	USHORT		ToDs:1;				
	USHORT		FrDs:1;				
	USHORT		MoreFrag:1;			
	USHORT		Retry:1;			
	USHORT		PwrMgmt:1;			
	USHORT		MoreData:1;			
	USHORT		Wep:1;				
	USHORT		Order:1;			
#endif 
} FRAME_CONTROL, *PFRAME_CONTROL;

typedef	struct	PACKED _HEADER_802_11	{
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
	UCHAR			Addr3[MAC_ADDR_LEN];
#ifdef RT_BIG_ENDIAN
	USHORT			Sequence:12;
	USHORT			Frag:4;
#else
	USHORT			Frag:4;
	USHORT			Sequence:12;
#endif 
	UCHAR			Octet[0];
}	HEADER_802_11, *PHEADER_802_11;

typedef struct PACKED _PSPOLL_FRAME {
    FRAME_CONTROL   FC;
    USHORT          Aid;
    UCHAR           Bssid[MAC_ADDR_LEN];
    UCHAR           Ta[MAC_ADDR_LEN];
}   PSPOLL_FRAME, *PPSPOLL_FRAME;

typedef	struct	PACKED _RTS_FRAME	{
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
}RTS_FRAME, *PRTS_FRAME;

#endif 
