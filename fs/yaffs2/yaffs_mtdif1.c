



#include "yportenv.h"
#include "yaffs_guts.h"
#include "yaffs_packedtags1.h"
#include "yaffs_tagscompat.h"	

#include "linux/kernel.h"
#include "linux/version.h"
#include "linux/types.h"
#include "linux/mtd/mtd.h"


#if (MTD_VERSION_CODE > MTD_VERSION(2, 6, 17))

const char *yaffs_mtdif1_c_version = "$Id$";

#ifndef CONFIG_YAFFS_9BYTE_TAGS
# define YTAG1_SIZE 8
#else
# define YTAG1_SIZE 9
#endif

#if 0

static struct nand_ecclayout nand_oob_16 = {
	.eccbytes = 6,
	.eccpos = { 8, 9, 10, 13, 14, 15 },
	.oobavail = 9,
	.oobfree = { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};
#endif


int nandmtd1_WriteChunkWithTagsToNAND(yaffs_Device *dev,
	int chunkInNAND, const __u8 *data, const yaffs_ExtendedTags *etags)
{
	struct mtd_info *mtd = dev->genericDevice;
	int chunkBytes = dev->nDataBytesPerChunk;
	loff_t addr = ((loff_t)chunkInNAND) * chunkBytes;
	struct mtd_oob_ops ops;
	yaffs_PackedTags1 pt1;
	int retval;

	
	compile_time_assertion(sizeof(yaffs_PackedTags1) == 12);
	compile_time_assertion(sizeof(yaffs_Tags) == 8);

	yaffs_PackTags1(&pt1, etags);
	yaffs_CalcTagsECC((yaffs_Tags *)&pt1);

	
#ifndef CONFIG_YAFFS_9BYTE_TAGS
	if (etags->chunkDeleted) {
		memset(&pt1, 0xff, 8);
		
		pt1.deleted = 0;
	}
#else
	((__u8 *)&pt1)[8] = 0xff;
	if (etags->chunkDeleted) {
		memset(&pt1, 0xff, 8);
		
		((__u8 *)&pt1)[8] = 0;
	}
#endif

	memset(&ops, 0, sizeof(ops));
	ops.mode = MTD_OOB_AUTO;
	ops.len = (data) ? chunkBytes : 0;
	ops.ooblen = YTAG1_SIZE;
	ops.datbuf = (__u8 *)data;
	ops.oobbuf = (__u8 *)&pt1;

	retval = mtd->write_oob(mtd, addr, &ops);
	if (retval) {
		yaffs_trace(YAFFS_TRACE_MTD,
			"write_oob failed, chunk %d, mtd error %d\n",
			chunkInNAND, retval);
	}
	return retval ? YAFFS_FAIL : YAFFS_OK;
}


static int rettags(yaffs_ExtendedTags *etags, int eccResult, int retval)
{
	if (etags) {
		memset(etags, 0, sizeof(*etags));
		etags->eccResult = eccResult;
	}
	return retval;
}


int nandmtd1_ReadChunkWithTagsFromNAND(yaffs_Device *dev,
	int chunkInNAND, __u8 *data, yaffs_ExtendedTags *etags)
{
	struct mtd_info *mtd = dev->genericDevice;
	int chunkBytes = dev->nDataBytesPerChunk;
	loff_t addr = ((loff_t)chunkInNAND) * chunkBytes;
	int eccres = YAFFS_ECC_RESULT_NO_ERROR;
	struct mtd_oob_ops ops;
	yaffs_PackedTags1 pt1;
	int retval;
	int deleted;

	memset(&ops, 0, sizeof(ops));
	ops.mode = MTD_OOB_AUTO;
	ops.len = (data) ? chunkBytes : 0;
	ops.ooblen = YTAG1_SIZE;
	ops.datbuf = data;
	ops.oobbuf = (__u8 *)&pt1;

#if (MTD_VERSION_CODE < MTD_VERSION(2, 6, 20))
	
	ops.len = (ops.datbuf) ? ops.len : ops.ooblen;
#endif
	
	retval = mtd->read_oob(mtd, addr, &ops);
	if (retval) {
		yaffs_trace(YAFFS_TRACE_MTD,
			"read_oob failed, chunk %d, mtd error %d\n",
			chunkInNAND, retval);
	}

	switch (retval) {
	case 0:
		
		break;

	case -EUCLEAN:
		
		eccres = YAFFS_ECC_RESULT_FIXED;
		dev->eccFixed++;
		break;

	case -EBADMSG:
		
		dev->eccUnfixed++;
		
	default:
		rettags(etags, YAFFS_ECC_RESULT_UNFIXED, 0);
		etags->blockBad = (mtd->block_isbad)(mtd, addr);
		return YAFFS_FAIL;
	}

	
	if (yaffs_CheckFF((__u8 *)&pt1, 8)) {
		
		return rettags(etags, YAFFS_ECC_RESULT_NO_ERROR, YAFFS_OK);
	}

#ifndef CONFIG_YAFFS_9BYTE_TAGS
	
	deleted = !pt1.deleted;
	pt1.deleted = 1;
#else
	deleted = (yaffs_CountBits(((__u8 *)&pt1)[8]) < 7);
#endif

	
	retval = yaffs_CheckECCOnTags((yaffs_Tags *)&pt1);
	switch (retval) {
	case 0:
		
		break;
	case 1:
		
		dev->tagsEccFixed++;
		if (eccres == YAFFS_ECC_RESULT_NO_ERROR)
			eccres = YAFFS_ECC_RESULT_FIXED;
		break;
	default:
		
		dev->tagsEccUnfixed++;
		return rettags(etags, YAFFS_ECC_RESULT_UNFIXED, YAFFS_FAIL);
	}

	
	pt1.shouldBeFF = 0xFFFFFFFF;
	yaffs_UnpackTags1(etags, &pt1);
	etags->eccResult = eccres;

	
	etags->chunkDeleted = deleted;
	return YAFFS_OK;
}


int nandmtd1_MarkNANDBlockBad(struct yaffs_DeviceStruct *dev, int blockNo)
{
	struct mtd_info *mtd = dev->genericDevice;
	int blocksize = dev->nChunksPerBlock * dev->nDataBytesPerChunk;
	int retval;

	yaffs_trace(YAFFS_TRACE_BAD_BLOCKS, "marking block %d bad\n", blockNo);

	retval = mtd->block_markbad(mtd, (loff_t)blocksize * blockNo);
	return (retval) ? YAFFS_FAIL : YAFFS_OK;
}


static int nandmtd1_TestPrerequists(struct mtd_info *mtd)
{
	
	
	int oobavail = mtd->ecclayout->oobavail;

	if (oobavail < YTAG1_SIZE) {
		yaffs_trace(YAFFS_TRACE_ERROR,
			"mtd device has only %d bytes for tags, need %d\n",
			oobavail, YTAG1_SIZE);
		return YAFFS_FAIL;
	}
	return YAFFS_OK;
}


int nandmtd1_QueryNANDBlock(struct yaffs_DeviceStruct *dev, int blockNo,
	yaffs_BlockState *pState, __u32 *pSequenceNumber)
{
	struct mtd_info *mtd = dev->genericDevice;
	int chunkNo = blockNo * dev->nChunksPerBlock;
	loff_t addr = (loff_t)chunkNo * dev->nDataBytesPerChunk;
	yaffs_ExtendedTags etags;
	int state = YAFFS_BLOCK_STATE_DEAD;
	int seqnum = 0;
	int retval;

	
	if (nandmtd1_TestPrerequists(mtd) != YAFFS_OK)
		return YAFFS_FAIL;

	retval = nandmtd1_ReadChunkWithTagsFromNAND(dev, chunkNo, NULL, &etags);
	etags.blockBad = (mtd->block_isbad)(mtd, addr);
	if (etags.blockBad) {
		yaffs_trace(YAFFS_TRACE_BAD_BLOCKS,
			"block %d is marked bad\n", blockNo);
		state = YAFFS_BLOCK_STATE_DEAD;
	} else if (etags.eccResult != YAFFS_ECC_RESULT_NO_ERROR) {
		
		state = YAFFS_BLOCK_STATE_NEEDS_SCANNING;
	} else if (etags.chunkUsed) {
		state = YAFFS_BLOCK_STATE_NEEDS_SCANNING;
		seqnum = etags.sequenceNumber;
	} else {
		state = YAFFS_BLOCK_STATE_EMPTY;
	}

	*pState = state;
	*pSequenceNumber = seqnum;

	
	return YAFFS_OK;
}

#endif 
