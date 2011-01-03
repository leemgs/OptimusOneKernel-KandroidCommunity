
#ifndef __PMIC8058_NFC_H__
#define __PMIC8058_NFC_H__

struct pm8058_nfc_device;


#define	PM_NFC_VDDLDO_MON_LEVEL		0x0003
#define	PM_NFC_VPH_PWR_EN		0x0008
#define	PM_NFC_EXT_VDDLDO_EN		0x0010
#define	PM_NFC_EN			0x0020
#define	PM_NFC_LDO_EN			0x0040
#define	PM_NFC_SUPPORT_EN		0x0080

#define	PM_NFC_EXT_EN_HIGH		0x0100
#define	PM_NFC_MBG_EN_HIGH		0x0200
#define	PM_NFC_VDDLDO_OK_HIGH		0x0400
#define	PM_NFC_DTEST1_MODE		0x2000
#define	PM_NFC_ATEST_EN			0x4000
#define	PM_NFC_VDDLDO_MON_EN		0x8000

#define	PM_NFC_CTRL_REQ			(PM_NFC_SUPPORT_EN |\
					PM_NFC_LDO_EN |\
					PM_NFC_EN |\
					PM_NFC_EXT_VDDLDO_EN |\
					PM_NFC_VPH_PWR_EN |\
					PM_NFC_VDDLDO_MON_LEVEL)

#define	PM_NFC_TEST_REQ			(PM_NFC_VDDLDO_MON_EN |\
					PM_NFC_DTEST1_MODE |\
					PM_NFC_ATEST_EN)

#define	PM_NFC_TEST_STATUS		(PM_NFC_EXT_EN_HIGH |\
					PM_NFC_MBG_EN_HIGH |\
					PM_NFC_VDDLDO_OK_HIGH)


struct pm8058_nfc_device *pm8058_nfc_request(void);


int pm8058_nfc_config(struct pm8058_nfc_device *nfcdev, u32 mask, u32 flags);


int pm8058_nfc_get_status(struct pm8058_nfc_device *nfcdev,
			  u32 mask, u32 *status);


void pm8058_nfc_free(struct pm8058_nfc_device *nfcdev);

#endif 
