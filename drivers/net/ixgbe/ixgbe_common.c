

#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/netdevice.h>

#include "ixgbe.h"
#include "ixgbe_common.h"
#include "ixgbe_phy.h"

static s32 ixgbe_poll_eeprom_eerd_done(struct ixgbe_hw *hw);
static s32 ixgbe_acquire_eeprom(struct ixgbe_hw *hw);
static s32 ixgbe_get_eeprom_semaphore(struct ixgbe_hw *hw);
static void ixgbe_release_eeprom_semaphore(struct ixgbe_hw *hw);
static s32 ixgbe_ready_eeprom(struct ixgbe_hw *hw);
static void ixgbe_standby_eeprom(struct ixgbe_hw *hw);
static void ixgbe_shift_out_eeprom_bits(struct ixgbe_hw *hw, u16 data,
                                        u16 count);
static u16 ixgbe_shift_in_eeprom_bits(struct ixgbe_hw *hw, u16 count);
static void ixgbe_raise_eeprom_clk(struct ixgbe_hw *hw, u32 *eec);
static void ixgbe_lower_eeprom_clk(struct ixgbe_hw *hw, u32 *eec);
static void ixgbe_release_eeprom(struct ixgbe_hw *hw);
static u16 ixgbe_calc_eeprom_checksum(struct ixgbe_hw *hw);

static void ixgbe_enable_rar(struct ixgbe_hw *hw, u32 index);
static void ixgbe_disable_rar(struct ixgbe_hw *hw, u32 index);
static s32 ixgbe_mta_vector(struct ixgbe_hw *hw, u8 *mc_addr);
static void ixgbe_add_uc_addr(struct ixgbe_hw *hw, u8 *addr, u32 vmdq);
static s32 ixgbe_setup_fc(struct ixgbe_hw *hw, s32 packetbuf_num);


s32 ixgbe_start_hw_generic(struct ixgbe_hw *hw)
{
	u32 ctrl_ext;

	
	hw->phy.media_type = hw->mac.ops.get_media_type(hw);

	
	hw->phy.ops.identify(hw);

	
	hw->mac.ops.clear_vfta(hw);

	
	hw->mac.ops.clear_hw_cntrs(hw);

	
	ctrl_ext = IXGBE_READ_REG(hw, IXGBE_CTRL_EXT);
	ctrl_ext |= IXGBE_CTRL_EXT_NS_DIS;
	IXGBE_WRITE_REG(hw, IXGBE_CTRL_EXT, ctrl_ext);
	IXGBE_WRITE_FLUSH(hw);

	
	ixgbe_setup_fc(hw, 0);

	
	hw->adapter_stopped = false;

	return 0;
}


s32 ixgbe_init_hw_generic(struct ixgbe_hw *hw)
{
	s32 status;

	
	status = hw->mac.ops.reset_hw(hw);

	if (status == 0) {
		
		status = hw->mac.ops.start_hw(hw);
	}

	return status;
}


s32 ixgbe_clear_hw_cntrs_generic(struct ixgbe_hw *hw)
{
	u16 i = 0;

	IXGBE_READ_REG(hw, IXGBE_CRCERRS);
	IXGBE_READ_REG(hw, IXGBE_ILLERRC);
	IXGBE_READ_REG(hw, IXGBE_ERRBC);
	IXGBE_READ_REG(hw, IXGBE_MSPDC);
	for (i = 0; i < 8; i++)
		IXGBE_READ_REG(hw, IXGBE_MPC(i));

	IXGBE_READ_REG(hw, IXGBE_MLFC);
	IXGBE_READ_REG(hw, IXGBE_MRFC);
	IXGBE_READ_REG(hw, IXGBE_RLEC);
	IXGBE_READ_REG(hw, IXGBE_LXONTXC);
	IXGBE_READ_REG(hw, IXGBE_LXONRXC);
	IXGBE_READ_REG(hw, IXGBE_LXOFFTXC);
	IXGBE_READ_REG(hw, IXGBE_LXOFFRXC);

	for (i = 0; i < 8; i++) {
		IXGBE_READ_REG(hw, IXGBE_PXONTXC(i));
		IXGBE_READ_REG(hw, IXGBE_PXONRXC(i));
		IXGBE_READ_REG(hw, IXGBE_PXOFFTXC(i));
		IXGBE_READ_REG(hw, IXGBE_PXOFFRXC(i));
	}

	IXGBE_READ_REG(hw, IXGBE_PRC64);
	IXGBE_READ_REG(hw, IXGBE_PRC127);
	IXGBE_READ_REG(hw, IXGBE_PRC255);
	IXGBE_READ_REG(hw, IXGBE_PRC511);
	IXGBE_READ_REG(hw, IXGBE_PRC1023);
	IXGBE_READ_REG(hw, IXGBE_PRC1522);
	IXGBE_READ_REG(hw, IXGBE_GPRC);
	IXGBE_READ_REG(hw, IXGBE_BPRC);
	IXGBE_READ_REG(hw, IXGBE_MPRC);
	IXGBE_READ_REG(hw, IXGBE_GPTC);
	IXGBE_READ_REG(hw, IXGBE_GORCL);
	IXGBE_READ_REG(hw, IXGBE_GORCH);
	IXGBE_READ_REG(hw, IXGBE_GOTCL);
	IXGBE_READ_REG(hw, IXGBE_GOTCH);
	for (i = 0; i < 8; i++)
		IXGBE_READ_REG(hw, IXGBE_RNBC(i));
	IXGBE_READ_REG(hw, IXGBE_RUC);
	IXGBE_READ_REG(hw, IXGBE_RFC);
	IXGBE_READ_REG(hw, IXGBE_ROC);
	IXGBE_READ_REG(hw, IXGBE_RJC);
	IXGBE_READ_REG(hw, IXGBE_MNGPRC);
	IXGBE_READ_REG(hw, IXGBE_MNGPDC);
	IXGBE_READ_REG(hw, IXGBE_MNGPTC);
	IXGBE_READ_REG(hw, IXGBE_TORL);
	IXGBE_READ_REG(hw, IXGBE_TORH);
	IXGBE_READ_REG(hw, IXGBE_TPR);
	IXGBE_READ_REG(hw, IXGBE_TPT);
	IXGBE_READ_REG(hw, IXGBE_PTC64);
	IXGBE_READ_REG(hw, IXGBE_PTC127);
	IXGBE_READ_REG(hw, IXGBE_PTC255);
	IXGBE_READ_REG(hw, IXGBE_PTC511);
	IXGBE_READ_REG(hw, IXGBE_PTC1023);
	IXGBE_READ_REG(hw, IXGBE_PTC1522);
	IXGBE_READ_REG(hw, IXGBE_MPTC);
	IXGBE_READ_REG(hw, IXGBE_BPTC);
	for (i = 0; i < 16; i++) {
		IXGBE_READ_REG(hw, IXGBE_QPRC(i));
		IXGBE_READ_REG(hw, IXGBE_QBRC(i));
		IXGBE_READ_REG(hw, IXGBE_QPTC(i));
		IXGBE_READ_REG(hw, IXGBE_QBTC(i));
	}

	return 0;
}


s32 ixgbe_read_pba_num_generic(struct ixgbe_hw *hw, u32 *pba_num)
{
	s32 ret_val;
	u16 data;

	ret_val = hw->eeprom.ops.read(hw, IXGBE_PBANUM0_PTR, &data);
	if (ret_val) {
		hw_dbg(hw, "NVM Read Error\n");
		return ret_val;
	}
	*pba_num = (u32)(data << 16);

	ret_val = hw->eeprom.ops.read(hw, IXGBE_PBANUM1_PTR, &data);
	if (ret_val) {
		hw_dbg(hw, "NVM Read Error\n");
		return ret_val;
	}
	*pba_num |= data;

	return 0;
}


