



#include "typedefs.h"
#include "bcmutils.h"
#include "bcmendian.h"
#include "uamp_api.h"
#include "wlioctl.h"
#include "dhdioctl.h"
#include "proto/bt_amp_hci.h"
#include "proto/bcmevent.h"
#include "proto/802.11_bta.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <pthread.h>

#include <linux/if_ether.h>
#include <mqueue.h>

#include <linux/sockios.h>
#include <linux/ethtool.h>





#define UAMP_DEBUG	1

#define DEV_TYPE_LEN	3 

#define UAMP_EVT_Q_STR		"/uamp_evt_q"
#define UAMP_PKT_RX_Q_STR	"/uamp_pkt_rx_q"

#if UAMP_DEBUG
	#define UAMP_PRINT(a)	printf a
	#define UAMP_TRACE(a)	printf a
	#define UAMP_ERROR(a)	printf a
#else
	#define UAMP_PRINT(a)	printf a
	#define UAMP_TRACE(a)
	#define UAMP_ERROR(a)	printf a
#endif

#if ((BRCM_BLUETOOTH_HOST == 1) && (UAMP_IS_GKI_AWARE == 1))
#include "gki.h"
#define UAMP_ALLOC(a)	GKI_getbuf(a+sizeof(BT_HDR))
#define UAMP_FREE(a)	GKI_freebuf(a)
#else
#define UAMP_ALLOC(a)	malloc(a)
#define UAMP_FREE(a)	free(a)
#endif   

#define GET_UAMP_FROM_ID(id)	(((id) == 0) ? &g_uamp_mgr.uamp : NULL)

#define MAX_IOVAR_LEN	2096



typedef struct UAMP_STATE
{
	
	tUAMP_ID		id;

	
	mqd_t			evt_q;
	mqd_t			pkt_rx_q;

	
	int			evt_fd;
	int			evt_fd_pipe[2];


	
	int			pkt_rx_fd;
	int			pkt_rx_fd_pipe[2];

	
	uint32			event_data[WLC_IOCTL_SMLEN/4];
	uint32			pkt_data[MAX_IOVAR_LEN/4];

} UAMP_STATE;



typedef struct UAMP_MGR
{
	
	tUAMP_CBACK		callback;

	
	struct ifreq		ifr;

	
	UAMP_STATE		uamp;

} UAMP_MGR;




static UAMP_MGR		g_uamp_mgr;




static void usage(void);
static int uamp_accept_test(void);
static int uamp_create_test(void);
static UINT16 uamp_write_cmd(uint16 opcode, uint8 *params, uint8 len,
                             amp_hci_cmd_t *cmd, unsigned int max_len);
static UINT16 uamp_write_data(uint16 handle, uint8 *data, uint8 len,
                              amp_hci_ACL_data_t *pkt, unsigned int max_len);

static int ioctl_get(int cmd, void *buf, int len);
static int ioctl_set(int cmd, void *buf, int len);
static int iovar_set(const char *iovar, void *param, int paramlen);
static int iovar_setbuf(const char *iovar, void *param, int paramlen, void *bufptr,
                        int buflen);
static int iovar_mkbuf(const char *name, char *data, uint datalen, char *iovar_buf,
                       uint buflen, int *perr);
static int wl_ioctl(int cmd, void *buf, int len, bool set);
static void wl_get_interface_name(struct ifreq *ifr);
static int wl_get_dev_type(char *name, void *buf, int len);
static void syserr(char *s);

static int init_event_rx(UAMP_STATE *uamp);
static void deinit_event_rx(UAMP_STATE *uamp);
static void* event_thread(void *param);
static void handle_event(UAMP_STATE *uamp);

static int init_pkt_rx(UAMP_STATE *uamp);
static void deinit_pkt_rx(UAMP_STATE *uamp);
static void* packet_rx_thread(void *param);
static void handle_rx_pkt(UAMP_STATE *uamp);


#if BRCM_BLUETOOTH_HOST == 1
#if UAMP_IS_GKI_AWARE == 1
void wl_event_gki_callback(wl_event_msg_t* event, void* event_data);
int wl_btamp_rx_gki_pkt_callback(wl_drv_netif_pkt pkt, unsigned int len);
#endif   
static void *uamp_get_acl_buf(unsigned int len);
void *hcisu_amp_get_acl_buf(int len);      
void hcisu_handle_amp_data_buf(void *pkt, unsigned int len);   
void hcisu_handle_amp_evt_buf(void* evt, unsigned int len);
int wl_is_drv_init_done(void);
#endif   




