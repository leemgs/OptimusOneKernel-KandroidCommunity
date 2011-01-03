




#ifndef BNX2X_DUMP_H
#define BNX2X_DUMP_H


struct dump_sign {
	u32 time_stamp;
	u32 diag_ver;
	u32 grc_dump_ver;
};

#define TSTORM_WAITP_ADDR		0x1b8a80
#define CSTORM_WAITP_ADDR		0x238a80
#define XSTORM_WAITP_ADDR		0x2b8a80
#define USTORM_WAITP_ADDR		0x338a80
#define TSTORM_CAM_MODE			0x1b1440

#define RI_E1				0x1
#define RI_E1H				0x2
#define RI_ONLINE			0x100

#define RI_E1_OFFLINE			(RI_E1)
#define RI_E1_ONLINE			(RI_E1 | RI_ONLINE)
#define RI_E1H_OFFLINE			(RI_E1H)
#define RI_E1H_ONLINE			(RI_E1H | RI_ONLINE)
#define RI_ALL_OFFLINE			(RI_E1 | RI_E1H)
#define RI_ALL_ONLINE			(RI_E1 | RI_E1H | RI_ONLINE)

#define MAX_TIMER_PENDING		200
#define TIMER_SCAN_DONT_CARE		0xFF


struct dump_hdr {
	u32		 hdr_size;	
	struct dump_sign dump_sign;
	u32		 xstorm_waitp;
	u32		 tstorm_waitp;
	u32		 ustorm_waitp;
	u32		 cstorm_waitp;
	u16		 info;
	u8		 idle_chk;
	u8		 reserved;
};

struct reg_addr {
	u32 addr;
	u32 size;
	u16 info;
};

struct wreg_addr {
	u32 addr;
	u32 size;
	u32 read_regs_count;
	const u32 *read_regs;
	u16 info;
};


