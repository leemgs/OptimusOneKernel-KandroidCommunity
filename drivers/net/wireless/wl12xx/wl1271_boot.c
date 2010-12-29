

#include <linux/gpio.h>

#include "wl1271_acx.h"
#include "wl1271_reg.h"
#include "wl1271_boot.h"
#include "wl1271_spi.h"
#include "wl1271_event.h"

static struct wl1271_partition_set part_table[PART_TABLE_LEN] = {
	[PART_DOWN] = {
		.mem = {
			.start = 0x00000000,
			.size  = 0x000177c0
		},
		.reg = {
			.start = REGISTERS_BASE,
			.size  = 0x00008800
		},
	},

	[PART_WORK] = {
		.mem = {
			.start = 0x00040000,
			.size  = 0x00014fc0
		},
		.reg = {
			.start = REGISTERS_BASE,
			.size  = 0x0000b000
		},
	},

	[PART_DRPW] = {
		.mem = {
			.start = 0x00040000,
			.size  = 0x00014fc0
		},
		.reg = {
			.start = DRPW_BASE,
			.size  = 0x00006000
		}
	}
};

static void wl1271_boot_set_ecpu_ctrl(struct wl1271 *wl, u32 flag)
{
	u32 cpu_ctrl;

	
	cpu_ctrl = wl1271_reg_read32(wl, ACX_REG_ECPU_CONTROL);

	
	cpu_ctrl |= flag;
	wl1271_reg_write32(wl, ACX_REG_ECPU_CONTROL, cpu_ctrl);
}

static void wl1271_boot_fw_version(struct wl1271 *wl)
{
	struct wl1271_static_data static_data;

	wl1271_spi_mem_read(wl, wl->cmd_box_addr,
			    &static_data, sizeof(static_data));

	strncpy(wl->chip.fw_ver, static_data.fw_version,
		sizeof(wl->chip.fw_ver));

	
	wl->chip.fw_ver[sizeof(wl->chip.fw_ver) - 1] = '\0';
}

static int wl1271_boot_upload_firmware_chunk(struct wl1271 *wl, void *buf,
					     size_t fw_data_len, u32 dest)
{
	int addr, chunk_num, partition_limit;
	u8 *p;

	

	wl1271_debug(DEBUG_BOOT, "starting firmware upload");

	wl1271_debug(DEBUG_BOOT, "fw_data_len %zd chunk_size %d",
		     fw_data_len, CHUNK_SIZE);


	if ((fw_data_len % 4) != 0) {
		wl1271_error("firmware length not multiple of four");
		return -EIO;
	}

	wl1271_set_partition(wl, dest,
			     part_table[PART_DOWN].mem.size,
			     part_table[PART_DOWN].reg.start,
			     part_table[PART_DOWN].reg.size);

	
	chunk_num = 0;
	partition_limit = part_table[PART_DOWN].mem.size;

	while (chunk_num < fw_data_len / CHUNK_SIZE) {
		
		addr = dest + (chunk_num + 2) * CHUNK_SIZE;
		if (addr > partition_limit) {
			addr = dest + chunk_num * CHUNK_SIZE;
			partition_limit = chunk_num * CHUNK_SIZE +
				part_table[PART_DOWN].mem.size;

			
			wl1271_set_partition(wl,
					     addr,
					     part_table[PART_DOWN].mem.size,
					     part_table[PART_DOWN].reg.start,
					     part_table[PART_DOWN].reg.size);
		}

		
		addr = dest + chunk_num * CHUNK_SIZE;
		p = buf + chunk_num * CHUNK_SIZE;
		wl1271_debug(DEBUG_BOOT, "uploading fw chunk 0x%p to 0x%x",
			     p, addr);
		wl1271_spi_mem_write(wl, addr, p, CHUNK_SIZE);

		chunk_num++;
	}

	
	addr = dest + chunk_num * CHUNK_SIZE;
	p = buf + chunk_num * CHUNK_SIZE;
	wl1271_debug(DEBUG_BOOT, "uploading fw last chunk (%zd B) 0x%p to 0x%x",
		     fw_data_len % CHUNK_SIZE, p, addr);
	wl1271_spi_mem_write(wl, addr, p, fw_data_len % CHUNK_SIZE);

	return 0;
}

