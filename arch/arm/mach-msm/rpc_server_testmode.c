




#include <linux/module.h>
#include <linux/kernel.h> 

#include <mach/msm_rpcrouter.h>

#include "rpc_server_testmode.h"

#include "rpc_server_testmode_keytest.h"
#include "rpc_server_testmode_memory.h"
#include "rpc_server_testmode_sound.h"
#include "rpc_server_testmode_motor.h"
#include "testmode_sensor.h"
#include "rpc_server_testmode_cam.h"


#include "testmode_input.h"

#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include <linux/sched.h>
#include <linux/wait.h>

#include <linux/delay.h>


#define DEBUG_TESTMODE

#define TESTMODE_APPS_APISPROG		0x30000101
#define TESTMODE_APPS_APISVERS		0


#define ONCRPC_TESTMODE_CMD_RPOC  3
#define ONCRPC_TESTMODE_CMD_LARGE_PROC 4

#define HANDLE_OK  0
#define HANLDE_FAIL 1
#define HANDLE_ERROR 2
#define HANDLE_OK_MIDDLE 4

#define TESTMODE_DEVICE_NAME "testmode"

#define TESTMODE_SUB1_CMD_SIZE	4
#define TESTMODE_SUB2_CMD_SIZE	4
#define TESTMODE_CMD_BUF_SIZE		((TESTMODE_SUB1_CMD_SIZE)+(TESTMODE_SUB2_CMD_SIZE)+(1))

#define TESTMODE_RSP_BUF_SIZE		((4) + (40))

dev_t testmode_device_number;

struct testmode_dev {
	unsigned int major;
	unsigned int minor_start;
	
	char *name;
	struct semaphore sem;
	struct cdev *cdev;
	struct class *testmode_class;
};

static DECLARE_WAIT_QUEUE_HEAD(testmode_read_wq);
static DECLARE_WAIT_QUEUE_HEAD(testmode_write_wq);

static int read_flag = 0, write_flag = 0;
static unsigned char testmode_cmd_buf[TESTMODE_CMD_BUF_SIZE] = {0x0,};
static unsigned char testmode_rsp_buf[TESTMODE_RSP_BUF_SIZE] = {0x0,};

static struct testmode_dev *driver;
static struct cdev *testmode_device;

void* testmode_dummy(uint32_t sub1_cmd, uint32_t sub2_cmd);
void* testmode_factory_reset(uint32_t sub1_cmd, uint32_t sub2_cmd);
void *testmode_keyinput(uint32_t sub1_cmd, uint32_t sub2_cmd);
void* testmode_lcd(uint32_t sub1_cmd, uint32_t sub2_cmd);
void *testmode_bluetooth(uint32_t sub1_cmd, uint32_t sub2_cmd);
void *testmode_wifi_dmb(uint32_t sub1_cmd, uint32_t sub2_cmd);
void *testmode_mp3_test(uint32_t sub1_cmd, uint32_t sub2_cmd);
void *testmode_dispath_command(uint32_t sub1_cmd, uint32_t sub2_cmd);

void* testmode_first_boot_complete_check(uint32_t sub1_cmd, uint32_t sub2_cmd);

extern void* testmode_sleep_mode_test(uint32_t sub1_cmd, uint32_t sub2_cmd); 
#ifdef LGE_FW_LU2300
extern void qwerty_input_report_key(unsigned char scancode);
extern int qwerty_exist_scancode(unsigned char scancode);
#endif
extern int keypad_exist_scancode(unsigned char scancode);
extern void keypad_input_report_key(unsigned char scancode);


