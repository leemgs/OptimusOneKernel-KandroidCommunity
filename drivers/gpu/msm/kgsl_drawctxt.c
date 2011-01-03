
#include <linux/string.h>
#include <linux/types.h>
#include <linux/msm_kgsl.h>

#include "yamato_reg.h"
#include "kgsl.h"
#include "kgsl_yamato.h"
#include "kgsl_log.h"
#include "kgsl_pm4types.h"
#include "kgsl_drawctxt.h"
#include "kgsl_cmdstream.h"





#define ALU_CONSTANTS	2048	
#define NUM_REGISTERS	1024	
#ifdef DISABLE_SHADOW_WRITES
#define CMD_BUFFER_LEN  9216	
#else
#define CMD_BUFFER_LEN	3072	
#endif
#define TEX_CONSTANTS		(32*6)	
#define BOOL_CONSTANTS		8	
#define LOOP_CONSTANTS		56	
#define SHADER_INSTRUCT_LOG2	9U	

#if defined(PM4_IM_STORE)

#define SHADER_INSTRUCT		(1<<SHADER_INSTRUCT_LOG2)
#else
#define SHADER_INSTRUCT		0
#endif


#define LCC_SHADOW_SIZE		0x2000	

#define ALU_SHADOW_SIZE		LCC_SHADOW_SIZE	
#define REG_SHADOW_SIZE		0x1000	
#ifdef DISABLE_SHADOW_WRITES
#define CMD_BUFFER_SIZE     0x9000	
#else
#define CMD_BUFFER_SIZE		0x3000	
#endif
#define TEX_SHADOW_SIZE		(TEX_CONSTANTS*4)	
#define SHADER_SHADOW_SIZE      (SHADER_INSTRUCT*12)	

#define REG_OFFSET		LCC_SHADOW_SIZE
#define CMD_OFFSET		(REG_OFFSET + REG_SHADOW_SIZE)
#define TEX_OFFSET		(CMD_OFFSET + CMD_BUFFER_SIZE)
#define	SHADER_OFFSET		((TEX_OFFSET + TEX_SHADOW_SIZE + 32) & ~31)

#define CONTEXT_SIZE		(SHADER_OFFSET + 3 * SHADER_SHADOW_SIZE)


struct tmp_ctx {
	unsigned int *start;	
	unsigned int *cmd;	

	
	uint32_t bool_shadow;	
	uint32_t loop_shadow;	

#if defined(PM4_IM_STORE)
	uint32_t shader_shared;	
	uint32_t shader_vertex;	
	uint32_t shader_pixel;	
#endif

	
	uint32_t reg_values[4];
	uint32_t chicken_restore;

	uint32_t gmem_base;	

};


unsigned int uint2float(unsigned int uintval)
{
	unsigned int exp = 0;
	unsigned int frac = 0;
	unsigned int u = uintval;

	
	if (uintval == 0)
		return 0;
	
	if (u >= 0x10000) {
		exp += 16;
		u >>= 16;
	}
	if (u >= 0x100) {
		exp += 8;
		u >>= 8;
	}
	if (u >= 0x10) {
		exp += 4;
		u >>= 4;
	}
	if (u >= 0x4) {
		exp += 2;
		u >>= 2;
	}
	if (u >= 0x2) {
		exp += 1;
		u >>= 1;
	}

	
	frac = (uintval & (~(1 << exp))) << (23 - exp);

	
	exp = (exp + 127) << 23;

	return exp | frac;
}




#define GMEM2SYS_VTX_PGM_LEN	0x12

static unsigned int gmem2sys_vtx_pgm[GMEM2SYS_VTX_PGM_LEN] = {
	0x00011003, 0x00001000, 0xc2000000,
	0x00001004, 0x00001000, 0xc4000000,
	0x00001005, 0x00002000, 0x00000000,
	0x1cb81000, 0x00398a88, 0x00000003,
	0x140f803e, 0x00000000, 0xe2010100,
	0x14000000, 0x00000000, 0xe2000000
};



#define GMEM2SYS_FRAG_PGM_LEN	0x0c

static unsigned int gmem2sys_frag_pgm[GMEM2SYS_FRAG_PGM_LEN] = {
	0x00000000, 0x1002c400, 0x10000000,
	0x00001003, 0x00002000, 0x00000000,
	0x140f8000, 0x00000000, 0x22000000,
	0x14000000, 0x00000000, 0xe2000000
};




#define SYS2GMEM_VTX_PGM_LEN	0x18

static unsigned int sys2gmem_vtx_pgm[SYS2GMEM_VTX_PGM_LEN] = {
	0x00052003, 0x00001000, 0xc2000000, 0x00001005,
	0x00001000, 0xc4000000, 0x00001006, 0x10071000,
	0x20000000, 0x18981000, 0x0039ba88, 0x00000003,
	0x12982000, 0x40257b08, 0x00000002, 0x140f803e,
	0x00000000, 0xe2010100, 0x140f8000, 0x00000000,
	0xe2020200, 0x14000000, 0x00000000, 0xe2000000
};



#define SYS2GMEM_FRAG_PGM_LEN	0x0f

static unsigned int sys2gmem_frag_pgm[SYS2GMEM_FRAG_PGM_LEN] = {
	0x00011002, 0x00001000, 0xc4000000, 0x00001003,
	0x10041000, 0x20000000, 0x10000001, 0x1ffff688,
	0x00000002, 0x140f8000, 0x00000000, 0xe2000000,
	0x14000000, 0x00000000, 0xe2000000
};


#define SYS2GMEM_TEX_CONST_LEN	6

static unsigned int sys2gmem_tex_const[SYS2GMEM_TEX_CONST_LEN] = {
	
	0x00000002,		

	
	0x00000800,		

	
	0,			

	
	0 << 1 | 1 << 4 | 2 << 7 | 3 << 10 | 2 << 23,

	
	0,

	
	1 << 9			
};


#define QUAD_LEN				12

static unsigned int gmem_copy_quad[QUAD_LEN] = {
	0x00000000, 0x00000000, 0x3f800000,
	0x00000000, 0x00000000, 0x3f800000,
	0x00000000, 0x00000000, 0x3f800000,
	0x00000000, 0x00000000, 0x3f800000
};

#define TEXCOORD_LEN			8

static unsigned int gmem_copy_texcoord[TEXCOORD_LEN] = {
	0x00000000, 0x3f800000,
	0x3f800000, 0x3f800000,
	0x00000000, 0x00000000,
	0x3f800000, 0x00000000
};

#define NUM_COLOR_FORMATS   13

static enum SURFACEFORMAT surface_format_table[NUM_COLOR_FORMATS] = {
	FMT_4_4_4_4,		
	FMT_1_5_5_5,		
	FMT_5_6_5,		
	FMT_8,			
	FMT_8_8,		
	FMT_8_8_8_8,		
	FMT_8_8_8_8,		
	FMT_16_FLOAT,		
	FMT_16_16_FLOAT,	
	FMT_16_16_16_16_FLOAT,	
	FMT_32_FLOAT,		
	FMT_32_32_FLOAT,	
	FMT_32_32_32_32_FLOAT,	
};

