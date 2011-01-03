

#ifndef	uint8
#define	uint8  unsigned	char
#endif

#ifndef	uint32
#define	uint32 unsigned	long int
#endif


#ifndef	__MD5_H__
#define	__MD5_H__

#define MD5_MAC_LEN 16

typedef struct _MD5_CTX {
    UINT32   Buf[4];             
	UCHAR   Input[64];          
	UINT32   LenInBitCount[2];   
}   MD5_CTX;

VOID MD5Init(MD5_CTX *pCtx);
VOID MD5Update(MD5_CTX *pCtx, UCHAR *pData, UINT32 LenInBytes);
VOID MD5Final(UCHAR Digest[16], MD5_CTX *pCtx);
VOID MD5Transform(UINT32 Buf[4], UINT32 Mes[16]);

void md5_mac(u8 *key, size_t key_len, u8 *data, size_t data_len, u8 *mac);
void hmac_md5(u8 *key, size_t key_len, u8 *data, size_t data_len, u8 *mac);




typedef	struct _SHA_CTX
{
	UINT32   Buf[5];             
	UCHAR   Input[80];          
	UINT32   LenInBitCount[2];   

}	SHA_CTX;

VOID SHAInit(SHA_CTX *pCtx);
UCHAR SHAUpdate(SHA_CTX *pCtx, UCHAR *pData, UINT32 LenInBytes);
VOID SHAFinal(SHA_CTX *pCtx, UCHAR Digest[20]);
VOID SHATransform(UINT32 Buf[5], UINT32 Mes[20]);

#define SHA_DIGEST_LEN 20
#endif 


#ifndef	_AES_H
#define	_AES_H

typedef	struct
{
	uint32 erk[64];		
	uint32 drk[64];		
	int	nr;				
}
aes_context;

int	 rtmp_aes_set_key( aes_context *ctx,	uint8 *key,	int	nbits );
void rtmp_aes_encrypt( aes_context *ctx,	uint8 input[16], uint8 output[16] );
void rtmp_aes_decrypt( aes_context *ctx,	uint8 input[16], uint8 output[16] );

void F(char *password, unsigned char *ssid, int ssidlength, int iterations, int count, unsigned char *output);
int PasswordHash(char *password, unsigned char *ssid, int ssidlength, unsigned char *output);

#endif 

