
#include <linux/netdevice.h>
#include "i2400m.h"


#define D_SUBMODULE tx
#include "debug-levels.h"

enum {
	
	I2400M_TX_BUF_SIZE = 32768,
	
	I2400M_TX_PLD_MAX = 12,
	I2400M_TX_PLD_SIZE = sizeof(struct i2400m_msg_hdr)
	+ I2400M_TX_PLD_MAX * sizeof(struct i2400m_pld),
	I2400M_TX_SKIP = 0x80000000,
};

#define TAIL_FULL ((void *)~(unsigned long)NULL)


static inline
size_t __i2400m_tx_tail_room(struct i2400m *i2400m)
{
	size_t tail_room;
	size_t tx_in;

	if (unlikely(i2400m->tx_in) == 0)
		return I2400M_TX_BUF_SIZE;
	tx_in = i2400m->tx_in % I2400M_TX_BUF_SIZE;
	tail_room = I2400M_TX_BUF_SIZE - tx_in;
	tail_room %= I2400M_TX_BUF_SIZE;
	return tail_room;
}



static
void *i2400m_tx_fifo_push(struct i2400m *i2400m, size_t size, size_t padding)
{
	struct device *dev = i2400m_dev(i2400m);
	size_t room, tail_room, needed_size;
	void *ptr;

	needed_size = size + padding;
	room = I2400M_TX_BUF_SIZE - (i2400m->tx_in - i2400m->tx_out);
	if (room < needed_size)	{ 
		d_printf(2, dev, "fifo push %zu/%zu: no space\n",
			 size, padding);
		return NULL;
	}
	
	tail_room = __i2400m_tx_tail_room(i2400m);
	if (tail_room < needed_size) {
		if (i2400m->tx_out % I2400M_TX_BUF_SIZE
		    < i2400m->tx_in % I2400M_TX_BUF_SIZE) {
			d_printf(2, dev, "fifo push %zu/%zu: tail full\n",
				 size, padding);
			return TAIL_FULL;	
		} else {
			d_printf(2, dev, "fifo push %zu/%zu: no head space\n",
				 size, padding);
			return NULL;	
		}
	}
	ptr = i2400m->tx_buf + i2400m->tx_in % I2400M_TX_BUF_SIZE;
	d_printf(2, dev, "fifo push %zu/%zu: at @%zu\n", size, padding,
		 i2400m->tx_in % I2400M_TX_BUF_SIZE);
	i2400m->tx_in += size;
	return ptr;
}



static
void i2400m_tx_skip_tail(struct i2400m *i2400m)
{
	struct device *dev = i2400m_dev(i2400m);
	size_t tx_in = i2400m->tx_in % I2400M_TX_BUF_SIZE;
	size_t tail_room = __i2400m_tx_tail_room(i2400m);
	struct i2400m_msg_hdr *msg = i2400m->tx_buf + tx_in;
	if (unlikely(tail_room == 0))
		return;
	BUG_ON(tail_room < sizeof(*msg));
	msg->size = tail_room | I2400M_TX_SKIP;
	d_printf(2, dev, "skip tail: skipping %zu bytes @%zu\n",
		 tail_room, tx_in);
	i2400m->tx_in += tail_room;
}



static
unsigned i2400m_tx_fits(struct i2400m *i2400m)
{
	struct i2400m_msg_hdr *msg_hdr = i2400m->tx_msg;
	return le16_to_cpu(msg_hdr->num_pls) < I2400M_TX_PLD_MAX;

}



static
void i2400m_tx_new(struct i2400m *i2400m)
{
	struct device *dev = i2400m_dev(i2400m);
	struct i2400m_msg_hdr *tx_msg;
	BUG_ON(i2400m->tx_msg != NULL);
try_head:
	tx_msg = i2400m_tx_fifo_push(i2400m, I2400M_TX_PLD_SIZE, 0);
	if (tx_msg == NULL)
		goto out;
	else if (tx_msg == TAIL_FULL) {
		i2400m_tx_skip_tail(i2400m);
		d_printf(2, dev, "new TX message: tail full, trying head\n");
		goto try_head;
	}
	memset(tx_msg, 0, I2400M_TX_PLD_SIZE);
	tx_msg->size = I2400M_TX_PLD_SIZE;
out:
	i2400m->tx_msg = tx_msg;
	d_printf(2, dev, "new TX message: %p @%zu\n",
		 tx_msg, (void *) tx_msg - i2400m->tx_buf);
}



