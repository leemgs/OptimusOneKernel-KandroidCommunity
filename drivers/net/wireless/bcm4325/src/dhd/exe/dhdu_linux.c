

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#ifndef TARGETENV_android
#include <error.h>
#endif 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <typedefs.h>
#include <dhdioctl.h>
#include "dhdu.h"

#define DEV_TYPE_LEN 4 

static void
syserr(char *s)
{
	fprintf(stderr, "%s: ", dhdu_av0);
	perror(s);
	exit(errno);
}

static int
dhd_ioctl(void *dhd, int cmd, void *buf, int len, bool set)
{
	struct ifreq *ifr = (struct ifreq *)dhd;
	dhd_ioctl_t ioc;
	int ret = 0;
	int s;

	
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = set;
	ioc.driver = DHD_IOCTL_MAGIC;
	ifr->ifr_data = (caddr_t) &ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, ifr)) < 0) {
		if (cmd != DHD_GET_MAGIC) {
			ret = IOCTL_ERROR;
		}
	}

	
	close(s);
	return ret;
}

static int
dhd_get_dev_type(char *name, void *buf, int len)
{
	int s;
	int ret;
	struct ifreq ifr;
	struct ethtool_drvinfo info;

	
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	
	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	strcpy(info.driver, "?dhd");
	ifr.ifr_data = (caddr_t)&info;
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {

		
		if (errno == EPERM)
			syserr("dhd_get_dev_type");

		*(char *)buf = '\0';
	}
	else
		strncpy(buf, info.driver, len);

	close(s);
	return ret;
}

int
dhd_get(void *dhd, int cmd, void *buf, int len)
{
	return dhd_ioctl(dhd, cmd, buf, len, FALSE);
}

int
dhd_set(void *dhd, int cmd, void *buf, int len)
{
	return dhd_ioctl(dhd, cmd, buf, len, TRUE);
}

void
dhd_find(struct ifreq *ifr)
{
	char proc_net_dev[] = "/proc/net/dev";
	FILE *fp;
	char buf[1000], *c, *name;
	char dev_type[DEV_TYPE_LEN];

	ifr->ifr_name[0] = '\0';

	
	if (!(fp = fopen(proc_net_dev, "r")) ||
	    !fgets(buf, sizeof(buf), fp) ||
	    !fgets(buf, sizeof(buf), fp))
		return;

	while (fgets(buf, sizeof(buf), fp)) {
		c = buf;
		while (isspace(*c))
			c++;
		if (!(name = strsep(&c, ":")))
			continue;
		strncpy(ifr->ifr_name, name, IFNAMSIZ);
		if (dhd_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 &&
			!strncmp(dev_type, "dhd", 3))
			if (dhd_check((void *)ifr) == 0)
				break;
		ifr->ifr_name[0] = '\0';
	}

	fclose(fp);
}

int
main(int argc, char **argv)
{
	struct ifreq ifr;
	cmd_t *cmd = NULL;
	int err = 0;
	char *ifname = NULL;
	int help = 0;
	int status = CMD_DHD;

	UNUSED_PARAMETER(argc);

	dhdu_av0 = argv[0];

	memset(&ifr, 0, sizeof(ifr));

	for (++argv; *argv;) {

		
		if ((status = dhd_option(&argv, &ifname, &help)) == CMD_OPT) {
			if (help)
				break;
			if (ifname) {
				if (strlen(ifname) > IFNAMSIZ) {
					fprintf(stderr, "%s: interface name too long\n", dhdu_av0);
					break;
				}
				strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
			}
			continue;
		}

		
		else if (status == CMD_ERR)
			break;

		
		if (!*ifr.ifr_name)
			dhd_find(&ifr);
		
		if (!*ifr.ifr_name || dhd_check((void *)&ifr)) {
			fprintf(stderr, "%s: dhd driver adapter not found\n", dhdu_av0);
			exit(1);
		}

		
		for (cmd = dhd_cmds; cmd->name && strcmp(cmd->name, *argv); cmd++);

		
		if (cmd->name == NULL)
			cmd = &dhd_varcmd;

		
		if (cmd->name)
			err = (*cmd->func)((void *)&ifr, cmd, argv);
		break;
	}

	
	if (!cmd)
		dhd_usage(NULL);
	else if (err == USAGE_ERROR)
		dhd_cmd_usage(cmd);
	else if (err == IOCTL_ERROR)
		dhd_printlasterror((void *)&ifr);

	return err;
}
