

#ifndef __MD4_H__
#define __MD4_H__


typedef	struct	_MD4_CTX_	{
	ULONG	state[4];        
	ULONG	count[2];        
	UCHAR	buffer[64];      
}	MD4_CTX;

VOID MD4Init (MD4_CTX *);
VOID MD4Update (MD4_CTX *, PUCHAR, UINT);
VOID MD4Final (UCHAR [16], MD4_CTX *);

#endif 