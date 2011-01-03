
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/mii.h>
#include <net/dst.h>

#include <asm/octeon/octeon.h>

#include "ethernet-defines.h"

#include "cvmx-fpa.h"


static int cvm_oct_fill_hw_skbuff(int pool, int size, int elements)
{
	int freed = elements;
	while (freed) {

		struct sk_buff *skb = dev_alloc_skb(size + 128);
		if (unlikely(skb == NULL)) {
			pr_warning
			    ("Failed to allocate skb for hardware pool %d\n",
			     pool);
			break;
		}

		skb_reserve(skb, 128 - (((unsigned long)skb->data) & 0x7f));
		*(struct sk_buff **)(skb->data - sizeof(void *)) = skb;
		cvmx_fpa_free(skb->data, pool, DONT_WRITEBACK(size / 128));
		freed--;
	}
	return elements - freed;
}


static void cvm_oct_free_hw_skbuff(int pool, int size, int elements)
{
	char *memory;

	do {
		memory = cvmx_fpa_alloc(pool);
		if (memory) {
			struct sk_buff *skb =
			    *(struct sk_buff **)(memory - sizeof(void *));
			elements--;
			dev_kfree_skb(skb);
		}
	} while (memory);

	if (elements < 0)
		pr_warning("Freeing of pool %u had too many skbuffs (%d)\n",
		     pool, elements);
	else if (elements > 0)
		pr_warning("Freeing of pool %u is missing %d skbuffs\n",
		       pool, elements);
}


static int cvm_oct_fill_hw_memory(int pool, int size, int elements)
{
	char *memory;
	int freed = elements;

	if (USE_32BIT_SHARED) {
		extern uint64_t octeon_reserve32_memory;

		memory =
		    cvmx_bootmem_alloc_range(elements * size, 128,
					     octeon_reserve32_memory,
					     octeon_reserve32_memory +
					     (CONFIG_CAVIUM_RESERVE32 << 20) -
					     1);
		if (memory == NULL)
			panic("Unable to allocate %u bytes for FPA pool %d\n",
			      elements * size, pool);

		pr_notice("Memory range %p - %p reserved for "
			  "hardware\n", memory,
			  memory + elements * size - 1);

		while (freed) {
			cvmx_fpa_free(memory, pool, 0);
			memory += size;
			freed--;
		}
	} else {
		while (freed) {
			
			memory = kmalloc(size + 127, GFP_ATOMIC);
			if (unlikely(memory == NULL)) {
				pr_warning("Unable to allocate %u bytes for "
					   "FPA pool %d\n",
				     elements * size, pool);
				break;
			}
			memory = (char *)(((unsigned long)memory + 127) & -128);
			cvmx_fpa_free(memory, pool, 0);
			freed--;
		}
	}
	return elements - freed;
}


static void cvm_oct_free_hw_memory(int pool, int size, int elements)
{
	if (USE_32BIT_SHARED) {
		pr_warning("Warning: 32 shared memory is not freeable\n");
	} else {
		char *memory;
		do {
			memory = cvmx_fpa_alloc(pool);
			if (memory) {
				elements--;
				kfree(phys_to_virt(cvmx_ptr_to_phys(memory)));
			}
		} while (memory);

		if (elements < 0)
			pr_warning("Freeing of pool %u had too many "
				   "buffers (%d)\n",
			       pool, elements);
		else if (elements > 0)
			pr_warning("Warning: Freeing of pool %u is "
				"missing %d buffers\n",
			     pool, elements);
	}
}

int cvm_oct_mem_fill_fpa(int pool, int size, int elements)
{
	int freed;
	if (USE_SKBUFFS_IN_HW)
		freed = cvm_oct_fill_hw_skbuff(pool, size, elements);
	else
		freed = cvm_oct_fill_hw_memory(pool, size, elements);
	return freed;
}

void cvm_oct_mem_empty_fpa(int pool, int size, int elements)
{
	if (USE_SKBUFFS_IN_HW)
		cvm_oct_free_hw_skbuff(pool, size, elements);
	else
		cvm_oct_free_hw_memory(pool, size, elements);
}
