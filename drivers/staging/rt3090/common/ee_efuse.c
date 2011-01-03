

#include "../rt_config.h"


#define EFUSE_USAGE_MAP_START	0x2d0
#define EFUSE_USAGE_MAP_END		0x2fc
#define EFUSE_USAGE_MAP_SIZE	45



#define EFUSE_EEPROM_DEFULT_FILE	"RT30xxEEPROM.bin"
#define MAX_EEPROM_BIN_FILE_SIZE	1024



#define EFUSE_TAG				0x2fe


#ifdef RT_BIG_ENDIAN
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		UINT32            SEL_EFUSE:1;
		UINT32            EFSROM_KICK:1;
		UINT32            RESERVED:4;
		UINT32            EFSROM_AIN:10;
		UINT32            EFSROM_LDO_ON_TIME:2;
		UINT32            EFSROM_LDO_OFF_TIME:6;
		UINT32            EFSROM_MODE:2;
		UINT32            EFSROM_AOUT:6;
	}	field;
	UINT32			word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#else
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		UINT32            EFSROM_AOUT:6;
		UINT32            EFSROM_MODE:2;
		UINT32            EFSROM_LDO_OFF_TIME:6;
		UINT32            EFSROM_LDO_ON_TIME:2;
		UINT32            EFSROM_AIN:10;
		UINT32            RESERVED:4;
		UINT32            EFSROM_KICK:1;
		UINT32            SEL_EFUSE:1;
	}	field;
	UINT32			word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#endif 

static UCHAR eFuseReadRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData);

static VOID eFuseReadPhysical(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUSHORT lpInBuffer,
	IN	ULONG nInBufferSize,
	OUT	PUSHORT lpOutBuffer,
	IN	ULONG nOutBufferSize);

static VOID eFusePhysicalWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData);

static NTSTATUS eFuseWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	IN	USHORT* pData);

static VOID eFuseWritePhysical(
	IN	PRTMP_ADAPTER	pAd,
	PUSHORT lpInBuffer,
	ULONG nInBufferSize,
	PUCHAR lpOutBuffer,
	ULONG nOutBufferSize);


static NTSTATUS eFuseWriteRegistersFromBin(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	IN	USHORT* pData);



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

	RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);

	
	
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	
	eFuseCtrlStruc.field.EFSROM_MODE = 0;

	
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

	
	i = 0;
	while(i < 500)
	{
		
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);
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
		
		efuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC);
		
		
		RTMP_IO_READ32(pAd, efuseDataOffset, &data);
		
		
		
		
		
		
		
		
		
#ifdef RT_BIG_ENDIAN
		data = data << (8*((Offset & 0x3)^0x2));
#else
		data = data >> (8*(Offset & 0x3));
#endif 

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

	RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);

	
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	
	
	eFuseCtrlStruc.field.EFSROM_MODE = 1;

	
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

	
	i = 0;
	while(i < 500)
	{
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);
		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
			break;
		RTMPusecDelay(2);
		i++;
	}

	
	
	
	
	
	
	
	
	efuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC)  ;

	RTMP_IO_READ32(pAd, efuseDataOffset, &data);

#ifdef RT_BIG_ENDIAN
		data = data << (8*((Offset & 0x3)^0x2));
#else
	data = data >> (8*(Offset & 0x3));
#endif 

	NdisMoveMemory(pData, &data, Length);

}


static VOID eFuseReadPhysical(
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
	int		i;

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
	NTSTATUS Status = STATUS_SUCCESS;
	UCHAR	EFSROM_AOUT;
	int	i;

	for(i=0; i<Length; i+=2)
	{
		EFSROM_AOUT = eFuseReadRegisters(pAd, Offset+i, 2, &pOutBuf[i/2]);
	}
	return Status;
}


static VOID eFusePhysicalWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	USHORT	efuseDataOffset;
	UINT32	data, eFuseDataBuffer[4];

	

	
	
	RTMP_IO_READ32(pAd, EFUSE_CTRL,  &eFuseCtrlStruc.word);

	
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	
	eFuseCtrlStruc.field.EFSROM_MODE = 1;

	
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

	
	i = 0;
	while(i < 500)
	{
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);

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
	

	

	RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);

	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	
	eFuseCtrlStruc.field.EFSROM_MODE = 3;

	
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

	
	i = 0;

	while(i < 500)
	{
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);

		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
			break;

		RTMPusecDelay(2);
		i++;
	}
}