testmode_func_table_entry_type g_testmode_func_table[] =
{
        
        {TEST_MODE_SW_REV, testmode_dummy, TEST_MODE_MODEM_PROC},
        {TEST_MODE_LCD, testmode_lcd, TEST_MODE_APP_PROC},
        {TEST_MODE_FOLDER, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_MOTOR, testmode_motor_test, TEST_MODE_APP_PROC},
        {TEST_MODE_ACOUSTIC, testmode_acoustic_test, TEST_MODE_APP_PROC},	
        {TEST_MODE_MIDI, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_VOD, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_CAM, testmode_cam, TEST_MODE_APP_PROC},	
        {TEST_MODE_BUZZER, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_NOT_DEFINED_9,  NULL, TEST_MODE_MODEM_PROC},

        
        {TEST_MODE_FACTORY_INIT, NULL, TEST_MODE_MODEM_PROC},					
        {TEST_MODE_EFS_INTEGRITY, testmode_efs_integrity, TEST_MODE_APP_PROC}, 
        {TEST_MODE_TX_POWER, NULL, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_IRDA, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_TRAFFIC_CHIP, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_MFORMAT, NULL, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_PHONE_CLEAR, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_NOT_DEFINED_17, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_LIGHTSENSOR, NULL, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_WIPI, NULL, TEST_MODE_MODEM_PROC},

        
        {TEST_MODE_BREW_CNT, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_BREW_SIZE, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_KEY_TEST, testmode_key_test, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_EXTERNAL_MEMORY, testmode_external_memory, TEST_MODE_APP_PROC},
        {TEST_MODE_BLUETOOTH, testmode_bluetooth, TEST_MODE_APP_PROC},
        {TEST_MODE_BATT_BAR_LEVEL, NULL, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_ANT_BAR_SIZE, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_MP3_TEST, testmode_mp3_test, TEST_MODE_APP_PROC},
        {TEST_MODE_FM_TX,NULL, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_SGPS, NULL, TEST_MODE_MODEM_PROC},

        
        {TEST_MODE_GEOMETRIC_SENSOR, NULL, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_ACCELATOR,  testmode_accelator, TEST_MODE_APP_PROC},
        {TEST_MODE_ALCHOL_SENSOR,  NULL, TEST_MODE_MODEM_PROC},	
        	
        {TEST_MODE_WLAN_TDMB_SDMB,  testmode_wifi_dmb, TEST_MODE_APP_PROC},
        	
        {TEST_MODE_TV_OUT,  NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_NOT_DEFINED_35,  NULL, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_MANUAL_TEST, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_REMOVABLE_DISK_TEST, NULL, TEST_MODE_MODEM_PROC},		
        {TEST_MODE_UV_SENSOR_TEST, testmode_uv_sensor_test, TEST_MODE_APP_PROC},
        {TEST_MODE_3D_ACCELATOR_TEST, NULL, TEST_MODE_MODEM_PROC},		

        
        {TEST_MODE_KEYDATA_INPUT, testmode_keyinput, TEST_MODE_APP_PROC},
        {TEST_MODE_MEMORY_CAPACITY_CHECK, testmode_memory_volume_check, TEST_MODE_APP_PROC},
        {TEST_MODE_SLEEP_MODE_TEST, testmode_sleep_mode_test, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_SPEAKER_PHONE_MODE, testmode_speaker_phone_test, TEST_MODE_APP_PROC},
        {TEST_MODE_VIRTUAL_SIM_TEST, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_PHOTO_SENSOR, NULL, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_VCO_SELF_TUNING_TEST, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_MRD_USB_TEST, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_TEST_SCR_CAL_BACKUP_TEST, NULL, TEST_MODE_MODEM_PROC},	
        {TEST_MODE_PROXIMITY_SENSOR_TEST, testmode_proximity, TEST_MODE_MODEM_PROC},

        
        {TEST_MODE_FACTORY_RESET_TEST, testmode_factory_reset, TEST_MODE_MODEM_PROC},
        {TEST_MODE_VOLUME_LEVEL_TEST, testmode_volume_level_test, TEST_MODE_APP_PROC},
        
        
        {TEST_MODE_MEMORY_BAD_BLOCK_CHECK, testmode_memory_bad_block_check, TEST_MODE_MODEM_PROC},
        {TEST_MODE_FIRST_BOOTING_COMPLETE_CHECK, testmode_first_boot_complete_check, TEST_MODE_APP_PROC},
        
        {TEST_MODE_PID_CMD, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_SW_VERSION_CMD, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_IMEI_CMD, NULL, TEST_MODE_MODEM_PROC},
        
        {TEST_MODE_CAL_CHECK_CMD, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_BT_ADDRESS_CMD, NULL, TEST_MODE_MODEM_PROC},
        {TEST_MODE_MAX, NULL, TEST_MODE_MODEM_PROC},
        

};

struct rpc_misc_apps_bases_args {
	uint32_t sub_cmd1;
	uint32_t sub_cmd2;
};