static unsigned int format2bytesperpixel[NUM_COLOR_FORMATS] = {
	2,			
	2,			
	2,			
	1,			
	2,			
	4,			
	4,			
	2,			
	4,			
	8,			
	4,			
	8,			
	16,			
};


#define SHADER_CONST_ADDR	(11 * 6 + 3)


#define PM4_REG(reg)		((0x4 << 16) | (GSL_HAL_SUBBLOCK_OFFSET(reg)))


static void config_gmemsize(struct gmem_shadow_t *shadow, int gmem_size)
{
	int w = 64, h = 64;	

	shadow->format = COLORX_8_8_8_8;
	
	gmem_size = (gmem_size + 3) / 4;

	
	while (w * h < gmem_size)
		if (w < h)
			w *= 2;
		else
			h *= 2;

	shadow->width = w;
	shadow->pitch = w;
	shadow->height = h;
	shadow->gmem_pitch = shadow->pitch;

	shadow->size = shadow->pitch * shadow->height * 4;
}

static unsigned int gpuaddr(unsigned int *cmd, struct kgsl_memdesc *memdesc)
{
	return memdesc->gpuaddr + ((char *)cmd - (char *)memdesc->hostptr);
}

static void
create_ib1(struct kgsl_drawctxt *drawctxt, unsigned int *cmd,
	   unsigned int *start, unsigned int *end)
{
	cmd[0] = PM4_HDR_INDIRECT_BUFFER_PFD;
	cmd[1] = gpuaddr(start, &drawctxt->gpustate);
	cmd[2] = end - start;
}

static unsigned int *program_shader(unsigned int *cmds, int vtxfrag,
				    unsigned int *shader_pgm, int dwords)
{
	
	*cmds++ = pm4_type3_packet(PM4_IM_LOAD_IMMEDIATE, 2 + dwords);
	
	*cmds++ = vtxfrag;
	
	*cmds++ = ((0 << 16) | dwords);

	memcpy(cmds, shader_pgm, dwords << 2);
	cmds += dwords;

	return cmds;
}

static unsigned int *reg_to_mem(unsigned int *cmds, uint32_t dst,
				uint32_t src, int dwords)
{
	while (dwords-- > 0) {
		*cmds++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
		*cmds++ = src++;
		*cmds++ = dst;
		dst += 4;
	}

	return cmds;
}

#ifdef DISABLE_SHADOW_WRITES

static void build_reg_to_mem_range(unsigned int start, unsigned int end,
				   unsigned int **cmd,
				   struct kgsl_drawctxt *drawctxt)
{
	unsigned int i = start;

	for (i = start; i <= end; i++) {
		*(*cmd)++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
		*(*cmd)++ = i | (1 << 30);
		*(*cmd)++ =
		    ((drawctxt->gpustate.gpuaddr + REG_OFFSET) & 0xFFFFE000) +
		    (i - 0x2000) * 4;
	}
}

#endif


static unsigned int *build_chicken_restore_cmds(struct kgsl_drawctxt *drawctxt,
						struct tmp_ctx *ctx)
{
	unsigned int *start = ctx->cmd;
	unsigned int *cmds = start;

	*cmds++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmds++ = 0;

	*cmds++ = pm4_type0_packet(REG_TP0_CHICKEN, 1);
	ctx->chicken_restore = gpuaddr(cmds, &drawctxt->gpustate);
	*cmds++ = 0x00000000;

	
	create_ib1(drawctxt, drawctxt->chicken_restore, start, cmds);

	return cmds;
}


static void build_regsave_cmds(struct kgsl_device *device,
			       struct kgsl_drawctxt *drawctxt,
			       struct tmp_ctx *ctx)
{
	unsigned int *start = ctx->cmd;
	unsigned int *cmd = start;
	unsigned int pm_override1;

	kgsl_yamato_regread(device, REG_RBBM_PM_OVERRIDE1, &pm_override1);

	*cmd++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmd++ = 0;

#ifdef DISABLE_SHADOW_WRITES
	
	*cmd++ = pm4_type3_packet(PM4_CONTEXT_UPDATE, 1);
	*cmd++ = 0;
#endif

	
	*cmd++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE1, 1);
	*cmd++ = pm_override1 | (1 << 6);

#ifdef DISABLE_SHADOW_WRITES
	
	build_reg_to_mem_range(REG_RB_SURFACE_INFO, REG_RB_DEPTH_INFO, &cmd,
			       drawctxt);
	build_reg_to_mem_range(REG_COHER_DEST_BASE_0,
			       REG_PA_SC_SCREEN_SCISSOR_BR, &cmd, drawctxt);
	build_reg_to_mem_range(REG_PA_SC_WINDOW_OFFSET,
			       REG_PA_SC_WINDOW_SCISSOR_BR, &cmd, drawctxt);
	build_reg_to_mem_range(REG_VGT_MAX_VTX_INDX, REG_RB_FOG_COLOR, &cmd,
			       drawctxt);
	build_reg_to_mem_range(REG_RB_STENCILREFMASK_BF,
			       REG_PA_CL_VPORT_ZOFFSET, &cmd, drawctxt);
	build_reg_to_mem_range(REG_SQ_PROGRAM_CNTL, REG_SQ_WRAPPING_1, &cmd,
			       drawctxt);
	build_reg_to_mem_range(REG_RB_DEPTHCONTROL, REG_RB_MODECONTROL, &cmd,
			       drawctxt);
	build_reg_to_mem_range(REG_PA_SU_POINT_SIZE, REG_PA_SC_LINE_STIPPLE,
			       &cmd, drawctxt);
	build_reg_to_mem_range(REG_PA_SC_VIZ_QUERY, REG_PA_SC_VIZ_QUERY, &cmd,
			       drawctxt);
	build_reg_to_mem_range(REG_PA_SC_LINE_CNTL, REG_SQ_PS_CONST, &cmd,
			       drawctxt);
	build_reg_to_mem_range(REG_PA_SC_AA_MASK, REG_PA_SC_AA_MASK, &cmd,
			       drawctxt);
	build_reg_to_mem_range(REG_VGT_VERTEX_REUSE_BLOCK_CNTL,
			       REG_RB_DEPTH_CLEAR, &cmd, drawctxt);
	build_reg_to_mem_range(REG_RB_SAMPLE_COUNT_CTL, REG_RB_COLOR_DEST_MASK,
			       &cmd, drawctxt);
	build_reg_to_mem_range(REG_PA_SU_POLY_OFFSET_FRONT_SCALE,
			       REG_PA_SU_POLY_OFFSET_BACK_OFFSET, &cmd,
			       drawctxt);

	
	cmd =
	    reg_to_mem(cmd, (drawctxt->gpustate.gpuaddr) & 0xFFFFE000,
		       REG_SQ_CONSTANT_0, ALU_CONSTANTS);

	
	cmd =
	    reg_to_mem(cmd,
		       (drawctxt->gpustate.gpuaddr + TEX_OFFSET) & 0xFFFFE000,
		       REG_SQ_FETCH_0, TEX_CONSTANTS);
