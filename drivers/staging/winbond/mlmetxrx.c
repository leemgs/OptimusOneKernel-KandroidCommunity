
















#include "sysdef.h"

#include "mds_f.h"


u8 MLMESendFrame(struct wbsoft_priv * adapter, u8 *pMMPDU, u16 len, u8 DataType)

{
	if (adapter->sMlmeFrame.IsInUsed != PACKET_FREE_TO_USE) {
		adapter->sMlmeFrame.wNumTxMMPDUDiscarded++;
		return false;
	}
	adapter->sMlmeFrame.IsInUsed = PACKET_COME_FROM_MLME;

	
	adapter->sMlmeFrame.pMMPDU = pMMPDU;
	adapter->sMlmeFrame.DataType = DataType;
	
	adapter->sMlmeFrame.len = len;
	adapter->sMlmeFrame.wNumTxMMPDU++;

	
	

	
	Mds_Tx(adapter);
	return true;
}

void MLME_GetNextPacket(struct wbsoft_priv *adapter, struct wb35_descriptor *desc)
{
	desc->InternalUsed = desc->buffer_start_index + desc->buffer_number;
	desc->InternalUsed %= MAX_DESCRIPTOR_BUFFER_INDEX;
	desc->buffer_address[desc->InternalUsed] = adapter->sMlmeFrame.pMMPDU;
	desc->buffer_size[desc->InternalUsed] = adapter->sMlmeFrame.len;
	desc->buffer_total_size += adapter->sMlmeFrame.len;
	desc->buffer_number++;
	desc->Type = adapter->sMlmeFrame.DataType;
}

static void MLMEfreeMMPDUBuffer(struct wbsoft_priv *adapter, s8 *pData)
{
	int i;

	
	for (i = 0; i < MAX_NUM_TX_MMPDU; i++) {
		if (pData == (s8 *)&(adapter->sMlmeFrame.TxMMPDU[i]))
			break;
	}
	if (adapter->sMlmeFrame.TxMMPDUInUse[i])
		adapter->sMlmeFrame.TxMMPDUInUse[i] = false;
	else  {
		
		
	}
}

void
MLME_SendComplete(struct wbsoft_priv * adapter, u8 PacketID, unsigned char SendOK)
{
	MLME_TXCALLBACK	TxCallback;

    
	adapter->sMlmeFrame.len = 0;
	MLMEfreeMMPDUBuffer( adapter, adapter->sMlmeFrame.pMMPDU );


	TxCallback.bResult = MLME_SUCCESS;

	
	adapter->sMlmeFrame.IsInUsed = PACKET_FREE_TO_USE;
}