void *testmode_reponse_not_supported(void)
{
	struct testmode_relay_result *relay_result;

	relay_result = (struct testmode_relay_result *)kzalloc(sizeof(struct testmode_relay_result), GFP_KERNEL);

	relay_result->ret_value = TESTMODE_NOT_SUPPORTED;
	strcpy(relay_result->ret_string, "TESTMODE_NOT_SUPPORTED");
	return relay_result;
}


void* testmode_dispath_command(uint32_t sub1_cmd, uint32_t sub2_cmd)
{
#ifdef DEBUG_TESTMODE
	int i = 0;
	uint32_t t_sub1 = 0;
#endif

	struct testmode_relay_result *relay_result;
	
	relay_result = (struct testmode_relay_result *)kzalloc(sizeof(struct testmode_relay_result), GFP_KERNEL);

	relay_result->ret_value = TESTMODE_SUCCESS;
		
	testmode_cmd_buf[0] = (unsigned char)((0xff000000 & sub1_cmd) >> 24);
	testmode_cmd_buf[1] = (unsigned char)((0x00ff0000 & sub1_cmd) >> 16);
	testmode_cmd_buf[2] = (unsigned char)((0x0000ff00 & sub1_cmd) >> 8);
	testmode_cmd_buf[3] = (unsigned char)((0x000000ff & sub1_cmd));

	testmode_cmd_buf[4] = (unsigned char)((0xff000000 & sub2_cmd) >> 24);
	testmode_cmd_buf[5] = (unsigned char)((0x00ff0000 & sub2_cmd) >> 16);
	testmode_cmd_buf[6] = (unsigned char)((0x0000ff00 & sub2_cmd) >> 8);
	testmode_cmd_buf[7] = (unsigned char)((0x000000ff & sub2_cmd));

#ifdef DEBUG_TESTMODE
	printk("[LGE] dispatch: ");
	for (i = 0 ; i < 8 ; i++)
		printk("%d", testmode_cmd_buf[i]);
	printk("\n");

	printk("[LGE] t_sub1 = %d\n", t_sub1);
#endif

	
	read_flag =1;
	wake_up_interruptible(&testmode_read_wq); 

	
	wait_event_interruptible(testmode_write_wq, write_flag != 0);
	write_flag = 0;

	relay_result->ret_value  =  0x01000000 * testmode_rsp_buf[0];
	relay_result->ret_value += 0x00010000 * testmode_rsp_buf[1];
	relay_result->ret_value += 0x00000100 * testmode_rsp_buf[2];
	relay_result->ret_value += 0x00000001 * testmode_rsp_buf[3];

	strncpy(relay_result->ret_string, &testmode_rsp_buf[4], MAX_STRING_RET);

#ifdef DEBUG_TESTMODE
	printk("[LGE] ret_value = %d, ret_string = %s\n", relay_result->ret_value, relay_result->ret_string);
#endif
	
	return relay_result;
}

void* testmode_factory_reset(uint32_t sub1_cmd, uint32_t sub2_cmd)
{
	struct testmode_relay_result *relay_result;

	relay_result = (struct testmode_relay_result *)kzalloc(sizeof(struct testmode_relay_result), GFP_KERNEL);

	switch (sub2_cmd) 
	{
		
		case 0:
			testmode_input_report_evt(TESTMODE_INPUT_KEY_INIT);
			msleep(1000);
			testmode_input_report_evt(TESTMODE_INPUT_KEY_UNLOCK);
			testmode_input_report_evt(TESTMODE_INPUT_FACTORY_RESET);
			relay_result->ret_value = TESTMODE_SUCCESS;
			break;

        case 98:
            testmode_input_report_evt(TESTMODE_INPUT_KEY_UNLOCK);
            relay_result->ret_value = TESTMODE_SUCCESS;
            break;

		default :
				kzfree(relay_result);
				relay_result = testmode_reponse_not_supported();
				break;	
	}

	return relay_result;
}

void *testmode_bluetooth(uint32_t sub1_cmd, uint32_t sub2_cmd)
{
	struct testmode_relay_result *relay_result;

	testmode_input_report_evt(TESTMODE_INPUT_KEY_INIT);
	msleep(1000);
	testmode_input_report_evt(TESTMODE_INPUT_KEY_UNLOCK);

	msleep(1500);


  if(sub2_cmd == 5)
  {
    testmode_input_report_evt(TESTMODE_PATH_CHANGE_TO_BT_SCO_CALL_PATH_OFF);


  }
  else if(sub2_cmd == 2)
  {
      
  }
  else
  {


    testmode_input_report_evt(TESTMODE_PATH_CHANGE_TO_BT_SCO_CALL_PATH);

 }

	relay_result = testmode_dispath_command(sub1_cmd, sub2_cmd);

	return relay_result;

}


