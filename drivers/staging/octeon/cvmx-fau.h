



#ifndef __CVMX_FAU_H__
#define __CVMX_FAU_H__



#define CVMX_FAU_LOAD_IO_ADDRESS    cvmx_build_io_address(0x1e, 0)
#define CVMX_FAU_BITS_SCRADDR       63, 56
#define CVMX_FAU_BITS_LEN           55, 48
#define CVMX_FAU_BITS_INEVAL        35, 14
#define CVMX_FAU_BITS_TAGWAIT       13, 13
#define CVMX_FAU_BITS_NOADD         13, 13
#define CVMX_FAU_BITS_SIZE          12, 11
#define CVMX_FAU_BITS_REGISTER      10, 0

typedef enum {
	CVMX_FAU_OP_SIZE_8 = 0,
	CVMX_FAU_OP_SIZE_16 = 1,
	CVMX_FAU_OP_SIZE_32 = 2,
	CVMX_FAU_OP_SIZE_64 = 3
} cvmx_fau_op_size_t;


typedef struct {
	uint64_t error:1;
	int64_t value:63;
} cvmx_fau_tagwait64_t;


typedef struct {
	uint64_t error:1;
	int32_t value:31;
} cvmx_fau_tagwait32_t;


typedef struct {
	uint64_t error:1;
	int16_t value:15;
} cvmx_fau_tagwait16_t;


typedef struct {
	uint64_t error:1;
	int8_t value:7;
} cvmx_fau_tagwait8_t;


typedef union {
	uint64_t u64;
	struct {
		uint64_t invalid:1;
		uint64_t data:63;	
	} s;
} cvmx_fau_async_tagwait_result_t;


static inline uint64_t __cvmx_fau_store_address(uint64_t noadd, uint64_t reg)
{
	return CVMX_ADD_IO_SEG(CVMX_FAU_LOAD_IO_ADDRESS) |
	       cvmx_build_bits(CVMX_FAU_BITS_NOADD, noadd) |
	       cvmx_build_bits(CVMX_FAU_BITS_REGISTER, reg);
}


static inline uint64_t __cvmx_fau_atomic_address(uint64_t tagwait, uint64_t reg,
						 int64_t value)
{
	return CVMX_ADD_IO_SEG(CVMX_FAU_LOAD_IO_ADDRESS) |
	       cvmx_build_bits(CVMX_FAU_BITS_INEVAL, value) |
	       cvmx_build_bits(CVMX_FAU_BITS_TAGWAIT, tagwait) |
	       cvmx_build_bits(CVMX_FAU_BITS_REGISTER, reg);
}


static inline int64_t cvmx_fau_fetch_and_add64(cvmx_fau_reg_64_t reg,
					       int64_t value)
{
	return cvmx_read64_int64(__cvmx_fau_atomic_address(0, reg, value));
}


static inline int32_t cvmx_fau_fetch_and_add32(cvmx_fau_reg_32_t reg,
					       int32_t value)
{
	return cvmx_read64_int32(__cvmx_fau_atomic_address(0, reg, value));
}


static inline int16_t cvmx_fau_fetch_and_add16(cvmx_fau_reg_16_t reg,
					       int16_t value)
{
	return cvmx_read64_int16(__cvmx_fau_atomic_address(0, reg, value));
}


static inline int8_t cvmx_fau_fetch_and_add8(cvmx_fau_reg_8_t reg, int8_t value)
{
	return cvmx_read64_int8(__cvmx_fau_atomic_address(0, reg, value));
}


static inline cvmx_fau_tagwait64_t
cvmx_fau_tagwait_fetch_and_add64(cvmx_fau_reg_64_t reg, int64_t value)
{
	union {
		uint64_t i64;
		cvmx_fau_tagwait64_t t;
	} result;
	result.i64 =
	    cvmx_read64_int64(__cvmx_fau_atomic_address(1, reg, value));
	return result.t;
}


static inline cvmx_fau_tagwait32_t
cvmx_fau_tagwait_fetch_and_add32(cvmx_fau_reg_32_t reg, int32_t value)
{
	union {
		uint64_t i32;
		cvmx_fau_tagwait32_t t;
	} result;
	result.i32 =
	    cvmx_read64_int32(__cvmx_fau_atomic_address(1, reg, value));
	return result.t;
}


static inline cvmx_fau_tagwait16_t
cvmx_fau_tagwait_fetch_and_add16(cvmx_fau_reg_16_t reg, int16_t value)
{
	union {
		uint64_t i16;
		cvmx_fau_tagwait16_t t;
	} result;
	result.i16 =
	    cvmx_read64_int16(__cvmx_fau_atomic_address(1, reg, value));
	return result.t;
}


