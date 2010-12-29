


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <net/mac80211.h>

#include "iwl-eeprom.h"
#include "iwl-dev.h"
#include "iwl-core.h"
#include "iwl-io.h"
#include "iwl-commands.h"
#include "iwl-debug.h"
#include "iwl-power.h"




bool no_sleep_autoadjust = true;
module_param(no_sleep_autoadjust, bool, S_IRUGO);
MODULE_PARM_DESC(no_sleep_autoadjust,
		 "don't automatically adjust sleep level "
		 "according to maximum network latency");



struct iwl_power_vec_entry {
	struct iwl_powertable_cmd cmd;
	u8 no_dtim;
};

#define IWL_DTIM_RANGE_0_MAX	2
#define IWL_DTIM_RANGE_1_MAX	10

#define NOSLP cpu_to_le16(0), 0, 0
#define SLP IWL_POWER_DRIVER_ALLOW_SLEEP_MSK, 0, 0
#define TU_TO_USEC 1024
#define SLP_TOUT(T) cpu_to_le32((T) * TU_TO_USEC)
#define SLP_VEC(X0, X1, X2, X3, X4) {cpu_to_le32(X0), \
				     cpu_to_le32(X1), \
				     cpu_to_le32(X2), \
				     cpu_to_le32(X3), \
				     cpu_to_le32(X4)}


static const struct iwl_power_vec_entry range_0[IWL_POWER_NUM] = {
	{{SLP, SLP_TOUT(200), SLP_TOUT(500), SLP_VEC(1, 2, 2, 2, 0xFF)}, 0},
	{{SLP, SLP_TOUT(200), SLP_TOUT(300), SLP_VEC(1, 2, 2, 2, 0xFF)}, 0},
	{{SLP, SLP_TOUT(50), SLP_TOUT(100), SLP_VEC(2, 2, 2, 2, 0xFF)}, 0},
	{{SLP, SLP_TOUT(50), SLP_TOUT(25), SLP_VEC(2, 2, 4, 4, 0xFF)}, 1},
	{{SLP, SLP_TOUT(25), SLP_TOUT(25), SLP_VEC(2, 2, 4, 6, 0xFF)}, 2}
};



static const struct iwl_power_vec_entry range_1[IWL_POWER_NUM] = {
	{{SLP, SLP_TOUT(200), SLP_TOUT(500), SLP_VEC(1, 2, 3, 4, 4)}, 0},
	{{SLP, SLP_TOUT(200), SLP_TOUT(300), SLP_VEC(1, 2, 3, 4, 7)}, 0},
	{{SLP, SLP_TOUT(50), SLP_TOUT(100), SLP_VEC(2, 4, 6, 7, 9)}, 0},
	{{SLP, SLP_TOUT(50), SLP_TOUT(25), SLP_VEC(2, 4, 6, 9, 10)}, 1},
	{{SLP, SLP_TOUT(25), SLP_TOUT(25), SLP_VEC(2, 4, 7, 10, 10)}, 2}
};


static const struct iwl_power_vec_entry range_2[IWL_POWER_NUM] = {
	{{SLP, SLP_TOUT(200), SLP_TOUT(500), SLP_VEC(1, 2, 3, 4, 0xFF)}, 0},
	{{SLP, SLP_TOUT(200), SLP_TOUT(300), SLP_VEC(2, 4, 6, 7, 0xFF)}, 0},
	{{SLP, SLP_TOUT(50), SLP_TOUT(100), SLP_VEC(2, 7, 9, 9, 0xFF)}, 0},
	{{SLP, SLP_TOUT(50), SLP_TOUT(25), SLP_VEC(2, 7, 9, 9, 0xFF)}, 0},
	{{SLP, SLP_TOUT(25), SLP_TOUT(25), SLP_VEC(4, 7, 10, 10, 0xFF)}, 0}
};

static void iwl_static_sleep_cmd(struct iwl_priv *priv,
				 struct iwl_powertable_cmd *cmd,
				 enum iwl_power_level lvl, int period)
{
	const struct iwl_power_vec_entry *table;
	int max_sleep, i;
	bool skip;

	table = range_2;
	if (period < IWL_DTIM_RANGE_1_MAX)
		table = range_1;
	if (period < IWL_DTIM_RANGE_0_MAX)
		table = range_0;

	BUG_ON(lvl < 0 || lvl >= IWL_POWER_NUM);

