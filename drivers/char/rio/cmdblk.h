

#ifndef __rio_cmdblk_h__
#define __rio_cmdblk_h__



struct CmdBlk {
	struct CmdBlk *NextP;	
	struct PKT Packet;	
	
	int (*PreFuncP) (unsigned long, struct CmdBlk *);
	int PreArg;		
	
	int (*PostFuncP) (unsigned long, struct CmdBlk *);
	int PostArg;		
};

#define NUM_RIO_CMD_BLKS (3 * (MAX_RUP * 4 + LINKS_PER_UNIT * 4))
#endif
