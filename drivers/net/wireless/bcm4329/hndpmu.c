

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndpmu.h>


#define	PMU_ERROR(args)

#define	PMU_MSG(args)



typedef struct {
	uint8 strength;			
	uint8 sel;			
} sdiod_drive_str_t;


static const sdiod_drive_str_t sdiod_drive_strength_tab1[] = {
	{4, 0x2},
	{2, 0x3},
	{1, 0x0},
	{0, 0x0} };


static const sdiod_drive_str_t sdiod_drive_strength_tab2[] = {
	{12, 0x7},
	{10, 0x6},
	{8, 0x5},
	{6, 0x4},
	{4, 0x2},
	{2, 0x1},
	{0, 0x0} };

#define SDIOD_DRVSTR_KEY(chip, pmu)	(((chip) << 16) | (pmu))

void
si_sdiod_drive_strength_init(si_t *sih, osl_t *osh, uint32 drivestrength)
{
	chipcregs_t *cc;
	uint origidx, intr_val = 0;
	sdiod_drive_str_t *str_tab = NULL;
	uint32 str_mask = 0;
	uint32 str_shift = 0;

	if (!(sih->cccaps & CC_CAP_PMU)) {
		return;
	}

	
	cc = (chipcregs_t *) si_switch_core(sih, CC_CORE_ID, &origidx, &intr_val);

	switch (SDIOD_DRVSTR_KEY(sih->chip, sih->pmurev)) {
	case SDIOD_DRVSTR_KEY(BCM4325_CHIP_ID, 1):
		str_tab = (sdiod_drive_str_t *)&sdiod_drive_strength_tab1;
		str_mask = 0x30000000;
		str_shift = 28;
		break;
	case SDIOD_DRVSTR_KEY(BCM4325_CHIP_ID, 2):
	case SDIOD_DRVSTR_KEY(BCM4325_CHIP_ID, 3):
	case SDIOD_DRVSTR_KEY(BCM4315_CHIP_ID, 4):
		str_tab = (sdiod_drive_str_t *)&sdiod_drive_strength_tab2;
		str_mask = 0x00003800;
		str_shift = 11;
		break;

	default:
		PMU_MSG(("No SDIO Drive strength init done for chip %x rev %d pmurev %d\n",
		         sih->chip, sih->chiprev, sih->pmurev));

		break;
	}

	if (str_tab != NULL) {
		uint32 drivestrength_sel = 0;
		uint32 cc_data_temp;
		int i;

		for (i = 0; str_tab[i].strength != 0; i ++) {
			if (drivestrength >= str_tab[i].strength) {
				drivestrength_sel = str_tab[i].sel;
				break;
			}
		}

		W_REG(osh, &cc->chipcontrol_addr, 1);
		cc_data_temp = R_REG(osh, &cc->chipcontrol_data);
		cc_data_temp &= ~str_mask;
		drivestrength_sel <<= str_shift;
		cc_data_temp |= drivestrength_sel;
		W_REG(osh, &cc->chipcontrol_data, cc_data_temp);

		PMU_MSG(("SDIO: %dmA drive strength selected, set to 0x%08x\n",
		         drivestrength, cc_data_temp));
	}

	
	si_restore_core(sih, origidx, intr_val);
}