	*cmd = table[lvl].cmd;

	if (period == 0) {
		skip = false;
		period = 1;
	} else {
		skip = !!table[lvl].no_dtim;
	}

	if (skip) {
		__le32 slp_itrvl = cmd->sleep_interval[IWL_POWER_VEC_SIZE - 1];
		max_sleep = le32_to_cpu(slp_itrvl);
		if (max_sleep == 0xFF)
			max_sleep = period * (skip + 1);
		else if (max_sleep > period)
			max_sleep = (le32_to_cpu(slp_itrvl) / period) * period;
		cmd->flags |= IWL_POWER_SLEEP_OVER_DTIM_MSK;
	} else {
		max_sleep = period;
		cmd->flags &= ~IWL_POWER_SLEEP_OVER_DTIM_MSK;
	}

	for (i = 0; i < IWL_POWER_VEC_SIZE; i++)
		if (le32_to_cpu(cmd->sleep_interval[i]) > max_sleep)
			cmd->sleep_interval[i] = cpu_to_le32(max_sleep);

	if (priv->power_data.pci_pm)
		cmd->flags |= IWL_POWER_PCI_PM_MSK;
	else
		cmd->flags &= ~IWL_POWER_PCI_PM_MSK;

	IWL_DEBUG_POWER(priv, "Sleep command for index %d\n", lvl + 1);
}


static const struct iwl_tt_trans tt_range_0[IWL_TI_STATE_MAX - 1] = {
	{IWL_TI_0, IWL_ABSOLUTE_ZERO, 104},
	{IWL_TI_1, 105, CT_KILL_THRESHOLD},
	{IWL_TI_CT_KILL, CT_KILL_THRESHOLD + 1, IWL_ABSOLUTE_MAX}
};
static const struct iwl_tt_trans tt_range_1[IWL_TI_STATE_MAX - 1] = {
	{IWL_TI_0, IWL_ABSOLUTE_ZERO, 95},
	{IWL_TI_2, 110, CT_KILL_THRESHOLD},
	{IWL_TI_CT_KILL, CT_KILL_THRESHOLD + 1, IWL_ABSOLUTE_MAX}
};
static const struct iwl_tt_trans tt_range_2[IWL_TI_STATE_MAX - 1] = {
	{IWL_TI_1, IWL_ABSOLUTE_ZERO, 100},
	{IWL_TI_CT_KILL, CT_KILL_THRESHOLD + 1, IWL_ABSOLUTE_MAX},
	{IWL_TI_CT_KILL, CT_KILL_THRESHOLD + 1, IWL_ABSOLUTE_MAX}
};
static const struct iwl_tt_trans tt_range_3[IWL_TI_STATE_MAX - 1] = {
	{IWL_TI_0, IWL_ABSOLUTE_ZERO, CT_KILL_EXIT_THRESHOLD},
	{IWL_TI_CT_KILL, CT_KILL_EXIT_THRESHOLD + 1, IWL_ABSOLUTE_MAX},
	{IWL_TI_CT_KILL, CT_KILL_EXIT_THRESHOLD + 1, IWL_ABSOLUTE_MAX}
};


static const struct iwl_tt_restriction restriction_range[IWL_TI_STATE_MAX] = {
	{IWL_ANT_OK_MULTI, IWL_ANT_OK_MULTI, true },
	{IWL_ANT_OK_SINGLE, IWL_ANT_OK_MULTI, true },
	{IWL_ANT_OK_SINGLE, IWL_ANT_OK_SINGLE, false },
	{IWL_ANT_OK_NONE, IWL_ANT_OK_NONE, false }
};


static void iwl_power_sleep_cam_cmd(struct iwl_priv *priv,
				    struct iwl_powertable_cmd *cmd)
{
	memset(cmd, 0, sizeof(*cmd));

	if (priv->power_data.pci_pm)
		cmd->flags |= IWL_POWER_PCI_PM_MSK;

	IWL_DEBUG_POWER(priv, "Sleep command for CAM\n");
}

