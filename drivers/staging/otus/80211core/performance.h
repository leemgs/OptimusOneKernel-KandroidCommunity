
#ifndef _PERFORMANCE_H
#define _PERFORMANCE_H

#ifdef ZM_ENABLE_PERFORMANCE_EVALUATION

struct zsSummary
{
    u32_t tx_msdu_count;
    u32_t tx_mpdu_count;
    u32_t rx_msdu_count;
    u32_t rx_mpdu_count;
    u32_t tick_base;
    u16_t rx_seq_base;
    u16_t rx_broken_seq;
    u16_t rx_broken_sum;
    u16_t rx_broken_seq_dis;
    u16_t rx_duplicate_seq;
    u16_t rx_duplicate_error;
    u16_t rx_old_seq;
    u16_t rx_lost_sum;
    u16_t tx_idle_count;
    u16_t rx_idle_count;
    u16_t reset_count;
    u16_t reset_sum;
    u16_t rx_free;
    u16_t rx_amsdu_len;
    u16_t rx_flush;
    u16_t rx_clear;
    u32_t rx_reorder;
};

struct zsVariation
{
    u32_t tx_msdu_tick[100];
    u32_t tx_mpdu_tick[100];
    u32_t rx_msdu_tick[100];
    u32_t rx_mpdu_tick[100];

    u32_t tx_msdu_mean;
    u32_t tx_mpdu_mean;
    u32_t rx_msdu_mean;
    u32_t rx_mpdu_mean;

    u32_t tx_msdu_sum;
    u32_t tx_mpdu_sum;
    u32_t rx_msdu_sum;
    u32_t rx_mpdu_sum;

    u32_t tx_msdu_var;
    u32_t tx_mpdu_var;
    u32_t rx_msdu_var;
    u32_t rx_mpdu_var;
};

struct zsThroughput
{
    u32_t tx[50];
    u32_t rx[50];
    u16_t head;
    u16_t tail;
    u16_t size;
    LARGE_INTEGER sys_time;
    LARGE_INTEGER freq;
};

void zfiPerformanceInit(zdev_t* dev);
void zfiPerformanceRefresh(zdev_t* dev);

void zfiTxPerformanceMSDU(zdev_t* dev, u32_t tick);
void zfiRxPerformanceMSDU(zdev_t* dev, u32_t tick);
void zfiTxPerformanceMPDU(zdev_t* dev, u32_t tick);
void zfiRxPerformanceMPDU(zdev_t* dev, zbuf_t* buf);
void zfiRxPerformanceSeq(zdev_t* dev, zbuf_t* buf);
void zfiRxPerformanceReg(zdev_t* dev, u32_t reg, u32_t rsp);
void zfiRxPerformanceDup(zdev_t* dev, zbuf_t* buf1, zbuf_t* buf2);
void zfiRxPerformanceFree(zdev_t* dev, zbuf_t* buf);
void zfiRxPerformanceAMSDU(zdev_t* dev, zbuf_t* buf, u16_t len);
void zfiRxPerformanceFlush(zdev_t* dev);
void zfiRxPerformanceClear(zdev_t* dev);
void zfiRxPerformanceReorder(zdev_t* dev);
#endif 
#endif 