#define REGS_COUNT			558
static const struct reg_addr reg_addrs[REGS_COUNT] = {
	{ 0x2000, 341, RI_ALL_ONLINE }, { 0x2800, 103, RI_ALL_ONLINE },
	{ 0x3000, 287, RI_ALL_ONLINE }, { 0x3800, 331, RI_ALL_ONLINE },
	{ 0x8800, 6, RI_E1_ONLINE }, { 0xa000, 223, RI_ALL_ONLINE },
	{ 0xa388, 1, RI_ALL_ONLINE }, { 0xa398, 1, RI_ALL_ONLINE },
	{ 0xa39c, 7, RI_E1H_ONLINE }, { 0xa3c0, 3, RI_E1H_ONLINE },
	{ 0xa3d0, 1, RI_E1H_ONLINE }, { 0xa3d8, 1, RI_E1H_ONLINE },
	{ 0xa3e0, 1, RI_E1H_ONLINE }, { 0xa3e8, 1, RI_E1H_ONLINE },
	{ 0xa3f0, 1, RI_E1H_ONLINE }, { 0xa3f8, 1, RI_E1H_ONLINE },
	{ 0xa400, 69, RI_ALL_ONLINE }, { 0xa518, 1, RI_ALL_ONLINE },
	{ 0xa520, 1, RI_ALL_ONLINE }, { 0xa528, 1, RI_ALL_ONLINE },
	{ 0xa530, 1, RI_ALL_ONLINE }, { 0xa538, 1, RI_ALL_ONLINE },
	{ 0xa540, 1, RI_ALL_ONLINE }, { 0xa548, 1, RI_ALL_ONLINE },
	{ 0xa550, 1, RI_ALL_ONLINE }, { 0xa558, 1, RI_ALL_ONLINE },
	{ 0xa560, 1, RI_ALL_ONLINE }, { 0xa568, 1, RI_ALL_ONLINE },
	{ 0xa570, 1, RI_ALL_ONLINE }, { 0xa580, 1, RI_ALL_ONLINE },
	{ 0xa590, 1, RI_ALL_ONLINE }, { 0xa5a0, 1, RI_ALL_ONLINE },
	{ 0xa5c0, 1, RI_ALL_ONLINE }, { 0xa5e0, 1, RI_E1H_ONLINE },
	{ 0xa5e8, 1, RI_E1H_ONLINE }, { 0xa5f0, 1, RI_E1H_ONLINE },
	{ 0xa5f8, 10, RI_E1H_ONLINE }, { 0x10000, 236, RI_ALL_ONLINE },
	{ 0x103bc, 1, RI_ALL_ONLINE }, { 0x103cc, 1, RI_ALL_ONLINE },
	{ 0x103dc, 1, RI_ALL_ONLINE }, { 0x10400, 57, RI_ALL_ONLINE },
	{ 0x104e8, 2, RI_ALL_ONLINE }, { 0x104f4, 2, RI_ALL_ONLINE },
	{ 0x10500, 146, RI_ALL_ONLINE }, { 0x10750, 2, RI_ALL_ONLINE },
	{ 0x10760, 2, RI_ALL_ONLINE }, { 0x10770, 2, RI_ALL_ONLINE },
	{ 0x10780, 2, RI_ALL_ONLINE }, { 0x10790, 2, RI_ALL_ONLINE },
	{ 0x107a0, 2, RI_ALL_ONLINE }, { 0x107b0, 2, RI_ALL_ONLINE },
	{ 0x107c0, 2, RI_ALL_ONLINE }, { 0x107d0, 2, RI_ALL_ONLINE },
	{ 0x107e0, 2, RI_ALL_ONLINE }, { 0x10880, 2, RI_ALL_ONLINE },
	{ 0x10900, 2, RI_ALL_ONLINE }, { 0x12000, 1, RI_ALL_ONLINE },
	{ 0x14000, 1, RI_ALL_ONLINE }, { 0x16000, 26, RI_E1H_ONLINE },
	{ 0x16070, 18, RI_E1H_ONLINE }, { 0x160c0, 27, RI_E1H_ONLINE },
	{ 0x16140, 1, RI_E1H_ONLINE }, { 0x16160, 1, RI_E1H_ONLINE },
	{ 0x16180, 2, RI_E1H_ONLINE }, { 0x161c0, 2, RI_E1H_ONLINE },
	{ 0x16204, 5, RI_E1H_ONLINE }, { 0x18000, 1, RI_E1H_ONLINE },
	{ 0x18008, 1, RI_E1H_ONLINE }, { 0x20000, 24, RI_ALL_ONLINE },
	{ 0x20060, 8, RI_ALL_ONLINE }, { 0x20080, 138, RI_ALL_ONLINE },
	{ 0x202b4, 1, RI_ALL_ONLINE }, { 0x202c4, 1, RI_ALL_ONLINE },
	{ 0x20400, 2, RI_ALL_ONLINE }, { 0x2040c, 8, RI_ALL_ONLINE },
	{ 0x2042c, 18, RI_E1H_ONLINE }, { 0x20480, 1, RI_ALL_ONLINE },
	{ 0x20500, 1, RI_ALL_ONLINE }, { 0x20600, 1, RI_ALL_ONLINE },
	{ 0x28000, 1, RI_ALL_ONLINE }, { 0x28004, 8191, RI_ALL_OFFLINE },
	{ 0x30000, 1, RI_ALL_ONLINE }, { 0x30004, 16383, RI_ALL_OFFLINE },
	{ 0x40000, 98, RI_ALL_ONLINE }, { 0x40194, 1, RI_ALL_ONLINE },
	{ 0x401a4, 1, RI_ALL_ONLINE }, { 0x401a8, 11, RI_E1H_ONLINE },
	{ 0x40200, 4, RI_ALL_ONLINE }, { 0x40400, 43, RI_ALL_ONLINE },
	{ 0x404b8, 1, RI_ALL_ONLINE }, { 0x404c8, 1, RI_ALL_ONLINE },
	{ 0x404cc, 3, RI_E1H_ONLINE }, { 0x40500, 2, RI_ALL_ONLINE },
	{ 0x40510, 2, RI_ALL_ONLINE }, { 0x40520, 2, RI_ALL_ONLINE },
	{ 0x40530, 2, RI_ALL_ONLINE }, { 0x40540, 2, RI_ALL_ONLINE },
	{ 0x42000, 164, RI_ALL_ONLINE }, { 0x4229c, 1, RI_ALL_ONLINE },
	{ 0x422ac, 1, RI_ALL_ONLINE }, { 0x422bc, 1, RI_ALL_ONLINE },
	{ 0x422d4, 5, RI_E1H_ONLINE }, { 0x42400, 49, RI_ALL_ONLINE },
	{ 0x424c8, 38, RI_ALL_ONLINE }, { 0x42568, 2, RI_ALL_ONLINE },
	{ 0x42800, 1, RI_ALL_ONLINE }, { 0x50000, 20, RI_ALL_ONLINE },
	{ 0x50050, 8, RI_ALL_ONLINE }, { 0x50070, 88, RI_ALL_ONLINE },
	{ 0x501dc, 1, RI_ALL_ONLINE }, { 0x501ec, 1, RI_ALL_ONLINE },
	{ 0x501f0, 4, RI_E1H_ONLINE }, { 0x50200, 2, RI_ALL_ONLINE },
	{ 0x5020c, 7, RI_ALL_ONLINE }, { 0x50228, 6, RI_E1H_ONLINE },
	{ 0x50240, 1, RI_ALL_ONLINE }, { 0x50280, 1, RI_ALL_ONLINE },
	{ 0x52000, 1, RI_ALL_ONLINE }, { 0x54000, 1, RI_ALL_ONLINE },
	{ 0x54004, 3327, RI_ALL_OFFLINE }, { 0x58000, 1, RI_ALL_ONLINE },
	{ 0x58004, 8191, RI_ALL_OFFLINE }, { 0x60000, 71, RI_ALL_ONLINE },
	{ 0x60128, 1, RI_ALL_ONLINE }, { 0x60138, 1, RI_ALL_ONLINE },
	{ 0x6013c, 24, RI_E1H_ONLINE }, { 0x60200, 1, RI_ALL_ONLINE },
	{ 0x61000, 1, RI_ALL_ONLINE }, { 0x61004, 511, RI_ALL_OFFLINE },
	{ 0x70000, 8, RI_ALL_ONLINE }, { 0x70020, 21496, RI_ALL_OFFLINE },
	{ 0x85000, 3, RI_ALL_ONLINE }, { 0x8500c, 4, RI_ALL_OFFLINE },
	{ 0x8501c, 7, RI_ALL_ONLINE }, { 0x85038, 4, RI_ALL_OFFLINE },
	{ 0x85048, 1, RI_ALL_ONLINE }, { 0x8504c, 109, RI_ALL_OFFLINE },
	{ 0x85200, 32, RI_ALL_ONLINE }, { 0x85280, 11104, RI_ALL_OFFLINE },
	{ 0xa0000, 16384, RI_ALL_ONLINE }, { 0xb0000, 16384, RI_E1H_ONLINE },
	{ 0xc1000, 7, RI_ALL_ONLINE }, { 0xc1028, 1, RI_ALL_ONLINE },
	{ 0xc1038, 1, RI_ALL_ONLINE }, { 0xc1800, 2, RI_ALL_ONLINE },
	{ 0xc2000, 164, RI_ALL_ONLINE }, { 0xc229c, 1, RI_ALL_ONLINE },
	{ 0xc22ac, 1, RI_ALL_ONLINE }, { 0xc22bc, 1, RI_ALL_ONLINE },
	{ 0xc2400, 49, RI_ALL_ONLINE }, { 0xc24c8, 38, RI_ALL_ONLINE },
	{ 0xc2568, 2, RI_ALL_ONLINE }, { 0xc2600, 1, RI_ALL_ONLINE },
	{ 0xc4000, 165, RI_ALL_ONLINE }, { 0xc42a0, 1, RI_ALL_ONLINE },
	{ 0xc42b0, 1, RI_ALL_ONLINE }, { 0xc42c0, 1, RI_ALL_ONLINE },
	{ 0xc42e0, 7, RI_E1H_ONLINE }, { 0xc4400, 51, RI_ALL_ONLINE },
	{ 0xc44d0, 38, RI_ALL_ONLINE }, { 0xc4570, 2, RI_ALL_ONLINE },
	{ 0xc4600, 1, RI_ALL_ONLINE }, { 0xd0000, 19, RI_ALL_ONLINE },
	{ 0xd004c, 8, RI_ALL_ONLINE }, { 0xd006c, 91, RI_ALL_ONLINE },
	{ 0xd01e4, 1, RI_ALL_ONLINE }, { 0xd01f4, 1, RI_ALL_ONLINE },
	{ 0xd0200, 2, RI_ALL_ONLINE }, { 0xd020c, 7, RI_ALL_ONLINE },
	{ 0xd0228, 18, RI_E1H_ONLINE }, { 0xd0280, 1, RI_ALL_ONLINE },
	{ 0xd0300, 1, RI_ALL_ONLINE }, { 0xd0400, 1, RI_ALL_ONLINE },
	{ 0xd4000, 1, RI_ALL_ONLINE }, { 0xd4004, 2559, RI_ALL_OFFLINE },
	{ 0xd8000, 1, RI_ALL_ONLINE }, { 0xd8004, 8191, RI_ALL_OFFLINE },
	{ 0xe0000, 21, RI_ALL_ONLINE }, { 0xe0054, 8, RI_ALL_ONLINE },
	{ 0xe0074, 85, RI_ALL_ONLINE }, { 0xe01d4, 1, RI_ALL_ONLINE },
	{ 0xe01e4, 1, RI_ALL_ONLINE }, { 0xe0200, 2, RI_ALL_ONLINE },
	{ 0xe020c, 8, RI_ALL_ONLINE }, { 0xe022c, 18, RI_E1H_ONLINE },
	{ 0xe0280, 1, RI_ALL_ONLINE }, { 0xe0300, 1, RI_ALL_ONLINE },
	{ 0xe1000, 1, RI_ALL_ONLINE }, { 0xe2000, 1, RI_ALL_ONLINE },
	{ 0xe2004, 2047, RI_ALL_OFFLINE }, { 0xf0000, 1, RI_ALL_ONLINE },
	{ 0xf0004, 16383, RI_ALL_OFFLINE }, { 0x101000, 12, RI_ALL_ONLINE },
	{ 0x10103c, 1, RI_ALL_ONLINE }, { 0x10104c, 1, RI_ALL_ONLINE },
	{ 0x101050, 1, RI_E1H_ONLINE }, { 0x101100, 1, RI_ALL_ONLINE },
	{ 0x101800, 8, RI_ALL_ONLINE }, { 0x102000, 18, RI_ALL_ONLINE },
	{ 0x102054, 1, RI_ALL_ONLINE }, { 0x102064, 1, RI_ALL_ONLINE },
	{ 0x102080, 17, RI_ALL_ONLINE }, { 0x1020c8, 8, RI_E1H_ONLINE },
	{ 0x102400, 1, RI_ALL_ONLINE }, { 0x103000, 26, RI_ALL_ONLINE },
	{ 0x103074, 1, RI_ALL_ONLINE }, { 0x103084, 1, RI_ALL_ONLINE },
	{ 0x103094, 1, RI_ALL_ONLINE }, { 0x103098, 5, RI_E1H_ONLINE },
	{ 0x103800, 8, RI_ALL_ONLINE }, { 0x104000, 63, RI_ALL_ONLINE },
	{ 0x104108, 1, RI_ALL_ONLINE }, { 0x104118, 1, RI_ALL_ONLINE },
	{ 0x104200, 17, RI_ALL_ONLINE }, { 0x104400, 64, RI_ALL_ONLINE },
	{ 0x104500, 192, RI_ALL_OFFLINE }, { 0x104800, 64, RI_ALL_ONLINE },
	{ 0x104900, 192, RI_ALL_OFFLINE }, { 0x105000, 7, RI_ALL_ONLINE },
	{ 0x10501c, 1, RI_ALL_OFFLINE }, { 0x105020, 3, RI_ALL_ONLINE },
	{ 0x10502c, 1, RI_ALL_OFFLINE }, { 0x105030, 3, RI_ALL_ONLINE },
	{ 0x10503c, 1, RI_ALL_OFFLINE }, { 0x105040, 3, RI_ALL_ONLINE },
	{ 0x10504c, 1, RI_ALL_OFFLINE }, { 0x105050, 3, RI_ALL_ONLINE },
	{ 0x10505c, 1, RI_ALL_OFFLINE }, { 0x105060, 3, RI_ALL_ONLINE },
	{ 0x10506c, 1, RI_ALL_OFFLINE }, { 0x105070, 3, RI_ALL_ONLINE },
	{ 0x10507c, 1, RI_ALL_OFFLINE }, { 0x105080, 3, RI_ALL_ONLINE },
	{ 0x10508c, 1, RI_ALL_OFFLINE }, { 0x105090, 3, RI_ALL_ONLINE },
	{ 0x10509c, 1, RI_ALL_OFFLINE }, { 0x1050a0, 3, RI_ALL_ONLINE },
	{ 0x1050ac, 1, RI_ALL_OFFLINE }, { 0x1050b0, 3, RI_ALL_ONLINE },
	{ 0x1050bc, 1, RI_ALL_OFFLINE }, { 0x1050c0, 3, RI_ALL_ONLINE },
	{ 0x1050cc, 1, RI_ALL_OFFLINE }, { 0x1050d0, 3, RI_ALL_ONLINE },
	{ 0x1050dc, 1, RI_ALL_OFFLINE }, { 0x1050e0, 3, RI_ALL_ONLINE },
	{ 0x1050ec, 1, RI_ALL_OFFLINE }, { 0x1050f0, 3, RI_ALL_ONLINE },
	{ 0x1050fc, 1, RI_ALL_OFFLINE }, { 0x105100, 3, RI_ALL_ONLINE },
	{ 0x10510c, 1, RI_ALL_OFFLINE }, { 0x105110, 3, RI_ALL_ONLINE },
	{ 0x10511c, 1, RI_ALL_OFFLINE }, { 0x105120, 3, RI_ALL_ONLINE },
	{ 0x10512c, 1, RI_ALL_OFFLINE }, { 0x105130, 3, RI_ALL_ONLINE },
	{ 0x10513c, 1, RI_ALL_OFFLINE }, { 0x105140, 3, RI_ALL_ONLINE },
	{ 0x10514c, 1, RI_ALL_OFFLINE }, { 0x105150, 3, RI_ALL_ONLINE },
	{ 0x10515c, 1, RI_ALL_OFFLINE }, { 0x105160, 3, RI_ALL_ONLINE },
	{ 0x10516c, 1, RI_ALL_OFFLINE }, { 0x105170, 3, RI_ALL_ONLINE },
	{ 0x10517c, 1, RI_ALL_OFFLINE }, { 0x105180, 3, RI_ALL_ONLINE },
	{ 0x10518c, 1, RI_ALL_OFFLINE }, { 0x105190, 3, RI_ALL_ONLINE },
	{ 0x10519c, 1, RI_ALL_OFFLINE }, { 0x1051a0, 3, RI_ALL_ONLINE },
	{ 0x1051ac, 1, RI_ALL_OFFLINE }, { 0x1051b0, 3, RI_ALL_ONLINE },
	{ 0x1051bc, 1, RI_ALL_OFFLINE }, { 0x1051c0, 3, RI_ALL_ONLINE },
	{ 0x1051cc, 1, RI_ALL_OFFLINE }, { 0x1051d0, 3, RI_ALL_ONLINE },
	{ 0x1051dc, 1, RI_ALL_OFFLINE }, { 0x1051e0, 3, RI_ALL_ONLINE },
	{ 0x1051ec, 1, RI_ALL_OFFLINE }, { 0x1051f0, 3, RI_ALL_ONLINE },
	{ 0x1051fc, 1, RI_ALL_OFFLINE }, { 0x105200, 3, RI_ALL_ONLINE },
	{ 0x10520c, 1, RI_ALL_OFFLINE }, { 0x105210, 3, RI_ALL_ONLINE },
	{ 0x10521c, 1, RI_ALL_OFFLINE }, { 0x105220, 3, RI_ALL_ONLINE },
	{ 0x10522c, 1, RI_ALL_OFFLINE }, { 0x105230, 3, RI_ALL_ONLINE },
	{ 0x10523c, 1, RI_ALL_OFFLINE }, { 0x105240, 3, RI_ALL_ONLINE },
	{ 0x10524c, 1, RI_ALL_OFFLINE }, { 0x105250, 3, RI_ALL_ONLINE },
	{ 0x10525c, 1, RI_ALL_OFFLINE }, { 0x105260, 3, RI_ALL_ONLINE },
	{ 0x10526c, 1, RI_ALL_OFFLINE }, { 0x105270, 3, RI_ALL_ONLINE },
	{ 0x10527c, 1, RI_ALL_OFFLINE }, { 0x105280, 3, RI_ALL_ONLINE },
	{ 0x10528c, 1, RI_ALL_OFFLINE }, { 0x105290, 3, RI_ALL_ONLINE },
	{ 0x10529c, 1, RI_ALL_OFFLINE }, { 0x1052a0, 3, RI_ALL_ONLINE },
	{ 0x1052ac, 1, RI_ALL_OFFLINE }, { 0x1052b0, 3, RI_ALL_ONLINE },
	{ 0x1052bc, 1, RI_ALL_OFFLINE }, { 0x1052c0, 3, RI_ALL_ONLINE },
	{ 0x1052cc, 1, RI_ALL_OFFLINE }, { 0x1052d0, 3, RI_ALL_ONLINE },
	{ 0x1052dc, 1, RI_ALL_OFFLINE }, { 0x1052e0, 3, RI_ALL_ONLINE },
	{ 0x1052ec, 1, RI_ALL_OFFLINE }, { 0x1052f0, 3, RI_ALL_ONLINE },
	{ 0x1052fc, 1, RI_ALL_OFFLINE }, { 0x105300, 3, RI_ALL_ONLINE },
	{ 0x10530c, 1, RI_ALL_OFFLINE }, { 0x105310, 3, RI_ALL_ONLINE },
	{ 0x10531c, 1, RI_ALL_OFFLINE }, { 0x105320, 3, RI_ALL_ONLINE },
	{ 0x10532c, 1, RI_ALL_OFFLINE }, { 0x105330, 3, RI_ALL_ONLINE },
	{ 0x10533c, 1, RI_ALL_OFFLINE }, { 0x105340, 3, RI_ALL_ONLINE },
	{ 0x10534c, 1, RI_ALL_OFFLINE }, { 0x105350, 3, RI_ALL_ONLINE },
	{ 0x10535c, 1, RI_ALL_OFFLINE }, { 0x105360, 3, RI_ALL_ONLINE },
	{ 0x10536c, 1, RI_ALL_OFFLINE }, { 0x105370, 3, RI_ALL_ONLINE },
	{ 0x10537c, 1, RI_ALL_OFFLINE }, { 0x105380, 3, RI_ALL_ONLINE },
	{ 0x10538c, 1, RI_ALL_OFFLINE }, { 0x105390, 3, RI_ALL_ONLINE },
	{ 0x10539c, 1, RI_ALL_OFFLINE }, { 0x1053a0, 3, RI_ALL_ONLINE },
	{ 0x1053ac, 1, RI_ALL_OFFLINE }, { 0x1053b0, 3, RI_ALL_ONLINE },
	{ 0x1053bc, 1, RI_ALL_OFFLINE }, { 0x1053c0, 3, RI_ALL_ONLINE },
	{ 0x1053cc, 1, RI_ALL_OFFLINE }, { 0x1053d0, 3, RI_ALL_ONLINE },
	{ 0x1053dc, 1, RI_ALL_OFFLINE }, { 0x1053e0, 3, RI_ALL_ONLINE },
	{ 0x1053ec, 1, RI_ALL_OFFLINE }, { 0x1053f0, 3, RI_ALL_ONLINE },
	{ 0x1053fc, 769, RI_ALL_OFFLINE }, { 0x108000, 33, RI_ALL_ONLINE },
	{ 0x108090, 1, RI_ALL_ONLINE }, { 0x1080a0, 1, RI_ALL_ONLINE },
	{ 0x1080ac, 5, RI_E1H_ONLINE }, { 0x108100, 5, RI_ALL_ONLINE },
	{ 0x108120, 5, RI_ALL_ONLINE }, { 0x108200, 74, RI_ALL_ONLINE },
	{ 0x108400, 74, RI_ALL_ONLINE }, { 0x108800, 152, RI_ALL_ONLINE },
	{ 0x109000, 1, RI_ALL_ONLINE }, { 0x120000, 347, RI_ALL_ONLINE },
	{ 0x120578, 1, RI_ALL_ONLINE }, { 0x120588, 1, RI_ALL_ONLINE },
	{ 0x120598, 1, RI_ALL_ONLINE }, { 0x12059c, 23, RI_E1H_ONLINE },
	{ 0x120614, 1, RI_E1H_ONLINE }, { 0x12061c, 30, RI_E1H_ONLINE },
	{ 0x12080c, 65, RI_ALL_ONLINE }, { 0x120a00, 2, RI_ALL_ONLINE },
	{ 0x122000, 2, RI_ALL_ONLINE }, { 0x128000, 2, RI_E1H_ONLINE },
	{ 0x140000, 114, RI_ALL_ONLINE }, { 0x1401d4, 1, RI_ALL_ONLINE },
	{ 0x1401e4, 1, RI_ALL_ONLINE }, { 0x140200, 6, RI_ALL_ONLINE },
	{ 0x144000, 4, RI_ALL_ONLINE }, { 0x148000, 4, RI_ALL_ONLINE },
	{ 0x14c000, 4, RI_ALL_ONLINE }, { 0x150000, 4, RI_ALL_ONLINE },
	{ 0x154000, 4, RI_ALL_ONLINE }, { 0x158000, 4, RI_ALL_ONLINE },
	{ 0x15c000, 7, RI_E1H_ONLINE }, { 0x161000, 7, RI_ALL_ONLINE },
	{ 0x161028, 1, RI_ALL_ONLINE }, { 0x161038, 1, RI_ALL_ONLINE },
	{ 0x161800, 2, RI_ALL_ONLINE }, { 0x164000, 60, RI_ALL_ONLINE },
	{ 0x1640fc, 1, RI_ALL_ONLINE }, { 0x16410c, 1, RI_ALL_ONLINE },
	{ 0x164110, 2, RI_E1H_ONLINE }, { 0x164200, 1, RI_ALL_ONLINE },
	{ 0x164208, 1, RI_ALL_ONLINE }, { 0x164210, 1, RI_ALL_ONLINE },
	{ 0x164218, 1, RI_ALL_ONLINE }, { 0x164220, 1, RI_ALL_ONLINE },
	{ 0x164228, 1, RI_ALL_ONLINE }, { 0x164230, 1, RI_ALL_ONLINE },
	{ 0x164238, 1, RI_ALL_ONLINE }, { 0x164240, 1, RI_ALL_ONLINE },
	{ 0x164248, 1, RI_ALL_ONLINE }, { 0x164250, 1, RI_ALL_ONLINE },
	{ 0x164258, 1, RI_ALL_ONLINE }, { 0x164260, 1, RI_ALL_ONLINE },
	{ 0x164270, 2, RI_ALL_ONLINE }, { 0x164280, 2, RI_ALL_ONLINE },
	{ 0x164800, 2, RI_ALL_ONLINE }, { 0x165000, 2, RI_ALL_ONLINE },
	{ 0x166000, 164, RI_ALL_ONLINE }, { 0x16629c, 1, RI_ALL_ONLINE },
	{ 0x1662ac, 1, RI_ALL_ONLINE }, { 0x1662bc, 1, RI_ALL_ONLINE },
	{ 0x166400, 49, RI_ALL_ONLINE }, { 0x1664c8, 38, RI_ALL_ONLINE },
	{ 0x166568, 2, RI_ALL_ONLINE }, { 0x166800, 1, RI_ALL_ONLINE },
	{ 0x168000, 270, RI_ALL_ONLINE }, { 0x168444, 1, RI_ALL_ONLINE },
	{ 0x168454, 1, RI_ALL_ONLINE }, { 0x168800, 19, RI_ALL_ONLINE },
	{ 0x168900, 1, RI_ALL_ONLINE }, { 0x168a00, 128, RI_ALL_ONLINE },
	{ 0x16a000, 1, RI_ALL_ONLINE }, { 0x16a004, 1535, RI_ALL_OFFLINE },
	{ 0x16c000, 1, RI_ALL_ONLINE }, { 0x16c004, 1535, RI_ALL_OFFLINE },
	{ 0x16e000, 16, RI_E1H_ONLINE }, { 0x16e100, 1, RI_E1H_ONLINE },
	{ 0x16e200, 2, RI_E1H_ONLINE }, { 0x16e400, 183, RI_E1H_ONLINE },
	{ 0x170000, 93, RI_ALL_ONLINE }, { 0x170180, 1, RI_ALL_ONLINE },
	{ 0x170190, 1, RI_ALL_ONLINE }, { 0x170200, 4, RI_ALL_ONLINE },
	{ 0x170214, 1, RI_ALL_ONLINE }, { 0x178000, 1, RI_ALL_ONLINE },
	{ 0x180000, 61, RI_ALL_ONLINE }, { 0x180100, 1, RI_ALL_ONLINE },
	{ 0x180110, 1, RI_ALL_ONLINE }, { 0x180120, 1, RI_ALL_ONLINE },
	{ 0x180130, 1, RI_ALL_ONLINE }, { 0x18013c, 2, RI_E1H_ONLINE },
	{ 0x180200, 58, RI_ALL_ONLINE }, { 0x180340, 4, RI_ALL_ONLINE },
	{ 0x180400, 1, RI_ALL_ONLINE }, { 0x180404, 255, RI_ALL_OFFLINE },
	{ 0x181000, 4, RI_ALL_ONLINE }, { 0x181010, 1020, RI_ALL_OFFLINE },
	{ 0x1a0000, 1, RI_ALL_ONLINE }, { 0x1a0004, 1023, RI_ALL_OFFLINE },
	{ 0x1a1000, 1, RI_ALL_ONLINE }, { 0x1a1004, 4607, RI_ALL_OFFLINE },
	{ 0x1a5800, 2560, RI_E1H_OFFLINE }, { 0x1a8000, 64, RI_ALL_OFFLINE },
	{ 0x1a8100, 1984, RI_E1H_OFFLINE }, { 0x1aa000, 1, RI_E1H_ONLINE },
	{ 0x1aa004, 6655, RI_E1H_OFFLINE }, { 0x1b1800, 128, RI_ALL_OFFLINE },
	{ 0x1b1c00, 128, RI_ALL_OFFLINE }, { 0x1b2000, 1, RI_ALL_OFFLINE },
	{ 0x1b2400, 64, RI_E1H_OFFLINE }, { 0x1b8200, 1, RI_ALL_ONLINE },
	{ 0x1b8240, 1, RI_ALL_ONLINE }, { 0x1b8280, 1, RI_ALL_ONLINE },
	{ 0x1b82c0, 1, RI_ALL_ONLINE }, { 0x1b8a00, 1, RI_ALL_ONLINE },
	{ 0x1b8a80, 1, RI_ALL_ONLINE }, { 0x1c0000, 2, RI_ALL_ONLINE },
	{ 0x200000, 65, RI_ALL_ONLINE }, { 0x200110, 1, RI_ALL_ONLINE },
	{ 0x200120, 1, RI_ALL_ONLINE }, { 0x200130, 1, RI_ALL_ONLINE },
	{ 0x200140, 1, RI_ALL_ONLINE }, { 0x20014c, 2, RI_E1H_ONLINE },
	{ 0x200200, 58, RI_ALL_ONLINE }, { 0x200340, 4, RI_ALL_ONLINE },
	{ 0x200400, 1, RI_ALL_ONLINE }, { 0x200404, 255, RI_ALL_OFFLINE },
	{ 0x202000, 4, RI_ALL_ONLINE }, { 0x202010, 2044, RI_ALL_OFFLINE },
	{ 0x220000, 1, RI_ALL_ONLINE }, { 0x220004, 1023, RI_ALL_OFFLINE },
	{ 0x221000, 1, RI_ALL_ONLINE }, { 0x221004, 4607, RI_ALL_OFFLINE },
	{ 0x225800, 1536, RI_E1H_OFFLINE }, { 0x227000, 1, RI_E1H_ONLINE },
	{ 0x227004, 1023, RI_E1H_OFFLINE }, { 0x228000, 64, RI_ALL_OFFLINE },
	{ 0x228100, 8640, RI_E1H_OFFLINE }, { 0x231800, 128, RI_ALL_OFFLINE },
	{ 0x231c00, 128, RI_ALL_OFFLINE }, { 0x232000, 1, RI_ALL_OFFLINE },
	{ 0x232400, 64, RI_E1H_OFFLINE }, { 0x238200, 1, RI_ALL_ONLINE },
	{ 0x238240, 1, RI_ALL_ONLINE }, { 0x238280, 1, RI_ALL_ONLINE },
	{ 0x2382c0, 1, RI_ALL_ONLINE }, { 0x238a00, 1, RI_ALL_ONLINE },
	{ 0x238a80, 1, RI_ALL_ONLINE }, { 0x240000, 2, RI_ALL_ONLINE },
	{ 0x280000, 65, RI_ALL_ONLINE }, { 0x280110, 1, RI_ALL_ONLINE },
	{ 0x280120, 1, RI_ALL_ONLINE }, { 0x280130, 1, RI_ALL_ONLINE },
	{ 0x280140, 1, RI_ALL_ONLINE }, { 0x28014c, 2, RI_E1H_ONLINE },
	{ 0x280200, 58, RI_ALL_ONLINE }, { 0x280340, 4, RI_ALL_ONLINE },
	{ 0x280400, 1, RI_ALL_ONLINE }, { 0x280404, 255, RI_ALL_OFFLINE },
	{ 0x282000, 4, RI_ALL_ONLINE }, { 0x282010, 2044, RI_ALL_OFFLINE },
	{ 0x2a0000, 1, RI_ALL_ONLINE }, { 0x2a0004, 1023, RI_ALL_OFFLINE },
	{ 0x2a1000, 1, RI_ALL_ONLINE }, { 0x2a1004, 4607, RI_ALL_OFFLINE },
	{ 0x2a5800, 2560, RI_E1H_OFFLINE }, { 0x2a8000, 64, RI_ALL_OFFLINE },
	{ 0x2a8100, 960, RI_E1H_OFFLINE }, { 0x2a9000, 1, RI_E1H_ONLINE },
	{ 0x2a9004, 7679, RI_E1H_OFFLINE }, { 0x2b1800, 128, RI_ALL_OFFLINE },
	{ 0x2b1c00, 128, RI_ALL_OFFLINE }, { 0x2b2000, 1, RI_ALL_OFFLINE },
	{ 0x2b2400, 64, RI_E1H_OFFLINE }, { 0x2b8200, 1, RI_ALL_ONLINE },
	{ 0x2b8240, 1, RI_ALL_ONLINE }, { 0x2b8280, 1, RI_ALL_ONLINE },
	{ 0x2b82c0, 1, RI_ALL_ONLINE }, { 0x2b8a00, 1, RI_ALL_ONLINE },
	{ 0x2b8a80, 1, RI_ALL_ONLINE }, { 0x2c0000, 2, RI_ALL_ONLINE },
	{ 0x300000, 65, RI_ALL_ONLINE }, { 0x300110, 1, RI_ALL_ONLINE },
	{ 0x300120, 1, RI_ALL_ONLINE }, { 0x300130, 1, RI_ALL_ONLINE },
	{ 0x300140, 1, RI_ALL_ONLINE }, { 0x30014c, 2, RI_E1H_ONLINE },
	{ 0x300200, 58, RI_ALL_ONLINE }, { 0x300340, 4, RI_ALL_ONLINE },
	{ 0x300400, 1, RI_ALL_ONLINE }, { 0x300404, 255, RI_ALL_OFFLINE },
	{ 0x302000, 4, RI_ALL_ONLINE }, { 0x302010, 2044, RI_ALL_OFFLINE },
	{ 0x320000, 1, RI_ALL_ONLINE }, { 0x320004, 1023, RI_ALL_OFFLINE },
	{ 0x321000, 1, RI_ALL_ONLINE }, { 0x321004, 4607, RI_ALL_OFFLINE },
	{ 0x325800, 2560, RI_E1H_OFFLINE }, { 0x328000, 64, RI_ALL_OFFLINE },
	{ 0x328100, 536, RI_E1H_OFFLINE }, { 0x328960, 1, RI_E1H_ONLINE },
	{ 0x328964, 8103, RI_E1H_OFFLINE }, { 0x331800, 128, RI_ALL_OFFLINE },
	{ 0x331c00, 128, RI_ALL_OFFLINE }, { 0x332000, 1, RI_ALL_OFFLINE },
	{ 0x332400, 64, RI_E1H_OFFLINE }, { 0x338200, 1, RI_ALL_ONLINE },
	{ 0x338240, 1, RI_ALL_ONLINE }, { 0x338280, 1, RI_ALL_ONLINE },
	{ 0x3382c0, 1, RI_ALL_ONLINE }, { 0x338a00, 1, RI_ALL_ONLINE },
	{ 0x338a80, 1, RI_ALL_ONLINE }, { 0x340000, 2, RI_ALL_ONLINE }
};


