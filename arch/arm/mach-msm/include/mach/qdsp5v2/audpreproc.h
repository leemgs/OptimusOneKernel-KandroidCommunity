

#ifndef _MACH_QDSP5_V2_AUDPREPROC_H
#define _MACH_QDSP5_V2_AUDPREPROC_H

#include <mach/qdsp5v2/qdsp5audpreproccmdi.h>
#include <mach/qdsp5v2/qdsp5audpreprocmsg.h>


typedef void (*audpreproc_event_func)(void *private, unsigned id, void *msg);

struct audpreproc_event_callback {
	audpreproc_event_func fn;
	void *private;
};



int audpreproc_aenc_alloc(unsigned enc_type, const char **module_name,
		unsigned *queue_id);
void audpreproc_aenc_free(int enc_id);

int audpreproc_enable(int enc_id, audpreproc_event_func func, void *private);
void audpreproc_disable(int enc_id, void *private);

int audpreproc_send_audreccmdqueue(void *cmd, unsigned len);

int audpreproc_send_preproccmdqueue(void *cmd, unsigned len);

int audpreproc_dsp_set_agc(struct audpreproc_cmd_cfg_agc_params *agc,
	unsigned len);
int audpreproc_dsp_set_agc2(struct audpreproc_cmd_cfg_agc_params_2 *agc2,
	unsigned len);
int audpreproc_dsp_set_ns(struct audpreproc_cmd_cfg_ns_params *ns,
	unsigned len);
int audpreproc_dsp_set_iir(
struct audpreproc_cmd_cfg_iir_tuning_filter_params *iir, unsigned len);

int audpreproc_dsp_set_agc(struct audpreproc_cmd_cfg_agc_params *agc,
 unsigned int len);

int audpreproc_dsp_set_iir(
struct audpreproc_cmd_cfg_iir_tuning_filter_params *iir, unsigned int len);

int audpreproc_unregister_event_callback(struct audpreproc_event_callback *ecb);

int audpreproc_register_event_callback(struct audpreproc_event_callback *ecb);


#endif 
