
#include <linux/pid_namespace.h>
#include <linux/clocksource.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/console.h>
#include <linux/threads.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/reboot.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/sysrq.h>
#include <linux/init.h>
#include <linux/kgdb.h>
#include <linux/pid.h>
#include <linux/smp.h>
#include <linux/mm.h>

#include <asm/cacheflush.h>
#include <asm/byteorder.h>
#include <asm/atomic.h>
#include <asm/system.h>
#include <asm/unaligned.h>

static int kgdb_break_asap;

#define KGDB_MAX_THREAD_QUERY 17
struct kgdb_state {
	int			ex_vector;
	int			signo;
	int			err_code;
	int			cpu;
	int			pass_exception;
	unsigned long		thr_query;
	unsigned long		threadid;
	long			kgdb_usethreadid;
	struct pt_regs		*linux_regs;
};

static struct debuggerinfo_struct {
	void			*debuggerinfo;
	struct task_struct	*task;
} kgdb_info[NR_CPUS];


int				kgdb_connected;
EXPORT_SYMBOL_GPL(kgdb_connected);


static int			kgdb_io_module_registered;


static int			exception_level;

static struct kgdb_io		*kgdb_io_ops;
static DEFINE_SPINLOCK(kgdb_registration_lock);


static int kgdb_con_registered;

static int kgdb_use_con;

static int __init opt_kgdb_con(char *str)
{
	kgdb_use_con = 1;
	return 0;
}

early_param("kgdbcon", opt_kgdb_con);

module_param(kgdb_use_con, int, 0644);


static struct kgdb_bkpt		kgdb_break[KGDB_MAX_BREAKPOINTS] = {
	[0 ... KGDB_MAX_BREAKPOINTS-1] = { .state = BP_UNDEFINED }
};


atomic_t			kgdb_active = ATOMIC_INIT(-1);


static atomic_t			passive_cpu_wait[NR_CPUS];
static atomic_t			cpu_in_kgdb[NR_CPUS];
atomic_t			kgdb_setting_breakpoint;

struct task_struct		*kgdb_usethread;
struct task_struct		*kgdb_contthread;

int				kgdb_single_step;


static char			remcom_in_buffer[BUFMAX];
static char			remcom_out_buffer[BUFMAX];


static unsigned long		gdb_regs[(NUMREGBYTES +
					sizeof(unsigned long) - 1) /
					sizeof(unsigned long)];


atomic_t			kgdb_cpu_doing_single_step = ATOMIC_INIT(-1);


static int kgdb_do_roundup = 1;

static int __init opt_nokgdbroundup(char *str)
{
	kgdb_do_roundup = 0;

	return 0;
}

early_param("nokgdbroundup", opt_nokgdbroundup);




int __weak kgdb_arch_set_breakpoint(unsigned long addr, char *saved_instr)
{
	int err;

	err = probe_kernel_read(saved_instr, (char *)addr, BREAK_INSTR_SIZE);
	if (err)
		return err;

	return probe_kernel_write((char *)addr, arch_kgdb_ops.gdb_bpt_instr,
				  BREAK_INSTR_SIZE);
}

int __weak kgdb_arch_remove_breakpoint(unsigned long addr, char *bundle)
{
	return probe_kernel_write((char *)addr,
				  (char *)bundle, BREAK_INSTR_SIZE);
}

int __weak kgdb_validate_break_address(unsigned long addr)
{
	char tmp_variable[BREAK_INSTR_SIZE];
	int err;
	
	err = kgdb_arch_set_breakpoint(addr, tmp_variable);
	if (err)
		return err;
	err = kgdb_arch_remove_breakpoint(addr, tmp_variable);
	if (err)
		printk(KERN_ERR "KGDB: Critical breakpoint error, kernel "
		   "memory destroyed at: %lx", addr);
	return err;
}

unsigned long __weak kgdb_arch_pc(int exception, struct pt_regs *regs)
{
	return instruction_pointer(regs);
}

int __weak kgdb_arch_init(void)
{
	return 0;
}

int __weak kgdb_skipexception(int exception, struct pt_regs *regs)
{
	return 0;
}

void __weak
kgdb_post_primary_code(struct pt_regs *regs, int e_vector, int err_code)
{
	return;
}


void __weak kgdb_disable_hw_debug(struct pt_regs *regs)
{
}



static int hex(char ch)
{
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	return -1;
}


