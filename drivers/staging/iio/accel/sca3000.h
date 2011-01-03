
#define SCA3000_WRITE_REG(a) (((a) << 2) | 0x02)
#define SCA3000_READ_REG(a) ((a) << 2)

#define SCA3000_REG_ADDR_REVID			0x00
#define SCA3000_REVID_MAJOR_MASK		0xf0
#define SCA3000_REVID_MINOR_MASK		0x0f

#define SCA3000_REG_ADDR_STATUS			0x02
#define SCA3000_LOCKED				0x20
#define SCA3000_EEPROM_CS_ERROR			0x02
#define SCA3000_SPI_FRAME_ERROR			0x01


#define SCA3000_REG_ADDR_X_MSB			0x05
#define SCA3000_REG_ADDR_Y_MSB			0x07
#define SCA3000_REG_ADDR_Z_MSB			0x09

#define SCA3000_REG_ADDR_RING_OUT		0x0f


#define SCA3000_REG_ADDR_TEMP_MSB		0x13

#define SCA3000_REG_ADDR_MODE			0x14
#define SCA3000_MODE_PROT_MASK			0x28

#define SCA3000_RING_BUF_ENABLE			0x80
#define SCA3000_RING_BUF_8BIT			0x40

#define SCA3000_FREE_FALL_DETECT		0x10
#define SCA3000_MEAS_MODE_NORMAL		0x00
#define SCA3000_MEAS_MODE_OP_1			0x01
#define SCA3000_MEAS_MODE_OP_2			0x02


#define SCA3000_MEAS_MODE_MOT_DET		0x03

#define SCA3000_REG_ADDR_BUF_COUNT		0x15

#define SCA3000_REG_ADDR_INT_STATUS		0x16

#define SCA3000_INT_STATUS_THREE_QUARTERS	0x80
#define SCA3000_INT_STATUS_HALF			0x40

#define SCA3000_INT_STATUS_FREE_FALL		0x08
#define SCA3000_INT_STATUS_Y_TRIGGER		0x04
#define SCA3000_INT_STATUS_X_TRIGGER		0x02
#define SCA3000_INT_STATUS_Z_TRIGGER		0x01


#define SCA3000_REG_ADDR_CTRL_SEL		0x18

#define SCA3000_REG_CTRL_SEL_I2C_DISABLE	0x01
#define SCA3000_REG_CTRL_SEL_MD_CTRL		0x02
#define SCA3000_REG_CTRL_SEL_MD_Y_TH		0x03
#define SCA3000_REG_CTRL_SEL_MD_X_TH		0x04
#define SCA3000_REG_CTRL_SEL_MD_Z_TH		0x05

#define SCA3000_REG_CTRL_SEL_OUT_CTRL		0x0B
#define SCA3000_OUT_CTRL_PROT_MASK		0xE0
#define SCA3000_OUT_CTRL_BUF_X_EN		0x10
#define SCA3000_OUT_CTRL_BUF_Y_EN		0x08
#define SCA3000_OUT_CTRL_BUF_Z_EN		0x04
#define SCA3000_OUT_CTRL_BUF_DIV_4		0x02
#define SCA3000_OUT_CTRL_BUF_DIV_2		0x01


#define SCA3000_MD_CTRL_PROT_MASK		0xC0
#define SCA3000_MD_CTRL_OR_Y			0x01
#define SCA3000_MD_CTRL_OR_X			0x02
#define SCA3000_MD_CTRL_OR_Z			0x04

#define SCA3000_MD_CTRL_AND_Y			0x08
#define SCA3000_MD_CTRL_AND_X			0x10
#define SAC3000_MD_CTRL_AND_Z			0x20


#define SCA3000_REG_ADDR_UNLOCK			0x1e

#define SCA3000_REG_ADDR_INT_MASK		0x21
#define SCA3000_INT_MASK_PROT_MASK		0x1C

#define SCA3000_INT_MASK_RING_THREE_QUARTER	0x80
#define SCA3000_INT_MASK_RING_HALF		0x40

#define SCA3000_INT_MASK_ALL_INTS		0x02
#define SCA3000_INT_MASK_ACTIVE_HIGH		0x01
#define SCA3000_INT_MASK_ACTIVE_LOW		0x00


#define SCA3000_REG_ADDR_CTRL_DATA		0x22


#define SCA3000_OP_MODE_BYPASS			0x01
#define SCA3000_OP_MODE_NARROW			0x02
#define SCA3000_OP_MODE_WIDE			0x04
#define SCA3000_MAX_TX 6
#define SCA3000_MAX_RX 2


struct sca3000_state {
	struct spi_device		*us;
	const struct sca3000_chip_info	*info;
	struct iio_dev			*indio_dev;
	struct work_struct		interrupt_handler_ws;
	s64				last_timestamp;
	int				mo_det_use_count;
	struct mutex			lock;
	int				bpse;
	u8				*tx;
	
	u8				*rx;
};


struct sca3000_chip_info {
	const char		*name;
	bool			temp_output;
	int			measurement_mode_freq;
	int			option_mode_1;
	int			option_mode_1_freq;
	int			option_mode_2;
	int			option_mode_2_freq;
};


int sca3000_read_data(struct sca3000_state *st,
		      u8 reg_address_high,
		      u8 **rx_p,
		      int len);


int sca3000_write_reg(struct sca3000_state *st, u8 address, u8 val);


static inline int sca3000_11bit_convert(uint8_t msb, uint8_t lsb)
{
	int16_t val;

	val = ((lsb >> 3) & 0x1C) | (msb << 5);
	val |= (val & (1 << 12)) ? 0xE000 : 0;

	return val;
};

static inline int sca3000_13bit_convert(uint8_t msb, uint8_t lsb)
{
	s16 val;

	val = ((lsb >> 3) & 0x1F) | (msb << 5);
	
	val |= (val & (1 << 12)) ? 0xE000 : 0;

	return val;
};


#ifdef CONFIG_IIO_RING_BUFFER

void sca3000_register_ring_funcs(struct iio_dev *indio_dev);


int sca3000_configure_ring(struct iio_dev *indio_dev);


void sca3000_unconfigure_ring(struct iio_dev *indio_dev);


void sca3000_ring_int_process(u8 val, struct iio_ring_buffer *ring);

#else
static inline void sca3000_register_ring_funcs(struct iio_dev *indio_dev) {};

static inline
int sca3000_register_ring_access_and_init(struct iio_dev *indio_dev)
{
	return 0;
};

static inline void sca3000_ring_int_process(u8 val, void *ring) {};

#endif

