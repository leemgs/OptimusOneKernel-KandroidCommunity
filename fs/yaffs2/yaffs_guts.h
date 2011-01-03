

#ifndef __YAFFS_GUTS_H__
#define __YAFFS_GUTS_H__

#include "devextras.h"
#include "yportenv.h"

#define YAFFS_OK	1
#define YAFFS_FAIL  0


#define YAFFS_MAGIC			0x5941FF53

#define YAFFS_NTNODES_LEVEL0	  	16
#define YAFFS_TNODES_LEVEL0_BITS	4
#define YAFFS_TNODES_LEVEL0_MASK	0xf

#define YAFFS_NTNODES_INTERNAL 		(YAFFS_NTNODES_LEVEL0 / 2)
#define YAFFS_TNODES_INTERNAL_BITS 	(YAFFS_TNODES_LEVEL0_BITS - 1)
#define YAFFS_TNODES_INTERNAL_MASK	0x7
#define YAFFS_TNODES_MAX_LEVEL		6

#ifndef CONFIG_YAFFS_NO_YAFFS1
#define YAFFS_BYTES_PER_SPARE		16
#define YAFFS_BYTES_PER_CHUNK		512
#define YAFFS_CHUNK_SIZE_SHIFT		9
#define YAFFS_CHUNKS_PER_BLOCK		32
#define YAFFS_BYTES_PER_BLOCK		(YAFFS_CHUNKS_PER_BLOCK*YAFFS_BYTES_PER_CHUNK)
#endif

#define YAFFS_MIN_YAFFS2_CHUNK_SIZE 	1024
#define YAFFS_MIN_YAFFS2_SPARE_SIZE	32

#define YAFFS_MAX_CHUNK_ID		0x000FFFFF

#define YAFFS_UNUSED_OBJECT_ID		0x0003FFFF

#define YAFFS_ALLOCATION_NOBJECTS	100
#define YAFFS_ALLOCATION_NTNODES	100
#define YAFFS_ALLOCATION_NLINKS		100

#define YAFFS_NOBJECT_BUCKETS		256


#define YAFFS_OBJECT_SPACE		0x40000

#define YAFFS_CHECKPOINT_VERSION 	3

#ifdef CONFIG_YAFFS_UNICODE
#define YAFFS_MAX_NAME_LENGTH		127
#define YAFFS_MAX_ALIAS_LENGTH		79
#else
#define YAFFS_MAX_NAME_LENGTH		255
#define YAFFS_MAX_ALIAS_LENGTH		159
#endif

#define YAFFS_SHORT_NAME_LENGTH		15


#define YAFFS_OBJECTID_ROOT		1
#define YAFFS_OBJECTID_LOSTNFOUND	2
#define YAFFS_OBJECTID_UNLINKED		3
#define YAFFS_OBJECTID_DELETED		4


#define YAFFS_OBJECTID_SB_HEADER	0x10
#define YAFFS_OBJECTID_CHECKPOINT_DATA	0x20
#define YAFFS_SEQUENCE_CHECKPOINT_DATA  0x21



#define YAFFS_MAX_SHORT_OP_CACHES	20

#define YAFFS_N_TEMP_BUFFERS		6


#define YAFFS_WR_ATTEMPTS		(5*64)


#define YAFFS_LOWEST_SEQUENCE_NUMBER	0x00001000
#define YAFFS_HIGHEST_SEQUENCE_NUMBER	0xEFFFFF00


#define YAFFS_SEQUENCE_BAD_BLOCK	0xFFFF0000


typedef struct {
	struct yaffs_ObjectStruct *object;
	int chunkId;
	int lastUse;
	int dirty;
	int nBytes;		
	int locked;		
#ifdef CONFIG_YAFFS_YAFFS2
	__u8 *data;
#else
	__u8 data[YAFFS_BYTES_PER_CHUNK];
#endif
} yaffs_ChunkCache;