static void get_packet(char *buffer)
{
	unsigned char checksum;
	unsigned char xmitcsum;
	int count;
	char ch;

	do {
		
		while ((ch = (kgdb_io_ops->read_char())) != '$')
			;

		kgdb_connected = 1;
		checksum = 0;
		xmitcsum = -1;

		count = 0;

		
		while (count < (BUFMAX - 1)) {
			ch = kgdb_io_ops->read_char();
			if (ch == '#')
				break;
			checksum = checksum + ch;
			buffer[count] = ch;
			count = count + 1;
		}
		buffer[count] = 0;

		if (ch == '#') {
			xmitcsum = hex(kgdb_io_ops->read_char()) << 4;
			xmitcsum += hex(kgdb_io_ops->read_char());

			if (checksum != xmitcsum)
				
				kgdb_io_ops->write_char('-');
			else
				
				kgdb_io_ops->write_char('+');
			if (kgdb_io_ops->flush)
				kgdb_io_ops->flush();
		}
	} while (checksum != xmitcsum);
}


static void put_packet(char *buffer)
{
	unsigned char checksum;
	int count;
	char ch;

	
	while (1) {
		kgdb_io_ops->write_char('$');
		checksum = 0;
		count = 0;

		while ((ch = buffer[count])) {
			kgdb_io_ops->write_char(ch);
			checksum += ch;
			count++;
		}

		kgdb_io_ops->write_char('#');
		kgdb_io_ops->write_char(hex_asc_hi(checksum));
		kgdb_io_ops->write_char(hex_asc_lo(checksum));
		if (kgdb_io_ops->flush)
			kgdb_io_ops->flush();

		
		ch = kgdb_io_ops->read_char();

		if (ch == 3)
			ch = kgdb_io_ops->read_char();

		
		if (ch == '+')
			return;

		
		if (ch == '$') {
			kgdb_io_ops->write_char('-');
			if (kgdb_io_ops->flush)
				kgdb_io_ops->flush();
			return;
		}
	}
}


int kgdb_mem2hex(char *mem, char *buf, int count)
{
	char *tmp;
	int err;

	
	tmp = buf + count;

	err = probe_kernel_read(tmp, mem, count);
	if (!err) {
		while (count > 0) {
			buf = pack_hex_byte(buf, *tmp);
			tmp++;
			count--;
		}

		*buf = 0;
	}

	return err;
}


static int kgdb_ebin2mem(char *buf, char *mem, int count)
{
	int err = 0;
	char c;

	while (count-- > 0) {
		c = *buf++;
		if (c == 0x7d)
			c = *buf++ ^ 0x20;

		err = probe_kernel_write(mem, &c, 1);
		if (err)
			break;

		mem++;
	}

	return err;
}


int kgdb_hex2mem(char *buf, char *mem, int count)
{
	char *tmp_raw;
	char *tmp_hex;

	
	tmp_raw = buf + count * 2;

	tmp_hex = tmp_raw - 1;
	while (tmp_hex >= buf) {
		tmp_raw--;
		*tmp_raw = hex(*tmp_hex--);
		*tmp_raw |= hex(*tmp_hex--) << 4;
	}

	return probe_kernel_write(mem, tmp_raw, count);
}


int kgdb_hex2long(char **ptr, unsigned long *long_val)
{
	int hex_val;
	int num = 0;
	int negate = 0;

	*long_val = 0;

	if (**ptr == '-') {
		negate = 1;
		(*ptr)++;
	}
	while (**ptr) {
		hex_val = hex(**ptr);
		if (hex_val < 0)
			break;

		*long_val = (*long_val << 4) | hex_val;
		num++;
		(*ptr)++;
	}

	if (negate)
		*long_val = -*long_val;

	return num;
}


static int write_mem_msg(int binary)
{
	char *ptr = &remcom_in_buffer[1];
	unsigned long addr;
	unsigned long length;
	int err;

	if (kgdb_hex2long(&ptr, &addr) > 0 && *(ptr++) == ',' &&
	    kgdb_hex2long(&ptr, &length) > 0 && *(ptr++) == ':') {
		if (binary)
			err = kgdb_ebin2mem(ptr, (char *)addr, length);
		else
			err = kgdb_hex2mem(ptr, (char *)addr, length);
		if (err)
			return err;
		if (CACHE_FLUSH_IS_SAFE)
			flush_icache_range(addr, addr + length);
		return 0;
	}

	return -EINVAL;
}

static void error_packet(char *pkt, int error)
{
	error = -error;
	pkt[0] = 'E';
	pkt[1] = hex_asc[(error / 10)];
	pkt[2] = hex_asc[(error % 10)];
	pkt[3] = '\0';
}



#define BUF_THREAD_ID_SIZE	16

static char *pack_threadid(char *pkt, unsigned char *id)
{
	char *limit;

	limit = pkt + BUF_THREAD_ID_SIZE;
	while (pkt < limit)
		pkt = pack_hex_byte(pkt, *id++);

	return pkt;
}

static void int_to_threadref(unsigned char *id, int value)
{
	unsigned char *scan;
	int i = 4;

	scan = (unsigned char *)id;
	while (i--)
		*scan++ = 0;
	put_unaligned_be32(value, scan);
}

static struct task_struct *getthread(struct pt_regs *regs, int tid)
{
	
