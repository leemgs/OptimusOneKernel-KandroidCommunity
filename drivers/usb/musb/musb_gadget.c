

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/dma-mapping.h>

#include "musb_core.h"







void musb_g_giveback(
	struct musb_ep		*ep,
	struct usb_request	*request,
	int			status)
__releases(ep->musb->lock)
__acquires(ep->musb->lock)
{
	struct musb_request	*req;
	struct musb		*musb;
	int			busy = ep->busy;

	req = to_musb_request(request);

	list_del(&request->list);
	if (req->request.status == -EINPROGRESS)
		req->request.status = status;
	musb = req->musb;

	ep->busy = 1;
	spin_unlock(&musb->lock);
	if (is_dma_capable()) {
		if (req->mapped) {
			dma_unmap_single(musb->controller,
					req->request.dma,
					req->request.length,
					req->tx
						? DMA_TO_DEVICE
						: DMA_FROM_DEVICE);
			req->request.dma = DMA_ADDR_INVALID;
			req->mapped = 0;
		} else if (req->request.dma != DMA_ADDR_INVALID)
			dma_sync_single_for_cpu(musb->controller,
					req->request.dma,
					req->request.length,
					req->tx
						? DMA_TO_DEVICE
						: DMA_FROM_DEVICE);
	}
	if (request->status == 0)
		DBG(5, "%s done request %p,  %d/%d\n",
				ep->end_point.name, request,
				req->request.actual, req->request.length);
	else
		DBG(2, "%s request %p, %d/%d fault %d\n",
				ep->end_point.name, request,
				req->request.actual, req->request.length,
				request->status);
	req->request.complete(&req->ep->end_point, &req->request);
	spin_lock(&musb->lock);
	ep->busy = busy;
}




static void nuke(struct musb_ep *ep, const int status)
{
	struct musb_request	*req = NULL;
	void __iomem *epio = ep->musb->endpoints[ep->current_epnum].regs;

	ep->busy = 1;

	if (is_dma_capable() && ep->dma) {
		struct dma_controller	*c = ep->musb->dma_controller;
		int value;

		if (ep->is_in) {
			
			musb_writew(epio, MUSB_TXCSR,
				    MUSB_TXCSR_DMAMODE | MUSB_TXCSR_FLUSHFIFO);
			musb_writew(epio, MUSB_TXCSR,
					0 | MUSB_TXCSR_FLUSHFIFO);
		} else {
			musb_writew(epio, MUSB_RXCSR,
					0 | MUSB_RXCSR_FLUSHFIFO);
			musb_writew(epio, MUSB_RXCSR,
					0 | MUSB_RXCSR_FLUSHFIFO);
		}

		value = c->channel_abort(ep->dma);
		DBG(value ? 1 : 6, "%s: abort DMA --> %d\n", ep->name, value);
		c->channel_release(ep->dma);
		ep->dma = NULL;
	}

	while (!list_empty(&(ep->req_list))) {
		req = container_of(ep->req_list.next, struct musb_request,
				request.list);
		musb_g_giveback(ep, &req->request, status);
	}
}







static inline int max_ep_writesize(struct musb *musb, struct musb_ep *ep)
{
	if (can_bulk_split(musb, ep->type))
		return ep->hw_ep->max_packet_sz_tx;
	else
		return ep->packet_sz;
}


#ifdef CONFIG_USB_INVENTRA_DMA



#endif


static void txstate(struct musb *musb, struct musb_request *req)
{
	u8			epnum = req->epnum;
	struct musb_ep		*musb_ep;
	void __iomem		*epio = musb->endpoints[epnum].regs;
	struct usb_request	*request;
	u16			fifo_count = 0, csr;
	int			use_dma = 0;

	musb_ep = req->ep;

	
	if (dma_channel_status(musb_ep->dma) == MUSB_DMA_STATUS_BUSY) {
		DBG(4, "dma pending...\n");
		return;
	}

	
	csr = musb_readw(epio, MUSB_TXCSR);

	request = &req->request;
	fifo_count = min(max_ep_writesize(musb, musb_ep),
			(int)(request->length - request->actual));

	if (csr & MUSB_TXCSR_TXPKTRDY) {
		DBG(5, "%s old packet still ready , txcsr %03x\n",
				musb_ep->end_point.name, csr);
		return;
	}

	if (csr & MUSB_TXCSR_P_SENDSTALL) {
		DBG(5, "%s stalling, txcsr %03x\n",
				musb_ep->end_point.name, csr);
		return;
	}

	DBG(4, "hw_ep%d, maxpacket %d, fifo count %d, txcsr %03x\n",
			epnum, musb_ep->packet_sz, fifo_count,
			csr);

#ifndef	CONFIG_MUSB_PIO_ONLY
	if (is_dma_capable() && musb_ep->dma) {
		struct dma_controller	*c = musb->dma_controller;

		use_dma = (request->dma != DMA_ADDR_INVALID);

		

#ifdef CONFIG_USB_INVENTRA_DMA
		{
			size_t request_size;

			
			request_size = min(request->length,
						musb_ep->dma->max_len);
			if (request_size < musb_ep->packet_sz)
				musb_ep->dma->desired_mode = 0;
			else
				musb_ep->dma->desired_mode = 1;

			use_dma = use_dma && c->channel_program(
					musb_ep->dma, musb_ep->packet_sz,
					musb_ep->dma->desired_mode,
					request->dma, request_size);
			if (use_dma) {
				if (musb_ep->dma->desired_mode == 0) {
					
					csr &= ~(MUSB_TXCSR_AUTOSET
						| MUSB_TXCSR_DMAENAB);
					musb_writew(epio, MUSB_TXCSR, csr
						| MUSB_TXCSR_P_WZC_BITS);
					csr &= ~MUSB_TXCSR_DMAMODE;
					csr |= (MUSB_TXCSR_DMAENAB |
							MUSB_TXCSR_MODE);
					
				} else
					csr |= (MUSB_TXCSR_AUTOSET
							| MUSB_TXCSR_DMAENAB
							| MUSB_TXCSR_DMAMODE
							| MUSB_TXCSR_MODE);

				csr &= ~MUSB_TXCSR_P_UNDERRUN;
				musb_writew(epio, MUSB_TXCSR, csr);
			}
		}

#elif defined(CONFIG_USB_TI_CPPI_DMA)
		
		csr &= ~(MUSB_TXCSR_P_UNDERRUN | MUSB_TXCSR_TXPKTRDY);
		csr |= MUSB_TXCSR_DMAENAB | MUSB_TXCSR_DMAMODE |
		       MUSB_TXCSR_MODE;
		musb_writew(epio, MUSB_TXCSR,
			(MUSB_TXCSR_P_WZC_BITS & ~MUSB_TXCSR_P_UNDERRUN)
				| csr);

		
		csr = musb_readw(epio, MUSB_TXCSR);

		

		
		use_dma = use_dma && c->channel_program(
				musb_ep->dma, musb_ep->packet_sz,
				0,
				request->dma,
				request->length);
		if (!use_dma) {
			c->channel_release(musb_ep->dma);
			musb_ep->dma = NULL;
			csr &= ~MUSB_TXCSR_DMAENAB;
			musb_writew(epio, MUSB_TXCSR, csr);
			
		}
#elif defined(CONFIG_USB_TUSB_OMAP_DMA)
		use_dma = use_dma && c->channel_program(
				musb_ep->dma, musb_ep->packet_sz,
				request->zero,
				request->dma,
				request->length);
#endif
	}
#endif

	if (!use_dma) {
		musb_write_fifo(musb_ep->hw_ep, fifo_count,
				(u8 *) (request->buf + request->actual));
		request->actual += fifo_count;
		csr |= MUSB_TXCSR_TXPKTRDY;
		csr &= ~MUSB_TXCSR_P_UNDERRUN;
		musb_writew(epio, MUSB_TXCSR, csr);
	}

	
	DBG(3, "%s TX/IN %s len %d/%d, txcsr %04x, fifo %d/%d\n",
			musb_ep->end_point.name, use_dma ? "dma" : "pio",
			request->actual, request->length,
			musb_readw(epio, MUSB_TXCSR),
			fifo_count,
			musb_readw(epio, MUSB_TXMAXP));
}


