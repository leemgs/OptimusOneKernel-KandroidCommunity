

#ifndef __ASM__ARCH_MSM_ADSP_H
#define __ASM__ARCH_MSM_ADSP_H

struct msm_adsp_module;

struct msm_adsp_ops {
	
	void (*event)(void *driver_data, unsigned id, size_t len,
		      void (*getevent)(void *ptr, size_t len));
};


int msm_adsp_get(const char *name, struct msm_adsp_module **module,
		 struct msm_adsp_ops *ops, void *driver_data);
void msm_adsp_put(struct msm_adsp_module *module);
int msm_adsp_enable(struct msm_adsp_module *module);
int msm_adsp_disable(struct msm_adsp_module *module);
int adsp_set_clkrate(struct msm_adsp_module *module, unsigned long clk_rate);
int msm_adsp_disable_event_rsp(struct msm_adsp_module *module);


int msm_adsp_write(struct msm_adsp_module *module,
		   unsigned queue_id,
		   void *data, size_t len);

#define ADSP_MESSAGE_ID 0xFFFF


#define QDSP_lpmCommandQueue              0
#define QDSP_mpuAfeQueue                  1
#define QDSP_mpuGraphicsCmdQueue          2
#define QDSP_mpuModmathCmdQueue           3
#define QDSP_mpuVDecCmdQueue              4
#define QDSP_mpuVDecPktQueue              5
#define QDSP_mpuVEncCmdQueue              6
#define QDSP_rxMpuDecCmdQueue             7
#define QDSP_rxMpuDecPktQueue             8
#define QDSP_txMpuEncQueue                9
#define QDSP_uPAudPPCmd1Queue             10
#define QDSP_uPAudPPCmd2Queue             11
#define QDSP_uPAudPPCmd3Queue             12
#define QDSP_uPAudPlay0BitStreamCtrlQueue 13
#define QDSP_uPAudPlay1BitStreamCtrlQueue 14
#define QDSP_uPAudPlay2BitStreamCtrlQueue 15
#define QDSP_uPAudPlay3BitStreamCtrlQueue 16
#define QDSP_uPAudPlay4BitStreamCtrlQueue 17
#define QDSP_uPAudPreProcCmdQueue         18
#define QDSP_uPAudRecBitStreamQueue       19
#define QDSP_uPAudRecCmdQueue             20
#define QDSP_uPDiagQueue                  21
#define QDSP_uPJpegActionCmdQueue         22
#define QDSP_uPJpegCfgCmdQueue            23
#define QDSP_uPVocProcQueue               24
#define QDSP_vfeCommandQueue              25
#define QDSP_vfeCommandScaleQueue         26
#define QDSP_vfeCommandTableQueue         27
#define QDSP_vfeFtmCmdQueue               28
#define QDSP_vfeFtmCmdScaleQueue          29
#define QDSP_vfeFtmCmdTableQueue          30
#define QDSP_uPJpegFtmCfgCmdQueue         31
#define QDSP_uPJpegFtmActionCmdQueue      32
#define QDSP_apuAfeQueue                  33
#define QDSP_mpuRmtQueue                  34
#define QDSP_uPAudPreProcAudRecCmdQueue   35
#define QDSP_uPAudRec0BitStreamQueue      36
#define QDSP_uPAudRec0CmdQueue            37
#define QDSP_uPAudRec1BitStreamQueue      38
#define QDSP_uPAudRec1CmdQueue            39
#define QDSP_MAX_NUM_QUEUES               40

#endif