static void iwl_power_fill_sleep_cmd(struct iwl_priv *priv,
				     struct iwl_powertable_cmd *cmd,
				     int dynps_ms, int wakeup_period)
{
	
	static const u8 slp_succ_r0[IWL_POWER_VEC_SIZE] = { 2, 2, 2, 2, 2 };
	static const u8 slp_succ_r1[IWL_POWER_VEC_SIZE] = { 2, 4, 6, 7, 9 };
	static const u8 slp_succ_r2[IWL_POWER_VEC_SIZE] = { 2, 7, 9, 9, 0xFF };
	const u8 *slp_succ = slp_succ_r0;
	int i;

	if (wakeup_period > IWL_DTIM_RANGE_0_MAX)
		slp_succ = slp_succ_r1;
	if (wakeup_period > IWL_DTIM_RANGE_1_MAX)
		slp_succ = slp_succ_r2;

	memset(cmd, 0, sizeof(*cmd));

	cmd->flags = IWL_POWER_DRIVER_ALLOW_SLEEP_MSK |
		     IWL_POWER_FAST_PD; 

	if (priv->power_data.pci_pm)
		cmd->flags |= IWL_POWER_PCI_PM_MSK;

	cmd->rx_data_timeout = cpu_to_le32(1000 * dynps_ms);
	cmd->tx_data_timeout = cpu_to_le32(1000 * dynps_ms);

	for (i = 0; i < IWL_POWER_VEC_SIZE; i++)
		cmd->sleep_interval[i] =
			cpu_to_le32(min_t(int, slp_succ[i], wakeup_period));

	IWL_DEBUG_POWER(priv, "Automatic sleep command\n");
}

static int iwl_set_power(struct iwl_priv *priv, struct iwl_powertable_cmd *cmd)
{
	IWL_DEBUG_POWER(priv, "Sending power/sleep command\n");
	IWL_DEBUG_POWER(priv, "Flags value = 0x%08X\n", cmd->flags);
	IWL_DEBUG_POWER(priv, "Tx timeout = %u\n", le32_to_cpu(cmd->tx_data_timeout));
	IWL_DEBUG_POWER(priv, "Rx timeout = %u\n", le32_to_cpu(cmd->rx_data_timeout));
	IWL_DEBUG_POWER(priv, "Sleep interval vector = { %d , %d , %d , %d , %d }\n",
			le32_to_cpu(cmd->sleep_interval[0]),
			le32_to_cpu(cmd->sleep_interval[1]),
			le32_to_cpu(cmd->sleep_interval[2]),
			le32_to_cpu(cmd->sleep_interval[3]),
			le32_to_cpu(cmd->sleep_interval[4]));

	return iwl_send_cmd_pdu(priv, POWER_TABLE_CMD,
				sizeof(struct iwl_powertable_cmd), cmd);
}


int iwl_power_update_mode(struct iwl_priv *priv, bool force)
{
	int ret = 0;
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;
	bool enabled = (priv->iw_mode == NL80211_IFTYPE_STATION) &&
			(priv->hw->conf.flags & IEEE80211_CONF_PS);
	bool update_chains;
	struct iwl_powertable_cmd cmd;
	int dtimper;

	
	update_chains = priv->chain_noise_data.state == IWL_CHAIN_NOISE_DONE ||
			priv->chain_noise_data.state == IWL_CHAIN_NOISE_ALIVE;

	if (priv->vif)
		dtimper = priv->vif->bss_conf.dtim_period;
	else
		dtimper = 1;

	if (priv->cfg->broken_powersave)
		iwl_power_sleep_cam_cmd(priv, &cmd);
	else if (tt->state >= IWL_TI_1)
		iwl_static_sleep_cmd(priv, &cmd, tt->tt_power_mode, dtimper);
	else if (!enabled)
		iwl_power_sleep_cam_cmd(priv, &cmd);
	else if (priv->power_data.debug_sleep_level_override >= 0)
		iwl_static_sleep_cmd(priv, &cmd,
				     priv->power_data.debug_sleep_level_override,
				     dtimper);
	else if (no_sleep_autoadjust)
		iwl_static_sleep_cmd(priv, &cmd, IWL_POWER_INDEX_1, dtimper);
	else
		iwl_power_fill_sleep_cmd(priv, &cmd,
					 priv->hw->conf.dynamic_ps_timeout,
					 priv->hw->conf.max_sleep_period);

	if (iwl_is_ready_rf(priv) &&
	    (memcmp(&priv->power_data.sleep_cmd, &cmd, sizeof(cmd)) || force)) {
		if (cmd.flags & IWL_POWER_DRIVER_ALLOW_SLEEP_MSK)
			set_bit(STATUS_POWER_PMI, &priv->status);

		ret = iwl_set_power(priv, &cmd);
		if (!ret) {
			if (!(cmd.flags & IWL_POWER_DRIVER_ALLOW_SLEEP_MSK))
				clear_bit(STATUS_POWER_PMI, &priv->status);

			if (priv->cfg->ops->lib->update_chain_flags &&
			    update_chains)
				priv->cfg->ops->lib->update_chain_flags(priv);
			else if (priv->cfg->ops->lib->update_chain_flags)
				IWL_DEBUG_POWER(priv,
					"Cannot update the power, chain noise "
					"calibration running: %d\n",
					priv->chain_noise_data.state);
			memcpy(&priv->power_data.sleep_cmd, &cmd, sizeof(cmd));
		} else
			IWL_ERR(priv, "set power fail, ret = %d", ret);
	}

	return ret;
}
EXPORT_SYMBOL(iwl_power_update_mode);

