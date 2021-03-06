
#ifndef _ORINOCO_SCAN_H_
#define _ORINOCO_SCAN_H_


struct orinoco_private;
struct agere_ext_scan_info;


void orinoco_add_extscan_result(struct orinoco_private *priv,
				struct agere_ext_scan_info *atom,
				size_t len);
void orinoco_add_hostscan_results(struct orinoco_private *dev,
				  unsigned char *buf,
				  size_t len);

#endif 
