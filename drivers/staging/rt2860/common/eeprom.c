
#include "../rt_config.h"


VOID RaiseClock(
    IN	PRTMP_ADAPTER	pAd,
    IN  UINT32 *x)
{
    *x = *x | EESK;
    RTMP_IO_WRITE32(pAd, E2PROM_CSR, *x);
    RTMPusecDelay(1);				
}


VOID LowerClock(
    IN	PRTMP_ADAPTER	pAd,
    IN  UINT32 *x)
{
    *x = *x & ~EESK;
    RTMP_IO_WRITE32(pAd, E2PROM_CSR, *x);
    RTMPusecDelay(1);
}


USHORT ShiftInBits(
    IN	PRTMP_ADAPTER	pAd)
{
    UINT32		x,i;
	USHORT      data=0;

    RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

    x &= ~( EEDO | EEDI);

    for(i=0; i<16; i++)
    {
        data = data << 1;
        RaiseClock(pAd, &x);

        RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	LowerClock(pAd, &x); 

        x &= ~(EEDI);
        if(x & EEDO)
            data |= 1;
    }

    return data;
}


VOID ShiftOutBits(
    IN	PRTMP_ADAPTER	pAd,
    IN  USHORT data,
    IN  USHORT count)
{
    UINT32       x,mask;

    mask = 0x01 << (count - 1);
    RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

    x &= ~(EEDO | EEDI);

    do
    {
        x &= ~EEDI;
        if(data & mask)		x |= EEDI;

        RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

        RaiseClock(pAd, &x);
        LowerClock(pAd, &x);

        mask = mask >> 1;
    } while(mask);

    x &= ~EEDI;
    RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
}


VOID EEpromCleanup(
    IN	PRTMP_ADAPTER	pAd)
{
    UINT32 x;

    RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

    x &= ~(EECS | EEDI);
    RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

    RaiseClock(pAd, &x);
    LowerClock(pAd, &x);
}

VOID EWEN(
	IN	PRTMP_ADAPTER	pAd)
{
    UINT32	x;

    
    RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
    x &= ~(EEDI | EEDO | EESK);
    x |= EECS;
    RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);

    
    ShiftOutBits(pAd, EEPROM_EWEN_OPCODE, 5);
    ShiftOutBits(pAd, 0, 6);

    EEpromCleanup(pAd);
}

VOID EWDS(
	IN	PRTMP_ADAPTER	pAd)
{
    UINT32	x;

    
    RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
    x &= ~(EEDI | EEDO | EESK);
    x |= EECS;
    RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);

    
    ShiftOutBits(pAd, EEPROM_EWDS_OPCODE, 5);
    ShiftOutBits(pAd, 0, 6);

    EEpromCleanup(pAd);
}


USHORT RTMP_EEPROM_READ16(
    IN	PRTMP_ADAPTER	pAd,
    IN  USHORT Offset)
{
    UINT32		x;
    USHORT		data;

#ifdef RT2870
	if (pAd->NicConfig2.field.AntDiversity)
    {
    	pAd->EepromAccess = TRUE;
    }
#endif
    Offset /= 2;
    
    RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
    x &= ~(EEDI | EEDO | EESK);
    x |= EECS;
    RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	
    if (!IS_RT3090(pAd))
    {
	
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);
    }

    
    ShiftOutBits(pAd, EEPROM_READ_OPCODE, 3);
    ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);

    
    data = ShiftInBits(pAd);

    EEpromCleanup(pAd);

#ifdef RT2870
	
    
    
	if ((pAd->NicConfig2.field.AntDiversity) || (pAd->RfIcType == RFIC_3020))
    {
	    pAd->EepromAccess = FALSE;
	    AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);
    }
#endif
    return data;
}	