static
void i2400m_tx_close(struct i2400m *i2400m)
{
	struct device *dev = i2400m_dev(i2400m);
	struct i2400m_msg_hdr *tx_msg = i2400m->tx_msg;
	struct i2400m_msg_hdr *tx_msg_moved;
	size_t aligned_size, padding, hdr_size;
	void *pad_buf;
	unsigned num_pls;

	if (tx_msg->size & I2400M_TX_SKIP)	
		goto out;
	num_pls = le16_to_cpu(tx_msg->num_pls);
	
	if (num_pls == 0) {
		tx_msg->size |= I2400M_TX_SKIP;
		goto out;
	}
	
	hdr_size = sizeof(*tx_msg)
		+ le16_to_cpu(tx_msg->num_pls) * sizeof(tx_msg->pld[0]);
	hdr_size = ALIGN(hdr_size, I2400M_PL_ALIGN);
	tx_msg->offset = I2400M_TX_PLD_SIZE - hdr_size;
	tx_msg_moved = (void *) tx_msg + tx_msg->offset;
	memmove(tx_msg_moved, tx_msg, hdr_size);
	tx_msg_moved->size -= tx_msg->offset;
	
	aligned_size = ALIGN(tx_msg_moved->size, i2400m->bus_tx_block_size);
	padding = aligned_size - tx_msg_moved->size;
	if (padding > 0) {
		pad_buf = i2400m_tx_fifo_push(i2400m, padding, 0);
		if (unlikely(WARN_ON(pad_buf == NULL
				     || pad_buf == TAIL_FULL))) {
			
			dev_err(dev,
				"SW BUG! Possible data leakage from memory the "
				"device should not read for padding - "
				"size %lu aligned_size %zu tx_buf %p in "
				"%zu out %zu\n",
				(unsigned long) tx_msg_moved->size,
				aligned_size, i2400m->tx_buf, i2400m->tx_in,
				i2400m->tx_out);
		} else
			memset(pad_buf, 0xad, padding);
	}
	tx_msg_moved->padding = cpu_to_le16(padding);
	tx_msg_moved->size += padding;
	if (tx_msg != tx_msg_moved)
		tx_msg->size += padding;
out:
	i2400m->tx_msg = NULL;
}



int i2400m_tx(struct i2400m *i2400m, const void *buf, size_t buf_len,
	      enum i2400m_pt pl_type)
{
	int result = -ENOSPC;
	struct device *dev = i2400m_dev(i2400m);
	unsigned long flags;
	size_t padded_len;
	void *ptr;
	unsigned is_singleton = pl_type == I2400M_PT_RESET_WARM
		|| pl_type == I2400M_PT_RESET_COLD;

	d_fnstart(3, dev, "(i2400m %p skb %p [%zu bytes] pt %u)\n",
		  i2400m, buf, buf_len, pl_type);
	padded_len = ALIGN(buf_len, I2400M_PL_ALIGN);
	d_printf(5, dev, "padded_len %zd buf_len %zd\n", padded_len, buf_len);
	
	spin_lock_irqsave(&i2400m->tx_lock, flags);
try_new:
	if (unlikely(i2400m->tx_msg == NULL))
		i2400m_tx_new(i2400m);
	else if (unlikely(!i2400m_tx_fits(i2400m)
			  || (is_singleton && i2400m->tx_msg->num_pls != 0))) {
		d_printf(2, dev, "closing TX message (fits %u singleton "
			 "%u num_pls %u)\n", i2400m_tx_fits(i2400m),
			 is_singleton, i2400m->tx_msg->num_pls);
		i2400m_tx_close(i2400m);
		i2400m_tx_new(i2400m);
	}
	if (i2400m->tx_msg == NULL)
		goto error_tx_new;
	if (i2400m->tx_msg->size + padded_len > I2400M_TX_BUF_SIZE / 2) {
		d_printf(2, dev, "TX: message too big, going new\n");
		i2400m_tx_close(i2400m);
		i2400m_tx_new(i2400m);
	}
	if (i2400m->tx_msg == NULL)
		goto error_tx_new;
	
