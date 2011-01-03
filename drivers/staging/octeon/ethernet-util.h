

#define DEBUGPRINT(format, ...) do { if (printk_ratelimit()) 		\
					printk(format, ##__VA_ARGS__);	\
				} while (0)


static inline void *cvm_oct_get_buffer_ptr(union cvmx_buf_ptr packet_ptr)
{
	return cvmx_phys_to_ptr(((packet_ptr.s.addr >> 7) - packet_ptr.s.back)
				<< 7);
}


static inline int INTERFACE(int ipd_port)
{
	if (ipd_port < 32)	
		return ipd_port >> 4;
	else if (ipd_port < 36)	
		return 2;
	else if (ipd_port < 40)	
		return 3;
	else if (ipd_port == 40)	
		return 4;
	else
		panic("Illegal ipd_port %d passed to INTERFACE\n", ipd_port);
}


static inline int INDEX(int ipd_port)
{
	if (ipd_port < 32)
		return ipd_port & 15;
	else
		return ipd_port & 3;
}
