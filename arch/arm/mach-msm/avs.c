

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kernel_stat.h>
#include <linux/workqueue.h>

#include "avs.h"

#define AVSDSCR_INPUT 0x01004860 
#define TSCSR_INPUT   0x00000001 

#define TEMPRS 16                
#define GET_TEMPR() (avs_get_tscsr() >> 28) 

struct mutex avs_lock;

static struct avs_state_s
{
	u32 freq_cnt;		
	short *avs_v;		
	int (*set_vdd) (int);	
	int changing;		
	u32 freq_idx;		
	int vdd;		
} avs_state;


static void avs_update_voltage_table(short *vdd_table)
{
	u32 avscsr;
	int cpu;
	int vu;
	int l2;
	int i;
	u32 cur_freq_idx;
	short cur_voltage;

	cur_freq_idx = avs_state.freq_idx;
	cur_voltage = avs_state.vdd;

	avscsr = avs_test_delays();
	AVSDEBUG("avscsr=%x, avsdscr=%x\n", avscsr, avs_get_avsdscr());

	
	cpu = ((avscsr >> 23) & 2) + ((avscsr >> 16) & 1);
	vu  = ((avscsr >> 28) & 2) + ((avscsr >> 21) & 1);
	l2  = ((avscsr >> 29) & 2) + ((avscsr >> 22) & 1);

	if ((cpu == 3) || (vu == 3) || (l2 == 3)) {
		printk(KERN_ERR "AVS: Dly Synth O/P error\n");
	} else if ((cpu == 2) || (l2 == 2) || (vu == 2)) {
		
		AVSDEBUG("cpu=%d l2=%d vu=%d\n", cpu, l2, vu);
		AVSDEBUG("Voltage up at %d\n", cur_freq_idx);

		if (cur_voltage >= VOLTAGE_MAX)
			printk(KERN_ERR
				"AVS: Voltage can not get high enough!\n");

		
		for (i = 0; i < avs_state.freq_cnt; i++) {
			vdd_table[i] = cur_voltage + VOLTAGE_STEP;
			if (vdd_table[i] > VOLTAGE_MAX)
				vdd_table[i] = VOLTAGE_MAX;
		}
	} else if ((cpu == 1) && (l2 == 1) && (vu == 1)) {
		if ((cur_voltage - VOLTAGE_STEP >= VOLTAGE_MIN) &&
		    (cur_voltage <= vdd_table[cur_freq_idx])) {
			vdd_table[cur_freq_idx] = cur_voltage - VOLTAGE_STEP;
			AVSDEBUG("Voltage down for %d and lower levels\n",
				cur_freq_idx);

			
			for (i = 0; i < cur_freq_idx; i++) {
				if (vdd_table[i] > vdd_table[cur_freq_idx])
					vdd_table[i] = vdd_table[cur_freq_idx];
			}
		}
	}
}


static short avs_get_target_voltage(int freq_idx, bool update_table)
{
	unsigned cur_tempr = GET_TEMPR();
	unsigned temp_index = cur_tempr*avs_state.freq_cnt;

	
	short *vdd_table = avs_state.avs_v + temp_index;

	if (update_table)
		avs_update_voltage_table(vdd_table);

	return vdd_table[freq_idx];
}



static int avs_set_target_voltage(int freq_idx, bool update_table)
{
	int rc = 0;
	int new_voltage = avs_get_target_voltage(freq_idx, update_table);
	if (avs_state.vdd != new_voltage) {
		AVSDEBUG("AVS setting V to %d mV @%d\n",
			new_voltage, freq_idx);
		rc = avs_state.set_vdd(new_voltage);
		if (rc)
			return rc;
		avs_state.vdd = new_voltage;
	}
	return rc;
}


int avs_adjust_freq(u32 freq_idx, int begin)
{
	int rc = 0;

	if (!avs_state.set_vdd) {
		
		return 0;
	}

	if (freq_idx >= avs_state.freq_cnt) {
		AVSDEBUG("Out of range :%d\n", freq_idx);
		return -EINVAL;
	}

	mutex_lock(&avs_lock);
	if ((begin && (freq_idx > avs_state.freq_idx)) ||
	    (!begin && (freq_idx < avs_state.freq_idx))) {
		
		rc = avs_set_target_voltage(freq_idx, 0);
		if (rc)
			goto aaf_out;

		avs_state.freq_idx = freq_idx;
	}
	avs_state.changing = begin;
aaf_out:
	mutex_unlock(&avs_lock);

	return rc;
}


static struct delayed_work avs_work;
static struct workqueue_struct  *kavs_wq;
#define AVS_DELAY ((CONFIG_HZ * 50 + 999) / 1000)

static void do_avs_timer(struct work_struct *work)
{
	int cur_freq_idx;

	mutex_lock(&avs_lock);
	if (!avs_state.changing) {
		
		cur_freq_idx = avs_state.freq_idx;
		avs_set_target_voltage(cur_freq_idx, 1);
	}
	mutex_unlock(&avs_lock);
	queue_delayed_work_on(0, kavs_wq, &avs_work, AVS_DELAY);
}


static void __init avs_timer_init(void)
{
	INIT_DELAYED_WORK_DEFERRABLE(&avs_work, do_avs_timer);
	queue_delayed_work_on(0, kavs_wq, &avs_work, AVS_DELAY);
}

static void __exit avs_timer_exit(void)
{
	cancel_delayed_work(&avs_work);
}

static int __init avs_work_init(void)
{
	kavs_wq = create_workqueue("avs");
	if (!kavs_wq) {
		printk(KERN_ERR "AVS initialization failed\n");
		return -EFAULT;
	}
	avs_timer_init();

	return 1;
}

static void __exit avs_work_exit(void)
{
	avs_timer_exit();
	destroy_workqueue(kavs_wq);
}

int __init avs_init(int (*set_vdd)(int), u32 freq_cnt, u32 freq_idx)
{
	int i;

	mutex_init(&avs_lock);

	if (freq_cnt == 0)
		return -EINVAL;

	avs_state.freq_cnt = freq_cnt;

	if (freq_idx >= avs_state.freq_cnt)
		return -EINVAL;

	avs_state.avs_v = kmalloc(TEMPRS * avs_state.freq_cnt *
		sizeof(avs_state.avs_v[0]), GFP_KERNEL);

	if (avs_state.avs_v == 0)
		return -ENOMEM;

	for (i = 0; i < TEMPRS*avs_state.freq_cnt; i++)
		avs_state.avs_v[i] = VOLTAGE_MAX;

	avs_reset_delays(AVSDSCR_INPUT);
	avs_set_tscsr(TSCSR_INPUT);

	avs_state.set_vdd = set_vdd;
	avs_state.changing = 0;
	avs_state.freq_idx = -1;
	avs_state.vdd = -1;
	avs_adjust_freq(freq_idx, 0);

	avs_work_init();

	return 0;
}

void __exit avs_exit()
{
	avs_work_exit();

	kfree(avs_state.avs_v);
}


