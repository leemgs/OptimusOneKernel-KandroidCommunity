

#ifndef MIDI_H
#define MIDI_H


#include <sound/rawmidi.h>

#include "midibuf.h"


#define MIDI_BUFFER_SIZE 1024


struct snd_line6_midi {
	
	struct usb_line6 *line6;

	
	struct snd_rawmidi_substream *substream_receive;

	
	struct snd_rawmidi_substream *substream_transmit;

	
	int num_active_send_urbs;

	
	spinlock_t send_urb_lock;

	
	spinlock_t midi_transmit_lock;

	
	wait_queue_head_t send_wait;

	
	int midi_mask_transmit;

	
	int midi_mask_receive;

	
	struct MidiBuffer midibuf_in;

	
	struct MidiBuffer midibuf_out;
};


extern int line6_init_midi(struct usb_line6 *line6);
extern void line6_midi_receive(struct usb_line6 *line6, unsigned char *data,
			       int length);


#endif
