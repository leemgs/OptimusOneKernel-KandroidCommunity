

#include <net/iw_handler.h>
#include <net/lib80211.h>
#include <linux/kfifo.h>
#include <linux/sched.h>
#include "host.h"
#include "hostcmd.h"
#include "decl.h"
#include "defs.h"
#include "dev.h"
#include "assoc.h"
#include "wext.h"
#include "cmd.h"

static struct cmd_ctrl_node *lbs_get_cmd_ctrl_node(struct lbs_private *priv);



int lbs_cmd_copyback(struct lbs_private *priv, unsigned long extra,
		     struct cmd_header *resp)
{
	struct cmd_header *buf = (void *)extra;
	uint16_t copy_len;

	copy_len = min(le16_to_cpu(buf->size), le16_to_cpu(resp->size));
	memcpy(buf, resp, copy_len);
	return 0;
}
EXPORT_SYMBOL_GPL(lbs_cmd_copyback);


static int lbs_cmd_async_callback(struct lbs_private *priv, unsigned long extra,
		     struct cmd_header *resp)
{
	return 0;
}



static u8 is_command_allowed_in_ps(u16 cmd)
{
	switch (cmd) {
	case CMD_802_11_RSSI:
		return 1;
	default:
		break;
	}
	return 0;
}


int lbs_update_hw_spec(struct lbs_private *priv)
{
	struct cmd_ds_get_hw_spec cmd;
	int ret = -1;
	u32 i;

	lbs_deb_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	memcpy(cmd.permanentaddr, priv->current_addr, ETH_ALEN);
	ret = lbs_cmd_with_response(priv, CMD_GET_HW_SPEC, &cmd);
	if (ret)
		goto out;

	priv->fwcapinfo = le32_to_cpu(cmd.fwcapinfo);

	
	priv->fwrelease = le32_to_cpu(cmd.fwrelease);
	priv->fwrelease = (priv->fwrelease << 8) |
		(priv->fwrelease >> 24 & 0xff);

	
	lbs_pr_info("%pM, fw %u.%u.%up%u, cap 0x%08x\n",
		cmd.permanentaddr,
		priv->fwrelease >> 24 & 0xff,
		priv->fwrelease >> 16 & 0xff,
		priv->fwrelease >>  8 & 0xff,
		priv->fwrelease       & 0xff,
		priv->fwcapinfo);
	lbs_deb_cmd("GET_HW_SPEC: hardware interface 0x%x, hardware spec 0x%04x\n",
		    cmd.hwifversion, cmd.version);

	
	
	
	
	
	if (MRVL_FW_MAJOR_REV(priv->fwrelease) == MRVL_FW_V5)
		priv->mesh_fw_ver = MESH_FW_OLD;
	else if ((MRVL_FW_MAJOR_REV(priv->fwrelease) >= MRVL_FW_V10) &&
		(priv->fwcapinfo & MESH_CAPINFO_ENABLE_MASK))
		priv->mesh_fw_ver = MESH_FW_NEW;
	else
		priv->mesh_fw_ver = MESH_NONE;

	
	if (MRVL_FW_MAJOR_REV(priv->fwrelease) == MRVL_FW_V4)
		priv->regioncode = (le16_to_cpu(cmd.regioncode) >> 8) & 0xFF;
	else
		priv->regioncode = le16_to_cpu(cmd.regioncode) & 0xFF;

	for (i = 0; i < MRVDRV_MAX_REGION_CODE; i++) {
		
		if (priv->regioncode == lbs_region_code_to_index[i])
			break;
	}

	
	if (i >= MRVDRV_MAX_REGION_CODE) {
		priv->regioncode = 0x10;
		lbs_pr_info("unidentified region code; using the default (USA)\n");
	}

	if (priv->current_addr[0] == 0xff)
		memmove(priv->current_addr, cmd.permanentaddr, ETH_ALEN);

	memcpy(priv->dev->dev_addr, priv->current_addr, ETH_ALEN);
	if (priv->mesh_dev)
		memcpy(priv->mesh_dev->dev_addr, priv->current_addr, ETH_ALEN);

	if (lbs_set_regiontable(priv, priv->regioncode, 0)) {
		ret = -1;
		goto out;
	}

	if (lbs_set_universaltable(priv, 0)) {
		ret = -1;
		goto out;
	}

out:
	lbs_deb_leave(LBS_DEB_CMD);
	return ret;
}

int lbs_host_sleep_cfg(struct lbs_private *priv, uint32_t criteria,
		struct wol_config *p_wol_config)
{
	struct cmd_ds_host_sleep cmd_config;
	int ret;

	cmd_config.hdr.size = cpu_to_le16(sizeof(cmd_config));
	cmd_config.criteria = cpu_to_le32(criteria);
	cmd_config.gpio = priv->wol_gpio;
	cmd_config.gap = priv->wol_gap;

	if (p_wol_config != NULL)
		memcpy((uint8_t *)&cmd_config.wol_conf, (uint8_t *)p_wol_config,
				sizeof(struct wol_config));
	else
		cmd_config.wol_conf.action = CMD_ACT_ACTION_NONE;

	ret = lbs_cmd_with_response(priv, CMD_802_11_HOST_SLEEP_CFG, &cmd_config);
	if (!ret) {
		if (criteria) {
			lbs_deb_cmd("Set WOL criteria to %x\n", criteria);
			priv->wol_criteria = criteria;
		} else
			memcpy((uint8_t *) p_wol_config,
					(uint8_t *)&cmd_config.wol_conf,
					sizeof(struct wol_config));
	} else {
		lbs_pr_info("HOST_SLEEP_CFG failed %d\n", ret);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(lbs_host_sleep_cfg);

static int lbs_cmd_802_11_ps_mode(struct cmd_ds_command *cmd,
				   u16 cmd_action)
{
	struct cmd_ds_802_11_ps_mode *psm = &cmd->params.psmode;

	lbs_deb_enter(LBS_DEB_CMD);

	cmd->command = cpu_to_le16(CMD_802_11_PS_MODE);
	cmd->size = cpu_to_le16(sizeof(struct cmd_ds_802_11_ps_mode) +
				S_DS_GEN);
	psm->action = cpu_to_le16(cmd_action);
	psm->multipledtim = 0;
	switch (cmd_action) {
	case CMD_SUBCMD_ENTER_PS:
		lbs_deb_cmd("PS command:" "SubCode- Enter PS\n");

		psm->locallisteninterval = 0;
		psm->nullpktinterval = 0;
		psm->multipledtim =
		    cpu_to_le16(MRVDRV_DEFAULT_MULTIPLE_DTIM);
		break;

	case CMD_SUBCMD_EXIT_PS:
		lbs_deb_cmd("PS command:" "SubCode- Exit PS\n");
		break;

	case CMD_SUBCMD_SLEEP_CONFIRMED:
		lbs_deb_cmd("PS command: SubCode- sleep confirm\n");
		break;

	default:
		break;
	}

	lbs_deb_leave(LBS_DEB_CMD);
	return 0;
}

int lbs_cmd_802_11_inactivity_timeout(struct lbs_private *priv,
				      uint16_t cmd_action, uint16_t *timeout)
{
	struct cmd_ds_802_11_inactivity_timeout cmd;
	int ret;

	lbs_deb_enter(LBS_DEB_CMD);

	cmd.hdr.command = cpu_to_le16(CMD_802_11_INACTIVITY_TIMEOUT);
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));

	cmd.action = cpu_to_le16(cmd_action);

	if (cmd_action == CMD_ACT_SET)
		cmd.timeout = cpu_to_le16(*timeout);
	else
		cmd.timeout = 0;

	ret = lbs_cmd_with_response(priv, CMD_802_11_INACTIVITY_TIMEOUT, &cmd);

	if (!ret)
		*timeout = le16_to_cpu(cmd.timeout);

	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return 0;
}

int lbs_cmd_802_11_sleep_params(struct lbs_private *priv, uint16_t cmd_action,
				struct sleep_params *sp)
{
	struct cmd_ds_802_11_sleep_params cmd;
	int ret;

