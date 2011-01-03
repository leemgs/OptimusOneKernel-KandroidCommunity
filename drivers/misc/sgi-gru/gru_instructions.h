

#ifndef __GRU_INSTRUCTIONS_H__
#define __GRU_INSTRUCTIONS_H__

extern int gru_check_status_proc(void *cb);
extern int gru_wait_proc(void *cb);
extern void gru_wait_abort_proc(void *cb);





#if defined(CONFIG_IA64)
#include <linux/compiler.h>
#include <asm/intrinsics.h>
#define __flush_cache(p)		ia64_fc((unsigned long)p)

#define gru_ordered_store_int(p, v)					\
		do {							\
			barrier();					\
			*((volatile int *)(p)) = v; 	\
		} while (0)
#elif defined(CONFIG_X86_64)
#define __flush_cache(p)		clflush(p)
#define gru_ordered_store_int(p, v)					\
		do {							\
			barrier();					\
			*(int *)p = v;					\
		} while (0)
#else
#error "Unsupported architecture"
#endif


#define CBS_IDLE			0
#define CBS_EXCEPTION			1
#define CBS_ACTIVE			2
#define CBS_CALL_OS			3


#define CBSS_MSG_QUEUE_MASK		7
#define CBSS_IMPLICIT_ABORT_ACTIVE_MASK	8


#define CBSS_NO_ERROR			0
#define CBSS_LB_OVERFLOWED		1
#define CBSS_QLIMIT_REACHED		2
#define CBSS_PAGE_OVERFLOW		3
#define CBSS_AMO_NACKED			4
#define CBSS_PUT_NACKED			5


struct control_block_extended_exc_detail {
	unsigned long	cb;
	int		opc;
	int		ecause;
	int		exopc;
	long		exceptdet0;
	int		exceptdet1;
	int		cbrstate;
	int		cbrexecstatus;
};




struct gru_instruction_bits {
    
    unsigned int		icmd:      1;
    unsigned char		ima:	   3;	
    unsigned char		reserved0: 4;
    unsigned int		xtype:     3;
    unsigned int		iaa0:      2;
    unsigned int		iaa1:      2;
    unsigned char		reserved1: 1;
    unsigned char		opc:       8;	
    unsigned char		exopc:     8;	
    
    unsigned int		idef2:    22;	
    unsigned char		reserved2: 2;
    unsigned char		istatus:   2;
    unsigned char		isubstatus:4;
    unsigned char		reserved3: 1;
    unsigned char		tlb_fault_color: 1;
    
    unsigned long		idef4;		
    
    unsigned long		idef1;		
    unsigned long		idef5;		
    unsigned long		idef6;		
    unsigned long		idef3;		
    unsigned long		reserved4;
    
    unsigned long		avalue;		 
};


struct gru_instruction {
    
    unsigned int		op32;    
    unsigned int		tri0;
    unsigned long		tri1_bufsize;		
    unsigned long		baddr0;			
    unsigned long		nelem;			
    unsigned long		op1_stride;		
    unsigned long		op2_value_baddr1;	
    unsigned long		reserved0;		
    unsigned long		avalue;			
};


#define GRU_CB_ICMD_SHFT	0
#define GRU_CB_ICMD_MASK	0x1
#define GRU_CB_XTYPE_SHFT	8
#define GRU_CB_XTYPE_MASK	0x7
#define GRU_CB_IAA0_SHFT	11
#define GRU_CB_IAA0_MASK	0x3
#define GRU_CB_IAA1_SHFT	13
#define GRU_CB_IAA1_MASK	0x3
#define GRU_CB_IMA_SHFT		1
#define GRU_CB_IMA_MASK		0x3
#define GRU_CB_OPC_SHFT		16
#define GRU_CB_OPC_MASK		0xff
#define GRU_CB_EXOPC_SHFT	24
#define GRU_CB_EXOPC_MASK	0xff


#define OP_NOP		0x00
#define OP_BCOPY	0x01
#define OP_VLOAD	0x02
#define OP_IVLOAD	0x03
#define OP_VSTORE	0x04
#define OP_IVSTORE	0x05
#define OP_VSET		0x06
#define OP_IVSET	0x07
#define OP_MESQ		0x08
#define OP_GAMXR	0x09
#define OP_GAMIR	0x0a
#define OP_GAMIRR	0x0b
#define OP_GAMER	0x0c
#define OP_GAMERR	0x0d
#define OP_BSTORE	0x0e
#define OP_VFLUSH	0x0f





#define EOP_IR_FETCH	0x01 
#define EOP_IR_CLR	0x02 
#define EOP_IR_INC	0x05 
#define EOP_IR_DEC	0x07 
#define EOP_IR_QCHK1	0x0d 
#define EOP_IR_QCHK2	0x0e 


