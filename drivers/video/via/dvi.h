

#ifndef __DVI_H__
#define __DVI_H__


#define     VT1632_DEVICE_ID_REG        0x02
#define     VT1632_DEVICE_ID            0x92

#define     GET_DVI_SIZE_BY_SYSTEM_BIOS     0x01
#define     GET_DVI_SIZE_BY_VGA_BIOS        0x02
#define     GET_DVI_SZIE_BY_HW_STRAPPING    0x03



#define     DVI_PANEL_ID0_640X480       0x00

#define     DVI_PANEL_ID1_800x600       0x01

#define     DVI_PANEL_ID1_1024x768      0x02

#define     DVI_PANEL_ID1_1280x768      0x03

#define     DVI_PANEL_ID1_1280x1024     0x04

#define     DVI_PANEL_ID1_1400x1050     0x05

#define     DVI_PANEL_ID1_1600x1200     0x06


#define     EDID_VERSION_1      1
#define     EDID_VERSION_2      2

#define     DEV_CONNECT_DVI     0x01
#define     DEV_CONNECT_HDMI    0x02

struct VideoModeTable *viafb_get_cea_mode_tbl_pointer(int Index);
int viafb_dvi_sense(void);
void viafb_dvi_disable(void);
void viafb_dvi_enable(void);
int viafb_tmds_trasmitter_identify(void);
void viafb_init_dvi_size(void);
void viafb_dvi_set_mode(int video_index, int mode_bpp, int set_iga);

#endif 