void musb_g_tx(struct musb *musb, u8 epnum)
{
	u16			csr;
	struct usb_request	*request;
	u8 __iomem		*mbase = musb->mregs;
	struct musb_ep		*musb_ep = &musb->endpoints[epnum].ep_in;
	void __iomem		*epio = musb->endpoints[epnum].regs;
	struct dma_channel	*dma;

	musb_ep_select(mbase, epnum);
	request = next_request(musb_ep);

	csr = musb_readw(epio, MUSB_TXCSR);
	DBG(4, "<== %s, txcsr %04x\n", musb_ep->end_point.name, csr);

	dma = is_dma_capable() ? musb_ep->dma : NULL;
	do {
		
		if (csr & MUSB_TXCSR_P_SENTSTALL) {
			csr |= MUSB_TXCSR_P_WZC_BITS;
			csr &= ~MUSB_TXCSR_P_SENTSTALL;
			musb_writew(epio, MUSB_TXCSR, csr);
			break;
		}

		if (csr & MUSB_TXCSR_P_UNDERRUN) {
			
			csr |= MUSB_TXCSR_P_WZC_BITS;
			csr &= ~(MUSB_TXCSR_P_UNDERRUN
					| MUSB_TXCSR_TXPKTRDY);
			musb_writew(epio, MUSB_TXCSR, csr);
			DBG(20, "underrun on ep%d, req %p\n", epnum, request);
		}

		if (dma_channel_status(dma) == MUSB_DMA_STATUS_BUSY) {
			
			DBG(5, "%s dma still busy?\n", musb_ep->end_point.name);
			break;
		}

		if (request) {
			u8	is_dma = 0;

			if (dma && (csr & MUSB_TXCSR_DMAENAB)) {
				is_dma = 1;
				csr |= MUSB_TXCSR_P_WZC_BITS;
				csr &= ~(MUSB_TXCSR_DMAENAB
						| MUSB_TXCSR_P_UNDERRUN
						| MUSB_TXCSR_TXPKTRDY);
				musb_writew(epio, MUSB_TXCSR, csr);
				
				csr = musb_readw(epio, MUSB_TXCSR);
				request->actual += musb_ep->dma->actual_len;
				DBG(4, "TXCSR%d %04x, dma off, "
						"len %zu, req %p\n",
					epnum, csr,
					musb_ep->dma->actual_len,
					request);
			}

			if (is_dma || request->actual == request->length) {

				
				if ((request->zero
						&& request->length
						&& (request->length
							% musb_ep->packet_sz)
							== 0)
#ifdef CONFIG_USB_INVENTRA_DMA
					|| (is_dma &&
						((!dma->desired_mode) ||
						    (request->actual &
						    (musb_ep->packet_sz - 1))))
#endif
				) {
					
					if (csr & MUSB_TXCSR_TXPKTRDY)
						break;

					DBG(4, "sending zero pkt\n");
					musb_writew(epio, MUSB_TXCSR,
							MUSB_TXCSR_MODE
							| MUSB_TXCSR_TXPKTRDY);
					request->zero = 0;
				}

				
				musb_g_giveback(musb_ep, request, 0);

				
				musb_ep_select(mbase, epnum);
				csr = musb_readw(epio, MUSB_TXCSR);
				if (csr & MUSB_TXCSR_FIFONOTEMPTY)
					break;
				request = musb_ep->desc
						? next_request(musb_ep)
						: NULL;
				if (!request) {
					DBG(4, "%s idle now\n",
						musb_ep->end_point.name);
					break;
				}
			}

			txstate(musb, to_musb_request(request));
		}

	} while (0);
}



#ifdef CONFIG_USB_INVENTRA_DMA



#endif


