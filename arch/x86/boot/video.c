



#include "boot.h"
#include "video.h"
#include "vesa.h"

static void store_cursor_position(void)
{
	struct biosregs ireg, oreg;

	initregs(&ireg);
	ireg.ah = 0x03;
	intcall(0x10, &ireg, &oreg);

	boot_params.screen_info.orig_x = oreg.dl;
	boot_params.screen_info.orig_y = oreg.dh;
}

static void store_video_mode(void)
{
	struct biosregs ireg, oreg;

	
	initregs(&ireg);
	ireg.ah = 0x0f;
	intcall(0x10, &ireg, &oreg);

	
	boot_params.screen_info.orig_video_mode = oreg.al & 0x7f;
	boot_params.screen_info.orig_video_page = oreg.bh;
}


static void store_mode_params(void)
{
	u16 font_size;
	int x, y;

	
	if (graphic_mode)
		return;

	store_cursor_position();
	store_video_mode();

	if (boot_params.screen_info.orig_video_mode == 0x07) {
		
		video_segment = 0xb000;
	} else {
		
		video_segment = 0xb800;
	}

	set_fs(0);
	font_size = rdfs16(0x485); 
	boot_params.screen_info.orig_video_points = font_size;

	x = rdfs16(0x44a);
	y = (adapter == ADAPTER_CGA) ? 25 : rdfs8(0x484)+1;

	if (force_x)
		x = force_x;
	if (force_y)
		y = force_y;

	boot_params.screen_info.orig_video_cols  = x;
	boot_params.screen_info.orig_video_lines = y;
}

static unsigned int get_entry(void)
{
	char entry_buf[4];
	int i, len = 0;
	int key;
	unsigned int v;

	do {
		key = getchar();

		if (key == '\b') {
			if (len > 0) {
				puts("\b \b");
				len--;
			}
		} else if ((key >= '0' && key <= '9') ||
			   (key >= 'A' && key <= 'Z') ||
			   (key >= 'a' && key <= 'z')) {
			if (len < sizeof entry_buf) {
				entry_buf[len++] = key;
				putchar(key);
			}
		}
	} while (key != '\r');
	putchar('\n');

	if (len == 0)
		return VIDEO_CURRENT_MODE; 

	v = 0;
	for (i = 0; i < len; i++) {
		v <<= 4;
		key = entry_buf[i] | 0x20;
		v += (key > '9') ? key-'a'+10 : key-'0';
	}

	return v;
}

static void display_menu(void)
{
	struct card_info *card;
	struct mode_info *mi;
	char ch;
	int i;
	int nmodes;
	int modes_per_line;
	int col;

	nmodes = 0;
	for (card = video_cards; card < video_cards_end; card++)
		nmodes += card->nmodes;

	modes_per_line = 1;
	if (nmodes >= 20)
		modes_per_line = 3;

	for (col = 0; col < modes_per_line; col++)
		puts("Mode: Resolution:  Type: ");
	putchar('\n');

	col = 0;
	ch = '0';
	for (card = video_cards; card < video_cards_end; card++) {
		mi = card->modes;
		for (i = 0; i < card->nmodes; i++, mi++) {
			char resbuf[32];
			int visible = mi->x && mi->y;
			u16 mode_id = mi->mode ? mi->mode :
				(mi->y << 8)+mi->x;

			if (!visible)
				continue; 

			if (mi->depth)
				sprintf(resbuf, "%dx%d", mi->y, mi->depth);
			else
				sprintf(resbuf, "%d", mi->y);

			printf("%c %03X %4dx%-7s %-6s",
			       ch, mode_id, mi->x, resbuf, card->card_name);
			col++;
			if (col >= modes_per_line) {
				putchar('\n');
				col = 0;
			}

			if (ch == '9')
				ch = 'a';
			else if (ch == 'z' || ch == ' ')
				ch = ' '; 
			else
				ch++;
		}
	}
	if (col)
		putchar('\n');
}

#define H(x)	((x)-'a'+10)
#define SCAN	((H('s')<<12)+(H('c')<<8)+(H('a')<<4)+H('n'))

static unsigned int mode_menu(void)
{
	int key;
	unsigned int sel;

	puts("Press <ENTER> to see video modes available, "
	     "<SPACE> to continue, or wait 30 sec\n");

	kbd_flush();
	while (1) {
		key = getchar_timeout();
		if (key == ' ' || key == 0)
			return VIDEO_CURRENT_MODE; 
		if (key == '\r')
			break;
		putchar('\a');	
	}


	for (;;) {
		display_menu();

		puts("Enter a video mode or \"scan\" to scan for "
		     "additional modes: ");
		sel = get_entry();
		if (sel != SCAN)
			return sel;

		probe_cards(1);
	}
}


static struct saved_screen {
	int x, y;
	int curx, cury;
	u16 *data;
} saved;

static void save_screen(void)
{
	
	saved.x = boot_params.screen_info.orig_video_cols;
	saved.y = boot_params.screen_info.orig_video_lines;
	saved.curx = boot_params.screen_info.orig_x;
	saved.cury = boot_params.screen_info.orig_y;

	if (!heap_free(saved.x*saved.y*sizeof(u16)+512))
		return;		

	saved.data = GET_HEAP(u16, saved.x*saved.y);

	set_fs(video_segment);
	copy_from_fs(saved.data, 0, saved.x*saved.y*sizeof(u16));
}

static void restore_screen(void)
{
	
	int xs = boot_params.screen_info.orig_video_cols;
	int ys = boot_params.screen_info.orig_video_lines;
	int y;
	addr_t dst = 0;
	u16 *src = saved.data;
	struct biosregs ireg;

	if (graphic_mode)
		return;		

	if (!src)
		return;		

	

	set_fs(video_segment);
	for (y = 0; y < ys; y++) {
		int npad;

		if (y < saved.y) {
			int copy = (xs < saved.x) ? xs : saved.x;
			copy_to_fs(dst, src, copy*sizeof(u16));
			dst += copy*sizeof(u16);
			src += saved.x;
			npad = (xs < saved.x) ? 0 : xs-saved.x;
		} else {
			npad = xs;
		}

		
		asm volatile("pushw %%es ; "
			     "movw %2,%%es ; "
			     "shrw %%cx ; "
			     "jnc 1f ; "
			     "stosw \n\t"
			     "1: rep;stosl ; "
			     "popw %%es"
			     : "+D" (dst), "+c" (npad)
			     : "bdS" (video_segment),
			       "a" (0x07200720));
	}

	
	initregs(&ireg);
	ireg.ah = 0x02;		
	ireg.dh = saved.cury;
	ireg.dl = saved.curx;
	intcall(0x10, &ireg, NULL);
}

void set_video(void)
{
	u16 mode = boot_params.hdr.vid_mode;

	RESET_HEAP();

	store_mode_params();
	save_screen();
	probe_cards(0);

	for (;;) {
		if (mode == ASK_VGA)
			mode = mode_menu();

		if (!set_mode(mode))
			break;

		printf("Undefined video mode number: %x\n", mode);
		mode = ASK_VGA;
	}
	boot_params.hdr.vid_mode = mode;
	vesa_store_edid();
	store_mode_params();

	if (do_restore)
		restore_screen();
}