#else

	
	*cmd++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmd++ = 0;

	
	*cmd++ = pm4_type3_packet(PM4_LOAD_CONSTANT_CONTEXT, 3);
	*cmd++ = (drawctxt->gpustate.gpuaddr + REG_OFFSET) & 0xFFFFE000;
	*cmd++ = 4 << 16;	
	*cmd++ = 0x0;		

	
	*cmd++ = pm4_type3_packet(PM4_LOAD_CONSTANT_CONTEXT, 3);
	*cmd++ = drawctxt->gpustate.gpuaddr & 0xFFFFE000;
	*cmd++ = 0 << 16;	
	*cmd++ = 0x0;		

	
	*cmd++ = pm4_type3_packet(PM4_LOAD_CONSTANT_CONTEXT, 3);
	*cmd++ = (drawctxt->gpustate.gpuaddr + TEX_OFFSET) & 0xFFFFE000;
	*cmd++ = 1 << 16;	
	*cmd++ = 0x0;		
#endif

	
	*cmd++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
	*cmd++ = REG_SQ_GPR_MANAGEMENT;
	*cmd++ = ctx->reg_values[0];

	*cmd++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
	*cmd++ = REG_TP0_CHICKEN;
	*cmd++ = ctx->reg_values[1];

	*cmd++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
	*cmd++ = REG_RBBM_PM_OVERRIDE1;
	*cmd++ = ctx->reg_values[2];

	*cmd++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
	*cmd++ = REG_RBBM_PM_OVERRIDE2;
	*cmd++ = ctx->reg_values[3];

	
	cmd = reg_to_mem(cmd, ctx->bool_shadow, REG_SQ_CF_BOOLEANS,
			 BOOL_CONSTANTS);

	
	cmd = reg_to_mem(cmd, ctx->loop_shadow, REG_SQ_CF_LOOP, LOOP_CONSTANTS);

	
	*cmd++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmd++ = 0;
	*cmd++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE1, 1);
	*cmd++ = pm_override1;

	
	create_ib1(drawctxt, drawctxt->reg_save, start, cmd);

	ctx->cmd = cmd;
}


static unsigned int *build_gmem2sys_cmds(struct kgsl_device *device,
					 struct kgsl_drawctxt *drawctxt,
					 struct tmp_ctx *ctx,
					 struct gmem_shadow_t *shadow)
{
	unsigned int *cmds = shadow->gmem_save_commands;
	unsigned int *start = cmds;
	unsigned int pm_override1;
	
	unsigned int bytesperpixel = format2bytesperpixel[shadow->format];
	unsigned int addr =
	    (shadow->gmemshadow.gpuaddr + shadow->offset * bytesperpixel);
	unsigned int offset = (addr - (addr & 0xfffff000)) / bytesperpixel;

	kgsl_yamato_regread(device, REG_RBBM_PM_OVERRIDE1, &pm_override1);

	
	*cmds++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
	*cmds++ = REG_TP0_CHICKEN;
	if (ctx)
		*cmds++ = ctx->chicken_restore;
	else
		cmds++;

	*cmds++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmds++ = 0;

	
	*cmds++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE1, 1);
	*cmds++ = pm_override1 | (1 << 6);

	
	*cmds++ = pm4_type0_packet(REG_TP0_CHICKEN, 1);
	*cmds++ = 0x00000000;

	
	*cmds++ = pm4_type0_packet(REG_PA_SC_AA_CONFIG, 1);
	*cmds++ = 0x00000000;

	

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 4);
	*cmds++ = (0x1 << 16) | SHADER_CONST_ADDR;
	*cmds++ = 0;
	
	*cmds++ = shadow->quad_vertices.gpuaddr | 0x3;
	
	*cmds++ = 0x00000030;

	
	*cmds++ = pm4_type0_packet(REG_TC_CNTL_STATUS, 1);
	*cmds++ = 0x1;

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 4);
	*cmds++ = PM4_REG(REG_VGT_MAX_VTX_INDX);
	*cmds++ = 0x00ffffff;	
	*cmds++ = 0x0;		
	*cmds++ = 0x00000000;	

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_PA_SC_AA_MASK);
	*cmds++ = 0x0000ffff;	

	
	cmds = program_shader(cmds, 0, gmem2sys_vtx_pgm, GMEM2SYS_VTX_PGM_LEN);

	
	cmds =
	    program_shader(cmds, 1, gmem2sys_frag_pgm, GMEM2SYS_FRAG_PGM_LEN);

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_SQ_PROGRAM_CNTL);
	*cmds++ = 0x10010001;
	*cmds++ = 0x00000008;

	

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_PA_CL_VTE_CNTL);
	
	*cmds++ = 0x00000b00;

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_RB_SURFACE_INFO);
	*cmds++ = shadow->gmem_pitch;	

	
	
	if (ctx) {
		BUG_ON(ctx->gmem_base & 0xFFF);
		*cmds++ =
		    (shadow->
		     format << RB_COLOR_INFO__COLOR_FORMAT__SHIFT) | ctx->
		    gmem_base;
	} else {
		unsigned int temp = *cmds;
		*cmds++ = (temp & ~RB_COLOR_INFO__COLOR_FORMAT_MASK) |
			(shadow->format << RB_COLOR_INFO__COLOR_FORMAT__SHIFT);
	}

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_RB_DEPTHCONTROL);
	*cmds++ = 0;

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_PA_SU_SC_MODE_CNTL);
	*cmds++ = 0x00080240;

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_PA_SC_SCREEN_SCISSOR_TL);
	*cmds++ = (0 << 16) | 0;
	*cmds++ = (0x1fff << 16) | (0x1fff);
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_PA_SC_WINDOW_SCISSOR_TL);
	*cmds++ = (unsigned int)((1U << 31) | (0 << 16) | 0);
	*cmds++ = (0x1fff << 16) | (0x1fff);

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_PA_CL_VPORT_ZSCALE);
	*cmds++ = 0xbf800000;	
	*cmds++ = 0x0;

	

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 6);
	*cmds++ = PM4_REG(REG_RB_COPY_CONTROL);
	*cmds++ = 0;		
	*cmds++ = addr & 0xfffff000;	
	*cmds++ = shadow->pitch >> 5;	

	
	*cmds++ = 0x0003c008 |
	    (shadow->format << RB_COPY_DEST_INFO__COPY_DEST_FORMAT__SHIFT);
	
	BUG_ON(offset & 0xfffff000);
	*cmds++ = offset;

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_RB_MODECONTROL);
	*cmds++ = 0x6;		

	
	*cmds++ = pm4_type3_packet(PM4_DRAW_INDX, 2);
	*cmds++ = 0;		
	
	*cmds++ = 0x00030088;

	
	*cmds++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmds++ = 0;
	*cmds++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE1, 1);
	*cmds++ = pm_override1;
	
	create_ib1(drawctxt, shadow->gmem_save, start, cmds);

	return cmds;
}




static unsigned int *build_sys2gmem_cmds(struct kgsl_device *device,
					 struct kgsl_drawctxt *drawctxt,
					 struct tmp_ctx *ctx,
					 struct gmem_shadow_t *shadow)
{
	unsigned int *cmds = shadow->gmem_restore_commands;
	unsigned int *start = cmds;
	unsigned int pm_override1;

	kgsl_yamato_regread(device, REG_RBBM_PM_OVERRIDE1, &pm_override1);

	
	*cmds++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
	*cmds++ = REG_TP0_CHICKEN;
	if (ctx)
		*cmds++ = ctx->chicken_restore;
	else
		cmds++;

