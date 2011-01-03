#ifndef _RAID0_H
#define _RAID0_H

struct strip_zone
{
	sector_t zone_end;	
	sector_t dev_start;	
	int nb_dev;		
};

struct raid0_private_data
{
	struct strip_zone *strip_zone;
	mdk_rdev_t **devlist; 
	int nr_strip_zones;
};

typedef struct raid0_private_data raid0_conf_t;

#endif