	if (tid == 0 || tid == -1)
		tid = -atomic_read(&kgdb_active) - 2;
	if (tid < 0) {
		if (kgdb_info[-tid - 2].task)
			return kgdb_info[-tid - 2].task;
		else
			return idle_task(-tid - 2);
	}

	
	return find_task_by_pid_ns(tid, &init_pid_ns);
}



#ifdef CONFIG_SMP
static void kgdb_wait(struct pt_regs *regs)
{
	unsigned long flags;
	int cpu;

	local_irq_save(flags);
	cpu = raw_smp_processor_id();
	kgdb_info[cpu].debuggerinfo = regs;
	kgdb_info[cpu].task = current;
	
	smp_wmb();
	atomic_set(&cpu_in_kgdb[cpu], 1);

	
	while (atomic_read(&passive_cpu_wait[cpu]))
		cpu_relax();

	kgdb_info[cpu].debuggerinfo = NULL;
	kgdb_info[cpu].task = NULL;

	
	if (arch_kgdb_ops.correct_hw_break)
		arch_kgdb_ops.correct_hw_break();

	
	atomic_set(&cpu_in_kgdb[cpu], 0);
	touch_softlockup_watchdog();
	clocksource_touch_watchdog();
	local_irq_restore(flags);
}
#endif


static void kgdb_flush_swbreak_addr(unsigned long addr)
{
	if (!CACHE_FLUSH_IS_SAFE)
		return;

	if (current->mm && current->mm->mmap_cache) {
		flush_cache_range(current->mm->mmap_cache,
				  addr, addr + BREAK_INSTR_SIZE);
	}
	
	flush_icache_range(addr, addr + BREAK_INSTR_SIZE);
}


static int kgdb_activate_sw_breakpoints(void)
{
	unsigned long addr;
	int error = 0;
	int i;

	for (i = 0; i < KGDB_MAX_BREAKPOINTS; i++) {
		if (kgdb_break[i].state != BP_SET)
			continue;

		addr = kgdb_break[i].bpt_addr;
		error = kgdb_arch_set_breakpoint(addr,
				kgdb_break[i].saved_instr);
		if (error)
			return error;

		kgdb_flush_swbreak_addr(addr);
		kgdb_break[i].state = BP_ACTIVE;
	}
	return 0;
}

static int kgdb_set_sw_break(unsigned long addr)
{
	int err = kgdb_validate_break_address(addr);
	int breakno = -1;
	int i;

	if (err)
		return err;

	for (i = 0; i < KGDB_MAX_BREAKPOINTS; i++) {
		if ((kgdb_break[i].state == BP_SET) &&
					(kgdb_break[i].bpt_addr == addr))
			return -EEXIST;
	}
	for (i = 0; i < KGDB_MAX_BREAKPOINTS; i++) {
		if (kgdb_break[i].state == BP_REMOVED &&
					kgdb_break[i].bpt_addr == addr) {
			breakno = i;
			break;
		}
	}

	if (breakno == -1) {
		for (i = 0; i < KGDB_MAX_BREAKPOINTS; i++) {
			if (kgdb_break[i].state == BP_UNDEFINED) {
				breakno = i;
				break;
			}
		}
	}

	if (breakno == -1)
		return -E2BIG;

	kgdb_break[breakno].state = BP_SET;
	kgdb_break[breakno].type = BP_BREAKPOINT;
	kgdb_break[breakno].bpt_addr = addr;

	return 0;
}

static int kgdb_deactivate_sw_breakpoints(void)
{
	unsigned long addr;
	int error = 0;
	int i;

	for (i = 0; i < KGDB_MAX_BREAKPOINTS; i++) {
		if (kgdb_break[i].state != BP_ACTIVE)
			continue;
		addr = kgdb_break[i].bpt_addr;
		error = kgdb_arch_remove_breakpoint(addr,
					kgdb_break[i].saved_instr);
		if (error)
			return error;

		kgdb_flush_swbreak_addr(addr);
		kgdb_break[i].state = BP_SET;
	}
	return 0;
}

static int kgdb_remove_sw_break(unsigned long addr)
{
	int i;

	for (i = 0; i < KGDB_MAX_BREAKPOINTS; i++) {
		if ((kgdb_break[i].state == BP_SET) &&
				(kgdb_break[i].bpt_addr == addr)) {
			kgdb_break[i].state = BP_REMOVED;
			return 0;
		}
	}
	return -ENOENT;
}

int kgdb_isremovedbreak(unsigned long addr)
{
	int i;

	for (i = 0; i < KGDB_MAX_BREAKPOINTS; i++) {
		if ((kgdb_break[i].state == BP_REMOVED) &&
					(kgdb_break[i].bpt_addr == addr))
			return 1;
	}
	return 0;
}