bool iwl_ht_enabled(struct iwl_priv *priv)
{
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;
	struct iwl_tt_restriction *restriction;

	if (!priv->thermal_throttle.advanced_tt)
		return true;
	restriction = tt->restriction + tt->state;
	return restriction->is_ht;
}
EXPORT_SYMBOL(iwl_ht_enabled);

enum iwl_antenna_ok iwl_tx_ant_restriction(struct iwl_priv *priv)
{
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;
	struct iwl_tt_restriction *restriction;

	if (!priv->thermal_throttle.advanced_tt)
		return IWL_ANT_OK_MULTI;
	restriction = tt->restriction + tt->state;
	return restriction->tx_stream;
}
EXPORT_SYMBOL(iwl_tx_ant_restriction);

enum iwl_antenna_ok iwl_rx_ant_restriction(struct iwl_priv *priv)
{
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;
	struct iwl_tt_restriction *restriction;

	if (!priv->thermal_throttle.advanced_tt)
		return IWL_ANT_OK_MULTI;
	restriction = tt->restriction + tt->state;
	return restriction->rx_stream;
}

#define CT_KILL_EXIT_DURATION (5)	


static void iwl_tt_check_exit_ct_kill(unsigned long data)
{
	struct iwl_priv *priv = (struct iwl_priv *)data;
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;
	unsigned long flags;

	if (test_bit(STATUS_EXIT_PENDING, &priv->status))
		return;

	if (tt->state == IWL_TI_CT_KILL) {
		if (priv->thermal_throttle.ct_kill_toggle) {
			iwl_write32(priv, CSR_UCODE_DRV_GP1_CLR,
				    CSR_UCODE_DRV_GP1_REG_BIT_CT_KILL_EXIT);
			priv->thermal_throttle.ct_kill_toggle = false;
		} else {
			iwl_write32(priv, CSR_UCODE_DRV_GP1_SET,
				    CSR_UCODE_DRV_GP1_REG_BIT_CT_KILL_EXIT);
			priv->thermal_throttle.ct_kill_toggle = true;
		}
		iwl_read32(priv, CSR_UCODE_DRV_GP1);
		spin_lock_irqsave(&priv->reg_lock, flags);
		if (!iwl_grab_nic_access(priv))
			iwl_release_nic_access(priv);
		spin_unlock_irqrestore(&priv->reg_lock, flags);

		
		mod_timer(&priv->thermal_throttle.ct_kill_exit_tm, jiffies +
			  CT_KILL_EXIT_DURATION * HZ);
	}
}

static void iwl_perform_ct_kill_task(struct iwl_priv *priv,
			   bool stop)
{
	if (stop) {
		IWL_DEBUG_POWER(priv, "Stop all queues\n");
		if (priv->mac80211_registered)
			ieee80211_stop_queues(priv->hw);
		IWL_DEBUG_POWER(priv,
				"Schedule 5 seconds CT_KILL Timer\n");
		mod_timer(&priv->thermal_throttle.ct_kill_exit_tm, jiffies +
			  CT_KILL_EXIT_DURATION * HZ);
	} else {
		IWL_DEBUG_POWER(priv, "Wake all queues\n");
		if (priv->mac80211_registered)
			ieee80211_wake_queues(priv->hw);
	}
}

#define IWL_MINIMAL_POWER_THRESHOLD		(CT_KILL_THRESHOLD_LEGACY)
#define IWL_REDUCED_PERFORMANCE_THRESHOLD_2	(100)
#define IWL_REDUCED_PERFORMANCE_THRESHOLD_1	(90)