#ifndef CONFIG_YAFFS_NO_YAFFS1
typedef struct {
	unsigned chunkId:20;
	unsigned serialNumber:2;
	unsigned byteCountLSB:10;
	unsigned objectId:18;
	unsigned ecc:12;
	unsigned byteCountMSB:2;
} yaffs_Tags;

typedef union {
	yaffs_Tags asTags;
	__u8 asBytes[8];
} yaffs_TagsUnion;

#endif



typedef enum {
	YAFFS_ECC_RESULT_UNKNOWN,
	YAFFS_ECC_RESULT_NO_ERROR,
	YAFFS_ECC_RESULT_FIXED,
	YAFFS_ECC_RESULT_UNFIXED
} yaffs_ECCResult;

typedef enum {
	YAFFS_OBJECT_TYPE_UNKNOWN,
	YAFFS_OBJECT_TYPE_FILE,
	YAFFS_OBJECT_TYPE_SYMLINK,
	YAFFS_OBJECT_TYPE_DIRECTORY,
	YAFFS_OBJECT_TYPE_HARDLINK,
	YAFFS_OBJECT_TYPE_SPECIAL
} yaffs_ObjectType;

#define YAFFS_OBJECT_TYPE_MAX YAFFS_OBJECT_TYPE_SPECIAL

typedef struct {

	unsigned validMarker0;
	unsigned chunkUsed;	
	unsigned objectId;	
	unsigned chunkId;	
	unsigned byteCount;	

	
	yaffs_ECCResult eccResult;
	unsigned blockBad;

	
	unsigned chunkDeleted;	
	unsigned serialNumber;	

	
	unsigned sequenceNumber;	

	

	unsigned extraHeaderInfoAvailable;	
	unsigned extraParentObjectId;	
	unsigned extraIsShrinkHeader;	
	unsigned extraShadows;		

	yaffs_ObjectType extraObjectType;	

	unsigned extraFileLength;		
	unsigned extraEquivalentObjectId;	

	unsigned validMarker1;

} yaffs_ExtendedTags;


typedef struct {
	__u8 tagByte0;
	__u8 tagByte1;
	__u8 tagByte2;
	__u8 tagByte3;
	__u8 pageStatus;	
	__u8 blockStatus;
	__u8 tagByte4;
	__u8 tagByte5;
	__u8 ecc1[3];
	__u8 tagByte6;
	__u8 tagByte7;
	__u8 ecc2[3];
} yaffs_Spare;


struct yaffs_NANDSpare {
	yaffs_Spare spare;
	int eccres1;
	int eccres2;
};



typedef enum {
	YAFFS_BLOCK_STATE_UNKNOWN = 0,

	YAFFS_BLOCK_STATE_SCANNING,
	YAFFS_BLOCK_STATE_NEEDS_SCANNING,
	

	YAFFS_BLOCK_STATE_EMPTY,
	

	YAFFS_BLOCK_STATE_ALLOCATING,
	

	YAFFS_BLOCK_STATE_FULL,
	

	YAFFS_BLOCK_STATE_DIRTY,
	

	YAFFS_BLOCK_STATE_CHECKPOINT,
	

	YAFFS_BLOCK_STATE_COLLECTING,
	

	YAFFS_BLOCK_STATE_DEAD
	
} yaffs_BlockState;

#define	YAFFS_NUMBER_OF_BLOCK_STATES (YAFFS_BLOCK_STATE_DEAD + 1)


typedef struct {

	int softDeletions:10;	
	int pagesInUse:10;	
	unsigned blockState:4;	
	__u32 needsRetiring:1;	
				
	__u32 skipErasedCheck:1; 
	__u32 gcPrioritise:1; 	
	__u32 chunkErrorStrikes:3; 

#ifdef CONFIG_YAFFS_YAFFS2
	__u32 hasShrinkHeader:1; 
	__u32 sequenceNumber;	 
#endif

} yaffs_BlockInfo;