#define EOP_IRR_FETCH	0x01 
#define EOP_IRR_CLR	0x02 
#define EOP_IRR_INC	0x05 
#define EOP_IRR_DEC	0x07 
#define EOP_IRR_DECZ	0x0f 


#define EOP_ER_SWAP	0x00 
#define EOP_ER_OR	0x01 
#define EOP_ER_AND	0x02 
#define EOP_ER_XOR	0x03 
#define EOP_ER_ADD	0x04 
#define EOP_ER_CSWAP	0x08 
#define EOP_ER_CADD	0x0c 


#define EOP_ERR_SWAP	0x00 
#define EOP_ERR_OR	0x01 
#define EOP_ERR_AND	0x02 
#define EOP_ERR_XOR	0x03 
#define EOP_ERR_ADD	0x04 
#define EOP_ERR_CSWAP	0x08 
#define EOP_ERR_EPOLL	0x09 
#define EOP_ERR_NPOLL	0x0a 


#define EOP_XR_CSWAP	0x0b 



#define XTYPE_B		0x0	
#define XTYPE_S		0x1	
#define XTYPE_W		0x2	
#define XTYPE_DW	0x3	
#define XTYPE_CL	0x6	



#define IAA_RAM		0x0	
#define IAA_NCRAM	0x2	
#define IAA_MMIO	0x1	
#define IAA_REGISTER	0x3	



#define IMA_MAPPED	0x0	
#define IMA_CB_DELAY	0x1	
#define IMA_UNMAPPED	0x2	
#define IMA_INTERRUPT	0x4	


#define CBE_CAUSE_RI				(1 << 0)
#define CBE_CAUSE_INVALID_INSTRUCTION		(1 << 1)
#define CBE_CAUSE_UNMAPPED_MODE_FORBIDDEN	(1 << 2)
#define CBE_CAUSE_PE_CHECK_DATA_ERROR		(1 << 3)
#define CBE_CAUSE_IAA_GAA_MISMATCH		(1 << 4)
#define CBE_CAUSE_DATA_SEGMENT_LIMIT_EXCEPTION	(1 << 5)
#define CBE_CAUSE_OS_FATAL_TLB_FAULT		(1 << 6)
#define CBE_CAUSE_EXECUTION_HW_ERROR		(1 << 7)
#define CBE_CAUSE_TLBHW_ERROR			(1 << 8)
#define CBE_CAUSE_RA_REQUEST_TIMEOUT		(1 << 9)
#define CBE_CAUSE_HA_REQUEST_TIMEOUT		(1 << 10)
#define CBE_CAUSE_RA_RESPONSE_FATAL		(1 << 11)
#define CBE_CAUSE_RA_RESPONSE_NON_FATAL		(1 << 12)
#define CBE_CAUSE_HA_RESPONSE_FATAL		(1 << 13)
#define CBE_CAUSE_HA_RESPONSE_NON_FATAL		(1 << 14)
#define CBE_CAUSE_ADDRESS_SPACE_DECODE_ERROR	(1 << 15)
#define CBE_CAUSE_PROTOCOL_STATE_DATA_ERROR	(1 << 16)
#define CBE_CAUSE_RA_RESPONSE_DATA_ERROR	(1 << 17)
#define CBE_CAUSE_HA_RESPONSE_DATA_ERROR	(1 << 18)


#define CBR_EXS_ABORT_OCC_BIT			0
#define CBR_EXS_INT_OCC_BIT			1
#define CBR_EXS_PENDING_BIT			2
#define CBR_EXS_QUEUED_BIT			3
#define CBR_EXS_TLB_INVAL_BIT			4
#define CBR_EXS_EXCEPTION_BIT			5

#define CBR_EXS_ABORT_OCC			(1 << CBR_EXS_ABORT_OCC_BIT)
#define CBR_EXS_INT_OCC				(1 << CBR_EXS_INT_OCC_BIT)
#define CBR_EXS_PENDING				(1 << CBR_EXS_PENDING_BIT)
#define CBR_EXS_QUEUED				(1 << CBR_EXS_QUEUED_BIT)
#define CBR_TLB_INVAL				(1 << CBR_EXS_TLB_INVAL_BIT)
#define CBR_EXS_EXCEPTION			(1 << CBR_EXS_EXCEPTION_BIT)


#define EXCEPTION_RETRY_BITS (CBE_CAUSE_EXECUTION_HW_ERROR |		\
			      CBE_CAUSE_TLBHW_ERROR |			\
			      CBE_CAUSE_RA_REQUEST_TIMEOUT |		\
			      CBE_CAUSE_RA_RESPONSE_NON_FATAL |		\
			      CBE_CAUSE_HA_RESPONSE_NON_FATAL |		\
			      CBE_CAUSE_RA_RESPONSE_DATA_ERROR |	\
			      CBE_CAUSE_HA_RESPONSE_DATA_ERROR		\
			      )


