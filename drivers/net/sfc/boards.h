

#ifndef EFX_BOARDS_H
#define EFX_BOARDS_H


enum efx_board_type {
	EFX_BOARD_SFE4001 = 1,
	EFX_BOARD_SFE4002 = 2,
	EFX_BOARD_SFN4111T = 0x51,
	EFX_BOARD_SFN4112F = 0x52,
};

extern void efx_set_board_info(struct efx_nic *efx, u16 revision_info);


extern int sfe4001_init(struct efx_nic *efx);

extern int sfn4111t_init(struct efx_nic *efx);

#endif
