

#include "../comedi.h"
#include "../comedilib.h"
#include "../comedidev.h"

#include <linux/string.h>
#include <linux/delay.h>

int comedi_data_write(void *dev, unsigned int subdev, unsigned int chan,
		      unsigned int range, unsigned int aref, unsigned int data)
{
	struct comedi_insn insn;

	memset(&insn, 0, sizeof(insn));
	insn.insn = INSN_WRITE;
	insn.n = 1;
	insn.data = &data;
	insn.subdev = subdev;
	insn.chanspec = CR_PACK(chan, range, aref);

	return comedi_do_insn(dev, &insn);
}

int comedi_data_read(void *dev, unsigned int subdev, unsigned int chan,
		     unsigned int range, unsigned int aref, unsigned int *data)
{
	struct comedi_insn insn;

	memset(&insn, 0, sizeof(insn));
	insn.insn = INSN_READ;
	insn.n = 1;
	insn.data = data;
	insn.subdev = subdev;
	insn.chanspec = CR_PACK(chan, range, aref);

	return comedi_do_insn(dev, &insn);
}

int comedi_data_read_hint(void *dev, unsigned int subdev,
			  unsigned int chan, unsigned int range,
			  unsigned int aref)
{
	struct comedi_insn insn;
	unsigned int dummy_data;

	memset(&insn, 0, sizeof(insn));
	insn.insn = INSN_READ;
	insn.n = 0;
	insn.data = &dummy_data;
	insn.subdev = subdev;
	insn.chanspec = CR_PACK(chan, range, aref);

	return comedi_do_insn(dev, &insn);
}

int comedi_data_read_delayed(void *dev, unsigned int subdev,
			     unsigned int chan, unsigned int range,
			     unsigned int aref, unsigned int *data,
			     unsigned int nano_sec)
{
	int retval;

	retval = comedi_data_read_hint(dev, subdev, chan, range, aref);
	if (retval < 0)
		return retval;

	udelay((nano_sec + 999) / 1000);

	return comedi_data_read(dev, subdev, chan, range, aref, data);
}