	*cmds++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmds++ = 0;

	
	*cmds++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE1, 1);
	*cmds++ = pm_override1 | (1 << 6);

	
	*cmds++ = pm4_type0_packet(REG_TP0_CHICKEN, 1);
	*cmds++ = 0x00000000;

	
	*cmds++ = pm4_type0_packet(REG_PA_SC_AA_CONFIG, 1);
	*cmds++ = 0x00000000;
	

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 7);

	*cmds++ = (0x1 << 16) | (9 * 6);
	
	*cmds++ = shadow->quad_vertices.gpuaddr | 0x3;
	
	*cmds++ = 0x00000030;
	
	*cmds++ = shadow->quad_texcoords.gpuaddr | 0x3;
	
	*cmds++ = 0x00000020;
	*cmds++ = 0;
	*cmds++ = 0;

	
	*cmds++ = pm4_type0_packet(REG_TC_CNTL_STATUS, 1);
	*cmds++ = 0x1;

	cmds = program_shader(cmds, 0, sys2gmem_vtx_pgm, SYS2GMEM_VTX_PGM_LEN);

	
	cmds =
	    program_shader(cmds, 1, sys2gmem_frag_pgm, SYS2GMEM_FRAG_PGM_LEN);

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_SQ_PROGRAM_CNTL);
	*cmds++ = 0x10030002;
	*cmds++ = 0x00000008;

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_PA_SC_AA_MASK);
	*cmds++ = 0x0000ffff;	

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_PA_SC_VIZ_QUERY);
	*cmds++ = 0x0;		

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_RB_COLORCONTROL);
	*cmds++ = 0x00000c20;

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 4);
	*cmds++ = PM4_REG(REG_VGT_MAX_VTX_INDX);
	*cmds++ = 0x00ffffff;	
	*cmds++ = 0x0;		
	*cmds++ = 0x00000000;	

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_VGT_VERTEX_REUSE_BLOCK_CNTL);
	*cmds++ = 0x00000002;	
	*cmds++ = 0x00000002;	

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_SQ_INTERPOLATOR_CNTL);
	
	*cmds++ = 0xffffffff;	

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_PA_SC_AA_CONFIG);
	*cmds++ = 0x00000000;	

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_PA_SU_SC_MODE_CNTL);
	*cmds++ = 0x00080240;

	
	*cmds++ =
	    pm4_type3_packet(PM4_SET_CONSTANT, (SYS2GMEM_TEX_CONST_LEN + 1));
	*cmds++ = (0x1 << 16) | (0 * 6);
	memcpy(cmds, sys2gmem_tex_const, SYS2GMEM_TEX_CONST_LEN << 2);
	cmds[0] |= (shadow->pitch >> 5) << 22;
	cmds[1] |=
	    shadow->gmemshadow.gpuaddr | surface_format_table[shadow->format];
	cmds[2] |=
	    (shadow->width + shadow->offset_x - 1) | (shadow->height +
						      shadow->offset_y -
						      1) << 13;
	cmds += SYS2GMEM_TEX_CONST_LEN;

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_RB_SURFACE_INFO);
	*cmds++ = shadow->gmem_pitch;	

	
	if (ctx)
		*cmds++ =
		    (shadow->
		     format << RB_COLOR_INFO__COLOR_FORMAT__SHIFT) | ctx->
		    gmem_base;
	else {
		unsigned int temp = *cmds;
		*cmds++ = (temp & ~RB_COLOR_INFO__COLOR_FORMAT_MASK) |
			(shadow->format << RB_COLOR_INFO__COLOR_FORMAT__SHIFT);
	}

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_RB_DEPTHCONTROL);
	*cmds++ = 0;		

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_PA_SC_SCREEN_SCISSOR_TL);
	*cmds++ = (0 << 16) | 0;
	*cmds++ = ((0x1fff) << 16) | 0x1fff;
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_PA_SC_WINDOW_SCISSOR_TL);
	*cmds++ = (unsigned int)((1U << 31) | (0 << 16) | 0);
	*cmds++ = ((0x1fff) << 16) | 0x1fff;

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_PA_CL_VTE_CNTL);
	
	*cmds++ = 0x00000b00;

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_PA_CL_VPORT_ZSCALE);
	*cmds++ = 0xbf800000;
	*cmds++ = 0x0;

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_RB_COLOR_MASK);
	*cmds++ = 0x0000000f;	

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_RB_COLOR_DEST_MASK);
	*cmds++ = 0xffffffff;

	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 3);
	*cmds++ = PM4_REG(REG_SQ_WRAPPING_0);
	*cmds++ = 0x00000000;
	*cmds++ = 0x00000000;

	
	*cmds++ = pm4_type3_packet(PM4_SET_CONSTANT, 2);
	*cmds++ = PM4_REG(REG_RB_MODECONTROL);
	
	*cmds++ = 0x4;

	
	*cmds++ = pm4_type3_packet(PM4_DRAW_INDX, 2);
	*cmds++ = 0;		
	
	*cmds++ = 0x00030088;

	
	*cmds++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmds++ = 0;
	*cmds++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE1, 1);
	*cmds++ = pm_override1;

	
	create_ib1(drawctxt, shadow->gmem_restore, start, cmds);

	return cmds;
}


static unsigned *reg_range(unsigned int *cmd, unsigned int start,
			   unsigned int end)
{
	*cmd++ = PM4_REG(start);	
	*cmd++ = end - start + 1;	
	return cmd;
}

static void build_regrestore_cmds(struct kgsl_device *device,
				  struct kgsl_drawctxt *drawctxt,
				  struct tmp_ctx *ctx)
{
	unsigned int *start = ctx->cmd;
	unsigned int *cmd = start;
	unsigned int pm_override1;

	kgsl_yamato_regread(device, REG_RBBM_PM_OVERRIDE1, &pm_override1);

	*cmd++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmd++ = 0;

	
	*cmd++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE1, 1);
	*cmd++ = pm_override1 | (1 << 6);

	
	
	cmd++;
#ifdef DISABLE_SHADOW_WRITES
	
	*cmd++ = ((drawctxt->gpustate.gpuaddr + REG_OFFSET) & 0xFFFFE000) | 1;
#else
	*cmd++ = (drawctxt->gpustate.gpuaddr + REG_OFFSET) & 0xFFFFE000;
#endif

	cmd = reg_range(cmd, REG_RB_SURFACE_INFO, REG_PA_SC_SCREEN_SCISSOR_BR);
	cmd = reg_range(cmd, REG_PA_SC_WINDOW_OFFSET,
			REG_PA_SC_WINDOW_SCISSOR_BR);
	cmd = reg_range(cmd, REG_VGT_MAX_VTX_INDX, REG_PA_CL_VPORT_ZOFFSET);
	cmd = reg_range(cmd, REG_SQ_PROGRAM_CNTL, REG_SQ_WRAPPING_1);
	cmd = reg_range(cmd, REG_RB_DEPTHCONTROL, REG_RB_MODECONTROL);
	cmd = reg_range(cmd, REG_PA_SU_POINT_SIZE,
			REG_PA_SC_VIZ_QUERY); 
	cmd = reg_range(cmd, REG_PA_SC_LINE_CNTL, REG_RB_COLOR_DEST_MASK);
	cmd = reg_range(cmd, REG_PA_SU_POLY_OFFSET_FRONT_SCALE,
			REG_PA_SU_POLY_OFFSET_BACK_OFFSET);

	
	start[4] =
	    pm4_type3_packet(PM4_LOAD_CONSTANT_CONTEXT, (cmd - start) - 5);
	
