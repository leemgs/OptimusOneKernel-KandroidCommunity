

#include <linux/kernel.h>

#include "zd_rf.h"
#include "zd_usb.h"
#include "zd_chip.h"

static const u32 chan_rv[][2] = {
	RF_CHANNEL( 1) = { 0x09ec00, 0x8cccc8 },
	RF_CHANNEL( 2) = { 0x09ec00, 0x8cccd8 },
	RF_CHANNEL( 3) = { 0x09ec00, 0x8cccc0 },
	RF_CHANNEL( 4) = { 0x09ec00, 0x8cccd0 },
	RF_CHANNEL( 5) = { 0x05ec00, 0x8cccc8 },
	RF_CHANNEL( 6) = { 0x05ec00, 0x8cccd8 },
	RF_CHANNEL( 7) = { 0x05ec00, 0x8cccc0 },
	RF_CHANNEL( 8) = { 0x05ec00, 0x8cccd0 },
	RF_CHANNEL( 9) = { 0x0dec00, 0x8cccc8 },
	RF_CHANNEL(10) = { 0x0dec00, 0x8cccd8 },
	RF_CHANNEL(11) = { 0x0dec00, 0x8cccc0 },
	RF_CHANNEL(12) = { 0x0dec00, 0x8cccd0 },
	RF_CHANNEL(13) = { 0x03ec00, 0x8cccc8 },
	RF_CHANNEL(14) = { 0x03ec00, 0x866660 },
};

static const u32 std_rv[] = {
	0x4ff821,
	0xc5fbfc,
	0x21ebfe,
	0xafd401, 
	0x6cf56a,
	0xe04073,
	0x193d76,
	0x9dd844,
	0x500007,
	0xd8c010,
};

static const u32 rv_init1[] = {
	0x3c9000,
	0xbfffff,
	0x700000,
	0xf15d58,
};

static const u32 rv_init2[] = {
	0xf15d59,
	0xf15d5c,
	0xf15d58,
};

static const struct zd_ioreq16 ioreqs_sw[] = {
	{ CR128, 0x14 }, { CR129, 0x12 }, { CR130, 0x10 },
	{ CR38,  0x38 }, { CR136, 0xdf },
};

static int zd1211b_al7230b_finalize(struct zd_chip *chip)
{
	int r;
	static const struct zd_ioreq16 ioreqs[] = {
		{ CR80,  0x30 }, { CR81,  0x30 }, { CR79,  0x58 },
		{ CR12,  0xf0 }, { CR77,  0x1b }, { CR78,  0x58 },
		{ CR203, 0x04 },
		{ },
		{ CR240, 0x80 },
	};

	r = zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
	if (r)
		return r;

	if (chip->new_phy_layout) {
		
		r = zd_iowrite16_locked(chip, 0xe5, CR9);
		if (r)
			return r;
	}

	return zd_iowrite16_locked(chip, 0x04, CR203);
}