void *testmode_wifi_dmb(uint32_t sub1_cmd, uint32_t sub2_cmd)
{
	struct testmode_relay_result *relay_result;

	if((sub2_cmd>= 0)&& (sub2_cmd<= 3))
	{
		relay_result = (struct testmode_relay_result *)kzalloc(sizeof(struct testmode_relay_result), GFP_KERNEL);
		testmode_input_report_evt(TESTMODE_INPUT_KEY_INIT);
		msleep(1000);
		testmode_input_report_evt(TESTMODE_INPUT_KEY_UNLOCK);		

		switch (sub2_cmd) 
		{
			
			case 0:	
				testmode_input_report_evt(TESTMODE_INPUT_KEY_TDMB_ON);
				relay_result->ret_value = TESTMODE_SUCCESS;
				break;
			case 1: 
				testmode_input_report_evt(TESTMODE_INPUT_KEY_TDMB_OFF);
				relay_result->ret_value = TESTMODE_SUCCESS;
				break;
			case 2: 
				testmode_input_report_evt(TESTMODE_INPUT_KEY_TDMB_RM_CH);
				relay_result->ret_value = TESTMODE_SUCCESS;
				break;
			case 3: 
				
				relay_result->ret_value = TESTMODE_SUCCESS;
				break;
			default :
				kzfree(relay_result);
				relay_result = testmode_reponse_not_supported();
				break;
		}
	}
	
#if 1	


	else if((sub2_cmd>= 4)&& (sub2_cmd<= 70))

#else
	else if((sub2_cmd>= 4)&& (sub2_cmd<= 69))
#endif

	{
		
		relay_result = testmode_dispath_command(sub1_cmd, sub2_cmd);
	}
	else
	{
			return testmode_reponse_not_supported();
	}

	return relay_result;
}




void *testmode_keyinput(uint32_t sub1_cmd, uint32_t sub2_cmd)
{
	struct testmode_relay_result *relay_result;
	
	relay_result = (struct testmode_relay_result *)kzalloc(sizeof(struct testmode_relay_result), GFP_KERNEL);

	relay_result->ret_value = TESTMODE_SUCCESS;

	switch(sub2_cmd)
	{
		case 35:
			testmode_input_report_evt(TESTMODE_POUND);			
			break;
		case 42:
			testmode_input_report_evt(TESTMODE_STAR);			
			break;	
		case 48:
			testmode_input_report_evt(TESTMODE_NUM_0);			
			break;			
		case 49:
			testmode_input_report_evt(TESTMODE_NUM_1);			
			break;
		case 50:
			testmode_input_report_evt(TESTMODE_NUM_2);			
			break;	
		case 51:
			testmode_input_report_evt(TESTMODE_NUM_3);			
			break;	
		case 52:
			testmode_input_report_evt(TESTMODE_NUM_4);			
			break;
		case 53:
			testmode_input_report_evt(TESTMODE_NUM_5);			
			break;	
		case 54:
			testmode_input_report_evt(TESTMODE_NUM_6);			
			break;	
		case 55:
			testmode_input_report_evt(TESTMODE_NUM_7);			
			break;
		case 56:
			testmode_input_report_evt(TESTMODE_NUM_8);			
			break;	
		case 57:
			testmode_input_report_evt(TESTMODE_NUM_9);			
			break;			
		case 80:
			testmode_input_report_evt(TESTMODE_CALL);			
			break;
		case 81:
			testmode_input_report_evt(TESTMODE_ENDCALL);			
			break;	
		case 82:
			testmode_input_report_evt(TESTMODE_CLEAR);			
			break;	
		case 83:
			testmode_input_report_evt(TESTMODE_DPAD_CENTER);			
			break;
		case 150:
			testmode_input_report_evt(TESTMODE_DPAD_UP);			
			break;	
		case 151:
			testmode_input_report_evt(TESTMODE_DPAD_DOWN);			
			break;	

		default:
			relay_result->ret_value = TESTMODE_NOT_SUPPORTED;
			break;
	}

	return relay_result;
}