	lbs_deb_enter(LBS_DEB_CMD);

	if (cmd_action == CMD_ACT_GET) {
		memset(&cmd, 0, sizeof(cmd));
	} else {
		cmd.error = cpu_to_le16(sp->sp_error);
		cmd.offset = cpu_to_le16(sp->sp_offset);
		cmd.stabletime = cpu_to_le16(sp->sp_stabletime);
		cmd.calcontrol = sp->sp_calcontrol;
		cmd.externalsleepclk = sp->sp_extsleepclk;
		cmd.reserved = cpu_to_le16(sp->sp_reserved);
	}
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(cmd_action);

	ret = lbs_cmd_with_response(priv, CMD_802_11_SLEEP_PARAMS, &cmd);

	if (!ret) {
		lbs_deb_cmd("error 0x%x, offset 0x%x, stabletime 0x%x, "
			    "calcontrol 0x%x extsleepclk 0x%x\n",
			    le16_to_cpu(cmd.error), le16_to_cpu(cmd.offset),
			    le16_to_cpu(cmd.stabletime), cmd.calcontrol,
			    cmd.externalsleepclk);

		sp->sp_error = le16_to_cpu(cmd.error);
		sp->sp_offset = le16_to_cpu(cmd.offset);
		sp->sp_stabletime = le16_to_cpu(cmd.stabletime);
		sp->sp_calcontrol = cmd.calcontrol;
		sp->sp_extsleepclk = cmd.externalsleepclk;
		sp->sp_reserved = le16_to_cpu(cmd.reserved);
	}

	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return 0;
}

int lbs_cmd_802_11_set_wep(struct lbs_private *priv, uint16_t cmd_action,
			   struct assoc_request *assoc)
{
	struct cmd_ds_802_11_set_wep cmd;
	int ret = 0;

	lbs_deb_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.command = cpu_to_le16(CMD_802_11_SET_WEP);
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));

	cmd.action = cpu_to_le16(cmd_action);

	if (cmd_action == CMD_ACT_ADD) {
		int i;

		
		cmd.keyindex = cpu_to_le16(assoc->wep_tx_keyidx &
					   CMD_WEP_KEY_INDEX_MASK);

		
		for (i = 0; i < 4; i++) {
			struct enc_key *pkey = &assoc->wep_keys[i];

			switch (pkey->len) {
			case KEY_LEN_WEP_40:
				cmd.keytype[i] = CMD_TYPE_WEP_40_BIT;
				memmove(cmd.keymaterial[i], pkey->key, pkey->len);
				lbs_deb_cmd("SET_WEP: add key %d (40 bit)\n", i);
				break;
			case KEY_LEN_WEP_104:
				cmd.keytype[i] = CMD_TYPE_WEP_104_BIT;
				memmove(cmd.keymaterial[i], pkey->key, pkey->len);
				lbs_deb_cmd("SET_WEP: add key %d (104 bit)\n", i);
				break;
			case 0:
				break;
			default:
				lbs_deb_cmd("SET_WEP: invalid key %d, length %d\n",
					    i, pkey->len);
				ret = -1;
				goto done;
				break;
			}
		}
	} else if (cmd_action == CMD_ACT_REMOVE) {
		

		
		cmd.keyindex = cpu_to_le16(priv->wep_tx_keyidx &
					   CMD_WEP_KEY_INDEX_MASK);
		lbs_deb_cmd("SET_WEP: remove key %d\n", priv->wep_tx_keyidx);
	}

	ret = lbs_cmd_with_response(priv, CMD_802_11_SET_WEP, &cmd);
done:
	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}

int lbs_cmd_802_11_enable_rsn(struct lbs_private *priv, uint16_t cmd_action,
			      uint16_t *enable)
{
	struct cmd_ds_802_11_enable_rsn cmd;
	int ret;

	lbs_deb_enter(LBS_DEB_CMD);

	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(cmd_action);

	if (cmd_action == CMD_ACT_GET)
		cmd.enable = 0;
	else {
		if (*enable)
			cmd.enable = cpu_to_le16(CMD_ENABLE_RSN);
		else
			cmd.enable = cpu_to_le16(CMD_DISABLE_RSN);
		lbs_deb_cmd("ENABLE_RSN: %d\n", *enable);
	}

	ret = lbs_cmd_with_response(priv, CMD_802_11_ENABLE_RSN, &cmd);
	if (!ret && cmd_action == CMD_ACT_GET)
		*enable = le16_to_cpu(cmd.enable);

	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}

static void set_one_wpa_key(struct MrvlIEtype_keyParamSet *keyparam,
                            struct enc_key *key)
{
	lbs_deb_enter(LBS_DEB_CMD);

	if (key->flags & KEY_INFO_WPA_ENABLED)
		keyparam->keyinfo |= cpu_to_le16(KEY_INFO_WPA_ENABLED);
	if (key->flags & KEY_INFO_WPA_UNICAST)
		keyparam->keyinfo |= cpu_to_le16(KEY_INFO_WPA_UNICAST);
	if (key->flags & KEY_INFO_WPA_MCAST)
		keyparam->keyinfo |= cpu_to_le16(KEY_INFO_WPA_MCAST);

	keyparam->type = cpu_to_le16(TLV_TYPE_KEY_MATERIAL);
	keyparam->keytypeid = cpu_to_le16(key->type);
	keyparam->keylen = cpu_to_le16(key->len);
	memcpy(keyparam->key, key->key, key->len);

	
	keyparam->length = cpu_to_le16(sizeof(*keyparam) - 4);
	lbs_deb_leave(LBS_DEB_CMD);
}

int lbs_cmd_802_11_key_material(struct lbs_private *priv, uint16_t cmd_action,
				struct assoc_request *assoc)
{
	struct cmd_ds_802_11_key_material cmd;
	int ret = 0;
	int index = 0;

	lbs_deb_enter(LBS_DEB_CMD);

	cmd.action = cpu_to_le16(cmd_action);
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));

	if (cmd_action == CMD_ACT_GET) {
		cmd.hdr.size = cpu_to_le16(S_DS_GEN + 2);
	} else {
		memset(cmd.keyParamSet, 0, sizeof(cmd.keyParamSet));

		if (test_bit(ASSOC_FLAG_WPA_UCAST_KEY, &assoc->flags)) {
			set_one_wpa_key(&cmd.keyParamSet[index],
					&assoc->wpa_unicast_key);
			index++;
		}

		if (test_bit(ASSOC_FLAG_WPA_MCAST_KEY, &assoc->flags)) {
			set_one_wpa_key(&cmd.keyParamSet[index],
					&assoc->wpa_mcast_key);
			index++;
		}

		
		cmd.hdr.size = cpu_to_le16(offsetof(typeof(cmd),
						    keyParamSet[index]));
	}
	ret = lbs_cmd_with_response(priv, CMD_802_11_KEY_MATERIAL, &cmd);
	
	if (!ret && cmd_action == CMD_ACT_GET) {
		void *buf_ptr = cmd.keyParamSet;
		void *resp_end = &(&cmd)[1];

		while (buf_ptr < resp_end) {
			struct MrvlIEtype_keyParamSet *keyparam = buf_ptr;
			struct enc_key *key;
			uint16_t param_set_len = le16_to_cpu(keyparam->length);
			uint16_t key_len = le16_to_cpu(keyparam->keylen);
			uint16_t key_flags = le16_to_cpu(keyparam->keyinfo);
			uint16_t key_type = le16_to_cpu(keyparam->keytypeid);
			void *end;

			end = (void *)keyparam + sizeof(keyparam->type)
				+ sizeof(keyparam->length) + param_set_len;

			
			if (end > resp_end)
				break;

			if (key_flags & KEY_INFO_WPA_UNICAST)
				key = &priv->wpa_unicast_key;
			else if (key_flags & KEY_INFO_WPA_MCAST)
				key = &priv->wpa_mcast_key;
			else
				break;

			
			memset(key, 0, sizeof(struct enc_key));
			if (key_len > sizeof(key->key))
				break;
			key->type = key_type;
			key->flags = key_flags;
			key->len = key_len;
			memcpy(key->key, keyparam->key, key->len);

			buf_ptr = end + 1;
		}
	}

	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}


