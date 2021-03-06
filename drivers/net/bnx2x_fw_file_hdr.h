

#ifndef BNX2X_INIT_FILE_HDR_H
#define BNX2X_INIT_FILE_HDR_H

struct bnx2x_fw_file_section {
	__be32 len;
	__be32 offset;
};

struct bnx2x_fw_file_hdr {
	struct bnx2x_fw_file_section init_ops;
	struct bnx2x_fw_file_section init_ops_offsets;
	struct bnx2x_fw_file_section init_data;
	struct bnx2x_fw_file_section tsem_int_table_data;
	struct bnx2x_fw_file_section tsem_pram_data;
	struct bnx2x_fw_file_section usem_int_table_data;
	struct bnx2x_fw_file_section usem_pram_data;
	struct bnx2x_fw_file_section csem_int_table_data;
	struct bnx2x_fw_file_section csem_pram_data;
	struct bnx2x_fw_file_section xsem_int_table_data;
	struct bnx2x_fw_file_section xsem_pram_data;
	struct bnx2x_fw_file_section fw_version;
};

#endif 
