

#ifndef	__ACTION_H__
#define	__ACTION_H__

typedef struct PACKED __HT_INFO_OCTET
{
	UCHAR	Request:1;
	UCHAR	Forty_MHz_Intolerant:1;
	UCHAR 	STA_Channel_Width:1;
	UCHAR	Reserved:5;
} HT_INFORMATION_OCTET;


typedef struct PACKED __FRAME_HT_INFO
{
	HEADER_802_11   		Hdr;
	UCHAR					Category;
	UCHAR					Action;
	HT_INFORMATION_OCTET	HT_Info;
}   FRAME_HT_INFO, *PFRAME_HT_INFO;

#endif 