void *testmode_lcd(uint32_t sub1_cmd, uint32_t sub2_cmd)
{
	struct testmode_relay_result *relay_result;
	
	relay_result = (struct testmode_relay_result *)kzalloc(sizeof(struct testmode_relay_result), GFP_KERNEL);

	relay_result->ret_value = TESTMODE_SUCCESS;

	switch (sub2_cmd) 
	{
	   case 0:
		 	testmode_input_report_evt(TESTMODE_INPUT_KEY_INIT);
			msleep(1000);
		 	testmode_input_report_evt(TESTMODE_INPUT_KEY_UNLOCK);
			testmode_input_report_evt(TESTMODE_INPUT_KEY_HOME);
			break;
			
		case 2:
		 	testmode_input_report_evt(TESTMODE_INPUT_KEY_INIT);
			msleep(1000);
		 	testmode_input_report_evt(TESTMODE_INPUT_KEY_UNLOCK);			
			testmode_input_report_evt(TESTMODE_INPUT_KEY_HOME);
			testmode_input_report_evt(TESTMODE_LCD_TILT);
			break;

		case 3:
		 	testmode_input_report_evt(TESTMODE_INPUT_KEY_INIT);
			msleep(1000);
		 	testmode_input_report_evt(TESTMODE_INPUT_KEY_UNLOCK);			
			testmode_input_report_evt(TESTMODE_INPUT_KEY_HOME);
			testmode_input_report_evt(TESTMODE_LCD_COLOR_DISPLAY);
			break;

		
		case 4:  
		 	testmode_input_report_evt(TESTMODE_INPUT_KEY_INIT);
			msleep(1000);
		 	testmode_input_report_evt(TESTMODE_INPUT_KEY_UNLOCK);			
			break;

		case 5: 
		 	testmode_input_report_evt(TESTMODE_LCDOFF_AND_LOCK);
			break;

		default:
			return testmode_reponse_not_supported();
	}

	return relay_result;
}



void* testmode_first_boot_complete_check(uint32_t sub1_cmd, uint32_t sub2_cmd)
{
    struct testmode_relay_result *relay_result;

    if( sub2_cmd == 0x00)
    {
        relay_result = testmode_dispath_command(sub1_cmd, sub2_cmd);
    }
    else
    {
        relay_result = testmode_reponse_not_supported();
    }

    return relay_result;

}


void* testmode_dummy(uint32_t sub1_cmd, uint32_t sub2_cmd)
{
	return testmode_reponse_not_supported();
}

static int  testmode_process_command(uint32_t sub1_cmd, uint32_t sub2_cmd, struct msm_rpc_server *server)
{
    struct testmode_relay_result *relay_result;
    testmode_func_table_entry_type *testmode_func_entry = NULL;
    int nIndex = 0;  

    if (sub1_cmd >= TEST_MODE_MAX) {
        relay_result = (struct testmode_relay_result *)testmode_reponse_not_supported();
        goto exit;
    }

    testmode_func_entry = &g_testmode_func_table[sub1_cmd];

    
    while( g_testmode_func_table[nIndex].sub1_cmd != TEST_MODE_MAX )
    {
        if( sub1_cmd == g_testmode_func_table[nIndex].sub1_cmd )
        {
            testmode_func_entry = &g_testmode_func_table[nIndex];
            break;
        }
        nIndex++;
    }

    if( testmode_func_entry ==NULL )
    {
        relay_result = testmode_reponse_not_supported();
        return relay_result;
    }
    

    if (testmode_func_entry->testmode_func == NULL) {
        relay_result = (struct testmode_relay_result *)testmode_reponse_not_supported();
        goto exit;
    }

    relay_result = (struct testmode_relay_result *)(testmode_func_entry->testmode_func)((uint32_t)sub1_cmd, (uint32_t)sub2_cmd);

exit:

    server->retvalues.ret_value = relay_result->ret_value;
    memcpy(server->retvalues.ret_string, relay_result->ret_string, MAX_STRING_RET);
    

    kfree(relay_result);

    return HANDLE_OK;
}

