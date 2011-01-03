

#include "../rt_config.h"
#include "../mlme_ex_def.h"








VOID StateMachineInitEx(
	IN STATE_MACHINE_EX *S,
	IN STATE_MACHINE_FUNC_EX Trans[],
	IN ULONG StNr,
	IN ULONG MsgNr,
	IN STATE_MACHINE_FUNC_EX DefFunc,
	IN ULONG InitState,
	IN ULONG Base)
{
	ULONG i, j;

	
	S->NrState = StNr;
	S->NrMsg   = MsgNr;
	S->Base    = Base;

	S->TransFunc  = Trans;

	
	for (i = 0; i < StNr; i++)
	{
		for (j = 0; j < MsgNr; j++)
		{
			S->TransFunc[i * MsgNr + j] = DefFunc;
		}
	}

	
	S->CurrState = InitState;

	return;
}


VOID StateMachineSetActionEx(
	IN STATE_MACHINE_EX *S,
	IN ULONG St,
	IN ULONG Msg,
	IN STATE_MACHINE_FUNC_EX Func)
{
	ULONG MsgIdx;

	MsgIdx = Msg - S->Base;

	if (St < S->NrState && MsgIdx < S->NrMsg)
	{
		
		S->TransFunc[St * S->NrMsg + MsgIdx] = Func;
	}

	return;
}


VOID StateMachinePerformActionEx(
	IN PRTMP_ADAPTER	pAd,
	IN STATE_MACHINE_EX *S,
	IN MLME_QUEUE_ELEM *Elem,
	USHORT Idx,
	PULONG pCurrState)
{
	if (S->TransFunc[(*pCurrState) * S->NrMsg + Elem->MsgType - S->Base])
		(*(S->TransFunc[(*pCurrState) * S->NrMsg + Elem->MsgType - S->Base]))(pAd, Elem, pCurrState, Idx);

	return;
}


BOOLEAN MlmeEnqueueEx(
	IN	PRTMP_ADAPTER	pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN USHORT Idx)
{
	INT Tail;
	MLME_QUEUE *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		return FALSE;


	
	if (MsgLen > MAX_LEN_OF_MLME_BUFFER)
	{
		DBGPRINT_ERR(("MlmeEnqueueEx: msg too large, size = %ld \n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue))
	{

		return FALSE;
	}

	RTMP_SEM_LOCK(&Queue->Lock);
	Tail = Queue->Tail;
	Queue->Tail++;
	Queue->Num++;
	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
	{
		Queue->Tail = 0;
	}
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen = MsgLen;
	Queue->Entry[Tail].Idx = Idx;
	if (Msg != NULL)
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

	RTMP_SEM_UNLOCK(&Queue->Lock);

	return TRUE;
}


VOID DropEx(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem,
	PULONG pCurrState,
	USHORT Idx)
{
	return;
}
