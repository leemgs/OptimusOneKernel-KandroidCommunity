#ifndef QDSP5AUDPLAYMSG_H
#define QDSP5AUDPLAYMSG_H



#define AUDPLAY_MSG_DEC_NEEDS_DATA		0x0001
#define AUDPLAY_MSG_DEC_NEEDS_DATA_MSG_LEN	\
	sizeof(audplay_msg_dec_needs_data)

typedef struct{
   
  unsigned int dec_id;           

  
  unsigned int adecDataReadPtrOffset;  

  
  unsigned int adecDataBufSize;
  
  unsigned int 	bitstream_free_len;
  unsigned int	bitstream_write_ptr;
  unsigned int	bitstarem_buf_start;
  unsigned int	bitstream_buf_len;
} __attribute__((packed)) audplay_msg_dec_needs_data;

#define AUDPLAY_UP_STREAM_INFO 0x0003
#define AUDPLAY_UP_STREAM_INFO_LEN \
  sizeof(struct audplay_msg_stream_info)

struct audplay_msg_stream_info {
  unsigned int decoder_id;
  unsigned int channel_info;
  unsigned int sample_freq;
  unsigned int bitstream_info;
  unsigned int bit_rate;
} __attribute__((packed));

#define AUDPLAY_MSG_BUFFER_UPDATE 0x0004
#define AUDPLAY_MSG_BUFFER_UPDATE_LEN \
  sizeof(struct audplay_msg_buffer_update)

struct audplay_msg_buffer_update {
  unsigned int buffer_write_count;
  unsigned int num_of_buffer;
  unsigned int buf0_address;
  unsigned int buf0_length;
  unsigned int buf1_address;
  unsigned int buf1_length;
} __attribute__((packed));

#define ADSP_MESSAGE_ID 0xFFFF
#endif 
