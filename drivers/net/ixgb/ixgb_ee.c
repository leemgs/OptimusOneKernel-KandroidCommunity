

#include "ixgb_hw.h"
#include "ixgb_ee.h"

static u16 ixgb_shift_in_bits(struct ixgb_hw *hw);

static void ixgb_shift_out_bits(struct ixgb_hw *hw,
				u16 data,
				u16 count);
static void ixgb_standby_eeprom(struct ixgb_hw *hw);

static bool ixgb_wait_eeprom_command(struct ixgb_hw *hw);

static void ixgb_cleanup_eeprom(struct ixgb_hw *hw);


static void
ixgb_raise_clock(struct ixgb_hw *hw,
		  u32 *eecd_reg)
{
	
	*eecd_reg = *eecd_reg | IXGB_EECD_SK;
	IXGB_WRITE_REG(hw, EECD, *eecd_reg);
	udelay(50);
	return;
}


static void
ixgb_lower_clock(struct ixgb_hw *hw,
		  u32 *eecd_reg)
{
	
	*eecd_reg = *eecd_reg & ~IXGB_EECD_SK;
	IXGB_WRITE_REG(hw, EECD, *eecd_reg);
	udelay(50);
	return;
}


static void
ixgb_shift_out_bits(struct ixgb_hw *hw,
					 u16 data,
					 u16 count)
{
	u32 eecd_reg;
	u32 mask;

	
	mask = 0x01 << (count - 1);
	eecd_reg = IXGB_READ_REG(hw, EECD);
	eecd_reg &= ~(IXGB_EECD_DO | IXGB_EECD_DI);
	do {
		
		eecd_reg &= ~IXGB_EECD_DI;

		if (data & mask)
			eecd_reg |= IXGB_EECD_DI;

		IXGB_WRITE_REG(hw, EECD, eecd_reg);

		udelay(50);

		ixgb_raise_clock(hw, &eecd_reg);
		ixgb_lower_clock(hw, &eecd_reg);

		mask = mask >> 1;

	} while (mask);

	
	eecd_reg &= ~IXGB_EECD_DI;
	IXGB_WRITE_REG(hw, EECD, eecd_reg);
	return;
}


static u16
ixgb_shift_in_bits(struct ixgb_hw *hw)
{
	u32 eecd_reg;
	u32 i;
	u16 data;

	

	eecd_reg = IXGB_READ_REG(hw, EECD);

	eecd_reg &= ~(IXGB_EECD_DO | IXGB_EECD_DI);
	data = 0;

	for (i = 0; i < 16; i++) {
		data = data << 1;
		ixgb_raise_clock(hw, &eecd_reg);

		eecd_reg = IXGB_READ_REG(hw, EECD);

		eecd_reg &= ~(IXGB_EECD_DI);
		if (eecd_reg & IXGB_EECD_DO)
			data |= 1;

		ixgb_lower_clock(hw, &eecd_reg);
	}

	return data;
}


static void
ixgb_setup_eeprom(struct ixgb_hw *hw)
{
	u32 eecd_reg;

	eecd_reg = IXGB_READ_REG(hw, EECD);

	
	eecd_reg &= ~(IXGB_EECD_SK | IXGB_EECD_DI);
	IXGB_WRITE_REG(hw, EECD, eecd_reg);

	
	eecd_reg |= IXGB_EECD_CS;
	IXGB_WRITE_REG(hw, EECD, eecd_reg);
	return;
}


static void
ixgb_standby_eeprom(struct ixgb_hw *hw)
{
	u32 eecd_reg;

	eecd_reg = IXGB_READ_REG(hw, EECD);

	
	eecd_reg &= ~(IXGB_EECD_CS | IXGB_EECD_SK);
	IXGB_WRITE_REG(hw, EECD, eecd_reg);
	udelay(50);

	
	eecd_reg |= IXGB_EECD_SK;
	IXGB_WRITE_REG(hw, EECD, eecd_reg);
	udelay(50);

	
	eecd_reg |= IXGB_EECD_CS;
	IXGB_WRITE_REG(hw, EECD, eecd_reg);
	udelay(50);

	
	eecd_reg &= ~IXGB_EECD_SK;
	IXGB_WRITE_REG(hw, EECD, eecd_reg);
	udelay(50);
	return;
}


