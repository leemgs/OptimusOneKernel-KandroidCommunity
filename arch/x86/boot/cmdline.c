



#include "boot.h"

static inline int myisspace(u8 c)
{
	return c <= ' ';	
}


int cmdline_find_option(const char *option, char *buffer, int bufsize)
{
	u32 cmdline_ptr = boot_params.hdr.cmd_line_ptr;
	addr_t cptr;
	char c;
	int len = -1;
	const char *opptr = NULL;
	char *bufptr = buffer;
	enum {
		st_wordstart,	
		st_wordcmp,	
		st_wordskip,	
		st_bufcpy	
	} state = st_wordstart;

	if (!cmdline_ptr || cmdline_ptr >= 0x100000)
		return -1;	

	cptr = cmdline_ptr & 0xf;
	set_fs(cmdline_ptr >> 4);

	while (cptr < 0x10000 && (c = rdfs8(cptr++))) {
		switch (state) {
		case st_wordstart:
			if (myisspace(c))
				break;

			
			state = st_wordcmp;
			opptr = option;
			

		case st_wordcmp:
			if (c == '=' && !*opptr) {
				len = 0;
				bufptr = buffer;
				state = st_bufcpy;
			} else if (myisspace(c)) {
				state = st_wordstart;
			} else if (c != *opptr++) {
				state = st_wordskip;
			}
			break;

		case st_wordskip:
			if (myisspace(c))
				state = st_wordstart;
			break;

		case st_bufcpy:
			if (myisspace(c)) {
				state = st_wordstart;
			} else {
				if (len < bufsize-1)
					*bufptr++ = c;
				len++;
			}
			break;
		}
	}

	if (bufsize)
		*bufptr = '\0';

	return len;
}


int cmdline_find_option_bool(const char *option)
{
	u32 cmdline_ptr = boot_params.hdr.cmd_line_ptr;
	addr_t cptr;
	char c;
	int pos = 0, wstart = 0;
	const char *opptr = NULL;
	enum {
		st_wordstart,	
		st_wordcmp,	
		st_wordskip,	
	} state = st_wordstart;

	if (!cmdline_ptr || cmdline_ptr >= 0x100000)
		return -1;	

	cptr = cmdline_ptr & 0xf;
	set_fs(cmdline_ptr >> 4);

	while (cptr < 0x10000) {
		c = rdfs8(cptr++);
		pos++;

		switch (state) {
		case st_wordstart:
			if (!c)
				return 0;
			else if (myisspace(c))
				break;

			state = st_wordcmp;
			opptr = option;
			wstart = pos;
			

		case st_wordcmp:
			if (!*opptr)
				if (!c || myisspace(c))
					return wstart;
				else
					state = st_wordskip;
			else if (!c)
				return 0;
			else if (c != *opptr++)
				state = st_wordskip;
			break;

		case st_wordskip:
			if (!c)
				return 0;
			else if (myisspace(c))
				state = st_wordstart;
			break;
		}
	}

	return 0;	
}
