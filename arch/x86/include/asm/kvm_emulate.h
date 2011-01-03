

#ifndef _ASM_X86_KVM_X86_EMULATE_H
#define _ASM_X86_KVM_X86_EMULATE_H

struct x86_emulate_ctxt;



#define X86EMUL_CONTINUE        0

#define X86EMUL_UNHANDLEABLE    1

#define X86EMUL_PROPAGATE_FAULT 2 
#define X86EMUL_RETRY_INSTR     2 
#define X86EMUL_CMPXCHG_FAILED  2 
struct x86_emulate_ops {
	
	int (*read_std)(unsigned long addr, void *val,
			unsigned int bytes, struct kvm_vcpu *vcpu);

	
	int (*read_emulated)(unsigned long addr,
			     void *val,
			     unsigned int bytes,
			     struct kvm_vcpu *vcpu);

	
	int (*write_emulated)(unsigned long addr,
			      const void *val,
			      unsigned int bytes,
			      struct kvm_vcpu *vcpu);

	
	int (*cmpxchg_emulated)(unsigned long addr,
				const void *old,
				const void *new,
				unsigned int bytes,
				struct kvm_vcpu *vcpu);

};


struct operand {
	enum { OP_REG, OP_MEM, OP_IMM, OP_NONE } type;
	unsigned int bytes;
	unsigned long val, orig_val, *ptr;
};

struct fetch_cache {
	u8 data[15];
	unsigned long start;
	unsigned long end;
};

struct decode_cache {
	u8 twobyte;
	u8 b;
	u8 lock_prefix;
	u8 rep_prefix;
	u8 op_bytes;
	u8 ad_bytes;
	u8 rex_prefix;
	struct operand src;
	struct operand src2;
	struct operand dst;
	bool has_seg_override;
	u8 seg_override;
	unsigned int d;
	unsigned long regs[NR_VCPU_REGS];
	unsigned long eip, eip_orig;
	
	u8 modrm;
	u8 modrm_mod;
	u8 modrm_reg;
	u8 modrm_rm;
	u8 use_modrm_ea;
	bool rip_relative;
	unsigned long modrm_ea;
	void *modrm_ptr;
	unsigned long modrm_val;
	struct fetch_cache fetch;
};

#define X86_SHADOW_INT_MOV_SS  1
#define X86_SHADOW_INT_STI     2

struct x86_emulate_ctxt {
	
	struct kvm_vcpu *vcpu;

	unsigned long eflags;
	
	int mode;
	u32 cs_base;

	
	int interruptibility;

	
	struct decode_cache decode;
};


#define REPE_PREFIX	1
#define REPNE_PREFIX	2


#define X86EMUL_MODE_REAL     0	
#define X86EMUL_MODE_PROT16   2	
#define X86EMUL_MODE_PROT32   4	
#define X86EMUL_MODE_PROT64   8	


#if defined(CONFIG_X86_32)
#define X86EMUL_MODE_HOST X86EMUL_MODE_PROT32
#elif defined(CONFIG_X86_64)
#define X86EMUL_MODE_HOST X86EMUL_MODE_PROT64
#endif

int x86_decode_insn(struct x86_emulate_ctxt *ctxt,
		    struct x86_emulate_ops *ops);
int x86_emulate_insn(struct x86_emulate_ctxt *ctxt,
		     struct x86_emulate_ops *ops);

#endif 
