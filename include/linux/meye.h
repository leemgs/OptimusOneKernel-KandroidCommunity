

#ifndef _MEYE_H_
#define _MEYE_H_





struct meye_params {
	unsigned char subsample;
	unsigned char quality;
	unsigned char sharpness;
	unsigned char agc;
	unsigned char picture;
	unsigned char framerate;
};


#define MEYEIOC_G_PARAMS	_IOR ('v', BASE_VIDIOCPRIVATE+0, struct meye_params)

#define MEYEIOC_S_PARAMS	_IOW ('v', BASE_VIDIOCPRIVATE+1, struct meye_params)

#define MEYEIOC_QBUF_CAPT	_IOW ('v', BASE_VIDIOCPRIVATE+2, int)

#define MEYEIOC_SYNC		_IOWR('v', BASE_VIDIOCPRIVATE+3, int)

#define MEYEIOC_STILLCAPT	_IO  ('v', BASE_VIDIOCPRIVATE+4)

#define MEYEIOC_STILLJCAPT	_IOR ('v', BASE_VIDIOCPRIVATE+5, int)


#define V4L2_CID_AGC		V4L2_CID_PRIVATE_BASE
#define V4L2_CID_MEYE_SHARPNESS	(V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_PICTURE	(V4L2_CID_PRIVATE_BASE + 2)
#define V4L2_CID_JPEGQUAL	(V4L2_CID_PRIVATE_BASE + 3)
#define V4L2_CID_FRAMERATE	(V4L2_CID_PRIVATE_BASE + 4)

#endif