	ptr = i2400m_tx_fifo_push(i2400m, padded_len,
				  i2400m->bus_tx_block_size);
	if (ptr == TAIL_FULL) {	
		d_printf(2, dev, "pl append: tail full\n");
		i2400m_tx_close(i2400m);
		i2400m_tx_skip_tail(i2400m);
		goto try_new;
	} else if (ptr == NULL) {	
		result = -ENOSPC;
		d_printf(2, dev, "pl append: all full\n");
	} else {			
		struct i2400m_msg_hdr *tx_msg = i2400m->tx_msg;
		unsigned num_pls = le16_to_cpu(tx_msg->num_pls);
		memcpy(ptr, buf, buf_len);
		memset(ptr + buf_len, 0xad, padded_len - buf_len);
		i2400m_pld_set(&tx_msg->pld[num_pls], buf_len, pl_type);
		d_printf(3, dev, "pld 0x%08x (type 0x%1x len 0x%04zx\n",
			 le32_to_cpu(tx_msg->pld[num_pls].val),
			 pl_type, buf_len);
		tx_msg->num_pls = le16_to_cpu(num_pls+1);
		tx_msg->size += padded_len;
		d_printf(2, dev, "TX: appended %zu b (up to %u b) pl #%u \n",
			padded_len, tx_msg->size, num_pls+1);
		d_printf(2, dev,
			 "TX: appended hdr @%zu %zu b pl #%u @%zu %zu/%zu b\n",
			 (void *)tx_msg - i2400m->tx_buf, (size_t)tx_msg->size,
			 num_pls+1, ptr - i2400m->tx_buf, buf_len, padded_len);
		result = 0;
		if (is_singleton)
			i2400m_tx_close(i2400m);
	}
error_tx_new:
	spin_unlock_irqrestore(&i2400m->tx_lock, flags);
	i2400m->bus_tx_kick(i2400m);	
	d_fnend(3, dev, "(i2400m %p skb %p [%zu bytes] pt %u) = %d\n",
		i2400m, buf, buf_len, pl_type, result);
	return result;
}
EXPORT_SYMBOL_GPL(i2400m_tx);



