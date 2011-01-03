

#ifndef EFX_SPI_H
#define EFX_SPI_H

#include "net_driver.h"



#define SPI_WRSR 0x01		
#define SPI_WRITE 0x02		
#define SPI_READ 0x03		
#define SPI_WRDI 0x04		
#define SPI_RDSR 0x05		
#define SPI_WREN 0x06		
#define SPI_SST_EWSR 0x50	

#define SPI_STATUS_WPEN 0x80	
#define SPI_STATUS_BP2 0x10	
#define SPI_STATUS_BP1 0x08	
#define SPI_STATUS_BP0 0x04	
#define SPI_STATUS_WEN 0x02	
#define SPI_STATUS_NRDY 0x01	


struct efx_spi_device {
	struct efx_nic *efx;
#ifdef CONFIG_SFC_MTD
	void *mtd;
#endif
	int device_id;
	unsigned int size;
	unsigned int addr_len;
	unsigned int munge_address:1;
	u8 erase_command;
	unsigned int erase_size;
	unsigned int block_size;
};

int falcon_spi_cmd(const struct efx_spi_device *spi, unsigned int command,
		   int address, const void* in, void *out, size_t len);
int falcon_spi_wait_write(const struct efx_spi_device *spi);
int falcon_spi_read(const struct efx_spi_device *spi, loff_t start,
		    size_t len, size_t *retlen, u8 *buffer);
int falcon_spi_write(const struct efx_spi_device *spi, loff_t start,
		     size_t len, size_t *retlen, const u8 *buffer);


#define FALCON_NVCONFIG_END 0x400U
#define FALCON_FLASH_BOOTCODE_START 0x8000U
#define EFX_EEPROM_BOOTCONFIG_START 0x800U
#define EFX_EEPROM_BOOTCONFIG_END 0x1800U

#endif 
