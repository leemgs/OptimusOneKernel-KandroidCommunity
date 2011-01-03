


#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <media/v4l2-common.h>
#include <media/cx25840.h>

#include "cx25840-core.h"

static int set_audclk_freq(struct i2c_client *client, u32 freq)
{
	struct cx25840_state *state = to_state(i2c_get_clientdata(client));

	if (freq != 32000 && freq != 44100 && freq != 48000)
		return -EINVAL;

	
	
	if (!state->is_cx23885 && !state->is_cx231xx)
		cx25840_write(client, 0x127, 0x50);

	if (state->aud_input != CX25840_AUDIO_SERIAL) {
		switch (freq) {
		case 32000:
			if (state->is_cx23885) {
				
				break;
			}

			if (!state->is_cx231xx) {
				
				cx25840_write4(client, 0x108, 0x1006040f);

				
				cx25840_write4(client, 0x110, 0x01bb39ee);
			}

			if (state->is_cx25836)
				break;

			
			cx25840_write4(client, 0x900, 0x0801f77f);
			cx25840_write4(client, 0x904, 0x0801f77f);
			cx25840_write4(client, 0x90c, 0x0801f77f);
			break;

		case 44100:
			if (state->is_cx23885) {
				
				break;
			}

			if (!state->is_cx231xx) {
				
				cx25840_write4(client, 0x108, 0x1009040f);

				
				cx25840_write4(client, 0x110, 0x00ec6bd6);
			}

			if (state->is_cx25836)
				break;

			
			cx25840_write4(client, 0x900, 0x08016d59);
			cx25840_write4(client, 0x904, 0x08016d59);
			cx25840_write4(client, 0x90c, 0x08016d59);
			break;

		case 48000:
			if (state->is_cx23885) {
				
				break;
			}

			if (!state->is_cx231xx) {
				
				cx25840_write4(client, 0x108, 0x100a040f);

				
				cx25840_write4(client, 0x110, 0x0098d6e5);
			}

			if (state->is_cx25836)
				break;

			
			cx25840_write4(client, 0x900, 0x08014faa);
			cx25840_write4(client, 0x904, 0x08014faa);
			cx25840_write4(client, 0x90c, 0x08014faa);
			break;
		}
	} else {
		switch (freq) {
		case 32000:
			if (state->is_cx23885) {
				
				break;
			}

			if (!state->is_cx231xx) {
				
				cx25840_write4(client, 0x108, 0x1e08040f);

				
				cx25840_write4(client, 0x110, 0x012a0869);
			}

			if (state->is_cx25836)
				break;

			
			cx25840_write4(client, 0x8f8, 0x08010000);

			
			cx25840_write4(client, 0x900, 0x08020000);
			cx25840_write4(client, 0x904, 0x08020000);
			cx25840_write4(client, 0x90c, 0x08020000);

			
			cx25840_write(client, 0x127, 0x54);
			break;

		case 44100:
			if (state->is_cx23885) {
				
				break;
			}


			if (!state->is_cx231xx) {
				
				cx25840_write4(client, 0x108, 0x1809040f);

				
				cx25840_write4(client, 0x110, 0x00ec6bd6);
			}

			if (state->is_cx25836)
				break;

			
			cx25840_write4(client, 0x8f8, 0x080160cd);

			
			cx25840_write4(client, 0x900, 0x08017385);
			cx25840_write4(client, 0x904, 0x08017385);
			cx25840_write4(client, 0x90c, 0x08017385);
			break;

		case 48000:
			if (!state->is_cx23885 && !state->is_cx231xx) {
				
				cx25840_write4(client, 0x108, 0x180a040f);

				
				cx25840_write4(client, 0x110, 0x0098d6e5);
			}

			if (state->is_cx25836)
				break;

			if (!state->is_cx23885 && !state->is_cx231xx) {
				
				cx25840_write4(client, 0x8f8, 0x08018000);

				
				cx25840_write4(client, 0x900, 0x08015555);
				cx25840_write4(client, 0x904, 0x08015555);
				cx25840_write4(client, 0x90c, 0x08015555);
			} else {

				cx25840_write4(client, 0x8f8, 0x0801867c);

				cx25840_write4(client, 0x900, 0x08014faa);
				cx25840_write4(client, 0x904, 0x08014faa);
				cx25840_write4(client, 0x90c, 0x08014faa);
			}
			break;
		}
	}

	state->audclk_freq = freq;

	return 0;
}

void cx25840_audio_set_path(struct i2c_client *client)
{
	struct cx25840_state *state = to_state(i2c_get_clientdata(client));

	
	cx25840_and_or(client, 0x810, ~0x1, 0x01);

	
	cx25840_and_or(client, 0x803, ~0x10, 0);

	
	cx25840_write(client, 0x8d3, 0x1f);

	if (state->aud_input == CX25840_AUDIO_SERIAL) {
		
		cx25840_write4(client, 0x8d0, 0x01011012);

		
	} else {
		
		cx25840_write4(client, 0x8d0, 0x1f063870);
	}

	set_audclk_freq(client, state->audclk_freq);

	if (state->aud_input != CX25840_AUDIO_SERIAL) {
		
		cx25840_and_or(client, 0x803, ~0x10, 0x10);
	}

	
	cx25840_and_or(client, 0x810, ~0x1, 0x00);

	
	if (state->is_cx23885 || state->is_cx231xx)
		cx25840_and_or(client, 0x803, ~0x10, 0x10);
}

