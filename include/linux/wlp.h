

#ifndef __LINUX__WLP_H_
#define __LINUX__WLP_H_

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/uwb.h>


#define WLP_PROTOCOL_ID 0x0100


#define WLP_VERSION 0x10


#define WLP_WSS_UUID_STRSIZE 48


#define WLP_WSS_NONCE_STRSIZE 48



#define WLP_WSS_NAME_SIZE 65


#define WLP_DATA_HLEN 4


enum wlp_wss_state {
	WLP_WSS_STATE_NONE = 0,
	WLP_WSS_STATE_PART_ENROLLED,
	WLP_WSS_STATE_ENROLLED,
	WLP_WSS_STATE_ACTIVE,
	WLP_WSS_STATE_CONNECTED,
};


enum wlp_wss_sec_status {
	WLP_WSS_UNSECURE = 0,
	WLP_WSS_SECURE,
};


enum wlp_frame_type {
	WLP_FRAME_STANDARD = 0,
	WLP_FRAME_ABBREVIATED,
	WLP_FRAME_CONTROL,
	WLP_FRAME_ASSOCIATION,
};


enum wlp_assoc_type {
	WLP_ASSOC_D1 = 2,
	WLP_ASSOC_D2 = 3,
	WLP_ASSOC_M1 = 4,
	WLP_ASSOC_M2 = 5,
	WLP_ASSOC_M3 = 7,
	WLP_ASSOC_M4 = 8,
	WLP_ASSOC_M5 = 9,
	WLP_ASSOC_M6 = 10,
	WLP_ASSOC_M7 = 11,
	WLP_ASSOC_M8 = 12,
	WLP_ASSOC_F0 = 14,
	WLP_ASSOC_E1 = 32,
	WLP_ASSOC_E2 = 33,
	WLP_ASSOC_C1 = 34,
	WLP_ASSOC_C2 = 35,
	WLP_ASSOC_C3 = 36,
	WLP_ASSOC_C4 = 37,
};


enum wlp_attr_type {
	WLP_ATTR_AUTH		= 0x1005, 
	WLP_ATTR_DEV_NAME 	= 0x1011, 
	WLP_ATTR_DEV_PWD_ID 	= 0x1012, 
	WLP_ATTR_E_HASH1	= 0x1014, 
	WLP_ATTR_E_HASH2	= 0x1015, 
	WLP_ATTR_E_SNONCE1	= 0x1016, 
	WLP_ATTR_E_SNONCE2	= 0x1017, 
	WLP_ATTR_ENCR_SET	= 0x1018, 
	WLP_ATTR_ENRL_NONCE	= 0x101A, 
	WLP_ATTR_KEYWRAP_AUTH	= 0x101E, 
	WLP_ATTR_MANUF		= 0x1021, 
	WLP_ATTR_MSG_TYPE	= 0x1022, 
	WLP_ATTR_MODEL_NAME	= 0x1023, 
	WLP_ATTR_MODEL_NR	= 0x1024, 
	WLP_ATTR_PUB_KEY	= 0x1032, 
	WLP_ATTR_REG_NONCE	= 0x1039, 
	WLP_ATTR_R_HASH1	= 0x103D, 
	WLP_ATTR_R_HASH2	= 0x103E, 
	WLP_ATTR_R_SNONCE1	= 0x103F, 
	WLP_ATTR_R_SNONCE2	= 0x1040, 
	WLP_ATTR_SERIAL		= 0x1042, 
	WLP_ATTR_UUID_E		= 0x1047, 
	WLP_ATTR_UUID_R		= 0x1048, 
	WLP_ATTR_PRI_DEV_TYPE	= 0x1054, 
	WLP_ATTR_SEC_DEV_TYPE	= 0x1055, 
	WLP_ATTR_PORT_DEV	= 0x1056, 
	WLP_ATTR_APP_EXT	= 0x1058, 
	WLP_ATTR_WLP_VER	= 0x2000, 
	WLP_ATTR_WSSID		= 0x2001, 
	WLP_ATTR_WSS_NAME	= 0x2002, 
	WLP_ATTR_WSS_SEC_STAT	= 0x2003, 
	WLP_ATTR_WSS_BCAST	= 0x2004, 
	WLP_ATTR_WSS_M_KEY	= 0x2005, 
	WLP_ATTR_ACC_ENRL	= 0x2006, 
	WLP_ATTR_WSS_INFO	= 0x2007, 
	WLP_ATTR_WSS_SEL_MTHD	= 0x2008, 
	WLP_ATTR_ASSC_MTHD_LIST	= 0x2009, 
	WLP_ATTR_SEL_ASSC_MTHD	= 0x200A, 
	WLP_ATTR_ENRL_HASH_COMM	= 0x200B, 
	WLP_ATTR_WSS_TAG	= 0x200C, 
	WLP_ATTR_WSS_VIRT	= 0x200D, 
	WLP_ATTR_WLP_ASSC_ERR	= 0x200E, 
	WLP_ATTR_VNDR_EXT	= 0x200F, 
};


