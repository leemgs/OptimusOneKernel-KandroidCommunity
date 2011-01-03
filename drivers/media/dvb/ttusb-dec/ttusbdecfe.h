

#ifndef TTUSBDECFE_H
#define TTUSBDECFE_H

#include <linux/dvb/frontend.h>

struct ttusbdecfe_config
{
	int (*send_command)(struct dvb_frontend* fe, const u8 command,
			    int param_length, const u8 params[],
			    int *result_length, u8 cmd_result[]);
};

extern struct dvb_frontend* ttusbdecfe_dvbs_attach(const struct ttusbdecfe_config* config);

extern struct dvb_frontend* ttusbdecfe_dvbt_attach(const struct ttusbdecfe_config* config);

#endif 