#define IDLE_REGS_COUNT			277
static const struct reg_addr idle_addrs[IDLE_REGS_COUNT] = {
	{ 0x2114, 1, RI_ALL_ONLINE }, { 0x2120, 1, RI_ALL_ONLINE },
	{ 0x212c, 4, RI_ALL_ONLINE }, { 0x2814, 1, RI_ALL_ONLINE },
	{ 0x281c, 2, RI_ALL_ONLINE }, { 0xa38c, 1, RI_ALL_ONLINE },
	{ 0xa408, 1, RI_ALL_ONLINE }, { 0xa42c, 12, RI_ALL_ONLINE },
	{ 0xa600, 5, RI_E1H_ONLINE }, { 0xa618, 1, RI_E1H_ONLINE },
	{ 0xc09c, 1, RI_ALL_ONLINE }, { 0x103b0, 1, RI_ALL_ONLINE },
	{ 0x103c0, 1, RI_ALL_ONLINE }, { 0x103d0, 1, RI_E1H_ONLINE },
	{ 0x2021c, 11, RI_ALL_ONLINE }, { 0x202a8, 1, RI_ALL_ONLINE },
	{ 0x202b8, 1, RI_ALL_ONLINE }, { 0x20404, 1, RI_ALL_ONLINE },
	{ 0x2040c, 2, RI_ALL_ONLINE }, { 0x2041c, 2, RI_ALL_ONLINE },
	{ 0x40154, 14, RI_ALL_ONLINE }, { 0x40198, 1, RI_ALL_ONLINE },
	{ 0x404ac, 1, RI_ALL_ONLINE }, { 0x404bc, 1, RI_ALL_ONLINE },
	{ 0x42290, 1, RI_ALL_ONLINE }, { 0x422a0, 1, RI_ALL_ONLINE },
	{ 0x422b0, 1, RI_ALL_ONLINE }, { 0x42548, 1, RI_ALL_ONLINE },
	{ 0x42550, 1, RI_ALL_ONLINE }, { 0x42558, 1, RI_ALL_ONLINE },
	{ 0x50160, 8, RI_ALL_ONLINE }, { 0x501d0, 1, RI_ALL_ONLINE },
	{ 0x501e0, 1, RI_ALL_ONLINE }, { 0x50204, 1, RI_ALL_ONLINE },
	{ 0x5020c, 2, RI_ALL_ONLINE }, { 0x5021c, 1, RI_ALL_ONLINE },
	{ 0x60090, 1, RI_ALL_ONLINE }, { 0x6011c, 1, RI_ALL_ONLINE },
	{ 0x6012c, 1, RI_ALL_ONLINE }, { 0xc101c, 1, RI_ALL_ONLINE },
	{ 0xc102c, 1, RI_ALL_ONLINE }, { 0xc2290, 1, RI_ALL_ONLINE },
	{ 0xc22a0, 1, RI_ALL_ONLINE }, { 0xc22b0, 1, RI_ALL_ONLINE },
	{ 0xc2548, 1, RI_ALL_ONLINE }, { 0xc2550, 1, RI_ALL_ONLINE },
	{ 0xc2558, 1, RI_ALL_ONLINE }, { 0xc4294, 1, RI_ALL_ONLINE },
	{ 0xc42a4, 1, RI_ALL_ONLINE }, { 0xc42b4, 1, RI_ALL_ONLINE },
	{ 0xc4550, 1, RI_ALL_ONLINE }, { 0xc4558, 1, RI_ALL_ONLINE },
	{ 0xc4560, 1, RI_ALL_ONLINE }, { 0xd016c, 8, RI_ALL_ONLINE },
	{ 0xd01d8, 1, RI_ALL_ONLINE }, { 0xd01e8, 1, RI_ALL_ONLINE },
	{ 0xd0204, 1, RI_ALL_ONLINE }, { 0xd020c, 3, RI_ALL_ONLINE },
	{ 0xe0154, 8, RI_ALL_ONLINE }, { 0xe01c8, 1, RI_ALL_ONLINE },
	{ 0xe01d8, 1, RI_ALL_ONLINE }, { 0xe0204, 1, RI_ALL_ONLINE },
	{ 0xe020c, 2, RI_ALL_ONLINE }, { 0xe021c, 2, RI_ALL_ONLINE },
	{ 0x101014, 1, RI_ALL_ONLINE }, { 0x101030, 1, RI_ALL_ONLINE },
	{ 0x101040, 1, RI_ALL_ONLINE }, { 0x102058, 1, RI_ALL_ONLINE },
	{ 0x102080, 16, RI_ALL_ONLINE }, { 0x103004, 2, RI_ALL_ONLINE },
	{ 0x103068, 1, RI_ALL_ONLINE }, { 0x103078, 1, RI_ALL_ONLINE },
	{ 0x103088, 1, RI_ALL_ONLINE }, { 0x10309c, 2, RI_E1H_ONLINE },
	{ 0x104004, 1, RI_ALL_ONLINE }, { 0x104018, 1, RI_ALL_ONLINE },
	{ 0x104020, 1, RI_ALL_ONLINE }, { 0x10403c, 1, RI_ALL_ONLINE },
	{ 0x1040fc, 1, RI_ALL_ONLINE }, { 0x10410c, 1, RI_ALL_ONLINE },
	{ 0x104400, 64, RI_ALL_ONLINE }, { 0x104800, 64, RI_ALL_ONLINE },
	{ 0x105000, 3, RI_ALL_ONLINE }, { 0x105010, 3, RI_ALL_ONLINE },
	{ 0x105020, 3, RI_ALL_ONLINE }, { 0x105030, 3, RI_ALL_ONLINE },
	{ 0x105040, 3, RI_ALL_ONLINE }, { 0x105050, 3, RI_ALL_ONLINE },
	{ 0x105060, 3, RI_ALL_ONLINE }, { 0x105070, 3, RI_ALL_ONLINE },
	{ 0x105080, 3, RI_ALL_ONLINE }, { 0x105090, 3, RI_ALL_ONLINE },
	{ 0x1050a0, 3, RI_ALL_ONLINE }, { 0x1050b0, 3, RI_ALL_ONLINE },
	{ 0x1050c0, 3, RI_ALL_ONLINE }, { 0x1050d0, 3, RI_ALL_ONLINE },
	{ 0x1050e0, 3, RI_ALL_ONLINE }, { 0x1050f0, 3, RI_ALL_ONLINE },
	{ 0x105100, 3, RI_ALL_ONLINE }, { 0x105110, 3, RI_ALL_ONLINE },
	{ 0x105120, 3, RI_ALL_ONLINE }, { 0x105130, 3, RI_ALL_ONLINE },
	{ 0x105140, 3, RI_ALL_ONLINE }, { 0x105150, 3, RI_ALL_ONLINE },
	{ 0x105160, 3, RI_ALL_ONLINE }, { 0x105170, 3, RI_ALL_ONLINE },
	{ 0x105180, 3, RI_ALL_ONLINE }, { 0x105190, 3, RI_ALL_ONLINE },
	{ 0x1051a0, 3, RI_ALL_ONLINE }, { 0x1051b0, 3, RI_ALL_ONLINE },
	{ 0x1051c0, 3, RI_ALL_ONLINE }, { 0x1051d0, 3, RI_ALL_ONLINE },
	{ 0x1051e0, 3, RI_ALL_ONLINE }, { 0x1051f0, 3, RI_ALL_ONLINE },
	{ 0x105200, 3, RI_ALL_ONLINE }, { 0x105210, 3, RI_ALL_ONLINE },
	{ 0x105220, 3, RI_ALL_ONLINE }, { 0x105230, 3, RI_ALL_ONLINE },
	{ 0x105240, 3, RI_ALL_ONLINE }, { 0x105250, 3, RI_ALL_ONLINE },
	{ 0x105260, 3, RI_ALL_ONLINE }, { 0x105270, 3, RI_ALL_ONLINE },
	{ 0x105280, 3, RI_ALL_ONLINE }, { 0x105290, 3, RI_ALL_ONLINE },
	{ 0x1052a0, 3, RI_ALL_ONLINE }, { 0x1052b0, 3, RI_ALL_ONLINE },
	{ 0x1052c0, 3, RI_ALL_ONLINE }, { 0x1052d0, 3, RI_ALL_ONLINE },
	{ 0x1052e0, 3, RI_ALL_ONLINE }, { 0x1052f0, 3, RI_ALL_ONLINE },
	{ 0x105300, 3, RI_ALL_ONLINE }, { 0x105310, 3, RI_ALL_ONLINE },
	{ 0x105320, 3, RI_ALL_ONLINE }, { 0x105330, 3, RI_ALL_ONLINE },
	{ 0x105340, 3, RI_ALL_ONLINE }, { 0x105350, 3, RI_ALL_ONLINE },
	{ 0x105360, 3, RI_ALL_ONLINE }, { 0x105370, 3, RI_ALL_ONLINE },
	{ 0x105380, 3, RI_ALL_ONLINE }, { 0x105390, 3, RI_ALL_ONLINE },
	{ 0x1053a0, 3, RI_ALL_ONLINE }, { 0x1053b0, 3, RI_ALL_ONLINE },
	{ 0x1053c0, 3, RI_ALL_ONLINE }, { 0x1053d0, 3, RI_ALL_ONLINE },
	{ 0x1053e0, 3, RI_ALL_ONLINE }, { 0x1053f0, 3, RI_ALL_ONLINE },
	{ 0x108094, 1, RI_ALL_ONLINE }, { 0x1201b0, 2, RI_ALL_ONLINE },
	{ 0x12032c, 1, RI_ALL_ONLINE }, { 0x12036c, 3, RI_ALL_ONLINE },
	{ 0x120408, 2, RI_ALL_ONLINE }, { 0x120414, 15, RI_ALL_ONLINE },
	{ 0x120478, 2, RI_ALL_ONLINE }, { 0x12052c, 1, RI_ALL_ONLINE },
	{ 0x120564, 3, RI_ALL_ONLINE }, { 0x12057c, 1, RI_ALL_ONLINE },
	{ 0x12058c, 1, RI_ALL_ONLINE }, { 0x120608, 1, RI_E1H_ONLINE },
	{ 0x120808, 1, RI_E1_ONLINE }, { 0x12080c, 2, RI_ALL_ONLINE },
	{ 0x120818, 1, RI_ALL_ONLINE }, { 0x120820, 1, RI_ALL_ONLINE },
	{ 0x120828, 1, RI_ALL_ONLINE }, { 0x120830, 1, RI_ALL_ONLINE },
	{ 0x120838, 1, RI_ALL_ONLINE }, { 0x120840, 1, RI_ALL_ONLINE },
	{ 0x120848, 1, RI_ALL_ONLINE }, { 0x120850, 1, RI_ALL_ONLINE },
	{ 0x120858, 1, RI_ALL_ONLINE }, { 0x120860, 1, RI_ALL_ONLINE },
	{ 0x120868, 1, RI_ALL_ONLINE }, { 0x120870, 1, RI_ALL_ONLINE },
	{ 0x120878, 1, RI_ALL_ONLINE }, { 0x120880, 1, RI_ALL_ONLINE },
	{ 0x120888, 1, RI_ALL_ONLINE }, { 0x120890, 1, RI_ALL_ONLINE },
	{ 0x120898, 1, RI_ALL_ONLINE }, { 0x1208a0, 1, RI_ALL_ONLINE },
	{ 0x1208a8, 1, RI_ALL_ONLINE }, { 0x1208b0, 1, RI_ALL_ONLINE },
	{ 0x1208b8, 1, RI_ALL_ONLINE }, { 0x1208c0, 1, RI_ALL_ONLINE },
	{ 0x1208c8, 1, RI_ALL_ONLINE }, { 0x1208d0, 1, RI_ALL_ONLINE },
	{ 0x1208d8, 1, RI_ALL_ONLINE }, { 0x1208e0, 1, RI_ALL_ONLINE },
	{ 0x1208e8, 1, RI_ALL_ONLINE }, { 0x1208f0, 1, RI_ALL_ONLINE },
	{ 0x1208f8, 1, RI_ALL_ONLINE }, { 0x120900, 1, RI_ALL_ONLINE },
	{ 0x120908, 1, RI_ALL_ONLINE }, { 0x14005c, 2, RI_ALL_ONLINE },
	{ 0x1400d0, 2, RI_ALL_ONLINE }, { 0x1400e0, 1, RI_ALL_ONLINE },
	{ 0x1401c8, 1, RI_ALL_ONLINE }, { 0x140200, 6, RI_ALL_ONLINE },
	{ 0x16101c, 1, RI_ALL_ONLINE }, { 0x16102c, 1, RI_ALL_ONLINE },
	{ 0x164014, 2, RI_ALL_ONLINE }, { 0x1640f0, 1, RI_ALL_ONLINE },
	{ 0x166290, 1, RI_ALL_ONLINE }, { 0x1662a0, 1, RI_ALL_ONLINE },
	{ 0x1662b0, 1, RI_ALL_ONLINE }, { 0x166548, 1, RI_ALL_ONLINE },
	{ 0x166550, 1, RI_ALL_ONLINE }, { 0x166558, 1, RI_ALL_ONLINE },
	{ 0x168000, 1, RI_ALL_ONLINE }, { 0x168008, 1, RI_ALL_ONLINE },
	{ 0x168010, 1, RI_ALL_ONLINE }, { 0x168018, 1, RI_ALL_ONLINE },
	{ 0x168028, 2, RI_ALL_ONLINE }, { 0x168058, 4, RI_ALL_ONLINE },
	{ 0x168070, 1, RI_ALL_ONLINE }, { 0x168238, 1, RI_ALL_ONLINE },
	{ 0x1682d0, 2, RI_ALL_ONLINE }, { 0x1682e0, 1, RI_ALL_ONLINE },
	{ 0x168300, 67, RI_ALL_ONLINE }, { 0x168410, 2, RI_ALL_ONLINE },
	{ 0x168438, 1, RI_ALL_ONLINE }, { 0x168448, 1, RI_ALL_ONLINE },
	{ 0x168a00, 128, RI_ALL_ONLINE }, { 0x16e200, 128, RI_E1H_ONLINE },
	{ 0x16e404, 2, RI_E1H_ONLINE }, { 0x16e584, 70, RI_E1H_ONLINE },
	{ 0x1700a4, 1, RI_ALL_ONLINE }, { 0x1700ac, 2, RI_ALL_ONLINE },
	{ 0x1700c0, 1, RI_ALL_ONLINE }, { 0x170174, 1, RI_ALL_ONLINE },
	{ 0x170184, 1, RI_ALL_ONLINE }, { 0x1800f4, 1, RI_ALL_ONLINE },
	{ 0x180104, 1, RI_ALL_ONLINE }, { 0x180114, 1, RI_ALL_ONLINE },
	{ 0x180124, 1, RI_ALL_ONLINE }, { 0x18026c, 1, RI_ALL_ONLINE },
	{ 0x1802a0, 1, RI_ALL_ONLINE }, { 0x1a1000, 1, RI_ALL_ONLINE },
	{ 0x1aa000, 1, RI_E1H_ONLINE }, { 0x1b8000, 1, RI_ALL_ONLINE },
	{ 0x1b8040, 1, RI_ALL_ONLINE }, { 0x1b8080, 1, RI_ALL_ONLINE },
	{ 0x1b80c0, 1, RI_ALL_ONLINE }, { 0x200104, 1, RI_ALL_ONLINE },
	{ 0x200114, 1, RI_ALL_ONLINE }, { 0x200124, 1, RI_ALL_ONLINE },
	{ 0x200134, 1, RI_ALL_ONLINE }, { 0x20026c, 1, RI_ALL_ONLINE },
	{ 0x2002a0, 1, RI_ALL_ONLINE }, { 0x221000, 1, RI_ALL_ONLINE },
	{ 0x227000, 1, RI_E1H_ONLINE }, { 0x238000, 1, RI_ALL_ONLINE },
	{ 0x238040, 1, RI_ALL_ONLINE }, { 0x238080, 1, RI_ALL_ONLINE },
	{ 0x2380c0, 1, RI_ALL_ONLINE }, { 0x280104, 1, RI_ALL_ONLINE },
	{ 0x280114, 1, RI_ALL_ONLINE }, { 0x280124, 1, RI_ALL_ONLINE },
	{ 0x280134, 1, RI_ALL_ONLINE }, { 0x28026c, 1, RI_ALL_ONLINE },
	{ 0x2802a0, 1, RI_ALL_ONLINE }, { 0x2a1000, 1, RI_ALL_ONLINE },
	{ 0x2a9000, 1, RI_E1H_ONLINE }, { 0x2b8000, 1, RI_ALL_ONLINE },
	{ 0x2b8040, 1, RI_ALL_ONLINE }, { 0x2b8080, 1, RI_ALL_ONLINE },
	{ 0x2b80c0, 1, RI_ALL_ONLINE }, { 0x300104, 1, RI_ALL_ONLINE },
	{ 0x300114, 1, RI_ALL_ONLINE }, { 0x300124, 1, RI_ALL_ONLINE },
	{ 0x300134, 1, RI_ALL_ONLINE }, { 0x30026c, 1, RI_ALL_ONLINE },
	{ 0x3002a0, 1, RI_ALL_ONLINE }, { 0x321000, 1, RI_ALL_ONLINE },
	{ 0x328960, 1, RI_E1H_ONLINE }, { 0x338000, 1, RI_ALL_ONLINE },
	{ 0x338040, 1, RI_ALL_ONLINE }, { 0x338080, 1, RI_ALL_ONLINE },
	{ 0x3380c0, 1, RI_ALL_ONLINE }
};