static int zd1211_al7230b_init_hw(struct zd_rf *rf)
{
	int r;
	struct zd_chip *chip = zd_rf_to_chip(rf);

	
	static const struct zd_ioreq16 ioreqs_1[] = {
		
		{ CR240,  0x57 },
		{ },

		{ CR15,   0x20 }, { CR23,   0x40 }, { CR24,  0x20 },
		{ CR26,   0x11 }, { CR28,   0x3e }, { CR29,  0x00 },
		{ CR44,   0x33 },
		
		{ CR106,  0x22 },
		{ CR107,  0x1a }, { CR109,  0x09 }, { CR110,  0x27 },
		{ CR111,  0x2b }, { CR112,  0x2b }, { CR119,  0x0a },
		
		{ CR122,  0xfc },
		{ CR10,   0x89 },
		
		{ CR17,   0x28 },
		{ CR26,   0x93 }, { CR34,   0x30 },
		
		{ CR35,   0x3e },
		{ CR41,   0x24 }, { CR44,   0x32 },
		
		{ CR46,   0x96 },
		{ CR47,   0x1e }, { CR79,   0x58 }, { CR80,  0x30 },
		{ CR81,   0x30 }, { CR87,   0x0a }, { CR89,  0x04 },
		{ CR92,   0x0a }, { CR99,   0x28 },
		
		{ CR100,  0x02 },
		{ CR101,  0x13 }, { CR102,  0x27 },
		
		{ CR106,  0x22 },
		
		{ CR107,  0x3f },
		{ CR109,  0x09 },
		
		{ CR110,  0x1f },
		{ CR111,  0x1f }, { CR112,  0x1f }, { CR113, 0x27 },
		{ CR114,  0x27 },
		
		{ CR115,  0x24 },
		
		{ CR116,  0x3f },
		
		{ CR117,  0xfa },
		{ CR118,  0xfc }, { CR119,  0x10 }, { CR120, 0x4f },
		{ CR121,  0x77 }, { CR137,  0x88 },
		
		{ CR138,  0xa8 },
		
		{ CR252,  0x34 },
		
		{ CR253,  0x34 },

		
		{ CR251, 0x2f },
	};

	static const struct zd_ioreq16 ioreqs_2[] = {
		{ CR251, 0x3f }, 
		{ CR128, 0x14 }, { CR129, 0x12 }, { CR130, 0x10 },
		{ CR38,  0x38 }, { CR136, 0xdf },
	};

	r = zd_iowrite16a_locked(chip, ioreqs_1, ARRAY_SIZE(ioreqs_1));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, chan_rv[0], ARRAY_SIZE(chan_rv[0]));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, std_rv, ARRAY_SIZE(std_rv));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, rv_init1, ARRAY_SIZE(rv_init1));
	if (r)
		return r;

	r = zd_iowrite16a_locked(chip, ioreqs_2, ARRAY_SIZE(ioreqs_2));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, rv_init2, ARRAY_SIZE(rv_init2));
	if (r)
		return r;

	r = zd_iowrite16_locked(chip, 0x06, CR203);
	if (r)
		return r;
	r = zd_iowrite16_locked(chip, 0x80, CR240);
	if (r)
		return r;

	return 0;
}