static int remove_all_break(void)
{
	unsigned long addr;
	int error;
	int i;

	
	for (i = 0; i < KGDB_MAX_BREAKPOINTS; i++) {
		if (kgdb_break[i].state != BP_ACTIVE)
			goto setundefined;
		addr = kgdb_break[i].bpt_addr;
		error = kgdb_arch_remove_breakpoint(addr,
				kgdb_break[i].saved_instr);
		if (error)
			printk(KERN_ERR "KGDB: breakpoint remove failed: %lx\n",
			   addr);
setundefined:
		kgdb_break[i].state = BP_UNDEFINED;
	}

	
	if (arch_kgdb_ops.remove_all_hw_break)
		arch_kgdb_ops.remove_all_hw_break();

	return 0;
}


static inline int shadow_pid(int realpid)
{
	if (realpid)
		return realpid;

	return -raw_smp_processor_id() - 2;
}

static char gdbmsgbuf[BUFMAX + 1];

static void kgdb_msg_write(const char *s, int len)
{
	char *bufptr;
	int wcount;
	int i;

	
	gdbmsgbuf[0] = 'O';

	
	while (len > 0) {
		bufptr = gdbmsgbuf + 1;

		
		if ((len << 1) > (BUFMAX - 2))
			wcount = (BUFMAX - 2) >> 1;
		else
			wcount = len;

		
		for (i = 0; i < wcount; i++)
			bufptr = pack_hex_byte(bufptr, s[i]);
		*bufptr = '\0';

		
		s += wcount;
		len -= wcount;

		
		put_packet(gdbmsgbuf);
	}
}


static int kgdb_io_ready(int print_wait)
{
	if (!kgdb_io_ops)
		return 0;
	if (kgdb_connected)
		return 1;
	if (atomic_read(&kgdb_setting_breakpoint))
		return 1;
	if (print_wait)
		printk(KERN_CRIT "KGDB: Waiting for remote debugger\n");
	return 1;
}




static void gdb_cmd_status(struct kgdb_state *ks)
{
	
	remove_all_break();

	remcom_out_buffer[0] = 'S';
	pack_hex_byte(&remcom_out_buffer[1], ks->signo);
}


static void gdb_cmd_getregs(struct kgdb_state *ks)
{
	struct task_struct *thread;
	void *local_debuggerinfo;
	int i;

	thread = kgdb_usethread;
	if (!thread) {
		thread = kgdb_info[ks->cpu].task;
		local_debuggerinfo = kgdb_info[ks->cpu].debuggerinfo;
	} else {
		local_debuggerinfo = NULL;
		for_each_online_cpu(i) {
			
			if (thread == kgdb_info[i].task)
				local_debuggerinfo = kgdb_info[i].debuggerinfo;
		}
	}

	
	if (local_debuggerinfo) {
		pt_regs_to_gdb_regs(gdb_regs, local_debuggerinfo);
	} else {
		
		sleeping_thread_to_gdb_regs(gdb_regs, thread);
	}
	kgdb_mem2hex((char *)gdb_regs, remcom_out_buffer, NUMREGBYTES);
}


static void gdb_cmd_setregs(struct kgdb_state *ks)
{
	kgdb_hex2mem(&remcom_in_buffer[1], (char *)gdb_regs, NUMREGBYTES);

	if (kgdb_usethread && kgdb_usethread != current) {
		error_packet(remcom_out_buffer, -EINVAL);
	} else {
		gdb_regs_to_pt_regs(gdb_regs, ks->linux_regs);
		strcpy(remcom_out_buffer, "OK");
	}
}


static void gdb_cmd_memread(struct kgdb_state *ks)
{
	char *ptr = &remcom_in_buffer[1];
	unsigned long length;
	unsigned long addr;
	int err;

	if (kgdb_hex2long(&ptr, &addr) > 0 && *ptr++ == ',' &&
					kgdb_hex2long(&ptr, &length) > 0) {
		err = kgdb_mem2hex((char *)addr, remcom_out_buffer, length);
		if (err)
			error_packet(remcom_out_buffer, err);
	} else {
		error_packet(remcom_out_buffer, -EINVAL);
	}
}


static void gdb_cmd_memwrite(struct kgdb_state *ks)
{
	int err = write_mem_msg(0);

	if (err)
		error_packet(remcom_out_buffer, err);
	else
		strcpy(remcom_out_buffer, "OK");
}


static void gdb_cmd_binwrite(struct kgdb_state *ks)
{
	int err = write_mem_msg(1);

	if (err)
		error_packet(remcom_out_buffer, err);
	else
		strcpy(remcom_out_buffer, "OK");
}


static void gdb_cmd_detachkill(struct kgdb_state *ks)
{
	int error;

	
	if (remcom_in_buffer[0] == 'D') {
		error = remove_all_break();
		if (error < 0) {
			error_packet(remcom_out_buffer, error);
		} else {
			strcpy(remcom_out_buffer, "OK");
			kgdb_connected = 0;
		}
		put_packet(remcom_out_buffer);
	} else {
		
		remove_all_break();
		kgdb_connected = 0;
	}
}


