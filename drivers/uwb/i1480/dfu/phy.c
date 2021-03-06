
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/usb/wusb.h>
#include "i1480-dfu.h"



static
int i1480_mpi_write(struct i1480 *i1480, const void *data, size_t size)
{
	int result;
	struct i1480_cmd_mpi_write *cmd = i1480->cmd_buf;
	struct i1480_evt_confirm *reply = i1480->evt_buf;

	BUG_ON(size > 480);
	result = -ENOMEM;
	cmd->rccb.bCommandType = i1480_CET_VS1;
	cmd->rccb.wCommand = cpu_to_le16(i1480_CMD_MPI_WRITE);
	cmd->size = cpu_to_le16(size);
	memcpy(cmd->data, data, size);
	reply->rceb.bEventType = i1480_CET_VS1;
	reply->rceb.wEvent = i1480_CMD_MPI_WRITE;
	result = i1480_cmd(i1480, "MPI-WRITE", sizeof(*cmd) + size, sizeof(*reply));
	if (result < 0)
		goto out;
	if (reply->bResultCode != UWB_RC_RES_SUCCESS) {
		dev_err(i1480->dev, "MPI-WRITE: command execution failed: %d\n",
			reply->bResultCode);
		result = -EIO;
	}
out:
	return result;
}



static
int i1480_mpi_read(struct i1480 *i1480, u8 *data, u16 srcaddr, size_t size)
{
	int result;
	struct i1480_cmd_mpi_read *cmd = i1480->cmd_buf;
	struct i1480_evt_mpi_read *reply = i1480->evt_buf;
	unsigned cnt;

	memset(i1480->cmd_buf, 0x69, 512);
	memset(i1480->evt_buf, 0x69, 512);

	BUG_ON(size > (i1480->buf_size - sizeof(*reply)) / 3);
	result = -ENOMEM;
	cmd->rccb.bCommandType = i1480_CET_VS1;
	cmd->rccb.wCommand = cpu_to_le16(i1480_CMD_MPI_READ);
	cmd->size = cpu_to_le16(3*size);
	for (cnt = 0; cnt < size; cnt++) {
		cmd->data[cnt].page = (srcaddr + cnt) >> 8;
		cmd->data[cnt].offset = (srcaddr + cnt) & 0xff;
	}
	reply->rceb.bEventType = i1480_CET_VS1;
	reply->rceb.wEvent = i1480_CMD_MPI_READ;
	result = i1480_cmd(i1480, "MPI-READ", sizeof(*cmd) + 2*size,
			sizeof(*reply) + 3*size);
	if (result < 0)
		goto out;
	if (reply->bResultCode != UWB_RC_RES_SUCCESS) {
		dev_err(i1480->dev, "MPI-READ: command execution failed: %d\n",
			reply->bResultCode);
		result = -EIO;
	}
	for (cnt = 0; cnt < size; cnt++) {
		if (reply->data[cnt].page != (srcaddr + cnt) >> 8)
			dev_err(i1480->dev, "MPI-READ: page inconsistency at "
				"index %u: expected 0x%02x, got 0x%02x\n", cnt,
				(srcaddr + cnt) >> 8, reply->data[cnt].page);
		if (reply->data[cnt].offset != ((srcaddr + cnt) & 0x00ff))
			dev_err(i1480->dev, "MPI-READ: offset inconsistency at "
				"index %u: expected 0x%02x, got 0x%02x\n", cnt,
				(srcaddr + cnt) & 0x00ff,
				reply->data[cnt].offset);
		data[cnt] = reply->data[cnt].value;
	}
	result = 0;
out:
	return result;
}



int i1480_phy_fw_upload(struct i1480 *i1480)
{
	int result;
	const struct firmware *fw;
	const char *data_itr, *data_top;
	const size_t MAX_BLK_SIZE = 480;	
	size_t data_size;
	u8 phy_stat;

	result = request_firmware(&fw, i1480->phy_fw_name, i1480->dev);
	if (result < 0)
		goto out;
	
	for (data_itr = fw->data, data_top = data_itr + fw->size;
	     data_itr < data_top; data_itr += MAX_BLK_SIZE) {
		data_size = min(MAX_BLK_SIZE, (size_t) (data_top - data_itr));
		result = i1480_mpi_write(i1480, data_itr, data_size);
		if (result < 0)
			goto error_mpi_write;
	}
	
	result = i1480_mpi_read(i1480, &phy_stat, 0x0006, 1);
	if (result < 0) {
		dev_err(i1480->dev, "PHY: can't get status: %d\n", result);
		goto error_mpi_status;
	}
	if (phy_stat != 0) {
		result = -ENODEV;
		dev_info(i1480->dev, "error, PHY not ready: %u\n", phy_stat);
		goto error_phy_status;
	}
	dev_info(i1480->dev, "PHY fw '%s': uploaded\n", i1480->phy_fw_name);
error_phy_status:
error_mpi_status:
error_mpi_write:
	release_firmware(fw);
	if (result < 0)
		dev_err(i1480->dev, "PHY fw '%s': failed to upload (%d), "
			"power cycle device\n", i1480->phy_fw_name, result);
out:
	return result;
}