s32 ixgbe_get_mac_addr_generic(struct ixgbe_hw *hw, u8 *mac_addr)
{
	u32 rar_high;
	u32 rar_low;
	u16 i;

	rar_high = IXGBE_READ_REG(hw, IXGBE_RAH(0));
	rar_low = IXGBE_READ_REG(hw, IXGBE_RAL(0));

	for (i = 0; i < 4; i++)
		mac_addr[i] = (u8)(rar_low >> (i*8));

	for (i = 0; i < 2; i++)
		mac_addr[i+4] = (u8)(rar_high >> (i*8));

	return 0;
}


s32 ixgbe_get_bus_info_generic(struct ixgbe_hw *hw)
{
	struct ixgbe_adapter *adapter = hw->back;
	struct ixgbe_mac_info *mac = &hw->mac;
	u16 link_status;

	hw->bus.type = ixgbe_bus_type_pci_express;

	
	pci_read_config_word(adapter->pdev, IXGBE_PCI_LINK_STATUS,
	                     &link_status);

	switch (link_status & IXGBE_PCI_LINK_WIDTH) {
	case IXGBE_PCI_LINK_WIDTH_1:
		hw->bus.width = ixgbe_bus_width_pcie_x1;
		break;
	case IXGBE_PCI_LINK_WIDTH_2:
		hw->bus.width = ixgbe_bus_width_pcie_x2;
		break;
	case IXGBE_PCI_LINK_WIDTH_4:
		hw->bus.width = ixgbe_bus_width_pcie_x4;
		break;
	case IXGBE_PCI_LINK_WIDTH_8:
		hw->bus.width = ixgbe_bus_width_pcie_x8;
		break;
	default:
		hw->bus.width = ixgbe_bus_width_unknown;
		break;
	}

	switch (link_status & IXGBE_PCI_LINK_SPEED) {
	case IXGBE_PCI_LINK_SPEED_2500:
		hw->bus.speed = ixgbe_bus_speed_2500;
		break;
	case IXGBE_PCI_LINK_SPEED_5000:
		hw->bus.speed = ixgbe_bus_speed_5000;
		break;
	default:
		hw->bus.speed = ixgbe_bus_speed_unknown;
		break;
	}

	mac->ops.set_lan_id(hw);

	return 0;
}


void ixgbe_set_lan_id_multi_port_pcie(struct ixgbe_hw *hw)
{
	struct ixgbe_bus_info *bus = &hw->bus;
	u32 reg;

	reg = IXGBE_READ_REG(hw, IXGBE_STATUS);
	bus->func = (reg & IXGBE_STATUS_LAN_ID) >> IXGBE_STATUS_LAN_ID_SHIFT;
	bus->lan_id = bus->func;

	
	reg = IXGBE_READ_REG(hw, IXGBE_FACTPS);
	if (reg & IXGBE_FACTPS_LFS)
		bus->func ^= 0x1;
}


s32 ixgbe_stop_adapter_generic(struct ixgbe_hw *hw)
{
	u32 number_of_queues;
	u32 reg_val;
	u16 i;

	
	hw->adapter_stopped = true;

	
	reg_val = IXGBE_READ_REG(hw, IXGBE_RXCTRL);
	reg_val &= ~(IXGBE_RXCTRL_RXEN);
	IXGBE_WRITE_REG(hw, IXGBE_RXCTRL, reg_val);
	IXGBE_WRITE_FLUSH(hw);
	msleep(2);

	
	IXGBE_WRITE_REG(hw, IXGBE_EIMC, IXGBE_IRQ_CLEAR_MASK);

	
	IXGBE_READ_REG(hw, IXGBE_EICR);

	
	number_of_queues = hw->mac.max_tx_queues;
	for (i = 0; i < number_of_queues; i++) {
		reg_val = IXGBE_READ_REG(hw, IXGBE_TXDCTL(i));
		if (reg_val & IXGBE_TXDCTL_ENABLE) {
			reg_val &= ~IXGBE_TXDCTL_ENABLE;
			IXGBE_WRITE_REG(hw, IXGBE_TXDCTL(i), reg_val);
		}
	}

	
	if (ixgbe_disable_pcie_master(hw) != 0)
		hw_dbg(hw, "PCI-E Master disable polling has failed.\n");

	return 0;
}


s32 ixgbe_led_on_generic(struct ixgbe_hw *hw, u32 index)
{
	u32 led_reg = IXGBE_READ_REG(hw, IXGBE_LEDCTL);

	
	led_reg &= ~IXGBE_LED_MODE_MASK(index);
	led_reg |= IXGBE_LED_ON << IXGBE_LED_MODE_SHIFT(index);
	IXGBE_WRITE_REG(hw, IXGBE_LEDCTL, led_reg);
	IXGBE_WRITE_FLUSH(hw);

	return 0;
}


s32 ixgbe_led_off_generic(struct ixgbe_hw *hw, u32 index)
{
	u32 led_reg = IXGBE_READ_REG(hw, IXGBE_LEDCTL);

	
	led_reg &= ~IXGBE_LED_MODE_MASK(index);
	led_reg |= IXGBE_LED_OFF << IXGBE_LED_MODE_SHIFT(index);
	IXGBE_WRITE_REG(hw, IXGBE_LEDCTL, led_reg);
	IXGBE_WRITE_FLUSH(hw);

	return 0;
}


s32 ixgbe_init_eeprom_params_generic(struct ixgbe_hw *hw)
{
	struct ixgbe_eeprom_info *eeprom = &hw->eeprom;
	u32 eec;
	u16 eeprom_size;

	if (eeprom->type == ixgbe_eeprom_uninitialized) {
		eeprom->type = ixgbe_eeprom_none;
		
		eeprom->semaphore_delay = 10;

		
		eec = IXGBE_READ_REG(hw, IXGBE_EEC);
		if (eec & IXGBE_EEC_PRES) {
			eeprom->type = ixgbe_eeprom_spi;

			
			eeprom_size = (u16)((eec & IXGBE_EEC_SIZE) >>
					    IXGBE_EEC_SIZE_SHIFT);
			eeprom->word_size = 1 << (eeprom_size +
						  IXGBE_EEPROM_WORD_SIZE_SHIFT);
		}

		if (eec & IXGBE_EEC_ADDR_SIZE)
			eeprom->address_bits = 16;
		else
			eeprom->address_bits = 8;
		hw_dbg(hw, "Eeprom params: type = %d, size = %d, address bits: "
			  "%d\n", eeprom->type, eeprom->word_size,
			  eeprom->address_bits);
	}

	return 0;
}


s32 ixgbe_write_eeprom_generic(struct ixgbe_hw *hw, u16 offset, u16 data)
{
	s32 status;
	u8 write_opcode = IXGBE_EEPROM_WRITE_OPCODE_SPI;

	hw->eeprom.ops.init_params(hw);

	if (offset >= hw->eeprom.word_size) {
		status = IXGBE_ERR_EEPROM;
		goto out;
	}

	
	status = ixgbe_acquire_eeprom(hw);

	if (status == 0) {
		if (ixgbe_ready_eeprom(hw) != 0) {
			ixgbe_release_eeprom(hw);
			status = IXGBE_ERR_EEPROM;
		}
	}

	if (status == 0) {
		ixgbe_standby_eeprom(hw);

		
		ixgbe_shift_out_eeprom_bits(hw, IXGBE_EEPROM_WREN_OPCODE_SPI,
		                            IXGBE_EEPROM_OPCODE_BITS);

		ixgbe_standby_eeprom(hw);

		
		if ((hw->eeprom.address_bits == 8) && (offset >= 128))
			write_opcode |= IXGBE_EEPROM_A8_OPCODE_SPI;

		
		ixgbe_shift_out_eeprom_bits(hw, write_opcode,
		                            IXGBE_EEPROM_OPCODE_BITS);
		ixgbe_shift_out_eeprom_bits(hw, (u16)(offset*2),
		                            hw->eeprom.address_bits);

		
		data = (data >> 8) | (data << 8);
		ixgbe_shift_out_eeprom_bits(hw, data, 16);
		ixgbe_standby_eeprom(hw);

		msleep(hw->eeprom.semaphore_delay);
		
		ixgbe_release_eeprom(hw);
	}

out:
	return status;
}


