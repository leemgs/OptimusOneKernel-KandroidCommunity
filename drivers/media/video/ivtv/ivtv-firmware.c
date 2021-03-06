

#include "ivtv-driver.h"
#include "ivtv-mailbox.h"
#include "ivtv-firmware.h"
#include "ivtv-yuv.h"
#include <linux/firmware.h>

#define IVTV_MASK_SPU_ENABLE 		0xFFFFFFFE
#define IVTV_MASK_VPU_ENABLE15 		0xFFFFFFF6
#define IVTV_MASK_VPU_ENABLE16 		0xFFFFFFFB
#define IVTV_CMD_VDM_STOP 		0x00000000
#define IVTV_CMD_AO_STOP 		0x00000005
#define IVTV_CMD_APU_PING 		0x00000000
#define IVTV_CMD_VPU_STOP15 		0xFFFFFFFE
#define IVTV_CMD_VPU_STOP16 		0xFFFFFFEE
#define IVTV_CMD_HW_BLOCKS_RST 		0xFFFFFFFF
#define IVTV_CMD_SPU_STOP 		0x00000001
#define IVTV_CMD_SDRAM_PRECHARGE_INIT 	0x0000001A
#define IVTV_CMD_SDRAM_REFRESH_INIT 	0x80000640
#define IVTV_SDRAM_SLEEPTIME 		600

#define IVTV_DECODE_INIT_MPEG_FILENAME 	"v4l-cx2341x-init.mpg"
#define IVTV_DECODE_INIT_MPEG_SIZE 	(152*1024)


#define IVTV_FW_ENC_SIZE 		(376836)
#define IVTV_FW_DEC_SIZE 		(256*1024)

static int load_fw_direct(const char *fn, volatile u8 __iomem *mem, struct ivtv *itv, long size)
{
	const struct firmware *fw = NULL;
	int retries = 3;

retry:
	if (retries && request_firmware(&fw, fn, &itv->pdev->dev) == 0) {
		int i;
		volatile u32 __iomem *dst = (volatile u32 __iomem *)mem;
		const u32 *src = (const u32 *)fw->data;

		if (fw->size != size) {
			
			IVTV_INFO("Retry: file loaded was not %s (expected size %ld, got %zd)\n", fn, size, fw->size);
			release_firmware(fw);
			retries--;
			goto retry;
		}
		for (i = 0; i < fw->size; i += 4) {
			
			__raw_writel(*src, dst);
			dst++;
			src++;
		}
		IVTV_INFO("Loaded %s firmware (%zd bytes)\n", fn, fw->size);
		release_firmware(fw);
		return size;
	}
	IVTV_ERR("Unable to open firmware %s (must be %ld bytes)\n", fn, size);
	IVTV_ERR("Did you put the firmware in the hotplug firmware directory?\n");
	return -ENOMEM;
}