static int get_volume(struct i2c_client *client)
{
	struct cx25840_state *state = to_state(i2c_get_clientdata(client));
	int vol;

	if (state->unmute_volume >= 0)
		return state->unmute_volume;

	

	
	vol = 228 - cx25840_read(client, 0x8d4);
	vol = (vol / 2) + 23;
	return vol << 9;
}

static void set_volume(struct i2c_client *client, int volume)
{
	struct cx25840_state *state = to_state(i2c_get_clientdata(client));
	int vol;

	if (state->unmute_volume >= 0) {
		state->unmute_volume = volume;
		return;
	}

	
	vol = volume >> 9;

	
	if (vol <= 23) {
		vol = 0;
	} else {
		vol -= 23;
	}

	
	cx25840_write(client, 0x8d4, 228 - (vol * 2));
}

static int get_bass(struct i2c_client *client)
{
	

	
	int bass = cx25840_read(client, 0x8d9) & 0x3f;
	bass = (((48 - bass) * 0xffff) + 47) / 48;
	return bass;
}

static void set_bass(struct i2c_client *client, int bass)
{
	
	cx25840_and_or(client, 0x8d9, ~0x3f, 48 - (bass * 48 / 0xffff));
}

static int get_treble(struct i2c_client *client)
{
	

	
	int treble = cx25840_read(client, 0x8db) & 0x3f;
	treble = (((48 - treble) * 0xffff) + 47) / 48;
	return treble;
}

static void set_treble(struct i2c_client *client, int treble)
{
	
	cx25840_and_or(client, 0x8db, ~0x3f, 48 - (treble * 48 / 0xffff));
}

static int get_balance(struct i2c_client *client)
{
	

	
	int balance = cx25840_read(client, 0x8d5) & 0x7f;
	
	if ((cx25840_read(client, 0x8d5) & 0x80) == 0)
		balance = 0x80 - balance;
	else
		balance = 0x80 + balance;
	return balance << 8;
}

static void set_balance(struct i2c_client *client, int balance)
{
	int bal = balance >> 8;
	if (bal > 0x80) {
		
		cx25840_and_or(client, 0x8d5, 0x7f, 0x80);
		
		cx25840_and_or(client, 0x8d5, ~0x7f, bal & 0x7f);
	} else {
		
		cx25840_and_or(client, 0x8d5, 0x7f, 0x00);
		
		cx25840_and_or(client, 0x8d5, ~0x7f, 0x80 - bal);
	}
}

static int get_mute(struct i2c_client *client)
{
	struct cx25840_state *state = to_state(i2c_get_clientdata(client));

	return state->unmute_volume >= 0;
}

static void set_mute(struct i2c_client *client, int mute)
{
	struct cx25840_state *state = to_state(i2c_get_clientdata(client));

	if (mute && state->unmute_volume == -1) {
		int vol = get_volume(client);

		set_volume(client, 0);
		state->unmute_volume = vol;
	}
	else if (!mute && state->unmute_volume != -1) {
		int vol = state->unmute_volume;

		state->unmute_volume = -1;
		set_volume(client, vol);
	}
}

int cx25840_s_clock_freq(struct v4l2_subdev *sd, u32 freq)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct cx25840_state *state = to_state(sd);
	int retval;

	if (!state->is_cx25836)
		cx25840_and_or(client, 0x810, ~0x1, 1);
	if (state->aud_input != CX25840_AUDIO_SERIAL) {
		cx25840_and_or(client, 0x803, ~0x10, 0);
		cx25840_write(client, 0x8d3, 0x1f);
	}
	retval = set_audclk_freq(client, freq);
	if (state->aud_input != CX25840_AUDIO_SERIAL)
		cx25840_and_or(client, 0x803, ~0x10, 0x10);
	if (!state->is_cx25836)
		cx25840_and_or(client, 0x810, ~0x1, 0);
	return retval;
}

int cx25840_audio_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	switch (ctrl->id) {
	case V4L2_CID_AUDIO_VOLUME:
		ctrl->value = get_volume(client);
		break;
	case V4L2_CID_AUDIO_BASS:
		ctrl->value = get_bass(client);
		break;
	case V4L2_CID_AUDIO_TREBLE:
		ctrl->value = get_treble(client);
		break;
	case V4L2_CID_AUDIO_BALANCE:
		ctrl->value = get_balance(client);
		break;
	case V4L2_CID_AUDIO_MUTE:
		ctrl->value = get_mute(client);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int cx25840_audio_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	switch (ctrl->id) {
	case V4L2_CID_AUDIO_VOLUME:
		set_volume(client, ctrl->value);
		break;
	case V4L2_CID_AUDIO_BASS:
		set_bass(client, ctrl->value);
		break;
	case V4L2_CID_AUDIO_TREBLE:
		set_treble(client, ctrl->value);
		break;
	case V4L2_CID_AUDIO_BALANCE:
		set_balance(client, ctrl->value);
		break;
	case V4L2_CID_AUDIO_MUTE:
		set_mute(client, ctrl->value);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
