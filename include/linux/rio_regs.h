

#ifndef LINUX_RIO_REGS_H
#define LINUX_RIO_REGS_H


#define RIO_DEV_ID_CAR		0x00	
#define RIO_DEV_INFO_CAR	0x04	
#define RIO_ASM_ID_CAR		0x08	
#define  RIO_ASM_ID_MASK		0xffff0000	
#define  RIO_ASM_VEN_ID_MASK		0x0000ffff	

#define RIO_ASM_INFO_CAR	0x0c	
#define  RIO_ASM_REV_MASK		0xffff0000	
#define  RIO_EXT_FTR_PTR_MASK		0x0000ffff	

#define RIO_PEF_CAR		0x10	
#define  RIO_PEF_BRIDGE			0x80000000	
#define  RIO_PEF_MEMORY			0x40000000	
#define  RIO_PEF_PROCESSOR		0x20000000	
#define  RIO_PEF_SWITCH			0x10000000	
#define  RIO_PEF_INB_MBOX		0x00f00000	
#define  RIO_PEF_INB_MBOX0		0x00800000	
#define  RIO_PEF_INB_MBOX1		0x00400000	
#define  RIO_PEF_INB_MBOX2		0x00200000	
#define  RIO_PEF_INB_MBOX3		0x00100000	
#define  RIO_PEF_INB_DOORBELL		0x00080000	
#define  RIO_PEF_CTLS			0x00000010	
#define  RIO_PEF_EXT_FEATURES		0x00000008	
#define  RIO_PEF_ADDR_66		0x00000004	
#define  RIO_PEF_ADDR_50		0x00000002	
#define  RIO_PEF_ADDR_34		0x00000001	

#define RIO_SWP_INFO_CAR	0x14	
#define  RIO_SWP_INFO_PORT_TOTAL_MASK	0x0000ff00	
#define  RIO_SWP_INFO_PORT_NUM_MASK	0x000000ff	
#define  RIO_GET_TOTAL_PORTS(x)		((x & RIO_SWP_INFO_PORT_TOTAL_MASK) >> 8)

#define RIO_SRC_OPS_CAR		0x18	
#define  RIO_SRC_OPS_READ		0x00008000	
#define  RIO_SRC_OPS_WRITE		0x00004000	
#define  RIO_SRC_OPS_STREAM_WRITE	0x00002000	
#define  RIO_SRC_OPS_WRITE_RESPONSE	0x00001000	
#define  RIO_SRC_OPS_DATA_MSG		0x00000800	
#define  RIO_SRC_OPS_DOORBELL		0x00000400	
#define  RIO_SRC_OPS_ATOMIC_TST_SWP	0x00000100	
#define  RIO_SRC_OPS_ATOMIC_INC		0x00000080	
#define  RIO_SRC_OPS_ATOMIC_DEC		0x00000040	
#define  RIO_SRC_OPS_ATOMIC_SET		0x00000020	
#define  RIO_SRC_OPS_ATOMIC_CLR		0x00000010	
#define  RIO_SRC_OPS_PORT_WRITE		0x00000004	

#define RIO_DST_OPS_CAR		0x1c	
#define  RIO_DST_OPS_READ		0x00008000	
#define  RIO_DST_OPS_WRITE		0x00004000	
#define  RIO_DST_OPS_STREAM_WRITE	0x00002000	
#define  RIO_DST_OPS_WRITE_RESPONSE	0x00001000	
#define  RIO_DST_OPS_DATA_MSG		0x00000800	
#define  RIO_DST_OPS_DOORBELL		0x00000400	
#define  RIO_DST_OPS_ATOMIC_TST_SWP	0x00000100	
#define  RIO_DST_OPS_ATOMIC_INC		0x00000080	
#define  RIO_DST_OPS_ATOMIC_DEC		0x00000040	
#define  RIO_DST_OPS_ATOMIC_SET		0x00000020	
#define  RIO_DST_OPS_ATOMIC_CLR		0x00000010	
#define  RIO_DST_OPS_PORT_WRITE		0x00000004	

#define  RIO_OPS_READ			0x00008000	
#define  RIO_OPS_WRITE			0x00004000	
#define  RIO_OPS_STREAM_WRITE		0x00002000	
#define  RIO_OPS_WRITE_RESPONSE		0x00001000	
#define  RIO_OPS_DATA_MSG		0x00000800	
#define  RIO_OPS_DOORBELL		0x00000400	
#define  RIO_OPS_ATOMIC_TST_SWP		0x00000100	
#define  RIO_OPS_ATOMIC_INC		0x00000080	
#define  RIO_OPS_ATOMIC_DEC		0x00000040	
#define  RIO_OPS_ATOMIC_SET		0x00000020	
#define  RIO_OPS_ATOMIC_CLR		0x00000010	
#define  RIO_OPS_PORT_WRITE		0x00000004	

					