typedef struct {
	yaffs_ObjectType type;

	
	int parentObjectId;
	__u16 sum__NoLongerUsed;        
	YCHAR name[YAFFS_MAX_NAME_LENGTH + 1];

	
	__u32 yst_mode;         

#ifdef CONFIG_YAFFS_WINCE
	__u32 notForWinCE[5];
#else
	__u32 yst_uid;
	__u32 yst_gid;
	__u32 yst_atime;
	__u32 yst_mtime;
	__u32 yst_ctime;
#endif

	
	int fileSize;

	
	int equivalentObjectId;

	
	YCHAR alias[YAFFS_MAX_ALIAS_LENGTH + 1];

	__u32 yst_rdev;		

#ifdef CONFIG_YAFFS_WINCE
	__u32 win_ctime[2];
	__u32 win_atime[2];
	__u32 win_mtime[2];
#else
	__u32 roomToGrow[6];

#endif
	__u32 inbandShadowsObject;
	__u32 inbandIsShrink;

	__u32 reservedSpace[2];
	int shadowsObject;	

	
	__u32 isShrink;

} yaffs_ObjectHeader;



union yaffs_Tnode_union {
#ifdef CONFIG_YAFFS_TNODE_LIST_DEBUG
	union yaffs_Tnode_union *internal[YAFFS_NTNODES_INTERNAL + 1];
#else
	union yaffs_Tnode_union *internal[YAFFS_NTNODES_INTERNAL];
#endif


};

typedef union yaffs_Tnode_union yaffs_Tnode;

struct yaffs_TnodeList_struct {
	struct yaffs_TnodeList_struct *next;
	yaffs_Tnode *tnodes;
};

typedef struct yaffs_TnodeList_struct yaffs_TnodeList;




typedef struct {
	__u32 fileSize;
	__u32 scannedFileSize;
	__u32 shrinkSize;
	int topLevel;
	yaffs_Tnode *top;
} yaffs_FileStructure;

typedef struct {
	struct ylist_head children;     
} yaffs_DirectoryStructure;

typedef struct {
	YCHAR *alias;
} yaffs_SymLinkStructure;

typedef struct {
	struct yaffs_ObjectStruct *equivalentObject;
	__u32 equivalentObjectId;
} yaffs_HardLinkStructure;

typedef union {
	yaffs_FileStructure fileVariant;
	yaffs_DirectoryStructure directoryVariant;
	yaffs_SymLinkStructure symLinkVariant;
	yaffs_HardLinkStructure hardLinkVariant;
} yaffs_ObjectVariant;

struct yaffs_ObjectStruct {
	__u8 deleted:1;		
	__u8 softDeleted:1;	
	__u8 unlinked:1;	
	__u8 fake:1;		
	__u8 renameAllowed:1;	
	__u8 unlinkAllowed:1;
	__u8 dirty:1;		
	__u8 valid:1;		
	__u8 lazyLoaded:1;	

	__u8 deferedFree:1;	
	__u8 beingCreated:1;	
	__u8 isShadowed:1;      

	__u8 serial;		
	__u16 sum;		

	struct yaffs_DeviceStruct *myDev;       

	struct ylist_head hashLink;     

	struct ylist_head hardLinks;    

	
	
	struct yaffs_ObjectStruct *parent;
	struct ylist_head siblings;

	
	int hdrChunk;

	int nDataChunks;	

	__u32 objectId;		

	__u32 yst_mode;

#ifdef CONFIG_YAFFS_SHORT_NAMES_IN_RAM
	YCHAR shortName[YAFFS_SHORT_NAME_LENGTH + 1];
#endif

#ifndef __KERNEL__
	__u32 inUse;
#endif

#ifdef CONFIG_YAFFS_WINCE
	__u32 win_ctime[2];
	__u32 win_mtime[2];
	__u32 win_atime[2];
#else
	__u32 yst_uid;
	__u32 yst_gid;
	__u32 yst_atime;
	__u32 yst_mtime;
	__u32 yst_ctime;
#endif

	__u32 yst_rdev;

#ifdef __KERNEL__
	struct inode *myInode;

#endif

