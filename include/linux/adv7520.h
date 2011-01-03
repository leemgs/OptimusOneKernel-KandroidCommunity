

#ifndef _ADV7520_H_
#define _ADV7520_H_
#define ADV7520_DRV_NAME 		"adv7520"

#define HDMI_XRES			1280
#define HDMI_YRES			720
#define HDMI_PIXCLOCK_MAX		74250
#define ADV7520_EDIDI2CSLAVEADDRESS   	0xA0

#define DEBUG  0


#define ADV7520_AUDIO_CTS_20BIT_N   6272


#define HDMI_EDID_MAX_LENGTH    256


#define HDMI_EDID_MAX_DTDS      4




struct hdmi_edid_dtd_video {
	u8   pixel_clock[2];          
	u8   horiz_active;            
	u8   horiz_blanking;          
	u8   horiz_high;              
	u8   vert_active;             
	u8   vert_blanking;           
	u8   vert_high;               
	u8   horiz_sync_offset;       
	u8   horiz_sync_pulse;        
	u8   vert_sync_pulse;         
	u8   sync_pulse_high;         
	u8   horiz_image_size;        
	u8   vert_image_size;	      
	u8   image_size_high;         
	u8   horiz_border;            
	u8   vert_border;             
	u8   misc_settings;           
} ;


struct hdmi_edid {			
	u8   edid_header[8];            
	u8   manufacturerID[2];      	
	u8   product_id[2];           	
	u8   serial_number[4];        	
	u8   week_manufactured;       	
	u8   year_manufactured;       	
	u8   edid_version;            	
	u8   edid_revision;           	

	u8   video_in_definition;      	
	u8   max_horiz_image_size;      
	u8   max_vert_image_size;       
	u8   display_gamma;           	
	u8   power_features;          	
	u8   chroma_info[10];         	
	u8   timing_1;                	
	u8   timing_2;               	
	u8   timing_3;              	
	u8   std_timings[16];         	

	struct hdmi_edid_dtd_video dtd[4];   

	u8   extension_edid;          	
	u8   checksum;               	

	u8   extension_tag;           	
	u8   extention_rev;           	
	u8   offset_dtd;              	
	u8   num_dtd;                 	

	u8   data_block[123];         	
	u8   extension_checksum;      	
} ;

#endif