BT_API BOOLEAN UAMP_Init(tUAMP_CBACK p_cback)
{
	memset(&g_uamp_mgr, 0, sizeof(g_uamp_mgr));
	g_uamp_mgr.callback = p_cback;

	wl_get_interface_name(&g_uamp_mgr.ifr);

	return (TRUE);
}



BT_API BOOLEAN UAMP_Open(tUAMP_ID amp_id)
{
	UAMP_STATE *uamp = GET_UAMP_FROM_ID(amp_id);

#if BRCM_BLUETOOTH_HOST == 1
	if (!wl_is_drv_init_done()) {
		UAMP_ERROR(("%s: WLAN driver is not initialized! \n", __FUNCTION__));
		return FALSE;
	}
#endif   

	
	if ((init_event_rx(uamp)) < 0) {
		return (FALSE);
	}

	
	if ((init_pkt_rx(uamp)) < 0) {
		return (FALSE);
	}

	return (TRUE);
}



BT_API void UAMP_Close(tUAMP_ID amp_id)
{
	UAMP_STATE	*uamp = GET_UAMP_FROM_ID(amp_id);

#if BRCM_BLUETOOTH_HOST == 1
	if (!wl_is_drv_init_done()) {
		UAMP_ERROR(("%s: WLAN driver is not initialized! \n", __FUNCTION__));
		return;
	}
#endif   

	
	deinit_pkt_rx(uamp);
	deinit_event_rx(uamp);
}


BT_API UINT16 UAMP_Write(tUAMP_ID amp_id, UINT8 *p_buf, UINT16 num_bytes, tUAMP_CH channel)
{
	int ret = -1;
	UINT16 num_bytes_written = num_bytes;

	UNUSED_PARAMETER(amp_id);

#if BRCM_BLUETOOTH_HOST == 1
	if (!wl_is_drv_init_done()) {
		UAMP_ERROR(("%s: WLAN driver is not initialized! \n", __FUNCTION__));
		return (0);
	}
#endif   

	if (channel == UAMP_CH_HCI_CMD) {
		ret = iovar_set("HCI_cmd", p_buf, num_bytes);
	}
	else if (channel == UAMP_CH_HCI_DATA) {
		ret = iovar_set("HCI_ACL_data", p_buf, num_bytes);
	}

	if (ret != 0) {
		num_bytes_written = 0;
	        UAMP_ERROR(("UAMP_Write error: %i  ( 0=success )\n", ret));
	}

	return (num_bytes_written);
}



BT_API UINT16 UAMP_Read(tUAMP_ID amp_id, UINT8 *p_buf, UINT16 buf_size, tUAMP_CH channel)
{
	UAMP_STATE		*uamp = GET_UAMP_FROM_ID(amp_id);
	mqd_t 			num_bytes;
	unsigned int 		msg_prio;

#if BRCM_BLUETOOTH_HOST == 1
	if (!wl_is_drv_init_done()) {
		UAMP_ERROR(("%s: WLAN driver is not initialized! \n", __FUNCTION__));
		return (0);
	}
#endif   


	if (channel == UAMP_CH_HCI_EVT) {
		
		num_bytes = mq_receive(uamp->evt_q, (char *)p_buf, buf_size, &msg_prio);
		if (num_bytes == -1) {
			UAMP_ERROR(("%s: Event queue receive error!\n", __FUNCTION__));
			return (0);
		}

		return (num_bytes);
	}
	else if (channel == UAMP_CH_HCI_DATA) {
		
		num_bytes = mq_receive(uamp->pkt_rx_q, (char *)p_buf, buf_size, &msg_prio);
		if (num_bytes == -1) {
			UAMP_ERROR(("%s: Pkt queue receive error!\n", __FUNCTION__));
			return (0);
		}

		return (num_bytes);
	}

	return (0);
}



static int
ioctl_get(int cmd, void *buf, int len)
{
	return wl_ioctl(cmd, buf, len, FALSE);
}



static int
ioctl_set(int cmd, void *buf, int len)
{
	return wl_ioctl(cmd, buf, len, TRUE);
}