static void
ixgb_clock_eeprom(struct ixgb_hw *hw)
{
	u32 eecd_reg;

	eecd_reg = IXGB_READ_REG(hw, EECD);

	
	eecd_reg |= IXGB_EECD_SK;
	IXGB_WRITE_REG(hw, EECD, eecd_reg);
	udelay(50);

	
	eecd_reg &= ~IXGB_EECD_SK;
	IXGB_WRITE_REG(hw, EECD, eecd_reg);
	udelay(50);
	return;
}


static void
ixgb_cleanup_eeprom(struct ixgb_hw *hw)
{
	u32 eecd_reg;

	eecd_reg = IXGB_READ_REG(hw, EECD);

	eecd_reg &= ~(IXGB_EECD_CS | IXGB_EECD_DI);

	IXGB_WRITE_REG(hw, EECD, eecd_reg);

	ixgb_clock_eeprom(hw);
	return;
}


static bool
ixgb_wait_eeprom_command(struct ixgb_hw *hw)
{
	u32 eecd_reg;
	u32 i;

	
	ixgb_standby_eeprom(hw);

	
	for (i = 0; i < 200; i++) {
		eecd_reg = IXGB_READ_REG(hw, EECD);

		if (eecd_reg & IXGB_EECD_DO)
			return (true);

		udelay(50);
	}
	ASSERT(0);
	return (false);
}


bool
ixgb_validate_eeprom_checksum(struct ixgb_hw *hw)
{
	u16 checksum = 0;
	u16 i;

	for (i = 0; i < (EEPROM_CHECKSUM_REG + 1); i++)
		checksum += ixgb_read_eeprom(hw, i);

	if (checksum == (u16) EEPROM_SUM)
		return (true);
	else
		return (false);
}


void
ixgb_update_eeprom_checksum(struct ixgb_hw *hw)
{
	u16 checksum = 0;
	u16 i;

	for (i = 0; i < EEPROM_CHECKSUM_REG; i++)
		checksum += ixgb_read_eeprom(hw, i);

	checksum = (u16) EEPROM_SUM - checksum;

	ixgb_write_eeprom(hw, EEPROM_CHECKSUM_REG, checksum);
	return;
}


void
ixgb_write_eeprom(struct ixgb_hw *hw, u16 offset, u16 data)
{
	struct ixgb_ee_map_type *ee_map = (struct ixgb_ee_map_type *)hw->eeprom;

	
	ixgb_setup_eeprom(hw);

	
	ixgb_shift_out_bits(hw, EEPROM_EWEN_OPCODE, 5);
	ixgb_shift_out_bits(hw, 0, 4);

	
	ixgb_standby_eeprom(hw);

	
	ixgb_shift_out_bits(hw, EEPROM_WRITE_OPCODE, 3);
	ixgb_shift_out_bits(hw, offset, 6);

	
	ixgb_shift_out_bits(hw, data, 16);

	ixgb_wait_eeprom_command(hw);

	
	ixgb_standby_eeprom(hw);

	
	ixgb_shift_out_bits(hw, EEPROM_EWDS_OPCODE, 5);
	ixgb_shift_out_bits(hw, 0, 4);

	
	ixgb_cleanup_eeprom(hw);

	
	ee_map->init_ctrl_reg_1 = cpu_to_le16(EEPROM_ICW1_SIGNATURE_CLEAR);

	return;
}