static int zd1211b_al7230b_init_hw(struct zd_rf *rf)
{
	int r;
	struct zd_chip *chip = zd_rf_to_chip(rf);

	static const struct zd_ioreq16 ioreqs_1[] = {
		{ CR240, 0x57 }, { CR9,   0x9 },
		{ },
		{ CR10,  0x8b }, { CR15,  0x20 },
		{ CR17,  0x2B }, 
		{ CR20,  0x10 }, 
		{ CR23,  0x40 }, { CR24,  0x20 }, { CR26,  0x93 },
		{ CR28,  0x3e }, { CR29,  0x00 },
		{ CR33,  0x28 }, 
		{ CR34,  0x30 },
		{ CR35,  0x3e }, 
		{ CR41,  0x24 }, { CR44,  0x32 },
		{ CR46,  0x99 }, 
		{ CR47,  0x1e },

		
		{ CR48,  0x00 }, { CR49,  0x00 }, { CR51,  0x01 },
		{ CR52,  0x80 }, { CR53,  0x7e }, { CR65,  0x00 },
		{ CR66,  0x00 }, { CR67,  0x00 }, { CR68,  0x00 },
		{ CR69,  0x28 },

		{ CR79,  0x58 }, { CR80,  0x30 }, { CR81,  0x30 },
		{ CR87,  0x0A }, { CR89,  0x04 },
		{ CR90,  0x58 }, 
		{ CR91,  0x00 }, 
		{ CR92,  0x0a },
		{ CR98,  0x8d }, 
		{ CR99,  0x00 }, { CR100, 0x02 }, { CR101, 0x13 },
		{ CR102, 0x27 },
		{ CR106, 0x20 }, 
		{ CR109, 0x13 }, 
		{ CR112, 0x1f },
	};

	static const struct zd_ioreq16 ioreqs_new_phy[] = {
		{ CR107, 0x28 },
		{ CR110, 0x1f }, 
		{ CR111, 0x1f }, 
		{ CR116, 0x2a }, { CR118, 0xfa }, { CR119, 0x12 },
		{ CR121, 0x6c }, 
	};

	static const struct zd_ioreq16 ioreqs_old_phy[] = {
		{ CR107, 0x24 },
		{ CR110, 0x13 }, 
		{ CR111, 0x13 }, 
		{ CR116, 0x24 }, { CR118, 0xfc }, { CR119, 0x11 },
		{ CR121, 0x6a }, 
	};

	static const struct zd_ioreq16 ioreqs_2[] = {
		{ CR113, 0x27 }, { CR114, 0x27 }, { CR115, 0x24 },
		{ CR117, 0xfa }, { CR120, 0x4f },
		{ CR122, 0xfc }, 
		{ CR123, 0x57 }, 
		{ CR125, 0xad }, 
		{ CR126, 0x6c }, 
		{ CR127, 0x03 }, 
		{ CR130, 0x10 },
		{ CR131, 0x00 }, 
		{ CR137, 0x50 }, 
		{ CR138, 0xa8 }, 
		{ CR144, 0xac }, 
		{ CR148, 0x40 }, 
		{ CR149, 0x40 }, 
		{ CR150, 0x1a }, 
		{ CR252, 0x34 }, { CR253, 0x34 },
		{ CR251, 0x2f }, 
	};

	static const struct zd_ioreq16 ioreqs_3[] = {
		{ CR251, 0x7f }, 
		{ CR128, 0x14 }, { CR129, 0x12 }, { CR130, 0x10 },
		{ CR38,  0x38 }, { CR136, 0xdf },
	};

	r = zd_iowrite16a_locked(chip, ioreqs_1, ARRAY_SIZE(ioreqs_1));
	if (r)
		return r;

	if (chip->new_phy_layout)
		r = zd_iowrite16a_locked(chip, ioreqs_new_phy,
			ARRAY_SIZE(ioreqs_new_phy));
	else
		r = zd_iowrite16a_locked(chip, ioreqs_old_phy,
			ARRAY_SIZE(ioreqs_old_phy));
	if (r)
		return r;

	r = zd_iowrite16a_locked(chip, ioreqs_2, ARRAY_SIZE(ioreqs_2));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, chan_rv[0], ARRAY_SIZE(chan_rv[0]));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, std_rv, ARRAY_SIZE(std_rv));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, rv_init1, ARRAY_SIZE(rv_init1));
	if (r)
		return r;

	r = zd_iowrite16a_locked(chip, ioreqs_3, ARRAY_SIZE(ioreqs_3));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, rv_init2, ARRAY_SIZE(rv_init2));
	if (r)
		return r;

	return zd1211b_al7230b_finalize(chip);
}

static int zd1211_al7230b_set_channel(struct zd_rf *rf, u8 channel)
{
	int r;
	const u32 *rv = chan_rv[channel-1];
	struct zd_chip *chip = zd_rf_to_chip(rf);

	static const struct zd_ioreq16 ioreqs[] = {
		
		{ CR251, 0x3f },
		{ CR203, 0x06 }, { CR240, 0x08 },
	};

	r = zd_iowrite16_locked(chip, 0x57, CR240);
	if (r)
		return r;

	
	r = zd_iowrite16_locked(chip, 0x2f, CR251);
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, std_rv, ARRAY_SIZE(std_rv));
	if (r)
		return r;

	r = zd_rfwrite_cr_locked(chip, 0x3c9000);
	if (r)
		return r;
	r = zd_rfwrite_cr_locked(chip, 0xf15d58);
	if (r)
		return r;

	r = zd_iowrite16a_locked(chip, ioreqs_sw, ARRAY_SIZE(ioreqs_sw));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, rv, 2);
	if (r)
		return r;

	r = zd_rfwrite_cr_locked(chip, 0x3c9000);
	if (r)
		return r;

	return zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
}