static int
iovar_set(const char *iovar, void *param, int paramlen)
{
	static char smbuf[MAX_IOVAR_LEN];

	memset(smbuf, 0, sizeof(smbuf));

	return iovar_setbuf(iovar, param, paramlen, smbuf, sizeof(smbuf));
}


static int
iovar_setbuf(const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	int iolen;

	iolen = iovar_mkbuf(iovar, param, paramlen, bufptr, buflen, &err);
	if (err)
		return err;

	return ioctl_set(DHD_SET_VAR, bufptr, iolen);
}



static int
iovar_mkbuf(const char *name, char *data, uint datalen, char *iovar_buf, uint buflen, int *perr)
{
	int iovar_len;

	iovar_len = strlen(name) + 1;

	
	if ((iovar_len + datalen) > buflen) {
		*perr = -1;
		return 0;
	}

	
	if (datalen > 0)
		memmove(&iovar_buf[iovar_len], data, datalen);

	
	strcpy(iovar_buf, name);

	*perr = 0;
	return (iovar_len + datalen);
}



static int
wl_ioctl(int cmd, void *buf, int len, bool set)
{
	struct ifreq ifr;
	dhd_ioctl_t ioc;
	int ret = 0;
	int s;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, g_uamp_mgr.ifr.ifr_name, sizeof(ifr.ifr_name));

	
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		ret = -1;
		return ret;
	}

	
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = set;
	ioc.driver = DHD_IOCTL_MAGIC;
	ifr.ifr_data = (caddr_t) &ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0) {
		ret = -1;
	}

	
	close(s);
	return ret;
}


static int
wl_get_dev_type(char *name, void *buf, int len)
{
	int s;
	int ret;
	struct ifreq ifr;
	struct ethtool_drvinfo info;

	
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	
	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t)&info;
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {

		
		if (errno == EPERM)
			syserr("wl_get_dev_type");

		*(char *)buf = '\0';
	} else {
		strncpy(buf, info.driver, len);
	}

	close(s);
	return ret;
}


static void
wl_get_interface_name(struct ifreq *ifr)
{
	char proc_net_dev[] = "/proc/net/dev";
	FILE *fp;
	char buf[1000], *c, *name;
	char dev_type[DEV_TYPE_LEN];
	int ret = -1;

	ifr->ifr_name[0] = '\0';

	if (!(fp = fopen(proc_net_dev, "r")))
		return;

	
	if (!fgets(buf, sizeof(buf), fp) ||
	    !fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		c = buf;
		while (isspace(*c))
			c++;
		if (!(name = strsep(&c, ":")))
			continue;
		strncpy(ifr->ifr_name, name, IFNAMSIZ);
		if (wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 &&
			(!strncmp(dev_type, "wl", 2) || !strncmp(dev_type, "dhd", 3)))
		{
			ret = 0;
			break;
		}
		ifr->ifr_name[0] = '\0';
	}

	fclose(fp);
}


static void
syserr(char *s)
{
	fprintf(stderr, "uamp_linux:");
	perror(s);
	exit(errno);
}


#if BRCM_BLUETOOTH_HOST == 1
static void *uamp_get_acl_buf(unsigned int len)
{
	return (hcisu_amp_get_acl_buf(len));
}
#endif   



static int
init_pkt_rx(UAMP_STATE *uamp)
{
	struct ifreq		ifr;
	int			fd = -1;
	struct sockaddr_ll	local;
	int 			err;
	int			fd_pipe[2] = {-1, -1};
	pthread_t		h;


	memset(&ifr, 0, sizeof(ifr));
	wl_get_interface_name(&ifr);

	
	fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_802_2));
	if (fd < 0) {
		UAMP_ERROR(("%s: Cannot open socket", __FUNCTION__));
		return (-1);
	}

	err = ioctl(fd, SIOCGIFINDEX, &ifr);
	if (err < 0) {
		UAMP_ERROR(("%s: Cannot get index %d\n", __FUNCTION__, err));
		close(fd);
		return (-1);
	}

	memset(&local, 0, sizeof(local));
	local.sll_family	= PF_PACKET;
	local.sll_protocol	= htons(ETH_P_802_2);
	local.sll_ifindex	= ifr.ifr_ifindex;

	if (bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0) {
		UAMP_ERROR(("%s: Cannot bind socket", __FUNCTION__));
		close(fd);
		return (-1);
	}


	
	if (pipe(fd_pipe) != 0) {
		UAMP_ERROR(("%s: pipe failed\n", __FUNCTION__));
		goto cleanup;
	}

	
	uamp->pkt_rx_fd		= fd;
	uamp->pkt_rx_fd_pipe[0]	= fd_pipe[0];
	uamp->pkt_rx_fd_pipe[1]	= fd_pipe[1];


	

	uamp->pkt_rx_q = mq_open(UAMP_PKT_RX_Q_STR, O_RDWR | O_CREAT, 0666, NULL);



	
	pthread_create(&h, NULL, packet_rx_thread, uamp);

	return (fd);

