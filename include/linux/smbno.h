#ifndef _SMBNO_H_
#define _SMBNO_H_


#define aRONLY	(1L<<0)
#define aHIDDEN	(1L<<1)
#define aSYSTEM	(1L<<2)
#define aVOLID	(1L<<3)
#define aDIR	(1L<<4)
#define aARCH	(1L<<5)


#define SUCCESS 0  
#define ERRDOS 0x01 
#define ERRSRV 0x02 
#define ERRHRD 0x03  
#define ERRCMD 0xFF  



#define ERRbadfunc 1            
#define ERRbadfile 2            
#define ERRbadpath 3            
#define ERRnofids 4             
#define ERRnoaccess 5           
#define ERRbadfid 6             
#define ERRbadmcb 7             
#define ERRnomem 8              
#define ERRbadmem 9             
#define ERRbadenv 10            
#define ERRbadformat 11         
#define ERRbadaccess 12         
#define ERRbaddata 13           
#define ERRres 14               
#define ERRbaddrive 15          
#define ERRremcd 16             
#define ERRdiffdevice 17        
#define ERRnofiles 18           
#define ERRbadshare 32          
#define ERRlock 33              
#define ERRfilexists 80         
#define ERRbadpipe 230          
#define ERRpipebusy 231         
#define ERRpipeclosing 232      
#define ERRnotconnected 233     
#define ERRmoredata 234         

#define ERROR_INVALID_PARAMETER	 87
#define ERROR_DISK_FULL		112
#define ERROR_INVALID_NAME	123
#define ERROR_DIR_NOT_EMPTY	145
#define ERROR_NOT_LOCKED	158
#define ERROR_ALREADY_EXISTS	183  
#define ERROR_EAS_DIDNT_FIT	275 
#define ERROR_EAS_NOT_SUPPORTED	282 



#define ERRerror 1              
#define ERRbadpw 2              
#define ERRbadtype 3            
#define ERRaccess 4          
#define ERRinvnid 5             
#define ERRinvnetname 6         
#define ERRinvdevice 7          
#define ERRqfull 49             
#define ERRqtoobig 50           
#define ERRinvpfid 52           
#define ERRsmbcmd 64            
#define ERRsrverror 65          
#define ERRfilespecs 67         
#define ERRbadlink 68           
#define ERRbadpermits 69        
#define ERRbadpid 70            
#define ERRsetattrmode 71       
#define ERRpaused 81            
#define ERRmsgoff 82            
#define ERRnoroom 83            
#define ERRrmuns 87             
#define ERRtimeout 88           
#define ERRnoresource  89   
#define ERRtoomanyuids 90       
#define ERRbaduid 91            
#define ERRuseMPX 250    
#define ERRuseSTD 251    
#define ERRcontMPX 252          
#define ERRbadPW                
#define ERRnosupport 0xFFFF



#define ERRnowrite 19           
#define ERRbadunit 20           
#define ERRnotready 21          
#define ERRbadcmd 22            
#define ERRdata 23              
#define ERRbadreq 24            
#define ERRseek 25
#define ERRbadmedia 26
#define ERRbadsector 27
#define ERRnopaper 28
#define ERRwrite 29             
#define ERRread 30              
#define ERRgeneral 31           
#define ERRwrongdisk 34
#define ERRFCBunavail 35
#define ERRsharebufexc 36       
#define ERRdiskfull 39


#define SMB_ACCMASK	0x0003
#define SMB_O_RDONLY	0x0000
#define SMB_O_WRONLY	0x0001
#define SMB_O_RDWR	0x0002


#define smb_com 8
#define smb_rcls 9
#define smb_reh 10
#define smb_err 11
#define smb_flg 13
#define smb_flg2 14
#define smb_reb 13
#define smb_tid 28
#define smb_pid 30
#define smb_uid 32
#define smb_mid 34
#define smb_wct 36
#define smb_vwv 37
#define smb_vwv0 37
#define smb_vwv1 39
#define smb_vwv2 41
#define smb_vwv3 43
#define smb_vwv4 45
#define smb_vwv5 47
#define smb_vwv6 49
#define smb_vwv7 51
#define smb_vwv8 53
#define smb_vwv9 55
#define smb_vwv10 57
#define smb_vwv11 59
#define smb_vwv12 61
#define smb_vwv13 63
#define smb_vwv14 65