struct i2400m_msg_hdr *i2400m_tx_msg_get(struct i2400m *i2400m,
					 size_t *bus_size)
{
	struct device *dev = i2400m_dev(i2400m);
	struct i2400m_msg_hdr *tx_msg, *tx_msg_moved;
	unsigned long flags, pls;

	d_fnstart(3, dev, "(i2400m %p bus_size %p)\n", i2400m, bus_size);
	spin_lock_irqsave(&i2400m->tx_lock, flags);
skip:
	tx_msg_moved = NULL;
	if (i2400m->tx_in == i2400m->tx_out) {	
		i2400m->tx_in = 0;
		i2400m->tx_out = 0;
		d_printf(2, dev, "TX: FIFO empty: resetting\n");
		goto out_unlock;
	}
	tx_msg = i2400m->tx_buf + i2400m->tx_out % I2400M_TX_BUF_SIZE;
	if (tx_msg->size & I2400M_TX_SKIP) {	
		d_printf(2, dev, "TX: skip: msg @%zu (%zu b)\n",
			 i2400m->tx_out % I2400M_TX_BUF_SIZE,
			 (size_t) tx_msg->size & ~I2400M_TX_SKIP);
		i2400m->tx_out += tx_msg->size & ~I2400M_TX_SKIP;
		goto skip;
	}

	if (tx_msg->num_pls == 0) {		
		if (tx_msg == i2400m->tx_msg) {	
			d_printf(2, dev,
				 "TX: FIFO empty: open msg w/o payloads @%zu\n",
				 (void *) tx_msg - i2400m->tx_buf);
			tx_msg = NULL;
			goto out_unlock;
		} else {			
			d_printf(2, dev,
				 "TX: skip msg w/o payloads @%zu (%zu b)\n",
				 (void *) tx_msg - i2400m->tx_buf,
				 (size_t) tx_msg->size);
			i2400m->tx_out += tx_msg->size & ~I2400M_TX_SKIP;
			goto skip;
		}
	}
	if (tx_msg == i2400m->tx_msg)		
		i2400m_tx_close(i2400m);

	
	tx_msg_moved = (void *) tx_msg + tx_msg->offset;
	i2400m->tx_msg_size = tx_msg->size;
	*bus_size = tx_msg_moved->size;
	d_printf(2, dev, "TX: pid %d msg hdr at @%zu offset +@%zu "
		 "size %zu bus_size %zu\n",
		 current->pid, (void *) tx_msg - i2400m->tx_buf,
		 (size_t) tx_msg->offset, (size_t) tx_msg->size,
		 (size_t) tx_msg_moved->size);
	tx_msg_moved->barker = le32_to_cpu(I2400M_H2D_PREVIEW_BARKER);
	tx_msg_moved->sequence = le32_to_cpu(i2400m->tx_sequence++);

	pls = le32_to_cpu(tx_msg_moved->num_pls);
	i2400m->tx_pl_num += pls;		
	if (pls > i2400m->tx_pl_max)
		i2400m->tx_pl_max = pls;
	if (pls < i2400m->tx_pl_min)
		i2400m->tx_pl_min = pls;
	i2400m->tx_num++;
	i2400m->tx_size_acc += *bus_size;
	if (*bus_size < i2400m->tx_size_min)
		i2400m->tx_size_min = *bus_size;
	if (*bus_size > i2400m->tx_size_max)
		i2400m->tx_size_max = *bus_size;
out_unlock:
	spin_unlock_irqrestore(&i2400m->tx_lock, flags);
	d_fnstart(3, dev, "(i2400m %p bus_size %p [%zu]) = %p\n",
		  i2400m, bus_size, *bus_size, tx_msg_moved);
	return tx_msg_moved;
}
EXPORT_SYMBOL_GPL(i2400m_tx_msg_get);



void i2400m_tx_msg_sent(struct i2400m *i2400m)
{
	unsigned n;
	unsigned long flags;
	struct device *dev = i2400m_dev(i2400m);

	d_fnstart(3, dev, "(i2400m %p)\n", i2400m);
	spin_lock_irqsave(&i2400m->tx_lock, flags);
	i2400m->tx_out += i2400m->tx_msg_size;
	d_printf(2, dev, "TX: sent %zu b\n", (size_t) i2400m->tx_msg_size);
	i2400m->tx_msg_size = 0;
	BUG_ON(i2400m->tx_out > i2400m->tx_in);
	
	n = i2400m->tx_out / I2400M_TX_BUF_SIZE;
	i2400m->tx_out %= I2400M_TX_BUF_SIZE;
	i2400m->tx_in -= n * I2400M_TX_BUF_SIZE;
	spin_unlock_irqrestore(&i2400m->tx_lock, flags);
	d_fnend(3, dev, "(i2400m %p) = void\n", i2400m);
}
EXPORT_SYMBOL_GPL(i2400m_tx_msg_sent);



int i2400m_tx_setup(struct i2400m *i2400m)
{
	int result;

	
	INIT_WORK(&i2400m->wake_tx_ws, i2400m_wake_tx_work);

	i2400m->tx_sequence = 0;
	i2400m->tx_buf = kmalloc(I2400M_TX_BUF_SIZE, GFP_KERNEL);
	if (i2400m->tx_buf == NULL)
		result = -ENOMEM;
	else
		result = 0;
	
	BUG_ON(i2400m->bus_tx_block_size == 0);
	return result;

}



void i2400m_tx_release(struct i2400m *i2400m)
{
	kfree(i2400m->tx_buf);
}