VOID RTMP_EEPROM_WRITE16(
    IN	PRTMP_ADAPTER	pAd,
    IN  USHORT Offset,
    IN  USHORT Data)
{
    UINT32 x;

#ifdef RT2870
	if (pAd->NicConfig2.field.AntDiversity)
    {
    	pAd->EepromAccess = TRUE;
    }
#endif
	Offset /= 2;

	EWEN(pAd);

    
    RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
    x &= ~(EEDI | EEDO | EESK);
    x |= EECS;
    RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	
    if (!IS_RT3090(pAd))
    {
	
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);
    }

    
    ShiftOutBits(pAd, EEPROM_WRITE_OPCODE, 3);
    ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);
	ShiftOutBits(pAd, Data, 16);		

    
    RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	EEpromCleanup(pAd);

	RTMPusecDelay(10000);	

	EWDS(pAd);

    EEpromCleanup(pAd);

#ifdef RT2870
	
    
    
	if ((pAd->NicConfig2.field.AntDiversity) || (pAd->RfIcType == RFIC_3020))
    {
	    pAd->EepromAccess = FALSE;
	    AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);
    }
#endif
}

#ifdef RT2870

UCHAR eFuseReadRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	USHORT	efuseDataOffset;
	UINT32	data;

	RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

	
	
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	
	eFuseCtrlStruc.field.EFSROM_MODE = 0;

	
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

	
	i = 0;
	while(i < 100)
	{
		
		RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);
		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			break;
		}
		RTMPusecDelay(2);
		i++;
	}

	
	if (eFuseCtrlStruc.field.EFSROM_AOUT == 0x3f)
	{
		for(i=0; i<Length/2; i++)
			*(pData+2*i) = 0xffff;
	}
	else
	{
		
		efuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC)  ;
		
		
		RTMP_IO_READ32(pAd, efuseDataOffset, &data);
		
		
		
		
		
		
		
		
		
		data = data >> (8*(Offset & 0x3));

		NdisMoveMemory(pData, &data, Length);
	}

	return (UCHAR) eFuseCtrlStruc.field.EFSROM_AOUT;

}


VOID eFusePhysicalReadRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	USHORT	efuseDataOffset;
	UINT32	data;

	RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

	
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	
	
	eFuseCtrlStruc.field.EFSROM_MODE = 1;

	
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

	
	i = 0;
	while(i < 100)
	{
		RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);
		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
			break;
		RTMPusecDelay(2);
		i++;
	}

	
	
	
	
	
	
	
	
	efuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC)  ;

	RTMP_IO_READ32(pAd, efuseDataOffset, &data);

	data = data >> (8*(Offset & 0x3));

	NdisMoveMemory(pData, &data, Length);

}


VOID eFuseReadPhysical(
	IN	PRTMP_ADAPTER	pAd,
  	IN	PUSHORT lpInBuffer,
  	IN	ULONG nInBufferSize,
  	OUT	PUSHORT lpOutBuffer,
  	IN	ULONG nOutBufferSize
)
{
	USHORT* pInBuf = (USHORT*)lpInBuffer;
	USHORT* pOutBuf = (USHORT*)lpOutBuffer;

	USHORT Offset = pInBuf[0];					
	USHORT Length = pInBuf[1];					
	int 		i;

	for(i=0; i<Length; i+=2)
	{
		eFusePhysicalReadRegisters(pAd,Offset+i, 2, &pOutBuf[i/2]);
	}
}


NTSTATUS eFuseRead(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUCHAR			pData,
	IN	USHORT			Length)
{
	USHORT* pOutBuf = (USHORT*)pData;
	NTSTATUS	Status = STATUS_SUCCESS;
	UCHAR	EFSROM_AOUT;
	int	i;

	for(i=0; i<Length; i+=2)
	{
		EFSROM_AOUT = eFuseReadRegisters(pAd, Offset+i, 2, &pOutBuf[i/2]);
	}
	return Status;
}


