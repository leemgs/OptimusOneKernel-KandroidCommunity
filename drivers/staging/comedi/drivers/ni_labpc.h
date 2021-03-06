

#ifndef _NI_LABPC_H
#define _NI_LABPC_H

#define EEPROM_SIZE	256	
#define NUM_AO_CHAN	2	

enum labpc_bustype { isa_bustype, pci_bustype, pcmcia_bustype };
enum labpc_register_layout { labpc_plus_layout, labpc_1200_layout };
enum transfer_type { fifo_not_empty_transfer, fifo_half_full_transfer,
	isa_dma_transfer
};

struct labpc_board_struct {
	const char *name;
	int device_id;		
	int ai_speed;		
	enum labpc_bustype bustype;	
	enum labpc_register_layout register_layout;	
	int has_ao;		
	const struct comedi_lrange *ai_range_table;
	const int *ai_range_code;
	const int *ai_range_is_unipolar;
	unsigned ai_scan_up:1;	
	unsigned memory_mapped_io:1;	
};

struct labpc_private {
	struct mite_struct *mite;	
	volatile unsigned long long count;	
	unsigned int ao_value[NUM_AO_CHAN];	
	
	volatile unsigned int command1_bits;
	volatile unsigned int command2_bits;
	volatile unsigned int command3_bits;
	volatile unsigned int command4_bits;
	volatile unsigned int command5_bits;
	volatile unsigned int command6_bits;
	
	volatile unsigned int status1_bits;
	volatile unsigned int status2_bits;
	unsigned int divisor_a0;	
	unsigned int divisor_b0;	
	unsigned int divisor_b1;	
	unsigned int dma_chan;	
	u16 *dma_buffer;	
	unsigned int dma_transfer_size;	
	enum transfer_type current_transfer;	
	unsigned int eeprom_data[EEPROM_SIZE];	
	unsigned int caldac[16];	
	
	unsigned int (*read_byte) (unsigned long address);
	void (*write_byte) (unsigned int byte, unsigned long address);
};

int labpc_common_attach(struct comedi_device *dev, unsigned long iobase,
			unsigned int irq, unsigned int dma);
int labpc_common_detach(struct comedi_device *dev);

extern const int labpc_1200_is_unipolar[];
extern const int labpc_1200_ai_gain_bits[];
extern const struct comedi_lrange range_labpc_1200_ai;

#endif 
