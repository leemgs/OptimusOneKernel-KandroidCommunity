

#ifndef	_bcmsdpcm_h_
#define	_bcmsdpcm_h_



#define SDPCM_SHARED_VERSION       0x0001
#define SDPCM_SHARED_VERSION_MASK  0x00FF
#define SDPCM_SHARED_ASSERT_BUILT  0x0100
#define SDPCM_SHARED_ASSERT        0x0200
#define SDPCM_SHARED_TRAP          0x0400

typedef struct {
	uint32	flags;
	uint32  trap_addr;
	uint32  assert_exp_addr;
	uint32  assert_file_addr;
	uint32  assert_line;
	uint32	console_addr;		
	uint32  msgtrace_addr;
} sdpcm_shared_t;

extern sdpcm_shared_t sdpcm_shared;


extern void sdpcmd_fwhalt(void);

#endif	