VOID eFusePhysicalWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	USHORT	efuseDataOffset;
	UINT32	data, eFuseDataBuffer[4];

	

	
	
	RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

	
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	
	eFuseCtrlStruc.field.EFSROM_MODE = 1;

	
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

	
	i = 0;
	while(i < 100)
	{
		RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
			break;
		RTMPusecDelay(2);
		i++;
	}

	
	efuseDataOffset =  EFUSE_DATA3;
	for(i=0; i< 4; i++)
	{
		RTMP_IO_READ32(pAd, efuseDataOffset, (PUINT32) &eFuseDataBuffer[i]);
		efuseDataOffset -=  4;
	}

	
	efuseDataOffset = (Offset & 0xc) >> 2;
	data = pData[0] & 0xffff;
	
	if((Offset % 4) != 0)
	{
		eFuseDataBuffer[efuseDataOffset] = (eFuseDataBuffer[efuseDataOffset] & 0xffff) | (data << 16);
	}
	else
	{
		eFuseDataBuffer[efuseDataOffset] = (eFuseDataBuffer[efuseDataOffset] & 0xffff0000) | data;
	}

	efuseDataOffset =  EFUSE_DATA3;
	for(i=0; i< 4; i++)
	{
		RTMP_IO_WRITE32(pAd, efuseDataOffset, eFuseDataBuffer[i]);
		efuseDataOffset -= 4;
	}
	

	
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	
	eFuseCtrlStruc.field.EFSROM_MODE = 3;

	
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

	
	i = 0;
	while(i < 100)
	{
		RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
			break;

		RTMPusecDelay(2);
		i++;
	}
}


NTSTATUS eFuseWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	IN	USHORT* pData)
{
	USHORT	i;
	USHORT	eFuseData;
	USHORT	LogicalAddress, BlkNum = 0xffff;
	UCHAR	EFSROM_AOUT;

	USHORT addr,tmpaddr, InBuf[3], tmpOffset;
	USHORT buffer[8];
	BOOLEAN		bWriteSuccess = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters Offset=%x, pData=%x\n", Offset, *pData));

	
	
	
	tmpOffset = Offset & 0xfffe;
	EFSROM_AOUT = eFuseReadRegisters(pAd, tmpOffset, 2, &eFuseData);

	if( EFSROM_AOUT == 0x3f)
	{	
		
		
		
		for (i=EFUSE_USAGE_MAP_START; i<=EFUSE_USAGE_MAP_END; i+=2)
		{
			
			
			eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
			if( (LogicalAddress & 0xff) == 0)
			{
				BlkNum = i-EFUSE_USAGE_MAP_START;
				break;
			}
			else if(( (LogicalAddress >> 8) & 0xff) == 0)
			{
				if (i != EFUSE_USAGE_MAP_END)
				{
					BlkNum = i-EFUSE_USAGE_MAP_START+1;
				}
				break;
			}
		}
	}
	else
	{
		BlkNum = EFSROM_AOUT;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters BlkNum = %d \n", BlkNum));

	if(BlkNum == 0xffff)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
		return FALSE;
	}

	
	
	for(i =0; i<8; i++)
	{
		addr = BlkNum * 0x10 ;

		InBuf[0] = addr+2*i;
		InBuf[1] = 2;
		InBuf[2] = 0x0;

		eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

		buffer[i] = InBuf[2];
	}

	
	buffer[ (Offset >> 1) % 8] = pData[0];

	do
	{
		
		if(!bWriteSuccess)
		{
			for(i =0; i<8; i++)
			{
				addr = BlkNum * 0x10 ;

				InBuf[0] = addr+2*i;
				InBuf[1] = 2;
				InBuf[2] = buffer[i];

				eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2);
			}
		}
		else
		{
				addr = BlkNum * 0x10 ;

				InBuf[0] = addr+(Offset % 16);
				InBuf[1] = 2;
				InBuf[2] = pData[0];

				eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2);
		}

		
		addr = EFUSE_USAGE_MAP_START+BlkNum;

		tmpaddr = addr;

		if(addr % 2 != 0)
			addr = addr -1;
		InBuf[0] = addr;
		InBuf[1] = 2;

		
		tmpOffset = Offset;
		tmpOffset >>= 4;
		tmpOffset |= ((~((tmpOffset & 0x01) ^ ( tmpOffset >> 1 & 0x01) ^  (tmpOffset >> 2 & 0x01) ^  (tmpOffset >> 3 & 0x01))) << 6) & 0x40;
		tmpOffset |= ((~( (tmpOffset >> 2 & 0x01) ^ (tmpOffset >> 3 & 0x01) ^ (tmpOffset >> 4 & 0x01) ^ ( tmpOffset >> 5 & 0x01))) << 7) & 0x80;

		
		if(tmpaddr%2 != 0)
			InBuf[2] = tmpOffset<<8;
		else
			InBuf[2] = tmpOffset;

		eFuseWritePhysical(pAd,&InBuf[0], 6, NULL, 0);

		
		bWriteSuccess = TRUE;
		for(i =0; i<8; i++)
		{
			addr = BlkNum * 0x10 ;

			InBuf[0] = addr+2*i;
			InBuf[1] = 2;
			InBuf[2] = 0x0;

			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

			if(buffer[i] != InBuf[2])
			{
				bWriteSuccess = FALSE;
				break;
			}
		}

		
		if (!bWriteSuccess)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Not bWriteSuccess BlkNum = %d\n", BlkNum));

			
			addr = EFUSE_USAGE_MAP_START+BlkNum;

			
			BlkNum = 0xffff;
			for (i=EFUSE_USAGE_MAP_START; i<=EFUSE_USAGE_MAP_END; i+=2)
			{
				eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
				if( (LogicalAddress & 0xff) == 0)
				{
					BlkNum = i-EFUSE_USAGE_MAP_START;
					break;
				}
				else if(( (LogicalAddress >> 8) & 0xff) == 0)
				{
					if (i != EFUSE_USAGE_MAP_END)
					{
						BlkNum = i+1-EFUSE_USAGE_MAP_START;
					}
					break;
				}
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Not bWriteSuccess new BlkNum = %d\n", BlkNum));
			if(BlkNum == 0xffff)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
				return FALSE;
			}

			
			tmpaddr = addr;

			if(addr % 2 != 0)
				addr = addr -1;
			InBuf[0] = addr;
			InBuf[1] = 2;

			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

			
			if(tmpaddr%2 != 0)
			{
				
				for (i=8; i<15; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}
				}
			}
			else
			{
				
				for (i=0; i<8; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}
				}
			}
			eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 0);
		}
	}
	while(!bWriteSuccess);

	return TRUE;
}