#define smb_tpscnt smb_vwv0
#define smb_tdscnt smb_vwv1
#define smb_mprcnt smb_vwv2
#define smb_mdrcnt smb_vwv3
#define smb_msrcnt smb_vwv4
#define smb_flags smb_vwv5
#define smb_timeout smb_vwv6
#define smb_pscnt smb_vwv9
#define smb_psoff smb_vwv10
#define smb_dscnt smb_vwv11
#define smb_dsoff smb_vwv12
#define smb_suwcnt smb_vwv13
#define smb_setup smb_vwv14
#define smb_setup0 smb_setup
#define smb_setup1 (smb_setup+2)
#define smb_setup2 (smb_setup+4)


#define smb_spscnt smb_vwv2
#define smb_spsoff smb_vwv3
#define smb_spsdisp smb_vwv4
#define smb_sdscnt smb_vwv5
#define smb_sdsoff smb_vwv6
#define smb_sdsdisp smb_vwv7
#define smb_sfid smb_vwv8


#define smb_tprcnt smb_vwv0
#define smb_tdrcnt smb_vwv1
#define smb_prcnt smb_vwv3
#define smb_proff smb_vwv4
#define smb_prdisp smb_vwv5
#define smb_drcnt smb_vwv6
#define smb_droff smb_vwv7
#define smb_drdisp smb_vwv8


#define SMBmkdir      0x00   
#define SMBrmdir      0x01   
#define SMBopen       0x02   
#define SMBcreate     0x03   
#define SMBclose      0x04   
#define SMBflush      0x05   
#define SMBunlink     0x06   
#define SMBmv         0x07   
#define SMBgetatr     0x08   
#define SMBsetatr     0x09   
#define SMBread       0x0A   
#define SMBwrite      0x0B   
#define SMBlock       0x0C   
#define SMBunlock     0x0D   
#define SMBctemp      0x0E   
#define SMBmknew      0x0F   
#define SMBchkpth     0x10   
#define SMBexit       0x11   
#define SMBlseek      0x12   
#define SMBtcon       0x70   
#define SMBtconX      0x75   
#define SMBtdis       0x71   
#define SMBnegprot    0x72   
#define SMBdskattr    0x80   
#define SMBsearch     0x81   
#define SMBsplopen    0xC0   
#define SMBsplwr      0xC1   
#define SMBsplclose   0xC2   
#define SMBsplretq    0xC3   
#define SMBsends      0xD0   
#define SMBsendb      0xD1   
#define SMBfwdname    0xD2   
#define SMBcancelf    0xD3   
#define SMBgetmac     0xD4   
#define SMBsendstrt   0xD5   
#define SMBsendend    0xD6   
#define SMBsendtxt    0xD7   


#define SMBlockread	  0x13   
#define SMBwriteunlock 0x14 
#define SMBreadbraw   0x1a  
#define SMBwritebraw  0x1d  
#define SMBwritec     0x20  
#define SMBwriteclose 0x2c  


#define SMBreadBraw      0x1A   
#define SMBreadBmpx      0x1B   
#define SMBreadBs        0x1C   
#define SMBwriteBraw     0x1D   
#define SMBwriteBmpx     0x1E   
#define SMBwriteBs       0x1F   
#define SMBwriteC        0x20   
#define SMBsetattrE      0x22   
#define SMBgetattrE      0x23   
#define SMBlockingX      0x24   
#define SMBtrans         0x25   
#define SMBtranss        0x26   
#define SMBioctl         0x27   
#define SMBioctls        0x28   
#define SMBcopy          0x29   
#define SMBmove          0x2A   
#define SMBecho          0x2B   
#define SMBopenX         0x2D   
#define SMBreadX         0x2E   
#define SMBwriteX        0x2F   
#define SMBsesssetupX    0x73   
#define SMBtconX         0x75   
#define SMBffirst        0x82   
#define SMBfunique       0x83   
#define SMBfclose        0x84   
#define SMBinvalid       0xFE   



#define SMBtrans2        0x32   
#define SMBtranss2       0x33   
#define SMBfindclose     0x34   
#define SMBfindnclose    0x35   
#define SMBulogoffX      0x74   


#define TRANSACT2_OPEN          0
#define TRANSACT2_FINDFIRST     1
#define TRANSACT2_FINDNEXT      2
#define TRANSACT2_QFSINFO       3
#define TRANSACT2_SETFSINFO     4
#define TRANSACT2_QPATHINFO     5
#define TRANSACT2_SETPATHINFO   6
#define TRANSACT2_QFILEINFO     7
#define TRANSACT2_SETFILEINFO   8
#define TRANSACT2_FSCTL         9
#define TRANSACT2_IOCTL           10
#define TRANSACT2_FINDNOTIFYFIRST 11
#define TRANSACT2_FINDNOTIFYNEXT  12
#define TRANSACT2_MKDIR           13


