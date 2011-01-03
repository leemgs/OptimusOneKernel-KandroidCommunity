

#include "int.h"
#include "rxtx.h"
#include "dpc.h"
#include "control.h"
#include "desc.h"
#include "device.h"












static int          msglevel                =MSG_LEVEL_INFO;


#define USB_CTL_WAIT   500 

#ifndef URB_ASYNC_UNLINK
#define URB_ASYNC_UNLINK    0
#endif






static
VOID
s_nsInterruptUsbIoCompleteRead(
    IN struct urb *urb
    );


static
VOID
s_nsBulkInUsbIoCompleteRead(
    IN struct urb *urb
    );


static
VOID
s_nsBulkOutIoCompleteWrite(
    IN struct urb *urb
    );


static
VOID
s_nsControlInUsbIoCompleteRead(
    IN struct urb *urb
    );

static
VOID
s_nsControlInUsbIoCompleteWrite(
    IN struct urb *urb
    );







NTSTATUS
PIPEnsControlOutAsyn(
    IN PSDevice     pDevice,
    IN BYTE         byRequest,
    IN WORD         wValue,
    IN WORD         wIndex,
    IN WORD         wLength,
    IN PBYTE        pbyBuffer
    )
{
    NTSTATUS                ntStatus;


    if (MP_TEST_FLAG(pDevice, fMP_DISCONNECTED))
        return STATUS_FAILURE;


    if (MP_TEST_FLAG(pDevice, fMP_CONTROL_WRITES)) {
        return STATUS_FAILURE;
    }

    if (in_interrupt()) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"in_interrupt return ..byRequest %x\n", byRequest);
        return STATUS_FAILURE;
    }

    ntStatus = usb_control_msg(
                            pDevice->usb,
                            usb_sndctrlpipe(pDevice->usb , 0),
                            byRequest,
                            0x40, 
                            wValue,
                            wIndex,
                            (PVOID) pbyBuffer,
                            wLength,
                            HZ
                          );
    if (ntStatus >= 0) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"usb_sndctrlpipe ntStatus= %d\n", ntStatus);
        ntStatus = 0;
    } else {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"usb_sndctrlpipe fail, ntStatus= %d\n", ntStatus);
    }

    return ntStatus;
}





NTSTATUS
PIPEnsControlOut(
    IN PSDevice     pDevice,
    IN BYTE         byRequest,
    IN WORD         wValue,
    IN WORD         wIndex,
    IN WORD         wLength,
    IN PBYTE        pbyBuffer
    )
{
    NTSTATUS            ntStatus = 0;
    int ii;


    if (MP_TEST_FLAG(pDevice, fMP_DISCONNECTED))
        return STATUS_FAILURE;

    if (MP_TEST_FLAG(pDevice, fMP_CONTROL_WRITES)) {
        return STATUS_FAILURE;
    }

	pDevice->sUsbCtlRequest.bRequestType = 0x40;
	pDevice->sUsbCtlRequest.bRequest = byRequest;
	pDevice->sUsbCtlRequest.wValue = cpu_to_le16p(&wValue);
	pDevice->sUsbCtlRequest.wIndex = cpu_to_le16p(&wIndex);
	pDevice->sUsbCtlRequest.wLength = cpu_to_le16p(&wLength);
	pDevice->pControlURB->transfer_flags |= URB_ASYNC_UNLINK;
    pDevice->pControlURB->actual_length = 0;
    
  	usb_fill_control_urb(pDevice->pControlURB, pDevice->usb,
			 usb_sndctrlpipe(pDevice->usb , 0), (char *) &pDevice->sUsbCtlRequest,
			 pbyBuffer, wLength, s_nsControlInUsbIoCompleteWrite, pDevice);

	if ((ntStatus = usb_submit_urb(pDevice->pControlURB, GFP_ATOMIC)) != 0) {
		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"control send request submission failed: %d\n", ntStatus);
		return STATUS_FAILURE;
	}
	else {
	    MP_SET_FLAG(pDevice, fMP_CONTROL_WRITES);
	}
	spin_unlock_irq(&pDevice->lock);
    for (ii = 0; ii <= USB_CTL_WAIT; ii ++) {
        if (MP_TEST_FLAG(pDevice, fMP_CONTROL_WRITES))
            mdelay(1);
        else
            break;
        if (ii >= USB_CTL_WAIT) {
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"control send request submission timeout \n");
            spin_lock_irq(&pDevice->lock);
            MP_CLEAR_FLAG(pDevice, fMP_CONTROL_WRITES);
            return STATUS_FAILURE;
        }
    }
	spin_lock_irq(&pDevice->lock);

    return STATUS_SUCCESS;
}