	yaffs_ObjectType variantType;

	yaffs_ObjectVariant variant;

};

typedef struct yaffs_ObjectStruct yaffs_Object;

struct yaffs_ObjectList_struct {
	yaffs_Object *objects;
	struct yaffs_ObjectList_struct *next;
};

typedef struct yaffs_ObjectList_struct yaffs_ObjectList;

typedef struct {
	struct ylist_head list;
	int count;
} yaffs_ObjectBucket;




typedef struct {
	int structType;
	__u32 objectId;
	__u32 parentId;
	int hdrChunk;
	yaffs_ObjectType variantType:3;
	__u8 deleted:1;
	__u8 softDeleted:1;
	__u8 unlinked:1;
	__u8 fake:1;
	__u8 renameAllowed:1;
	__u8 unlinkAllowed:1;
	__u8 serial;

	int nDataChunks;
	__u32 fileSizeOrEquivalentObjectId;
} yaffs_CheckpointObject;



typedef struct {
	__u8 *buffer;
	int line;	
	int maxLine;
} yaffs_TempBuffer;



struct yaffs_DeviceStruct {
	struct ylist_head devList;
	const char *name;

	
	int nDataBytesPerChunk;	
	int nChunksPerBlock;	
	int spareBytesPerChunk;	
	int startBlock;		
	int endBlock;		
	int nReservedBlocks;	
				


	
	

 


	int nShortOpCaches;	

	int useHeaderFileSize;	

	int emptyLostAndFound;  

	int useNANDECC;		

	void *genericDevice;	
	void *superBlock;

	

	int (*writeChunkToNAND) (struct yaffs_DeviceStruct *dev,
					int chunkInNAND, const __u8 *data,
					const yaffs_Spare *spare);
	int (*readChunkFromNAND) (struct yaffs_DeviceStruct *dev,
					int chunkInNAND, __u8 *data,
					yaffs_Spare *spare);
	int (*eraseBlockInNAND) (struct yaffs_DeviceStruct *dev,
					int blockInNAND);
	int (*initialiseNAND) (struct yaffs_DeviceStruct *dev);
	int (*deinitialiseNAND) (struct yaffs_DeviceStruct *dev);

#ifdef CONFIG_YAFFS_YAFFS2
	int (*writeChunkWithTagsToNAND) (struct yaffs_DeviceStruct *dev,
					 int chunkInNAND, const __u8 *data,
					 const yaffs_ExtendedTags *tags);
	int (*readChunkWithTagsFromNAND) (struct yaffs_DeviceStruct *dev,
					  int chunkInNAND, __u8 *data,
					  yaffs_ExtendedTags *tags);
	int (*markNANDBlockBad) (struct yaffs_DeviceStruct *dev, int blockNo);
	int (*queryNANDBlock) (struct yaffs_DeviceStruct *dev, int blockNo,
			       yaffs_BlockState *state, __u32 *sequenceNumber);
#endif

	int isYaffs2;

	
	void (*removeObjectCallback)(struct yaffs_ObjectStruct *obj);

	
	void (*markSuperBlockDirty)(void *superblock);

	int wideTnodesDisabled; 

	YCHAR *pathDividers;	


	

	
	__u8 skipCheckpointRead;
	__u8 skipCheckpointWrite;

	

	__u16 chunkGroupBits;	
	__u16 chunkGroupSize;	

	
	__u32 tnodeWidth;
	__u32 tnodeMask;

	
	__u32 chunkShift; 
	__u32 chunkDiv;   
	__u32 chunkMask;  

	
	int inbandTags;
	__u32 totalBytesPerChunk;

#ifdef __KERNEL__

	struct semaphore sem;	
	struct semaphore grossLock;	
	struct rw_semaphore dirLock; 
	__u8 *spareBuffer;	
	void (*putSuperFunc) (struct super_block *sb);
        struct ylist_head searchContexts;

#endif

	int isMounted;