cleanup:
	if (-1 != fd)		close(fd);
	if (-1 != fd_pipe[0])	close(fd_pipe[0]);
	if (-1 != fd_pipe[1])	close(fd_pipe[1]);
	return (-1);
}



static void
deinit_pkt_rx(UAMP_STATE *uamp)
{
	
	mq_close(uamp->pkt_rx_q);
	mq_unlink(UAMP_PKT_RX_Q_STR);

	
	write(uamp->pkt_rx_fd_pipe[1], NULL, 0);
	close(uamp->pkt_rx_fd_pipe[1]);
}



static void*
packet_rx_thread(void *param)
{
	UAMP_STATE *uamp = (UAMP_STATE *) param;

	UAMP_PRINT(("Start packet rx wait loop\n"));

	while (1) {
		fd_set		rfds;	
		int		last_fd;
		int		ret;

		FD_ZERO(&rfds);
		FD_SET(uamp->pkt_rx_fd_pipe[0], &rfds);
		FD_SET(uamp->pkt_rx_fd, &rfds);
		last_fd = MAX(uamp->pkt_rx_fd_pipe[0], uamp->pkt_rx_fd);

		
		ret = select(last_fd+1, &rfds, NULL, NULL, NULL);

		
		if (0 > ret) {
			UAMP_ERROR(("%s: Unhandled signal on pkt rx socket\n", __FUNCTION__));
			break;
		}

		
		if (FD_ISSET(uamp->pkt_rx_fd_pipe[0], &rfds)) {
			UAMP_PRINT(("%s: stop rcvd on dispatcher pipe\n", __FUNCTION__));
			break;
		}

		
		if (FD_ISSET(uamp->pkt_rx_fd, &rfds)) {
			handle_rx_pkt(uamp);
		}

	}  

	UAMP_PRINT(("%s: End packet rx wait loop\n", __FUNCTION__));

	close(uamp->pkt_rx_fd);
	close(uamp->pkt_rx_fd_pipe[0]);

	UAMP_TRACE(("Exit %s\n", __FUNCTION__));
	return (NULL);
}


static void
handle_rx_pkt(UAMP_STATE *uamp)
{
	int				bytes;
	struct dot11_llc_snap_header	*lsh;
	amp_hci_ACL_data_t		*acl_data;

	
	bytes = recv(uamp->pkt_rx_fd, uamp->pkt_data, sizeof(uamp->pkt_data), MSG_DONTWAIT);

	
	if (bytes < 0) {
		if (errno != EINTR && errno != EAGAIN) {
			UAMP_ERROR(("%s: Error reading packet rx socket: %s\n",
			            __FUNCTION__, strerror(errno)));
			return;
		}
	}

	if (bytes == 0) {
		UAMP_ERROR(("%s: EOF on packet rx socket", __FUNCTION__));
		return;
	}


	
	lsh = (struct dot11_llc_snap_header *)uamp->pkt_data;
	if (bcmp(lsh, BT_SIG_SNAP_MPROT, DOT11_LLC_SNAP_HDR_LEN - 2) != 0 ||
		ntoh16(lsh->type) != BTA_PROT_L2CAP) {
		
		return;
	}


	UAMP_TRACE(("%s: received packet!\n", __FUNCTION__));

	acl_data = (amp_hci_ACL_data_t *) &lsh[1];
	bytes -= DOT11_LLC_SNAP_HDR_LEN;

#if BRCM_BLUETOOTH_HOST == 1
	hcisu_handle_amp_data_buf(acl_data, bytes);
#else
	{
		tUAMP_EVT_DATA			uamp_evt_data;
#if UAMP_DEBUG == 1
		
		{
			int i;
			uint8 *data = acl_data->data;
			UAMP_TRACE(("data(%d): ", bytes));
			for (i = 0; i < bytes; i++) {
				UAMP_TRACE(("0x%x ", data[i]));
			}
			UAMP_TRACE(("\n"));
		}
#endif   

		
		if (mq_send(uamp->pkt_rx_q, (const char *)acl_data, bytes, 0) != 0) {
			
			UAMP_ERROR(("%s: Unable to queue rx packet data!\n", __FUNCTION__));
			return;
		}


		
		memset(&uamp_evt_data, 0, sizeof(uamp_evt_data));
		uamp_evt_data.channel = UAMP_CH_HCI_DATA;
		g_uamp_mgr.callback(0, UAMP_EVT_RX_READY, &uamp_evt_data);
	}
#endif 
}