static void iwl_legacy_tt_handler(struct iwl_priv *priv, s32 temp)
{
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;
	enum iwl_tt_state old_state;

#ifdef CONFIG_IWLWIFI_DEBUG
	if ((tt->tt_previous_temp) &&
	    (temp > tt->tt_previous_temp) &&
	    ((temp - tt->tt_previous_temp) >
	    IWL_TT_INCREASE_MARGIN)) {
		IWL_DEBUG_POWER(priv,
			"Temperature increase %d degree Celsius\n",
			(temp - tt->tt_previous_temp));
	}
#endif
	old_state = tt->state;
	
	if (temp >= IWL_MINIMAL_POWER_THRESHOLD)
		tt->state = IWL_TI_CT_KILL;
	else if (temp >= IWL_REDUCED_PERFORMANCE_THRESHOLD_2)
		tt->state = IWL_TI_2;
	else if (temp >= IWL_REDUCED_PERFORMANCE_THRESHOLD_1)
		tt->state = IWL_TI_1;
	else
		tt->state = IWL_TI_0;

#ifdef CONFIG_IWLWIFI_DEBUG
	tt->tt_previous_temp = temp;
#endif
	if (tt->state != old_state) {
		switch (tt->state) {
		case IWL_TI_0:
			
			break;
		case IWL_TI_1:
			tt->tt_power_mode = IWL_POWER_INDEX_3;
			break;
		case IWL_TI_2:
			tt->tt_power_mode = IWL_POWER_INDEX_4;
			break;
		default:
			tt->tt_power_mode = IWL_POWER_INDEX_5;
			break;
		}
		mutex_lock(&priv->mutex);
		if (iwl_power_update_mode(priv, true)) {
			
			tt->state = old_state;
			IWL_ERR(priv, "Cannot update power mode, "
					"TT state not updated\n");
		} else {
			if (tt->state == IWL_TI_CT_KILL)
				iwl_perform_ct_kill_task(priv, true);
			else if (old_state == IWL_TI_CT_KILL &&
				 tt->state != IWL_TI_CT_KILL)
				iwl_perform_ct_kill_task(priv, false);
			IWL_DEBUG_POWER(priv, "Temperature state changed %u\n",
					tt->state);
			IWL_DEBUG_POWER(priv, "Power Index change to %u\n",
					tt->tt_power_mode);
		}
		mutex_unlock(&priv->mutex);
	}
}


static void iwl_advance_tt_handler(struct iwl_priv *priv, s32 temp)
{
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;
	int i;
	bool changed = false;
	enum iwl_tt_state old_state;
	struct iwl_tt_trans *transaction;

	old_state = tt->state;
	for (i = 0; i < IWL_TI_STATE_MAX - 1; i++) {
		
		transaction = tt->transaction +
			((old_state * (IWL_TI_STATE_MAX - 1)) + i);
		if (temp >= transaction->tt_low &&
		    temp <= transaction->tt_high) {
#ifdef CONFIG_IWLWIFI_DEBUG
			if ((tt->tt_previous_temp) &&
			    (temp > tt->tt_previous_temp) &&
			    ((temp - tt->tt_previous_temp) >
			    IWL_TT_INCREASE_MARGIN)) {
				IWL_DEBUG_POWER(priv,
					"Temperature increase %d "
					"degree Celsius\n",
					(temp - tt->tt_previous_temp));
			}
			tt->tt_previous_temp = temp;
#endif
			if (old_state !=
			    transaction->next_state) {
				changed = true;
				tt->state =
					transaction->next_state;
			}
			break;
		}
	}
	if (changed) {
		struct iwl_rxon_cmd *rxon = &priv->staging_rxon;

		if (tt->state >= IWL_TI_1) {
			
			tt->tt_power_mode = IWL_POWER_INDEX_5;
			if (!iwl_ht_enabled(priv))
				
				rxon->flags &= ~(RXON_FLG_CHANNEL_MODE_MSK |
					RXON_FLG_CTRL_CHANNEL_LOC_HI_MSK |
					RXON_FLG_HT40_PROT_MSK |
					RXON_FLG_HT_PROT_MSK);
			else {
				
				iwl_set_rxon_ht(priv, &priv->current_ht_config);
			}

		} else {
			

			
			iwl_set_rxon_ht(priv, &priv->current_ht_config);
		}
		mutex_lock(&priv->mutex);
		if (iwl_power_update_mode(priv, true)) {
			
			IWL_ERR(priv, "Cannot update power mode, "
					"TT state not updated\n");
			tt->state = old_state;
		} else {
			IWL_DEBUG_POWER(priv,
					"Thermal Throttling to new state: %u\n",
					tt->state);
			if (old_state != IWL_TI_CT_KILL &&
			    tt->state == IWL_TI_CT_KILL) {
				IWL_DEBUG_POWER(priv, "Enter IWL_TI_CT_KILL\n");
				iwl_perform_ct_kill_task(priv, true);

			} else if (old_state == IWL_TI_CT_KILL &&
				  tt->state != IWL_TI_CT_KILL) {
				IWL_DEBUG_POWER(priv, "Exit IWL_TI_CT_KILL\n");
				iwl_perform_ct_kill_task(priv, false);
			}
		}
		mutex_unlock(&priv->mutex);
	}
}