void ivtv_halt_firmware(struct ivtv *itv)
{
	IVTV_DEBUG_INFO("Preparing for firmware halt.\n");
	if (itv->has_cx23415 && itv->dec_mbox.mbox)
		ivtv_vapi(itv, CX2341X_DEC_HALT_FW, 0);
	if (itv->enc_mbox.mbox)
		ivtv_vapi(itv, CX2341X_ENC_HALT_FW, 0);

	ivtv_msleep_timeout(10, 0);
	itv->enc_mbox.mbox = itv->dec_mbox.mbox = NULL;

	IVTV_DEBUG_INFO("Stopping VDM\n");
	write_reg(IVTV_CMD_VDM_STOP, IVTV_REG_VDM);

	IVTV_DEBUG_INFO("Stopping AO\n");
	write_reg(IVTV_CMD_AO_STOP, IVTV_REG_AO);

	IVTV_DEBUG_INFO("pinging (?) APU\n");
	write_reg(IVTV_CMD_APU_PING, IVTV_REG_APU);

	IVTV_DEBUG_INFO("Stopping VPU\n");
	if (!itv->has_cx23415)
		write_reg(IVTV_CMD_VPU_STOP16, IVTV_REG_VPU);
	else
		write_reg(IVTV_CMD_VPU_STOP15, IVTV_REG_VPU);

	IVTV_DEBUG_INFO("Resetting Hw Blocks\n");
	write_reg(IVTV_CMD_HW_BLOCKS_RST, IVTV_REG_HW_BLOCKS);

	IVTV_DEBUG_INFO("Stopping SPU\n");
	write_reg(IVTV_CMD_SPU_STOP, IVTV_REG_SPU);

	ivtv_msleep_timeout(10, 0);

	IVTV_DEBUG_INFO("init Encoder SDRAM pre-charge\n");
	write_reg(IVTV_CMD_SDRAM_PRECHARGE_INIT, IVTV_REG_ENC_SDRAM_PRECHARGE);

	IVTV_DEBUG_INFO("init Encoder SDRAM refresh to 1us\n");
	write_reg(IVTV_CMD_SDRAM_REFRESH_INIT, IVTV_REG_ENC_SDRAM_REFRESH);

	if (itv->has_cx23415) {
		IVTV_DEBUG_INFO("init Decoder SDRAM pre-charge\n");
		write_reg(IVTV_CMD_SDRAM_PRECHARGE_INIT, IVTV_REG_DEC_SDRAM_PRECHARGE);

		IVTV_DEBUG_INFO("init Decoder SDRAM refresh to 1us\n");
		write_reg(IVTV_CMD_SDRAM_REFRESH_INIT, IVTV_REG_DEC_SDRAM_REFRESH);
	}

	IVTV_DEBUG_INFO("Sleeping for %dms\n", IVTV_SDRAM_SLEEPTIME);
	ivtv_msleep_timeout(IVTV_SDRAM_SLEEPTIME, 0);
}

void ivtv_firmware_versions(struct ivtv *itv)
{
	u32 data[CX2341X_MBOX_MAX_DATA];

	
	ivtv_vapi_result(itv, data, CX2341X_ENC_GET_VERSION, 0);
	IVTV_INFO("Encoder revision: 0x%08x\n", data[0]);

	if (data[0] != 0x02060039)
		IVTV_WARN("Recommended firmware version is 0x02060039.\n");

	if (itv->has_cx23415) {
		
		ivtv_vapi_result(itv, data, CX2341X_DEC_GET_VERSION, 0);
		IVTV_INFO("Decoder revision: 0x%08x\n", data[0]);
	}
}

static int ivtv_firmware_copy(struct ivtv *itv)
{
	IVTV_DEBUG_INFO("Loading encoder image\n");
	if (load_fw_direct(CX2341X_FIRM_ENC_FILENAME,
		   itv->enc_mem, itv, IVTV_FW_ENC_SIZE) != IVTV_FW_ENC_SIZE) {
		IVTV_DEBUG_WARN("failed loading encoder firmware\n");
		return -3;
	}
	if (!itv->has_cx23415)
		return 0;

	IVTV_DEBUG_INFO("Loading decoder image\n");
	if (load_fw_direct(CX2341X_FIRM_DEC_FILENAME,
		   itv->dec_mem, itv, IVTV_FW_DEC_SIZE) != IVTV_FW_DEC_SIZE) {
		IVTV_DEBUG_WARN("failed loading decoder firmware\n");
		return -1;
	}
	return 0;
}

static volatile struct ivtv_mailbox __iomem *ivtv_search_mailbox(const volatile u8 __iomem *mem, u32 size)
{
	int i;

	
	for (i = 0; i < size; i += 0x100) {
		if (readl(mem + i)      == 0x12345678 &&
		    readl(mem + i + 4)  == 0x34567812 &&
		    readl(mem + i + 8)  == 0x56781234 &&
		    readl(mem + i + 12) == 0x78123456) {
			return (volatile struct ivtv_mailbox __iomem *)(mem + i + 16);
		}
	}
	return NULL;
}

