

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#include <linux/msm_audio.h>

#include <mach/qdsp5v2/qdsp5audppmsg.h>
#include <mach/qdsp5v2/qdsp5audplaycmdi.h>
#include <mach/qdsp5v2/qdsp5audplaymsg.h>
#include <mach/qdsp5v2/audpp.h>
#include <mach/qdsp5v2/codec_utils.h>
#include <mach/qdsp5v2/pcm_funcs.h>
#include <mach/debug_mm.h>

long pcm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	MM_DBG("pcm_ioctl() cmd = %d\n", cmd);

	return -EINVAL;
}

void audpp_cmd_cfg_pcm_params(struct audio *audio)
{
	struct audpp_cmd_cfg_adec_params_wav cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.common.cmd_id = AUDPP_CMD_CFG_ADEC_PARAMS;
	cmd.common.length = AUDPP_CMD_CFG_ADEC_PARAMS_WAV_LEN >> 1;
	cmd.common.dec_id = audio->dec_id;
	cmd.common.input_sampling_frequency = audio->out_sample_rate;
	cmd.stereo_cfg = audio->out_channel_mode;
	cmd.pcm_width = audio->out_bits;
	cmd.sign = 0;
	audpp_send_queue2(&cmd, sizeof(cmd));
}