static int zd1211b_al7230b_set_channel(struct zd_rf *rf, u8 channel)
{
	int r;
	const u32 *rv = chan_rv[channel-1];
	struct zd_chip *chip = zd_rf_to_chip(rf);

	r = zd_iowrite16_locked(chip, 0x57, CR240);
	if (r)
		return r;
	r = zd_iowrite16_locked(chip, 0xe4, CR9);
	if (r)
		return r;

	
	r = zd_iowrite16_locked(chip, 0x2f, CR251);
	if (r)
		return r;
	r = zd_rfwritev_cr_locked(chip, std_rv, ARRAY_SIZE(std_rv));
	if (r)
		return r;

	r = zd_rfwrite_cr_locked(chip, 0x3c9000);
	if (r)
		return r;
	r = zd_rfwrite_cr_locked(chip, 0xf15d58);
	if (r)
		return r;

	r = zd_iowrite16a_locked(chip, ioreqs_sw, ARRAY_SIZE(ioreqs_sw));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, rv, 2);
	if (r)
		return r;

	r = zd_rfwrite_cr_locked(chip, 0x3c9000);
	if (r)
		return r;

	r = zd_iowrite16_locked(chip, 0x7f, CR251);
	if (r)
		return r;

	return zd1211b_al7230b_finalize(chip);
}

static int zd1211_al7230b_switch_radio_on(struct zd_rf *rf)
{
	struct zd_chip *chip = zd_rf_to_chip(rf);
	static const struct zd_ioreq16 ioreqs[] = {
		{ CR11,  0x00 },
		{ CR251, 0x3f },
	};

	return zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
}

static int zd1211b_al7230b_switch_radio_on(struct zd_rf *rf)
{
	struct zd_chip *chip = zd_rf_to_chip(rf);
	static const struct zd_ioreq16 ioreqs[] = {
		{ CR11,  0x00 },
		{ CR251, 0x7f },
	};

	return zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
}

static int al7230b_switch_radio_off(struct zd_rf *rf)
{
	struct zd_chip *chip = zd_rf_to_chip(rf);
	static const struct zd_ioreq16 ioreqs[] = {
		{ CR11,  0x04 },
		{ CR251, 0x2f },
	};

	return zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
}


static int zd1211b_al7230b_patch_6m(struct zd_rf *rf, u8 channel)
{
	struct zd_chip *chip = zd_rf_to_chip(rf);
	struct zd_ioreq16 ioreqs[] = {
		{ CR128, 0x14 }, { CR129, 0x12 },
	};

	
	if (channel == 1) {
		ioreqs[0].value = 0x0e;
		ioreqs[1].value = 0x10;
	} else if (channel == 11) {
		ioreqs[0].value = 0x10;
		ioreqs[1].value = 0x10;
	}

	dev_dbg_f(zd_chip_dev(chip), "patching for channel %d\n", channel);
	return zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
}

int zd_rf_init_al7230b(struct zd_rf *rf)
{
	struct zd_chip *chip = zd_rf_to_chip(rf);

	if (zd_chip_is_zd1211b(chip)) {
		rf->init_hw = zd1211b_al7230b_init_hw;
		rf->switch_radio_on = zd1211b_al7230b_switch_radio_on;
		rf->set_channel = zd1211b_al7230b_set_channel;
		rf->patch_6m_band_edge = zd1211b_al7230b_patch_6m;
	} else {
		rf->init_hw = zd1211_al7230b_init_hw;
		rf->switch_radio_on = zd1211_al7230b_switch_radio_on;
		rf->set_channel = zd1211_al7230b_set_channel;
		rf->patch_6m_band_edge = zd_rf_generic_patch_6m;
		rf->patch_cck_gain = 1;
	}

	rf->switch_radio_off = al7230b_switch_radio_off;

	return 0;
}
