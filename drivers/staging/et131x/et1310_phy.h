

#ifndef _ET1310_PHY_H_
#define _ET1310_PHY_H_

#include "et1310_address_map.h"

#define TRUEPHY_SUCCESS 0
#define TRUEPHY_FAILURE 1


#define MI_CONTROL_REG                      0
#define MI_STATUS_REG                       1
#define MI_PHY_IDENTIFIER_1_REG             2
#define MI_PHY_IDENTIFIER_2_REG             3
#define MI_AUTONEG_ADVERTISEMENT_REG        4
#define MI_AUTONEG_LINK_PARTNER_ABILITY_REG 5
#define MI_AUTONEG_EXPANSION_REG            6
#define MI_AUTONEG_NEXT_PAGE_TRANSMIT_REG   7
#define MI_LINK_PARTNER_NEXT_PAGE_REG       8
#define MI_1000BASET_CONTROL_REG            9
#define MI_1000BASET_STATUS_REG             10
#define MI_RESERVED11_REG                   11
#define MI_RESERVED12_REG                   12
#define MI_RESERVED13_REG                   13
#define MI_RESERVED14_REG                   14
#define MI_EXTENDED_STATUS_REG              15


#define VMI_RESERVED16_REG                  16
#define VMI_RESERVED17_REG                  17
#define VMI_RESERVED18_REG                  18
#define VMI_LOOPBACK_CONTROL_REG            19
#define VMI_RESERVED20_REG                  20
#define VMI_MI_CONTROL_REG                  21
#define VMI_PHY_CONFIGURATION_REG           22
#define VMI_PHY_CONTROL_REG                 23
#define VMI_INTERRUPT_MASK_REG              24
#define VMI_INTERRUPT_STATUS_REG            25
#define VMI_PHY_STATUS_REG                  26
#define VMI_LED_CONTROL_1_REG               27
#define VMI_LED_CONTROL_2_REG               28
#define VMI_RESERVED29_REG                  29
#define VMI_RESERVED30_REG                  30
#define VMI_RESERVED31_REG                  31


typedef struct _MI_REGS_t {
	u8 bmcr;	
	u8 bmsr;	
	u8 idr1;	
	u8 idr2;	
	u8 anar;	
	u8 anlpar;	
	u8 aner;	
	u8 annptr;	
	u8 lpnpr;	
	u8 gcr;		
	u8 gsr;		
	u8 mi_res1[4];	
	u8 esr;		
	u8 mi_res2[3];	
	u8 loop_ctl;	
	u8 mi_res3;	
	u8 mcr;		
	u8 pcr;		
	u8 phy_ctl;	
	u8 imr;		
	u8 isr;		
	u8 psr;		
	u8 lcr1;		
	u8 lcr2;		
	u8 mi_res4[3];	
} MI_REGS_t, *PMI_REGS_t;


typedef union _MI_BMCR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 reset:1;		
		u16 loopback:1;		
		u16 speed_sel:1;		
		u16 enable_autoneg:1;	
		u16 power_down:1;		
		u16 isolate:1;		
		u16 restart_autoneg:1;	
		u16 duplex_mode:1;		
		u16 col_test:1;		
		u16 speed_1000_sel:1;	
		u16 res1:6;		
#else
		u16 res1:6;		
		u16 speed_1000_sel:1;	
		u16 col_test:1;		
		u16 duplex_mode:1;		
		u16 restart_autoneg:1;	
		u16 isolate:1;		
		u16 power_down:1;		
		u16 enable_autoneg:1;	
		u16 speed_sel:1;		
		u16 loopback:1;		
		u16 reset:1;		
#endif
	} bits;
} MI_BMCR_t, *PMI_BMCR_t;


typedef union _MI_BMSR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 link_100T4:1;		
		u16 link_100fdx:1;		
		u16 link_100hdx:1;		
		u16 link_10fdx:1;		
		u16 link_10hdx:1;		
		u16 link_100T2fdx:1;	
		u16 link_100T2hdx:1;	
		u16 extend_status:1;	
		u16 res1:1;		
		u16 preamble_supress:1;	
		u16 auto_neg_complete:1;	
		u16 remote_fault:1;	
		u16 auto_neg_able:1;	
		u16 link_status:1;		
		u16 jabber_detect:1;	
		u16 ext_cap:1;		
