

#ifndef _dhdu_cmd_h_
#define _dhdu_cmd_h_

typedef struct cmd cmd_t;
typedef int (cmd_func_t)(void *dhd, cmd_t *cmd, char **argv);


struct cmd {
	char *name;
	cmd_func_t *func;
	int get;
	int set;
	char *help;
};


extern cmd_t dhd_cmds[];
extern cmd_t dhd_varcmd;


extern int dhd_get(void *dhd, int cmd, void *buf, int len);
extern int dhd_set(void *dhd, int cmd, void *buf, int len);

#endif 