static int gdb_cmd_reboot(struct kgdb_state *ks)
{
	
	if (strcmp(remcom_in_buffer, "R0") == 0) {
		printk(KERN_CRIT "Executing emergency reboot\n");
		strcpy(remcom_out_buffer, "OK");
		put_packet(remcom_out_buffer);

		
		machine_emergency_restart();
		kgdb_connected = 0;

		return 1;
	}
	return 0;
}


static void gdb_cmd_query(struct kgdb_state *ks)
{
	struct task_struct *g;
	struct task_struct *p;
	unsigned char thref[8];
	char *ptr;
	int i;
	int cpu;
	int finished = 0;

	switch (remcom_in_buffer[1]) {
	case 's':
	case 'f':
		if (memcmp(remcom_in_buffer + 2, "ThreadInfo", 10)) {
			error_packet(remcom_out_buffer, -EINVAL);
			break;
		}

		i = 0;
		remcom_out_buffer[0] = 'm';
		ptr = remcom_out_buffer + 1;
		if (remcom_in_buffer[1] == 'f') {
			
			for_each_online_cpu(cpu) {
				ks->thr_query = 0;
				int_to_threadref(thref, -cpu - 2);
				pack_threadid(ptr, thref);
				ptr += BUF_THREAD_ID_SIZE;
				*(ptr++) = ',';
				i++;
			}
		}

		do_each_thread(g, p) {
			if (i >= ks->thr_query && !finished) {
				int_to_threadref(thref, p->pid);
				pack_threadid(ptr, thref);
				ptr += BUF_THREAD_ID_SIZE;
				*(ptr++) = ',';
				ks->thr_query++;
				if (ks->thr_query % KGDB_MAX_THREAD_QUERY == 0)
					finished = 1;
			}
			i++;
		} while_each_thread(g, p);

		*(--ptr) = '\0';
		break;

	case 'C':
		
		strcpy(remcom_out_buffer, "QC");
		ks->threadid = shadow_pid(current->pid);
		int_to_threadref(thref, ks->threadid);
		pack_threadid(remcom_out_buffer + 2, thref);
		break;
	case 'T':
		if (memcmp(remcom_in_buffer + 1, "ThreadExtraInfo,", 16)) {
			error_packet(remcom_out_buffer, -EINVAL);
			break;
		}
		ks->threadid = 0;
		ptr = remcom_in_buffer + 17;
		kgdb_hex2long(&ptr, &ks->threadid);
		if (!getthread(ks->linux_regs, ks->threadid)) {
			error_packet(remcom_out_buffer, -EINVAL);
			break;
		}
		if ((int)ks->threadid > 0) {
			kgdb_mem2hex(getthread(ks->linux_regs,
					ks->threadid)->comm,
					remcom_out_buffer, 16);
		} else {
			static char tmpstr[23 + BUF_THREAD_ID_SIZE];

			sprintf(tmpstr, "shadowCPU%d",
					(int)(-ks->threadid - 2));
			kgdb_mem2hex(tmpstr, remcom_out_buffer, strlen(tmpstr));
		}
		break;
	}
}


static void gdb_cmd_task(struct kgdb_state *ks)
{
	struct task_struct *thread;
	char *ptr;

	switch (remcom_in_buffer[1]) {
	case 'g':
		ptr = &remcom_in_buffer[2];
		kgdb_hex2long(&ptr, &ks->threadid);
		thread = getthread(ks->linux_regs, ks->threadid);
		if (!thread && ks->threadid > 0) {
			error_packet(remcom_out_buffer, -EINVAL);
			break;
		}
		kgdb_usethread = thread;
		ks->kgdb_usethreadid = ks->threadid;
		strcpy(remcom_out_buffer, "OK");
		break;
	case 'c':
		ptr = &remcom_in_buffer[2];
		kgdb_hex2long(&ptr, &ks->threadid);
		if (!ks->threadid) {
			kgdb_contthread = NULL;
		} else {
			thread = getthread(ks->linux_regs, ks->threadid);
			if (!thread && ks->threadid > 0) {
				error_packet(remcom_out_buffer, -EINVAL);
				break;
			}
			kgdb_contthread = thread;
		}
		strcpy(remcom_out_buffer, "OK");
		break;
	}
}


static void gdb_cmd_thread(struct kgdb_state *ks)
{
	char *ptr = &remcom_in_buffer[1];
	struct task_struct *thread;

	kgdb_hex2long(&ptr, &ks->threadid);
	thread = getthread(ks->linux_regs, ks->threadid);
	if (thread)
		strcpy(remcom_out_buffer, "OK");
	else
		error_packet(remcom_out_buffer, -EINVAL);
}