#define RIO_MBOX_CSR		0x40	
#define  RIO_MBOX0_AVAIL		0x80000000	
#define  RIO_MBOX0_FULL			0x40000000	
#define  RIO_MBOX0_EMPTY		0x20000000	
#define  RIO_MBOX0_BUSY			0x10000000	
#define  RIO_MBOX0_FAIL			0x08000000	
#define  RIO_MBOX0_ERROR		0x04000000	
#define  RIO_MBOX1_AVAIL		0x00800000	
#define  RIO_MBOX1_FULL			0x00200000	
#define  RIO_MBOX1_EMPTY		0x00200000	
#define  RIO_MBOX1_BUSY			0x00100000	
#define  RIO_MBOX1_FAIL			0x00080000	
#define  RIO_MBOX1_ERROR		0x00040000	
#define  RIO_MBOX2_AVAIL		0x00008000	
#define  RIO_MBOX2_FULL			0x00004000	
#define  RIO_MBOX2_EMPTY		0x00002000	
#define  RIO_MBOX2_BUSY			0x00001000	
#define  RIO_MBOX2_FAIL			0x00000800	
#define  RIO_MBOX2_ERROR		0x00000400	
#define  RIO_MBOX3_AVAIL		0x00000080	
#define  RIO_MBOX3_FULL			0x00000040	
#define  RIO_MBOX3_EMPTY		0x00000020	
#define  RIO_MBOX3_BUSY			0x00000010	
#define  RIO_MBOX3_FAIL			0x00000008	
#define  RIO_MBOX3_ERROR		0x00000004	

#define RIO_WRITE_PORT_CSR	0x44	
#define RIO_DOORBELL_CSR	0x44	
#define  RIO_DOORBELL_AVAIL		0x80000000	
#define  RIO_DOORBELL_FULL		0x40000000	
#define  RIO_DOORBELL_EMPTY		0x20000000	
#define  RIO_DOORBELL_BUSY		0x10000000	
#define  RIO_DOORBELL_FAILED		0x08000000	
#define  RIO_DOORBELL_ERROR		0x04000000	
#define  RIO_WRITE_PORT_AVAILABLE	0x00000080	
#define  RIO_WRITE_PORT_FULL		0x00000040	
#define  RIO_WRITE_PORT_EMPTY		0x00000020	
#define  RIO_WRITE_PORT_BUSY		0x00000010	
#define  RIO_WRITE_PORT_FAILED		0x00000008	
#define  RIO_WRITE_PORT_ERROR		0x00000004	

					

#define RIO_PELL_CTRL_CSR	0x4c	
#define   RIO_PELL_ADDR_66		0x00000004	
#define   RIO_PELL_ADDR_50		0x00000002	
#define   RIO_PELL_ADDR_34		0x00000001	

					

#define RIO_LCSH_BA		0x58	
#define RIO_LCSL_BA		0x5c	

#define RIO_DID_CSR		0x60	

					

#define RIO_HOST_DID_LOCK_CSR	0x68	
#define RIO_COMPONENT_TAG_CSR	0x6c	

					
					
					




#define RIO_EFB_PTR_MASK	0xffff0000
#define RIO_EFB_ID_MASK		0x0000ffff
#define RIO_GET_BLOCK_PTR(x)	((x & RIO_EFB_PTR_MASK) >> 16)
#define RIO_GET_BLOCK_ID(x)	(x & RIO_EFB_ID_MASK)


#define RIO_EFB_PAR_EP_ID	0x0001	
#define RIO_EFB_PAR_EP_REC_ID	0x0002	
#define RIO_EFB_PAR_EP_FREE_ID	0x0003	
#define RIO_EFB_SER_EP_ID	0x0004	
#define RIO_EFB_SER_EP_REC_ID	0x0005	
#define RIO_EFB_SER_EP_FREE_ID	0x0006	


#define RIO_PORT_MNT_HEADER		0x0000
#define RIO_PORT_REQ_CTL_CSR		0x0020
#define RIO_PORT_RSP_CTL_CSR		0x0024	
#define RIO_PORT_GEN_CTL_CSR		0x003c
#define  RIO_PORT_GEN_HOST		0x80000000
#define  RIO_PORT_GEN_MASTER		0x40000000
#define  RIO_PORT_GEN_DISCOVERED	0x20000000
#define RIO_PORT_N_MNT_REQ_CSR(x)	(0x0040 + x*0x20)	
#define RIO_PORT_N_MNT_RSP_CSR(x)	(0x0044 + x*0x20)	
#define RIO_PORT_N_ACK_STS_CSR(x)	(0x0048 + x*0x20)	
#define RIO_PORT_N_ERR_STS_CSR(x)	(0x58 + x*0x20)
#define PORT_N_ERR_STS_PORT_OK	0x00000002
#define RIO_PORT_N_CTL_CSR(x)		(0x5c + x*0x20)

#endif				