s32 ixgbe_read_eeprom_bit_bang_generic(struct ixgbe_hw *hw, u16 offset,
                                       u16 *data)
{
	s32 status;
	u16 word_in;
	u8 read_opcode = IXGBE_EEPROM_READ_OPCODE_SPI;

	hw->eeprom.ops.init_params(hw);

	if (offset >= hw->eeprom.word_size) {
		status = IXGBE_ERR_EEPROM;
		goto out;
	}

	
	status = ixgbe_acquire_eeprom(hw);

	if (status == 0) {
		if (ixgbe_ready_eeprom(hw) != 0) {
			ixgbe_release_eeprom(hw);
			status = IXGBE_ERR_EEPROM;
		}
	}

	if (status == 0) {
		ixgbe_standby_eeprom(hw);

		
		if ((hw->eeprom.address_bits == 8) && (offset >= 128))
			read_opcode |= IXGBE_EEPROM_A8_OPCODE_SPI;

		
		ixgbe_shift_out_eeprom_bits(hw, read_opcode,
		                            IXGBE_EEPROM_OPCODE_BITS);
		ixgbe_shift_out_eeprom_bits(hw, (u16)(offset*2),
		                            hw->eeprom.address_bits);

		
		word_in = ixgbe_shift_in_eeprom_bits(hw, 16);
		*data = (word_in >> 8) | (word_in << 8);

		
		ixgbe_release_eeprom(hw);
	}

out:
	return status;
}


s32 ixgbe_read_eeprom_generic(struct ixgbe_hw *hw, u16 offset, u16 *data)
{
	u32 eerd;
	s32 status;

	hw->eeprom.ops.init_params(hw);

	if (offset >= hw->eeprom.word_size) {
		status = IXGBE_ERR_EEPROM;
		goto out;
	}

	eerd = (offset << IXGBE_EEPROM_READ_ADDR_SHIFT) +
	       IXGBE_EEPROM_READ_REG_START;

	IXGBE_WRITE_REG(hw, IXGBE_EERD, eerd);
	status = ixgbe_poll_eeprom_eerd_done(hw);

	if (status == 0)
		*data = (IXGBE_READ_REG(hw, IXGBE_EERD) >>
		         IXGBE_EEPROM_READ_REG_DATA);
	else
		hw_dbg(hw, "Eeprom read timed out\n");

out:
	return status;
}


static s32 ixgbe_poll_eeprom_eerd_done(struct ixgbe_hw *hw)
{
	u32 i;
	u32 reg;
	s32 status = IXGBE_ERR_EEPROM;

	for (i = 0; i < IXGBE_EERD_ATTEMPTS; i++) {
		reg = IXGBE_READ_REG(hw, IXGBE_EERD);
		if (reg & IXGBE_EEPROM_READ_REG_DONE) {
			status = 0;
			break;
		}
		udelay(5);
	}
	return status;
}


static s32 ixgbe_acquire_eeprom(struct ixgbe_hw *hw)
{
	s32 status = 0;
	u32 eec = 0;
	u32 i;

	if (ixgbe_acquire_swfw_sync(hw, IXGBE_GSSR_EEP_SM) != 0)
		status = IXGBE_ERR_SWFW_SYNC;

	if (status == 0) {
		eec = IXGBE_READ_REG(hw, IXGBE_EEC);

		
		eec |= IXGBE_EEC_REQ;
		IXGBE_WRITE_REG(hw, IXGBE_EEC, eec);

		for (i = 0; i < IXGBE_EEPROM_GRANT_ATTEMPTS; i++) {
			eec = IXGBE_READ_REG(hw, IXGBE_EEC);
			if (eec & IXGBE_EEC_GNT)
				break;
			udelay(5);
		}

		
		if (!(eec & IXGBE_EEC_GNT)) {
			eec &= ~IXGBE_EEC_REQ;
			IXGBE_WRITE_REG(hw, IXGBE_EEC, eec);
			hw_dbg(hw, "Could not acquire EEPROM grant\n");

			ixgbe_release_swfw_sync(hw, IXGBE_GSSR_EEP_SM);
			status = IXGBE_ERR_EEPROM;
		}
	}

	
	if (status == 0) {
		
		eec &= ~(IXGBE_EEC_CS | IXGBE_EEC_SK);
		IXGBE_WRITE_REG(hw, IXGBE_EEC, eec);
		IXGBE_WRITE_FLUSH(hw);
		udelay(1);
	}
	return status;
}


static s32 ixgbe_get_eeprom_semaphore(struct ixgbe_hw *hw)
{
	s32 status = IXGBE_ERR_EEPROM;
	u32 timeout;
	u32 i;
	u32 swsm;

	
	timeout = hw->eeprom.word_size + 1;

	
	for (i = 0; i < timeout; i++) {
		
		swsm = IXGBE_READ_REG(hw, IXGBE_SWSM);
		if (!(swsm & IXGBE_SWSM_SMBI)) {
			status = 0;
			break;
		}
		msleep(1);
	}

	
	if (status == 0) {
		for (i = 0; i < timeout; i++) {
			swsm = IXGBE_READ_REG(hw, IXGBE_SWSM);

			
			swsm |= IXGBE_SWSM_SWESMBI;
			IXGBE_WRITE_REG(hw, IXGBE_SWSM, swsm);

			
			swsm = IXGBE_READ_REG(hw, IXGBE_SWSM);
			if (swsm & IXGBE_SWSM_SWESMBI)
				break;

			udelay(50);
		}

		
		if (i >= timeout) {
			hw_dbg(hw, "Driver can't access the Eeprom - Semaphore "
			       "not granted.\n");
			ixgbe_release_eeprom_semaphore(hw);
			status = IXGBE_ERR_EEPROM;
		}
	}

	return status;
}


static void ixgbe_release_eeprom_semaphore(struct ixgbe_hw *hw)
{
	u32 swsm;

	swsm = IXGBE_READ_REG(hw, IXGBE_SWSM);

	
	swsm &= ~(IXGBE_SWSM_SWESMBI | IXGBE_SWSM_SMBI);
	IXGBE_WRITE_REG(hw, IXGBE_SWSM, swsm);
	IXGBE_WRITE_FLUSH(hw);
}


static s32 ixgbe_ready_eeprom(struct ixgbe_hw *hw)
{
	s32 status = 0;
	u16 i;
	u8 spi_stat_reg;

	
	for (i = 0; i < IXGBE_EEPROM_MAX_RETRY_SPI; i += 5) {
		ixgbe_shift_out_eeprom_bits(hw, IXGBE_EEPROM_RDSR_OPCODE_SPI,
		                            IXGBE_EEPROM_OPCODE_BITS);
		spi_stat_reg = (u8)ixgbe_shift_in_eeprom_bits(hw, 8);
		if (!(spi_stat_reg & IXGBE_EEPROM_STATUS_RDY_SPI))
			break;

		udelay(5);
		ixgbe_standby_eeprom(hw);
	};

	
	if (i >= IXGBE_EEPROM_MAX_RETRY_SPI) {
		hw_dbg(hw, "SPI EEPROM Status error\n");
		status = IXGBE_ERR_EEPROM;
	}

	return status;
}