static void rxstate(struct musb *musb, struct musb_request *req)
{
	const u8		epnum = req->epnum;
	struct usb_request	*request = &req->request;
	struct musb_ep		*musb_ep = &musb->endpoints[epnum].ep_out;
	void __iomem		*epio = musb->endpoints[epnum].regs;
	unsigned		fifo_count = 0;
	u16			len = musb_ep->packet_sz;
	u16			csr = musb_readw(epio, MUSB_RXCSR);

	
	if (dma_channel_status(musb_ep->dma) == MUSB_DMA_STATUS_BUSY) {
		DBG(4, "DMA pending...\n");
		return;
	}

	if (csr & MUSB_RXCSR_P_SENDSTALL) {
		DBG(5, "%s stalling, RXCSR %04x\n",
		    musb_ep->end_point.name, csr);
		return;
	}

	if (is_cppi_enabled() && musb_ep->dma) {
		struct dma_controller	*c = musb->dma_controller;
		struct dma_channel	*channel = musb_ep->dma;

		
		if (c->channel_program(channel,
				musb_ep->packet_sz,
				!request->short_not_ok,
				request->dma + request->actual,
				request->length - request->actual)) {

			
			csr &= ~(MUSB_RXCSR_AUTOCLEAR
					| MUSB_RXCSR_DMAMODE);
			csr |= MUSB_RXCSR_DMAENAB | MUSB_RXCSR_P_WZC_BITS;
			musb_writew(epio, MUSB_RXCSR, csr);
			return;
		}
	}

	if (csr & MUSB_RXCSR_RXPKTRDY) {
		len = musb_readw(epio, MUSB_RXCOUNT);
		if (request->actual < request->length) {
#ifdef CONFIG_USB_INVENTRA_DMA
			if (is_dma_capable() && musb_ep->dma) {
				struct dma_controller	*c;
				struct dma_channel	*channel;
				int			use_dma = 0;

				c = musb->dma_controller;
				channel = musb_ep->dma;

	

				csr |= MUSB_RXCSR_DMAENAB;
#ifdef USE_MODE1
				csr |= MUSB_RXCSR_AUTOCLEAR;
				

				
				musb_writew(epio, MUSB_RXCSR,
					csr | MUSB_RXCSR_DMAMODE);
#endif
				musb_writew(epio, MUSB_RXCSR, csr);

				if (request->actual < request->length) {
					int transfer_size = 0;
#ifdef USE_MODE1
					transfer_size = min(request->length,
							channel->max_len);
#else
					transfer_size = len;
#endif
					if (transfer_size <= musb_ep->packet_sz)
						musb_ep->dma->desired_mode = 0;
					else
						musb_ep->dma->desired_mode = 1;

					use_dma = c->channel_program(
							channel,
							musb_ep->packet_sz,
							channel->desired_mode,
							request->dma
							+ request->actual,
							transfer_size);
				}

				if (use_dma)
					return;
			}
#endif	

			fifo_count = request->length - request->actual;
			DBG(3, "%s OUT/RX pio fifo %d/%d, maxpacket %d\n",
					musb_ep->end_point.name,
					len, fifo_count,
					musb_ep->packet_sz);

			fifo_count = min_t(unsigned, len, fifo_count);

#ifdef	CONFIG_USB_TUSB_OMAP_DMA
			if (tusb_dma_omap() && musb_ep->dma) {
				struct dma_controller *c = musb->dma_controller;
				struct dma_channel *channel = musb_ep->dma;
				u32 dma_addr = request->dma + request->actual;
				int ret;

				ret = c->channel_program(channel,
						musb_ep->packet_sz,
						channel->desired_mode,
						dma_addr,
						fifo_count);
				if (ret)
					return;
			}
#endif

			musb_read_fifo(musb_ep->hw_ep, fifo_count, (u8 *)
					(request->buf + request->actual));
			request->actual += fifo_count;

			

			
			csr |= MUSB_RXCSR_P_WZC_BITS;
			csr &= ~MUSB_RXCSR_RXPKTRDY;
			musb_writew(epio, MUSB_RXCSR, csr);
		}
	}

	
	if (request->actual == request->length || len < musb_ep->packet_sz)
		musb_g_giveback(musb_ep, request, 0);
}


void musb_g_rx(struct musb *musb, u8 epnum)
{
	u16			csr;
	struct usb_request	*request;
	void __iomem		*mbase = musb->mregs;
	struct musb_ep		*musb_ep = &musb->endpoints[epnum].ep_out;
	void __iomem		*epio = musb->endpoints[epnum].regs;
	struct dma_channel	*dma;

	musb_ep_select(mbase, epnum);

	request = next_request(musb_ep);

	csr = musb_readw(epio, MUSB_RXCSR);
	dma = is_dma_capable() ? musb_ep->dma : NULL;

	DBG(4, "<== %s, rxcsr %04x%s %p\n", musb_ep->end_point.name,
			csr, dma ? " (dma)" : "", request);

	if (csr & MUSB_RXCSR_P_SENTSTALL) {
		csr |= MUSB_RXCSR_P_WZC_BITS;
		csr &= ~MUSB_RXCSR_P_SENTSTALL;
		musb_writew(epio, MUSB_RXCSR, csr);
		return;
	}

	if (csr & MUSB_RXCSR_P_OVERRUN) {
		
		csr &= ~MUSB_RXCSR_P_OVERRUN;
		musb_writew(epio, MUSB_RXCSR, csr);

		DBG(3, "%s iso overrun on %p\n", musb_ep->name, request);
		if (request && request->status == -EINPROGRESS)
			request->status = -EOVERFLOW;
	}
	if (csr & MUSB_RXCSR_INCOMPRX) {
		
		DBG(4, "%s, incomprx\n", musb_ep->end_point.name);
	}

	if (dma_channel_status(dma) == MUSB_DMA_STATUS_BUSY) {
		
		DBG((csr & MUSB_RXCSR_DMAENAB) ? 4 : 1,
			"%s busy, csr %04x\n",
			musb_ep->end_point.name, csr);
		return;
	}

	if (dma && (csr & MUSB_RXCSR_DMAENAB)) {
		csr &= ~(MUSB_RXCSR_AUTOCLEAR
				| MUSB_RXCSR_DMAENAB
				| MUSB_RXCSR_DMAMODE);
		musb_writew(epio, MUSB_RXCSR,
			MUSB_RXCSR_P_WZC_BITS | csr);

		request->actual += musb_ep->dma->actual_len;

		DBG(4, "RXCSR%d %04x, dma off, %04x, len %zu, req %p\n",
			epnum, csr,
			musb_readw(epio, MUSB_RXCSR),
			musb_ep->dma->actual_len, request);

#if defined(CONFIG_USB_INVENTRA_DMA) || defined(CONFIG_USB_TUSB_OMAP_DMA)
		
		if ((dma->desired_mode == 0)
				|| (dma->actual_len
					& (musb_ep->packet_sz - 1))) {
			
			csr &= ~MUSB_RXCSR_RXPKTRDY;
			musb_writew(epio, MUSB_RXCSR, csr);
		}

		
		if ((request->actual < request->length)
				&& (musb_ep->dma->actual_len
					== musb_ep->packet_sz))
			return;
#endif
		musb_g_giveback(musb_ep, request, 0);

		request = next_request(musb_ep);
		if (!request)
			return;
	}

	
	if (request)
		rxstate(musb, to_musb_request(request));
	else
		DBG(3, "packet waiting for %s%s request\n",
				musb_ep->desc ? "" : "inactive ",
				musb_ep->end_point.name);
	return;
}



