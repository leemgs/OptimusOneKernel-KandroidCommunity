

#include <linux/module.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-i2c-drv.h>

MODULE_DESCRIPTION("wm8775 driver");
MODULE_AUTHOR("Ulf Eklund, Hans Verkuil");
MODULE_LICENSE("GPL");





enum {
	R7 = 7, R11 = 11,
	R12, R13, R14, R15, R16, R17, R18, R19, R20, R21, R23 = 23,
	TOT_REGS
};

struct wm8775_state {
	struct v4l2_subdev sd;
	u8 input;		
	u8 muted;
};

static inline struct wm8775_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct wm8775_state, sd);
}

static int wm8775_write(struct v4l2_subdev *sd, int reg, u16 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int i;

	if (reg < 0 || reg >= TOT_REGS) {
		v4l2_err(sd, "Invalid register R%d\n", reg);
		return -1;
	}

	for (i = 0; i < 3; i++)
		if (i2c_smbus_write_byte_data(client,
				(reg << 1) | (val >> 8), val & 0xff) == 0)
			return 0;
	v4l2_err(sd, "I2C: cannot write %03x to register R%d\n", val, reg);
	return -1;
}

static int wm8775_s_routing(struct v4l2_subdev *sd,
			    u32 input, u32 output, u32 config)
{
	struct wm8775_state *state = to_state(sd);

	
	if (input > 15) {
		v4l2_err(sd, "Invalid input %d.\n", input);
		return -EINVAL;
	}
	state->input = input;
	if (state->muted)
		return 0;
	wm8775_write(sd, R21, 0x0c0);
	wm8775_write(sd, R14, 0x1d4);
	wm8775_write(sd, R15, 0x1d4);
	wm8775_write(sd, R21, 0x100 + state->input);
	return 0;
}

static int wm8775_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct wm8775_state *state = to_state(sd);

	if (ctrl->id != V4L2_CID_AUDIO_MUTE)
		return -EINVAL;
	ctrl->value = state->muted;
	return 0;
}

static int wm8775_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct wm8775_state *state = to_state(sd);

	if (ctrl->id != V4L2_CID_AUDIO_MUTE)
		return -EINVAL;
	state->muted = ctrl->value;
	wm8775_write(sd, R21, 0x0c0);
	wm8775_write(sd, R14, 0x1d4);
	wm8775_write(sd, R15, 0x1d4);
	if (!state->muted)
		wm8775_write(sd, R21, 0x100 + state->input);
	return 0;
}

static int wm8775_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_WM8775, 0);
}

static int wm8775_log_status(struct v4l2_subdev *sd)
{
	struct wm8775_state *state = to_state(sd);

	v4l2_info(sd, "Input: %d%s\n", state->input,
			state->muted ? " (muted)" : "");
	return 0;
}

static int wm8775_s_frequency(struct v4l2_subdev *sd, struct v4l2_frequency *freq)
{
	struct wm8775_state *state = to_state(sd);

	
	wm8775_write(sd, R21, 0x0c0);
	wm8775_write(sd, R14, 0x1d4);
	wm8775_write(sd, R15, 0x1d4);
	wm8775_write(sd, R21, 0x100 + state->input);
	return 0;
}



static const struct v4l2_subdev_core_ops wm8775_core_ops = {
	.log_status = wm8775_log_status,
	.g_chip_ident = wm8775_g_chip_ident,
	.g_ctrl = wm8775_g_ctrl,
	.s_ctrl = wm8775_s_ctrl,
};

static const struct v4l2_subdev_tuner_ops wm8775_tuner_ops = {
	.s_frequency = wm8775_s_frequency,
};

static const struct v4l2_subdev_audio_ops wm8775_audio_ops = {
	.s_routing = wm8775_s_routing,
};

static const struct v4l2_subdev_ops wm8775_ops = {
	.core = &wm8775_core_ops,
	.tuner = &wm8775_tuner_ops,
	.audio = &wm8775_audio_ops,
};







static int wm8775_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct wm8775_state *state;
	struct v4l2_subdev *sd;

	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	v4l_info(client, "chip found @ 0x%02x (%s)\n",
			client->addr << 1, client->adapter->name);

	state = kmalloc(sizeof(struct wm8775_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;
	sd = &state->sd;
	v4l2_i2c_subdev_init(sd, client, &wm8775_ops);
	state->input = 2;
	state->muted = 0;

	

	
	wm8775_write(sd, R23, 0x000);
	
	wm8775_write(sd, R7, 0x000);
	
	wm8775_write(sd, R11, 0x021);
	
	wm8775_write(sd, R12, 0x102);
	
	wm8775_write(sd, R13, 0x000);
	
	wm8775_write(sd, R14, 0x1d4);
	
	wm8775_write(sd, R15, 0x1d4);
	
	wm8775_write(sd, R16, 0x1bf);
	
	wm8775_write(sd, R17, 0x185);
	
	wm8775_write(sd, R18, 0x0a2);
	
	wm8775_write(sd, R19, 0x005);
	
	wm8775_write(sd, R20, 0x07a);
	
	wm8775_write(sd, R21, 0x102);
	return 0;
}

static int wm8775_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id wm8775_id[] = {
	{ "wm8775", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, wm8775_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "wm8775",
	.probe = wm8775_probe,
	.remove = wm8775_remove,
	.id_table = wm8775_id,
};
