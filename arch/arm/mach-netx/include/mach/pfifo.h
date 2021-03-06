


#ifndef ASM_ARCH_PFIFO_H
#define ASM_ARCH_PFIFO_H

static inline int pfifo_push(int no, unsigned int pointer)
{
	writel(pointer, NETX_PFIFO_BASE(no));
	return 0;
}

static inline unsigned int pfifo_pop(int no)
{
	return readl(NETX_PFIFO_BASE(no));
}

static inline int pfifo_fill_level(int no)
{

	return readl(NETX_PFIFO_FILL_LEVEL(no));
}

static inline int pfifo_full(int no)
{
	return readl(NETX_PFIFO_FULL) & (1<<no) ? 1 : 0;
}

static inline int pfifo_empty(int no)
{
	return readl(NETX_PFIFO_EMPTY) & (1<<no) ? 1 : 0;
}

int pfifo_request(unsigned int pfifo_mask);
void pfifo_free(unsigned int pfifo_mask);

#endif 
