#ifndef __NET_SCHED_RED_H
#define __NET_SCHED_RED_H

#include <linux/types.h>
#include <net/pkt_sched.h>
#include <net/inet_ecn.h>
#include <net/dsfield.h>



#define RED_STAB_SIZE	256
#define RED_STAB_MASK	(RED_STAB_SIZE - 1)

struct red_stats
{
	u32		prob_drop;	
	u32		prob_mark;	
	u32		forced_drop;	
	u32		forced_mark;	
	u32		pdrop;          
	u32		other;          
	u32		backlog;
};

struct red_parms
{
	
	u32		qth_min;	
	u32		qth_max;	
	u32		Scell_max;
	u32		Rmask;		
	u8		Scell_log;
	u8		Wlog;		
	u8		Plog;		
	u8		Stab[RED_STAB_SIZE];

	
	int		qcount;		
	u32		qR;		

	unsigned long	qavg;		
	psched_time_t	qidlestart;	
};

static inline u32 red_rmask(u8 Plog)
{
	return Plog < 32 ? ((1 << Plog) - 1) : ~0UL;
}

static inline void red_set_parms(struct red_parms *p,
				 u32 qth_min, u32 qth_max, u8 Wlog, u8 Plog,
				 u8 Scell_log, u8 *stab)
{
	
	p->qavg		= 0;

	p->qcount	= -1;
	p->qth_min	= qth_min << Wlog;
	p->qth_max	= qth_max << Wlog;
	p->Wlog		= Wlog;
	p->Plog		= Plog;
	p->Rmask	= red_rmask(Plog);
	p->Scell_log	= Scell_log;
	p->Scell_max	= (255 << Scell_log);

	memcpy(p->Stab, stab, sizeof(p->Stab));
}

static inline int red_is_idling(struct red_parms *p)
{
	return p->qidlestart != PSCHED_PASTPERFECT;
}

static inline void red_start_of_idle_period(struct red_parms *p)
{
	p->qidlestart = psched_get_time();
}

static inline void red_end_of_idle_period(struct red_parms *p)
{
	p->qidlestart = PSCHED_PASTPERFECT;
}

static inline void red_restart(struct red_parms *p)
{
	red_end_of_idle_period(p);
	p->qavg = 0;
	p->qcount = -1;
}

static inline unsigned long red_calc_qavg_from_idle_time(struct red_parms *p)
{
	psched_time_t now;
	long us_idle;
	int  shift;

	now = psched_get_time();
	us_idle = psched_tdiff_bounded(now, p->qidlestart, p->Scell_max);

	

	shift = p->Stab[(us_idle >> p->Scell_log) & RED_STAB_MASK];

	if (shift)
		return p->qavg >> shift;
	else {
		
		us_idle = (p->qavg * (u64)us_idle) >> p->Scell_log;

		if (us_idle < (p->qavg >> 1))
			return p->qavg - us_idle;
		else
			return p->qavg >> 1;
	}
}

static inline unsigned long red_calc_qavg_no_idle_time(struct red_parms *p,
						       unsigned int backlog)
{
	
	return p->qavg + (backlog - (p->qavg >> p->Wlog));
}

static inline unsigned long red_calc_qavg(struct red_parms *p,
					  unsigned int backlog)
{
	if (!red_is_idling(p))
		return red_calc_qavg_no_idle_time(p, backlog);
	else
		return red_calc_qavg_from_idle_time(p);
}

static inline u32 red_random(struct red_parms *p)
{
	return net_random() & p->Rmask;
}

static inline int red_mark_probability(struct red_parms *p, unsigned long qavg)
{
	
	return !(((qavg - p->qth_min) >> p->Wlog) * p->qcount < p->qR);
}

enum {
	RED_BELOW_MIN_THRESH,
	RED_BETWEEN_TRESH,
	RED_ABOVE_MAX_TRESH,
};

static inline int red_cmp_thresh(struct red_parms *p, unsigned long qavg)
{
	if (qavg < p->qth_min)
		return RED_BELOW_MIN_THRESH;
	else if (qavg >= p->qth_max)
		return RED_ABOVE_MAX_TRESH;
	else
		return RED_BETWEEN_TRESH;
}

enum {
	RED_DONT_MARK,
	RED_PROB_MARK,
	RED_HARD_MARK,
};

static inline int red_action(struct red_parms *p, unsigned long qavg)
{
	switch (red_cmp_thresh(p, qavg)) {
		case RED_BELOW_MIN_THRESH:
			p->qcount = -1;
			return RED_DONT_MARK;

		case RED_BETWEEN_TRESH:
			if (++p->qcount) {
				if (red_mark_probability(p, qavg)) {
					p->qcount = 0;
					p->qR = red_random(p);
					return RED_PROB_MARK;
				}
			} else
				p->qR = red_random(p);

			return RED_DONT_MARK;

		case RED_ABOVE_MAX_TRESH:
			p->qcount = -1;
			return RED_HARD_MARK;
	}

	BUG();
	return RED_DONT_MARK;
}

#endif