#ifdef DISABLE_SHADOW_WRITES
	start[6] |= (0 << 24) | (4 << 16);	
#else
	start[6] |= (1 << 24) | (4 << 16);
#endif

	
	*cmd++ = pm4_type0_packet(REG_SQ_GPR_MANAGEMENT, 1);
	ctx->reg_values[0] = gpuaddr(cmd, &drawctxt->gpustate);
	*cmd++ = 0x00040400;

	*cmd++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmd++ = 0;
	*cmd++ = pm4_type0_packet(REG_TP0_CHICKEN, 1);
	ctx->reg_values[1] = gpuaddr(cmd, &drawctxt->gpustate);
	*cmd++ = 0x00000000;

	*cmd++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE1, 1);
	ctx->reg_values[2] = gpuaddr(cmd, &drawctxt->gpustate);
	*cmd++ = 0x00000000;

	*cmd++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE2, 1);
	ctx->reg_values[3] = gpuaddr(cmd, &drawctxt->gpustate);
	*cmd++ = 0x00000000;

	
	*cmd++ = pm4_type3_packet(PM4_LOAD_CONSTANT_CONTEXT, 3);
	*cmd++ = drawctxt->gpustate.gpuaddr & 0xFFFFE000;
#ifdef DISABLE_SHADOW_WRITES
	*cmd++ = (0 << 24) | (0 << 16) | 0;	
#else
	*cmd++ = (1 << 24) | (0 << 16) | 0;
#endif
	*cmd++ = ALU_CONSTANTS;

	
	*cmd++ = pm4_type3_packet(PM4_LOAD_CONSTANT_CONTEXT, 3);
	*cmd++ = (drawctxt->gpustate.gpuaddr + TEX_OFFSET) & 0xFFFFE000;
#ifdef DISABLE_SHADOW_WRITES
	
	*cmd++ = (0 << 24) | (1 << 16) | 0;
#else
	*cmd++ = (1 << 24) | (1 << 16) | 0;
#endif
	*cmd++ = TEX_CONSTANTS;

	
	*cmd++ = pm4_type3_packet(PM4_SET_CONSTANT, 1 + BOOL_CONSTANTS);
	*cmd++ = (2 << 16) | 0;

	
	ctx->bool_shadow = gpuaddr(cmd, &drawctxt->gpustate);
	cmd += BOOL_CONSTANTS;

	
	*cmd++ = pm4_type3_packet(PM4_SET_CONSTANT, 1 + LOOP_CONSTANTS);
	*cmd++ = (3 << 16) | 0;

	
	ctx->loop_shadow = gpuaddr(cmd, &drawctxt->gpustate);
	cmd += LOOP_CONSTANTS;

	
	*cmd++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmd++ = 0;
	*cmd++ = pm4_type0_packet(REG_RBBM_PM_OVERRIDE1, 1);
	*cmd++ = pm_override1;

	
	create_ib1(drawctxt, drawctxt->reg_restore, start, cmd);

	ctx->cmd = cmd;
}


static void set_gmem_copy_quad(struct gmem_shadow_t *shadow)
{
	
	gmem_copy_quad[1] = uint2float(shadow->height + shadow->gmem_offset_y);
	gmem_copy_quad[3] = uint2float(shadow->width + shadow->gmem_offset_x);
	gmem_copy_quad[4] = uint2float(shadow->height + shadow->gmem_offset_y);
	gmem_copy_quad[9] = uint2float(shadow->width + shadow->gmem_offset_x);

	gmem_copy_quad[0] = uint2float(shadow->gmem_offset_x);
	gmem_copy_quad[6] = uint2float(shadow->gmem_offset_x);
	gmem_copy_quad[7] = uint2float(shadow->gmem_offset_y);
	gmem_copy_quad[10] = uint2float(shadow->gmem_offset_y);

	BUG_ON(shadow->offset_x);
	BUG_ON(shadow->offset_y);

	memcpy(shadow->quad_vertices.hostptr, gmem_copy_quad, QUAD_LEN << 2);

	memcpy(shadow->quad_texcoords.hostptr, gmem_copy_texcoord,
	       TEXCOORD_LEN << 2);
}


static void build_quad_vtxbuff(struct kgsl_drawctxt *drawctxt,
		       struct tmp_ctx *ctx, struct gmem_shadow_t *shadow)
{
	unsigned int *cmd = ctx->cmd;

	
	shadow->quad_vertices.hostptr = cmd;
	shadow->quad_vertices.gpuaddr = gpuaddr(cmd, &drawctxt->gpustate);

	cmd += QUAD_LEN;

	
	shadow->quad_texcoords.hostptr = cmd;
	shadow->quad_texcoords.gpuaddr = gpuaddr(cmd, &drawctxt->gpustate);

	cmd += TEXCOORD_LEN;

	set_gmem_copy_quad(shadow);

	ctx->cmd = cmd;
}

static void
build_shader_save_restore_cmds(struct kgsl_drawctxt *drawctxt,
			       struct tmp_ctx *ctx)
{
	unsigned int *cmd = ctx->cmd;
	unsigned int *save, *restore, *fixup;
#if defined(PM4_IM_STORE)
	unsigned int *startSizeVtx, *startSizePix, *startSizeShared;
#endif
	unsigned int *partition1;
	unsigned int *shaderBases, *partition2;

#if defined(PM4_IM_STORE)
	
	ctx->shader_vertex = drawctxt->gpustate.gpuaddr + SHADER_OFFSET;
	ctx->shader_pixel = ctx->shader_vertex + SHADER_SHADOW_SIZE;
	ctx->shader_shared = ctx->shader_pixel + SHADER_SHADOW_SIZE;
#endif

	

	restore = cmd;		

	
	*cmd++ = pm4_type3_packet(PM4_INVALIDATE_STATE, 1);
	*cmd++ = 0x00000300;	

	
	*cmd++ = pm4_type3_packet(PM4_SET_SHADER_BASES, 1);
	shaderBases = cmd++;	

	
	*cmd++ = pm4_type0_packet(REG_SQ_INST_STORE_MANAGMENT, 1);
	partition1 = cmd++;	

#if defined(PM4_IM_STORE)
	
	*cmd++ = pm4_type3_packet(PM4_IM_LOAD, 2);
	*cmd++ = ctx->shader_vertex + 0x0;	
	startSizeVtx = cmd++;	

	
	*cmd++ = pm4_type3_packet(PM4_IM_LOAD, 2);
	*cmd++ = ctx->shader_pixel + 0x1;	
	startSizePix = cmd++;	

	
	*cmd++ = pm4_type3_packet(PM4_IM_LOAD, 2);
	*cmd++ = ctx->shader_shared + 0x2;	
	startSizeShared = cmd++;	
#endif

	
	create_ib1(drawctxt, drawctxt->shader_restore, restore, cmd);

	

