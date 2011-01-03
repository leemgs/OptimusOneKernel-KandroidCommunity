

#ifndef _VCD_ERR_STATUS_H_
#define _VCD_ERR_STATUS_H_

#define VCD_EVT_RESP_BASE              0x1000
#define VCD_EVT_RESP_OPEN              (VCD_EVT_RESP_BASE + 0x1)
#define VCD_EVT_RESP_START             (VCD_EVT_RESP_BASE + 0x2)
#define VCD_EVT_RESP_STOP              (VCD_EVT_RESP_BASE + 0x3)
#define VCD_EVT_RESP_PAUSE             (VCD_EVT_RESP_BASE + 0x4)
#define VCD_EVT_RESP_FLUSH_INPUT_DONE  (VCD_EVT_RESP_BASE + 0x5)
#define VCD_EVT_RESP_FLUSH_OUTPUT_DONE (VCD_EVT_RESP_BASE + 0x6)
#define VCD_EVT_RESP_INPUT_FLUSHED     (VCD_EVT_RESP_BASE + 0x7)
#define VCD_EVT_RESP_OUTPUT_FLUSHED    (VCD_EVT_RESP_BASE + 0x8)
#define VCD_EVT_RESP_INPUT_DONE        (VCD_EVT_RESP_BASE + 0x9)
#define VCD_EVT_RESP_OUTPUT_DONE       (VCD_EVT_RESP_BASE + 0xa)

#define VCD_EVT_IND_BASE               0x2000
#define VCD_EVT_IND_INPUT_RECONFIG     (VCD_EVT_IND_BASE + 0x1)
#define VCD_EVT_IND_OUTPUT_RECONFIG    (VCD_EVT_IND_BASE + 0x2)
#define VCD_EVT_IND_HWERRFATAL         (VCD_EVT_IND_BASE + 0x3)
#define VCD_EVT_IND_RESOURCES_LOST     (VCD_EVT_IND_BASE + 0x4)

#define VCD_S_SUCCESS           0x0

#define VCD_S_ERR_BASE              0x80000000
#define VCD_ERR_FAIL                (VCD_S_ERR_BASE + 0x1)
#define VCD_ERR_ALLOC_FAIL          (VCD_S_ERR_BASE + 0x2)
#define VCD_ERR_ILLEGAL_OP          (VCD_S_ERR_BASE + 0x3)
#define VCD_ERR_ILLEGAL_PARM        (VCD_S_ERR_BASE + 0x4)
#define VCD_ERR_BAD_POINTER         (VCD_S_ERR_BASE + 0x5)
#define VCD_ERR_BAD_HANDLE          (VCD_S_ERR_BASE + 0x6)
#define VCD_ERR_NOT_SUPPORTED       (VCD_S_ERR_BASE + 0x7)
#define VCD_ERR_BAD_STATE           (VCD_S_ERR_BASE + 0x8)
#define VCD_ERR_BUSY                (VCD_S_ERR_BASE + 0x9)
#define VCD_ERR_MAX_CLIENT          (VCD_S_ERR_BASE + 0xa)
#define VCD_ERR_IFRAME_EXPECTED     (VCD_S_ERR_BASE + 0xb)
#define VCD_ERR_INTRLCD_FIELD_DROP  (VCD_S_ERR_BASE + 0xc)
#define VCD_ERR_HW_FATAL            (VCD_S_ERR_BASE + 0xd)
#define VCD_ERR_BITSTREAM_ERR       (VCD_S_ERR_BASE + 0xe)
#define VCD_ERR_INDEX_NOMORE        (VCD_S_ERR_BASE + 0xf)
#define VCD_FAILED(rc)   ((rc > VCD_S_ERR_BASE) ? TRUE : FALSE)

#endif
