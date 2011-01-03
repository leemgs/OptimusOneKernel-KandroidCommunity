

#ifndef _ASM_X86_SYSCALLS_H
#define _ASM_X86_SYSCALLS_H

#include <linux/compiler.h>
#include <linux/linkage.h>
#include <linux/signal.h>
#include <linux/types.h>



asmlinkage long sys_ioperm(unsigned long, unsigned long, int);


int sys_fork(struct pt_regs *);
int sys_vfork(struct pt_regs *);


asmlinkage int sys_modify_ldt(int, void __user *, unsigned long);


long sys_rt_sigreturn(struct pt_regs *);


asmlinkage int sys_set_thread_area(struct user_desc __user *);
asmlinkage int sys_get_thread_area(struct user_desc __user *);


#ifdef CONFIG_X86_32

long sys_iopl(struct pt_regs *);


int sys_clone(struct pt_regs *);
int sys_execve(struct pt_regs *);


asmlinkage int sys_sigsuspend(int, int, old_sigset_t);
asmlinkage int sys_sigaction(int, const struct old_sigaction __user *,
			     struct old_sigaction __user *);
int sys_sigaltstack(struct pt_regs *);
unsigned long sys_sigreturn(struct pt_regs *);


struct mmap_arg_struct;
struct sel_arg_struct;
struct oldold_utsname;
struct old_utsname;

asmlinkage int old_mmap(struct mmap_arg_struct __user *);
asmlinkage int old_select(struct sel_arg_struct __user *);
asmlinkage int sys_ipc(uint, int, int, int, void __user *, long);
asmlinkage int sys_uname(struct old_utsname __user *);
asmlinkage int sys_olduname(struct oldold_utsname __user *);


int sys_vm86old(struct pt_regs *);
int sys_vm86(struct pt_regs *);

#else 



asmlinkage long sys_iopl(unsigned int, struct pt_regs *);


asmlinkage long sys_clone(unsigned long, unsigned long,
			  void __user *, void __user *,
			  struct pt_regs *);
asmlinkage long sys_execve(char __user *, char __user * __user *,
			   char __user * __user *,
			   struct pt_regs *);
long sys_arch_prctl(int, unsigned long);


asmlinkage long sys_sigaltstack(const stack_t __user *, stack_t __user *,
				struct pt_regs *);


struct new_utsname;

asmlinkage long sys_mmap(unsigned long, unsigned long, unsigned long,
			 unsigned long, unsigned long, unsigned long);
asmlinkage long sys_uname(struct new_utsname __user *);

#endif 
#endif 