static int
init_event_rx(UAMP_STATE *uamp)
{
	struct ifreq		ifr;
	int			fd = -1;
	struct sockaddr_ll	local;
	int 			err;
	int			fd_pipe[2] = {-1, -1};
	pthread_t		h;

	memset(&ifr, 0, sizeof(ifr));
	wl_get_interface_name(&ifr);
	UAMP_PRINT(("ifr_name (%s)\n", ifr.ifr_name));

	
	fd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE_BRCM));
	if (fd < 0) {
		UAMP_ERROR(("%s: Cannot open socket", __FUNCTION__));
		return (-1);
	}

	err = ioctl(fd, SIOCGIFINDEX, &ifr);
	if (err < 0) {
		UAMP_ERROR(("%s: Cannot get index %d\n", __FUNCTION__, err));
		close(fd);
		return (-1);
	}

	memset(&local, 0, sizeof(local));
	local.sll_family	= AF_PACKET;
	local.sll_protocol	= htons(ETHER_TYPE_BRCM);
	local.sll_ifindex	= ifr.ifr_ifindex;

	if (bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0) {
		UAMP_ERROR(("%s: Cannot bind event socket", __FUNCTION__));
		close(fd);
		return (-1);
	}


	
	if (pipe(fd_pipe) != 0) {
		UAMP_ERROR(("%s: pipe failed\n", __FUNCTION__));
		goto cleanup;
	}

	
	uamp->evt_fd		= fd;
	uamp->evt_fd_pipe[0]	= fd_pipe[0];
	uamp->evt_fd_pipe[1]	= fd_pipe[1];


	

	uamp->evt_q = mq_open(UAMP_EVT_Q_STR, O_RDWR | O_CREAT, 0666, NULL);
	UAMP_PRINT(("evt_q(0x%x)\n", (int)uamp->evt_q));



	
	pthread_create(&h, NULL, event_thread, uamp);

	return (fd);

cleanup:
	if (-1 != fd)		close(fd);
	if (-1 != fd_pipe[0])	close(fd_pipe[0]);
	if (-1 != fd_pipe[1])	close(fd_pipe[1]);
	return (-1);
}



static void
deinit_event_rx(UAMP_STATE	*uamp)
{
	
	mq_close(uamp->evt_q);
	mq_unlink(UAMP_EVT_Q_STR);

	
	write(uamp->evt_fd_pipe[1], NULL, 0);
	close(uamp->evt_fd_pipe[1]);
}


static void*
event_thread(void *param)
{
	UAMP_STATE *uamp = (UAMP_STATE *) param;

	UAMP_PRINT(("Start event wait loop\n"));

	while (1) {
		fd_set		rfds;	
		int		last_fd;
		int		ret;

		FD_ZERO(&rfds);
		FD_SET(uamp->evt_fd_pipe[0], &rfds);
		FD_SET(uamp->evt_fd, &rfds);
		last_fd = MAX(uamp->evt_fd_pipe[0], uamp->evt_fd);

		
		ret = select(last_fd+1, &rfds, NULL, NULL, NULL);

		
		if (0 > ret) {
			UAMP_ERROR(("%s: Unhandled signal on brcm event socket\n", __FUNCTION__));
			break;
		}

		
		if (FD_ISSET(uamp->evt_fd_pipe[0], &rfds)) {
			UAMP_PRINT(("%s: stop rcvd on dispatcher pipe\n", __FUNCTION__));
			break;
		}

		
		if (FD_ISSET(uamp->evt_fd, &rfds)) {
			handle_event(uamp);
		}

	}  

	UAMP_PRINT(("%s: End event wait loop\n", __FUNCTION__));

	close(uamp->evt_fd);
	close(uamp->evt_fd_pipe[0]);

	UAMP_TRACE(("Exit %s\n", __FUNCTION__));
	return (NULL);
}