#define SMB_INFO_STANDARD		1
#define SMB_INFO_QUERY_EA_SIZE		2
#define SMB_INFO_QUERY_EAS_FROM_LIST	3
#define SMB_INFO_QUERY_ALL_EAS		4
#define SMB_INFO_IS_NAME_VALID		6


#define SMB_FIND_FILE_DIRECTORY_INFO		0x101
#define SMB_FIND_FILE_FULL_DIRECTORY_INFO	0x102
#define SMB_FIND_FILE_NAMES_INFO		0x103
#define SMB_FIND_FILE_BOTH_DIRECTORY_INFO	0x104


#define SMB_QUERY_FILE_BASIC_INFO	0x101
#define SMB_QUERY_FILE_STANDARD_INFO	0x102
#define SMB_QUERY_FILE_EA_INFO		0x103
#define SMB_QUERY_FILE_NAME_INFO	0x104
#define SMB_QUERY_FILE_ALL_INFO		0x107
#define SMB_QUERY_FILE_ALT_NAME_INFO	0x108
#define SMB_QUERY_FILE_STREAM_INFO	0x109
#define SMB_QUERY_FILE_COMPRESSION_INFO	0x10b


#define SMB_SET_FILE_BASIC_INFO		0x101
#define SMB_SET_FILE_DISPOSITION_INFO	0x102
#define SMB_SET_FILE_ALLOCATION_INFO	0x103
#define SMB_SET_FILE_END_OF_FILE_INFO	0x104


#define SMB_FLAGS_SUPPORT_LOCKREAD	0x01
#define SMB_FLAGS_CLIENT_BUF_AVAIL	0x02
#define SMB_FLAGS_RESERVED		0x04
#define SMB_FLAGS_CASELESS_PATHNAMES	0x08
#define SMB_FLAGS_CANONICAL_PATHNAMES	0x10
#define SMB_FLAGS_REQUEST_OPLOCK	0x20
#define SMB_FLAGS_REQUEST_BATCH_OPLOCK	0x40
#define SMB_FLAGS_REPLY			0x80


#define SMB_FLAGS2_LONG_PATH_COMPONENTS		0x0001
#define SMB_FLAGS2_EXTENDED_ATTRIBUTES		0x0002
#define SMB_FLAGS2_DFS_PATHNAMES		0x1000
#define SMB_FLAGS2_READ_PERMIT_NO_EXECUTE	0x2000
#define SMB_FLAGS2_32_BIT_ERROR_CODES		0x4000 
#define SMB_FLAGS2_UNICODE_STRINGS		0x8000



#define MIN_UNIX_INFO_LEVEL		0x200
#define MAX_UNIX_INFO_LEVEL		0x2FF
#define SMB_FIND_FILE_UNIX		0x202
#define SMB_QUERY_FILE_UNIX_BASIC	0x200
#define SMB_QUERY_FILE_UNIX_LINK	0x201
#define SMB_QUERY_FILE_UNIX_HLINK	0x202
#define SMB_SET_FILE_UNIX_BASIC		0x200
#define SMB_SET_FILE_UNIX_LINK		0x201
#define SMB_SET_FILE_UNIX_HLINK		0x203
#define SMB_QUERY_CIFS_UNIX_INFO	0x200


#define SMB_MODE_NO_CHANGE		0xFFFFFFFF
#define SMB_UID_NO_CHANGE		0xFFFFFFFF
#define SMB_GID_NO_CHANGE		0xFFFFFFFF
#define SMB_TIME_NO_CHANGE		0xFFFFFFFFFFFFFFFFULL
#define SMB_SIZE_NO_CHANGE		0xFFFFFFFFFFFFFFFFULL


#define UNIX_TYPE_FILE		0
#define UNIX_TYPE_DIR		1
#define UNIX_TYPE_SYMLINK	2
#define UNIX_TYPE_CHARDEV	3
#define UNIX_TYPE_BLKDEV	4
#define UNIX_TYPE_FIFO		5
#define UNIX_TYPE_SOCKET	6
#define UNIX_TYPE_UNKNOWN	0xFFFFFFFF

#endif 