#else
		u16 ext_cap:1;		
		u16 jabber_detect:1;	
		u16 link_status:1;		
		u16 auto_neg_able:1;	
		u16 remote_fault:1;	
		u16 auto_neg_complete:1;	
		u16 preamble_supress:1;	
		u16 res1:1;		
		u16 extend_status:1;	
		u16 link_100T2hdx:1;	
		u16 link_100T2fdx:1;	
		u16 link_10hdx:1;		
		u16 link_10fdx:1;		
		u16 link_100hdx:1;		
		u16 link_100fdx:1;		
		u16 link_100T4:1;		
#endif
	} bits;
} MI_BMSR_t, *PMI_BMSR_t;


typedef union _MI_IDR1_t {
	u16 value;
	struct {
		u16 ieee_address:16;	
	} bits;
} MI_IDR1_t, *PMI_IDR1_t;


typedef union _MI_IDR2_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 ieee_address:6;	
		u16 model_no:6;		
		u16 rev_no:4;		
#else
		u16 rev_no:4;		
		u16 model_no:6;		
		u16 ieee_address:6;	
#endif
	} bits;
} MI_IDR2_t, *PMI_IDR2_t;


typedef union _MI_ANAR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 np_indication:1;	
		u16 res2:1;		
		u16 remote_fault:1;	
		u16 res1:1;		
		u16 cap_asmpause:1;	
		u16 cap_pause:1;		
		u16 cap_100T4:1;		
		u16 cap_100fdx:1;		
		u16 cap_100hdx:1;		
		u16 cap_10fdx:1;		
		u16 cap_10hdx:1;		
		u16 selector:5;		
#else
		u16 selector:5;		
		u16 cap_10hdx:1;		
		u16 cap_10fdx:1;		
		u16 cap_100hdx:1;		
		u16 cap_100fdx:1;		
		u16 cap_100T4:1;		
		u16 cap_pause:1;		
		u16 cap_asmpause:1;	
		u16 res1:1;		
		u16 remote_fault:1;	
		u16 res2:1;		
		u16 np_indication:1;	
#endif
	} bits;
} MI_ANAR_t, *PMI_ANAR_t;


typedef struct _MI_ANLPAR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 np_indication:1;	
		u16 acknowledge:1;		
		u16 remote_fault:1;	
		u16 res1:1;		
		u16 cap_asmpause:1;	
		u16 cap_pause:1;		
		u16 cap_100T4:1;		
		u16 cap_100fdx:1;		
		u16 cap_100hdx:1;		
		u16 cap_10fdx:1;		
		u16 cap_10hdx:1;		
		u16 selector:5;		
#else
		u16 selector:5;		
		u16 cap_10hdx:1;		
		u16 cap_10fdx:1;		
		u16 cap_100hdx:1;		
		u16 cap_100fdx:1;		
		u16 cap_100T4:1;		
		u16 cap_pause:1;		
		u16 cap_asmpause:1;	
		u16 res1:1;		
		u16 remote_fault:1;	
		u16 acknowledge:1;		
		u16 np_indication:1;	
#endif
	} bits;
} MI_ANLPAR_t, *PMI_ANLPAR_t;


typedef union _MI_ANER_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 res:11;	
		u16 pdf:1;		
		u16 lp_np_able:1;	
		u16 np_able:1;	
		u16 page_rx:1;	
		u16 lp_an_able:1;	
#else
		u16 lp_an_able:1;	
		u16 page_rx:1;	
		u16 np_able:1;	
		u16 lp_np_able:1;	
		u16 pdf:1;		
		u16 res:11;	
#endif
	} bits;
} MI_ANER_t, *PMI_ANER_t;


typedef union _MI_ANNPTR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 np:1;		
		u16 res1:1;	
		u16 msg_page:1;	
		u16 ack2:1;	
		u16 toggle:1;	
		u16 msg:11;	
#else
		u16 msg:11;	
		u16 toggle:1;	
		u16 ack2:1;	
		u16 msg_page:1;	
		u16 res1:1;	
		u16 np:1;		