static int musb_gadget_enable(struct usb_ep *ep,
			const struct usb_endpoint_descriptor *desc)
{
	unsigned long		flags;
	struct musb_ep		*musb_ep;
	struct musb_hw_ep	*hw_ep;
	void __iomem		*regs;
	struct musb		*musb;
	void __iomem	*mbase;
	u8		epnum;
	u16		csr;
	unsigned	tmp;
	int		status = -EINVAL;

	if (!ep || !desc)
		return -EINVAL;

	musb_ep = to_musb_ep(ep);
	hw_ep = musb_ep->hw_ep;
	regs = hw_ep->regs;
	musb = musb_ep->musb;
	mbase = musb->mregs;
	epnum = musb_ep->current_epnum;

	spin_lock_irqsave(&musb->lock, flags);

	if (musb_ep->desc) {
		status = -EBUSY;
		goto fail;
	}
	musb_ep->type = usb_endpoint_type(desc);

	
	if (usb_endpoint_num(desc) != epnum)
		goto fail;

	
	tmp = le16_to_cpu(desc->wMaxPacketSize);
	if (tmp & ~0x07ff)
		goto fail;
	musb_ep->packet_sz = tmp;

	
	musb_ep_select(mbase, epnum);
	if (usb_endpoint_dir_in(desc)) {
		u16 int_txe = musb_readw(mbase, MUSB_INTRTXE);

		if (hw_ep->is_shared_fifo)
			musb_ep->is_in = 1;
		if (!musb_ep->is_in)
			goto fail;
		if (tmp > hw_ep->max_packet_sz_tx)
			goto fail;

		int_txe |= (1 << epnum);
		musb_writew(mbase, MUSB_INTRTXE, int_txe);

		
		musb_writew(regs, MUSB_TXMAXP, tmp);

		csr = MUSB_TXCSR_MODE | MUSB_TXCSR_CLRDATATOG;
		if (musb_readw(regs, MUSB_TXCSR)
				& MUSB_TXCSR_FIFONOTEMPTY)
			csr |= MUSB_TXCSR_FLUSHFIFO;
		if (musb_ep->type == USB_ENDPOINT_XFER_ISOC)
			csr |= MUSB_TXCSR_P_ISO;

		
		musb_writew(regs, MUSB_TXCSR, csr);
		
		musb_writew(regs, MUSB_TXCSR, csr);

	} else {
		u16 int_rxe = musb_readw(mbase, MUSB_INTRRXE);

		if (hw_ep->is_shared_fifo)
			musb_ep->is_in = 0;
		if (musb_ep->is_in)
			goto fail;
		if (tmp > hw_ep->max_packet_sz_rx)
			goto fail;

		int_rxe |= (1 << epnum);
		musb_writew(mbase, MUSB_INTRRXE, int_rxe);

		
		musb_writew(regs, MUSB_RXMAXP, tmp);

		
		if (hw_ep->is_shared_fifo) {
			csr = musb_readw(regs, MUSB_TXCSR);
			csr &= ~(MUSB_TXCSR_MODE | MUSB_TXCSR_TXPKTRDY);
			musb_writew(regs, MUSB_TXCSR, csr);
		}

		csr = MUSB_RXCSR_FLUSHFIFO | MUSB_RXCSR_CLRDATATOG;
		if (musb_ep->type == USB_ENDPOINT_XFER_ISOC)
			csr |= MUSB_RXCSR_P_ISO;
		else if (musb_ep->type == USB_ENDPOINT_XFER_INT)
			csr |= MUSB_RXCSR_DISNYET;

		
		musb_writew(regs, MUSB_RXCSR, csr);
		musb_writew(regs, MUSB_RXCSR, csr);
	}

	
	if (is_dma_capable() && musb->dma_controller) {
		struct dma_controller	*c = musb->dma_controller;

		musb_ep->dma = c->channel_alloc(c, hw_ep,
				(desc->bEndpointAddress & USB_DIR_IN));
	} else
		musb_ep->dma = NULL;

	musb_ep->desc = desc;
	musb_ep->busy = 0;
	status = 0;

	pr_debug("%s periph: enabled %s for %s %s, %smaxpacket %d\n",
			musb_driver_name, musb_ep->end_point.name,
			({ char *s; switch (musb_ep->type) {
			case USB_ENDPOINT_XFER_BULK:	s = "bulk"; break;
			case USB_ENDPOINT_XFER_INT:	s = "int"; break;
			default:			s = "iso"; break;
			}; s; }),
			musb_ep->is_in ? "IN" : "OUT",
			musb_ep->dma ? "dma, " : "",
			musb_ep->packet_sz);

	schedule_work(&musb->irq_work);

fail:
	spin_unlock_irqrestore(&musb->lock, flags);
	return status;
}


static int musb_gadget_disable(struct usb_ep *ep)
{
	unsigned long	flags;
	struct musb	*musb;
	u8		epnum;
	struct musb_ep	*musb_ep;
	void __iomem	*epio;
	int		status = 0;

	musb_ep = to_musb_ep(ep);
	musb = musb_ep->musb;
	epnum = musb_ep->current_epnum;
	epio = musb->endpoints[epnum].regs;

	spin_lock_irqsave(&musb->lock, flags);
	musb_ep_select(musb->mregs, epnum);

	
	if (musb_ep->is_in) {
		u16 int_txe = musb_readw(musb->mregs, MUSB_INTRTXE);
		int_txe &= ~(1 << epnum);
		musb_writew(musb->mregs, MUSB_INTRTXE, int_txe);
		musb_writew(epio, MUSB_TXMAXP, 0);
	} else {
		u16 int_rxe = musb_readw(musb->mregs, MUSB_INTRRXE);
		int_rxe &= ~(1 << epnum);
		musb_writew(musb->mregs, MUSB_INTRRXE, int_rxe);
		musb_writew(epio, MUSB_RXMAXP, 0);
	}

	musb_ep->desc = NULL;

	
	nuke(musb_ep, -ESHUTDOWN);

	schedule_work(&musb->irq_work);

	spin_unlock_irqrestore(&(musb->lock), flags);

	DBG(2, "%s\n", musb_ep->end_point.name);

	return status;
}


struct usb_request *musb_alloc_request(struct usb_ep *ep, gfp_t gfp_flags)
{
	struct musb_ep		*musb_ep = to_musb_ep(ep);
	struct musb_request	*request = NULL;

	request = kzalloc(sizeof *request, gfp_flags);
	if (request) {
		INIT_LIST_HEAD(&request->request.list);
		request->request.dma = DMA_ADDR_INVALID;
		request->epnum = musb_ep->current_epnum;
		request->ep = musb_ep;
	}

	return &request->request;
}


void musb_free_request(struct usb_ep *ep, struct usb_request *req)
{
	kfree(to_musb_request(req));
}

static LIST_HEAD(buffers);

struct free_record {
	struct list_head	list;
	struct device		*dev;
	unsigned		bytes;
	dma_addr_t		dma;
};


static void musb_ep_restart(struct musb *musb, struct musb_request *req)
{
	DBG(3, "<== %s request %p len %u on hw_ep%d\n",
		req->tx ? "TX/IN" : "RX/OUT",
		&req->request, req->request.length, req->epnum);

	musb_ep_select(musb->mregs, req->epnum);
	if (req->tx)
		txstate(musb, req);
	else
		rxstate(musb, req);
}