static void gdb_cmd_break(struct kgdb_state *ks)
{
	
	char *bpt_type = &remcom_in_buffer[1];
	char *ptr = &remcom_in_buffer[2];
	unsigned long addr;
	unsigned long length;
	int error = 0;

	if (arch_kgdb_ops.set_hw_breakpoint && *bpt_type >= '1') {
		
		if (*bpt_type > '4')
			return;
	} else {
		if (*bpt_type != '0' && *bpt_type != '1')
			
			return;
	}

	
	if (*bpt_type == '1' && !(arch_kgdb_ops.flags & KGDB_HW_BREAKPOINT))
		
		return;

	if (*(ptr++) != ',') {
		error_packet(remcom_out_buffer, -EINVAL);
		return;
	}
	if (!kgdb_hex2long(&ptr, &addr)) {
		error_packet(remcom_out_buffer, -EINVAL);
		return;
	}
	if (*(ptr++) != ',' ||
		!kgdb_hex2long(&ptr, &length)) {
		error_packet(remcom_out_buffer, -EINVAL);
		return;
	}

	if (remcom_in_buffer[0] == 'Z' && *bpt_type == '0')
		error = kgdb_set_sw_break(addr);
	else if (remcom_in_buffer[0] == 'z' && *bpt_type == '0')
		error = kgdb_remove_sw_break(addr);
	else if (remcom_in_buffer[0] == 'Z')
		error = arch_kgdb_ops.set_hw_breakpoint(addr,
			(int)length, *bpt_type - '0');
	else if (remcom_in_buffer[0] == 'z')
		error = arch_kgdb_ops.remove_hw_breakpoint(addr,
			(int) length, *bpt_type - '0');

	if (error == 0)
		strcpy(remcom_out_buffer, "OK");
	else
		error_packet(remcom_out_buffer, error);
}


static int gdb_cmd_exception_pass(struct kgdb_state *ks)
{
	
	if (remcom_in_buffer[1] == '0' && remcom_in_buffer[2] == '9') {

		ks->pass_exception = 1;
		remcom_in_buffer[0] = 'c';

	} else if (remcom_in_buffer[1] == '1' && remcom_in_buffer[2] == '5') {

		ks->pass_exception = 1;
		remcom_in_buffer[0] = 'D';
		remove_all_break();
		kgdb_connected = 0;
		return 1;

	} else {
		error_packet(remcom_out_buffer, -EINVAL);
		return 0;
	}

	
	return -1;
}


static int gdb_serial_stub(struct kgdb_state *ks)
{
	int error = 0;
	int tmp;

	
	memset(remcom_out_buffer, 0, sizeof(remcom_out_buffer));

	if (kgdb_connected) {
		unsigned char thref[8];
		char *ptr;

		
		ptr = remcom_out_buffer;
		*ptr++ = 'T';
		ptr = pack_hex_byte(ptr, ks->signo);
		ptr += strlen(strcpy(ptr, "thread:"));
		int_to_threadref(thref, shadow_pid(current->pid));
		ptr = pack_threadid(ptr, thref);
		*ptr++ = ';';
		put_packet(remcom_out_buffer);
	}

	kgdb_usethread = kgdb_info[ks->cpu].task;
	ks->kgdb_usethreadid = shadow_pid(kgdb_info[ks->cpu].task->pid);
	ks->pass_exception = 0;

	while (1) {
		error = 0;

		
		memset(remcom_out_buffer, 0, sizeof(remcom_out_buffer));

		get_packet(remcom_in_buffer);

		switch (remcom_in_buffer[0]) {
		case '?': 
			gdb_cmd_status(ks);
			break;
		case 'g': 
			gdb_cmd_getregs(ks);
			break;
		case 'G': 
			gdb_cmd_setregs(ks);
			break;
		case 'm': 
			gdb_cmd_memread(ks);
			break;
		case 'M': 
			gdb_cmd_memwrite(ks);
			break;
		case 'X': 
			gdb_cmd_binwrite(ks);
			break;
			
		case 'D': 
		case 'k': 
			gdb_cmd_detachkill(ks);
			goto default_handle;
		case 'R': 
			if (gdb_cmd_reboot(ks))
				goto default_handle;
			break;
		case 'q': 
			gdb_cmd_query(ks);
			break;
		case 'H': 
			gdb_cmd_task(ks);
			break;
		case 'T': 
			gdb_cmd_thread(ks);
			break;
		case 'z': 
		case 'Z': 
			gdb_cmd_break(ks);
			break;
		case 'C': 
			tmp = gdb_cmd_exception_pass(ks);
			if (tmp > 0)
				goto default_handle;
			if (tmp == 0)
				break;
			
		case 'c': 
		case 's': 
			if (kgdb_contthread && kgdb_contthread != current) {
				
				error_packet(remcom_out_buffer, -EINVAL);
				break;
			}
			kgdb_activate_sw_breakpoints();
			
		default:
default_handle:
			error = kgdb_arch_handle_exception(ks->ex_vector,
						ks->signo,
						ks->err_code,
						remcom_in_buffer,
						remcom_out_buffer,
						ks->linux_regs);
			
			if (error >= 0 || remcom_in_buffer[0] == 'D' ||
			    remcom_in_buffer[0] == 'k') {
				error = 0;
				goto kgdb_exit;
			}

		}

		
		put_packet(remcom_out_buffer);
	}

kgdb_exit:
	if (ks->pass_exception)
		error = 1;
	return error;
}

