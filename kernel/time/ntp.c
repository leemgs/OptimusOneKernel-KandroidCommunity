
#include <linux/capability.h>
#include <linux/clocksource.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/math64.h>
#include <linux/timex.h>
#include <linux/time.h>
#include <linux/mm.h>




unsigned long			tick_usec = TICK_USEC;


unsigned long			tick_nsec;

u64				tick_length;
static u64			tick_length_base;

static struct hrtimer		leap_timer;

#define MAX_TICKADJ		500LL		
#define MAX_TICKADJ_SCALED \
	(((MAX_TICKADJ * NSEC_PER_USEC) << NTP_SCALE_SHIFT) / NTP_INTERVAL_FREQ)




static int			time_state = TIME_OK;


int				time_status = STA_UNSYNC;


static long			time_tai;


static s64			time_offset;


static long			time_constant = 2;


long				time_maxerror = NTP_PHASE_LIMIT;


long				time_esterror = NTP_PHASE_LIMIT;


static s64			time_freq;


static long			time_reftime;

long				time_adjust;


static s64			ntp_tick_adj;




static void ntp_update_frequency(void)
{
	u64 second_length;
	u64 new_base;

	second_length		 = (u64)(tick_usec * NSEC_PER_USEC * USER_HZ)
						<< NTP_SCALE_SHIFT;

	second_length		+= ntp_tick_adj;
	second_length		+= time_freq;

	tick_nsec		 = div_u64(second_length, HZ) >> NTP_SCALE_SHIFT;
	new_base		 = div_u64(second_length, NTP_INTERVAL_FREQ);

	
	tick_length		+= new_base - tick_length_base;
	tick_length_base	 = new_base;
}

static inline s64 ntp_update_offset_fll(s64 offset64, long secs)
{
	time_status &= ~STA_MODE;

	if (secs < MINSEC)
		return 0;

	if (!(time_status & STA_FLL) && (secs <= MAXSEC))
		return 0;

	time_status |= STA_MODE;

	return div_s64(offset64 << (NTP_SCALE_SHIFT - SHIFT_FLL), secs);
}

static void ntp_update_offset(long offset)
{
	s64 freq_adj;
	s64 offset64;
	long secs;

	if (!(time_status & STA_PLL))
		return;

	if (!(time_status & STA_NANO))
		offset *= NSEC_PER_USEC;

	
	offset = min(offset, MAXPHASE);
	offset = max(offset, -MAXPHASE);

	
	secs = xtime.tv_sec - time_reftime;
	if (unlikely(time_status & STA_FREQHOLD))
		secs = 0;

	time_reftime = xtime.tv_sec;

	offset64    = offset;
	freq_adj    = (offset64 * secs) <<
			(NTP_SCALE_SHIFT - 2 * (SHIFT_PLL + 2 + time_constant));

	freq_adj    += ntp_update_offset_fll(offset64, secs);

	freq_adj    = min(freq_adj + time_freq, MAXFREQ_SCALED);

	time_freq   = max(freq_adj, -MAXFREQ_SCALED);

	time_offset = div_s64(offset64 << NTP_SCALE_SHIFT, NTP_INTERVAL_FREQ);
}


void ntp_clear(void)
{
	time_adjust	= 0;		
	time_status	|= STA_UNSYNC;
	time_maxerror	= NTP_PHASE_LIMIT;
	time_esterror	= NTP_PHASE_LIMIT;

	ntp_update_frequency();

	tick_length	= tick_length_base;
	time_offset	= 0;
}


static enum hrtimer_restart ntp_leap_second(struct hrtimer *timer)
{
	enum hrtimer_restart res = HRTIMER_NORESTART;

	write_seqlock(&xtime_lock);

	switch (time_state) {
	case TIME_OK:
		break;
	case TIME_INS:
		timekeeping_leap_insert(-1);
		time_state = TIME_OOP;
		printk(KERN_NOTICE
			"Clock: inserting leap second 23:59:60 UTC\n");
		hrtimer_add_expires_ns(&leap_timer, NSEC_PER_SEC);
		res = HRTIMER_RESTART;
		break;
	case TIME_DEL:
		timekeeping_leap_insert(1);
		time_tai--;
		time_state = TIME_WAIT;
		printk(KERN_NOTICE
			"Clock: deleting leap second 23:59:59 UTC\n");
		break;
	case TIME_OOP:
		time_tai++;
		time_state = TIME_WAIT;
		
	case TIME_WAIT:
		if (!(time_status & (STA_INS | STA_DEL)))
			time_state = TIME_OK;
		break;
	}