	fixup = cmd;		

	
	*cmd++ = pm4_type0_packet(REG_SCRATCH_REG2, 1);
	partition2 = cmd++;	

	
	*cmd++ = pm4_type3_packet(PM4_REG_RMW, 3);
	*cmd++ = REG_SCRATCH_REG2;
	
	*cmd++ = 0x0FFF0FFF;
	
	*cmd++ = (unsigned int)((SHADER_INSTRUCT_LOG2 - 5U) << 29);

	
	*cmd++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
	*cmd++ = REG_SCRATCH_REG2;
	
	*cmd++ = gpuaddr(shaderBases, &drawctxt->gpustate);

	
	create_ib1(drawctxt, drawctxt->shader_fixup, fixup, cmd);

	

	save = cmd;		

	*cmd++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmd++ = 0;

	
	*cmd++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
	*cmd++ = REG_SQ_INST_STORE_MANAGMENT;
	
	*cmd++ = gpuaddr(partition1, &drawctxt->gpustate);
	*cmd++ = pm4_type3_packet(PM4_REG_TO_MEM, 2);
	*cmd++ = REG_SQ_INST_STORE_MANAGMENT;
	
	*cmd++ = gpuaddr(partition2, &drawctxt->gpustate);

#if defined(PM4_IM_STORE)

	
	*cmd++ = pm4_type3_packet(PM4_IM_STORE, 2);
	*cmd++ = ctx->shader_vertex + 0x0;	
	
	*cmd++ = gpuaddr(startSizeVtx, &drawctxt->gpustate);

	
	*cmd++ = pm4_type3_packet(PM4_IM_STORE, 2);
	*cmd++ = ctx->shader_pixel + 0x1;	
	
	*cmd++ = gpuaddr(startSizePix, &drawctxt->gpustate);

	

	*cmd++ = pm4_type3_packet(PM4_IM_STORE, 2);
	*cmd++ = ctx->shader_shared + 0x2;	
	
	*cmd++ = gpuaddr(startSizeShared, &drawctxt->gpustate);

#endif

	*cmd++ = pm4_type3_packet(PM4_WAIT_FOR_IDLE, 1);
	*cmd++ = 0;

	
	create_ib1(drawctxt, drawctxt->shader_save, save, cmd);

	ctx->cmd = cmd;
}


static int
create_gpustate_shadow(struct kgsl_device *device,
		       struct kgsl_drawctxt *drawctxt, struct tmp_ctx *ctx)
{
	uint32_t flags;

	flags = (KGSL_MEMFLAGS_CONPHYS | KGSL_MEMFLAGS_ALIGN8K);

	
	if (kgsl_sharedmem_alloc(flags, CONTEXT_SIZE, &drawctxt->gpustate) != 0)
		return -ENOMEM;
	if (kgsl_mmu_map(drawctxt->pagetable, drawctxt->gpustate.physaddr,
			 drawctxt->gpustate.size,
			 GSL_PT_PAGE_RV | GSL_PT_PAGE_WV,
			 &drawctxt->gpustate.gpuaddr,
			 KGSL_MEMFLAGS_CONPHYS | KGSL_MEMFLAGS_ALIGN8K)) {
		kgsl_sharedmem_free(&drawctxt->gpustate);
		return -EINVAL;
	}

	drawctxt->flags |= CTXT_FLAGS_STATE_SHADOW;

	
	kgsl_sharedmem_set(&drawctxt->gpustate, 0, 0, CONTEXT_SIZE);

	
	ctx->cmd = ctx->start
	    = (unsigned int *)((char *)drawctxt->gpustate.hostptr + CMD_OFFSET);

	
	kgsl_yamato_idle(device, KGSL_TIMEOUT_DEFAULT);
	build_regrestore_cmds(device, drawctxt, ctx);
	build_regsave_cmds(device, drawctxt, ctx);

	build_shader_save_restore_cmds(drawctxt, ctx);

	return 0;
}


static int
create_gmem_shadow(struct kgsl_yamato_device *yamato_device,
		   struct kgsl_drawctxt *drawctxt,
		   struct tmp_ctx *ctx)
{
	unsigned int flags = KGSL_MEMFLAGS_CONPHYS | KGSL_MEMFLAGS_ALIGN8K, i;
	struct kgsl_device *device = &yamato_device->dev;

	config_gmemsize(&drawctxt->context_gmem_shadow,
			yamato_device->gmemspace.sizebytes);
	ctx->gmem_base = yamato_device->gmemspace.gpu_base;

	
	if (kgsl_sharedmem_alloc(flags, drawctxt->context_gmem_shadow.size,
				 &drawctxt->context_gmem_shadow.gmemshadow) !=
	    0)
		return -ENOMEM;
	if (kgsl_mmu_map(drawctxt->pagetable,
			 drawctxt->context_gmem_shadow.gmemshadow.physaddr,
			 drawctxt->context_gmem_shadow.gmemshadow.size,
			 GSL_PT_PAGE_RV | GSL_PT_PAGE_WV,
			 &drawctxt->context_gmem_shadow.gmemshadow.gpuaddr,
			 KGSL_MEMFLAGS_CONPHYS | KGSL_MEMFLAGS_ALIGN8K)) {
		kgsl_sharedmem_free(&drawctxt->context_gmem_shadow.
					gmemshadow);
		return -EINVAL;
	}

	
	drawctxt->flags |= CTXT_FLAGS_GMEM_SHADOW | CTXT_FLAGS_GMEM_SAVE;

	
	kgsl_sharedmem_set(&drawctxt->context_gmem_shadow.gmemshadow, 0, 0,
			   drawctxt->context_gmem_shadow.size);

	
	build_quad_vtxbuff(drawctxt, ctx, &drawctxt->context_gmem_shadow);

	
	ctx->cmd = build_chicken_restore_cmds(drawctxt, ctx);

	
	
	kgsl_yamato_idle(device, KGSL_TIMEOUT_DEFAULT);
	drawctxt->context_gmem_shadow.gmem_save_commands = ctx->cmd;
	ctx->cmd =
	    build_gmem2sys_cmds(device, drawctxt, ctx,
				&drawctxt->context_gmem_shadow);
	drawctxt->context_gmem_shadow.gmem_restore_commands = ctx->cmd;
	ctx->cmd =
	    build_sys2gmem_cmds(device, drawctxt, ctx,
				&drawctxt->context_gmem_shadow);

	for (i = 0; i < KGSL_MAX_GMEM_SHADOW_BUFFERS; i++) {
		build_quad_vtxbuff(drawctxt, ctx,
				   &drawctxt->user_gmem_shadow[i]);

		drawctxt->user_gmem_shadow[i].gmem_save_commands = ctx->cmd;
		ctx->cmd =
		    build_gmem2sys_cmds(device, drawctxt, ctx,
					&drawctxt->user_gmem_shadow[i]);

		drawctxt->user_gmem_shadow[i].gmem_restore_commands = ctx->cmd;
		ctx->cmd =
		    build_sys2gmem_cmds(device, drawctxt, ctx,
					&drawctxt->user_gmem_shadow[i]);
	}

	return 0;
}



int kgsl_drawctxt_init(struct kgsl_device *device)
{
	return 0;
}