static int wl1271_boot_upload_firmware(struct wl1271 *wl)
{
	u32 chunks, addr, len;
	u8 *fw;

	fw = wl->fw;
	chunks = be32_to_cpup((u32 *) fw);
	fw += sizeof(u32);

	wl1271_debug(DEBUG_BOOT, "firmware chunks to be uploaded: %u", chunks);

	while (chunks--) {
		addr = be32_to_cpup((u32 *) fw);
		fw += sizeof(u32);
		len = be32_to_cpup((u32 *) fw);
		fw += sizeof(u32);

		if (len > 300000) {
			wl1271_info("firmware chunk too long: %u", len);
			return -EINVAL;
		}
		wl1271_debug(DEBUG_BOOT, "chunk %d addr 0x%x len %u",
			     chunks, addr, len);
		wl1271_boot_upload_firmware_chunk(wl, fw, len, addr);
		fw += len;
	}

	return 0;
}

static int wl1271_boot_upload_nvs(struct wl1271 *wl)
{
	size_t nvs_len, burst_len;
	int i;
	u32 dest_addr, val;
	u8 *nvs_ptr, *nvs, *nvs_aligned;

	nvs = wl->nvs;
	if (nvs == NULL)
		return -ENODEV;

	nvs_ptr = nvs;

	nvs_len = wl->nvs_len;

	
	nvs[11] = wl->mac_addr[0];
	nvs[10] = wl->mac_addr[1];
	nvs[6] = wl->mac_addr[2];
	nvs[5] = wl->mac_addr[3];
	nvs[4] = wl->mac_addr[4];
	nvs[3] = wl->mac_addr[5];

	

	
	while (nvs_ptr[0]) {
		burst_len = nvs_ptr[0];
		dest_addr = (nvs_ptr[1] & 0xfe) | ((u32)(nvs_ptr[2] << 8));

		
		dest_addr += REGISTERS_BASE;

		
		nvs_ptr += 3;

		for (i = 0; i < burst_len; i++) {
			val = (nvs_ptr[0] | (nvs_ptr[1] << 8)
			       | (nvs_ptr[2] << 16) | (nvs_ptr[3] << 24));

			wl1271_debug(DEBUG_BOOT,
				     "nvs burst write 0x%x: 0x%x",
				     dest_addr, val);
			wl1271_reg_write32(wl, dest_addr, val);

			nvs_ptr += 4;
			dest_addr += 4;
		}
	}

	
	nvs_ptr += 7;
	nvs_len -= nvs_ptr - nvs;
	nvs_len = ALIGN(nvs_len, 4);

	
	
	wl1271_set_partition(wl,
			     part_table[PART_WORK].mem.start,
			     part_table[PART_WORK].mem.size,
			     part_table[PART_WORK].reg.start,
			     part_table[PART_WORK].reg.size);

	
	nvs_aligned = kmemdup(nvs_ptr, nvs_len, GFP_KERNEL);

	
	
	wl1271_spi_mem_write(wl, CMD_MBOX_ADDRESS, nvs_aligned, nvs_len);

	kfree(nvs_aligned);
	return 0;
}

static void wl1271_boot_enable_interrupts(struct wl1271 *wl)
{
	enable_irq(wl->irq);
	wl1271_reg_write32(wl, ACX_REG_INTERRUPT_MASK,
			   WL1271_ACX_INTR_ALL & ~(WL1271_INTR_MASK));
	wl1271_reg_write32(wl, HI_CFG, HI_CFG_DEF_VAL);
}