#endif
	} bits;
} MI_ANNPTR_t, *PMI_ANNPTR_t;


typedef union _MI_LPNPR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 np:1;		
		u16 ack:1;		
		u16 msg_page:1;	
		u16 ack2:1;	
		u16 toggle:1;	
		u16 msg:11;	
#else
		u16 msg:11;	
		u16 toggle:1;	
		u16 ack2:1;	
		u16 msg_page:1;	
		u16 ack:1;		
		u16 np:1;		
#endif
	} bits;
} MI_LPNPR_t, *PMI_LPNPR_t;


typedef union _MI_GCR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 test_mode:3;		
		u16 ms_config_en:1;	
		u16 ms_value:1;		
		u16 port_type:1;		
		u16 link_1000fdx:1;	
		u16 link_1000hdx:1;	
		u16 res:8;			
#else
		u16 res:8;			
		u16 link_1000hdx:1;	
		u16 link_1000fdx:1;	
		u16 port_type:1;		
		u16 ms_value:1;		
		u16 ms_config_en:1;	
		u16 test_mode:3;		
#endif
	} bits;
} MI_GCR_t, *PMI_GCR_t;


typedef union _MI_GSR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 ms_config_fault:1;	
		u16 ms_resolve:1;		
		u16 local_rx_status:1;	
		u16 remote_rx_status:1;	
		u16 link_1000fdx:1;	
		u16 link_1000hdx:1;	
		u16 res:2;			
		u16 idle_err_cnt:8;	
#else
		u16 idle_err_cnt:8;	
		u16 res:2;			
		u16 link_1000hdx:1;	
		u16 link_1000fdx:1;	
		u16 remote_rx_status:1;	
		u16 local_rx_status:1;	
		u16 ms_resolve:1;		
		u16 ms_config_fault:1;	
#endif
	} bits;
} MI_GSR_t, *PMI_GSR_t;


typedef union _MI_RES_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 res15:1;	
		u16 res14:1;	
		u16 res13:1;	
		u16 res12:1;	
		u16 res11:1;	
		u16 res10:1;	
		u16 res9:1;	
		u16 res8:1;	
		u16 res7:1;	
		u16 res6:1;	
		u16 res5:1;	
		u16 res4:1;	
		u16 res3:1;	
		u16 res2:1;	
		u16 res1:1;	
		u16 res0:1;	
#else
		u16 res0:1;	
		u16 res1:1;	
		u16 res2:1;	
		u16 res3:1;	
		u16 res4:1;	
		u16 res5:1;	
		u16 res6:1;	
		u16 res7:1;	
		u16 res8:1;	
		u16 res9:1;	
		u16 res10:1;	
		u16 res11:1;	
		u16 res12:1;	
		u16 res13:1;	
		u16 res14:1;	
		u16 res15:1;	
#endif
	} bits;
} MI_RES_t, *PMI_RES_t;


typedef union _MI_ESR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 link_1000Xfdx:1;	
		u16 link_1000Xhdx:1;	
		u16 link_1000fdx:1;	
		u16 link_1000hdx:1;	
		u16 res:12;		
#else
		u16 res:12;		
		u16 link_1000hdx:1;	
		u16 link_1000fdx:1;	
		u16 link_1000Xhdx:1;	
		u16 link_1000Xfdx:1;	
#endif
	} bits;
} MI_ESR_t, *PMI_ESR_t;




typedef union _MI_LCR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 mii_en:1;		
		u16 pcs_en:1;		
		u16 pmd_en:1;		
		u16 all_digital_en:1;	
		u16 replica_en:1;		
		u16 line_driver_en:1;	
		u16 res:10;		
#else
		u16 res:10;		
		u16 line_driver_en:1;	
		u16 replica_en:1;		
		u16 all_digital_en:1;	
		u16 pmd_en:1;		
		u16 pcs_en:1;		
		u16 mii_en:1;		
#endif
	} bits;
} MI_LCR_t, *PMI_LCR_t;




typedef union _MI_MICR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 res1:5;		
		u16 mi_error_count:7;	
		u16 res2:1;		
		u16 ignore_10g_fr:1;	
		u16 res3:1;		
		u16 preamble_supress_en:1;	
