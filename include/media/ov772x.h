

#ifndef __OV772X_H__
#define __OV772X_H__

#include <media/soc_camera.h>


#define OV772X_FLAG_VFLIP     0x00000001 
#define OV772X_FLAG_HFLIP     0x00000002 


struct ov772x_edge_ctrl {
	unsigned char strength;
	unsigned char threshold;
	unsigned char upper;
	unsigned char lower;
};

#define OV772X_MANUAL_EDGE_CTRL	0x80 
#define EDGE_STRENGTH_MASK	0x1F
#define EDGE_THRESHOLD_MASK	0x0F
#define EDGE_UPPER_MASK		0xFF
#define EDGE_LOWER_MASK		0xFF

#define OV772X_AUTO_EDGECTRL(u, l)	\
{					\
	.upper = (u & EDGE_UPPER_MASK),	\
	.lower = (l & EDGE_LOWER_MASK),	\
}

#define OV772X_MANUAL_EDGECTRL(s, t)					\
{									\
	.strength  = (s & EDGE_STRENGTH_MASK) | OV772X_MANUAL_EDGE_CTRL,\
	.threshold = (t & EDGE_THRESHOLD_MASK),				\
}


struct ov772x_camera_info {
	unsigned long          buswidth;
	unsigned long          flags;
	struct soc_camera_link link;
	struct ov772x_edge_ctrl edgectrl;
};

#endif 