	int isCheckpointed;


	
	int internalStartBlock;
	int internalEndBlock;
	int blockOffset;
	int chunkOffset;


	
	int checkpointPageSequence;   
	int checkpointByteCount;
	int checkpointByteOffset;
	__u8 *checkpointBuffer;
	int checkpointOpenForWrite;
	int blocksInCheckpoint;
	int checkpointCurrentChunk;
	int checkpointCurrentBlock;
	int checkpointNextBlock;
	int *checkpointBlockList;
	int checkpointMaxBlocks;
	__u32 checkpointSum;
	__u32 checkpointXor;

	int nCheckpointBlocksRequired; 

	
	yaffs_BlockInfo *blockInfo;
	__u8 *chunkBits;	
	unsigned blockInfoAlt:1;	
	unsigned chunkBitsAlt:1;	
	int chunkBitmapStride;	

	int nErasedBlocks;
	int allocationBlock;	
	__u32 allocationPage;
	int allocationBlockFinder;	

	
	int nTnodesCreated;
	yaffs_Tnode *freeTnodes;
	int nFreeTnodes;
	yaffs_TnodeList *allocatedTnodeList;

	int isDoingGC;
	int gcBlock;
	int gcChunk;

	int nObjectsCreated;
	yaffs_Object *freeObjects;
	int nFreeObjects;

	int nHardLinks;

	yaffs_ObjectList *allocatedObjectList;

	yaffs_ObjectBucket objectBucket[YAFFS_NOBJECT_BUCKETS];

	int nFreeChunks;

	int currentDirtyChecker;	

	__u32 *gcCleanupList;	
	int nonAggressiveSkip;	

	
	int nPageWrites;
	int nPageReads;
	int nBlockErasures;
	int nErasureFailures;
	int nGCCopies;
	int garbageCollections;
	int passiveGarbageCollections;
	int nRetriedWrites;
	int nRetiredBlocks;
	int eccFixed;
	int eccUnfixed;
	int tagsEccFixed;
	int tagsEccUnfixed;
	int nDeletions;
	int nUnmarkedDeletions;

	int hasPendingPrioritisedGCs; 

	
	yaffs_Object *rootDir;
	yaffs_Object *lostNFoundDir;

	

	int bufferedBlock;	
	int doingBufferedBlockRewrite;

	yaffs_ChunkCache *srCache;
	int srLastUse;

	int cacheHits;

	
	yaffs_Object *unlinkedDir;	
	yaffs_Object *deletedDir;	
	yaffs_Object *unlinkedDeletion;	
	int nDeletedFiles;		
	int nUnlinkedFiles;		
	int nBackgroundDeletions;	


	
	yaffs_TempBuffer tempBuffer[YAFFS_N_TEMP_BUFFERS];
	int maxTemp;
	int tempInUse;
	int unmanagedTempAllocations;
	int unmanagedTempDeallocations;

	
	unsigned sequenceNumber;	
	unsigned oldestDirtySequence;

};

typedef struct yaffs_DeviceStruct yaffs_Device;


typedef struct {
	int StructType;
	int version;
	int checkpointStartBlock;
	int checkpointEndBlock;
	int startBlock;
	int endBlock;
	int rfu[100];
} yaffs_SuperBlockHeader;


typedef struct {
	int structType;
	int nErasedBlocks;
	int allocationBlock;	
	__u32 allocationPage;
	int nFreeChunks;

	int nDeletedFiles;		
	int nUnlinkedFiles;		
	int nBackgroundDeletions;	

	
	unsigned sequenceNumber;	
	unsigned oldestDirtySequence;

} yaffs_CheckpointDevice;


typedef struct {
	int structType;
	__u32 magic;
	__u32 version;
	__u32 head;
} yaffs_CheckpointValidity;




int yaffs_GutsInitialise(yaffs_Device *dev);
void yaffs_Deinitialise(yaffs_Device *dev);

int yaffs_GetNumberOfFreeChunks(yaffs_Device *dev);

