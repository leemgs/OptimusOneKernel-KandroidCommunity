

#ifndef __CS46XX_DSP_SPOS_H__
#define __CS46XX_DSP_SPOS_H__

#include "cs46xx_dsp_scb_types.h"
#include "cs46xx_dsp_task_types.h"

#define SYMBOL_CONSTANT  0x0
#define SYMBOL_SAMPLE    0x1
#define SYMBOL_PARAMETER 0x2
#define SYMBOL_CODE      0x3

#define SEGTYPE_SP_PROGRAM              0x00000001
#define SEGTYPE_SP_PARAMETER            0x00000002
#define SEGTYPE_SP_SAMPLE               0x00000003
#define SEGTYPE_SP_COEFFICIENT          0x00000004

#define DSP_SPOS_UU      0x0deadul     
#define DSP_SPOS_DC      0x0badul      
#define DSP_SPOS_DC_DC   0x0bad0badul  
#define DSP_SPOS_UUUU    0xdeadc0edul  
#define DSP_SPOS_UUHI    0xdeadul
#define DSP_SPOS_UULO    0xc0edul
#define DSP_SPOS_DCDC    0x0badf1d0ul  
#define DSP_SPOS_DCDCHI  0x0badul
#define DSP_SPOS_DCDCLO  0xf1d0ul

#define DSP_MAX_TASK_NAME   60
#define DSP_MAX_SYMBOL_NAME 100
#define DSP_MAX_SCB_NAME    60
#define DSP_MAX_SCB_DESC    200
#define DSP_MAX_TASK_DESC   50

#define DSP_MAX_PCM_CHANNELS 32
#define DSP_MAX_SRC_NR       14

#define DSP_PCM_MAIN_CHANNEL        1
#define DSP_PCM_REAR_CHANNEL        2
#define DSP_PCM_CENTER_LFE_CHANNEL  3
#define DSP_PCM_S71_CHANNEL         4 
#define DSP_IEC958_CHANNEL          5

#define DSP_SPDIF_STATUS_OUTPUT_ENABLED       1
#define DSP_SPDIF_STATUS_PLAYBACK_OPEN        2
#define DSP_SPDIF_STATUS_HW_ENABLED           4
#define DSP_SPDIF_STATUS_INPUT_CTRL_ENABLED   8

struct dsp_symbol_entry {
	u32 address;
	char symbol_name[DSP_MAX_SYMBOL_NAME];
	int symbol_type;

	
	struct dsp_module_desc * module;
	int deleted;
};

struct dsp_symbol_desc {
	int nsymbols;

	struct dsp_symbol_entry *symbols;

	
	int highest_frag_index;
};

struct dsp_segment_desc {
	int segment_type;
	u32 offset;
	u32 size;
	u32 * data;
};

struct dsp_module_desc {
	char * module_name;
	struct dsp_symbol_desc symbol_table;
	int nsegments;
	struct dsp_segment_desc * segments;

	
	u32 overlay_begin_address;
	u32 load_address;
	int nfixups;
};

struct dsp_scb_descriptor {
	char scb_name[DSP_MAX_SCB_NAME];
	u32 address;
	int index;
	u32 *data;

	struct dsp_scb_descriptor * sub_list_ptr;
	struct dsp_scb_descriptor * next_scb_ptr;
	struct dsp_scb_descriptor * parent_scb_ptr;

	struct dsp_symbol_entry * task_entry;
	struct dsp_symbol_entry * scb_symbol;

	struct snd_info_entry *proc_info;
	int ref_count;
	spinlock_t lock;

	int deleted;
};

struct dsp_task_descriptor {
	char task_name[DSP_MAX_TASK_NAME];
	int size;
	u32 address;
	int index;
	u32 *data;
};

struct dsp_pcm_channel_descriptor {
	int active;
	int src_slot;
	int pcm_slot;
	u32 sample_rate;
	u32 unlinked;
	struct dsp_scb_descriptor * pcm_reader_scb;
	struct dsp_scb_descriptor * src_scb;
	struct dsp_scb_descriptor * mixer_scb;

	void * private_data;
};

struct dsp_spos_instance {
	struct dsp_symbol_desc symbol_table; 

	int nmodules;
	struct dsp_module_desc * modules; 

	struct dsp_segment_desc code;

	
	struct dsp_scb_descriptor * master_mix_scb;
	u16 dac_volume_right;
	u16 dac_volume_left;

	
	struct dsp_scb_descriptor * rear_mix_scb;

	
	struct dsp_scb_descriptor * center_lfe_mix_scb;

	int npcm_channels;
	int nsrc_scb;
	struct dsp_pcm_channel_descriptor pcm_channels[DSP_MAX_PCM_CHANNELS];
	int src_scb_slots[DSP_MAX_SRC_NR];

	
	struct dsp_symbol_entry * null_algorithm; 
	struct dsp_symbol_entry * s16_up;         

	  
	struct snd_card *snd_card;
	struct snd_info_entry * proc_dsp_dir;
	struct snd_info_entry * proc_sym_info_entry;
	struct snd_info_entry * proc_modules_info_entry;
	struct snd_info_entry * proc_parameter_dump_info_entry;
	struct snd_info_entry * proc_sample_dump_info_entry;

	
	int nscb;
	int scb_highest_frag_index;
	struct dsp_scb_descriptor scbs[DSP_MAX_SCB_DESC];
	struct snd_info_entry * proc_scb_info_entry;
	struct dsp_scb_descriptor * the_null_scb;

	
	int ntask;
	struct dsp_task_descriptor tasks[DSP_MAX_TASK_DESC];
	struct snd_info_entry * proc_task_info_entry;

	
	int spdif_status_out;
	int spdif_status_in;
	u16 spdif_input_volume_right;
	u16 spdif_input_volume_left;
	
	unsigned int spdif_csuv_default;
	unsigned int spdif_csuv_stream;

	
	struct dsp_scb_descriptor * spdif_in_src;
	
	struct dsp_scb_descriptor * asynch_rx_scb;

	
	struct dsp_scb_descriptor * record_mixer_scb;
    
	
	struct dsp_scb_descriptor * codec_in_scb;

	
	struct dsp_scb_descriptor * ref_snoop_scb;

	
	struct dsp_scb_descriptor * spdif_pcm_input_scb;

	
	struct dsp_scb_descriptor * asynch_tx_scb;

	
	struct dsp_scb_descriptor * pcm_input;
	struct dsp_scb_descriptor * adc_input;

	int spdif_in_sample_rate;
};

#endif 
