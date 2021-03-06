

#ifndef __rio_map_h__
#define __rio_map_h__



#define MAX_MAP_ENTRY 17
#define	TOTAL_MAP_ENTRIES (MAX_MAP_ENTRY*RIO_SLOTS)
#define	MAX_NAME_LEN 32

struct Map {
	unsigned int HostUniqueNum;	
	unsigned int RtaUniqueNum;	
	
	unsigned short ID;		
	unsigned short ID2;		
	unsigned long Flags;		
	unsigned long SysPort;		
	struct Top Topology[LINKS_PER_UNIT];	
	char Name[MAX_NAME_LEN];	
};


#define	RTA_BOOTED		0x00000001
#define RTA_NEWBOOT		0x00000010
#define	MSG_DONE		0x00000020
#define	RTA_INTERCONNECT	0x00000040
#define	RTA16_SECOND_SLOT	0x00000080
#define	BEEN_HERE		0x00000100
#define SLOT_TENTATIVE		0x40000000
#define SLOT_IN_USE		0x80000000



#endif
