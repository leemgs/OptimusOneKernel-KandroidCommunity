

#include "../comedi.h"
#include "../comedilib.h"
#include "../comedidev.h"

#include <linux/module.h>

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/slab.h>



EXPORT_SYMBOL(comedi_register_callback);
EXPORT_SYMBOL(comedi_get_krange);
EXPORT_SYMBOL(comedi_get_buf_head_pos);
EXPORT_SYMBOL(comedi_set_user_int_count);
EXPORT_SYMBOL(comedi_map);
EXPORT_SYMBOL(comedi_unmap);



EXPORT_SYMBOL(comedi_open);
EXPORT_SYMBOL(comedi_close);


EXPORT_SYMBOL(comedi_loglevel);
EXPORT_SYMBOL(comedi_perror);
EXPORT_SYMBOL(comedi_strerror);

EXPORT_SYMBOL(comedi_fileno);


EXPORT_SYMBOL(comedi_get_n_subdevices);
EXPORT_SYMBOL(comedi_get_version_code);
EXPORT_SYMBOL(comedi_get_driver_name);
EXPORT_SYMBOL(comedi_get_board_name);


EXPORT_SYMBOL(comedi_get_subdevice_type);
EXPORT_SYMBOL(comedi_find_subdevice_by_type);
EXPORT_SYMBOL(comedi_get_subdevice_flags);
EXPORT_SYMBOL(comedi_get_n_channels);



EXPORT_SYMBOL(comedi_get_maxdata);
#ifdef KCOMEDILIB_DEPRECATED
EXPORT_SYMBOL(comedi_get_rangetype);
#endif
EXPORT_SYMBOL(comedi_get_n_ranges);



EXPORT_SYMBOL(comedi_get_buffer_size);

EXPORT_SYMBOL(comedi_get_buffer_contents);
EXPORT_SYMBOL(comedi_get_buffer_offset);



EXPORT_SYMBOL(comedi_do_insn);
EXPORT_SYMBOL(comedi_lock);
EXPORT_SYMBOL(comedi_unlock);





EXPORT_SYMBOL(comedi_data_read);
EXPORT_SYMBOL(comedi_data_read_hint);
EXPORT_SYMBOL(comedi_data_read_delayed);
EXPORT_SYMBOL(comedi_data_write);
EXPORT_SYMBOL(comedi_dio_config);
EXPORT_SYMBOL(comedi_dio_read);
EXPORT_SYMBOL(comedi_dio_write);
EXPORT_SYMBOL(comedi_dio_bitfield);






EXPORT_SYMBOL(comedi_cancel);
EXPORT_SYMBOL(comedi_command);
EXPORT_SYMBOL(comedi_command_test);
EXPORT_SYMBOL(comedi_poll);


EXPORT_SYMBOL(comedi_mark_buffer_read);
EXPORT_SYMBOL(comedi_mark_buffer_written);


EXPORT_SYMBOL(comedi_get_len_chanlist);






