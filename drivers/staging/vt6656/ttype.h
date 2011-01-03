


#ifndef __TTYPE_H__
#define __TTYPE_H__




#ifndef VOID
#define VOID            void
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif


#ifndef TxInSleep
#define TxInSleep
#endif





#ifndef Safe_Close
#define Safe_Close
#endif


#ifndef Adhoc_STA
#define Adhoc_STA
#endif

typedef int             BOOL;

#if !defined(TRUE)
#define TRUE            1
#endif
#if !defined(FALSE)
#define FALSE           0
#endif


#if !defined(SUCCESS)
#define SUCCESS         0
#endif


#ifndef  update_BssList
#define update_BssList
#endif

#ifndef WPA_SM_Transtatus
#define WPA_SM_Transtatus
#endif

#ifndef Calcu_LinkQual
#define Calcu_LinkQual
#endif





typedef signed char             I8;     

typedef unsigned char           U8;     
typedef unsigned short          U16;    
typedef unsigned long           U32;    


typedef char            CHAR;
typedef signed short    SHORT;
typedef signed int      INT;
typedef signed long     LONG;

typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef unsigned int    UINT;
typedef unsigned long   ULONG;
typedef unsigned long long	ULONGLONG; 



typedef unsigned char   BYTE;           
typedef unsigned short  WORD;           
typedef unsigned long   DWORD;          




typedef union tagUQuadWord {
    struct {
        DWORD   dwLowDword;
        DWORD   dwHighDword;
    } u;
    double      DoNotUseThisField;
} UQuadWord;
typedef UQuadWord       QWORD;          




typedef unsigned long   ULONG_PTR;      
typedef unsigned long   DWORD_PTR;      


typedef unsigned int *   PUINT;

typedef BYTE *           PBYTE;

typedef WORD *           PWORD;

typedef DWORD *          PDWORD;

typedef QWORD *          PQWORD;

typedef void *           PVOID;


#ifdef STRICT
typedef void *HANDLE;
#else
typedef PVOID HANDLE;
#endif

#endif 