NTSTATUS
PIPEnsControlIn(
    IN PSDevice     pDevice,
    IN BYTE         byRequest,
    IN WORD         wValue,
    IN WORD         wIndex,
    IN WORD         wLength,
    IN OUT  PBYTE   pbyBuffer
    )
{
    NTSTATUS            ntStatus = 0;
    int ii;

    if (MP_TEST_FLAG(pDevice, fMP_DISCONNECTED))
        return STATUS_FAILURE;

    if (MP_TEST_FLAG(pDevice, fMP_CONTROL_READS)) {
        return STATUS_FAILURE;
    }
	pDevice->sUsbCtlRequest.bRequestType = 0xC0;
	pDevice->sUsbCtlRequest.bRequest = byRequest;
	pDevice->sUsbCtlRequest.wValue = cpu_to_le16p(&wValue);
	pDevice->sUsbCtlRequest.wIndex = cpu_to_le16p(&wIndex);
	pDevice->sUsbCtlRequest.wLength = cpu_to_le16p(&wLength);
	pDevice->pControlURB->transfer_flags |= URB_ASYNC_UNLINK;
    pDevice->pControlURB->actual_length = 0;
	usb_fill_control_urb(pDevice->pControlURB, pDevice->usb,
			 usb_rcvctrlpipe(pDevice->usb , 0), (char *) &pDevice->sUsbCtlRequest,
			 pbyBuffer, wLength, s_nsControlInUsbIoCompleteRead, pDevice);

	if ((ntStatus = usb_submit_urb(pDevice->pControlURB, GFP_ATOMIC)) != 0) {
		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"control request submission failed: %d\n", ntStatus);
	}else {
		MP_SET_FLAG(pDevice, fMP_CONTROL_READS);
    }

	spin_unlock_irq(&pDevice->lock);
    for (ii = 0; ii <= USB_CTL_WAIT; ii ++) {
        if (MP_TEST_FLAG(pDevice, fMP_CONTROL_READS))
            mdelay(1);
        else {
            break;
        }
        if (ii >= USB_CTL_WAIT) {
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"control rcv request submission timeout \n");
            spin_lock_irq(&pDevice->lock);
            MP_CLEAR_FLAG(pDevice, fMP_CONTROL_READS);
            return STATUS_FAILURE;
        }
    }
	spin_lock_irq(&pDevice->lock);

    return ntStatus;
}

static
VOID
s_nsControlInUsbIoCompleteWrite(
    IN struct urb *urb
    )
{
    PSDevice        pDevice;

	pDevice = urb->context;
	switch (urb->status) {
	case 0:
		break;
	case -EINPROGRESS:
		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ctrl write urb status EINPROGRESS%d\n", urb->status);
		break;
	case -ENOENT:
		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ctrl write urb status ENOENT %d\n", urb->status);
		break;
	default:
		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ctrl write urb status %d\n", urb->status);
	}

    MP_CLEAR_FLAG(pDevice, fMP_CONTROL_WRITES);
}




static
VOID
s_nsControlInUsbIoCompleteRead(
    IN struct urb *urb
    )
{
    PSDevice        pDevice;

	pDevice = urb->context;
	switch (urb->status) {
	case 0:
		break;
	case -EINPROGRESS:
		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ctrl read urb status EINPROGRESS%d\n", urb->status);
		break;
	case -ENOENT:
		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ctrl read urb status = ENOENT %d\n", urb->status);
		break;
	default:
		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ctrl read urb status %d\n", urb->status);
	}

    MP_CLEAR_FLAG(pDevice, fMP_CONTROL_READS);
}





NTSTATUS
PIPEnsInterruptRead(
    IN PSDevice pDevice
    )
{
    NTSTATUS            ntStatus = STATUS_FAILURE;


    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"---->s_nsStartInterruptUsbRead()\n");

    if(pDevice->intBuf.bInUse == TRUE){
        return (STATUS_FAILURE);
    }
    pDevice->intBuf.bInUse = TRUE;

    pDevice->ulIntInPosted++;

    
    
    
    
#if 0            
	usb_fill_int_urb(pDevice->pInterruptURB,
	                 pDevice->usb,
	                 usb_rcvintpipe(pDevice->usb, 1),
	                 (PVOID) pDevice->intBuf.pDataBuf,
	                 MAX_INTERRUPT_SIZE,
	                 s_nsInterruptUsbIoCompleteRead,
	                 pDevice,
	                 pDevice->int_interval
	                 );
#else            
#ifndef Safe_Close
	usb_fill_int_urb(pDevice->pInterruptURB,
	                 pDevice->usb,
	                 usb_rcvintpipe(pDevice->usb, 1),
	                 (PVOID) pDevice->intBuf.pDataBuf,
	                 MAX_INTERRUPT_SIZE,
	                 s_nsInterruptUsbIoCompleteRead,
	                 pDevice,
	                 pDevice->int_interval
	                 );