VOID eFuseWritePhysical(
	IN	PRTMP_ADAPTER	pAd,
  	PUSHORT lpInBuffer,
	ULONG nInBufferSize,
  	PUCHAR lpOutBuffer,
  	ULONG nOutBufferSize
)
{
	USHORT* pInBuf = (USHORT*)lpInBuffer;
	int 		i;
	

	USHORT Offset = pInBuf[0];					
	USHORT Length = pInBuf[1];					
	USHORT* pValueX = &pInBuf[2];				
		
		
		
		
		
		
		
	for(i=0; i<Length; i+=2)
	{
		eFusePhysicalWriteRegisters(pAd, Offset+i, 2, &pValueX[i/2]);
	}
}



NTSTATUS eFuseWrite(
   	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length)
{
	int i;

	USHORT* pValueX = (PUSHORT) pData;				
		
 		
		
		
		
		
		
		
		
	for(i=0; i<length; i+=2)
	{
		eFuseWriteRegisters(pAd, Offset+i, 2, &pValueX[i/2]);
	}

	return TRUE;
}


INT set_eFuseGetFreeBlockCount_Proc(
   	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg)
{
	USHORT i;
	USHORT	LogicalAddress;
	USHORT efusefreenum=0;
	if(!pAd->bUseEfuse)
		return FALSE;
	for (i = EFUSE_USAGE_MAP_START; i <= EFUSE_USAGE_MAP_END; i+=2)
	{
		eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
		if( (LogicalAddress & 0xff) == 0)
		{
			efusefreenum= (UCHAR) (EFUSE_USAGE_MAP_END-i+1);
			break;
		}
		else if(( (LogicalAddress >> 8) & 0xff) == 0)
		{
			efusefreenum = (UCHAR) (EFUSE_USAGE_MAP_END-i);
			break;
		}

		if(i == EFUSE_USAGE_MAP_END)
			efusefreenum = 0;
	}
	printk("efuseFreeNumber is %d\n",efusefreenum);
	return TRUE;
}
INT set_eFusedump_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg)
{
USHORT InBuf[3];
	INT i=0;
	if(!pAd->bUseEfuse)
		return FALSE;
	for(i =0; i<EFUSE_USAGE_MAP_END/2; i++)
	{
		InBuf[0] = 2*i;
		InBuf[1] = 2;
		InBuf[2] = 0x0;

		eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);
		if(i%4==0)
		printk("\nBlock %x:",i/8);
		printk("%04x ",InBuf[2]);
	}
	return TRUE;
}
INT	set_eFuseLoadFromBin_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg)
{
	CHAR					*src;
	struct file				*srcf;
	INT 					retval;
   	mm_segment_t			orgfs;
	UCHAR					*buffer;
	UCHAR					BinFileSize=0;
	INT						i = 0,j=0,k=1;
	USHORT					*PDATA;
	USHORT					DATA;
	BinFileSize=strlen("RT30xxEEPROM.bin");
	src = kmalloc(128, MEM_ALLOC_FLAG);
	NdisZeroMemory(src, 128);

 	if(strlen(arg)>0)
	{

		NdisMoveMemory(src, arg, strlen(arg));
 	}

	else
	{

		NdisMoveMemory(src, "RT30xxEEPROM.bin", BinFileSize);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("FileName=%s\n",src));
	buffer = kmalloc(MAX_EEPROM_BIN_FILE_SIZE, MEM_ALLOC_FLAG);

	if(buffer == NULL)
	{
		kfree(src);
		 return FALSE;
}
	PDATA=kmalloc(sizeof(USHORT)*8,MEM_ALLOC_FLAG);

	if(PDATA==NULL)
	{
		kfree(src);

		kfree(buffer);
		return FALSE;
	}

    	orgfs = get_fs();
   	 set_fs(KERNEL_DS);

	if (src && *src)
	{
		srcf = filp_open(src, O_RDONLY, 0);
		if (IS_ERR(srcf))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("--> Error %ld opening %s\n", -PTR_ERR(srcf),src));
			return FALSE;
		}
		else
		{
			
			if (srcf->f_op && srcf->f_op->read)
			{
				memset(buffer, 0x00, MAX_EEPROM_BIN_FILE_SIZE);
				while(srcf->f_op->read(srcf, &buffer[i], 1, &srcf->f_pos)==1)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%02X ",buffer[i]));
					if((i+1)%8==0)
						DBGPRINT(RT_DEBUG_TRACE, ("\n"));
              			i++;
						if(i>=MAX_EEPROM_BIN_FILE_SIZE)
							{
								DBGPRINT(RT_DEBUG_ERROR, ("--> Error %ld reading %s, The file is too large[1024]\n", -PTR_ERR(srcf),src));
								kfree(PDATA);
								kfree(buffer);
								kfree(src);
								return FALSE;
							}
			       }
			}
			else
			{
						DBGPRINT(RT_DEBUG_ERROR, ("--> Error!! System doest not support read function\n"));
						kfree(PDATA);
						kfree(buffer);
						kfree(src);
						return FALSE;
			}
      		}


	}
	else
		{
					DBGPRINT(RT_DEBUG_ERROR, ("--> Error src  or srcf is null\n"));
					kfree(PDATA);
					kfree(buffer);
					return FALSE;

		}


	retval=filp_close(srcf,NULL);

	if (retval)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> Error %d closing %s\n", -retval, src));
	}
	set_fs(orgfs);

	for(j=0;j<i;j++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%02X ",buffer[j]));
		if((j+1)%2==0)
			PDATA[j/2%8]=((buffer[j]<<8)&0xff00)|(buffer[j-1]&0xff);
		if(j%16==0)
		{
			k=buffer[j];
		}
		else
		{
			k&=buffer[j];
			if((j+1)%16==0)
			{

				DBGPRINT(RT_DEBUG_TRACE, (" result=%02X,blk=%02x\n",k,j/16));

				if(k!=0xff)
					eFuseWriteRegistersFromBin(pAd,(USHORT)j-15, 16, PDATA);
				else
					{
						if(eFuseReadRegisters(pAd,j, 2,(PUSHORT)&DATA)!=0x3f)
							eFuseWriteRegistersFromBin(pAd,(USHORT)j-15, 16, PDATA);
					}
				
				NdisZeroMemory(PDATA,16);


			}
		}


	}


	kfree(PDATA);
	kfree(buffer);
	kfree(src);
	return TRUE;
}
NTSTATUS eFuseWriteRegistersFromBin(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	IN	USHORT* pData)
{
	USHORT	i;
	USHORT	eFuseData;
	USHORT	LogicalAddress, BlkNum = 0xffff;
	UCHAR	EFSROM_AOUT,Loop=0;
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	USHORT	efuseDataOffset;
	UINT32	data,tempbuffer;
	USHORT addr,tmpaddr, InBuf[3], tmpOffset;
	UINT32 buffer[4];
	BOOLEAN		bWriteSuccess = TRUE;
	BOOLEAN		bNotWrite=TRUE;
	BOOLEAN		bAllocateNewBlk=TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegistersFromBin Offset=%x, pData=%04x:%04x:%04x:%04x\n", Offset, *pData,*(pData+1),*(pData+2),*(pData+3)));

	do
	{
	
	
	
	Loop++;
	tmpOffset = Offset & 0xfffe;
	EFSROM_AOUT = eFuseReadRegisters(pAd, tmpOffset, 2, &eFuseData);

	if( EFSROM_AOUT == 0x3f)
	{	
		
		
		
		bAllocateNewBlk=TRUE;
		for (i=EFUSE_USAGE_MAP_START; i<=EFUSE_USAGE_MAP_END; i+=2)
		{
			
			
			eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
			if( (LogicalAddress & 0xff) == 0)
			{
				BlkNum = i-EFUSE_USAGE_MAP_START;
				break;
			}
			else if(( (LogicalAddress >> 8) & 0xff) == 0)
			{
				if (i != EFUSE_USAGE_MAP_END)
				{
					BlkNum = i-EFUSE_USAGE_MAP_START+1;
				}
				break;
			}
		}
	}
	else
	{
		bAllocateNewBlk=FALSE;
		BlkNum = EFSROM_AOUT;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters BlkNum = %d \n", BlkNum));

	if(BlkNum == 0xffff)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
		return FALSE;
	}
	
	
	
	if(bAllocateNewBlk)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Allocate New Blk\n"));
		efuseDataOffset =  EFUSE_DATA3;
		for(i=0; i< 4; i++)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Allocate New Blk, Data%d=%04x%04x\n",3-i,pData[2*i+1],pData[2*i]));
			tempbuffer=((pData[2*i+1]<<16)&0xffff0000)|pData[2*i];


			RTMP_IO_WRITE32(pAd, efuseDataOffset,tempbuffer);
			efuseDataOffset -= 4;

		}
		

		
		eFuseCtrlStruc.field.EFSROM_AIN = BlkNum* 0x10 ;

		
		eFuseCtrlStruc.field.EFSROM_MODE = 3;

		
		eFuseCtrlStruc.field.EFSROM_KICK = 1;

		NdisMoveMemory(&data, &eFuseCtrlStruc, 4);

		RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

		
		i = 0;
		while(i < 100)
		{
			RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

			if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
				break;

			RTMPusecDelay(2);
			i++;
		}

	}
	else
	{	
		
		
		
		
		RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

		
		eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

		
		eFuseCtrlStruc.field.EFSROM_MODE = 0;

		
		eFuseCtrlStruc.field.EFSROM_KICK = 1;

		NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
		RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

		
		i = 0;
		while(i < 100)
		{
			RTMP_IO_READ32(pAd, EFUSE_CTRL, (PUINT32) &eFuseCtrlStruc);

			if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
				break;
			RTMPusecDelay(2);
			i++;
		}

		
		efuseDataOffset =  EFUSE_DATA3;
		for(i=0; i< 4; i++)
		{
			RTMP_IO_READ32(pAd, efuseDataOffset, (PUINT32) &buffer[i]);
			efuseDataOffset -=  4;
		}
		
		for(i =0; i<4; i++)
		{
			tempbuffer=((pData[2*i+1]<<16)&0xffff0000)|pData[2*i];
			DBGPRINT(RT_DEBUG_TRACE, ("buffer[%d]=%x,pData[%d]=%x,pData[%d]=%x,tempbuffer=%x\n",i,buffer[i],2*i,pData[2*i],2*i+1,pData[2*i+1],tempbuffer));

			if(((buffer[i]&0xffff0000)==(pData[2*i+1]<<16))&&((buffer[i]&0xffff)==pData[2*i]))
				bNotWrite&=TRUE;
			else
			{
				bNotWrite&=FALSE;
				break;
			}
		}
		if(!bNotWrite)
		{
		printk("The data is not the same\n");

			for(i =0; i<8; i++)
			{
				addr = BlkNum * 0x10 ;

				InBuf[0] = addr+2*i;
				InBuf[1] = 2;
				InBuf[2] = pData[i];

				eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2);
			}

		}
		else
			return TRUE;
	     }



		
		addr = EFUSE_USAGE_MAP_START+BlkNum;

		tmpaddr = addr;

		if(addr % 2 != 0)
			addr = addr -1;
		InBuf[0] = addr;
		InBuf[1] = 2;

		
		tmpOffset = Offset;
		tmpOffset >>= 4;
		tmpOffset |= ((~((tmpOffset & 0x01) ^ ( tmpOffset >> 1 & 0x01) ^  (tmpOffset >> 2 & 0x01) ^  (tmpOffset >> 3 & 0x01))) << 6) & 0x40;
		tmpOffset |= ((~( (tmpOffset >> 2 & 0x01) ^ (tmpOffset >> 3 & 0x01) ^ (tmpOffset >> 4 & 0x01) ^ ( tmpOffset >> 5 & 0x01))) << 7) & 0x80;

		
		if(tmpaddr%2 != 0)
			InBuf[2] = tmpOffset<<8;
		else
			InBuf[2] = tmpOffset;

		eFuseWritePhysical(pAd,&InBuf[0], 6, NULL, 0);

		
		bWriteSuccess = TRUE;
		for(i =0; i<8; i++)
		{
			addr = BlkNum * 0x10 ;

			InBuf[0] = addr+2*i;
			InBuf[1] = 2;
			InBuf[2] = 0x0;

			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);
			DBGPRINT(RT_DEBUG_TRACE, ("addr=%x, buffer[i]=%x,InBuf[2]=%x\n",InBuf[0],pData[i],InBuf[2]));
			if(pData[i] != InBuf[2])
			{
				bWriteSuccess = FALSE;
				break;
			}
		}

		

		if (!bWriteSuccess&&Loop<2)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegistersFromBin::Not bWriteSuccess BlkNum = %d\n", BlkNum));

			
			addr = EFUSE_USAGE_MAP_START+BlkNum;

			
			BlkNum = 0xffff;
			for (i=EFUSE_USAGE_MAP_START; i<=EFUSE_USAGE_MAP_END; i+=2)
			{
				eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
				if( (LogicalAddress & 0xff) == 0)
				{
					BlkNum = i-EFUSE_USAGE_MAP_START;
					break;
				}
				else if(( (LogicalAddress >> 8) & 0xff) == 0)
				{
					if (i != EFUSE_USAGE_MAP_END)
					{
						BlkNum = i+1-EFUSE_USAGE_MAP_START;
					}
					break;
				}
			}
			DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegistersFromBin::Not bWriteSuccess new BlkNum = %d\n", BlkNum));
			if(BlkNum == 0xffff)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegistersFromBin: out of free E-fuse space!!!\n"));
				return FALSE;
			}

			
			tmpaddr = addr;

			if(addr % 2 != 0)
				addr = addr -1;
			InBuf[0] = addr;
			InBuf[1] = 2;

			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

			
			if(tmpaddr%2 != 0)
			{
				
				for (i=8; i<15; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}
				}
			}
			else
			{
				
				for (i=0; i<8; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}
				}
			}
			eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 0);
		}

	}
	while(!bWriteSuccess&&Loop<2);

	return TRUE;
}
#endif
