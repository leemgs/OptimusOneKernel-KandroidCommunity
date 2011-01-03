

#ifndef _P80211META_H
#define _P80211META_H






typedef struct p80211meta {
	char *name;		
	u32 did;		
	u32 flags;		
	u32 min;		
	u32 max;		

	u32 maxlen;		
	u32 minlen;		
	p80211enum_t *enumptr;	
	p80211_totext_t totextptr;	
	p80211_fromtext_t fromtextptr;	
	p80211_valid_t validfunptr;	
} p80211meta_t;

typedef struct grplistitem {
	char *name;
	p80211meta_t *itemlist;
} grplistitem_t;

typedef struct catlistitem {
	char *name;
	grplistitem_t *grplist;
} catlistitem_t;

#endif 
