


#define V4L2_PIX_FMT_MPEG4     v4l2_fourcc('M', 'P', 'G', '4') 


#define	GO7007IOC_S_BITRATE	_IOW('V', BASE_VIDIOC_PRIVATE + 0, int)
#define	GO7007IOC_G_BITRATE	_IOR('V', BASE_VIDIOC_PRIVATE + 1, int)

enum go7007_aspect_ratio {
	GO7007_ASPECT_RATIO_1_1 = 0,
	GO7007_ASPECT_RATIO_4_3_NTSC = 1,
	GO7007_ASPECT_RATIO_4_3_PAL = 2,
	GO7007_ASPECT_RATIO_16_9_NTSC = 3,
	GO7007_ASPECT_RATIO_16_9_PAL = 4,
};


struct go7007_comp_params {
	__u32 gop_size;
	__u32 max_b_frames;
	enum go7007_aspect_ratio aspect_ratio;
	__u32 flags;
	__u32 reserved[8];
};

#define GO7007_COMP_CLOSED_GOP		0x00000001
#define GO7007_COMP_OMIT_SEQ_HEADER	0x00000002

enum go7007_mpeg_video_standard {
	GO7007_MPEG_VIDEO_MPEG1 = 0,
	GO7007_MPEG_VIDEO_MPEG2 = 1,
	GO7007_MPEG_VIDEO_MPEG4 = 2,
};


struct go7007_mpeg_params {
	enum go7007_mpeg_video_standard mpeg_video_standard;
	__u32 flags;
	__u32 pali;
	__u32 reserved[8];
};

#define GO7007_MPEG_FORCE_DVD_MODE	0x00000001
#define GO7007_MPEG_OMIT_GOP_HEADER	0x00000002
#define GO7007_MPEG_REPEAT_SEQHEADER	0x00000004

#define GO7007_MPEG_PROFILE(format, pali)	(((format)<<24)|(pali))

#define GO7007_MPEG2_PROFILE_MAIN_MAIN		GO7007_MPEG_PROFILE(2, 0x48)

#define GO7007_MPEG4_PROFILE_S_L0		GO7007_MPEG_PROFILE(4, 0x08)
#define GO7007_MPEG4_PROFILE_S_L1		GO7007_MPEG_PROFILE(4, 0x01)
#define GO7007_MPEG4_PROFILE_S_L2		GO7007_MPEG_PROFILE(4, 0x02)
#define GO7007_MPEG4_PROFILE_S_L3		GO7007_MPEG_PROFILE(4, 0x03)
#define GO7007_MPEG4_PROFILE_ARTS_L1		GO7007_MPEG_PROFILE(4, 0x91)
#define GO7007_MPEG4_PROFILE_ARTS_L2		GO7007_MPEG_PROFILE(4, 0x92)
#define GO7007_MPEG4_PROFILE_ARTS_L3		GO7007_MPEG_PROFILE(4, 0x93)
#define GO7007_MPEG4_PROFILE_ARTS_L4		GO7007_MPEG_PROFILE(4, 0x94)
#define GO7007_MPEG4_PROFILE_AS_L0		GO7007_MPEG_PROFILE(4, 0xf0)
#define GO7007_MPEG4_PROFILE_AS_L1		GO7007_MPEG_PROFILE(4, 0xf1)
#define GO7007_MPEG4_PROFILE_AS_L2		GO7007_MPEG_PROFILE(4, 0xf2)
#define GO7007_MPEG4_PROFILE_AS_L3		GO7007_MPEG_PROFILE(4, 0xf3)
#define GO7007_MPEG4_PROFILE_AS_L4		GO7007_MPEG_PROFILE(4, 0xf4)
#define GO7007_MPEG4_PROFILE_AS_L5		GO7007_MPEG_PROFILE(4, 0xf5)

struct go7007_md_params {
	__u16 region;
	__u16 trigger;
	__u16 pixel_threshold;
	__u16 motion_threshold;
	__u32 reserved[8];
};

struct go7007_md_region {
	__u16 region;
	__u16 flags;
	struct v4l2_clip *clips;
	__u32 reserved[8];
};

#define	GO7007IOC_S_MPEG_PARAMS	_IOWR('V', BASE_VIDIOC_PRIVATE + 2, \
					struct go7007_mpeg_params)
#define	GO7007IOC_G_MPEG_PARAMS	_IOR('V', BASE_VIDIOC_PRIVATE + 3, \
					struct go7007_mpeg_params)
#define	GO7007IOC_S_COMP_PARAMS	_IOWR('V', BASE_VIDIOC_PRIVATE + 4, \
					struct go7007_comp_params)
#define	GO7007IOC_G_COMP_PARAMS	_IOR('V', BASE_VIDIOC_PRIVATE + 5, \
					struct go7007_comp_params)
#define	GO7007IOC_S_MD_PARAMS	_IOWR('V', BASE_VIDIOC_PRIVATE + 6, \
					struct go7007_md_params)
#define	GO7007IOC_G_MD_PARAMS	_IOR('V', BASE_VIDIOC_PRIVATE + 7, \
					struct go7007_md_params)
#define	GO7007IOC_S_MD_REGION	_IOW('V', BASE_VIDIOC_PRIVATE + 8, \
					struct go7007_md_region)