enum wlp_dev_category_id {
	WLP_DEV_CAT_COMPUTER = 1,
	WLP_DEV_CAT_INPUT,
	WLP_DEV_CAT_PRINT_SCAN_FAX_COPIER,
	WLP_DEV_CAT_CAMERA,
	WLP_DEV_CAT_STORAGE,
	WLP_DEV_CAT_INFRASTRUCTURE,
	WLP_DEV_CAT_DISPLAY,
	WLP_DEV_CAT_MULTIM,
	WLP_DEV_CAT_GAMING,
	WLP_DEV_CAT_TELEPHONE,
	WLP_DEV_CAT_OTHER = 65535,
};


enum wlp_wss_sel_mthd {
	WLP_WSS_ENRL_SELECT = 1,	
	WLP_WSS_REG_SELECT,		
};


enum wlp_assc_error {
	WLP_ASSOC_ERROR_NONE,
	WLP_ASSOC_ERROR_AUTH,		
	WLP_ASSOC_ERROR_ROGUE,		
	WLP_ASSOC_ERROR_BUSY,		
	WLP_ASSOC_ERROR_LOCK,		
	WLP_ASSOC_ERROR_NOT_READY,	
	WLP_ASSOC_ERROR_INV,		
	WLP_ASSOC_ERROR_MSG_TIME,	
	WLP_ASSOC_ERROR_ENR_TIME,	
	WLP_ASSOC_ERROR_PW,		
	WLP_ASSOC_ERROR_VER,		
	WLP_ASSOC_ERROR_INT,		
	WLP_ASSOC_ERROR_UNDEF,		
	WLP_ASSOC_ERROR_NUM,		
	WLP_ASSOC_ERROR_WAIT,		
};


enum wlp_parameters {
	WLP_PER_MSG_TIMEOUT = 15,	
};


struct wlp_ie {
	struct uwb_ie_hdr hdr;
	__le16 capabilities;
	__le16 cycle_param;
	__le16 acw_anchor_addr;
	u8 wssid_hash_list[];
} __attribute__((packed));

static inline int wlp_ie_hash_length(struct wlp_ie *ie)
{
	return (le16_to_cpu(ie->capabilities) >> 12) & 0xf;
}

static inline void wlp_ie_set_hash_length(struct wlp_ie *ie, int hash_length)
{
	u16 caps = le16_to_cpu(ie->capabilities);
	caps = (caps & ~(0xf << 12)) | (hash_length << 12);
	ie->capabilities = cpu_to_le16(caps);
}


struct wlp_nonce {
	u8 data[16];
} __attribute__((packed));


struct wlp_uuid {
	u8 data[16];
} __attribute__((packed));



struct wlp_dev_type {
	enum wlp_dev_category_id category:16;
	u8 OUI[3];
	u8 OUIsubdiv;
	__le16 subID;
} __attribute__((packed));


struct wlp_frame_hdr {
	__le16 mux_hdr;			
	enum wlp_frame_type type:8;
} __attribute__((packed));


struct wlp_attr_hdr {
	__le16 type;
	__le16 length;
} __attribute__((packed));