static void
handle_event(UAMP_STATE *uamp)
{
	int			bytes;
	bcm_event_t		*bcm_event;
	wl_event_msg_t		*wl_event;
	uint8			*wl_evt_data;
	uint32			datalen;

	
	bytes = recv(uamp->evt_fd, uamp->event_data, sizeof(uamp->event_data), MSG_DONTWAIT);

	
	if (bytes < 0) {
		if (errno != EINTR && errno != EAGAIN) {
			UAMP_ERROR(("%s: Error reading event socket: %s\n",
			            __FUNCTION__, strerror(errno)));
			return;
		}
	}

	if (bytes == 0) {
		UAMP_ERROR(("%s: EOF on event socket", __FUNCTION__));
		return;
	}


	
	bcm_event = (bcm_event_t *)uamp->event_data;
	if (ntoh32(bcm_event->event.event_type) != WLC_E_BTA_HCI_EVENT) {
		return;
	}

	UAMP_TRACE(("%s: received event!\n", __FUNCTION__));


	wl_event = &bcm_event->event;
	wl_evt_data = (uint8 *)&wl_event[1];
	datalen = ntoh32(wl_event->datalen);

#if BRCM_BLUETOOTH_HOST == 1
	hcisu_handle_amp_evt_buf(wl_evt_data, datalen);
#else
	{
		tUAMP_EVT_DATA		uamp_evt_data;

#if UAMP_DEBUG == 1
		
		{
			unsigned int i;
			UAMP_TRACE(("data(%d): ", datalen));
			for (i = 0; i < datalen; i++)
			{
				UAMP_TRACE(("0x%x ", wl_evt_data[i]));
			}
			UAMP_TRACE(("\n"));
		}
#endif   

		
		if (mq_send(uamp->evt_q, (const char *)wl_evt_data, datalen, 0) != 0) {
			
			UAMP_ERROR(("%s: Unable to queue event packet!\n", __FUNCTION__));
			return;
		}


		
		memset(&uamp_evt_data, 0, sizeof(uamp_evt_data));
		uamp_evt_data.channel = UAMP_CH_HCI_EVT;
		g_uamp_mgr.callback(0, UAMP_EVT_RX_READY, &uamp_evt_data);
	}
#endif   

}


#define UAMP_TEST 1
#if UAMP_TEST
int
main(int argc, char **argv)
{
	int ret;

	printf("Hello, world!\n");

	if (argc != 2) {
		usage();
		return (-1);
	}

	if (strcmp(argv[1], "-a") == 0) {
		ret = uamp_accept_test();
	}
	else if (strcmp(argv[1], "-c") == 0) {
		ret = uamp_create_test();
	}
	else {
		usage();
		return (-1);
	}


	return (ret);
}


static void usage(void)
{
	UAMP_PRINT(("Usage:\n"));
	UAMP_PRINT(("\t uamp [-a | -c]\n"));
	UAMP_PRINT(("\t\t -a: acceptor\n"));
	UAMP_PRINT(("\t\t -c: creator\n"));
}

#define WAIT_FOR_KEY(delay) \
	do { \
		usleep(1000*delay); \
		UAMP_PRINT(("Press key to continue\n")); \
		getchar(); \
	} \
	while (0);



static void uamp_callback(tUAMP_ID amp_id, tUAMP_EVT amp_evt, tUAMP_EVT_DATA *p_amp_evt_data)
{
	UINT8			buf[8192];
	amp_hci_ACL_data_t	*data;
	amp_hci_event_t		*evt;
	unsigned int		i;
	UINT16			num_bytes;

	UNUSED_PARAMETER(amp_evt);

	num_bytes = UAMP_Read(amp_id, buf, sizeof(buf), p_amp_evt_data->channel);
	if (num_bytes != 0) {
		if (p_amp_evt_data->channel == UAMP_CH_HCI_EVT) {
			evt = (amp_hci_event_t *) buf;
			UAMP_PRINT(("%s: evt - ecode(%d) plen(%d)\n",
			            __FUNCTION__, evt->ecode, evt->plen));

			for (i = 0; i < evt->plen; i++) {
				UAMP_PRINT(("0x%x ", evt->parms[i]));
			}
			UAMP_PRINT(("\n"));
		}
		else if (p_amp_evt_data->channel == UAMP_CH_HCI_DATA) {
			data = (amp_hci_ACL_data_t *) buf;
			UAMP_PRINT(("%s: data - dlen(%d)\n", __FUNCTION__, data->dlen));

			for (i = 0; i < data->dlen; i++) {
				UAMP_PRINT(("0x%x ", data->data[i]));
			}
			UAMP_PRINT(("\n"));
		}
	}
	else {
		UAMP_PRINT(("%s: UAMP_Read error\n", __FUNCTION__));
	}
}