static NTSTATUS eFuseWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	IN	USHORT* pData)
{
	USHORT	i,Loop=0;
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
	{	Loop++;
		
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
	while (!bWriteSuccess&&Loop<2);
	if(!bWriteSuccess)
		DBGPRINT(RT_DEBUG_ERROR,("Efsue Write Failed!!\n"));
	return TRUE;
}



static VOID eFuseWritePhysical(
	IN	PRTMP_ADAPTER	pAd,
	PUSHORT lpInBuffer,
	ULONG nInBufferSize,
	PUCHAR lpOutBuffer,
	ULONG nOutBufferSize
)
{
	USHORT* pInBuf = (USHORT*)lpInBuffer;
	int		i;
	
	USHORT Offset = pInBuf[0];					
	USHORT Length = pInBuf[1];					
	USHORT* pValueX = &pInBuf[2];				

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWritePhysical Offset=0x%x, length=%d\n", Offset, Length));

	{
		
		
		
		
		
		
		
		for (i=0; i<Length; i+=2)
		{
			eFusePhysicalWriteRegisters(pAd, Offset+i, 2, &pValueX[i/2]);
		}
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
	IN	PSTRING			arg)
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
	IN	PSTRING			arg)
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
	IN	PSTRING			arg)
{
	PSTRING					src;
	RTMP_OS_FD				srcf;
	RTMP_OS_FS_INFO			osfsInfo;
	INT						retval, memSize;
	PSTRING					buffer, memPtr;
	INT						i = 0,j=0,k=1;
	USHORT					*PDATA;
	USHORT					DATA;

	memSize = 128 + MAX_EEPROM_BIN_FILE_SIZE + sizeof(USHORT) * 8;
	memPtr = kmalloc(memSize, MEM_ALLOC_FLAG);
	if (memPtr == NULL)
		return FALSE;

	NdisZeroMemory(memPtr, memSize);
	src = memPtr; 
	buffer = src + 128;		
	PDATA = (USHORT*)(buffer + MAX_EEPROM_BIN_FILE_SIZE);	

	if(strlen(arg)>0)
		NdisMoveMemory(src, arg, strlen(arg));
	else
		NdisMoveMemory(src, EFUSE_EEPROM_DEFULT_FILE, strlen(EFUSE_EEPROM_DEFULT_FILE));
	DBGPRINT(RT_DEBUG_TRACE, ("FileName=%s\n",src));

	RtmpOSFSInfoChange(&osfsInfo, TRUE);

	srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
	if (IS_FILE_OPEN_ERR(srcf))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("--> Error opening file %s\n", src));
		retval = FALSE;
		goto recoverFS;
	}
	else
	{
		
		while(RtmpOSFileRead(srcf, &buffer[i], 1)==1)
		{
		i++;
			if(i>MAX_EEPROM_BIN_FILE_SIZE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("--> Error reading file %s, file size too large[>%d]\n", src, MAX_EEPROM_BIN_FILE_SIZE));
				retval = FALSE;
				goto closeFile;
			}
		}

		retval = RtmpOSFileClose(srcf);
		if (retval)
			DBGPRINT(RT_DEBUG_TRACE, ("--> Error closing file %s\n", src));
	}


	RtmpOSFSInfoChange(&osfsInfo, FALSE);

	for(j=0;j<i;j++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%02X ",buffer[j]&0xff));
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

	return TRUE;

closeFile:
	if (srcf)
		RtmpOSFileClose(srcf);

recoverFS:
	RtmpOSFSInfoChange(&osfsInfo, FALSE);


	if (memPtr)
		kfree(memPtr);

	return retval;
}


static NTSTATUS eFuseWriteRegistersFromBin(
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
		

		
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);
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
		
		
		
		
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrlStruc.word);

		
		eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

		
		eFuseCtrlStruc.field.EFSROM_MODE = 0;

		
		eFuseCtrlStruc.field.EFSROM_KICK = 1;

		NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
		RTMP_IO_WRITE32(pAd, EFUSE_CTRL, data);

		
		i = 0;
		while(i < 500)
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


int rtmp_ee_efuse_read16(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	OUT USHORT *pValue)
{
	if(pAd->bFroceEEPROMBuffer || pAd->bEEPROMFile)
	{
	    DBGPRINT(RT_DEBUG_TRACE,  ("Read from EEPROM Buffer\n"));
	    NdisMoveMemory(pValue, &(pAd->EEPROMImage[Offset]), 2);
	}
	else
	    eFuseReadRegisters(pAd, Offset, 2, pValue);
	return (*pValue);
}


int rtmp_ee_efuse_write16(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	IN USHORT data)
{
    if(pAd->bFroceEEPROMBuffer||pAd->bEEPROMFile)
    {
        DBGPRINT(RT_DEBUG_TRACE,  ("Write to EEPROM Buffer\n"));
        NdisMoveMemory(&(pAd->EEPROMImage[Offset]), &data, 2);
    }
    else
        eFuseWriteRegisters(pAd, Offset, 2, &data);
	return 0;
}


int RtmpEfuseSupportCheck(
	IN RTMP_ADAPTER *pAd)
{
	USHORT value;

	if (IS_RT30xx(pAd))
	{
		eFusePhysicalReadRegisters(pAd, EFUSE_TAG, 2, &value);
		pAd->EFuseTag = (value & 0xff);
	}
	return 0;
}