int lbs_set_snmp_mib(struct lbs_private *priv, u32 oid, u16 val)
{
	struct cmd_ds_802_11_snmp_mib cmd;
	int ret;

	lbs_deb_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof (cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_SET);
	cmd.oid = cpu_to_le16((u16) oid);

	switch (oid) {
	case SNMP_MIB_OID_BSS_TYPE:
		cmd.bufsize = cpu_to_le16(sizeof(u8));
		cmd.value[0] = (val == IW_MODE_ADHOC) ? 2 : 1;
		break;
	case SNMP_MIB_OID_11D_ENABLE:
	case SNMP_MIB_OID_FRAG_THRESHOLD:
	case SNMP_MIB_OID_RTS_THRESHOLD:
	case SNMP_MIB_OID_SHORT_RETRY_LIMIT:
	case SNMP_MIB_OID_LONG_RETRY_LIMIT:
		cmd.bufsize = cpu_to_le16(sizeof(u16));
		*((__le16 *)(&cmd.value)) = cpu_to_le16(val);
		break;
	default:
		lbs_deb_cmd("SNMP_CMD: (set) unhandled OID 0x%x\n", oid);
		ret = -EINVAL;
		goto out;
	}

	lbs_deb_cmd("SNMP_CMD: (set) oid 0x%x, oid size 0x%x, value 0x%x\n",
		    le16_to_cpu(cmd.oid), le16_to_cpu(cmd.bufsize), val);

	ret = lbs_cmd_with_response(priv, CMD_802_11_SNMP_MIB, &cmd);

out:
	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}


int lbs_get_snmp_mib(struct lbs_private *priv, u32 oid, u16 *out_val)
{
	struct cmd_ds_802_11_snmp_mib cmd;
	int ret;

	lbs_deb_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof (cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_GET);
	cmd.oid = cpu_to_le16(oid);

	ret = lbs_cmd_with_response(priv, CMD_802_11_SNMP_MIB, &cmd);
	if (ret)
		goto out;

	switch (le16_to_cpu(cmd.bufsize)) {
	case sizeof(u8):
		if (oid == SNMP_MIB_OID_BSS_TYPE) {
			if (cmd.value[0] == 2)
				*out_val = IW_MODE_ADHOC;
			else
				*out_val = IW_MODE_INFRA;
		} else
			*out_val = cmd.value[0];
		break;
	case sizeof(u16):
		*out_val = le16_to_cpu(*((__le16 *)(&cmd.value)));
		break;
	default:
		lbs_deb_cmd("SNMP_CMD: (get) unhandled OID 0x%x size %d\n",
		            oid, le16_to_cpu(cmd.bufsize));
		break;
	}

out:
	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}


int lbs_get_tx_power(struct lbs_private *priv, s16 *curlevel, s16 *minlevel,
		     s16 *maxlevel)
{
	struct cmd_ds_802_11_rf_tx_power cmd;
	int ret;

	lbs_deb_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_GET);

	ret = lbs_cmd_with_response(priv, CMD_802_11_RF_TX_POWER, &cmd);
	if (ret == 0) {
		*curlevel = le16_to_cpu(cmd.curlevel);
		if (minlevel)
			*minlevel = cmd.minlevel;
		if (maxlevel)
			*maxlevel = cmd.maxlevel;
	}

	lbs_deb_leave(LBS_DEB_CMD);
	return ret;
}


int lbs_set_tx_power(struct lbs_private *priv, s16 dbm)
{
	struct cmd_ds_802_11_rf_tx_power cmd;
	int ret;

	lbs_deb_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_SET);
	cmd.curlevel = cpu_to_le16(dbm);

	lbs_deb_cmd("SET_RF_TX_POWER: %d dBm\n", dbm);

	ret = lbs_cmd_with_response(priv, CMD_802_11_RF_TX_POWER, &cmd);

	lbs_deb_leave(LBS_DEB_CMD);
	return ret;
}

static int lbs_cmd_802_11_monitor_mode(struct cmd_ds_command *cmd,
				      u16 cmd_action, void *pdata_buf)
{
	struct cmd_ds_802_11_monitor_mode *monitor = &cmd->params.monitor;

	cmd->command = cpu_to_le16(CMD_802_11_MONITOR_MODE);
	cmd->size =
	    cpu_to_le16(sizeof(struct cmd_ds_802_11_monitor_mode) +
			     S_DS_GEN);

	monitor->action = cpu_to_le16(cmd_action);
	if (cmd_action == CMD_ACT_SET) {
		monitor->mode =
		    cpu_to_le16((u16) (*(u32 *) pdata_buf));
	}

	return 0;
}

static __le16 lbs_rate_to_fw_bitmap(int rate, int lower_rates_ok)
{


	uint16_t ratemask;
	int i = lbs_data_rate_to_fw_index(rate);
	if (lower_rates_ok)
		ratemask = (0x1fef >> (12 - i));
	else
		ratemask = (1 << i);
	return cpu_to_le16(ratemask);
}

int lbs_cmd_802_11_rate_adapt_rateset(struct lbs_private *priv,
				      uint16_t cmd_action)
{
	struct cmd_ds_802_11_rate_adapt_rateset cmd;
	int ret;

	lbs_deb_enter(LBS_DEB_CMD);

	if (!priv->cur_rate && !priv->enablehwauto)
		return -EINVAL;

	cmd.hdr.size = cpu_to_le16(sizeof(cmd));

	cmd.action = cpu_to_le16(cmd_action);
	cmd.enablehwauto = cpu_to_le16(priv->enablehwauto);
	cmd.bitmap = lbs_rate_to_fw_bitmap(priv->cur_rate, priv->enablehwauto);
	ret = lbs_cmd_with_response(priv, CMD_802_11_RATE_ADAPT_RATESET, &cmd);
	if (!ret && cmd_action == CMD_ACT_GET) {
		priv->ratebitmap = le16_to_cpu(cmd.bitmap);
		priv->enablehwauto = le16_to_cpu(cmd.enablehwauto);
	}

	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(lbs_cmd_802_11_rate_adapt_rateset);


int lbs_set_data_rate(struct lbs_private *priv, u8 rate)
{
	struct cmd_ds_802_11_data_rate cmd;
	int ret = 0;

	lbs_deb_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));

	if (rate > 0) {
		cmd.action = cpu_to_le16(CMD_ACT_SET_TX_FIX_RATE);
		cmd.rates[0] = lbs_data_rate_to_fw_index(rate);
		if (cmd.rates[0] == 0) {
			lbs_deb_cmd("DATA_RATE: invalid requested rate of"
			            " 0x%02X\n", rate);
			ret = 0;
			goto out;
		}
		lbs_deb_cmd("DATA_RATE: set fixed 0x%02X\n", cmd.rates[0]);
	} else {
		cmd.action = cpu_to_le16(CMD_ACT_SET_TX_AUTO);
		lbs_deb_cmd("DATA_RATE: setting auto\n");
	}

	ret = lbs_cmd_with_response(priv, CMD_802_11_DATA_RATE, &cmd);
	if (ret)
		goto out;

	lbs_deb_hex(LBS_DEB_CMD, "DATA_RATE_RESP", (u8 *) &cmd, sizeof (cmd));

	
	priv->cur_rate = lbs_fw_index_to_data_rate(cmd.rates[0]);
	lbs_deb_cmd("DATA_RATE: current rate is 0x%02x\n", priv->cur_rate);

out:
	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}


int lbs_get_channel(struct lbs_private *priv)
{
	struct cmd_ds_802_11_rf_channel cmd;
	int ret = 0;

	lbs_deb_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_OPT_802_11_RF_CHANNEL_GET);

	ret = lbs_cmd_with_response(priv, CMD_802_11_RF_CHANNEL, &cmd);
	if (ret)
		goto out;

	ret = le16_to_cpu(cmd.channel);
	lbs_deb_cmd("current radio channel is %d\n", ret);