static int uamp_accept_test(void)
{
	uint8 set_event_mask_page_2_data[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	uint8 read_local_amp_assoc_data[] = {0, 0, 0};
	uint8 accept_physical_link_request_data[] = {0x11, 32, 3, 0x00, 0x01, 0x02,
		0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
		0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
		0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
	uint8 write_remote_amp_assoc_data[] = {0x11, 0x0, 0x0, 0x24, 0x00, 0x04,
		0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x05, 0x00, 0x01, 0x0f,
		0x00, 0x10, 0x09, 0x01, 0x06, 0x00, 0x00, 0x90, 0x4c, 0xc5, 0x06,
		0x79, 0x02, 0x09, 0x00, 0x55, 0x53, 0x20, 0xc9, 0x0c, 0x00, 0x01,
		0x01, 0x14};
	uint8 accept_logical_link_data[] = {0x11, 0x01, 0x01, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x01, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff};
	uint8 tx_data[] = {7, 6, 5, 4, 3, 2, 1, 0};
	uint8 disconnect_logical_link_data[] = {0};
	uint8 disconnect_physical_link_data[] = {0x11, 0};

	uint32 buf[64];
	amp_hci_cmd_t *cmd = (amp_hci_cmd_t *)buf;
	amp_hci_ACL_data_t *pkt = (amp_hci_ACL_data_t *)buf;


	UAMP_PRINT(("UAMP acceptor test\n"));


	UAMP_Init(uamp_callback);
	UAMP_Open(0);


	
	uamp_write_cmd(HCI_Set_Event_Mask_Page_2, set_event_mask_page_2_data,
	               sizeof(set_event_mask_page_2_data), cmd, sizeof(buf));

	
	uamp_write_cmd(HCI_Read_Local_AMP_ASSOC, read_local_amp_assoc_data,
	               sizeof(read_local_amp_assoc_data), cmd, sizeof(buf));
	WAIT_FOR_KEY(1000);

	
	uamp_write_cmd(HCI_Accept_Physical_Link_Request, accept_physical_link_request_data,
	               sizeof(accept_physical_link_request_data), cmd, sizeof(buf));


	
	
	uamp_write_cmd(HCI_Write_Remote_AMP_ASSOC, write_remote_amp_assoc_data,
	               sizeof(write_remote_amp_assoc_data), cmd, sizeof(buf));
	WAIT_FOR_KEY(1000);

	
	uamp_write_cmd(HCI_Accept_Logical_Link, accept_logical_link_data,
	               sizeof(accept_logical_link_data), cmd, sizeof(buf));
	WAIT_FOR_KEY(1000);

	
	uamp_write_data(0 | HCI_ACL_DATA_BC_FLAGS | HCI_ACL_DATA_PB_FLAGS, tx_data,
	                sizeof(tx_data), pkt, sizeof(buf));
	WAIT_FOR_KEY(1000);

	
	uamp_write_cmd(HCI_Disconnect_Logical_Link, disconnect_logical_link_data,
	               sizeof(disconnect_logical_link_data), cmd, sizeof(buf));

	
	uamp_write_cmd(HCI_Disconnect_Physical_Link, disconnect_physical_link_data,
	               sizeof(disconnect_physical_link_data), cmd, sizeof(buf));

	usleep(1000*1000);
	UAMP_Close(0);
	usleep(1000*1000);
	UAMP_PRINT(("UAMP acceptor test done!\n"));

	return (0);
}



static int uamp_create_test(void)
{
	uint8 set_event_mask_page_2_data[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	uint8 create_physical_link_data[] = {0x10, 32, 3, 0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
		0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
		0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
	uint8 write_remote_amp_assoc_data[] = {0x10, 0x0, 0x0, 0x21, 0x00, 0x04,
		0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x05, 0x00, 0x01, 0x0f,
		0x00, 0x10, 0x09, 0x01, 0x06, 0x00, 0x00, 0x90, 0x4c, 0xc6, 0x02,
		0x5b, 0x02, 0x06, 0x00, 0x55, 0x53, 0x20, 0xc9, 0x0c, 0x00};
	uint8 read_local_amp_assoc_data[] = {0x10, 0, 0, 100, 0};
	uint8 create_logical_link_data[] = {0x10, 0x01, 0x01, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x01, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff};
	uint8 disconnect_logical_link_data[] = {0};
	uint8 disconnect_physical_link_data[] = {0x10, 0};
	uint8 tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7};

	uint32 buf[64];
	amp_hci_cmd_t *cmd = (amp_hci_cmd_t *)buf;
	amp_hci_ACL_data_t *pkt = (amp_hci_ACL_data_t *)buf;

	UAMP_PRINT(("UAMP creator test\n"));


	UAMP_Init(uamp_callback);

	UAMP_Open(0);

	
	uamp_write_cmd(HCI_Set_Event_Mask_Page_2, set_event_mask_page_2_data,
	               sizeof(set_event_mask_page_2_data), cmd, sizeof(buf));

	
	uamp_write_cmd(HCI_Read_Local_AMP_Info, NULL, 0, cmd, sizeof(buf));
	WAIT_FOR_KEY(1000);

	
	uamp_write_cmd(HCI_Create_Physical_Link, create_physical_link_data,
	               sizeof(create_physical_link_data), cmd, sizeof(buf));

	
	
	uamp_write_cmd(HCI_Write_Remote_AMP_ASSOC, write_remote_amp_assoc_data,
	               sizeof(write_remote_amp_assoc_data), cmd, sizeof(buf));


	
	usleep(1000*1000);


	
	uamp_write_cmd(HCI_Read_Local_AMP_ASSOC, read_local_amp_assoc_data,
	               sizeof(read_local_amp_assoc_data), cmd, sizeof(buf));
	WAIT_FOR_KEY(1000);

	
	uamp_write_cmd(HCI_Create_Logical_Link, create_logical_link_data,
	               sizeof(create_logical_link_data), cmd, sizeof(buf));
	WAIT_FOR_KEY(1000);

	
	uamp_write_data(0 | HCI_ACL_DATA_BC_FLAGS | HCI_ACL_DATA_PB_FLAGS, tx_data,
	                sizeof(tx_data), pkt, sizeof(buf));
	WAIT_FOR_KEY(1000);

	
	uamp_write_cmd(HCI_Disconnect_Logical_Link, disconnect_logical_link_data,
	               sizeof(disconnect_logical_link_data), cmd, sizeof(buf));

	
	uamp_write_cmd(HCI_Disconnect_Physical_Link, disconnect_physical_link_data,
	               sizeof(disconnect_physical_link_data), cmd, sizeof(buf));

	usleep(1000*1000);
	UAMP_Close(0);
	usleep(1000*1000);
	UAMP_PRINT(("UAMP creator test done!\n"));

	return (0);
}



static UINT16 uamp_write_cmd(uint16 opcode, uint8 *params, uint8 len,
                             amp_hci_cmd_t *cmd, unsigned int max_len)
{
	memset(cmd, 0, sizeof(amp_hci_cmd_t));
	cmd->plen = len;
	cmd->opcode = opcode;
	assert(HCI_CMD_PREAMBLE_SIZE + len <= max_len);

	if (len != 0) {
		memcpy(cmd->parms, params, len);
	}

	return (UAMP_Write(0, (UINT8 *)cmd, HCI_CMD_PREAMBLE_SIZE + len, UAMP_CH_HCI_CMD));
}



static UINT16 uamp_write_data(uint16 handle, uint8 *data, uint8 len,
                              amp_hci_ACL_data_t *pkt, unsigned int max_len)
{
	memset(pkt, 0, sizeof(amp_hci_ACL_data_t));
	pkt->handle = handle;
	pkt->dlen = len;
	assert(HCI_ACL_DATA_PREAMBLE_SIZE + len <= max_len);

	if (len != 0) {
		memcpy(pkt->data, data, len);
	}

	return (UAMP_Write(0, (UINT8 *)pkt, HCI_ACL_DATA_PREAMBLE_SIZE + len, UAMP_CH_HCI_DATA));
}
#endif   
