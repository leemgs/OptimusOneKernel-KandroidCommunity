

#ifndef __TCRC_H__
#define __TCRC_H__

#include "ttype.h"













DWORD CRCdwCrc32(PBYTE pbyData, UINT cbByte, DWORD dwCrcSeed);
DWORD CRCdwGetCrc32(PBYTE pbyData, UINT cbByte);
DWORD CRCdwGetCrc32Ex(PBYTE pbyData, UINT cbByte, DWORD dwPreCRC);

#endif 