struct wlp_device_info {
	char name[33];
	char model_name[33];
	char manufacturer[65];
	char model_nr[33];
	char serial[33];
	struct wlp_dev_type prim_dev_type;
};


#define wlp_attr(type, name)						\
struct wlp_attr_##name {						\
	struct wlp_attr_hdr hdr;					\
	type name;							\
} __attribute__((packed));

#define wlp_attr_array(type, name)					\
struct wlp_attr_##name {						\
	struct wlp_attr_hdr hdr;					\
	type name[];							\
} __attribute__((packed));




wlp_attr_array(u8, dev_name)


wlp_attr(struct wlp_nonce, enonce)


wlp_attr_array(u8, manufacturer)


wlp_attr(u8, msg_type)


wlp_attr_array(u8, model_name)


wlp_attr_array(u8, model_nr)


wlp_attr(struct wlp_nonce, rnonce)


wlp_attr_array(u8, serial)


wlp_attr(struct wlp_uuid, uuid_e)


wlp_attr(struct wlp_uuid, uuid_r)


wlp_attr(struct wlp_dev_type, prim_dev_type)


wlp_attr(struct wlp_dev_type, sec_dev_type)


wlp_attr(u8, version)


wlp_attr(struct wlp_uuid, wssid)


wlp_attr_array(u8, wss_name)


wlp_attr(u8, wss_sec_status)


wlp_attr(struct uwb_mac_addr, wss_bcast)


wlp_attr(u8, accept_enrl)


struct wlp_wss_info {
	struct wlp_attr_wssid wssid;
	struct wlp_attr_wss_name name;
	struct wlp_attr_accept_enrl accept;
	struct wlp_attr_wss_sec_status sec_stat;
	struct wlp_attr_wss_bcast bcast;
} __attribute__((packed));


wlp_attr_array(struct wlp_wss_info, wss_info)


wlp_attr(u8, wss_sel_mthd)


wlp_attr(u8, wss_tag)


wlp_attr(struct uwb_mac_addr, wss_virt)


wlp_attr(u8, wlp_assc_err)


struct wlp_frame_std_abbrv_hdr {
	struct wlp_frame_hdr hdr;
	u8 tag;
} __attribute__((packed));


struct wlp_frame_assoc {
	struct wlp_frame_hdr hdr;
	enum wlp_assoc_type type:8;
	struct wlp_attr_version version;
	struct wlp_attr_msg_type msg_type;
	u8 attr[];
} __attribute__((packed));


struct wlp_eda {
	spinlock_t lock;
	struct list_head cache;	
};


struct wlp_wss_tmp_info {
	char name[WLP_WSS_NAME_SIZE];
	u8 accept_enroll;
	u8 sec_status;
	struct uwb_mac_addr bcast;
};

struct wlp_wssid_e {
	struct list_head node;
	struct wlp_uuid wssid;
	struct wlp_wss_tmp_info *info;
};


struct wlp_neighbor_e {
	struct list_head node;
	struct wlp_uuid uuid;
	struct uwb_dev *uwb_dev;
	struct list_head wssid; 
	struct wlp_device_info *info;
};

struct wlp;

struct wlp_session {
	enum wlp_assoc_type exp_message;
	void (*cb)(struct wlp *);
	void *cb_priv;
	void *data;
	struct uwb_dev_addr neighbor_addr;
};


struct wlp_wss {
	struct mutex mutex;
	struct kobject kobj;
	
	struct wlp_uuid wssid;
	u8 hash;
	char name[WLP_WSS_NAME_SIZE];
	struct uwb_mac_addr bcast;
	u8 secure_status:1;
	u8 master_key[16];
	
	u8 tag;
	struct uwb_mac_addr virtual_addr;
	
	u8 accept_enroll:1;
	enum wlp_wss_state state;
};


