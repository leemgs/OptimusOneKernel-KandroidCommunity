

struct pr_ram {
  word NextReq;         
  word NextRc;          
  word NextInd;         
  byte ReqInput;        
  byte ReqOutput;       
  byte ReqReserved;     
  byte Int;             
  byte XLock;           
  byte RcOutput;        
  byte IndOutput;       
  byte IMask;           
  byte Reserved1[2];    
  byte ReadyInt;        
  byte Reserved2[12];   
  byte InterfaceType;   
  word Signature;       
  byte B[1];            
};
typedef struct {
  word next;
  byte Req;
  byte ReqId;
  byte ReqCh;
  byte Reserved1;
  word Reference;
  byte Reserved[8];
  PBUFFER XBuffer;
} REQ;
typedef struct {
  word next;
  byte Rc;
  byte RcId;
  byte RcCh;
  byte Reserved1;
  word Reference;
  byte Reserved2[8];
} RC;
typedef struct {
  word next;
  byte Ind;
  byte IndId;
  byte IndCh;
  byte MInd;
  word MLength;
  word Reference;
  byte RNR;
  byte Reserved;
  dword Ack;
  PBUFFER RBuffer;
} IND;