int kgsl_drawctxt_close(struct kgsl_device *device)
{
	return 0;
}



int
kgsl_drawctxt_create(struct kgsl_device_private *dev_priv,
		     uint32_t flags, unsigned int *drawctxt_id)
{
	struct kgsl_drawctxt *drawctxt;
	struct kgsl_device *device = dev_priv->device;
	struct kgsl_yamato_device *yamato_device = (struct kgsl_yamato_device *)
							device;
	struct kgsl_pagetable *pagetable = dev_priv->process_priv->pagetable;
	int index;
	struct tmp_ctx ctx;

	KGSL_CTXT_INFO("pt %p flags %08x\n", pagetable, flags);
	if (yamato_device->drawctxt_count >= KGSL_CONTEXT_MAX)
		return -EINVAL;

	
	index = 0;
	while (index < KGSL_CONTEXT_MAX) {
		if (yamato_device->drawctxt[index].flags ==
							CTXT_FLAGS_NOT_IN_USE)
			break;

		index++;
	}

	if (index >= KGSL_CONTEXT_MAX)
		return -EINVAL;

	drawctxt = &yamato_device->drawctxt[index];
	drawctxt->pagetable = pagetable;
	drawctxt->flags = CTXT_FLAGS_IN_USE;
	drawctxt->bin_base_offset = 0;

	yamato_device->drawctxt_count++;

	if (create_gpustate_shadow(device, drawctxt, &ctx)
			!= 0) {
		kgsl_drawctxt_destroy(device, index);
		return -EINVAL;
	}

	
	drawctxt->flags |= CTXT_FLAGS_SHADER_SAVE;

	if (!(flags & KGSL_CONTEXT_NO_GMEM_ALLOC)) {
		
		memset(drawctxt->user_gmem_shadow, 0,
		       sizeof(struct gmem_shadow_t) *
				KGSL_MAX_GMEM_SHADOW_BUFFERS);

		if (create_gmem_shadow(yamato_device, drawctxt, &ctx)
				!= 0) {
			kgsl_drawctxt_destroy(device, index);
			return -EINVAL;
		}
	}

	BUG_ON(ctx.cmd - ctx.start > CMD_BUFFER_LEN);

	*drawctxt_id = index;

	KGSL_CTXT_INFO("return drawctxt_id %d\n", *drawctxt_id);
	return 0;
}



int kgsl_drawctxt_destroy(struct kgsl_device *device, unsigned int drawctxt_id)
{
	struct kgsl_drawctxt *drawctxt;
	struct kgsl_yamato_device *yamato_device = (struct kgsl_yamato_device *)
							device;

	if (drawctxt_id >= KGSL_CONTEXT_MAX)
		return -EINVAL;

	drawctxt = &yamato_device->drawctxt[drawctxt_id];

	KGSL_CTXT_INFO("drawctxt_id %d ptr %p\n", drawctxt_id, drawctxt);
	if (drawctxt->flags != CTXT_FLAGS_NOT_IN_USE) {
		
		if (yamato_device->drawctxt_active == drawctxt) {
			
			drawctxt->flags &= ~(CTXT_FLAGS_GMEM_SAVE |
					     CTXT_FLAGS_SHADER_SAVE |
					     CTXT_FLAGS_GMEM_SHADOW |
					     CTXT_FLAGS_STATE_SHADOW);

			kgsl_drawctxt_switch(yamato_device, NULL, 0);
		}

		kgsl_yamato_idle(device, KGSL_TIMEOUT_DEFAULT);

		
		if (drawctxt->gpustate.gpuaddr != 0) {
			kgsl_mmu_unmap(drawctxt->pagetable,
				       drawctxt->gpustate.gpuaddr,
				       drawctxt->gpustate.size);
			drawctxt->gpustate.gpuaddr = 0;
		}
		if (drawctxt->gpustate.physaddr != 0)
			kgsl_sharedmem_free(&drawctxt->gpustate);

		
		if (drawctxt->context_gmem_shadow.gmemshadow.gpuaddr != 0) {
			kgsl_mmu_unmap(drawctxt->pagetable,
			drawctxt->context_gmem_shadow.gmemshadow.gpuaddr,
			drawctxt->context_gmem_shadow.gmemshadow.size);
			drawctxt->context_gmem_shadow.gmemshadow.gpuaddr = 0;
		}

		if (drawctxt->context_gmem_shadow.gmemshadow.physaddr != 0)
			kgsl_sharedmem_free(&drawctxt->context_gmem_shadow.
					    gmemshadow);

		drawctxt->flags = CTXT_FLAGS_NOT_IN_USE;

		BUG_ON(yamato_device->drawctxt_count == 0);
		yamato_device->drawctxt_count--;
	}
	KGSL_CTXT_INFO("return\n");
	return 0;
}


int kgsl_drawctxt_bind_gmem_shadow(struct kgsl_yamato_device *yamato_device,
		unsigned int drawctxt_id,
		const struct kgsl_gmem_desc *gmem_desc,
		unsigned int shadow_x,
		unsigned int shadow_y,
		const struct kgsl_buffer_desc
		*shadow_buffer, unsigned int buffer_id)
{
	struct kgsl_drawctxt *drawctxt;
	struct kgsl_device *device = &yamato_device->dev;

    
    struct gmem_shadow_t *shadow;
	unsigned int i;

	if (device->flags & KGSL_FLAGS_SAFEMODE)
		
		return 0;

	drawctxt = &yamato_device->drawctxt[drawctxt_id];

	shadow = &drawctxt->user_gmem_shadow[buffer_id];

	if (!shadow_buffer->enabled) {
		
		KGSL_MEM_ERR("shadow is disabled in bind_gmem\n");
		shadow->gmemshadow.size = 0;
	} else {
		
		unsigned int width, height;

		BUG_ON(gmem_desc->x % 2); 
		BUG_ON(gmem_desc->y % 2);  
		BUG_ON(gmem_desc->width % 2); 
		
		BUG_ON(gmem_desc->height % 2);
		
		BUG_ON(gmem_desc->pitch % 32);

		BUG_ON(shadow_x % 2);  
		BUG_ON(shadow_y % 2);  

		BUG_ON(shadow_buffer->format < COLORX_4_4_4_4);
		BUG_ON(shadow_buffer->format > COLORX_32_32_32_32_FLOAT);
		
		BUG_ON(shadow_buffer->pitch % 32);

		BUG_ON(buffer_id < 0);
		BUG_ON(buffer_id > KGSL_MAX_GMEM_SHADOW_BUFFERS);

		width = gmem_desc->width;
		height = gmem_desc->height;

		shadow->width = width;
		shadow->format = shadow_buffer->format;

		shadow->height = height;
		shadow->pitch = shadow_buffer->pitch;

		memset(&shadow->gmemshadow, 0, sizeof(struct kgsl_memdesc));
		shadow->gmemshadow.hostptr = shadow_buffer->hostptr;
		shadow->gmemshadow.gpuaddr = shadow_buffer->gpuaddr;
		shadow->gmemshadow.physaddr = shadow->gmemshadow.gpuaddr;
		shadow->gmemshadow.size = shadow_buffer->size;

		
		shadow->offset =
		    (int)(shadow_buffer->pitch) * ((int)shadow_y -
						   (int)gmem_desc->y) +
		    (int)shadow_x - (int)gmem_desc->x;

		shadow->offset_x = shadow_x;
		shadow->offset_y = shadow_y;
		shadow->gmem_offset_x = gmem_desc->x;
		shadow->gmem_offset_y = gmem_desc->y;

		shadow->size = shadow->gmemshadow.size;

		shadow->gmem_pitch = gmem_desc->pitch;

		
		set_gmem_copy_quad(shadow);

		
		kgsl_yamato_idle(device, KGSL_TIMEOUT_DEFAULT);

		
		build_gmem2sys_cmds(device, drawctxt, NULL, shadow);
		build_sys2gmem_cmds(device, drawctxt, NULL, shadow);

		
		if (drawctxt->context_gmem_shadow.gmemshadow.physaddr != 0) {
			kgsl_sharedmem_free(&drawctxt->context_gmem_shadow.
					    gmemshadow);
			drawctxt->context_gmem_shadow.gmemshadow.physaddr = 0;
		}
	}

	
	drawctxt->flags &= ~CTXT_FLAGS_GMEM_SHADOW;
	for (i = 0; i < KGSL_MAX_GMEM_SHADOW_BUFFERS; i++) {
		if (drawctxt->user_gmem_shadow[i].gmemshadow.size > 0)
			drawctxt->flags |= CTXT_FLAGS_GMEM_SHADOW;
	}

	return 0;
}


