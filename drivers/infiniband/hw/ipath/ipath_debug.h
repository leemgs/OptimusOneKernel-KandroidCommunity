

#ifndef _IPATH_DEBUG_H
#define _IPATH_DEBUG_H

#ifndef _IPATH_DEBUGGING	
#define _IPATH_DEBUGGING 1
#endif

#if _IPATH_DEBUGGING



#define __IPATH_INFO        0x1	
#define __IPATH_DBG         0x2	
#define __IPATH_TRSAMPLE    0x8	

#define __IPATH_VERBDBG     0x40	
#define __IPATH_PKTDBG      0x80	

#define __IPATH_PROCDBG     0x100

#define __IPATH_MMDBG       0x200
#define __IPATH_ERRPKTDBG   0x400
#define __IPATH_USER_SEND   0x1000	
#define __IPATH_KERNEL_SEND 0x2000	
#define __IPATH_EPKTDBG     0x4000	
#define __IPATH_IPATHDBG    0x10000	
#define __IPATH_IPATHWARN   0x20000	
#define __IPATH_IPATHERR    0x40000	
#define __IPATH_IPATHPD     0x80000	
#define __IPATH_IPATHTABLE  0x100000	
#define __IPATH_LINKVERBDBG 0x200000	

#else				



#define __IPATH_INFO      0x0	
#define __IPATH_DBG       0x0	
#define __IPATH_TRSAMPLE  0x0	
#define __IPATH_VERBDBG   0x0	
#define __IPATH_PKTDBG    0x0	
#define __IPATH_PROCDBG   0x0	

#define __IPATH_MMDBG     0x0
#define __IPATH_EPKTDBG   0x0	
#define __IPATH_IPATHDBG  0x0	
#define __IPATH_IPATHWARN 0x0	
#define __IPATH_IPATHERR  0x0	
#define __IPATH_IPATHPD   0x0	
#define __IPATH_IPATHTABLE 0x0	
#define __IPATH_LINKVERBDBG 0x0	

#endif				

#define __IPATH_VERBOSEDBG __IPATH_VERBDBG

#endif				