union gru_mesqhead {
	unsigned long	val;
	struct {
		unsigned int	head;
		unsigned int	limit;
	};
};



static inline unsigned int
__opword(unsigned char opcode, unsigned char exopc, unsigned char xtype,
       unsigned char iaa0, unsigned char iaa1,
       unsigned char ima)
{
    return (1 << GRU_CB_ICMD_SHFT) |
	   (iaa0 << GRU_CB_IAA0_SHFT) |
	   (iaa1 << GRU_CB_IAA1_SHFT) |
	   (ima << GRU_CB_IMA_SHFT) |
	   (xtype << GRU_CB_XTYPE_SHFT) |
	   (opcode << GRU_CB_OPC_SHFT) |
	   (exopc << GRU_CB_EXOPC_SHFT);
}


static inline void gru_flush_cache(void *p)
{
	__flush_cache(p);
}


static inline void gru_start_instruction(struct gru_instruction *ins, int op32)
{
	gru_ordered_store_int(ins, op32);
	gru_flush_cache(ins);
}



#define CB_IMA(h)		((h) | IMA_UNMAPPED)


#define GRU_DINDEX(i)		((i) * GRU_CACHE_LINE_BYTES)


static inline void gru_vload(void *cb, unsigned long mem_addr,
		unsigned int tri0, unsigned char xtype, unsigned long nelem,
		unsigned long stride, unsigned long hints)
{
	struct gru_instruction *ins = (struct gru_instruction *)cb;

	ins->baddr0 = (long)mem_addr;
	ins->nelem = nelem;
	ins->tri0 = tri0;
	ins->op1_stride = stride;
	gru_start_instruction(ins, __opword(OP_VLOAD, 0, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_vstore(void *cb, unsigned long mem_addr,
		unsigned int tri0, unsigned char xtype, unsigned long nelem,
		unsigned long stride, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)mem_addr;
	ins->nelem = nelem;
	ins->tri0 = tri0;
	ins->op1_stride = stride;
	gru_start_instruction(ins, __opword(OP_VSTORE, 0, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_ivload(void *cb, unsigned long mem_addr,
		unsigned int tri0, unsigned int tri1, unsigned char xtype,
		unsigned long nelem, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)mem_addr;
	ins->nelem = nelem;
	ins->tri0 = tri0;
	ins->tri1_bufsize = tri1;
	gru_start_instruction(ins, __opword(OP_IVLOAD, 0, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_ivstore(void *cb, unsigned long mem_addr,
		unsigned int tri0, unsigned int tri1,
		unsigned char xtype, unsigned long nelem, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)mem_addr;
	ins->nelem = nelem;
	ins->tri0 = tri0;
	ins->tri1_bufsize = tri1;
	gru_start_instruction(ins, __opword(OP_IVSTORE, 0, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_vset(void *cb, unsigned long mem_addr,
		unsigned long value, unsigned char xtype, unsigned long nelem,
		unsigned long stride, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)mem_addr;
	ins->op2_value_baddr1 = value;
	ins->nelem = nelem;
	ins->op1_stride = stride;
	gru_start_instruction(ins, __opword(OP_VSET, 0, xtype, IAA_RAM, 0,
					 CB_IMA(hints)));
}

static inline void gru_ivset(void *cb, unsigned long mem_addr,
		unsigned int tri1, unsigned long value, unsigned char xtype,
		unsigned long nelem, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)mem_addr;
	ins->op2_value_baddr1 = value;
	ins->nelem = nelem;
	ins->tri1_bufsize = tri1;
	gru_start_instruction(ins, __opword(OP_IVSET, 0, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_vflush(void *cb, unsigned long mem_addr,
		unsigned long nelem, unsigned char xtype, unsigned long stride,
		unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)mem_addr;
	ins->op1_stride = stride;
	ins->nelem = nelem;
	gru_start_instruction(ins, __opword(OP_VFLUSH, 0, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_nop(void *cb, int hints)
{
	struct gru_instruction *ins = (void *)cb;

	gru_start_instruction(ins, __opword(OP_NOP, 0, 0, 0, 0, CB_IMA(hints)));
}


static inline void gru_bcopy(void *cb, const unsigned long src,
		unsigned long dest,
		unsigned int tri0, unsigned int xtype, unsigned long nelem,
		unsigned int bufsize, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)src;
	ins->op2_value_baddr1 = (long)dest;
	ins->nelem = nelem;
	ins->tri0 = tri0;
	ins->tri1_bufsize = bufsize;
	gru_start_instruction(ins, __opword(OP_BCOPY, 0, xtype, IAA_RAM,
					IAA_RAM, CB_IMA(hints)));
}

static inline void gru_bstore(void *cb, const unsigned long src,
		unsigned long dest, unsigned int tri0, unsigned int xtype,
		unsigned long nelem, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)src;
	ins->op2_value_baddr1 = (long)dest;
	ins->nelem = nelem;
	ins->tri0 = tri0;
	gru_start_instruction(ins, __opword(OP_BSTORE, 0, xtype, 0, IAA_RAM,
					CB_IMA(hints)));
}

static inline void gru_gamir(void *cb, int exopc, unsigned long src,
		unsigned int xtype, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)src;
	gru_start_instruction(ins, __opword(OP_GAMIR, exopc, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_gamirr(void *cb, int exopc, unsigned long src,
		unsigned int xtype, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)src;
	gru_start_instruction(ins, __opword(OP_GAMIRR, exopc, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_gamer(void *cb, int exopc, unsigned long src,
		unsigned int xtype,
		unsigned long operand1, unsigned long operand2,
		unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)src;
	ins->op1_stride = operand1;
	ins->op2_value_baddr1 = operand2;
	gru_start_instruction(ins, __opword(OP_GAMER, exopc, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_gamerr(void *cb, int exopc, unsigned long src,
		unsigned int xtype, unsigned long operand1,
		unsigned long operand2, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)src;
	ins->op1_stride = operand1;
	ins->op2_value_baddr1 = operand2;
	gru_start_instruction(ins, __opword(OP_GAMERR, exopc, xtype, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline void gru_gamxr(void *cb, unsigned long src,
		unsigned int tri0, unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)src;
	ins->nelem = 4;
	gru_start_instruction(ins, __opword(OP_GAMXR, EOP_XR_CSWAP, XTYPE_DW,
				 IAA_RAM, 0, CB_IMA(hints)));
}

static inline void gru_mesq(void *cb, unsigned long queue,
		unsigned long tri0, unsigned long nelem,
		unsigned long hints)
{
	struct gru_instruction *ins = (void *)cb;

	ins->baddr0 = (long)queue;
	ins->nelem = nelem;
	ins->tri0 = tri0;
	gru_start_instruction(ins, __opword(OP_MESQ, 0, XTYPE_CL, IAA_RAM, 0,
					CB_IMA(hints)));
}

static inline unsigned long gru_get_amo_value(void *cb)
{
	struct gru_instruction *ins = (void *)cb;

	return ins->avalue;
}

static inline int gru_get_amo_value_head(void *cb)
{
	struct gru_instruction *ins = (void *)cb;

	return ins->avalue & 0xffffffff;
}

static inline int gru_get_amo_value_limit(void *cb)
{
	struct gru_instruction *ins = (void *)cb;

	return ins->avalue >> 32;
}

static inline union gru_mesqhead  gru_mesq_head(int head, int limit)
{
	union gru_mesqhead mqh;

	mqh.head = head;
	mqh.limit = limit;
	return mqh;
}


extern int gru_get_cb_exception_detail(void *cb,
		       struct control_block_extended_exc_detail *excdet);

#define GRU_EXC_STR_SIZE		256



struct gru_control_block_status {
	unsigned int	icmd		:1;
	unsigned int	ima		:3;
	unsigned int	reserved0	:4;
	unsigned int	unused1		:24;
	unsigned int	unused2		:24;
	unsigned int	istatus		:2;
	unsigned int	isubstatus	:4;
	unsigned int	unused3		:2;
};


static inline int gru_get_cb_status(void *cb)
{
	struct gru_control_block_status *cbs = (void *)cb;

	return cbs->istatus;
}


static inline int gru_get_cb_message_queue_substatus(void *cb)
{
	struct gru_control_block_status *cbs = (void *)cb;

	return cbs->isubstatus & CBSS_MSG_QUEUE_MASK;
}


static inline int gru_get_cb_substatus(void *cb)
{
	struct gru_control_block_status *cbs = (void *)cb;

	return cbs->isubstatus;
}


static inline int gru_check_status(void *cb)
{
	struct gru_control_block_status *cbs = (void *)cb;
	int ret;

	ret = cbs->istatus;
	if (ret != CBS_ACTIVE)
		ret = gru_check_status_proc(cb);
	return ret;
}


static inline int gru_wait(void *cb)
{
	return gru_wait_proc(cb);
}


static inline void gru_wait_abort(void *cb)
{
	gru_wait_abort_proc(cb);
}



static inline void *gru_get_cb_pointer(void *gseg,
						      int index)
{
	return gseg + GRU_CB_BASE + index * GRU_HANDLE_STRIDE;
}


static inline void *gru_get_data_pointer(void *gseg, int index)
{
	return gseg + GRU_DS_BASE + index * GRU_CACHE_LINE_BYTES;
}


static inline int gru_get_tri(void *vaddr)
{
	return ((unsigned long)vaddr & (GRU_GSEG_PAGESIZE - 1)) - GRU_DS_BASE;
}
#endif		