struct wlp {
	struct mutex mutex;
	struct uwb_rc *rc;		
	struct net_device *ndev;
	struct uwb_pal pal;
	struct wlp_eda eda;
	struct wlp_uuid uuid;
	struct wlp_session *session;
	struct wlp_wss wss;
	struct mutex nbmutex; 
	struct list_head neighbors; 
	struct uwb_notifs_handler uwb_notifs_handler;
	struct wlp_device_info *dev_info;
	void (*fill_device_info)(struct wlp *wlp, struct wlp_device_info *info);
	int (*xmit_frame)(struct wlp *, struct sk_buff *,
			  struct uwb_dev_addr *);
	void (*stop_queue)(struct wlp *);
	void (*start_queue)(struct wlp *);
};




struct wlp_wss_attribute {
	struct attribute attr;
	ssize_t (*show)(struct wlp_wss *wss, char *buf);
	ssize_t (*store)(struct wlp_wss *wss, const char *buf, size_t count);
};

#define WSS_ATTR(_name, _mode, _show, _store) \
static struct wlp_wss_attribute wss_attr_##_name = __ATTR(_name, _mode,	\
							  _show, _store)

extern int wlp_setup(struct wlp *, struct uwb_rc *, struct net_device *ndev);
extern void wlp_remove(struct wlp *);
extern ssize_t wlp_neighborhood_show(struct wlp *, char *);
extern int wlp_wss_setup(struct net_device *, struct wlp_wss *);
extern void wlp_wss_remove(struct wlp_wss *);
extern ssize_t wlp_wss_activate_show(struct wlp_wss *, char *);
extern ssize_t wlp_wss_activate_store(struct wlp_wss *, const char *, size_t);
extern ssize_t wlp_eda_show(struct wlp *, char *);
extern ssize_t wlp_eda_store(struct wlp *, const char *, size_t);
extern ssize_t wlp_uuid_show(struct wlp *, char *);
extern ssize_t wlp_uuid_store(struct wlp *, const char *, size_t);
extern ssize_t wlp_dev_name_show(struct wlp *, char *);
extern ssize_t wlp_dev_name_store(struct wlp *, const char *, size_t);
extern ssize_t wlp_dev_manufacturer_show(struct wlp *, char *);
extern ssize_t wlp_dev_manufacturer_store(struct wlp *, const char *, size_t);
extern ssize_t wlp_dev_model_name_show(struct wlp *, char *);
extern ssize_t wlp_dev_model_name_store(struct wlp *, const char *, size_t);
extern ssize_t wlp_dev_model_nr_show(struct wlp *, char *);
extern ssize_t wlp_dev_model_nr_store(struct wlp *, const char *, size_t);
extern ssize_t wlp_dev_serial_show(struct wlp *, char *);
extern ssize_t wlp_dev_serial_store(struct wlp *, const char *, size_t);
extern ssize_t wlp_dev_prim_category_show(struct wlp *, char *);
extern ssize_t wlp_dev_prim_category_store(struct wlp *, const char *,
					   size_t);
extern ssize_t wlp_dev_prim_OUI_show(struct wlp *, char *);
extern ssize_t wlp_dev_prim_OUI_store(struct wlp *, const char *, size_t);
extern ssize_t wlp_dev_prim_OUI_sub_show(struct wlp *, char *);
extern ssize_t wlp_dev_prim_OUI_sub_store(struct wlp *, const char *,
					  size_t);
extern ssize_t wlp_dev_prim_subcat_show(struct wlp *, char *);
extern ssize_t wlp_dev_prim_subcat_store(struct wlp *, const char *,
					 size_t);
extern int wlp_receive_frame(struct device *, struct wlp *, struct sk_buff *,
			     struct uwb_dev_addr *);
extern int wlp_prepare_tx_frame(struct device *, struct wlp *,
			       struct sk_buff *, struct uwb_dev_addr *);
void wlp_reset_all(struct wlp *wlp);


static inline
void wlp_wss_init(struct wlp_wss *wss)
{
	mutex_init(&wss->mutex);
}

static inline
void wlp_init(struct wlp *wlp)
{
	INIT_LIST_HEAD(&wlp->neighbors);
	mutex_init(&wlp->mutex);
	mutex_init(&wlp->nbmutex);
	wlp_wss_init(&wlp->wss);
}


#endif 