out:
	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}

int lbs_update_channel(struct lbs_private *priv)
{
	int ret;

	
	lbs_deb_enter(LBS_DEB_ASSOC);

	ret = lbs_get_channel(priv);
	if (ret > 0) {
		priv->curbssparams.channel = ret;
		ret = 0;
	}
	lbs_deb_leave_args(LBS_DEB_ASSOC, "ret %d", ret);
	return ret;
}


int lbs_set_channel(struct lbs_private *priv, u8 channel)
{
	struct cmd_ds_802_11_rf_channel cmd;
#ifdef DEBUG
	u8 old_channel = priv->curbssparams.channel;
#endif
	int ret = 0;

	lbs_deb_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_OPT_802_11_RF_CHANNEL_SET);
	cmd.channel = cpu_to_le16(channel);

	ret = lbs_cmd_with_response(priv, CMD_802_11_RF_CHANNEL, &cmd);
	if (ret)
		goto out;

	priv->curbssparams.channel = (uint8_t) le16_to_cpu(cmd.channel);
	lbs_deb_cmd("channel switch from %d to %d\n", old_channel,
		priv->curbssparams.channel);

out:
	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}

static int lbs_cmd_802_11_rssi(struct lbs_private *priv,
				struct cmd_ds_command *cmd)
{

	lbs_deb_enter(LBS_DEB_CMD);
	cmd->command = cpu_to_le16(CMD_802_11_RSSI);
	cmd->size = cpu_to_le16(sizeof(struct cmd_ds_802_11_rssi) + S_DS_GEN);
	cmd->params.rssi.N = cpu_to_le16(DEFAULT_BCN_AVG_FACTOR);

	
	priv->SNR[TYPE_BEACON][TYPE_NOAVG] = 0;
	priv->SNR[TYPE_BEACON][TYPE_AVG] = 0;
	priv->NF[TYPE_BEACON][TYPE_NOAVG] = 0;
	priv->NF[TYPE_BEACON][TYPE_AVG] = 0;
	priv->RSSI[TYPE_BEACON][TYPE_NOAVG] = 0;
	priv->RSSI[TYPE_BEACON][TYPE_AVG] = 0;

	lbs_deb_leave(LBS_DEB_CMD);
	return 0;
}

static int lbs_cmd_reg_access(struct cmd_ds_command *cmdptr,
			       u8 cmd_action, void *pdata_buf)
{
	struct lbs_offset_value *offval;

	lbs_deb_enter(LBS_DEB_CMD);

	offval = (struct lbs_offset_value *)pdata_buf;

	switch (le16_to_cpu(cmdptr->command)) {
	case CMD_MAC_REG_ACCESS:
		{
			struct cmd_ds_mac_reg_access *macreg;

			cmdptr->size =
			    cpu_to_le16(sizeof (struct cmd_ds_mac_reg_access)
					+ S_DS_GEN);
			macreg =
			    (struct cmd_ds_mac_reg_access *)&cmdptr->params.
			    macreg;

			macreg->action = cpu_to_le16(cmd_action);
			macreg->offset = cpu_to_le16((u16) offval->offset);
			macreg->value = cpu_to_le32(offval->value);

			break;
		}

	case CMD_BBP_REG_ACCESS:
		{
			struct cmd_ds_bbp_reg_access *bbpreg;

			cmdptr->size =
			    cpu_to_le16(sizeof
					     (struct cmd_ds_bbp_reg_access)
					     + S_DS_GEN);
			bbpreg =
			    (struct cmd_ds_bbp_reg_access *)&cmdptr->params.
			    bbpreg;

			bbpreg->action = cpu_to_le16(cmd_action);
			bbpreg->offset = cpu_to_le16((u16) offval->offset);
			bbpreg->value = (u8) offval->value;

			break;
		}

	case CMD_RF_REG_ACCESS:
		{
			struct cmd_ds_rf_reg_access *rfreg;

			cmdptr->size =
			    cpu_to_le16(sizeof
					     (struct cmd_ds_rf_reg_access) +
					     S_DS_GEN);
			rfreg =
			    (struct cmd_ds_rf_reg_access *)&cmdptr->params.
			    rfreg;

			rfreg->action = cpu_to_le16(cmd_action);
			rfreg->offset = cpu_to_le16((u16) offval->offset);
			rfreg->value = (u8) offval->value;

			break;
		}

	default:
		break;
	}

	lbs_deb_leave(LBS_DEB_CMD);
	return 0;
}

static int lbs_cmd_bt_access(struct cmd_ds_command *cmd,
			       u16 cmd_action, void *pdata_buf)
{
	struct cmd_ds_bt_access *bt_access = &cmd->params.bt;
	lbs_deb_enter_args(LBS_DEB_CMD, "action %d", cmd_action);

	cmd->command = cpu_to_le16(CMD_BT_ACCESS);
	cmd->size = cpu_to_le16(sizeof(struct cmd_ds_bt_access) + S_DS_GEN);
	cmd->result = 0;
	bt_access->action = cpu_to_le16(cmd_action);

	switch (cmd_action) {
	case CMD_ACT_BT_ACCESS_ADD:
		memcpy(bt_access->addr1, pdata_buf, 2 * ETH_ALEN);
		lbs_deb_hex(LBS_DEB_MESH, "BT_ADD: blinded MAC addr", bt_access->addr1, 6);
		break;
	case CMD_ACT_BT_ACCESS_DEL:
		memcpy(bt_access->addr1, pdata_buf, 1 * ETH_ALEN);
		lbs_deb_hex(LBS_DEB_MESH, "BT_DEL: blinded MAC addr", bt_access->addr1, 6);
		break;
	case CMD_ACT_BT_ACCESS_LIST:
		bt_access->id = cpu_to_le32(*(u32 *) pdata_buf);
		break;
	case CMD_ACT_BT_ACCESS_RESET:
		break;
	case CMD_ACT_BT_ACCESS_SET_INVERT:
		bt_access->id = cpu_to_le32(*(u32 *) pdata_buf);
		break;
	case CMD_ACT_BT_ACCESS_GET_INVERT:
		break;
	default:
		break;
	}
	lbs_deb_leave(LBS_DEB_CMD);
	return 0;
}

static int lbs_cmd_fwt_access(struct cmd_ds_command *cmd,
			       u16 cmd_action, void *pdata_buf)
{
	struct cmd_ds_fwt_access *fwt_access = &cmd->params.fwt;
	lbs_deb_enter_args(LBS_DEB_CMD, "action %d", cmd_action);

	cmd->command = cpu_to_le16(CMD_FWT_ACCESS);
	cmd->size = cpu_to_le16(sizeof(struct cmd_ds_fwt_access) + S_DS_GEN);
	cmd->result = 0;

	if (pdata_buf)
		memcpy(fwt_access, pdata_buf, sizeof(*fwt_access));
	else
		memset(fwt_access, 0, sizeof(*fwt_access));

	fwt_access->action = cpu_to_le16(cmd_action);

	lbs_deb_leave(LBS_DEB_CMD);
	return 0;
}

int lbs_mesh_access(struct lbs_private *priv, uint16_t cmd_action,
		    struct cmd_ds_mesh_access *cmd)
{
	int ret;

	lbs_deb_enter_args(LBS_DEB_CMD, "action %d", cmd_action);

	cmd->hdr.command = cpu_to_le16(CMD_MESH_ACCESS);
	cmd->hdr.size = cpu_to_le16(sizeof(*cmd));
	cmd->hdr.result = 0;

	cmd->action = cpu_to_le16(cmd_action);

	ret = lbs_cmd_with_response(priv, CMD_MESH_ACCESS, cmd);

	lbs_deb_leave(LBS_DEB_CMD);
	return ret;
}