static void iwl_bg_ct_enter(struct work_struct *work)
{
	struct iwl_priv *priv = container_of(work, struct iwl_priv, ct_enter);
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;

	if (test_bit(STATUS_EXIT_PENDING, &priv->status))
		return;

	if (!iwl_is_ready(priv))
		return;

	if (tt->state != IWL_TI_CT_KILL) {
		IWL_ERR(priv, "Device reached critical temperature "
			      "- ucode going to sleep!\n");
		if (!priv->thermal_throttle.advanced_tt)
			iwl_legacy_tt_handler(priv,
					      IWL_MINIMAL_POWER_THRESHOLD);
		else
			iwl_advance_tt_handler(priv,
					       CT_KILL_THRESHOLD + 1);
	}
}


static void iwl_bg_ct_exit(struct work_struct *work)
{
	struct iwl_priv *priv = container_of(work, struct iwl_priv, ct_exit);
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;

	if (test_bit(STATUS_EXIT_PENDING, &priv->status))
		return;

	if (!iwl_is_ready(priv))
		return;

	
	del_timer_sync(&priv->thermal_throttle.ct_kill_exit_tm);

	if (tt->state == IWL_TI_CT_KILL) {
		IWL_ERR(priv,
			"Device temperature below critical"
			"- ucode awake!\n");
		if (!priv->thermal_throttle.advanced_tt)
			iwl_legacy_tt_handler(priv,
					IWL_REDUCED_PERFORMANCE_THRESHOLD_2);
		else
			iwl_advance_tt_handler(priv, CT_KILL_EXIT_THRESHOLD);
	}
}

void iwl_tt_enter_ct_kill(struct iwl_priv *priv)
{
	if (test_bit(STATUS_EXIT_PENDING, &priv->status))
		return;

	IWL_DEBUG_POWER(priv, "Queueing critical temperature enter.\n");
	queue_work(priv->workqueue, &priv->ct_enter);
}
EXPORT_SYMBOL(iwl_tt_enter_ct_kill);

void iwl_tt_exit_ct_kill(struct iwl_priv *priv)
{
	if (test_bit(STATUS_EXIT_PENDING, &priv->status))
		return;

	IWL_DEBUG_POWER(priv, "Queueing critical temperature exit.\n");
	queue_work(priv->workqueue, &priv->ct_exit);
}
EXPORT_SYMBOL(iwl_tt_exit_ct_kill);

static void iwl_bg_tt_work(struct work_struct *work)
{
	struct iwl_priv *priv = container_of(work, struct iwl_priv, tt_work);
	s32 temp = priv->temperature; 

	if (test_bit(STATUS_EXIT_PENDING, &priv->status))
		return;

	if ((priv->hw_rev & CSR_HW_REV_TYPE_MSK) == CSR_HW_REV_TYPE_4965)
		temp = KELVIN_TO_CELSIUS(priv->temperature);

	if (!priv->thermal_throttle.advanced_tt)
		iwl_legacy_tt_handler(priv, temp);
	else
		iwl_advance_tt_handler(priv, temp);
}

void iwl_tt_handler(struct iwl_priv *priv)
{
	if (test_bit(STATUS_EXIT_PENDING, &priv->status))
		return;

	IWL_DEBUG_POWER(priv, "Queueing thermal throttling work.\n");
	queue_work(priv->workqueue, &priv->tt_work);
}
EXPORT_SYMBOL(iwl_tt_handler);