#else
		u16 preamble_supress_en:1;	
		u16 res3:1;		
		u16 ignore_10g_fr:1;	
		u16 res2:1;		
		u16 mi_error_count:7;	
		u16 res1:5;		
#endif
	} bits;
} MI_MICR_t, *PMI_MICR_t;


typedef union _MI_PHY_CONFIG_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 crs_tx_en:1;		
		u16 res1:1;		
		u16 tx_fifo_depth:2;	
		u16 speed_downshift:2;	
		u16 pbi_detect:1;		
		u16 tbi_rate:1;		
		u16 alternate_np:1;	
		u16 group_mdio_en:1;	
		u16 tx_clock_en:1;		
		u16 sys_clock_en:1;	
		u16 res2:1;		
		u16 mac_if_mode:3;		
#else
		u16 mac_if_mode:3;		
		u16 res2:1;		
		u16 sys_clock_en:1;	
		u16 tx_clock_en:1;		
		u16 group_mdio_en:1;	
		u16 alternate_np:1;	
		u16 tbi_rate:1;		
		u16 pbi_detect:1;		
		u16 speed_downshift:2;	
		u16 tx_fifo_depth:2;	
		u16 res1:1;		
		u16 crs_tx_en:1;		
#endif
	} bits;
} MI_PHY_CONFIG_t, *PMI_PHY_CONFIG_t;


typedef union _MI_PHY_CONTROL_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 res1:1;		
		u16 tdr_en:1;		
		u16 res2:1;		
		u16 downshift_attempts:2;	
		u16 res3:5;		
		u16 jabber_10baseT:1;	
		u16 sqe_10baseT:1;		
		u16 tp_loopback_10baseT:1;	
		u16 preamble_gen_en:1;	
		u16 res4:1;		
		u16 force_int:1;		
#else
		u16 force_int:1;		
		u16 res4:1;		
		u16 preamble_gen_en:1;	
		u16 tp_loopback_10baseT:1;	
		u16 sqe_10baseT:1;		
		u16 jabber_10baseT:1;	
		u16 res3:5;		
		u16 downshift_attempts:2;	
		u16 res2:1;		
		u16 tdr_en:1;		
		u16 res1:1;		
#endif
	} bits;
} MI_PHY_CONTROL_t, *PMI_PHY_CONTROL_t;


typedef union _MI_IMR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 res1:6;		
		u16 mdio_sync_lost:1;	
		u16 autoneg_status:1;	
		u16 hi_bit_err:1;		
		u16 np_rx:1;		
		u16 err_counter_full:1;	
		u16 fifo_over_underflow:1;	
		u16 rx_status:1;		
		u16 link_status:1;		
		u16 automatic_speed:1;	
		u16 int_en:1;		
#else
		u16 int_en:1;		
		u16 automatic_speed:1;	
		u16 link_status:1;		
		u16 rx_status:1;		
		u16 fifo_over_underflow:1;	
		u16 err_counter_full:1;	
		u16 np_rx:1;		
		u16 hi_bit_err:1;		
		u16 autoneg_status:1;	
		u16 mdio_sync_lost:1;	
		u16 res1:6;		
#endif
	} bits;
} MI_IMR_t, *PMI_IMR_t;


typedef union _MI_ISR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 res1:6;		
		u16 mdio_sync_lost:1;	
		u16 autoneg_status:1;	
		u16 hi_bit_err:1;		
		u16 np_rx:1;		
		u16 err_counter_full:1;	
		u16 fifo_over_underflow:1;	
		u16 rx_status:1;		
		u16 link_status:1;		
		u16 automatic_speed:1;	
		u16 int_en:1;		
#else
		u16 int_en:1;		
		u16 automatic_speed:1;	
		u16 link_status:1;		
		u16 rx_status:1;		
		u16 fifo_over_underflow:1;	
		u16 err_counter_full:1;	
		u16 np_rx:1;		
		u16 hi_bit_err:1;		
		u16 autoneg_status:1;	
		u16 mdio_sync_lost:1;	
		u16 res1:6;		