static int handle_testmode_rpc_call(struct msm_rpc_server *server, struct rpc_request_hdr *req, unsigned len)
{
	int result = RPC_ACCEPTSTAT_TESTMODE_SUCCESS;

	switch (req->procedure)
	{
		case ONCRPC_TESTMODE_CMD_RPOC:
		{
			struct rpc_misc_apps_bases_args *args;
			args = (struct rpc_misc_apps_bases_args *)(req + 1);
			args->sub_cmd1 = be32_to_cpu(args->sub_cmd1);
			args->sub_cmd2 = be32_to_cpu(args->sub_cmd2);

			
			
						

			memset(server->retvalues.ret_string, 0, sizeof(server->retvalues.ret_string));
		
			if(testmode_process_command(args->sub_cmd1, args->sub_cmd2, server) == HANDLE_OK) {
				result = RPC_ACCEPTSTAT_TESTMODE_SUCCESS;
			} else { 
				result= RPC_ACCEPTSTAT_TESTMODE_ERROR;
			}
		}
			break;

		default:

			break;
	}

	return result;
}



int testmode_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int  testmode_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t  testmode_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	printk("[LGE] testmode write function called\n");

	if (count > TESTMODE_RSP_BUF_SIZE)
		return -EIO;

	if (copy_from_user(testmode_rsp_buf, buf, count)) {
		return -EFAULT;
	}

	printk("[LGE] testmode write, %s\n", testmode_rsp_buf);
	write_flag =1;
	wake_up_interruptible(&testmode_write_wq);
		
	return count;
}

ssize_t  testmode_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	int ret = 0;
	char *kbuf;
	
	printk("[LGE] testmode read function called\n");

	ret = wait_event_interruptible(testmode_read_wq, read_flag != 0);
	printk("[LGE] testmode_read, ret = %d, read_flag = %d\n", ret, read_flag);

	read_flag = 0;

	if (ret < 0) {
		return ret;
	}	

	count = sizeof(testmode_cmd_buf);
	kbuf = kzalloc(20+1, GFP_KERNEL);
	
	memcpy(kbuf, testmode_cmd_buf, TESTMODE_CMD_BUF_SIZE);
	printk("[LGE] testmode_read= %s\n", testmode_cmd_buf);

	if (copy_to_user(buf, (char *)kbuf, TESTMODE_CMD_BUF_SIZE)) {
		printk("[LGE] testmode read fail\n");
		return -EFAULT;
	}

	kfree(kbuf);
	printk("[LGE] testmode_read returns\n");
		
	return count;
}

static struct msm_rpc_server rpc_server = {

	.prog = TESTMODE_APPS_APISPROG,
	.vers = TESTMODE_APPS_APISVERS,
	.rpc_call = handle_testmode_rpc_call,

};

static struct file_operations td_fops = {
	.read = testmode_read,
	.write = testmode_write,
	.open = testmode_open,
	.release = testmode_release,
};

static int __init rpc_testmode_server_init(void)
{
	int ret = 0;
	
	

	driver = kzalloc(sizeof(struct testmode_dev) + 9, GFP_KERNEL);

	driver->name = ((void *)driver) + sizeof(struct testmode_dev);
	strncpy(driver->name, "testmode", 8);
	driver->name[8] = '\0';
	
	ret = alloc_chrdev_region(&testmode_device_number, 0, 1, TESTMODE_DEVICE_NAME);

	driver->major = MAJOR(testmode_device_number);
	driver->minor_start = MINOR(testmode_device_number);

	if (ret) {
		printk(KERN_ERR "[LGE] unable to register minors for testmode device\n");
		goto err_reg_dev;
	}
	
	testmode_device = cdev_alloc();
	driver->cdev = testmode_device;

	if (IS_ERR(testmode_device)) {
		printk(KERN_ERR "[LGE] unable to alloc for testmode device\n");
		goto err_alloc_dev;
	}
			
	cdev_init(testmode_device, &td_fops);
	testmode_device->owner = THIS_MODULE;

	
	cdev_add(testmode_device, testmode_device_number, 1);
	
	driver->testmode_class = class_create(THIS_MODULE, "testmode");
	
	if (IS_ERR(driver->testmode_class)) {
		printk(KERN_ERR "Error creating testmode class.\n");
		return -1;
	}
	
	device_create(driver->testmode_class, NULL, testmode_device_number,
					(void *)driver, "testmode");

	printk("[LGE][Testmode] Major : %d\n", MAJOR(testmode_device_number));
	printk("[LGE][Testmode] Minor : %d\n", MINOR(testmode_device_number));

	return msm_rpc_create_server(&rpc_server);

err_alloc_dev:
err_reg_dev:
	unregister_chrdev_region(testmode_device_number, 1);
	return ret;
	
}

module_init(rpc_testmode_server_init);

