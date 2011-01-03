

#include <net/cfg80211.h>
#include <net/ieee80211_radiotap.h>
#include <asm/unaligned.h>





int ieee80211_radiotap_iterator_init(
    struct ieee80211_radiotap_iterator *iterator,
    struct ieee80211_radiotap_header *radiotap_header,
    int max_length)
{
	
	if (radiotap_header->it_version)
		return -EINVAL;

	
	if (max_length < get_unaligned_le16(&radiotap_header->it_len))
		return -EINVAL;

	iterator->rtheader = radiotap_header;
	iterator->max_length = get_unaligned_le16(&radiotap_header->it_len);
	iterator->arg_index = 0;
	iterator->bitmap_shifter = get_unaligned_le32(&radiotap_header->it_present);
	iterator->arg = (u8 *)radiotap_header + sizeof(*radiotap_header);
	iterator->this_arg = NULL;

	

	if (unlikely(iterator->bitmap_shifter & (1<<IEEE80211_RADIOTAP_EXT))) {
		while (get_unaligned_le32(iterator->arg) &
		       (1 << IEEE80211_RADIOTAP_EXT)) {
			iterator->arg += sizeof(u32);

			

			if (((ulong)iterator->arg -
			     (ulong)iterator->rtheader) > iterator->max_length)
				return -EINVAL;
		}

		iterator->arg += sizeof(u32);

		
	}

	

	return 0;
}
EXPORT_SYMBOL(ieee80211_radiotap_iterator_init);




int ieee80211_radiotap_iterator_next(
    struct ieee80211_radiotap_iterator *iterator)
{

	

	static const u8 rt_sizes[] = {
		[IEEE80211_RADIOTAP_TSFT] = 0x88,
		[IEEE80211_RADIOTAP_FLAGS] = 0x11,
		[IEEE80211_RADIOTAP_RATE] = 0x11,
		[IEEE80211_RADIOTAP_CHANNEL] = 0x24,
		[IEEE80211_RADIOTAP_FHSS] = 0x22,
		[IEEE80211_RADIOTAP_DBM_ANTSIGNAL] = 0x11,
		[IEEE80211_RADIOTAP_DBM_ANTNOISE] = 0x11,
		[IEEE80211_RADIOTAP_LOCK_QUALITY] = 0x22,
		[IEEE80211_RADIOTAP_TX_ATTENUATION] = 0x22,
		[IEEE80211_RADIOTAP_DB_TX_ATTENUATION] = 0x22,
		[IEEE80211_RADIOTAP_DBM_TX_POWER] = 0x11,
		[IEEE80211_RADIOTAP_ANTENNA] = 0x11,
		[IEEE80211_RADIOTAP_DB_ANTSIGNAL] = 0x11,
		[IEEE80211_RADIOTAP_DB_ANTNOISE] = 0x11,
		[IEEE80211_RADIOTAP_RX_FLAGS] = 0x22,
		[IEEE80211_RADIOTAP_TX_FLAGS] = 0x22,
		[IEEE80211_RADIOTAP_RTS_RETRIES] = 0x11,
		[IEEE80211_RADIOTAP_DATA_RETRIES] = 0x11,
		
	};

	

	while (iterator->arg_index < sizeof(rt_sizes)) {
		int hit = 0;
		int pad;

		if (!(iterator->bitmap_shifter & 1))
			goto next_entry; 

		

		pad = (((ulong)iterator->arg) -
			((ulong)iterator->rtheader)) &
			((rt_sizes[iterator->arg_index] >> 4) - 1);

		if (pad)
			iterator->arg +=
				(rt_sizes[iterator->arg_index] >> 4) - pad;

		
		iterator->this_arg_index = iterator->arg_index;
		iterator->this_arg = iterator->arg;
		hit = 1;

		
		iterator->arg += rt_sizes[iterator->arg_index] & 0x0f;

		

		if (((ulong)iterator->arg - (ulong)iterator->rtheader) >
		    iterator->max_length)
			return -EINVAL;

	next_entry:
		iterator->arg_index++;
		if (unlikely((iterator->arg_index & 31) == 0)) {
			
			if (iterator->bitmap_shifter & 1) {
				
				
				iterator->bitmap_shifter =
				    get_unaligned_le32(iterator->next_bitmap);
				iterator->next_bitmap++;
			} else
				
				iterator->arg_index = sizeof(rt_sizes);
		} else 
			iterator->bitmap_shifter >>= 1;

		
		if (hit)
			return 0;
	}

	
	return -ENOENT;
}
EXPORT_SYMBOL(ieee80211_radiotap_iterator_next);
