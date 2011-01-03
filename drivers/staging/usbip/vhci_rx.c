

#include "usbip_common.h"
#include "vhci.h"



static struct urb *pickup_urb_and_free_priv(struct vhci_device *vdev,
					    __u32 seqnum)
{
	struct vhci_priv *priv, *tmp;
	struct urb *urb = NULL;
	int status;

	spin_lock(&vdev->priv_lock);

	list_for_each_entry_safe(priv, tmp, &vdev->priv_rx, list) {
		if (priv->seqnum == seqnum) {
			urb = priv->urb;
			status = urb->status;

			usbip_dbg_vhci_rx("find urb %p vurb %p seqnum %u\n",
				    urb, priv, seqnum);

			
			if (status != -EINPROGRESS) {
				if (status == -ENOENT ||
				     status == -ECONNRESET)
					dev_info(&urb->dev->dev,
						 "urb %p was unlinked "
						 "%ssynchronuously.\n", urb,
						 status == -ENOENT ? "" : "a");
				else
					dev_info(&urb->dev->dev,
						 "urb %p may be in a error, "
						 "status %d\n", urb, status);
			}

			list_del(&priv->list);
			kfree(priv);
			urb->hcpriv = NULL;

			break;
		}
	}

	spin_unlock(&vdev->priv_lock);

	return urb;
}

static void vhci_recv_ret_submit(struct vhci_device *vdev,
						struct usbip_header *pdu)
{
	struct usbip_device *ud = &vdev->ud;
	struct urb *urb;


	urb = pickup_urb_and_free_priv(vdev, pdu->base.seqnum);


	if (!urb) {
		usbip_uerr("cannot find a urb of seqnum %u\n",
							pdu->base.seqnum);
		usbip_uinfo("max seqnum %d\n",
					atomic_read(&the_controller->seqnum));
		usbip_event_add(ud, VDEV_EVENT_ERROR_TCP);
		return;
	}


	
	usbip_pack_pdu(pdu, urb, USBIP_RET_SUBMIT, 0);


	
	if (usbip_recv_xbuff(ud, urb) < 0)
		return;


	
	if (usbip_recv_iso(ud, urb) < 0)
		return;


	if (usbip_dbg_flag_vhci_rx)
		usbip_dump_urb(urb);


	usbip_dbg_vhci_rx("now giveback urb %p\n", urb);

	spin_lock(&the_controller->lock);
	usb_hcd_unlink_urb_from_ep(vhci_to_hcd(the_controller), urb);
	spin_unlock(&the_controller->lock);

	usb_hcd_giveback_urb(vhci_to_hcd(the_controller), urb, urb->status);


	usbip_dbg_vhci_rx("Leave\n");

	return;
}


static struct vhci_unlink *dequeue_pending_unlink(struct vhci_device *vdev,
		struct usbip_header *pdu)
{
	struct vhci_unlink *unlink, *tmp;

	spin_lock(&vdev->priv_lock);

	list_for_each_entry_safe(unlink, tmp, &vdev->unlink_rx, list) {
		usbip_uinfo("unlink->seqnum %lu\n", unlink->seqnum);
		if (unlink->seqnum == pdu->base.seqnum) {
			usbip_dbg_vhci_rx("found pending unlink, %lu\n",
							unlink->seqnum);
			list_del(&unlink->list);

			spin_unlock(&vdev->priv_lock);
			return unlink;
		}
	}

	spin_unlock(&vdev->priv_lock);

	return NULL;
}


static void vhci_recv_ret_unlink(struct vhci_device *vdev,
						struct usbip_header *pdu)
{
	struct vhci_unlink *unlink;
	struct urb *urb;

	usbip_dump_header(pdu);

	unlink = dequeue_pending_unlink(vdev, pdu);
	if (!unlink) {
		usbip_uinfo("cannot find the pending unlink %u\n",
							pdu->base.seqnum);
		return;
	}

	urb = pickup_urb_and_free_priv(vdev, unlink->unlink_seqnum);
	if (!urb) {
		
		usbip_uinfo("the urb (seqnum %d) was already given backed\n",
							pdu->base.seqnum);
	} else {
		usbip_dbg_vhci_rx("now giveback urb %p\n", urb);

		
		urb->status = pdu->u.ret_unlink.status;
		usbip_uinfo("%d\n", urb->status);

		spin_lock(&the_controller->lock);
		usb_hcd_unlink_urb_from_ep(vhci_to_hcd(the_controller), urb);
		spin_unlock(&the_controller->lock);

		usb_hcd_giveback_urb(vhci_to_hcd(the_controller), urb,
								urb->status);
	}

	kfree(unlink);

	return;
}


static void vhci_rx_pdu(struct usbip_device *ud)
{
	int ret;
	struct usbip_header pdu;
	struct vhci_device *vdev = container_of(ud, struct vhci_device, ud);


	usbip_dbg_vhci_rx("Enter\n");

	memset(&pdu, 0, sizeof(pdu));


	
	ret = usbip_xmit(0, ud->tcp_socket, (char *) &pdu, sizeof(pdu), 0);
	if (ret != sizeof(pdu)) {
		usbip_uerr("receiving pdu failed! size is %d, should be %d\n",
					ret, (unsigned int)sizeof(pdu));
		usbip_event_add(ud, VDEV_EVENT_ERROR_TCP);
		return;
	}

	usbip_header_correct_endian(&pdu, 0);

	if (usbip_dbg_flag_vhci_rx)
		usbip_dump_header(&pdu);

	switch (pdu.base.command) {
	case USBIP_RET_SUBMIT:
		vhci_recv_ret_submit(vdev, &pdu);
		break;
	case USBIP_RET_UNLINK:
		vhci_recv_ret_unlink(vdev, &pdu);
		break;
	default:
		
		usbip_uerr("unknown pdu %u\n", pdu.base.command);
		usbip_dump_header(&pdu);
		usbip_event_add(ud, VDEV_EVENT_ERROR_TCP);
	}
}




void vhci_rx_loop(struct usbip_task *ut)
{
	struct usbip_device *ud = container_of(ut, struct usbip_device, tcp_rx);


	while (1) {
		if (signal_pending(current)) {
			usbip_dbg_vhci_rx("signal catched!\n");
			break;
		}


		if (usbip_event_happened(ud))
			break;

		vhci_rx_pdu(ud);
	}
}