static int kgdb_reenter_check(struct kgdb_state *ks)
{
	unsigned long addr;

	if (atomic_read(&kgdb_active) != raw_smp_processor_id())
		return 0;

	
	exception_level++;
	addr = kgdb_arch_pc(ks->ex_vector, ks->linux_regs);
	kgdb_deactivate_sw_breakpoints();

	
	if (kgdb_remove_sw_break(addr) == 0) {
		exception_level = 0;
		kgdb_skipexception(ks->ex_vector, ks->linux_regs);
		kgdb_activate_sw_breakpoints();
		printk(KERN_CRIT "KGDB: re-enter error: breakpoint removed %lx\n",
			addr);
		WARN_ON_ONCE(1);

		return 1;
	}
	remove_all_break();
	kgdb_skipexception(ks->ex_vector, ks->linux_regs);

	if (exception_level > 1) {
		dump_stack();
		panic("Recursive entry to debugger");
	}

	printk(KERN_CRIT "KGDB: re-enter exception: ALL breakpoints killed\n");
	dump_stack();
	panic("Recursive entry to debugger");

	return 1;
}


int
kgdb_handle_exception(int evector, int signo, int ecode, struct pt_regs *regs)
{
	struct kgdb_state kgdb_var;
	struct kgdb_state *ks = &kgdb_var;
	unsigned long flags;
	int error = 0;
	int i, cpu;

	ks->cpu			= raw_smp_processor_id();
	ks->ex_vector		= evector;
	ks->signo		= signo;
	ks->ex_vector		= evector;
	ks->err_code		= ecode;
	ks->kgdb_usethreadid	= 0;
	ks->linux_regs		= regs;

	if (kgdb_reenter_check(ks))
		return 0; 

acquirelock:
	
	local_irq_save(flags);

	cpu = raw_smp_processor_id();

	
	while (atomic_cmpxchg(&kgdb_active, -1, cpu) != -1)
		cpu_relax();

	
	if (atomic_read(&kgdb_cpu_doing_single_step) != -1 &&
	    atomic_read(&kgdb_cpu_doing_single_step) != cpu) {

		atomic_set(&kgdb_active, -1);
		touch_softlockup_watchdog();
		clocksource_touch_watchdog();
		local_irq_restore(flags);

		goto acquirelock;
	}

	if (!kgdb_io_ready(1)) {
		error = 1;
		goto kgdb_restore; 
	}

	
	if (kgdb_skipexception(ks->ex_vector, ks->linux_regs))
		goto kgdb_restore;

	
	if (kgdb_io_ops->pre_exception)
		kgdb_io_ops->pre_exception();

	kgdb_info[ks->cpu].debuggerinfo = ks->linux_regs;
	kgdb_info[ks->cpu].task = current;

	kgdb_disable_hw_debug(ks->linux_regs);

	
	if (!kgdb_single_step) {
		for (i = 0; i < NR_CPUS; i++)
			atomic_set(&passive_cpu_wait[i], 1);
	}

	
	atomic_set(&cpu_in_kgdb[ks->cpu], 1);

#ifdef CONFIG_SMP
	
	if ((!kgdb_single_step) && kgdb_do_roundup)
		kgdb_roundup_cpus(flags);
#endif

	
	for_each_online_cpu(i) {
		while (!atomic_read(&cpu_in_kgdb[i]))
			cpu_relax();
	}

	
	kgdb_post_primary_code(ks->linux_regs, ks->ex_vector, ks->err_code);
	kgdb_deactivate_sw_breakpoints();
	kgdb_single_step = 0;
	kgdb_contthread = current;
	exception_level = 0;

	
	error = gdb_serial_stub(ks);

	
	if (kgdb_io_ops->post_exception)
		kgdb_io_ops->post_exception();

	kgdb_info[ks->cpu].debuggerinfo = NULL;
	kgdb_info[ks->cpu].task = NULL;
	atomic_set(&cpu_in_kgdb[ks->cpu], 0);

	if (!kgdb_single_step) {
		for (i = NR_CPUS-1; i >= 0; i--)
			atomic_set(&passive_cpu_wait[i], 0);
		
		for_each_online_cpu(i) {
			while (atomic_read(&cpu_in_kgdb[i]))
				cpu_relax();
		}
	}

kgdb_restore:
	
	atomic_set(&kgdb_active, -1);
	touch_softlockup_watchdog();
	clocksource_touch_watchdog();
	local_irq_restore(flags);

	return error;
}