static void ixgbe_standby_eeprom(struct ixgbe_hw *hw)
{
	u32 eec;

	eec = IXGBE_READ_REG(hw, IXGBE_EEC);

	
	eec |= IXGBE_EEC_CS;
	IXGBE_WRITE_REG(hw, IXGBE_EEC, eec);
	IXGBE_WRITE_FLUSH(hw);
	udelay(1);
	eec &= ~IXGBE_EEC_CS;
	IXGBE_WRITE_REG(hw, IXGBE_EEC, eec);
	IXGBE_WRITE_FLUSH(hw);
	udelay(1);
}


static void ixgbe_shift_out_eeprom_bits(struct ixgbe_hw *hw, u16 data,
                                        u16 count)
{
	u32 eec;
	u32 mask;
	u32 i;

	eec = IXGBE_READ_REG(hw, IXGBE_EEC);

	
	mask = 0x01 << (count - 1);

	for (i = 0; i < count; i++) {
		
		if (data & mask)
			eec |= IXGBE_EEC_DI;
		else
			eec &= ~IXGBE_EEC_DI;

		IXGBE_WRITE_REG(hw, IXGBE_EEC, eec);
		IXGBE_WRITE_FLUSH(hw);

		udelay(1);

		ixgbe_raise_eeprom_clk(hw, &eec);
		ixgbe_lower_eeprom_clk(hw, &eec);

		
		mask = mask >> 1;
	};

	
	eec &= ~IXGBE_EEC_DI;
	IXGBE_WRITE_REG(hw, IXGBE_EEC, eec);
	IXGBE_WRITE_FLUSH(hw);
}


static u16 ixgbe_shift_in_eeprom_bits(struct ixgbe_hw *hw, u16 count)
{
	u32 eec;
	u32 i;
	u16 data = 0;

	
	eec = IXGBE_READ_REG(hw, IXGBE_EEC);

	eec &= ~(IXGBE_EEC_DO | IXGBE_EEC_DI);

	for (i = 0; i < count; i++) {
		data = data << 1;
		ixgbe_raise_eeprom_clk(hw, &eec);

		eec = IXGBE_READ_REG(hw, IXGBE_EEC);

		eec &= ~(IXGBE_EEC_DI);
		if (eec & IXGBE_EEC_DO)
			data |= 1;

		ixgbe_lower_eeprom_clk(hw, &eec);
	}

	return data;
}


static void ixgbe_raise_eeprom_clk(struct ixgbe_hw *hw, u32 *eec)
{
	
	*eec = *eec | IXGBE_EEC_SK;
	IXGBE_WRITE_REG(hw, IXGBE_EEC, *eec);
	IXGBE_WRITE_FLUSH(hw);
	udelay(1);
}


static void ixgbe_lower_eeprom_clk(struct ixgbe_hw *hw, u32 *eec)
{
	
	*eec = *eec & ~IXGBE_EEC_SK;
	IXGBE_WRITE_REG(hw, IXGBE_EEC, *eec);
	IXGBE_WRITE_FLUSH(hw);
	udelay(1);
}


static void ixgbe_release_eeprom(struct ixgbe_hw *hw)
{
	u32 eec;

	eec = IXGBE_READ_REG(hw, IXGBE_EEC);

	eec |= IXGBE_EEC_CS;  
	eec &= ~IXGBE_EEC_SK; 

	IXGBE_WRITE_REG(hw, IXGBE_EEC, eec);
	IXGBE_WRITE_FLUSH(hw);

	udelay(1);

	
	eec &= ~IXGBE_EEC_REQ;
	IXGBE_WRITE_REG(hw, IXGBE_EEC, eec);

	ixgbe_release_swfw_sync(hw, IXGBE_GSSR_EEP_SM);
}


static u16 ixgbe_calc_eeprom_checksum(struct ixgbe_hw *hw)
{
	u16 i;
	u16 j;
	u16 checksum = 0;
	u16 length = 0;
	u16 pointer = 0;
	u16 word = 0;

	
	for (i = 0; i < IXGBE_EEPROM_CHECKSUM; i++) {
		if (hw->eeprom.ops.read(hw, i, &word) != 0) {
			hw_dbg(hw, "EEPROM read failed\n");
			break;
		}
		checksum += word;
	}

	
	for (i = IXGBE_PCIE_ANALOG_PTR; i < IXGBE_FW_PTR; i++) {
		hw->eeprom.ops.read(hw, i, &pointer);

		
		if (pointer != 0xFFFF && pointer != 0) {
			hw->eeprom.ops.read(hw, pointer, &length);

			if (length != 0xFFFF && length != 0) {
				for (j = pointer+1; j <= pointer+length; j++) {
					hw->eeprom.ops.read(hw, j, &word);
					checksum += word;
				}
			}
		}
	}

	checksum = (u16)IXGBE_EEPROM_SUM - checksum;

	return checksum;
}


s32 ixgbe_validate_eeprom_checksum_generic(struct ixgbe_hw *hw,
                                           u16 *checksum_val)
{
	s32 status;
	u16 checksum;
	u16 read_checksum = 0;

	
	status = hw->eeprom.ops.read(hw, 0, &checksum);

	if (status == 0) {
		checksum = ixgbe_calc_eeprom_checksum(hw);

		hw->eeprom.ops.read(hw, IXGBE_EEPROM_CHECKSUM, &read_checksum);

		
		if (read_checksum != checksum)
			status = IXGBE_ERR_EEPROM_CHECKSUM;

		
		if (checksum_val)
			*checksum_val = checksum;
	} else {
		hw_dbg(hw, "EEPROM read failed\n");
	}

	return status;
}


s32 ixgbe_update_eeprom_checksum_generic(struct ixgbe_hw *hw)
{
	s32 status;
	u16 checksum;

	
	status = hw->eeprom.ops.read(hw, 0, &checksum);

	if (status == 0) {
		checksum = ixgbe_calc_eeprom_checksum(hw);
		status = hw->eeprom.ops.write(hw, IXGBE_EEPROM_CHECKSUM,
		                            checksum);
	} else {
		hw_dbg(hw, "EEPROM read failed\n");
	}

	return status;
}


s32 ixgbe_validate_mac_addr(u8 *mac_addr)
{
	s32 status = 0;

	
	if (IXGBE_IS_MULTICAST(mac_addr))
		status = IXGBE_ERR_INVALID_MAC_ADDR;
	
	else if (IXGBE_IS_BROADCAST(mac_addr))
		status = IXGBE_ERR_INVALID_MAC_ADDR;
	
	else if (mac_addr[0] == 0 && mac_addr[1] == 0 && mac_addr[2] == 0 &&
	         mac_addr[3] == 0 && mac_addr[4] == 0 && mac_addr[5] == 0)
		status = IXGBE_ERR_INVALID_MAC_ADDR;

	return status;
}


s32 ixgbe_set_rar_generic(struct ixgbe_hw *hw, u32 index, u8 *addr, u32 vmdq,
                          u32 enable_addr)
{
	u32 rar_low, rar_high;
	u32 rar_entries = hw->mac.num_rar_entries;

	
	hw->mac.ops.set_vmdq(hw, index, vmdq);

	
	if (index < rar_entries) {
		
		rar_low = ((u32)addr[0] |
		           ((u32)addr[1] << 8) |
		           ((u32)addr[2] << 16) |
		           ((u32)addr[3] << 24));
		
		rar_high = IXGBE_READ_REG(hw, IXGBE_RAH(index));
		rar_high &= ~(0x0000FFFF | IXGBE_RAH_AV);
		rar_high |= ((u32)addr[4] | ((u32)addr[5] << 8));

		if (enable_addr != 0)
			rar_high |= IXGBE_RAH_AV;

		IXGBE_WRITE_REG(hw, IXGBE_RAL(index), rar_low);
		IXGBE_WRITE_REG(hw, IXGBE_RAH(index), rar_high);
	} else {
		hw_dbg(hw, "RAR index %d is out of range.\n", index);
	}

	return 0;
}