int ivtv_firmware_init(struct ivtv *itv)
{
	int err;

	ivtv_halt_firmware(itv);

	
	err = ivtv_firmware_copy(itv);
	if (err) {
		IVTV_DEBUG_WARN("Error %d loading firmware\n", err);
		return err;
	}

	
	write_reg(read_reg(IVTV_REG_SPU) & IVTV_MASK_SPU_ENABLE, IVTV_REG_SPU);
	ivtv_msleep_timeout(100, 0);
	if (itv->has_cx23415)
		write_reg(read_reg(IVTV_REG_VPU) & IVTV_MASK_VPU_ENABLE15, IVTV_REG_VPU);
	else
		write_reg(read_reg(IVTV_REG_VPU) & IVTV_MASK_VPU_ENABLE16, IVTV_REG_VPU);
	ivtv_msleep_timeout(100, 0);

	
	itv->enc_mbox.mbox = ivtv_search_mailbox(itv->enc_mem, IVTV_ENCODER_SIZE);
	if (itv->enc_mbox.mbox == NULL)
		IVTV_ERR("Encoder mailbox not found\n");
	else if (ivtv_vapi(itv, CX2341X_ENC_PING_FW, 0)) {
		IVTV_ERR("Encoder firmware dead!\n");
		itv->enc_mbox.mbox = NULL;
	}
	if (itv->enc_mbox.mbox == NULL)
		return -ENODEV;

	if (!itv->has_cx23415)
		return 0;

	itv->dec_mbox.mbox = ivtv_search_mailbox(itv->dec_mem, IVTV_DECODER_SIZE);
	if (itv->dec_mbox.mbox == NULL) {
		IVTV_ERR("Decoder mailbox not found\n");
	} else if (itv->has_cx23415 && ivtv_vapi(itv, CX2341X_DEC_PING_FW, 0)) {
		IVTV_ERR("Decoder firmware dead!\n");
		itv->dec_mbox.mbox = NULL;
	} else {
		
		ivtv_yuv_filter_check(itv);
	}
	return itv->dec_mbox.mbox ? 0 : -ENODEV;
}

void ivtv_init_mpeg_decoder(struct ivtv *itv)
{
	u32 data[CX2341X_MBOX_MAX_DATA];
	long readbytes;
	volatile u8 __iomem *mem_offset;

	data[0] = 0;
	data[1] = itv->params.width;	
	data[2] = itv->params.height;
	data[3] = itv->params.audio_properties;	
	if (ivtv_api(itv, CX2341X_DEC_SET_DECODER_SOURCE, 4, data)) {
		IVTV_ERR("ivtv_init_mpeg_decoder failed to set decoder source\n");
		return;
	}

	if (ivtv_vapi(itv, CX2341X_DEC_START_PLAYBACK, 2, 0, 1) != 0) {
		IVTV_ERR("ivtv_init_mpeg_decoder failed to start playback\n");
		return;
	}
	ivtv_api_get_data(&itv->dec_mbox, IVTV_MBOX_DMA, data);
	mem_offset = itv->dec_mem + data[1];

	if ((readbytes = load_fw_direct(IVTV_DECODE_INIT_MPEG_FILENAME,
		mem_offset, itv, IVTV_DECODE_INIT_MPEG_SIZE)) <= 0) {
		IVTV_DEBUG_WARN("failed to read mpeg decoder initialisation file %s\n",
				IVTV_DECODE_INIT_MPEG_FILENAME);
	} else {
		ivtv_vapi(itv, CX2341X_DEC_SCHED_DMA_FROM_HOST, 3, 0, readbytes, 0);
		ivtv_msleep_timeout(100, 0);
	}
	ivtv_vapi(itv, CX2341X_DEC_STOP_PLAYBACK, 4, 0, 0, 0, 1);
}