static int musb_gadget_queue(struct usb_ep *ep, struct usb_request *req,
			gfp_t gfp_flags)
{
	struct musb_ep		*musb_ep;
	struct musb_request	*request;
	struct musb		*musb;
	int			status = 0;
	unsigned long		lockflags;

	if (!ep || !req)
		return -EINVAL;
	if (!req->buf)
		return -ENODATA;

	musb_ep = to_musb_ep(ep);
	musb = musb_ep->musb;

	request = to_musb_request(req);
	request->musb = musb;

	if (request->ep != musb_ep)
		return -EINVAL;

	DBG(4, "<== to %s request=%p\n", ep->name, req);

	
	request->request.actual = 0;
	request->request.status = -EINPROGRESS;
	request->epnum = musb_ep->current_epnum;
	request->tx = musb_ep->is_in;

	if (is_dma_capable() && musb_ep->dma) {
		if (request->request.dma == DMA_ADDR_INVALID) {
			request->request.dma = dma_map_single(
					musb->controller,
					request->request.buf,
					request->request.length,
					request->tx
						? DMA_TO_DEVICE
						: DMA_FROM_DEVICE);
			request->mapped = 1;
		} else {
			dma_sync_single_for_device(musb->controller,
					request->request.dma,
					request->request.length,
					request->tx
						? DMA_TO_DEVICE
						: DMA_FROM_DEVICE);
			request->mapped = 0;
		}
	} else if (!req->buf) {
		return -ENODATA;
	} else
		request->mapped = 0;

	spin_lock_irqsave(&musb->lock, lockflags);

	
	if (!musb_ep->desc) {
		DBG(4, "req %p queued to %s while ep %s\n",
				req, ep->name, "disabled");
		status = -ESHUTDOWN;
		goto cleanup;
	}

	
	list_add_tail(&(request->request.list), &(musb_ep->req_list));

	
	if (!musb_ep->busy && &request->request.list == musb_ep->req_list.next)
		musb_ep_restart(musb, request);

cleanup:
	spin_unlock_irqrestore(&musb->lock, lockflags);
	return status;
}

static int musb_gadget_dequeue(struct usb_ep *ep, struct usb_request *request)
{
	struct musb_ep		*musb_ep = to_musb_ep(ep);
	struct usb_request	*r;
	unsigned long		flags;
	int			status = 0;
	struct musb		*musb = musb_ep->musb;

	if (!ep || !request || to_musb_request(request)->ep != musb_ep)
		return -EINVAL;

	spin_lock_irqsave(&musb->lock, flags);

	list_for_each_entry(r, &musb_ep->req_list, list) {
		if (r == request)
			break;
	}
	if (r != request) {
		DBG(3, "request %p not queued to %s\n", request, ep->name);
		status = -EINVAL;
		goto done;
	}

	
	if (musb_ep->req_list.next != &request->list || musb_ep->busy)
		musb_g_giveback(musb_ep, request, -ECONNRESET);

	
	else if (is_dma_capable() && musb_ep->dma) {
		struct dma_controller	*c = musb->dma_controller;

		musb_ep_select(musb->mregs, musb_ep->current_epnum);
		if (c->channel_abort)
			status = c->channel_abort(musb_ep->dma);
		else
			status = -EBUSY;
		if (status == 0)
			musb_g_giveback(musb_ep, request, -ECONNRESET);
	} else {
		
		musb_g_giveback(musb_ep, request, -ECONNRESET);
	}

done:
	spin_unlock_irqrestore(&musb->lock, flags);
	return status;
}


int musb_gadget_set_halt(struct usb_ep *ep, int value)
{
	struct musb_ep		*musb_ep = to_musb_ep(ep);
	u8			epnum = musb_ep->current_epnum;
	struct musb		*musb = musb_ep->musb;
	void __iomem		*epio = musb->endpoints[epnum].regs;
	void __iomem		*mbase;
	unsigned long		flags;
	u16			csr;
	struct musb_request	*request;
	int			status = 0;

	if (!ep)
		return -EINVAL;
	mbase = musb->mregs;

	spin_lock_irqsave(&musb->lock, flags);

	if ((USB_ENDPOINT_XFER_ISOC == musb_ep->type)) {
		status = -EINVAL;
		goto done;
	}

	musb_ep_select(mbase, epnum);

	request = to_musb_request(next_request(musb_ep));
	if (value) {
		if (request) {
			DBG(3, "request in progress, cannot halt %s\n",
			    ep->name);
			status = -EAGAIN;
			goto done;
		}
		
		if (musb_ep->is_in) {
			csr = musb_readw(epio, MUSB_TXCSR);
			if (csr & MUSB_TXCSR_FIFONOTEMPTY) {
				DBG(3, "FIFO busy, cannot halt %s\n", ep->name);
				status = -EAGAIN;
				goto done;
			}
		}
	}

	
	DBG(2, "%s: %s stall\n", ep->name, value ? "set" : "clear");
	if (musb_ep->is_in) {
		csr = musb_readw(epio, MUSB_TXCSR);
		csr |= MUSB_TXCSR_P_WZC_BITS
			| MUSB_TXCSR_CLRDATATOG;
		if (value)
			csr |= MUSB_TXCSR_P_SENDSTALL;
		else
			csr &= ~(MUSB_TXCSR_P_SENDSTALL
				| MUSB_TXCSR_P_SENTSTALL);
		csr &= ~MUSB_TXCSR_TXPKTRDY;
		musb_writew(epio, MUSB_TXCSR, csr);
	} else {
		csr = musb_readw(epio, MUSB_RXCSR);
		csr |= MUSB_RXCSR_P_WZC_BITS
			| MUSB_RXCSR_FLUSHFIFO
			| MUSB_RXCSR_CLRDATATOG;
		if (value)
			csr |= MUSB_RXCSR_P_SENDSTALL;
		else
			csr &= ~(MUSB_RXCSR_P_SENDSTALL
				| MUSB_RXCSR_P_SENTSTALL);
		musb_writew(epio, MUSB_RXCSR, csr);
	}

	
	if (!musb_ep->busy && !value && request) {
		DBG(3, "restarting the request\n");
		musb_ep_restart(musb, request);
	}

done:
	spin_unlock_irqrestore(&musb->lock, flags);
	return status;
}

static int musb_gadget_fifo_status(struct usb_ep *ep)
{
	struct musb_ep		*musb_ep = to_musb_ep(ep);
	void __iomem		*epio = musb_ep->hw_ep->regs;
	int			retval = -EINVAL;

	if (musb_ep->desc && !musb_ep->is_in) {
		struct musb		*musb = musb_ep->musb;
		int			epnum = musb_ep->current_epnum;
		void __iomem		*mbase = musb->mregs;
		unsigned long		flags;

		spin_lock_irqsave(&musb->lock, flags);

		musb_ep_select(mbase, epnum);
		
		retval = musb_readw(epio, MUSB_RXCOUNT);

		spin_unlock_irqrestore(&musb->lock, flags);
	}
	return retval;
}

