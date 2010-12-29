
#ifndef UAMP_API_H
#define UAMP_API_H


#include "typedefs.h"




#define BT_API


typedef bool	BOOLEAN;
typedef uint8	UINT8;
typedef uint16	UINT16;



#define UAMP_ID_1   1
#define UAMP_ID_2   2
typedef UINT8 tUAMP_ID;


#define UAMP_EVT_RX_READY           0   
#define UAMP_EVT_CTLR_REMOVED       1   
#define UAMP_EVT_CTLR_READY         2   
typedef UINT8 tUAMP_EVT;



#define UAMP_CH_HCI_CMD            0   
#define UAMP_CH_HCI_EVT            1   
#define UAMP_CH_HCI_DATA           2   
typedef UINT8 tUAMP_CH;


typedef union {
    tUAMP_CH channel;       
} tUAMP_EVT_DATA;



typedef void (*tUAMP_CBACK)(tUAMP_ID amp_id, tUAMP_EVT amp_evt, tUAMP_EVT_DATA *p_amp_evt_data);


#ifdef __cplusplus
extern "C"
{
#endif


BT_API BOOLEAN UAMP_Init(tUAMP_CBACK p_cback);



BT_API BOOLEAN UAMP_Open(tUAMP_ID amp_id);


BT_API void UAMP_Close(tUAMP_ID amp_id);



BT_API UINT16 UAMP_Write(tUAMP_ID amp_id, UINT8 *p_buf, UINT16 num_bytes, tUAMP_CH channel);


BT_API UINT16 UAMP_Read(tUAMP_ID amp_id, UINT8 *p_buf, UINT16 buf_size, tUAMP_CH channel);

#ifdef __cplusplus
}
#endif

#endif 
