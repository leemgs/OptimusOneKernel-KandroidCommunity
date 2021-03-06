

#ifndef _UPT1_DEFS_H
#define _UPT1_DEFS_H

struct UPT1_TxStats {
	u64			TSOPktsTxOK;  
	u64			TSOBytesTxOK;
	u64			ucastPktsTxOK;
	u64			ucastBytesTxOK;
	u64			mcastPktsTxOK;
	u64			mcastBytesTxOK;
	u64			bcastPktsTxOK;
	u64			bcastBytesTxOK;
	u64			pktsTxError;
	u64			pktsTxDiscard;
};

struct UPT1_RxStats {
	u64			LROPktsRxOK;    
	u64			LROBytesRxOK;   
	
	u64			ucastPktsRxOK;
	u64			ucastBytesRxOK;
	u64			mcastPktsRxOK;
	u64			mcastBytesRxOK;
	u64			bcastPktsRxOK;
	u64			bcastBytesRxOK;
	u64			pktsRxOutOfBuf;
	u64			pktsRxError;
};


enum {
	UPT1_IML_NONE		= 0, 
	UPT1_IML_HIGHEST	= 7, 
	UPT1_IML_ADAPTIVE	= 8, 
};

enum {
	UPT1_RSS_HASH_TYPE_NONE      = 0x0,
	UPT1_RSS_HASH_TYPE_IPV4      = 0x01,
	UPT1_RSS_HASH_TYPE_TCP_IPV4  = 0x02,
	UPT1_RSS_HASH_TYPE_IPV6      = 0x04,
	UPT1_RSS_HASH_TYPE_TCP_IPV6  = 0x08,
};

enum {
	UPT1_RSS_HASH_FUNC_NONE      = 0x0,
	UPT1_RSS_HASH_FUNC_TOEPLITZ  = 0x01,
};

#define UPT1_RSS_MAX_KEY_SIZE        40
#define UPT1_RSS_MAX_IND_TABLE_SIZE  128

struct UPT1_RSSConf {
	u16			hashType;
	u16			hashFunc;
	u16			hashKeySize;
	u16			indTableSize;
	u8			hashKey[UPT1_RSS_MAX_KEY_SIZE];
	u8			indTable[UPT1_RSS_MAX_IND_TABLE_SIZE];
};


enum {
	UPT1_F_RXCSUM		= 0x0001,   
	UPT1_F_RSS		= 0x0002,
	UPT1_F_RXVLAN		= 0x0004,   
	UPT1_F_LRO		= 0x0008,
};
#endif