static void musb_gadget_fifo_flush(struct usb_ep *ep)
{
	struct musb_ep	*musb_ep = to_musb_ep(ep);
	struct musb	*musb = musb_ep->musb;
	u8		epnum = musb_ep->current_epnum;
	void __iomem	*epio = musb->endpoints[epnum].regs;
	void __iomem	*mbase;
	unsigned long	flags;
	u16		csr, int_txe;

	mbase = musb->mregs;

	spin_lock_irqsave(&musb->lock, flags);
	musb_ep_select(mbase, (u8) epnum);

	
	int_txe = musb_readw(mbase, MUSB_INTRTXE);
	musb_writew(mbase, MUSB_INTRTXE, int_txe & ~(1 << epnum));

	if (musb_ep->is_in) {
		csr = musb_readw(epio, MUSB_TXCSR);
		if (csr & MUSB_TXCSR_FIFONOTEMPTY) {
			csr |= MUSB_TXCSR_FLUSHFIFO | MUSB_TXCSR_P_WZC_BITS;
			musb_writew(epio, MUSB_TXCSR, csr);
			
			musb_writew(epio, MUSB_TXCSR, csr);
		}
	} else {
		csr = musb_readw(epio, MUSB_RXCSR);
		csr |= MUSB_RXCSR_FLUSHFIFO | MUSB_RXCSR_P_WZC_BITS;
		musb_writew(epio, MUSB_RXCSR, csr);
		musb_writew(epio, MUSB_RXCSR, csr);
	}

	
	musb_writew(mbase, MUSB_INTRTXE, int_txe);
	spin_unlock_irqrestore(&musb->lock, flags);
}

static const struct usb_ep_ops musb_ep_ops = {
	.enable		= musb_gadget_enable,
	.disable	= musb_gadget_disable,
	.alloc_request	= musb_alloc_request,
	.free_request	= musb_free_request,
	.queue		= musb_gadget_queue,
	.dequeue	= musb_gadget_dequeue,
	.set_halt	= musb_gadget_set_halt,
	.fifo_status	= musb_gadget_fifo_status,
	.fifo_flush	= musb_gadget_fifo_flush
};



static int musb_gadget_get_frame(struct usb_gadget *gadget)
{
	struct musb	*musb = gadget_to_musb(gadget);

	return (int)musb_readw(musb->mregs, MUSB_FRAME);
}

static int musb_gadget_wakeup(struct usb_gadget *gadget)
{
	struct musb	*musb = gadget_to_musb(gadget);
	void __iomem	*mregs = musb->mregs;
	unsigned long	flags;
	int		status = -EINVAL;
	u8		power, devctl;
	int		retries;

	spin_lock_irqsave(&musb->lock, flags);

	switch (musb->xceiv->state) {
	case OTG_STATE_B_PERIPHERAL:
		
		if (musb->may_wakeup && musb->is_suspended)
			break;
		goto done;
	case OTG_STATE_B_IDLE:
		
		devctl = musb_readb(mregs, MUSB_DEVCTL);
		DBG(2, "Sending SRP: devctl: %02x\n", devctl);
		devctl |= MUSB_DEVCTL_SESSION;
		musb_writeb(mregs, MUSB_DEVCTL, devctl);
		devctl = musb_readb(mregs, MUSB_DEVCTL);
		retries = 100;
		while (!(devctl & MUSB_DEVCTL_SESSION)) {
			devctl = musb_readb(mregs, MUSB_DEVCTL);
			if (retries-- < 1)
				break;
		}
		retries = 10000;
		while (devctl & MUSB_DEVCTL_SESSION) {
			devctl = musb_readb(mregs, MUSB_DEVCTL);
			if (retries-- < 1)
				break;
		}

		
		musb_platform_try_idle(musb,
			jiffies + msecs_to_jiffies(1 * HZ));

		status = 0;
		goto done;
	default:
		DBG(2, "Unhandled wake: %s\n", otg_state_string(musb));
		goto done;
	}

	status = 0;

	power = musb_readb(mregs, MUSB_POWER);
	power |= MUSB_POWER_RESUME;
	musb_writeb(mregs, MUSB_POWER, power);
	DBG(2, "issue wakeup\n");

	
	mdelay(2);

	power = musb_readb(mregs, MUSB_POWER);
	power &= ~MUSB_POWER_RESUME;
	musb_writeb(mregs, MUSB_POWER, power);
done:
	spin_unlock_irqrestore(&musb->lock, flags);
	return status;
}

static int
musb_gadget_set_self_powered(struct usb_gadget *gadget, int is_selfpowered)
{
	struct musb	*musb = gadget_to_musb(gadget);

	musb->is_self_powered = !!is_selfpowered;
	return 0;
}

static void musb_pullup(struct musb *musb, int is_on)
{
	u8 power;

	power = musb_readb(musb->mregs, MUSB_POWER);
	if (is_on)
		power |= MUSB_POWER_SOFTCONN;
	else
		power &= ~MUSB_POWER_SOFTCONN;

	

	DBG(3, "gadget %s D+ pullup %s\n",
		musb->gadget_driver->function, is_on ? "on" : "off");
	musb_writeb(musb->mregs, MUSB_POWER, power);
}

#if 0
static int musb_gadget_vbus_session(struct usb_gadget *gadget, int is_active)
{
	DBG(2, "<= %s =>\n", __func__);

	

	return -EINVAL;
}
#endif

static int musb_gadget_vbus_draw(struct usb_gadget *gadget, unsigned mA)
{
	struct musb	*musb = gadget_to_musb(gadget);

	if (!musb->xceiv->set_power)
		return -EOPNOTSUPP;
	return otg_set_power(musb->xceiv, mA);
}

static int musb_gadget_pullup(struct usb_gadget *gadget, int is_on)
{
	struct musb	*musb = gadget_to_musb(gadget);
	unsigned long	flags;

	is_on = !!is_on;

	
	spin_lock_irqsave(&musb->lock, flags);
	if (is_on != musb->softconnect) {
		musb->softconnect = is_on;
		musb_pullup(musb, is_on);
	}
	spin_unlock_irqrestore(&musb->lock, flags);
	return 0;
}

static const struct usb_gadget_ops musb_gadget_operations = {
	.get_frame		= musb_gadget_get_frame,
	.wakeup			= musb_gadget_wakeup,
	.set_selfpowered	= musb_gadget_set_self_powered,
	
	.vbus_draw		= musb_gadget_vbus_draw,
	.pullup			= musb_gadget_pullup,
};






