

#ifndef _DRIVERS_USB_DIAG_H_
#define _DRIVERS_USB_DIAG_H_
#define ENOREQ -1
struct diag_request {
	char *buf;
	int length;
	int actual;
	int status;
	void *context;
};
struct diag_operations {

	int (*diag_connect)(void);
	int (*diag_disconnect)(void);
	int (*diag_char_write_complete)(struct diag_request *);
	int (*diag_char_read_complete)(struct diag_request *);
};

int diag_open(int);
void diag_close(void);
int diag_read(struct diag_request *);
int diag_write(struct diag_request *);

int diag_usb_register(struct diag_operations *);
int diag_usb_unregister(void);
int diag_read_from_cb(unsigned char * , int);
#endif