static int wl1271_boot_soft_reset(struct wl1271 *wl)
{
	unsigned long timeout;
	u32 boot_data;

	
	wl1271_reg_write32(wl, ACX_REG_SLV_SOFT_RESET, ACX_SLV_SOFT_RESET_BIT);

	
	timeout = jiffies + usecs_to_jiffies(SOFT_RESET_MAX_TIME);
	while (1) {
		boot_data = wl1271_reg_read32(wl, ACX_REG_SLV_SOFT_RESET);
		wl1271_debug(DEBUG_BOOT, "soft reset bootdata 0x%x", boot_data);
		if ((boot_data & ACX_SLV_SOFT_RESET_BIT) == 0)
			break;

		if (time_after(jiffies, timeout)) {
			
			wl1271_error("soft reset timeout");
			return -1;
		}

		udelay(SOFT_RESET_STALL_TIME);
	}

	
	wl1271_reg_write32(wl, ENABLE, 0x0);

	
	wl1271_reg_write32(wl, SPARE_A2, 0xffff);

	return 0;
}

static int wl1271_boot_run_firmware(struct wl1271 *wl)
{
	int loop, ret;
	u32 chip_id, interrupt;

	wl1271_boot_set_ecpu_ctrl(wl, ECPU_CONTROL_HALT);

	chip_id = wl1271_reg_read32(wl, CHIP_ID_B);

	wl1271_debug(DEBUG_BOOT, "chip id after firmware boot: 0x%x", chip_id);

	if (chip_id != wl->chip.id) {
		wl1271_error("chip id doesn't match after firmware boot");
		return -EIO;
	}

	
	loop = 0;
	while (loop++ < INIT_LOOP) {
		udelay(INIT_LOOP_DELAY);
		interrupt = wl1271_reg_read32(wl, ACX_REG_INTERRUPT_NO_CLEAR);

		if (interrupt == 0xffffffff) {
			wl1271_error("error reading hardware complete "
				     "init indication");
			return -EIO;
		}
		
		else if (interrupt & WL1271_ACX_INTR_INIT_COMPLETE) {
			wl1271_reg_write32(wl, ACX_REG_INTERRUPT_ACK,
					   WL1271_ACX_INTR_INIT_COMPLETE);
			break;
		}
	}

	if (loop >= INIT_LOOP) {
		wl1271_error("timeout waiting for the hardware to "
			     "complete initialization");
		return -EIO;
	}

	
	wl->cmd_box_addr = wl1271_reg_read32(wl, REG_COMMAND_MAILBOX_PTR);

	
	wl->event_box_addr = wl1271_reg_read32(wl, REG_EVENT_MAILBOX_PTR);

	
	wl1271_set_partition(wl,
			     part_table[PART_WORK].mem.start,
			     part_table[PART_WORK].mem.size,
			     part_table[PART_WORK].reg.start,
			     part_table[PART_WORK].reg.size);

	wl1271_debug(DEBUG_MAILBOX, "cmd_box_addr 0x%x event_box_addr 0x%x",
		     wl->cmd_box_addr, wl->event_box_addr);

	wl1271_boot_fw_version(wl);

	

	
	wl1271_boot_enable_interrupts(wl);

	
	wl->event_mask = 0xffffffff;

	ret = wl1271_event_unmask(wl);
	if (ret < 0) {
		wl1271_error("EVENT mask setting failed");
		return ret;
	}

	wl1271_event_mbox_config(wl);

	
	return 0;
}

static int wl1271_boot_write_irq_polarity(struct wl1271 *wl)
{
	u32 polarity, status, i;

	wl1271_reg_write32(wl, OCP_POR_CTR, OCP_REG_POLARITY);
	wl1271_reg_write32(wl, OCP_CMD, OCP_CMD_READ);

	
	for (i = 0; i < OCP_CMD_LOOP; i++) {
		polarity = wl1271_reg_read32(wl, OCP_DATA_READ);
		if (polarity & OCP_READY_MASK)
			break;
	}
	if (i == OCP_CMD_LOOP) {
		wl1271_error("OCP command timeout!");
		return -EIO;
	}

	status = polarity & OCP_STATUS_MASK;
	if (status != OCP_STATUS_OK) {
		wl1271_error("OCP command failed (%d)", status);
		return -EIO;
	}

	
	polarity &= ~POLARITY_LOW;

	wl1271_reg_write32(wl, OCP_POR_CTR, OCP_REG_POLARITY);
	wl1271_reg_write32(wl, OCP_DATA_WRITE, polarity);
	wl1271_reg_write32(wl, OCP_CMD, OCP_CMD_WRITE);

	return 0;
}