static int __lbs_mesh_config_send(struct lbs_private *priv,
				  struct cmd_ds_mesh_config *cmd,
				  uint16_t action, uint16_t type)
{
	int ret;
	u16 command = CMD_MESH_CONFIG_OLD;

	lbs_deb_enter(LBS_DEB_CMD);

	
	if (priv->mesh_fw_ver == MESH_FW_NEW)
		command = CMD_MESH_CONFIG |
			  (MESH_IFACE_ID << MESH_IFACE_BIT_OFFSET);

	cmd->hdr.command = cpu_to_le16(command);
	cmd->hdr.size = cpu_to_le16(sizeof(struct cmd_ds_mesh_config));
	cmd->hdr.result = 0;

	cmd->type = cpu_to_le16(type);
	cmd->action = cpu_to_le16(action);

	ret = lbs_cmd_with_response(priv, command, cmd);

	lbs_deb_leave(LBS_DEB_CMD);
	return ret;
}

int lbs_mesh_config_send(struct lbs_private *priv,
			 struct cmd_ds_mesh_config *cmd,
			 uint16_t action, uint16_t type)
{
	int ret;

	if (!(priv->fwcapinfo & FW_CAPINFO_PERSISTENT_CONFIG))
		return -EOPNOTSUPP;

	ret = __lbs_mesh_config_send(priv, cmd, action, type);
	return ret;
}


int lbs_mesh_config(struct lbs_private *priv, uint16_t action, uint16_t chan)
{
	struct cmd_ds_mesh_config cmd;
	struct mrvl_meshie *ie;
	DECLARE_SSID_BUF(ssid);

	memset(&cmd, 0, sizeof(cmd));
	cmd.channel = cpu_to_le16(chan);
	ie = (struct mrvl_meshie *)cmd.data;

	switch (action) {
	case CMD_ACT_MESH_CONFIG_START:
		ie->id = WLAN_EID_GENERIC;
		ie->val.oui[0] = 0x00;
		ie->val.oui[1] = 0x50;
		ie->val.oui[2] = 0x43;
		ie->val.type = MARVELL_MESH_IE_TYPE;
		ie->val.subtype = MARVELL_MESH_IE_SUBTYPE;
		ie->val.version = MARVELL_MESH_IE_VERSION;
		ie->val.active_protocol_id = MARVELL_MESH_PROTO_ID_HWMP;
		ie->val.active_metric_id = MARVELL_MESH_METRIC_ID;
		ie->val.mesh_capability = MARVELL_MESH_CAPABILITY;
		ie->val.mesh_id_len = priv->mesh_ssid_len;
		memcpy(ie->val.mesh_id, priv->mesh_ssid, priv->mesh_ssid_len);
		ie->len = sizeof(struct mrvl_meshie_val) -
			IW_ESSID_MAX_SIZE + priv->mesh_ssid_len;
		cmd.length = cpu_to_le16(sizeof(struct mrvl_meshie_val));
		break;
	case CMD_ACT_MESH_CONFIG_STOP:
		break;
	default:
		return -1;
	}
	lbs_deb_cmd("mesh config action %d type %x channel %d SSID %s\n",
		    action, priv->mesh_tlv, chan,
		    print_ssid(ssid, priv->mesh_ssid, priv->mesh_ssid_len));

	return __lbs_mesh_config_send(priv, &cmd, action, priv->mesh_tlv);
}

static int lbs_cmd_bcn_ctrl(struct lbs_private * priv,
				struct cmd_ds_command *cmd,
				u16 cmd_action)
{
	struct cmd_ds_802_11_beacon_control
		*bcn_ctrl = &cmd->params.bcn_ctrl;

	lbs_deb_enter(LBS_DEB_CMD);
	cmd->size =
	    cpu_to_le16(sizeof(struct cmd_ds_802_11_beacon_control)
			     + S_DS_GEN);
	cmd->command = cpu_to_le16(CMD_802_11_BEACON_CTRL);

	bcn_ctrl->action = cpu_to_le16(cmd_action);
	bcn_ctrl->beacon_enable = cpu_to_le16(priv->beacon_enable);
	bcn_ctrl->beacon_period = cpu_to_le16(priv->beacon_period);

	lbs_deb_leave(LBS_DEB_CMD);
	return 0;
}

static void lbs_queue_cmd(struct lbs_private *priv,
			  struct cmd_ctrl_node *cmdnode)
{
	unsigned long flags;
	int addtail = 1;

	lbs_deb_enter(LBS_DEB_HOST);

	if (!cmdnode) {
		lbs_deb_host("QUEUE_CMD: cmdnode is NULL\n");
		goto done;
	}
	if (!cmdnode->cmdbuf->size) {
		lbs_deb_host("DNLD_CMD: cmd size is zero\n");
		goto done;
	}
	cmdnode->result = 0;

	
	if (le16_to_cpu(cmdnode->cmdbuf->command) == CMD_802_11_PS_MODE) {
		struct cmd_ds_802_11_ps_mode *psm = (void *) &cmdnode->cmdbuf[1];

		if (psm->action == cpu_to_le16(CMD_SUBCMD_EXIT_PS)) {
			if (priv->psstate != PS_STATE_FULL_POWER)
				addtail = 0;
		}
	}

	spin_lock_irqsave(&priv->driver_lock, flags);

	if (addtail)
		list_add_tail(&cmdnode->list, &priv->cmdpendingq);
	else
		list_add(&cmdnode->list, &priv->cmdpendingq);

	spin_unlock_irqrestore(&priv->driver_lock, flags);

	lbs_deb_host("QUEUE_CMD: inserted command 0x%04x into cmdpendingq\n",
		     le16_to_cpu(cmdnode->cmdbuf->command));

done:
	lbs_deb_leave(LBS_DEB_HOST);
}

static void lbs_submit_command(struct lbs_private *priv,
			       struct cmd_ctrl_node *cmdnode)
{
	unsigned long flags;
	struct cmd_header *cmd;
	uint16_t cmdsize;
	uint16_t command;
	int timeo = 3 * HZ;
	int ret;

	lbs_deb_enter(LBS_DEB_HOST);

	cmd = cmdnode->cmdbuf;

	spin_lock_irqsave(&priv->driver_lock, flags);
	priv->cur_cmd = cmdnode;
	priv->cur_cmd_retcode = 0;
	spin_unlock_irqrestore(&priv->driver_lock, flags);

	cmdsize = le16_to_cpu(cmd->size);
	command = le16_to_cpu(cmd->command);

	
	if (command == CMD_802_11_SCAN || command == CMD_802_11_ASSOCIATE)
		timeo = 5 * HZ;

	lbs_deb_cmd("DNLD_CMD: command 0x%04x, seq %d, size %d\n",
		     command, le16_to_cpu(cmd->seqnum), cmdsize);
	lbs_deb_hex(LBS_DEB_CMD, "DNLD_CMD", (void *) cmdnode->cmdbuf, cmdsize);

	ret = priv->hw_host_to_card(priv, MVMS_CMD, (u8 *) cmd, cmdsize);

	if (ret) {
		lbs_pr_info("DNLD_CMD: hw_host_to_card failed: %d\n", ret);
		
		timeo = HZ/4;
	}

	
	mod_timer(&priv->command_timer, jiffies + timeo);

	lbs_deb_leave(LBS_DEB_HOST);
}


static void __lbs_cleanup_and_insert_cmd(struct lbs_private *priv,
					 struct cmd_ctrl_node *cmdnode)
{
	lbs_deb_enter(LBS_DEB_HOST);

	if (!cmdnode)
		goto out;

	cmdnode->callback = NULL;
	cmdnode->callback_arg = 0;

	memset(cmdnode->cmdbuf, 0, LBS_CMD_BUFFER_SIZE);

	list_add_tail(&cmdnode->list, &priv->cmdfreeq);
 out:
	lbs_deb_leave(LBS_DEB_HOST);
}

static void lbs_cleanup_and_insert_cmd(struct lbs_private *priv,
	struct cmd_ctrl_node *ptempcmd)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->driver_lock, flags);
	__lbs_cleanup_and_insert_cmd(priv, ptempcmd);
	spin_unlock_irqrestore(&priv->driver_lock, flags);
}

