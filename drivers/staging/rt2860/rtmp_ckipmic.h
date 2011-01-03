
#ifndef	__RTMP_CKIPMIC_H__
#define	__RTMP_CKIPMIC_H__

typedef	struct	_MIC_CONTEXT	{
	
	UCHAR		CK[16];				
	UCHAR		coefficient[16];	
	ULONGLONG	accum;				
	UINT		position;			
	UCHAR		part[4];			
}	MIC_CONTEXT, *PMIC_CONTEXT;

VOID xor_128(
    IN  PUCHAR              a,
    IN  PUCHAR              b,
    OUT PUCHAR              out);

UCHAR RTMPCkipSbox(
    IN  UCHAR               a);

VOID xor_32(
    IN  PUCHAR              a,
    IN  PUCHAR              b,
    OUT PUCHAR              out);

VOID next_key(
    IN  PUCHAR              key,
    IN  INT                 round);

VOID byte_sub(
    IN  PUCHAR              in,
    OUT PUCHAR              out);

VOID shift_row(
    IN  PUCHAR              in,
    OUT PUCHAR              out);

VOID mix_column(
    IN  PUCHAR              in,
    OUT PUCHAR              out);

#endif 