int kgsl_drawctxt_set_bin_base_offset(struct kgsl_device *device,
					unsigned int drawctxt_id,
					unsigned int offset)
{
	struct kgsl_drawctxt *drawctxt;
	struct kgsl_yamato_device *yamato_device = (struct kgsl_yamato_device *)
								device;

	drawctxt = &yamato_device->drawctxt[drawctxt_id];

	drawctxt->bin_base_offset = offset;

	return 0;
}


void
kgsl_drawctxt_switch(struct kgsl_yamato_device *yamato_device,
			struct kgsl_drawctxt *drawctxt,
			unsigned int flags)
{
	struct kgsl_drawctxt *active_ctxt = yamato_device->drawctxt_active;
	struct kgsl_device *device = &yamato_device->dev;
	unsigned int cmds[2];

	if (drawctxt) {
		if (flags & KGSL_CONTEXT_SAVE_GMEM)
			
			drawctxt->flags |= CTXT_FLAGS_GMEM_SAVE;
		else
			
			drawctxt->flags &= ~CTXT_FLAGS_GMEM_SAVE;
	}
	
	if (active_ctxt == drawctxt)
		return;

	KGSL_CTXT_INFO("from %p to %p flags %d\n",
			yamato_device->drawctxt_active, drawctxt, flags);
	
	if (active_ctxt != NULL) {
		KGSL_CTXT_INFO("active_ctxt flags %08x\n", active_ctxt->flags);
		
		KGSL_CTXT_DBG("save regs");
		kgsl_ringbuffer_issuecmds(device, 0, active_ctxt->reg_save, 3);

		if (active_ctxt->flags & CTXT_FLAGS_SHADER_SAVE) {
			
			KGSL_CTXT_DBG("save shader");
			kgsl_ringbuffer_issuecmds(device, KGSL_CMD_FLAGS_PMODE,
						  active_ctxt->shader_save, 3);

			
			KGSL_CTXT_DBG("save shader fixup");
			kgsl_ringbuffer_issuecmds(device, 0,
					active_ctxt->shader_fixup, 3);

			active_ctxt->flags |= CTXT_FLAGS_SHADER_RESTORE;
		}

		if (active_ctxt->flags & CTXT_FLAGS_GMEM_SAVE
			&& active_ctxt->flags & CTXT_FLAGS_GMEM_SHADOW) {
			
			unsigned int i, numbuffers = 0;
			KGSL_CTXT_DBG("save gmem");
			for (i = 0; i < KGSL_MAX_GMEM_SHADOW_BUFFERS; i++) {
				if (active_ctxt->user_gmem_shadow[i].gmemshadow.
				    size > 0) {
					kgsl_ringbuffer_issuecmds(device,
						KGSL_CMD_FLAGS_PMODE,
					  active_ctxt->user_gmem_shadow[i].
						gmem_save, 3);

					
					kgsl_ringbuffer_issuecmds(device, 0,
					  active_ctxt->chicken_restore, 3);

					numbuffers++;
				}
			}
			if (numbuffers == 0) {
				kgsl_ringbuffer_issuecmds(device,
				    KGSL_CMD_FLAGS_PMODE,
				    active_ctxt->context_gmem_shadow.gmem_save,
				    3);

				
				kgsl_ringbuffer_issuecmds(device, 0,
					 active_ctxt->chicken_restore, 3);
			}

			active_ctxt->flags |= CTXT_FLAGS_GMEM_RESTORE;
		}
	}

	yamato_device->drawctxt_active = drawctxt;

	
	if (drawctxt != NULL) {

		KGSL_CTXT_INFO("drawctxt flags %08x\n", drawctxt->flags);
		KGSL_CTXT_DBG("restore pagetable");
		kgsl_mmu_setstate(device, drawctxt->pagetable);

		
		if (drawctxt->flags & CTXT_FLAGS_GMEM_RESTORE) {
			unsigned int i, numbuffers = 0;
			KGSL_CTXT_DBG("restore gmem");

			for (i = 0; i < KGSL_MAX_GMEM_SHADOW_BUFFERS; i++) {
				if (drawctxt->user_gmem_shadow[i].gmemshadow.
				    size > 0) {
					kgsl_ringbuffer_issuecmds(device,
						KGSL_CMD_FLAGS_PMODE,
					  drawctxt->user_gmem_shadow[i].
						gmem_restore, 3);

					
					kgsl_ringbuffer_issuecmds(device, 0,
					  drawctxt->chicken_restore, 3);
					numbuffers++;
				}
			}
			if (numbuffers == 0) {
				kgsl_ringbuffer_issuecmds(device,
					KGSL_CMD_FLAGS_PMODE,
				  drawctxt->context_gmem_shadow.gmem_restore,
					3);

				
				kgsl_ringbuffer_issuecmds(device, 0,
				  drawctxt->chicken_restore, 3);
			}
			drawctxt->flags &= ~CTXT_FLAGS_GMEM_RESTORE;
		}

		
		KGSL_CTXT_DBG("restore regs");
		kgsl_ringbuffer_issuecmds(device, 0,
					  drawctxt->reg_restore, 3);

		
		if (drawctxt->flags & CTXT_FLAGS_SHADER_RESTORE) {
			KGSL_CTXT_DBG("restore shader");
			kgsl_ringbuffer_issuecmds(device, 0,
					  drawctxt->shader_restore, 3);
		}

		cmds[0] = pm4_type3_packet(PM4_SET_BIN_BASE_OFFSET, 1);
		cmds[1] = drawctxt->bin_base_offset;
		kgsl_ringbuffer_issuecmds(device, 0, cmds, 2);

	} else
		kgsl_mmu_setstate(device, device->mmu.defaultpagetable);

	KGSL_CTXT_INFO("return\n");
}