INT set_eFuseBufferModeWriteBack_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UINT Enable;


	if(strlen(arg)>0)
	{
		Enable= simple_strtol(arg, 0, 16);
	}
	else
		return FALSE;
	if(Enable==1)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("set_eFuseBufferMode_Proc:: Call WRITEEEPROMBUF"));
		eFuseWriteEeeppromBuf(pAd);
	}
	else
		return FALSE;
	return TRUE;
}



INT eFuseLoadEEPROM(
	IN PRTMP_ADAPTER pAd)
{
	PSTRING					src = NULL;
	INT						retval;
	RTMP_OS_FD				srcf;
	RTMP_OS_FS_INFO			osFSInfo;


	src=EFUSE_BUFFER_PATH;
	DBGPRINT(RT_DEBUG_TRACE, ("FileName=%s\n",src));


	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("--> Error %ld opening %s\n", -PTR_ERR(srcf),src));
			return FALSE;
		}
		else
		{

				memset(pAd->EEPROMImage, 0x00, MAX_EEPROM_BIN_FILE_SIZE);


			retval =RtmpOSFileRead(srcf, (PSTRING)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
			if (retval > 0)
							{
				RTMPSetProfileParameters(pAd, (PSTRING)pAd->EEPROMImage);
				retval = NDIS_STATUS_SUCCESS;
			}
			else
				DBGPRINT(RT_DEBUG_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", src, retval));

		}


	}
	else
		{
					DBGPRINT(RT_DEBUG_ERROR, ("--> Error src  or srcf is null\n"));
					return FALSE;

		}

	retval=RtmpOSFileClose(srcf);

	if (retval)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> Error %d closing %s\n", -retval, src));
	}


	RtmpOSFSInfoChange(&osFSInfo, FALSE);

	return TRUE;
}

INT eFuseWriteEeeppromBuf(
	IN PRTMP_ADAPTER pAd)
{

	PSTRING					src = NULL;
	INT						retval;
	RTMP_OS_FD				srcf;
	RTMP_OS_FS_INFO			osFSInfo;


	src=EFUSE_BUFFER_PATH;
	DBGPRINT(RT_DEBUG_TRACE, ("FileName=%s\n",src));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);



	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_WRONLY|O_CREAT, 0);

		if (IS_FILE_OPEN_ERR(srcf))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("--> Error %ld opening %s\n", -PTR_ERR(srcf),src));
			return FALSE;
		}
		else
		{


			RtmpOSFileWrite(srcf, (PSTRING)pAd->EEPROMImage,MAX_EEPROM_BIN_FILE_SIZE);

		}


	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("--> Error src  or srcf is null\n"));
		return FALSE;

	}

	retval=RtmpOSFileClose(srcf);

	if (retval)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> Error %d closing %s\n", -retval, src));
	}

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;
}


VOID eFuseGetFreeBlockCount(IN PRTMP_ADAPTER pAd,
	PUINT EfuseFreeBlock)
{
	USHORT i;
	USHORT	LogicalAddress;
	if(!pAd->bUseEfuse)
		{
		DBGPRINT(RT_DEBUG_TRACE,("eFuseGetFreeBlockCount Only supports efuse Mode\n"));
		return ;
		}
	for (i = EFUSE_USAGE_MAP_START; i <= EFUSE_USAGE_MAP_END; i+=2)
	{
		eFusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
		if( (LogicalAddress & 0xff) == 0)
		{
			*EfuseFreeBlock= (UCHAR) (EFUSE_USAGE_MAP_END-i+1);
			break;
		}
		else if(( (LogicalAddress >> 8) & 0xff) == 0)
		{
			*EfuseFreeBlock = (UCHAR) (EFUSE_USAGE_MAP_END-i);
			break;
		}

		if(i == EFUSE_USAGE_MAP_END)
			*EfuseFreeBlock = 0;
	}
	DBGPRINT(RT_DEBUG_TRACE,("eFuseGetFreeBlockCount is 0x%x\n",*EfuseFreeBlock));
}

INT eFuse_init(
	IN PRTMP_ADAPTER pAd)
{
	UINT	EfuseFreeBlock=0;
	DBGPRINT(RT_DEBUG_ERROR, ("NVM is Efuse and its size =%x[%x-%x] \n",EFUSE_USAGE_MAP_SIZE,EFUSE_USAGE_MAP_START,EFUSE_USAGE_MAP_END));
	eFuseGetFreeBlockCount(pAd, &EfuseFreeBlock);
	
	
	
	if(EfuseFreeBlock > (EFUSE_USAGE_MAP_END-5))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("NVM is Efuse and the information is too less to bring up interface. Force to use EEPROM Buffer Mode\n"));
		pAd->bFroceEEPROMBuffer = TRUE;
		eFuseLoadEEPROM(pAd);
	}
	else
		pAd->bFroceEEPROMBuffer = FALSE;
	DBGPRINT(RT_DEBUG_TRACE, ("NVM is Efuse and force to use EEPROM Buffer Mode=%x\n",pAd->bFroceEEPROMBuffer));

	return 0;
}