s32 ixgbe_clear_rar_generic(struct ixgbe_hw *hw, u32 index)
{
	u32 rar_high;
	u32 rar_entries = hw->mac.num_rar_entries;

	
	if (index < rar_entries) {
		
		rar_high = IXGBE_READ_REG(hw, IXGBE_RAH(index));
		rar_high &= ~(0x0000FFFF | IXGBE_RAH_AV);

		IXGBE_WRITE_REG(hw, IXGBE_RAL(index), 0);
		IXGBE_WRITE_REG(hw, IXGBE_RAH(index), rar_high);
	} else {
		hw_dbg(hw, "RAR index %d is out of range.\n", index);
	}

	
	hw->mac.ops.clear_vmdq(hw, index, IXGBE_CLEAR_VMDQ_ALL);

	return 0;
}


static void ixgbe_enable_rar(struct ixgbe_hw *hw, u32 index)
{
	u32 rar_high;

	rar_high = IXGBE_READ_REG(hw, IXGBE_RAH(index));
	rar_high |= IXGBE_RAH_AV;
	IXGBE_WRITE_REG(hw, IXGBE_RAH(index), rar_high);
}


static void ixgbe_disable_rar(struct ixgbe_hw *hw, u32 index)
{
	u32 rar_high;

	rar_high = IXGBE_READ_REG(hw, IXGBE_RAH(index));
	rar_high &= (~IXGBE_RAH_AV);
	IXGBE_WRITE_REG(hw, IXGBE_RAH(index), rar_high);
}


s32 ixgbe_init_rx_addrs_generic(struct ixgbe_hw *hw)
{
	u32 i;
	u32 rar_entries = hw->mac.num_rar_entries;

	
	if (ixgbe_validate_mac_addr(hw->mac.addr) ==
	    IXGBE_ERR_INVALID_MAC_ADDR) {
		
		hw->mac.ops.get_mac_addr(hw, hw->mac.addr);

		hw_dbg(hw, " Keeping Current RAR0 Addr =%.2X %.2X %.2X ",
		       hw->mac.addr[0], hw->mac.addr[1],
		       hw->mac.addr[2]);
		hw_dbg(hw, "%.2X %.2X %.2X\n", hw->mac.addr[3],
		       hw->mac.addr[4], hw->mac.addr[5]);
	} else {
		
		hw_dbg(hw, "Overriding MAC Address in RAR[0]\n");
		hw_dbg(hw, " New MAC Addr =%.2X %.2X %.2X ",
		       hw->mac.addr[0], hw->mac.addr[1],
		       hw->mac.addr[2]);
		hw_dbg(hw, "%.2X %.2X %.2X\n", hw->mac.addr[3],
		       hw->mac.addr[4], hw->mac.addr[5]);

		hw->mac.ops.set_rar(hw, 0, hw->mac.addr, 0, IXGBE_RAH_AV);
	}
	hw->addr_ctrl.overflow_promisc = 0;

	hw->addr_ctrl.rar_used_count = 1;

	
	hw_dbg(hw, "Clearing RAR[1-%d]\n", rar_entries - 1);
	for (i = 1; i < rar_entries; i++) {
		IXGBE_WRITE_REG(hw, IXGBE_RAL(i), 0);
		IXGBE_WRITE_REG(hw, IXGBE_RAH(i), 0);
	}

	
	hw->addr_ctrl.mc_addr_in_rar_count = 0;
	hw->addr_ctrl.mta_in_use = 0;
	IXGBE_WRITE_REG(hw, IXGBE_MCSTCTRL, hw->mac.mc_filter_type);

	hw_dbg(hw, " Clearing MTA\n");
	for (i = 0; i < hw->mac.mcft_size; i++)
		IXGBE_WRITE_REG(hw, IXGBE_MTA(i), 0);

	if (hw->mac.ops.init_uta_tables)
		hw->mac.ops.init_uta_tables(hw);

	return 0;
}