	write_sequnlock(&xtime_lock);

	return res;
}


void second_overflow(void)
{
	s64 delta;

	
	time_maxerror += MAXFREQ / NSEC_PER_USEC;
	if (time_maxerror > NTP_PHASE_LIMIT) {
		time_maxerror = NTP_PHASE_LIMIT;
		time_status |= STA_UNSYNC;
	}

	
	tick_length	 = tick_length_base;

	delta		 = shift_right(time_offset, SHIFT_PLL + time_constant);
	time_offset	-= delta;
	tick_length	+= delta;

	if (!time_adjust)
		return;

	if (time_adjust > MAX_TICKADJ) {
		time_adjust -= MAX_TICKADJ;
		tick_length += MAX_TICKADJ_SCALED;
		return;
	}

	if (time_adjust < -MAX_TICKADJ) {
		time_adjust += MAX_TICKADJ;
		tick_length -= MAX_TICKADJ_SCALED;
		return;
	}

	tick_length += (s64)(time_adjust * NSEC_PER_USEC / NTP_INTERVAL_FREQ)
							 << NTP_SCALE_SHIFT;
	time_adjust = 0;
}

#ifdef CONFIG_GENERIC_CMOS_UPDATE


int no_sync_cmos_clock  __read_mostly;

static void sync_cmos_clock(struct work_struct *work);

static DECLARE_DELAYED_WORK(sync_cmos_work, sync_cmos_clock);

static void sync_cmos_clock(struct work_struct *work)
{
	struct timespec now, next;
	int fail = 1;

	
	if (!ntp_synced()) {
		
		return;
	}

	getnstimeofday(&now);
	if (abs(now.tv_nsec - (NSEC_PER_SEC / 2)) <= tick_nsec / 2)
		fail = update_persistent_clock(now);

	next.tv_nsec = (NSEC_PER_SEC / 2) - now.tv_nsec - (TICK_NSEC / 2);
	if (next.tv_nsec <= 0)
		next.tv_nsec += NSEC_PER_SEC;

	if (!fail)
		next.tv_sec = 659;
	else
		next.tv_sec = 0;

	if (next.tv_nsec >= NSEC_PER_SEC) {
		next.tv_sec++;
		next.tv_nsec -= NSEC_PER_SEC;
	}
	schedule_delayed_work(&sync_cmos_work, timespec_to_jiffies(&next));
}

static void notify_cmos_timer(void)
{
	if (!no_sync_cmos_clock)
		schedule_delayed_work(&sync_cmos_work, 0);
}

#else
static inline void notify_cmos_timer(void) { }
#endif


static inline void ntp_start_leap_timer(struct timespec *ts)
{
	long now = ts->tv_sec;

	if (time_status & STA_INS) {
		time_state = TIME_INS;
		now += 86400 - now % 86400;
		hrtimer_start(&leap_timer, ktime_set(now, 0), HRTIMER_MODE_ABS);

		return;
	}

	if (time_status & STA_DEL) {
		time_state = TIME_DEL;
		now += 86400 - (now + 1) % 86400;
		hrtimer_start(&leap_timer, ktime_set(now, 0), HRTIMER_MODE_ABS);
	}
}


static inline void process_adj_status(struct timex *txc, struct timespec *ts)
{
	if ((time_status & STA_PLL) && !(txc->status & STA_PLL)) {
		time_state = TIME_OK;
		time_status = STA_UNSYNC;
	}

	
	if (!(time_status & STA_PLL) && (txc->status & STA_PLL))
		time_reftime = xtime.tv_sec;

	
	time_status &= STA_RONLY;
	time_status |= txc->status & ~STA_RONLY;

	switch (time_state) {
	case TIME_OK:
		ntp_start_leap_timer(ts);
		break;
	case TIME_INS:
	case TIME_DEL:
		time_state = TIME_OK;
		ntp_start_leap_timer(ts);
	case TIME_WAIT:
		if (!(time_status & (STA_INS | STA_DEL)))
			time_state = TIME_OK;
		break;
	case TIME_OOP:
		hrtimer_restart(&leap_timer);
		break;
	}
}

