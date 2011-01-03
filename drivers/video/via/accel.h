

#ifndef __ACCEL_H__
#define __ACCEL_H__

#define FB_ACCEL_VIA_UNICHROME  50


#define MMIO_VGABASE                0x8000
#define MMIO_CR_READ                (MMIO_VGABASE + 0x3D4)
#define MMIO_CR_WRITE               (MMIO_VGABASE + 0x3D5)
#define MMIO_SR_READ                (MMIO_VGABASE + 0x3C4)
#define MMIO_SR_WRITE               (MMIO_VGABASE + 0x3C5)


#define HW_Cursor_ON    0
#define HW_Cursor_OFF   1

#define CURSOR_SIZE     (8 * 1024)
#define VQ_SIZE         (256 * 1024)

#define VIA_MMIO_BLTBASE        0x200000
#define VIA_MMIO_BLTSIZE        0x200000


#define VIA_REG_GECMD           0x000
#define VIA_REG_GEMODE          0x004
#define VIA_REG_SRCPOS          0x008
#define VIA_REG_DSTPOS          0x00C

#define VIA_REG_DIMENSION       0x010
#define VIA_REG_PATADDR         0x014
#define VIA_REG_FGCOLOR         0x018
#define VIA_REG_BGCOLOR         0x01C

#define VIA_REG_CLIPTL          0x020

#define VIA_REG_CLIPBR          0x024
#define VIA_REG_OFFSET          0x028

#define VIA_REG_KEYCONTROL      0x02C
#define VIA_REG_SRCBASE         0x030
#define VIA_REG_DSTBASE         0x034

#define VIA_REG_PITCH           0x038
#define VIA_REG_MONOPAT0        0x03C
#define VIA_REG_MONOPAT1        0x040

#define VIA_REG_COLORPAT        0x100


#define VIA_PITCH_ENABLE        0x80000000


#define VIA_REG_CURSOR_MODE     0x2D0
#define VIA_REG_CURSOR_POS      0x2D4
#define VIA_REG_CURSOR_ORG      0x2D8
#define VIA_REG_CURSOR_BG       0x2DC
#define VIA_REG_CURSOR_FG       0x2E0


#define VIA_GEM_8bpp            0x00000000
#define VIA_GEM_16bpp           0x00000100
#define VIA_GEM_32bpp           0x00000300


#define VIA_GEC_NOOP            0x00000000
#define VIA_GEC_BLT             0x00000001
#define VIA_GEC_LINE            0x00000005


#define VIA_GEC_ROT             0x00000008

#define VIA_GEC_SRC_XY          0x00000000
#define VIA_GEC_SRC_LINEAR      0x00000010
#define VIA_GEC_DST_XY          0x00000000
#define VIA_GEC_DST_LINRAT      0x00000020

#define VIA_GEC_SRC_FB          0x00000000
#define VIA_GEC_SRC_SYS         0x00000040
#define VIA_GEC_DST_FB          0x00000000
#define VIA_GEC_DST_SYS         0x00000080


#define VIA_GEC_SRC_MONO        0x00000100

#define VIA_GEC_PAT_MONO        0x00000200

#define VIA_GEC_MSRC_OPAQUE     0x00000000

#define VIA_GEC_MSRC_TRANS      0x00000400

#define VIA_GEC_PAT_FB          0x00000000

#define VIA_GEC_PAT_REG         0x00000800

#define VIA_GEC_CLIP_DISABLE    0x00000000
#define VIA_GEC_CLIP_ENABLE     0x00001000

#define VIA_GEC_FIXCOLOR_PAT    0x00002000

#define VIA_GEC_INCX            0x00000000
#define VIA_GEC_DECY            0x00004000
#define VIA_GEC_INCY            0x00000000
#define VIA_GEC_DECX            0x00008000

#define VIA_GEC_MPAT_OPAQUE     0x00000000

#define VIA_GEC_MPAT_TRANS      0x00010000

#define VIA_GEC_MONO_UNPACK     0x00000000
#define VIA_GEC_MONO_PACK       0x00020000
#define VIA_GEC_MONO_DWORD      0x00000000
#define VIA_GEC_MONO_WORD       0x00040000
#define VIA_GEC_MONO_BYTE       0x00080000

#define VIA_GEC_LASTPIXEL_ON    0x00000000
#define VIA_GEC_LASTPIXEL_OFF   0x00100000
#define VIA_GEC_X_MAJOR         0x00000000
#define VIA_GEC_Y_MAJOR         0x00200000
#define VIA_GEC_QUICK_START     0x00800000


#define VIA_REG_STATUS          0x400
#define VIA_REG_CR_TRANSET      0x41C
#define VIA_REG_CR_TRANSPACE	0x420
#define VIA_REG_TRANSET         0x43C
#define VIA_REG_TRANSPACE       0x440




#define VIA_CMD_RGTR_BUSY       0x00000080

#define VIA_2D_ENG_BUSY         0x00000002

#define VIA_3D_ENG_BUSY         0x00000001

#define VIA_VR_QUEUE_BUSY       0x00020000

#define MAXLOOP                 0xFFFFFF

#define VIA_BITBLT_COLOR	1
#define VIA_BITBLT_MONO		2
#define VIA_BITBLT_FILL		3

int viafb_init_engine(struct fb_info *info);
void viafb_show_hw_cursor(struct fb_info *info, int Status);
void viafb_wait_engine_idle(struct fb_info *info);

#endif 