static struct musb *the_gadget;

static void musb_gadget_release(struct device *dev)
{
	
	dev_dbg(dev, "%s\n", __func__);
}


static void __init
init_peripheral_ep(struct musb *musb, struct musb_ep *ep, u8 epnum, int is_in)
{
	struct musb_hw_ep	*hw_ep = musb->endpoints + epnum;

	memset(ep, 0, sizeof *ep);

	ep->current_epnum = epnum;
	ep->musb = musb;
	ep->hw_ep = hw_ep;
	ep->is_in = is_in;

	INIT_LIST_HEAD(&ep->req_list);

	sprintf(ep->name, "ep%d%s", epnum,
			(!epnum || hw_ep->is_shared_fifo) ? "" : (
				is_in ? "in" : "out"));
	ep->end_point.name = ep->name;
	INIT_LIST_HEAD(&ep->end_point.ep_list);
	if (!epnum) {
		ep->end_point.maxpacket = 64;
		ep->end_point.ops = &musb_g_ep0_ops;
		musb->g.ep0 = &ep->end_point;
	} else {
		if (is_in)
			ep->end_point.maxpacket = hw_ep->max_packet_sz_tx;
		else
			ep->end_point.maxpacket = hw_ep->max_packet_sz_rx;
		ep->end_point.ops = &musb_ep_ops;
		list_add_tail(&ep->end_point.ep_list, &musb->g.ep_list);
	}
}


static inline void __init musb_g_init_endpoints(struct musb *musb)
{
	u8			epnum;
	struct musb_hw_ep	*hw_ep;
	unsigned		count = 0;

	
	INIT_LIST_HEAD(&(musb->g.ep_list));

	for (epnum = 0, hw_ep = musb->endpoints;
			epnum < musb->nr_endpoints;
			epnum++, hw_ep++) {
		if (hw_ep->is_shared_fifo ) {
			init_peripheral_ep(musb, &hw_ep->ep_in, epnum, 0);
			count++;
		} else {
			if (hw_ep->max_packet_sz_tx) {
				init_peripheral_ep(musb, &hw_ep->ep_in,
							epnum, 1);
				count++;
			}
			if (hw_ep->max_packet_sz_rx) {
				init_peripheral_ep(musb, &hw_ep->ep_out,
							epnum, 0);
				count++;
			}
		}
	}
}


int __init musb_gadget_setup(struct musb *musb)
{
	int status;

	
	if (the_gadget)
		return -EBUSY;
	the_gadget = musb;

	musb->g.ops = &musb_gadget_operations;
	musb->g.is_dualspeed = 1;
	musb->g.speed = USB_SPEED_UNKNOWN;

	
	dev_set_name(&musb->g.dev, "gadget");
	musb->g.dev.parent = musb->controller;
	musb->g.dev.dma_mask = musb->controller->dma_mask;
	musb->g.dev.release = musb_gadget_release;
	musb->g.name = musb_driver_name;

	if (is_otg_enabled(musb))
		musb->g.is_otg = 1;

	musb_g_init_endpoints(musb);

	musb->is_active = 0;
	musb_platform_try_idle(musb, 0);

	status = device_register(&musb->g.dev);
	if (status != 0)
		the_gadget = NULL;
	return status;
}

void musb_gadget_cleanup(struct musb *musb)
{
	if (musb != the_gadget)
		return;

	device_unregister(&musb->g.dev);
	the_gadget = NULL;
}


int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	int retval;
	unsigned long flags;
	struct musb *musb = the_gadget;

	if (!driver
			|| driver->speed != USB_SPEED_HIGH
			|| !driver->bind
			|| !driver->setup)
		return -EINVAL;

	
	if (!musb || !(musb->board_mode == MUSB_OTG
				|| musb->board_mode != MUSB_OTG)) {
		DBG(1, "%s, no dev??\n", __func__);
		return -ENODEV;
	}

	DBG(3, "registering driver %s\n", driver->function);
	spin_lock_irqsave(&musb->lock, flags);

	if (musb->gadget_driver) {
		DBG(1, "%s is already bound to %s\n",
				musb_driver_name,
				musb->gadget_driver->driver.name);
		retval = -EBUSY;
	} else {
		musb->gadget_driver = driver;
		musb->g.dev.driver = &driver->driver;
		driver->driver.bus = NULL;
		musb->softconnect = 1;
		retval = 0;
	}

	spin_unlock_irqrestore(&musb->lock, flags);

	if (retval == 0) {
		retval = driver->bind(&musb->g);
		if (retval != 0) {
			DBG(3, "bind to driver %s failed --> %d\n",
					driver->driver.name, retval);
			musb->gadget_driver = NULL;
			musb->g.dev.driver = NULL;
		}

		spin_lock_irqsave(&musb->lock, flags);

		otg_set_peripheral(musb->xceiv, &musb->g);
		musb->is_active = 1;

		

		if (!is_otg_enabled(musb))
			musb_start(musb);

		otg_set_peripheral(musb->xceiv, &musb->g);

		spin_unlock_irqrestore(&musb->lock, flags);

		if (is_otg_enabled(musb)) {
			DBG(3, "OTG startup...\n");

			
			retval = usb_add_hcd(musb_to_hcd(musb), -1, 0);
			if (retval < 0) {
				DBG(1, "add_hcd failed, %d\n", retval);
				spin_lock_irqsave(&musb->lock, flags);
				otg_set_peripheral(musb->xceiv, NULL);
				musb->gadget_driver = NULL;
				musb->g.dev.driver = NULL;
				spin_unlock_irqrestore(&musb->lock, flags);
			}
		}
	}

	return retval;
}
EXPORT_SYMBOL(usb_gadget_register_driver);

static void stop_activity(struct musb *musb, struct usb_gadget_driver *driver)
{
	int			i;
	struct musb_hw_ep	*hw_ep;

	
	if (musb->g.speed == USB_SPEED_UNKNOWN)
		driver = NULL;
	else
		musb->g.speed = USB_SPEED_UNKNOWN;

	
	if (musb->softconnect) {
		musb->softconnect = 0;
		musb_pullup(musb, 0);
	}
	musb_stop(musb);

	
	if (driver) {
		for (i = 0, hw_ep = musb->endpoints;
				i < musb->nr_endpoints;
				i++, hw_ep++) {
			musb_ep_select(musb->mregs, i);
			if (hw_ep->is_shared_fifo ) {
				nuke(&hw_ep->ep_in, -ESHUTDOWN);
			} else {
				if (hw_ep->max_packet_sz_tx)
					nuke(&hw_ep->ep_in, -ESHUTDOWN);
				if (hw_ep->max_packet_sz_rx)
					nuke(&hw_ep->ep_out, -ESHUTDOWN);
			}
		}

		spin_unlock(&musb->lock);
		driver->disconnect(&musb->g);
		spin_lock(&musb->lock);
	}
}