#endif
	} bits;
} MI_ISR_t, *PMI_ISR_t;


typedef union _MI_PSR_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 res1:1;		
		u16 autoneg_fault:2;	
		u16 autoneg_status:1;	
		u16 mdi_x_status:1;	
		u16 polarity_status:1;	
		u16 speed_status:2;	
		u16 duplex_status:1;	
		u16 link_status:1;		
		u16 tx_status:1;		
		u16 rx_status:1;		
		u16 collision_status:1;	
		u16 autoneg_en:1;		
		u16 pause_en:1;		
		u16 asymmetric_dir:1;	
#else
		u16 asymmetric_dir:1;	
		u16 pause_en:1;		
		u16 autoneg_en:1;		
		u16 collision_status:1;	
		u16 rx_status:1;		
		u16 tx_status:1;		
		u16 link_status:1;		
		u16 duplex_status:1;	
		u16 speed_status:2;	
		u16 polarity_status:1;	
		u16 mdi_x_status:1;	
		u16 autoneg_status:1;	
		u16 autoneg_fault:2;	
		u16 res1:1;		
#endif
	} bits;
} MI_PSR_t, *PMI_PSR_t;


typedef union _MI_LCR1_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 res1:2;		
		u16 led_dup_indicate:2;	
		u16 led_10baseT:2;		
		u16 led_collision:2;	
		u16 res2:2;		
		u16 res3:2;		
		u16 pulse_dur:2;		
		u16 pulse_stretch1:1;	
		u16 pulse_stretch0:1;	
#else
		u16 pulse_stretch0:1;	
		u16 pulse_stretch1:1;	
		u16 pulse_dur:2;		
		u16 res3:2;		
		u16 res2:2;		
		u16 led_collision:2;	
		u16 led_10baseT:2;		
		u16 led_dup_indicate:2;	
		u16 res1:2;		
#endif
	} bits;
} MI_LCR1_t, *PMI_LCR1_t;


typedef union _MI_LCR2_t {
	u16 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u16 led_link:4;		
		u16 led_tx_rx:4;		
		u16 led_100BaseTX:4;	
		u16 led_1000BaseT:4;	
#else
		u16 led_1000BaseT:4;	
		u16 led_100BaseTX:4;	
		u16 led_tx_rx:4;		
		u16 led_link:4;		
#endif
	} bits;
} MI_LCR2_t, *PMI_LCR2_t;




struct et131x_adapter;


void TPAL_SetPhy10HalfDuplex(struct et131x_adapter *adapter);
void TPAL_SetPhy10FullDuplex(struct et131x_adapter *adapter);
void TPAL_SetPhy10Force(struct et131x_adapter *pAdapter);
void TPAL_SetPhy100HalfDuplex(struct et131x_adapter *adapter);
void TPAL_SetPhy100FullDuplex(struct et131x_adapter *adapter);
void TPAL_SetPhy100Force(struct et131x_adapter *pAdapter);
void TPAL_SetPhy1000FullDuplex(struct et131x_adapter *adapter);
void TPAL_SetPhyAutoNeg(struct et131x_adapter *adapter);


int et131x_xcvr_find(struct et131x_adapter *adapter);
int et131x_setphy_normal(struct et131x_adapter *adapter);
int32_t PhyMiRead(struct et131x_adapter *adapter,
	       u8 xcvrAddr, u8 xcvrReg, u16 *value);


#define MiRead(adapter, xcvrReg, value) \
	PhyMiRead((adapter), (adapter)->Stats.xcvr_addr, (xcvrReg), (value))

int32_t MiWrite(struct et131x_adapter *adapter,
		u8 xcvReg, u16 value);
void et131x_Mii_check(struct et131x_adapter *pAdapter,
		      MI_BMSR_t bmsr, MI_BMSR_t bmsr_ints);


void SetPhy_10BaseTHalfDuplex(struct et131x_adapter *adapter);




#define TRUEPHY_BIT_CLEAR               0
#define TRUEPHY_BIT_SET                 1
#define TRUEPHY_BIT_READ                2


#ifndef TRUEPHY_READ
#define TRUEPHY_READ                    0
#define TRUEPHY_WRITE                   1
#define TRUEPHY_MASK                    2
#endif