u16
ixgb_read_eeprom(struct ixgb_hw *hw,
		  u16 offset)
{
	u16 data;

	
	ixgb_setup_eeprom(hw);

	
	ixgb_shift_out_bits(hw, EEPROM_READ_OPCODE, 3);
	
	ixgb_shift_out_bits(hw, offset, 6);

	
	data = ixgb_shift_in_bits(hw);

	
	ixgb_standby_eeprom(hw);

	return (data);
}


bool
ixgb_get_eeprom_data(struct ixgb_hw *hw)
{
	u16 i;
	u16 checksum = 0;
	struct ixgb_ee_map_type *ee_map;

	DEBUGFUNC("ixgb_get_eeprom_data");

	ee_map = (struct ixgb_ee_map_type *)hw->eeprom;

	DEBUGOUT("ixgb_ee: Reading eeprom data\n");
	for (i = 0; i < IXGB_EEPROM_SIZE ; i++) {
		u16 ee_data;
		ee_data = ixgb_read_eeprom(hw, i);
		checksum += ee_data;
		hw->eeprom[i] = cpu_to_le16(ee_data);
	}

	if (checksum != (u16) EEPROM_SUM) {
		DEBUGOUT("ixgb_ee: Checksum invalid.\n");
		
		ee_map->init_ctrl_reg_1 = cpu_to_le16(EEPROM_ICW1_SIGNATURE_CLEAR);
		return (false);
	}

	if ((ee_map->init_ctrl_reg_1 & cpu_to_le16(EEPROM_ICW1_SIGNATURE_MASK))
		 != cpu_to_le16(EEPROM_ICW1_SIGNATURE_VALID)) {
		DEBUGOUT("ixgb_ee: Signature invalid.\n");
		return(false);
	}

	return(true);
}


static bool
ixgb_check_and_get_eeprom_data (struct ixgb_hw* hw)
{
	struct ixgb_ee_map_type *ee_map = (struct ixgb_ee_map_type *)hw->eeprom;

	if ((ee_map->init_ctrl_reg_1 & cpu_to_le16(EEPROM_ICW1_SIGNATURE_MASK))
	    == cpu_to_le16(EEPROM_ICW1_SIGNATURE_VALID)) {
		return (true);
	} else {
		return ixgb_get_eeprom_data(hw);
	}
}


__le16
ixgb_get_eeprom_word(struct ixgb_hw *hw, u16 index)
{

	if ((index < IXGB_EEPROM_SIZE) &&
		(ixgb_check_and_get_eeprom_data(hw) == true)) {
	   return(hw->eeprom[index]);
	}

	return(0);
}


void
ixgb_get_ee_mac_addr(struct ixgb_hw *hw,
			u8 *mac_addr)
{
	int i;
	struct ixgb_ee_map_type *ee_map = (struct ixgb_ee_map_type *)hw->eeprom;

	DEBUGFUNC("ixgb_get_ee_mac_addr");

	if (ixgb_check_and_get_eeprom_data(hw) == true) {
		for (i = 0; i < IXGB_ETH_LENGTH_OF_ADDRESS; i++) {
			mac_addr[i] = ee_map->mac_addr[i];
			DEBUGOUT2("mac(%d) = %.2X\n", i, mac_addr[i]);
		}
	}
}



u32
ixgb_get_ee_pba_number(struct ixgb_hw *hw)
{
	if (ixgb_check_and_get_eeprom_data(hw) == true)
		return (le16_to_cpu(hw->eeprom[EEPROM_PBA_1_2_REG])
			| (le16_to_cpu(hw->eeprom[EEPROM_PBA_3_4_REG])<<16));

	return(0);
}



u16
ixgb_get_ee_device_id(struct ixgb_hw *hw)
{
	struct ixgb_ee_map_type *ee_map = (struct ixgb_ee_map_type *)hw->eeprom;

	if (ixgb_check_and_get_eeprom_data(hw) == true)
		return (le16_to_cpu(ee_map->device_id));

	return (0);
}