#else

    pDevice->pInterruptURB->interval = pDevice->int_interval;

usb_fill_bulk_urb(pDevice->pInterruptURB,
		pDevice->usb,
		usb_rcvbulkpipe(pDevice->usb, 1),
		(PVOID) pDevice->intBuf.pDataBuf,
		MAX_INTERRUPT_SIZE,
		s_nsInterruptUsbIoCompleteRead,
		pDevice);
#endif
#endif

	if ((ntStatus = usb_submit_urb(pDevice->pInterruptURB, GFP_ATOMIC)) != 0) {
	    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Submit int URB failed %d\n", ntStatus);
    }

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"<----s_nsStartInterruptUsbRead Return(%x)\n",ntStatus);
    return ntStatus;
}



static
VOID
s_nsInterruptUsbIoCompleteRead(
    IN struct urb *urb
    )

{
    PSDevice        pDevice;
    NTSTATUS        ntStatus;


    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"---->s_nsInterruptUsbIoCompleteRead\n");
    
    
    
    pDevice = (PSDevice)urb->context;

    
    
    
    
    
    
    
    
    ntStatus = urb->status;

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"s_nsInterruptUsbIoCompleteRead Status %d\n", ntStatus);

    
    
    if (( ntStatus != STATUS_SUCCESS )) {
        pDevice->ulBulkInError++;
        pDevice->intBuf.bInUse = FALSE;







            pDevice->fKillEventPollingThread = TRUE;

        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"IntUSBIoCompleteControl STATUS = %d\n", ntStatus );
    }
    else {
        pDevice->ulIntInBytesRead += (ULONG)urb->actual_length;
        pDevice->ulIntInContCRCError = 0;
        pDevice->bEventAvailable = TRUE;
        INTnsProcessData(pDevice);
    }

    STAvUpdateUSBCounter(&pDevice->scStatistic.USB_InterruptStat, ntStatus);


    if (pDevice->fKillEventPollingThread != TRUE) {
   #if 0               
	if ((ntStatus = usb_submit_urb(urb, GFP_ATOMIC)) != 0) {
	    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Re-Submit int URB failed %d\n", ntStatus);
    }
   #else                                                                                     
    #ifdef Safe_Close
       usb_fill_bulk_urb(pDevice->pInterruptURB,
		      pDevice->usb,
		      usb_rcvbulkpipe(pDevice->usb, 1),
		     (PVOID) pDevice->intBuf.pDataBuf,
		     MAX_INTERRUPT_SIZE,
		     s_nsInterruptUsbIoCompleteRead,
		     pDevice);

	if ((ntStatus = usb_submit_urb(pDevice->pInterruptURB, GFP_ATOMIC)) != 0) {
	    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Submit int URB failed %d\n", ntStatus);
           }

    #else
        tasklet_schedule(&pDevice->EventWorkItem);
    #endif
#endif
    }
    
    
    
    
    return ;
}


NTSTATUS
PIPEnsBulkInUsbRead(
    IN PSDevice pDevice,
    IN PRCB     pRCB
    )
{
    NTSTATUS            ntStatus= 0;
    struct urb          *pUrb;


    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"---->s_nsStartBulkInUsbRead\n");

    if (MP_TEST_FLAG(pDevice, fMP_DISCONNECTED))
        return STATUS_FAILURE;

    pDevice->ulBulkInPosted++;


	pUrb = pRCB->pUrb;
    
    
    
    
    if (pRCB->skb == NULL) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pRCB->skb is null \n");
        return ntStatus;
    }

	usb_fill_bulk_urb(pUrb,
		pDevice->usb,
		usb_rcvbulkpipe(pDevice->usb, 2),
		(PVOID) (pRCB->skb->data),
		MAX_TOTAL_SIZE_WITH_ALL_HEADERS,
		s_nsBulkInUsbIoCompleteRead,
		pRCB);

	if((ntStatus = usb_submit_urb(pUrb, GFP_ATOMIC)) != 0){
		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Submit Rx URB failed %d\n", ntStatus);
		return STATUS_FAILURE ;
	}
    pRCB->Ref = 1;
    pRCB->bBoolInUse= TRUE;

    return ntStatus;
}





static
VOID
s_nsBulkInUsbIoCompleteRead(
    IN struct urb *urb
    )