void lbs_complete_command(struct lbs_private *priv, struct cmd_ctrl_node *cmd,
			  int result)
{
	if (cmd == priv->cur_cmd)
		priv->cur_cmd_retcode = result;

	cmd->result = result;
	cmd->cmdwaitqwoken = 1;
	wake_up_interruptible(&cmd->cmdwait_q);

	if (!cmd->callback || cmd->callback == lbs_cmd_async_callback)
		__lbs_cleanup_and_insert_cmd(priv, cmd);
	priv->cur_cmd = NULL;
}

int lbs_set_radio(struct lbs_private *priv, u8 preamble, u8 radio_on)
{
	struct cmd_ds_802_11_radio_control cmd;
	int ret = -EINVAL;

	lbs_deb_enter(LBS_DEB_CMD);

	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_SET);

	
	if (priv->fwrelease < 0x09000000) {
		switch (preamble) {
		case RADIO_PREAMBLE_SHORT:
			if (!(priv->capability & WLAN_CAPABILITY_SHORT_PREAMBLE))
				goto out;
			
		case RADIO_PREAMBLE_AUTO:
		case RADIO_PREAMBLE_LONG:
			cmd.control = cpu_to_le16(preamble);
			break;
		default:
			goto out;
		}
	}

	if (radio_on)
		cmd.control |= cpu_to_le16(0x1);
	else {
		cmd.control &= cpu_to_le16(~0x1);
		priv->txpower_cur = 0;
	}

	lbs_deb_cmd("RADIO_CONTROL: radio %s, preamble %d\n",
		    radio_on ? "ON" : "OFF", preamble);

	priv->radio_on = radio_on;

	ret = lbs_cmd_with_response(priv, CMD_802_11_RADIO_CONTROL, &cmd);

out:
	lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}

void lbs_set_mac_control(struct lbs_private *priv)
{
	struct cmd_ds_mac_control cmd;

	lbs_deb_enter(LBS_DEB_CMD);

	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(priv->mac_control);
	cmd.reserved = 0;

	lbs_cmd_async(priv, CMD_MAC_CONTROL, &cmd.hdr, sizeof(cmd));

	lbs_deb_leave(LBS_DEB_CMD);
}


int lbs_prepare_and_send_command(struct lbs_private *priv,
			  u16 cmd_no,
			  u16 cmd_action,
			  u16 wait_option, u32 cmd_oid, void *pdata_buf)
{
	int ret = 0;
	struct cmd_ctrl_node *cmdnode;
	struct cmd_ds_command *cmdptr;
	unsigned long flags;

	lbs_deb_enter(LBS_DEB_HOST);

	if (!priv) {
		lbs_deb_host("PREP_CMD: priv is NULL\n");
		ret = -1;
		goto done;
	}

	if (priv->surpriseremoved) {
		lbs_deb_host("PREP_CMD: card removed\n");
		ret = -1;
		goto done;
	}

	cmdnode = lbs_get_cmd_ctrl_node(priv);

	if (cmdnode == NULL) {
		lbs_deb_host("PREP_CMD: cmdnode is NULL\n");

		
		wake_up_interruptible(&priv->waitq);
		ret = -1;
		goto done;
	}

	cmdnode->callback = NULL;
	cmdnode->callback_arg = (unsigned long)pdata_buf;

	cmdptr = (struct cmd_ds_command *)cmdnode->cmdbuf;

	lbs_deb_host("PREP_CMD: command 0x%04x\n", cmd_no);

	
	priv->seqnum++;
	cmdptr->seqnum = cpu_to_le16(priv->seqnum);

	cmdptr->command = cpu_to_le16(cmd_no);
	cmdptr->result = 0;

	switch (cmd_no) {
	case CMD_802_11_PS_MODE:
		ret = lbs_cmd_802_11_ps_mode(cmdptr, cmd_action);
		break;

	case CMD_MAC_REG_ACCESS:
	case CMD_BBP_REG_ACCESS:
	case CMD_RF_REG_ACCESS:
		ret = lbs_cmd_reg_access(cmdptr, cmd_action, pdata_buf);
		break;

	case CMD_802_11_MONITOR_MODE:
		ret = lbs_cmd_802_11_monitor_mode(cmdptr,
				          cmd_action, pdata_buf);
		break;

	case CMD_802_11_RSSI:
		ret = lbs_cmd_802_11_rssi(priv, cmdptr);
		break;

	case CMD_802_11_SET_AFC:
	case CMD_802_11_GET_AFC:

		cmdptr->command = cpu_to_le16(cmd_no);
		cmdptr->size = cpu_to_le16(sizeof(struct cmd_ds_802_11_afc) +
					   S_DS_GEN);

		memmove(&cmdptr->params.afc,
			pdata_buf, sizeof(struct cmd_ds_802_11_afc));

		ret = 0;
		goto done;

	case CMD_802_11D_DOMAIN_INFO:
		ret = lbs_cmd_802_11d_domain_info(priv, cmdptr,
						   cmd_no, cmd_action);
		break;

	case CMD_802_11_TPC_CFG:
		cmdptr->command = cpu_to_le16(CMD_802_11_TPC_CFG);
		cmdptr->size =
		    cpu_to_le16(sizeof(struct cmd_ds_802_11_tpc_cfg) +
				     S_DS_GEN);

		memmove(&cmdptr->params.tpccfg,
			pdata_buf, sizeof(struct cmd_ds_802_11_tpc_cfg));

		ret = 0;
		break;
	case CMD_802_11_LED_GPIO_CTRL:
		{
			struct mrvl_ie_ledgpio *gpio =
			    (struct mrvl_ie_ledgpio*)
			    cmdptr->params.ledgpio.data;

			memmove(&cmdptr->params.ledgpio,
				pdata_buf,
				sizeof(struct cmd_ds_802_11_led_ctrl));

			cmdptr->command =
			    cpu_to_le16(CMD_802_11_LED_GPIO_CTRL);

#define ACTION_NUMLED_TLVTYPE_LEN_FIELDS_LEN 8
			cmdptr->size =
			    cpu_to_le16(le16_to_cpu(gpio->header.len)
				+ S_DS_GEN
				+ ACTION_NUMLED_TLVTYPE_LEN_FIELDS_LEN);
			gpio->header.len = gpio->header.len;

			ret = 0;
			break;
		}

	case CMD_BT_ACCESS:
		ret = lbs_cmd_bt_access(cmdptr, cmd_action, pdata_buf);
		break;

	case CMD_FWT_ACCESS:
		ret = lbs_cmd_fwt_access(cmdptr, cmd_action, pdata_buf);
		break;

	case CMD_GET_TSF:
		cmdptr->command = cpu_to_le16(CMD_GET_TSF);
		cmdptr->size = cpu_to_le16(sizeof(struct cmd_ds_get_tsf) +
					   S_DS_GEN);
		ret = 0;
		break;
	case CMD_802_11_BEACON_CTRL:
		ret = lbs_cmd_bcn_ctrl(priv, cmdptr, cmd_action);
		break;
	default:
		lbs_pr_err("PREP_CMD: unknown command 0x%04x\n", cmd_no);
		ret = -1;
		break;
	}

	
	if (ret != 0) {
		lbs_deb_host("PREP_CMD: command preparation failed\n");
		lbs_cleanup_and_insert_cmd(priv, cmdnode);
		ret = -1;
		goto done;
	}

	cmdnode->cmdwaitqwoken = 0;

	lbs_queue_cmd(priv, cmdnode);
	wake_up_interruptible(&priv->waitq);

	if (wait_option & CMD_OPTION_WAITFORRSP) {
		lbs_deb_host("PREP_CMD: wait for response\n");
		might_sleep();
		wait_event_interruptible(cmdnode->cmdwait_q,
					 cmdnode->cmdwaitqwoken);
	}

	spin_lock_irqsave(&priv->driver_lock, flags);
	if (priv->cur_cmd_retcode) {
		lbs_deb_host("PREP_CMD: command failed with return code %d\n",
		       priv->cur_cmd_retcode);
		priv->cur_cmd_retcode = 0;
		ret = -1;
	}
	spin_unlock_irqrestore(&priv->driver_lock, flags);

done:
	lbs_deb_leave_args(LBS_DEB_HOST, "ret %d", ret);
	return ret;
}


