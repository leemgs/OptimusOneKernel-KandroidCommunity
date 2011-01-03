

#ifndef __MICHAEL_H__
#define __MICHAEL_H__





VOID MIC_vInit(DWORD dwK0, DWORD dwK1);

VOID MIC_vUnInit(void);


VOID MIC_vAppend(PBYTE src, UINT nBytes);



VOID MIC_vGetMIC(PDWORD pdwL, PDWORD pdwR);




#define ROL32( A, n ) \
 ( ((A) << (n)) | ( ((A)>>(32-(n)))  & ( (1UL << (n)) - 1 ) ) )
#define ROR32( A, n ) ROL32( (A), 32-(n) )

#endif 


