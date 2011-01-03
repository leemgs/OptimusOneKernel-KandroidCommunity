

struct sam9_smc_config {
	
	u8 ncs_read_setup;
	u8 nrd_setup;
	u8 ncs_write_setup;
	u8 nwe_setup;

	
	u8 ncs_read_pulse;
	u8 nrd_pulse;
	u8 ncs_write_pulse;
	u8 nwe_pulse;

	
	u16 read_cycle;
	u16 write_cycle;

	
	u32 mode;
	u8 tdf_cycles:4;
};

extern void __init sam9_smc_configure(int cs, struct sam9_smc_config* config);