int lbs_allocate_cmd_buffer(struct lbs_private *priv)
{
	int ret = 0;
	u32 bufsize;
	u32 i;
	struct cmd_ctrl_node *cmdarray;

	lbs_deb_enter(LBS_DEB_HOST);

	
	bufsize = sizeof(struct cmd_ctrl_node) * LBS_NUM_CMD_BUFFERS;
	if (!(cmdarray = kzalloc(bufsize, GFP_KERNEL))) {
		lbs_deb_host("ALLOC_CMD_BUF: tempcmd_array is NULL\n");
		ret = -1;
		goto done;
	}
	priv->cmd_array = cmdarray;

	
	for (i = 0; i < LBS_NUM_CMD_BUFFERS; i++) {
		cmdarray[i].cmdbuf = kzalloc(LBS_CMD_BUFFER_SIZE, GFP_KERNEL);
		if (!cmdarray[i].cmdbuf) {
			lbs_deb_host("ALLOC_CMD_BUF: ptempvirtualaddr is NULL\n");
			ret = -1;
			goto done;
		}
	}

	for (i = 0; i < LBS_NUM_CMD_BUFFERS; i++) {
		init_waitqueue_head(&cmdarray[i].cmdwait_q);
		lbs_cleanup_and_insert_cmd(priv, &cmdarray[i]);
	}
	ret = 0;

done:
	lbs_deb_leave_args(LBS_DEB_HOST, "ret %d", ret);
	return ret;
}


int lbs_free_cmd_buffer(struct lbs_private *priv)
{
	struct cmd_ctrl_node *cmdarray;
	unsigned int i;

	lbs_deb_enter(LBS_DEB_HOST);

	
	if (priv->cmd_array == NULL) {
		lbs_deb_host("FREE_CMD_BUF: cmd_array is NULL\n");
		goto done;
	}

	cmdarray = priv->cmd_array;

	
	for (i = 0; i < LBS_NUM_CMD_BUFFERS; i++) {
		if (cmdarray[i].cmdbuf) {
			kfree(cmdarray[i].cmdbuf);
			cmdarray[i].cmdbuf = NULL;
		}
	}

	
	if (priv->cmd_array) {
		kfree(priv->cmd_array);
		priv->cmd_array = NULL;
	}

done:
	lbs_deb_leave(LBS_DEB_HOST);
	return 0;
}


static struct cmd_ctrl_node *lbs_get_cmd_ctrl_node(struct lbs_private *priv)
{
	struct cmd_ctrl_node *tempnode;
	unsigned long flags;

	lbs_deb_enter(LBS_DEB_HOST);

	if (!priv)
		return NULL;

	spin_lock_irqsave(&priv->driver_lock, flags);

	if (!list_empty(&priv->cmdfreeq)) {
		tempnode = list_first_entry(&priv->cmdfreeq,
					    struct cmd_ctrl_node, list);
		list_del(&tempnode->list);
	} else {
		lbs_deb_host("GET_CMD_NODE: cmd_ctrl_node is not available\n");
		tempnode = NULL;
	}

	spin_unlock_irqrestore(&priv->driver_lock, flags);

	lbs_deb_leave(LBS_DEB_HOST);
	return tempnode;
}


int lbs_execute_next_command(struct lbs_private *priv)
{
	struct cmd_ctrl_node *cmdnode = NULL;
	struct cmd_header *cmd;
	unsigned long flags;
	int ret = 0;

	
	lbs_deb_enter(LBS_DEB_THREAD);

	spin_lock_irqsave(&priv->driver_lock, flags);

	if (priv->cur_cmd) {
		lbs_pr_alert( "EXEC_NEXT_CMD: already processing command!\n");
		spin_unlock_irqrestore(&priv->driver_lock, flags);
		ret = -1;
		goto done;
	}

	if (!list_empty(&priv->cmdpendingq)) {
		cmdnode = list_first_entry(&priv->cmdpendingq,
					   struct cmd_ctrl_node, list);
	}

	spin_unlock_irqrestore(&priv->driver_lock, flags);

	if (cmdnode) {
		cmd = cmdnode->cmdbuf;

		if (is_command_allowed_in_ps(le16_to_cpu(cmd->command))) {
			if ((priv->psstate == PS_STATE_SLEEP) ||
			    (priv->psstate == PS_STATE_PRE_SLEEP)) {
				lbs_deb_host(
				       "EXEC_NEXT_CMD: cannot send cmd 0x%04x in psstate %d\n",
				       le16_to_cpu(cmd->command),
				       priv->psstate);
				ret = -1;
				goto done;
			}
			lbs_deb_host("EXEC_NEXT_CMD: OK to send command "
				     "0x%04x in psstate %d\n",
				     le16_to_cpu(cmd->command), priv->psstate);
		} else if (priv->psstate != PS_STATE_FULL_POWER) {
			
			if (cmd->command != cpu_to_le16(CMD_802_11_PS_MODE)) {
				
				if ((priv->psstate == PS_STATE_SLEEP)
				    || (priv->psstate == PS_STATE_PRE_SLEEP)
				    ) {
					
					priv->needtowakeup = 1;
				} else
					lbs_ps_wakeup(priv, 0);

				ret = 0;
				goto done;
			} else {
				
				struct cmd_ds_802_11_ps_mode *psm = (void *)&cmd[1];

				lbs_deb_host(
				       "EXEC_NEXT_CMD: PS cmd, action 0x%02x\n",
				       psm->action);
				if (psm->action !=
				    cpu_to_le16(CMD_SUBCMD_EXIT_PS)) {
					lbs_deb_host(
					       "EXEC_NEXT_CMD: ignore ENTER_PS cmd\n");
					list_del(&cmdnode->list);
					spin_lock_irqsave(&priv->driver_lock, flags);
					lbs_complete_command(priv, cmdnode, 0);
					spin_unlock_irqrestore(&priv->driver_lock, flags);

					ret = 0;
					goto done;
				}

				if ((priv->psstate == PS_STATE_SLEEP) ||
				    (priv->psstate == PS_STATE_PRE_SLEEP)) {
					lbs_deb_host(
					       "EXEC_NEXT_CMD: ignore EXIT_PS cmd in sleep\n");
					list_del(&cmdnode->list);
					spin_lock_irqsave(&priv->driver_lock, flags);
					lbs_complete_command(priv, cmdnode, 0);
					spin_unlock_irqrestore(&priv->driver_lock, flags);
					priv->needtowakeup = 1;

					ret = 0;
					goto done;
				}

				lbs_deb_host(
				       "EXEC_NEXT_CMD: sending EXIT_PS\n");
			}
		}
		list_del(&cmdnode->list);
		lbs_deb_host("EXEC_NEXT_CMD: sending command 0x%04x\n",
			    le16_to_cpu(cmd->command));
		lbs_submit_command(priv, cmdnode);
	} else {
		
		if ((priv->psmode != LBS802_11POWERMODECAM) &&
		    (priv->psstate == PS_STATE_FULL_POWER) &&
		    ((priv->connect_status == LBS_CONNECTED) ||
		    (priv->mesh_connect_status == LBS_CONNECTED))) {
			if (priv->secinfo.WPAenabled ||
			    priv->secinfo.WPA2enabled) {
				
				if (priv->wpa_mcast_key.len ||
				    priv->wpa_unicast_key.len) {
					lbs_deb_host(
					       "EXEC_NEXT_CMD: WPA enabled and GTK_SET"
					       " go back to PS_SLEEP");
					lbs_ps_sleep(priv, 0);
				}
			} else {
				lbs_deb_host(
				       "EXEC_NEXT_CMD: cmdpendingq empty, "
				       "go back to PS_SLEEP");
				lbs_ps_sleep(priv, 0);
			}
		}
	}

	ret = 0;
done:
	lbs_deb_leave(LBS_DEB_THREAD);
	return ret;
}