static inline void process_adjtimex_modes(struct timex *txc, struct timespec *ts)
{
	if (txc->modes & ADJ_STATUS)
		process_adj_status(txc, ts);

	if (txc->modes & ADJ_NANO)
		time_status |= STA_NANO;

	if (txc->modes & ADJ_MICRO)
		time_status &= ~STA_NANO;

	if (txc->modes & ADJ_FREQUENCY) {
		time_freq = txc->freq * PPM_SCALE;
		time_freq = min(time_freq, MAXFREQ_SCALED);
		time_freq = max(time_freq, -MAXFREQ_SCALED);
	}

	if (txc->modes & ADJ_MAXERROR)
		time_maxerror = txc->maxerror;

	if (txc->modes & ADJ_ESTERROR)
		time_esterror = txc->esterror;

	if (txc->modes & ADJ_TIMECONST) {
		time_constant = txc->constant;
		if (!(time_status & STA_NANO))
			time_constant += 4;
		time_constant = min(time_constant, (long)MAXTC);
		time_constant = max(time_constant, 0l);
	}

	if (txc->modes & ADJ_TAI && txc->constant > 0)
		time_tai = txc->constant;

	if (txc->modes & ADJ_OFFSET)
		ntp_update_offset(txc->offset);

	if (txc->modes & ADJ_TICK)
		tick_usec = txc->tick;

	if (txc->modes & (ADJ_TICK|ADJ_FREQUENCY|ADJ_OFFSET))
		ntp_update_frequency();
}


int do_adjtimex(struct timex *txc)
{
	struct timespec ts;
	int result;

	
	if (txc->modes & ADJ_ADJTIME) {
		
		if (!(txc->modes & ADJ_OFFSET_SINGLESHOT))
			return -EINVAL;
		if (!(txc->modes & ADJ_OFFSET_READONLY) &&
		    !capable(CAP_SYS_TIME))
			return -EPERM;
	} else {
		
		 if (txc->modes && !capable(CAP_SYS_TIME))
			return -EPERM;

		
		if (txc->modes & ADJ_TICK &&
		    (txc->tick <  900000/USER_HZ ||
		     txc->tick > 1100000/USER_HZ))
			return -EINVAL;

		if (txc->modes & ADJ_STATUS && time_state != TIME_OK)
			hrtimer_cancel(&leap_timer);
	}

	getnstimeofday(&ts);

	write_seqlock_irq(&xtime_lock);

	if (txc->modes & ADJ_ADJTIME) {
		long save_adjust = time_adjust;

		if (!(txc->modes & ADJ_OFFSET_READONLY)) {
			
			time_adjust = txc->offset;
			ntp_update_frequency();
		}
		txc->offset = save_adjust;
	} else {

		
		if (txc->modes)
			process_adjtimex_modes(txc, &ts);

		txc->offset = shift_right(time_offset * NTP_INTERVAL_FREQ,
				  NTP_SCALE_SHIFT);
		if (!(time_status & STA_NANO))
			txc->offset /= NSEC_PER_USEC;
	}

	result = time_state;	
	if (time_status & (STA_UNSYNC|STA_CLOCKERR))
		result = TIME_ERROR;

	txc->freq	   = shift_right((time_freq >> PPM_SCALE_INV_SHIFT) *
					 PPM_SCALE_INV, NTP_SCALE_SHIFT);
	txc->maxerror	   = time_maxerror;
	txc->esterror	   = time_esterror;
	txc->status	   = time_status;
	txc->constant	   = time_constant;
	txc->precision	   = 1;
	txc->tolerance	   = MAXFREQ_SCALED / PPM_SCALE;
	txc->tick	   = tick_usec;
	txc->tai	   = time_tai;

	
	txc->ppsfreq	   = 0;
	txc->jitter	   = 0;
	txc->shift	   = 0;
	txc->stabil	   = 0;
	txc->jitcnt	   = 0;
	txc->calcnt	   = 0;
	txc->errcnt	   = 0;
	txc->stbcnt	   = 0;

	write_sequnlock_irq(&xtime_lock);

	txc->time.tv_sec = ts.tv_sec;
	txc->time.tv_usec = ts.tv_nsec;
	if (!(time_status & STA_NANO))
		txc->time.tv_usec /= NSEC_PER_USEC;

	notify_cmos_timer();

	return result;
}

static int __init ntp_tick_adj_setup(char *str)
{
	ntp_tick_adj = simple_strtol(str, NULL, 0);
	ntp_tick_adj <<= NTP_SCALE_SHIFT;

	return 1;
}

__setup("ntp_tick_adj=", ntp_tick_adj_setup);

void __init ntp_init(void)
{
	ntp_clear();
	hrtimer_init(&leap_timer, CLOCK_REALTIME, HRTIMER_MODE_ABS);
	leap_timer.function = ntp_leap_second;
}