static inline cvmx_fau_tagwait8_t
cvmx_fau_tagwait_fetch_and_add8(cvmx_fau_reg_8_t reg, int8_t value)
{
	union {
		uint64_t i8;
		cvmx_fau_tagwait8_t t;
	} result;
	result.i8 = cvmx_read64_int8(__cvmx_fau_atomic_address(1, reg, value));
	return result.t;
}


static inline uint64_t __cvmx_fau_iobdma_data(uint64_t scraddr, int64_t value,
					      uint64_t tagwait,
					      cvmx_fau_op_size_t size,
					      uint64_t reg)
{
	return CVMX_FAU_LOAD_IO_ADDRESS |
	       cvmx_build_bits(CVMX_FAU_BITS_SCRADDR, scraddr >> 3) |
	       cvmx_build_bits(CVMX_FAU_BITS_LEN, 1) |
	       cvmx_build_bits(CVMX_FAU_BITS_INEVAL, value) |
	       cvmx_build_bits(CVMX_FAU_BITS_TAGWAIT, tagwait) |
	       cvmx_build_bits(CVMX_FAU_BITS_SIZE, size) |
	       cvmx_build_bits(CVMX_FAU_BITS_REGISTER, reg);
}


static inline void cvmx_fau_async_fetch_and_add64(uint64_t scraddr,
						  cvmx_fau_reg_64_t reg,
						  int64_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data
			 (scraddr, value, 0, CVMX_FAU_OP_SIZE_64, reg));
}


static inline void cvmx_fau_async_fetch_and_add32(uint64_t scraddr,
						  cvmx_fau_reg_32_t reg,
						  int32_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data
			 (scraddr, value, 0, CVMX_FAU_OP_SIZE_32, reg));
}


static inline void cvmx_fau_async_fetch_and_add16(uint64_t scraddr,
						  cvmx_fau_reg_16_t reg,
						  int16_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data
			 (scraddr, value, 0, CVMX_FAU_OP_SIZE_16, reg));
}


static inline void cvmx_fau_async_fetch_and_add8(uint64_t scraddr,
						 cvmx_fau_reg_8_t reg,
						 int8_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data
			 (scraddr, value, 0, CVMX_FAU_OP_SIZE_8, reg));
}


static inline void cvmx_fau_async_tagwait_fetch_and_add64(uint64_t scraddr,
							  cvmx_fau_reg_64_t reg,
							  int64_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data
			 (scraddr, value, 1, CVMX_FAU_OP_SIZE_64, reg));
}


static inline void cvmx_fau_async_tagwait_fetch_and_add32(uint64_t scraddr,
							  cvmx_fau_reg_32_t reg,
							  int32_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data
			 (scraddr, value, 1, CVMX_FAU_OP_SIZE_32, reg));
}


static inline void cvmx_fau_async_tagwait_fetch_and_add16(uint64_t scraddr,
							  cvmx_fau_reg_16_t reg,
							  int16_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data
			 (scraddr, value, 1, CVMX_FAU_OP_SIZE_16, reg));
}


static inline void cvmx_fau_async_tagwait_fetch_and_add8(uint64_t scraddr,
							 cvmx_fau_reg_8_t reg,
							 int8_t value)
{
	cvmx_send_single(__cvmx_fau_iobdma_data
			 (scraddr, value, 1, CVMX_FAU_OP_SIZE_8, reg));
}


static inline void cvmx_fau_atomic_add64(cvmx_fau_reg_64_t reg, int64_t value)
{
	cvmx_write64_int64(__cvmx_fau_store_address(0, reg), value);
}


static inline void cvmx_fau_atomic_add32(cvmx_fau_reg_32_t reg, int32_t value)
{
	cvmx_write64_int32(__cvmx_fau_store_address(0, reg), value);
}


static inline void cvmx_fau_atomic_add16(cvmx_fau_reg_16_t reg, int16_t value)
{
	cvmx_write64_int16(__cvmx_fau_store_address(0, reg), value);
}


static inline void cvmx_fau_atomic_add8(cvmx_fau_reg_8_t reg, int8_t value)
{
	cvmx_write64_int8(__cvmx_fau_store_address(0, reg), value);
}


static inline void cvmx_fau_atomic_write64(cvmx_fau_reg_64_t reg, int64_t value)
{
	cvmx_write64_int64(__cvmx_fau_store_address(1, reg), value);
}


static inline void cvmx_fau_atomic_write32(cvmx_fau_reg_32_t reg, int32_t value)
{
	cvmx_write64_int32(__cvmx_fau_store_address(1, reg), value);
}


static inline void cvmx_fau_atomic_write16(cvmx_fau_reg_16_t reg, int16_t value)
{
	cvmx_write64_int16(__cvmx_fau_store_address(1, reg), value);
}


static inline void cvmx_fau_atomic_write8(cvmx_fau_reg_8_t reg, int8_t value)
{
	cvmx_write64_int8(__cvmx_fau_store_address(1, reg), value);
}

#endif 