#define TRUEPHY_SPEED_10MBPS            0
#define TRUEPHY_SPEED_100MBPS           1
#define TRUEPHY_SPEED_1000MBPS          2


#define TRUEPHY_DUPLEX_HALF             0
#define TRUEPHY_DUPLEX_FULL             1


#define TRUEPHY_CFG_SLAVE               0
#define TRUEPHY_CFG_MASTER              1


#define TRUEPHY_MDI                     0
#define TRUEPHY_MDIX                    1
#define TRUEPHY_AUTO_MDI_MDIX           2


#define TRUEPHY_POLARITY_NORMAL         0
#define TRUEPHY_POLARITY_INVERTED       1


#define TRUEPHY_ANEG_NOT_COMPLETE       0
#define TRUEPHY_ANEG_COMPLETE           1
#define TRUEPHY_ANEG_DISABLED           2


#define TRUEPHY_ADV_DUPLEX_NONE         0x00
#define TRUEPHY_ADV_DUPLEX_FULL         0x01
#define TRUEPHY_ADV_DUPLEX_HALF         0x02
#define TRUEPHY_ADV_DUPLEX_BOTH     \
    (TRUEPHY_ADV_DUPLEX_FULL | TRUEPHY_ADV_DUPLEX_HALF)

#define PHY_CONTROL                0x00	
#define PHY_STATUS                 0x01	
#define PHY_ID_1                   0x02	
#define PHY_ID_2                   0x03	
#define PHY_AUTO_ADVERTISEMENT     0x04	
#define PHY_AUTO_LINK_PARTNER      0x05	
#define PHY_AUTO_EXPANSION         0x06	
#define PHY_AUTO_NEXT_PAGE_TX      0x07	
#define PHY_LINK_PARTNER_NEXT_PAGE 0x08	
#define PHY_1000_CONTROL           0x09	
#define PHY_1000_STATUS            0x0A	

#define PHY_EXTENDED_STATUS        0x0F	


#define PHY_INDEX_REG              0x10
#define PHY_DATA_REG               0x11

#define PHY_MPHY_CONTROL_REG       0x12	

#define PHY_LOOPBACK_CONTROL       0x13	
					
#define PHY_REGISTER_MGMT_CONTROL  0x15	
#define PHY_CONFIG                 0x16	
#define PHY_PHY_CONTROL            0x17	
#define PHY_INTERRUPT_MASK         0x18	
#define PHY_INTERRUPT_STATUS       0x19	
#define PHY_PHY_STATUS             0x1A	
#define PHY_LED_1                  0x1B	
#define PHY_LED_2                  0x1C	
					
					


void ET1310_PhyInit(struct et131x_adapter *adapter);
void ET1310_PhyReset(struct et131x_adapter *adapter);
void ET1310_PhyPowerDown(struct et131x_adapter *adapter, bool down);
void ET1310_PhyAutoNeg(struct et131x_adapter *adapter, bool enable);
void ET1310_PhyDuplexMode(struct et131x_adapter *adapter, u16 duplex);
void ET1310_PhySpeedSelect(struct et131x_adapter *adapter, u16 speed);
void ET1310_PhyAdvertise1000BaseT(struct et131x_adapter *adapter,
				  u16 duplex);
void ET1310_PhyAdvertise100BaseT(struct et131x_adapter *adapter,
				 u16 duplex);
void ET1310_PhyAdvertise10BaseT(struct et131x_adapter *adapter,
				u16 duplex);
void ET1310_PhyLinkStatus(struct et131x_adapter *adapter,
			  u8 *Link_status,
			  u32 *autoneg,
			  u32 *linkspeed,
			  u32 *duplex_mode,
			  u32 *mdi_mdix,
			  u32 *masterslave, u32 *polarity);
void ET1310_PhyAndOrReg(struct et131x_adapter *adapter,
			u16 regnum, u16 andMask, u16 orMask);
void ET1310_PhyAccessMiBit(struct et131x_adapter *adapter,
			   u16 action,
			   u16 regnum, u16 bitnum, u8 *value);

#endif 