{
    PRCB    pRCB = (PRCB)urb->context;
    PSDevice pDevice = (PSDevice)pRCB->pDevice;
    ULONG   bytesRead;
    BOOL    bIndicateReceive = FALSE;
    BOOL    bReAllocSkb = FALSE;
    NTSTATUS    status;



    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"---->s_nsBulkInUsbIoCompleteRead\n");
    status = urb->status;
    bytesRead = urb->actual_length;

    if (status) {
        pDevice->ulBulkInError++;
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"BULK In failed %d\n", status);

     	#ifdef Calcu_LinkQual
           pDevice->scStatistic.RxFcsErrCnt ++;
	#endif








    } else {
        bIndicateReceive = TRUE;
        pDevice->ulBulkInContCRCError = 0;
        pDevice->ulBulkInBytesRead += bytesRead;

	#ifdef Calcu_LinkQual
           pDevice->scStatistic.RxOkCnt ++;
	#endif
    }


    STAvUpdateUSBCounter(&pDevice->scStatistic.USB_BulkInStat, status);

    if (bIndicateReceive) {
        spin_lock(&pDevice->lock);
        if (RXbBulkInProcessData(pDevice, pRCB, bytesRead) == TRUE)
            bReAllocSkb = TRUE;
        spin_unlock(&pDevice->lock);
    }
    pRCB->Ref--;
    if (pRCB->Ref == 0)
    {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"RxvFreeNormal %d \n",pDevice->NumRecvFreeList);
        spin_lock(&pDevice->lock);
        RXvFreeRCB(pRCB, bReAllocSkb);
        spin_unlock(&pDevice->lock);
    }


    return;
}


NDIS_STATUS
PIPEnsSendBulkOut(
    IN  PSDevice pDevice,
    IN  PUSB_SEND_CONTEXT pContext
    )
{
    NTSTATUS            status;
    struct urb          *pUrb;



    pDevice->bPWBitOn = FALSE;



    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"s_nsSendBulkOut\n");

    if(MP_IS_READY(pDevice) && MP_TEST_FLAG(pDevice, fMP_POST_WRITES)) {

        pUrb = pContext->pUrb;
        pDevice->ulBulkOutPosted++;

        usb_fill_bulk_urb(
        	    pUrb,
        		pDevice->usb,
        		usb_sndbulkpipe(pDevice->usb, 3),
        		(PVOID) &(pContext->Data[0]),
        		pContext->uBufLen,
        		s_nsBulkOutIoCompleteWrite,
        		pContext);

    	if((status = usb_submit_urb(pUrb, GFP_ATOMIC))!=0)
    	{
    		DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Submit Tx URB failed %d\n", status);
    		return STATUS_FAILURE;
    	}
        return STATUS_PENDING;
    }
    else {
        pContext->bBoolInUse = FALSE;
        return STATUS_RESOURCES;
    }
}


static
VOID
s_nsBulkOutIoCompleteWrite(
    IN struct urb *urb
    )
{
    PSDevice            pDevice;
    NTSTATUS            status;
    CONTEXT_TYPE        ContextType;
    ULONG               ulBufLen;
    PUSB_SEND_CONTEXT   pContext;


    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"---->s_nsBulkOutIoCompleteWrite\n");
    
    
    
    pContext = (PUSB_SEND_CONTEXT) urb->context;
    ASSERT( NULL != pContext );

    pDevice = pContext->pDevice;
    ContextType = pContext->Type;
    ulBufLen = pContext->uBufLen;

    if (!netif_device_present(pDevice->dev))
	    return;

   
    
    

    status = urb->status;
    
    STAvUpdateUSBCounter(&pDevice->scStatistic.USB_BulkOutStat, status);

    if(status == STATUS_SUCCESS) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Write %d bytes\n",(int)ulBufLen);
        pDevice->ulBulkOutBytesWrite += ulBufLen;
        pDevice->ulBulkOutContCRCError = 0;
	
           #ifdef TxInSleep
             pDevice->nTxDataTimeCout = 0;
           #endif

    } else {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"BULK Out failed %d\n", status);
        pDevice->ulBulkOutError++;
    }




    if ( CONTEXT_DATA_PACKET == ContextType ) {
        
        
	    if (pContext->pPacket != NULL) {
	        dev_kfree_skb_irq(pContext->pPacket);
	        pContext->pPacket = NULL;
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"tx  %d bytes\n",(int)ulBufLen);
	    }

        pDevice->dev->trans_start = jiffies;


        if (status == STATUS_SUCCESS) {
            pDevice->packetsSent++;
        }
        else {
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Send USB error! [%08xh]\n", status);
            pDevice->packetsSentDropped++;
        }

    }
    if (pDevice->bLinkPass == TRUE) {
        if (netif_queue_stopped(pDevice->dev))
            netif_wake_queue(pDevice->dev);
    }
    pContext->bBoolInUse = FALSE;

    return;
}