#define WREGS_COUNT_E1			1
static const u32 read_reg_e1_0[] = { 0x1b1000 };

static const struct wreg_addr wreg_addrs_e1[WREGS_COUNT_E1] = {
	{ 0x1b0c00, 192, 1, read_reg_e1_0, RI_E1_OFFLINE }
};


#define WREGS_COUNT_E1H			1
static const u32 read_reg_e1h_0[] = { 0x1b1040, 0x1b1000 };

static const struct wreg_addr wreg_addrs_e1h[WREGS_COUNT_E1H] = {
	{ 0x1b0c00, 256, 2, read_reg_e1h_0, RI_E1H_OFFLINE }
};


static const struct dump_sign dump_sign_all = { 0x49aa93ee, 0x40835, 0x22 };


#define TIMER_REGS_COUNT_E1		2
static const u32 timer_status_regs_e1[TIMER_REGS_COUNT_E1] =
	{ 0x164014, 0x164018 };
static const u32 timer_scan_regs_e1[TIMER_REGS_COUNT_E1] =
	{ 0x1640d0, 0x1640d4 };


#define TIMER_REGS_COUNT_E1H		2
static const u32 timer_status_regs_e1h[TIMER_REGS_COUNT_E1H] =
	{ 0x164014, 0x164018 };
static const u32 timer_scan_regs_e1h[TIMER_REGS_COUNT_E1H] =
	{ 0x1640d0, 0x1640d4 };


#endif 