void lbs_send_iwevcustom_event(struct lbs_private *priv, s8 *str)
{
	union iwreq_data iwrq;
	u8 buf[50];

	lbs_deb_enter(LBS_DEB_WEXT);

	memset(&iwrq, 0, sizeof(union iwreq_data));
	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf) - 1, "%s", str);

	iwrq.data.length = strlen(buf) + 1 + IW_EV_LCP_LEN;

	
	lbs_deb_wext("event indication string %s\n", (char *)buf);
	lbs_deb_wext("event indication length %d\n", iwrq.data.length);
	lbs_deb_wext("sending wireless event IWEVCUSTOM for %s\n", str);

	wireless_send_event(priv->dev, IWEVCUSTOM, &iwrq, buf);

	lbs_deb_leave(LBS_DEB_WEXT);
}

static void lbs_send_confirmsleep(struct lbs_private *priv)
{
	unsigned long flags;
	int ret;

	lbs_deb_enter(LBS_DEB_HOST);
	lbs_deb_hex(LBS_DEB_HOST, "sleep confirm", (u8 *) &confirm_sleep,
		sizeof(confirm_sleep));

	ret = priv->hw_host_to_card(priv, MVMS_CMD, (u8 *) &confirm_sleep,
		sizeof(confirm_sleep));
	if (ret) {
		lbs_pr_alert("confirm_sleep failed\n");
		goto out;
	}

	spin_lock_irqsave(&priv->driver_lock, flags);

	
	priv->dnld_sent = DNLD_RES_RECEIVED;

	
	if (!__kfifo_len(priv->event_fifo) && !priv->resp_len[priv->resp_idx])
		priv->psstate = PS_STATE_SLEEP;

	spin_unlock_irqrestore(&priv->driver_lock, flags);

out:
	lbs_deb_leave(LBS_DEB_HOST);
}

void lbs_ps_sleep(struct lbs_private *priv, int wait_option)
{
	lbs_deb_enter(LBS_DEB_HOST);

	

	lbs_prepare_and_send_command(priv, CMD_802_11_PS_MODE,
			      CMD_SUBCMD_ENTER_PS, wait_option, 0, NULL);

	lbs_deb_leave(LBS_DEB_HOST);
}


void lbs_ps_wakeup(struct lbs_private *priv, int wait_option)
{
	__le32 Localpsmode;

	lbs_deb_enter(LBS_DEB_HOST);

	Localpsmode = cpu_to_le32(LBS802_11POWERMODECAM);

	lbs_prepare_and_send_command(priv, CMD_802_11_PS_MODE,
			      CMD_SUBCMD_EXIT_PS,
			      wait_option, 0, &Localpsmode);

	lbs_deb_leave(LBS_DEB_HOST);
}


void lbs_ps_confirm_sleep(struct lbs_private *priv)
{
	unsigned long flags =0;
	int allowed = 1;

	lbs_deb_enter(LBS_DEB_HOST);

	spin_lock_irqsave(&priv->driver_lock, flags);
	if (priv->dnld_sent) {
		allowed = 0;
		lbs_deb_host("dnld_sent was set\n");
	}

	
	if (priv->cur_cmd) {
		allowed = 0;
		lbs_deb_host("cur_cmd was set\n");
	}

	
	if (__kfifo_len(priv->event_fifo) || priv->resp_len[priv->resp_idx]) {
		allowed = 0;
		lbs_deb_host("pending events or command responses\n");
	}
	spin_unlock_irqrestore(&priv->driver_lock, flags);

	if (allowed) {
		lbs_deb_host("sending lbs_ps_confirm_sleep\n");
		lbs_send_confirmsleep(priv);
	} else {
		lbs_deb_host("sleep confirm has been delayed\n");
	}

	lbs_deb_leave(LBS_DEB_HOST);
}



int lbs_set_tpc_cfg(struct lbs_private *priv, int enable, int8_t p0, int8_t p1,
		int8_t p2, int usesnr)
{
	struct cmd_ds_802_11_tpc_cfg cmd;
	int ret;

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_SET);
	cmd.enable = !!enable;
	cmd.usesnr = !!usesnr;
	cmd.P0 = p0;
	cmd.P1 = p1;
	cmd.P2 = p2;

	ret = lbs_cmd_with_response(priv, CMD_802_11_TPC_CFG, &cmd);

	return ret;
}



int lbs_set_power_adapt_cfg(struct lbs_private *priv, int enable, int8_t p0,
		int8_t p1, int8_t p2)
{
	struct cmd_ds_802_11_pa_cfg cmd;
	int ret;

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_SET);
	cmd.enable = !!enable;
	cmd.P0 = p0;
	cmd.P1 = p1;
	cmd.P2 = p2;

	ret = lbs_cmd_with_response(priv, CMD_802_11_PA_CFG , &cmd);

	return ret;
}


static struct cmd_ctrl_node *__lbs_cmd_async(struct lbs_private *priv,
	uint16_t command, struct cmd_header *in_cmd, int in_cmd_size,
	int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *),
	unsigned long callback_arg)
{
	struct cmd_ctrl_node *cmdnode;

	lbs_deb_enter(LBS_DEB_HOST);

	if (priv->surpriseremoved) {
		lbs_deb_host("PREP_CMD: card removed\n");
		cmdnode = ERR_PTR(-ENOENT);
		goto done;
	}

	cmdnode = lbs_get_cmd_ctrl_node(priv);
	if (cmdnode == NULL) {
		lbs_deb_host("PREP_CMD: cmdnode is NULL\n");

		
		wake_up_interruptible(&priv->waitq);
		cmdnode = ERR_PTR(-ENOBUFS);
		goto done;
	}

	cmdnode->callback = callback;
	cmdnode->callback_arg = callback_arg;

	
	memcpy(cmdnode->cmdbuf, in_cmd, in_cmd_size);

	
	priv->seqnum++;
	cmdnode->cmdbuf->command = cpu_to_le16(command);
	cmdnode->cmdbuf->size    = cpu_to_le16(in_cmd_size);
	cmdnode->cmdbuf->seqnum  = cpu_to_le16(priv->seqnum);
	cmdnode->cmdbuf->result  = 0;

	lbs_deb_host("PREP_CMD: command 0x%04x\n", command);

	cmdnode->cmdwaitqwoken = 0;
	lbs_queue_cmd(priv, cmdnode);
	wake_up_interruptible(&priv->waitq);

 done:
	lbs_deb_leave_args(LBS_DEB_HOST, "ret %p", cmdnode);
	return cmdnode;
}

void lbs_cmd_async(struct lbs_private *priv, uint16_t command,
	struct cmd_header *in_cmd, int in_cmd_size)
{
	lbs_deb_enter(LBS_DEB_CMD);
	__lbs_cmd_async(priv, command, in_cmd, in_cmd_size,
		lbs_cmd_async_callback, 0);
	lbs_deb_leave(LBS_DEB_CMD);
}

int __lbs_cmd(struct lbs_private *priv, uint16_t command,
	      struct cmd_header *in_cmd, int in_cmd_size,
	      int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *),
	      unsigned long callback_arg)
{
	struct cmd_ctrl_node *cmdnode;
	unsigned long flags;
	int ret = 0;

	lbs_deb_enter(LBS_DEB_HOST);

	cmdnode = __lbs_cmd_async(priv, command, in_cmd, in_cmd_size,
				  callback, callback_arg);
	if (IS_ERR(cmdnode)) {
		ret = PTR_ERR(cmdnode);
		goto done;
	}

	might_sleep();
	wait_event_interruptible(cmdnode->cmdwait_q, cmdnode->cmdwaitqwoken);

	spin_lock_irqsave(&priv->driver_lock, flags);
	ret = cmdnode->result;
	if (ret)
		lbs_pr_info("PREP_CMD: command 0x%04x failed: %d\n",
			    command, ret);

	__lbs_cleanup_and_insert_cmd(priv, cmdnode);
	spin_unlock_irqrestore(&priv->driver_lock, flags);

done:
	lbs_deb_leave_args(LBS_DEB_HOST, "ret %d", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(__lbs_cmd);


