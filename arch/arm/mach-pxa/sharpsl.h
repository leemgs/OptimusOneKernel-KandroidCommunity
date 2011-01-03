

#include <mach/sharpsl_pm.h>


struct corgissp_machinfo {
	int port;
	int cs_lcdcon;
	int cs_ads7846;
	int cs_max1111;
	int clk_lcdcon;
	int clk_ads7846;
	int clk_max1111;
};

void corgi_ssp_set_machinfo(struct corgissp_machinfo *machinfo);



void corgi_lcdtg_suspend(void);
void corgi_lcdtg_hw_init(int mode);



#define READ_GPIO_BIT(x)    (GPLR(x) & GPIO_bit(x))


#define MAX1111_BATT_VOLT   4u
#define MAX1111_BATT_TEMP   2u
#define MAX1111_ACIN_VOLT   6u

extern struct battery_thresh spitz_battery_levels_acin[];
extern struct battery_thresh spitz_battery_levels_noac[];
int sharpsl_pm_pxa_read_max1111(int channel);