void iwl_tt_initialize(struct iwl_priv *priv)
{
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;
	int size = sizeof(struct iwl_tt_trans) * (IWL_TI_STATE_MAX - 1);
	struct iwl_tt_trans *transaction;

	IWL_DEBUG_POWER(priv, "Initialize Thermal Throttling \n");

	memset(tt, 0, sizeof(struct iwl_tt_mgmt));

	tt->state = IWL_TI_0;
	init_timer(&priv->thermal_throttle.ct_kill_exit_tm);
	priv->thermal_throttle.ct_kill_exit_tm.data = (unsigned long)priv;
	priv->thermal_throttle.ct_kill_exit_tm.function = iwl_tt_check_exit_ct_kill;

	
	INIT_WORK(&priv->tt_work, iwl_bg_tt_work);
	INIT_WORK(&priv->ct_enter, iwl_bg_ct_enter);
	INIT_WORK(&priv->ct_exit, iwl_bg_ct_exit);

	switch (priv->hw_rev & CSR_HW_REV_TYPE_MSK) {
	case CSR_HW_REV_TYPE_6x00:
	case CSR_HW_REV_TYPE_6x50:
		IWL_DEBUG_POWER(priv, "Advanced Thermal Throttling\n");
		tt->restriction = kzalloc(sizeof(struct iwl_tt_restriction) *
					 IWL_TI_STATE_MAX, GFP_KERNEL);
		tt->transaction = kzalloc(sizeof(struct iwl_tt_trans) *
			IWL_TI_STATE_MAX * (IWL_TI_STATE_MAX - 1),
			GFP_KERNEL);
		if (!tt->restriction || !tt->transaction) {
			IWL_ERR(priv, "Fallback to Legacy Throttling\n");
			priv->thermal_throttle.advanced_tt = false;
			kfree(tt->restriction);
			tt->restriction = NULL;
			kfree(tt->transaction);
			tt->transaction = NULL;
		} else {
			transaction = tt->transaction +
				(IWL_TI_0 * (IWL_TI_STATE_MAX - 1));
			memcpy(transaction, &tt_range_0[0], size);
			transaction = tt->transaction +
				(IWL_TI_1 * (IWL_TI_STATE_MAX - 1));
			memcpy(transaction, &tt_range_1[0], size);
			transaction = tt->transaction +
				(IWL_TI_2 * (IWL_TI_STATE_MAX - 1));
			memcpy(transaction, &tt_range_2[0], size);
			transaction = tt->transaction +
				(IWL_TI_CT_KILL * (IWL_TI_STATE_MAX - 1));
			memcpy(transaction, &tt_range_3[0], size);
			size = sizeof(struct iwl_tt_restriction) *
				IWL_TI_STATE_MAX;
			memcpy(tt->restriction,
				&restriction_range[0], size);
			priv->thermal_throttle.advanced_tt = true;
		}
		break;
	default:
		IWL_DEBUG_POWER(priv, "Legacy Thermal Throttling\n");
		priv->thermal_throttle.advanced_tt = false;
		break;
	}
}
EXPORT_SYMBOL(iwl_tt_initialize);


void iwl_tt_exit(struct iwl_priv *priv)
{
	struct iwl_tt_mgmt *tt = &priv->thermal_throttle;

	
	del_timer_sync(&priv->thermal_throttle.ct_kill_exit_tm);
	cancel_work_sync(&priv->tt_work);
	cancel_work_sync(&priv->ct_enter);
	cancel_work_sync(&priv->ct_exit);

	if (priv->thermal_throttle.advanced_tt) {
		
		kfree(tt->restriction);
		tt->restriction = NULL;
		kfree(tt->transaction);
		tt->transaction = NULL;
	}
}
EXPORT_SYMBOL(iwl_tt_exit);


void iwl_power_initialize(struct iwl_priv *priv)
{
	u16 lctl = iwl_pcie_link_ctl(priv);

	priv->power_data.pci_pm = !(lctl & PCI_CFG_LINK_CTRL_VAL_L0S_EN);

	priv->power_data.debug_sleep_level_override = -1;

	memset(&priv->power_data.sleep_cmd, 0,
		sizeof(priv->power_data.sleep_cmd));
}
EXPORT_SYMBOL(iwl_power_initialize);