static void ixgbe_add_uc_addr(struct ixgbe_hw *hw, u8 *addr, u32 vmdq)
{
	u32 rar_entries = hw->mac.num_rar_entries;
	u32 rar;

	hw_dbg(hw, " UC Addr = %.2X %.2X %.2X %.2X %.2X %.2X\n",
	          addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	
	if (hw->addr_ctrl.rar_used_count < rar_entries) {
		rar = hw->addr_ctrl.rar_used_count -
		      hw->addr_ctrl.mc_addr_in_rar_count;
		hw->mac.ops.set_rar(hw, rar, addr, vmdq, IXGBE_RAH_AV);
		hw_dbg(hw, "Added a secondary address to RAR[%d]\n", rar);
		hw->addr_ctrl.rar_used_count++;
	} else {
		hw->addr_ctrl.overflow_promisc++;
	}

	hw_dbg(hw, "ixgbe_add_uc_addr Complete\n");
}


s32 ixgbe_update_uc_addr_list_generic(struct ixgbe_hw *hw,
				      struct list_head *uc_list)
{
	u32 i;
	u32 old_promisc_setting = hw->addr_ctrl.overflow_promisc;
	u32 uc_addr_in_use;
	u32 fctrl;
	struct netdev_hw_addr *ha;

	
	uc_addr_in_use = hw->addr_ctrl.rar_used_count - 1;
	hw->addr_ctrl.rar_used_count -= uc_addr_in_use;
	hw->addr_ctrl.overflow_promisc = 0;

	
	hw_dbg(hw, "Clearing RAR[1-%d]\n", uc_addr_in_use);
	for (i = 1; i <= uc_addr_in_use; i++) {
		IXGBE_WRITE_REG(hw, IXGBE_RAL(i), 0);
		IXGBE_WRITE_REG(hw, IXGBE_RAH(i), 0);
	}

	
	list_for_each_entry(ha, uc_list, list) {
		hw_dbg(hw, " Adding the secondary addresses:\n");
		ixgbe_add_uc_addr(hw, ha->addr, 0);
	}

	if (hw->addr_ctrl.overflow_promisc) {
		
		if (!old_promisc_setting && !hw->addr_ctrl.user_set_promisc) {
			hw_dbg(hw, " Entering address overflow promisc mode\n");
			fctrl = IXGBE_READ_REG(hw, IXGBE_FCTRL);
			fctrl |= IXGBE_FCTRL_UPE;
			IXGBE_WRITE_REG(hw, IXGBE_FCTRL, fctrl);
		}
	} else {
		
		if (old_promisc_setting && !hw->addr_ctrl.user_set_promisc) {
			hw_dbg(hw, " Leaving address overflow promisc mode\n");
			fctrl = IXGBE_READ_REG(hw, IXGBE_FCTRL);
			fctrl &= ~IXGBE_FCTRL_UPE;
			IXGBE_WRITE_REG(hw, IXGBE_FCTRL, fctrl);
		}
	}

	hw_dbg(hw, "ixgbe_update_uc_addr_list_generic Complete\n");
	return 0;
}


static s32 ixgbe_mta_vector(struct ixgbe_hw *hw, u8 *mc_addr)
{
	u32 vector = 0;

	switch (hw->mac.mc_filter_type) {
	case 0:   
		vector = ((mc_addr[4] >> 4) | (((u16)mc_addr[5]) << 4));
		break;
	case 1:   
		vector = ((mc_addr[4] >> 3) | (((u16)mc_addr[5]) << 5));
		break;
	case 2:   
		vector = ((mc_addr[4] >> 2) | (((u16)mc_addr[5]) << 6));
		break;
	case 3:   
		vector = ((mc_addr[4]) | (((u16)mc_addr[5]) << 8));
		break;
	default:  
		hw_dbg(hw, "MC filter type param set incorrectly\n");
		break;
	}

	
	vector &= 0xFFF;
	return vector;
}


static void ixgbe_set_mta(struct ixgbe_hw *hw, u8 *mc_addr)
{
	u32 vector;
	u32 vector_bit;
	u32 vector_reg;
	u32 mta_reg;

	hw->addr_ctrl.mta_in_use++;

	vector = ixgbe_mta_vector(hw, mc_addr);
	hw_dbg(hw, " bit-vector = 0x%03X\n", vector);

	
	vector_reg = (vector >> 5) & 0x7F;
	vector_bit = vector & 0x1F;
	mta_reg = IXGBE_READ_REG(hw, IXGBE_MTA(vector_reg));
	mta_reg |= (1 << vector_bit);
	IXGBE_WRITE_REG(hw, IXGBE_MTA(vector_reg), mta_reg);
}


s32 ixgbe_update_mc_addr_list_generic(struct ixgbe_hw *hw, u8 *mc_addr_list,
                                      u32 mc_addr_count, ixgbe_mc_addr_itr next)
{
	u32 i;
	u32 vmdq;

	
	hw->addr_ctrl.num_mc_addrs = mc_addr_count;
	hw->addr_ctrl.mta_in_use = 0;

	
	hw_dbg(hw, " Clearing MTA\n");
	for (i = 0; i < hw->mac.mcft_size; i++)
		IXGBE_WRITE_REG(hw, IXGBE_MTA(i), 0);

	
	for (i = 0; i < mc_addr_count; i++) {
		hw_dbg(hw, " Adding the multicast addresses:\n");
		ixgbe_set_mta(hw, next(hw, &mc_addr_list, &vmdq));
	}

	
	if (hw->addr_ctrl.mta_in_use > 0)
		IXGBE_WRITE_REG(hw, IXGBE_MCSTCTRL,
		                IXGBE_MCSTCTRL_MFE | hw->mac.mc_filter_type);

	hw_dbg(hw, "ixgbe_update_mc_addr_list_generic Complete\n");
	return 0;
}


s32 ixgbe_enable_mc_generic(struct ixgbe_hw *hw)
{
	u32 i;
	u32 rar_entries = hw->mac.num_rar_entries;
	struct ixgbe_addr_filter_info *a = &hw->addr_ctrl;

	if (a->mc_addr_in_rar_count > 0)
		for (i = (rar_entries - a->mc_addr_in_rar_count);
		     i < rar_entries; i++)
			ixgbe_enable_rar(hw, i);

	if (a->mta_in_use > 0)
		IXGBE_WRITE_REG(hw, IXGBE_MCSTCTRL, IXGBE_MCSTCTRL_MFE |
		                hw->mac.mc_filter_type);

	return 0;
}


s32 ixgbe_disable_mc_generic(struct ixgbe_hw *hw)
{
	u32 i;
	u32 rar_entries = hw->mac.num_rar_entries;
	struct ixgbe_addr_filter_info *a = &hw->addr_ctrl;

	if (a->mc_addr_in_rar_count > 0)
		for (i = (rar_entries - a->mc_addr_in_rar_count);
		     i < rar_entries; i++)
			ixgbe_disable_rar(hw, i);

	if (a->mta_in_use > 0)
		IXGBE_WRITE_REG(hw, IXGBE_MCSTCTRL, hw->mac.mc_filter_type);

	return 0;
}


s32 ixgbe_fc_enable_generic(struct ixgbe_hw *hw, s32 packetbuf_num)
{
	s32 ret_val = 0;
	u32 mflcn_reg, fccfg_reg;
	u32 reg;
	u32 rx_pba_size;

#ifdef CONFIG_DCB
	if (hw->fc.requested_mode == ixgbe_fc_pfc)
		goto out;

#endif 
	
	ret_val = ixgbe_fc_autoneg(hw);
	if (ret_val)
		goto out;

	
	mflcn_reg = IXGBE_READ_REG(hw, IXGBE_MFLCN);
	mflcn_reg &= ~(IXGBE_MFLCN_RFCE | IXGBE_MFLCN_RPFCE);

	fccfg_reg = IXGBE_READ_REG(hw, IXGBE_FCCFG);
	fccfg_reg &= ~(IXGBE_FCCFG_TFCE_802_3X | IXGBE_FCCFG_TFCE_PRIORITY);

	
	switch (hw->fc.current_mode) {
	case ixgbe_fc_none:
		
		break;
	case ixgbe_fc_rx_pause:
		
		mflcn_reg |= IXGBE_MFLCN_RFCE;
		break;
	case ixgbe_fc_tx_pause:
		
		fccfg_reg |= IXGBE_FCCFG_TFCE_802_3X;
		break;
	case ixgbe_fc_full:
		
		mflcn_reg |= IXGBE_MFLCN_RFCE;
		fccfg_reg |= IXGBE_FCCFG_TFCE_802_3X;
		break;
#ifdef CONFIG_DCB
	case ixgbe_fc_pfc:
		goto out;
		break;
#endif 
	default:
		hw_dbg(hw, "Flow control param set incorrectly\n");
		ret_val = IXGBE_ERR_CONFIG;
		goto out;
		break;
	}

	
	mflcn_reg |= IXGBE_MFLCN_DPF;
	IXGBE_WRITE_REG(hw, IXGBE_MFLCN, mflcn_reg);
	IXGBE_WRITE_REG(hw, IXGBE_FCCFG, fccfg_reg);

	reg = IXGBE_READ_REG(hw, IXGBE_MTQC);
	
	if (reg & IXGBE_MTQC_RT_ENA) {
		rx_pba_size = IXGBE_READ_REG(hw, IXGBE_RXPBSIZE(packetbuf_num));

		
		reg = (rx_pba_size >> 5) & 0xFFE0;
		IXGBE_WRITE_REG(hw, IXGBE_FCRTL_82599(packetbuf_num), reg);

		reg = (rx_pba_size >> 2) & 0xFFE0;
		if (hw->fc.current_mode & ixgbe_fc_tx_pause)
			reg |= IXGBE_FCRTH_FCEN;
		IXGBE_WRITE_REG(hw, IXGBE_FCRTH_82599(packetbuf_num), reg);
	} else {
		
		if (hw->fc.current_mode & ixgbe_fc_tx_pause) {
			if (hw->fc.send_xon) {
				IXGBE_WRITE_REG(hw,
				              IXGBE_FCRTL_82599(packetbuf_num),
			                      (hw->fc.low_water |
				              IXGBE_FCRTL_XONE));
			} else {
				IXGBE_WRITE_REG(hw,
				              IXGBE_FCRTL_82599(packetbuf_num),
				              hw->fc.low_water);
			}

			IXGBE_WRITE_REG(hw, IXGBE_FCRTH_82599(packetbuf_num),
			               (hw->fc.high_water | IXGBE_FCRTH_FCEN));
		}
	}

	
	reg = IXGBE_READ_REG(hw, IXGBE_FCTTV(packetbuf_num / 2));
	if ((packetbuf_num & 1) == 0)
		reg = (reg & 0xFFFF0000) | hw->fc.pause_time;
	else
		reg = (reg & 0x0000FFFF) | (hw->fc.pause_time << 16);
	IXGBE_WRITE_REG(hw, IXGBE_FCTTV(packetbuf_num / 2), reg);

	IXGBE_WRITE_REG(hw, IXGBE_FCRTV, (hw->fc.pause_time >> 1));

out:
	return ret_val;
}


s32 ixgbe_fc_autoneg(struct ixgbe_hw *hw)
{
	s32 ret_val = 0;
	ixgbe_link_speed speed;
	u32 pcs_anadv_reg, pcs_lpab_reg, linkstat;
	u32 links2, anlp1_reg, autoc_reg, links;
	bool link_up;

	
	hw->mac.ops.check_link(hw, &speed, &link_up, false);

	if (hw->fc.disable_fc_autoneg || (!link_up)) {
		hw->fc.fc_was_autonegged = false;
		hw->fc.current_mode = hw->fc.requested_mode;
		goto out;
	}

	
	if (hw->phy.media_type == ixgbe_media_type_backplane) {
		links = IXGBE_READ_REG(hw, IXGBE_LINKS);
		links2 = IXGBE_READ_REG(hw, IXGBE_LINKS2);
		if (((links & IXGBE_LINKS_KX_AN_COMP) == 0) ||
		    ((links2 & IXGBE_LINKS2_AN_SUPPORTED) == 0)) {
			hw->fc.fc_was_autonegged = false;
			hw->fc.current_mode = hw->fc.requested_mode;
			goto out;
		}
	}

	
	if (hw->phy.multispeed_fiber && (speed == IXGBE_LINK_SPEED_1GB_FULL)) {
		linkstat = IXGBE_READ_REG(hw, IXGBE_PCS1GLSTA);
		if (((linkstat & IXGBE_PCS1GLSTA_AN_COMPLETE) == 0) ||
		    ((linkstat & IXGBE_PCS1GLSTA_AN_TIMED_OUT) == 1)) {
			hw->fc.fc_was_autonegged = false;
			hw->fc.current_mode = hw->fc.requested_mode;
			goto out;
		}
	}

	
	if ((speed == IXGBE_LINK_SPEED_1GB_FULL) &&
	    (hw->phy.media_type != ixgbe_media_type_backplane)) {
		pcs_anadv_reg = IXGBE_READ_REG(hw, IXGBE_PCS1GANA);
		pcs_lpab_reg = IXGBE_READ_REG(hw, IXGBE_PCS1GANLP);
		if ((pcs_anadv_reg & IXGBE_PCS1GANA_SYM_PAUSE) &&
		    (pcs_lpab_reg & IXGBE_PCS1GANA_SYM_PAUSE)) {
			
			if (hw->fc.requested_mode == ixgbe_fc_full) {
				hw->fc.current_mode = ixgbe_fc_full;
				hw_dbg(hw, "Flow Control = FULL.\n");
			} else {
				hw->fc.current_mode = ixgbe_fc_rx_pause;
				hw_dbg(hw, "Flow Control=RX PAUSE only\n");
			}
		} else if (!(pcs_anadv_reg & IXGBE_PCS1GANA_SYM_PAUSE) &&
			   (pcs_anadv_reg & IXGBE_PCS1GANA_ASM_PAUSE) &&
			   (pcs_lpab_reg & IXGBE_PCS1GANA_SYM_PAUSE) &&
			   (pcs_lpab_reg & IXGBE_PCS1GANA_ASM_PAUSE)) {
			hw->fc.current_mode = ixgbe_fc_tx_pause;
			hw_dbg(hw, "Flow Control = TX PAUSE frames only.\n");
		} else if ((pcs_anadv_reg & IXGBE_PCS1GANA_SYM_PAUSE) &&
			   (pcs_anadv_reg & IXGBE_PCS1GANA_ASM_PAUSE) &&
			   !(pcs_lpab_reg & IXGBE_PCS1GANA_SYM_PAUSE) &&
			   (pcs_lpab_reg & IXGBE_PCS1GANA_ASM_PAUSE)) {
			hw->fc.current_mode = ixgbe_fc_rx_pause;
			hw_dbg(hw, "Flow Control = RX PAUSE frames only.\n");
		} else {
			hw->fc.current_mode = ixgbe_fc_none;
			hw_dbg(hw, "Flow Control = NONE.\n");
		}
	}

	if (hw->phy.media_type == ixgbe_media_type_backplane) {
		
		autoc_reg = IXGBE_READ_REG(hw, IXGBE_AUTOC);
		anlp1_reg = IXGBE_READ_REG(hw, IXGBE_ANLP1);

		if ((autoc_reg & IXGBE_AUTOC_SYM_PAUSE) &&
		    (anlp1_reg & IXGBE_ANLP1_SYM_PAUSE)) {
			
			if (hw->fc.requested_mode == ixgbe_fc_full) {
				hw->fc.current_mode = ixgbe_fc_full;
				hw_dbg(hw, "Flow Control = FULL.\n");
			} else {
				hw->fc.current_mode = ixgbe_fc_rx_pause;
				hw_dbg(hw, "Flow Control=RX PAUSE only\n");
			}
		} else if (!(autoc_reg & IXGBE_AUTOC_SYM_PAUSE) &&
			   (autoc_reg & IXGBE_AUTOC_ASM_PAUSE) &&
			   (anlp1_reg & IXGBE_ANLP1_SYM_PAUSE) &&
			   (anlp1_reg & IXGBE_ANLP1_ASM_PAUSE)) {
			hw->fc.current_mode = ixgbe_fc_tx_pause;
			hw_dbg(hw, "Flow Control = TX PAUSE frames only.\n");
		} else if ((autoc_reg & IXGBE_AUTOC_SYM_PAUSE) &&
			   (autoc_reg & IXGBE_AUTOC_ASM_PAUSE) &&
			   !(anlp1_reg & IXGBE_ANLP1_SYM_PAUSE) &&
			   (anlp1_reg & IXGBE_ANLP1_ASM_PAUSE)) {
			hw->fc.current_mode = ixgbe_fc_rx_pause;
			hw_dbg(hw, "Flow Control = RX PAUSE frames only.\n");
		} else {
			hw->fc.current_mode = ixgbe_fc_none;
			hw_dbg(hw, "Flow Control = NONE.\n");
		}
	}
	
	hw->fc.fc_was_autonegged = true;

out:
	return ret_val;
}


static s32 ixgbe_setup_fc(struct ixgbe_hw *hw, s32 packetbuf_num)
{
	s32 ret_val = 0;
	u32 reg;

#ifdef CONFIG_DCB
	if (hw->fc.requested_mode == ixgbe_fc_pfc) {
		hw->fc.current_mode = hw->fc.requested_mode;
		goto out;
	}

#endif
	
	if (packetbuf_num < 0 || packetbuf_num > 7) {
		hw_dbg(hw, "Invalid packet buffer number [%d], expected range "
		       "is 0-7\n", packetbuf_num);
		ret_val = IXGBE_ERR_INVALID_LINK_SETTINGS;
		goto out;
	}

	
	if (!hw->fc.low_water || !hw->fc.high_water || !hw->fc.pause_time) {
		hw_dbg(hw, "Invalid water mark configuration\n");
		ret_val = IXGBE_ERR_INVALID_LINK_SETTINGS;
		goto out;
	}

	
	if (hw->fc.strict_ieee && hw->fc.requested_mode == ixgbe_fc_rx_pause) {
		hw_dbg(hw, "ixgbe_fc_rx_pause not valid in strict "
		       "IEEE mode\n");
		ret_val = IXGBE_ERR_INVALID_LINK_SETTINGS;
		goto out;
	}

	
	if (hw->fc.requested_mode == ixgbe_fc_default)
		hw->fc.requested_mode = ixgbe_fc_full;

	
	reg = IXGBE_READ_REG(hw, IXGBE_PCS1GANA);

	
	switch (hw->fc.requested_mode) {
	case ixgbe_fc_none:
		
		reg &= ~(IXGBE_PCS1GANA_SYM_PAUSE | IXGBE_PCS1GANA_ASM_PAUSE);
		break;
	case ixgbe_fc_rx_pause:
		
		reg |= (IXGBE_PCS1GANA_SYM_PAUSE | IXGBE_PCS1GANA_ASM_PAUSE);
		break;
	case ixgbe_fc_tx_pause:
		
		reg |= (IXGBE_PCS1GANA_ASM_PAUSE);
		reg &= ~(IXGBE_PCS1GANA_SYM_PAUSE);
		break;
	case ixgbe_fc_full:
		
		reg |= (IXGBE_PCS1GANA_SYM_PAUSE | IXGBE_PCS1GANA_ASM_PAUSE);
		break;
#ifdef CONFIG_DCB
	case ixgbe_fc_pfc:
		goto out;
		break;
#endif 
	default:
		hw_dbg(hw, "Flow control param set incorrectly\n");
		ret_val = IXGBE_ERR_CONFIG;
		goto out;
		break;
	}

	IXGBE_WRITE_REG(hw, IXGBE_PCS1GANA, reg);
	reg = IXGBE_READ_REG(hw, IXGBE_PCS1GLCTL);

	
	if (hw->fc.strict_ieee)
		reg &= ~IXGBE_PCS1GLCTL_AN_1G_TIMEOUT_EN;

	IXGBE_WRITE_REG(hw, IXGBE_PCS1GLCTL, reg);
	hw_dbg(hw, "Set up FC; PCS1GLCTL = 0x%08X\n", reg);

	
	reg = IXGBE_READ_REG(hw, IXGBE_AUTOC);

	
	switch (hw->fc.requested_mode) {
	case ixgbe_fc_none:
		
		reg &= ~(IXGBE_AUTOC_SYM_PAUSE | IXGBE_AUTOC_ASM_PAUSE);
		break;
	case ixgbe_fc_rx_pause:
		
		reg |= (IXGBE_AUTOC_SYM_PAUSE | IXGBE_AUTOC_ASM_PAUSE);
		break;
	case ixgbe_fc_tx_pause:
		
		reg |= (IXGBE_AUTOC_ASM_PAUSE);
		reg &= ~(IXGBE_AUTOC_SYM_PAUSE);
		break;
	case ixgbe_fc_full:
		
		reg |= (IXGBE_AUTOC_SYM_PAUSE | IXGBE_AUTOC_ASM_PAUSE);
		break;
#ifdef CONFIG_DCB
	case ixgbe_fc_pfc:
		goto out;
		break;
#endif 
	default:
		hw_dbg(hw, "Flow control param set incorrectly\n");
		ret_val = IXGBE_ERR_CONFIG;
		goto out;
		break;
	}
	
	reg |= IXGBE_AUTOC_AN_RESTART;
	IXGBE_WRITE_REG(hw, IXGBE_AUTOC, reg);
	hw_dbg(hw, "Set up FC; IXGBE_AUTOC = 0x%08X\n", reg);

out:
	return ret_val;
}


s32 ixgbe_disable_pcie_master(struct ixgbe_hw *hw)
{
	u32 i;
	u32 reg_val;
	u32 number_of_queues;
	s32 status = IXGBE_ERR_MASTER_REQUESTS_PENDING;

	
	number_of_queues = hw->mac.max_rx_queues;
	for (i = 0; i < number_of_queues; i++) {
		reg_val = IXGBE_READ_REG(hw, IXGBE_RXDCTL(i));
		if (reg_val & IXGBE_RXDCTL_ENABLE) {
			reg_val &= ~IXGBE_RXDCTL_ENABLE;
			IXGBE_WRITE_REG(hw, IXGBE_RXDCTL(i), reg_val);
		}
	}

	reg_val = IXGBE_READ_REG(hw, IXGBE_CTRL);
	reg_val |= IXGBE_CTRL_GIO_DIS;
	IXGBE_WRITE_REG(hw, IXGBE_CTRL, reg_val);

	for (i = 0; i < IXGBE_PCI_MASTER_DISABLE_TIMEOUT; i++) {
		if (!(IXGBE_READ_REG(hw, IXGBE_STATUS) & IXGBE_STATUS_GIO)) {
			status = 0;
			break;
		}
		udelay(100);
	}

	return status;
}



s32 ixgbe_acquire_swfw_sync(struct ixgbe_hw *hw, u16 mask)
{
	u32 gssr;
	u32 swmask = mask;
	u32 fwmask = mask << 5;
	s32 timeout = 200;

	while (timeout) {
		if (ixgbe_get_eeprom_semaphore(hw))
			return IXGBE_ERR_SWFW_SYNC;

		gssr = IXGBE_READ_REG(hw, IXGBE_GSSR);
		if (!(gssr & (fwmask | swmask)))
			break;

		
		ixgbe_release_eeprom_semaphore(hw);
		msleep(5);
		timeout--;
	}

	if (!timeout) {
		hw_dbg(hw, "Driver can't access resource, GSSR timeout.\n");
		return IXGBE_ERR_SWFW_SYNC;
	}

	gssr |= swmask;
	IXGBE_WRITE_REG(hw, IXGBE_GSSR, gssr);

	ixgbe_release_eeprom_semaphore(hw);
	return 0;
}


void ixgbe_release_swfw_sync(struct ixgbe_hw *hw, u16 mask)
{
	u32 gssr;
	u32 swmask = mask;

	ixgbe_get_eeprom_semaphore(hw);

	gssr = IXGBE_READ_REG(hw, IXGBE_GSSR);
	gssr &= ~swmask;
	IXGBE_WRITE_REG(hw, IXGBE_GSSR, gssr);

	ixgbe_release_eeprom_semaphore(hw);
}


s32 ixgbe_enable_rx_dma_generic(struct ixgbe_hw *hw, u32 regval)
{
	IXGBE_WRITE_REG(hw, IXGBE_RXCTRL, regval);

	return 0;
}


s32 ixgbe_blink_led_start_generic(struct ixgbe_hw *hw, u32 index)
{
	ixgbe_link_speed speed = 0;
	bool link_up = 0;
	u32 autoc_reg = IXGBE_READ_REG(hw, IXGBE_AUTOC);
	u32 led_reg = IXGBE_READ_REG(hw, IXGBE_LEDCTL);

	
	hw->mac.ops.check_link(hw, &speed, &link_up, false);

	if (!link_up) {
		autoc_reg |= IXGBE_AUTOC_AN_RESTART;
		autoc_reg |= IXGBE_AUTOC_FLU;
		IXGBE_WRITE_REG(hw, IXGBE_AUTOC, autoc_reg);
		msleep(10);
	}

	led_reg &= ~IXGBE_LED_MODE_MASK(index);
	led_reg |= IXGBE_LED_BLINK(index);
	IXGBE_WRITE_REG(hw, IXGBE_LEDCTL, led_reg);
	IXGBE_WRITE_FLUSH(hw);

	return 0;
}


s32 ixgbe_blink_led_stop_generic(struct ixgbe_hw *hw, u32 index)
{
	u32 autoc_reg = IXGBE_READ_REG(hw, IXGBE_AUTOC);
	u32 led_reg = IXGBE_READ_REG(hw, IXGBE_LEDCTL);

	autoc_reg &= ~IXGBE_AUTOC_FLU;
	autoc_reg |= IXGBE_AUTOC_AN_RESTART;
	IXGBE_WRITE_REG(hw, IXGBE_AUTOC, autoc_reg);

	led_reg &= ~IXGBE_LED_MODE_MASK(index);
	led_reg &= ~IXGBE_LED_BLINK(index);
	led_reg |= IXGBE_LED_LINK_ACTIVE << IXGBE_LED_MODE_SHIFT(index);
	IXGBE_WRITE_REG(hw, IXGBE_LEDCTL, led_reg);
	IXGBE_WRITE_FLUSH(hw);

	return 0;
}
