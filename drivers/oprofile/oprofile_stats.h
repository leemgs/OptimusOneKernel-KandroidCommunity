

#ifndef OPROFILE_STATS_H
#define OPROFILE_STATS_H

#include <asm/atomic.h>

struct oprofile_stat_struct {
	atomic_t sample_lost_no_mm;
	atomic_t sample_lost_no_mapping;
	atomic_t bt_lost_no_mapping;
	atomic_t event_lost_overflow;
	atomic_t multiplex_counter;
};

extern struct oprofile_stat_struct oprofile_stats;


void oprofile_reset_stats(void);

struct super_block;
struct dentry;


void oprofile_create_stats_files(struct super_block *sb, struct dentry *root);

#endif 