int yaffs_RenameObject(yaffs_Object *oldDir, const YCHAR *oldName,
		       yaffs_Object *newDir, const YCHAR *newName);

int yaffs_Unlink(yaffs_Object *dir, const YCHAR *name);
int yaffs_DeleteObject(yaffs_Object *obj);

int yaffs_GetObjectName(yaffs_Object *obj, YCHAR *name, int buffSize);
int yaffs_GetObjectFileLength(yaffs_Object *obj);
int yaffs_GetObjectInode(yaffs_Object *obj);
unsigned yaffs_GetObjectType(yaffs_Object *obj);
int yaffs_GetObjectLinkCount(yaffs_Object *obj);

int yaffs_SetAttributes(yaffs_Object *obj, struct iattr *attr);
int yaffs_GetAttributes(yaffs_Object *obj, struct iattr *attr);


int yaffs_ReadDataFromFile(yaffs_Object *obj, __u8 *buffer, loff_t offset,
				int nBytes);
int yaffs_WriteDataToFile(yaffs_Object *obj, const __u8 *buffer, loff_t offset,
				int nBytes, int writeThrough);
int yaffs_ResizeFile(yaffs_Object *obj, loff_t newSize);

yaffs_Object *yaffs_MknodFile(yaffs_Object *parent, const YCHAR *name,
				__u32 mode, __u32 uid, __u32 gid);
int yaffs_FlushFile(yaffs_Object *obj, int updateTime);


void yaffs_FlushEntireDeviceCache(yaffs_Device *dev);

int yaffs_CheckpointSave(yaffs_Device *dev);
int yaffs_CheckpointRestore(yaffs_Device *dev);


yaffs_Object *yaffs_MknodDirectory(yaffs_Object *parent, const YCHAR *name,
				__u32 mode, __u32 uid, __u32 gid);
yaffs_Object *yaffs_FindObjectByName(yaffs_Object *theDir, const YCHAR *name);
int yaffs_ApplyToDirectoryChildren(yaffs_Object *theDir,
				   int (*fn) (yaffs_Object *));

yaffs_Object *yaffs_FindObjectByNumber(yaffs_Device *dev, __u32 number);


yaffs_Object *yaffs_Link(yaffs_Object *parent, const YCHAR *name,
			 yaffs_Object *equivalentObject);

yaffs_Object *yaffs_GetEquivalentObject(yaffs_Object *obj);


yaffs_Object *yaffs_MknodSymLink(yaffs_Object *parent, const YCHAR *name,
				 __u32 mode, __u32 uid, __u32 gid,
				 const YCHAR *alias);
YCHAR *yaffs_GetSymlinkAlias(yaffs_Object *obj);


yaffs_Object *yaffs_MknodSpecial(yaffs_Object *parent, const YCHAR *name,
				 __u32 mode, __u32 uid, __u32 gid, __u32 rdev);


yaffs_Object *yaffs_Root(yaffs_Device *dev);
yaffs_Object *yaffs_LostNFound(yaffs_Device *dev);

#ifdef CONFIG_YAFFS_WINCE

void yfsd_WinFileTimeNow(__u32 target[2]);
#endif

#ifdef __KERNEL__

void yaffs_HandleDeferedFree(yaffs_Object *obj);
#endif


int yaffs_DumpObject(yaffs_Object *obj);

void yaffs_GutsTest(yaffs_Device *dev);


void yaffs_InitialiseTags(yaffs_ExtendedTags *tags);
void yaffs_DeleteChunk(yaffs_Device *dev, int chunkId, int markNAND, int lyn);
int yaffs_CheckFF(__u8 *buffer, int nBytes);
void yaffs_HandleChunkError(yaffs_Device *dev, yaffs_BlockInfo *bi);

__u8 *yaffs_GetTempBuffer(yaffs_Device *dev, int lineNo);
void yaffs_ReleaseTempBuffer(yaffs_Device *dev, __u8 *buffer, int lineNo);

#endif
