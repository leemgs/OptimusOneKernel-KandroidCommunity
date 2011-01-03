


#ifndef __ETHERNET_DEFINES_H__
#define __ETHERNET_DEFINES_H__

#include "cvmx-config.h"


#define OCTEON_ETHERNET_VERSION "1.9"

#ifndef CONFIG_CAVIUM_RESERVE32
#define CONFIG_CAVIUM_RESERVE32 0
#endif

#if CONFIG_CAVIUM_RESERVE32
#define USE_32BIT_SHARED            1
#define USE_SKBUFFS_IN_HW           0
#define REUSE_SKBUFFS_WITHOUT_FREE  0
#else
#define USE_32BIT_SHARED            0
#define USE_SKBUFFS_IN_HW           1
#ifdef CONFIG_NETFILTER
#define REUSE_SKBUFFS_WITHOUT_FREE  0
#else
#define REUSE_SKBUFFS_WITHOUT_FREE  1
#endif
#endif


#define INTERRUPT_LIMIT             10000



#define USE_HW_TCPUDP_CHECKSUM      1

#define USE_MULTICORE_RECEIVE       1


#define USE_RED                     1
#define USE_ASYNC_IOBDMA            (CONFIG_CAVIUM_OCTEON_CVMSEG_SIZE > 0)


#define USE_10MBPS_PREAMBLE_WORKAROUND 1

#define DONT_WRITEBACK(x)           (x)




#define MAX_RX_PACKETS 120

#define MAX_SKB_TO_FREE 10
#define MAX_OUT_QUEUE_DEPTH 1000

#ifndef CONFIG_SMP
#undef USE_MULTICORE_RECEIVE
#define USE_MULTICORE_RECEIVE 0
#endif

#define IP_PROTOCOL_TCP             6
#define IP_PROTOCOL_UDP             0x11

#define FAU_NUM_PACKET_BUFFERS_TO_FREE (CVMX_FAU_REG_END - sizeof(uint32_t))
#define TOTAL_NUMBER_OF_PORTS       (CVMX_PIP_NUM_INPUT_PORTS+1)


#endif 