int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	unsigned long	flags;
	int		retval = 0;
	struct musb	*musb = the_gadget;

	if (!driver || !driver->unbind || !musb)
		return -EINVAL;

	

	spin_lock_irqsave(&musb->lock, flags);

#ifdef	CONFIG_USB_MUSB_OTG
	musb_hnp_stop(musb);
#endif

	if (musb->gadget_driver == driver) {

		(void) musb_gadget_vbus_draw(&musb->g, 0);

		musb->xceiv->state = OTG_STATE_UNDEFINED;
		stop_activity(musb, driver);
		otg_set_peripheral(musb->xceiv, NULL);

		DBG(3, "unregistering driver %s\n", driver->function);
		spin_unlock_irqrestore(&musb->lock, flags);
		driver->unbind(&musb->g);
		spin_lock_irqsave(&musb->lock, flags);

		musb->gadget_driver = NULL;
		musb->g.dev.driver = NULL;

		musb->is_active = 0;
		musb_platform_try_idle(musb, 0);
	} else
		retval = -EINVAL;
	spin_unlock_irqrestore(&musb->lock, flags);

	if (is_otg_enabled(musb) && retval == 0) {
		usb_remove_hcd(musb_to_hcd(musb));
		
	}

	return retval;
}
EXPORT_SYMBOL(usb_gadget_unregister_driver);






void musb_g_resume(struct musb *musb)
{
	musb->is_suspended = 0;
	switch (musb->xceiv->state) {
	case OTG_STATE_B_IDLE:
		break;
	case OTG_STATE_B_WAIT_ACON:
	case OTG_STATE_B_PERIPHERAL:
		musb->is_active = 1;
		if (musb->gadget_driver && musb->gadget_driver->resume) {
			spin_unlock(&musb->lock);
			musb->gadget_driver->resume(&musb->g);
			spin_lock(&musb->lock);
		}
		break;
	default:
		WARNING("unhandled RESUME transition (%s)\n",
				otg_state_string(musb));
	}
}


void musb_g_suspend(struct musb *musb)
{
	u8	devctl;

	devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
	DBG(3, "devctl %02x\n", devctl);

	switch (musb->xceiv->state) {
	case OTG_STATE_B_IDLE:
		if ((devctl & MUSB_DEVCTL_VBUS) == MUSB_DEVCTL_VBUS)
			musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
		break;
	case OTG_STATE_B_PERIPHERAL:
		musb->is_suspended = 1;
		if (musb->gadget_driver && musb->gadget_driver->suspend) {
			spin_unlock(&musb->lock);
			musb->gadget_driver->suspend(&musb->g);
			spin_lock(&musb->lock);
		}
		break;
	default:
		
		WARNING("unhandled SUSPEND transition (%s)\n",
				otg_state_string(musb));
	}
}


void musb_g_wakeup(struct musb *musb)
{
	musb_gadget_wakeup(&musb->g);
}


void musb_g_disconnect(struct musb *musb)
{
	void __iomem	*mregs = musb->mregs;
	u8	devctl = musb_readb(mregs, MUSB_DEVCTL);

	DBG(3, "devctl %02x\n", devctl);

	
	musb_writeb(mregs, MUSB_DEVCTL, devctl & MUSB_DEVCTL_SESSION);

	
	(void) musb_gadget_vbus_draw(&musb->g, 0);

	musb->g.speed = USB_SPEED_UNKNOWN;
	if (musb->gadget_driver && musb->gadget_driver->disconnect) {
		spin_unlock(&musb->lock);
		musb->gadget_driver->disconnect(&musb->g);
		spin_lock(&musb->lock);
	}

	switch (musb->xceiv->state) {
	default:
#ifdef	CONFIG_USB_MUSB_OTG
		DBG(2, "Unhandled disconnect %s, setting a_idle\n",
			otg_state_string(musb));
		musb->xceiv->state = OTG_STATE_A_IDLE;
		MUSB_HST_MODE(musb);
		break;
	case OTG_STATE_A_PERIPHERAL:
		musb->xceiv->state = OTG_STATE_A_WAIT_BCON;
		MUSB_HST_MODE(musb);
		break;
	case OTG_STATE_B_WAIT_ACON:
	case OTG_STATE_B_HOST:
#endif
	case OTG_STATE_B_PERIPHERAL:
	case OTG_STATE_B_IDLE:
		musb->xceiv->state = OTG_STATE_B_IDLE;
		break;
	case OTG_STATE_B_SRP_INIT:
		break;
	}

	musb->is_active = 0;
}

void musb_g_reset(struct musb *musb)
__releases(musb->lock)
__acquires(musb->lock)
{
	void __iomem	*mbase = musb->mregs;
	u8		devctl = musb_readb(mbase, MUSB_DEVCTL);
	u8		power;

	DBG(3, "<== %s addr=%x driver '%s'\n",
			(devctl & MUSB_DEVCTL_BDEVICE)
				? "B-Device" : "A-Device",
			musb_readb(mbase, MUSB_FADDR),
			musb->gadget_driver
				? musb->gadget_driver->driver.name
				: NULL
			);

	
	if (musb->g.speed != USB_SPEED_UNKNOWN)
		musb_g_disconnect(musb);

	
	else if (devctl & MUSB_DEVCTL_HR)
		musb_writeb(mbase, MUSB_DEVCTL, MUSB_DEVCTL_SESSION);


	
	power = musb_readb(mbase, MUSB_POWER);
	musb->g.speed = (power & MUSB_POWER_HSMODE)
			? USB_SPEED_HIGH : USB_SPEED_FULL;

	
	musb->is_active = 1;
	musb->is_suspended = 0;
	MUSB_DEV_MODE(musb);
	musb->address = 0;
	musb->ep0_state = MUSB_EP0_STAGE_SETUP;

	musb->may_wakeup = 0;
	musb->g.b_hnp_enable = 0;
	musb->g.a_alt_hnp_support = 0;
	musb->g.a_hnp_support = 0;

	
	if (devctl & MUSB_DEVCTL_BDEVICE) {
		musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
		musb->g.is_a_peripheral = 0;
	} else if (is_otg_enabled(musb)) {
		musb->xceiv->state = OTG_STATE_A_PERIPHERAL;
		musb->g.is_a_peripheral = 1;
	} else
		WARN_ON(1);

	
	(void) musb_gadget_vbus_draw(&musb->g,
			is_otg_enabled(musb) ? 8 : 100);
}