int kgdb_nmicallback(int cpu, void *regs)
{
#ifdef CONFIG_SMP
	if (!atomic_read(&cpu_in_kgdb[cpu]) &&
			atomic_read(&kgdb_active) != cpu &&
			atomic_read(&cpu_in_kgdb[atomic_read(&kgdb_active)])) {
		kgdb_wait((struct pt_regs *)regs);
		return 0;
	}
#endif
	return 1;
}

static void kgdb_console_write(struct console *co, const char *s,
   unsigned count)
{
	unsigned long flags;

	
	if (!kgdb_connected || atomic_read(&kgdb_active) != -1)
		return;

	local_irq_save(flags);
	kgdb_msg_write(s, count);
	local_irq_restore(flags);
}

static struct console kgdbcons = {
	.name		= "kgdb",
	.write		= kgdb_console_write,
	.flags		= CON_PRINTBUFFER | CON_ENABLED,
	.index		= -1,
};

#ifdef CONFIG_MAGIC_SYSRQ
static void sysrq_handle_gdb(int key, struct tty_struct *tty)
{
	if (!kgdb_io_ops) {
		printk(KERN_CRIT "ERROR: No KGDB I/O module available\n");
		return;
	}
	if (!kgdb_connected)
		printk(KERN_CRIT "Entering KGDB\n");

	kgdb_breakpoint();
}

static struct sysrq_key_op sysrq_gdb_op = {
	.handler	= sysrq_handle_gdb,
	.help_msg	= "debug(G)",
	.action_msg	= "DEBUG",
};
#endif

static void kgdb_register_callbacks(void)
{
	if (!kgdb_io_module_registered) {
		kgdb_io_module_registered = 1;
		kgdb_arch_init();
#ifdef CONFIG_MAGIC_SYSRQ
		register_sysrq_key('g', &sysrq_gdb_op);
#endif
		if (kgdb_use_con && !kgdb_con_registered) {
			register_console(&kgdbcons);
			kgdb_con_registered = 1;
		}
	}
}

static void kgdb_unregister_callbacks(void)
{
	
	if (kgdb_io_module_registered) {
		kgdb_io_module_registered = 0;
		kgdb_arch_exit();
#ifdef CONFIG_MAGIC_SYSRQ
		unregister_sysrq_key('g', &sysrq_gdb_op);
#endif
		if (kgdb_con_registered) {
			unregister_console(&kgdbcons);
			kgdb_con_registered = 0;
		}
	}
}

static void kgdb_initial_breakpoint(void)
{
	kgdb_break_asap = 0;

	printk(KERN_CRIT "kgdb: Waiting for connection from remote gdb...\n");
	kgdb_breakpoint();
}


int kgdb_register_io_module(struct kgdb_io *new_kgdb_io_ops)
{
	int err;

	spin_lock(&kgdb_registration_lock);

	if (kgdb_io_ops) {
		spin_unlock(&kgdb_registration_lock);

		printk(KERN_ERR "kgdb: Another I/O driver is already "
				"registered with KGDB.\n");
		return -EBUSY;
	}

	if (new_kgdb_io_ops->init) {
		err = new_kgdb_io_ops->init();
		if (err) {
			spin_unlock(&kgdb_registration_lock);
			return err;
		}
	}

	kgdb_io_ops = new_kgdb_io_ops;

	spin_unlock(&kgdb_registration_lock);

	printk(KERN_INFO "kgdb: Registered I/O driver %s.\n",
	       new_kgdb_io_ops->name);

	
	kgdb_register_callbacks();

	if (kgdb_break_asap)
		kgdb_initial_breakpoint();

	return 0;
}
EXPORT_SYMBOL_GPL(kgdb_register_io_module);


void kgdb_unregister_io_module(struct kgdb_io *old_kgdb_io_ops)
{
	BUG_ON(kgdb_connected);

	
	kgdb_unregister_callbacks();

	spin_lock(&kgdb_registration_lock);

	WARN_ON_ONCE(kgdb_io_ops != old_kgdb_io_ops);
	kgdb_io_ops = NULL;

	spin_unlock(&kgdb_registration_lock);

	printk(KERN_INFO
		"kgdb: Unregistered I/O driver %s, debugger disabled.\n",
		old_kgdb_io_ops->name);
}
EXPORT_SYMBOL_GPL(kgdb_unregister_io_module);


void kgdb_breakpoint(void)
{
	atomic_set(&kgdb_setting_breakpoint, 1);
	wmb(); 
	arch_kgdb_breakpoint();
	wmb(); 
	atomic_set(&kgdb_setting_breakpoint, 0);
}
EXPORT_SYMBOL_GPL(kgdb_breakpoint);

static int __init opt_kgdb_wait(char *str)
{
	kgdb_break_asap = 1;

	if (kgdb_io_module_registered)
		kgdb_initial_breakpoint();

	return 0;
}

early_param("kgdbwait", opt_kgdb_wait);