int wl1271_boot(struct wl1271 *wl)
{
	int ret = 0;
	u32 tmp, clk, pause;

	if (REF_CLOCK == 0 || REF_CLOCK == 2)
		
		clk = 0x3;
	else if (REF_CLOCK == 1 || REF_CLOCK == 3)
		
		clk = 0x5;

	wl1271_reg_write32(wl, PLL_PARAMETERS, clk);

	pause = wl1271_reg_read32(wl, PLL_PARAMETERS);

	wl1271_debug(DEBUG_BOOT, "pause1 0x%x", pause);

	pause &= ~(WU_COUNTER_PAUSE_VAL); 
	pause |= WU_COUNTER_PAUSE_VAL;
	wl1271_reg_write32(wl, WU_COUNTER_PAUSE, pause);

	
	wl1271_reg_write32(wl, WELP_ARM_COMMAND, WELP_ARM_COMMAND_VAL);
	udelay(500);

	wl1271_set_partition(wl,
			     part_table[PART_DRPW].mem.start,
			     part_table[PART_DRPW].mem.size,
			     part_table[PART_DRPW].reg.start,
			     part_table[PART_DRPW].reg.size);

	

	wl1271_debug(DEBUG_BOOT, "DRPW_SCRATCH_START %08x", DRPW_SCRATCH_START);
	clk = wl1271_reg_read32(wl, DRPW_SCRATCH_START);

	wl1271_debug(DEBUG_BOOT, "clk2 0x%x", clk);

	
	clk |= (REF_CLOCK << 1) << 4;
	wl1271_reg_write32(wl, DRPW_SCRATCH_START, clk);

	wl1271_set_partition(wl,
			     part_table[PART_WORK].mem.start,
			     part_table[PART_WORK].mem.size,
			     part_table[PART_WORK].reg.start,
			     part_table[PART_WORK].reg.size);

	
	wl1271_reg_write32(wl, ACX_REG_INTERRUPT_MASK, WL1271_ACX_INTR_ALL);

	ret = wl1271_boot_soft_reset(wl);
	if (ret < 0)
		goto out;

	
	ret = wl1271_boot_upload_nvs(wl);
	if (ret < 0)
		goto out;

	
	wl1271_debug(DEBUG_BOOT, "ACX_EEPROMLESS_IND_REG");

	wl1271_reg_write32(wl, ACX_EEPROMLESS_IND_REG, ACX_EEPROMLESS_IND_REG);

	tmp = wl1271_reg_read32(wl, CHIP_ID_B);

	wl1271_debug(DEBUG_BOOT, "chip id 0x%x", tmp);

	
	tmp = wl1271_reg_read32(wl, SCR_PAD2);

	ret = wl1271_boot_write_irq_polarity(wl);
	if (ret < 0)
		goto out;

	
	wl1271_reg_write32(wl, ACX_REG_INTERRUPT_MASK,
			   WL1271_ACX_ALL_EVENTS_VECTOR);

	

	ret = wl1271_boot_upload_firmware(wl);
	if (ret < 0)
		goto out;

	
	ret = wl1271_boot_run_firmware(wl);
	if (ret < 0)
		goto out;

	
	wl->rx_config = WL1271_DEFAULT_RX_CONFIG;
	wl->rx_filter = WL1271_DEFAULT_RX_FILTER;

	wl1271_event_mbox_config(wl);

out:
	return ret;
}
