



#include "pvrusb2-wm8775.h"


#include "pvrusb2-hdw-internal.h"
#include "pvrusb2-debug.h"
#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <linux/errno.h>
#include <linux/slab.h>

void pvr2_wm8775_subdev_update(struct pvr2_hdw *hdw, struct v4l2_subdev *sd)
{
	if (hdw->input_dirty || hdw->force_dirty) {
		u32 input;

		switch (hdw->input_val) {
		case PVR2_CVAL_INPUT_RADIO:
			input = 1;
			break;
		default:
			
			input = 2;
			break;
		}
		pvr2_trace(PVR2_TRACE_CHIPS, "subdev wm8775"
			   " set_input(val=%d route=0x%x)",
			   hdw->input_val, input);

		sd->ops->audio->s_routing(sd, input, 0, 0);
	}
}




