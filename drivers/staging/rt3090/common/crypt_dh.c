

#include "crypt_dh.h"
#include "crypt_biginteger.h"


void DH_PublicKey_Generate (
    IN UINT8 GValue[],
    IN UINT GValueLength,
    IN UINT8 PValue[],
    IN UINT PValueLength,
    IN UINT8 PrivateKey[],
    IN UINT PrivateKeyLength,
    OUT UINT8 PublicKey[],
    INOUT UINT *PublicKeyLength)
{
    PBIG_INTEGER pBI_G = NULL;
    PBIG_INTEGER pBI_P = NULL;
    PBIG_INTEGER pBI_PrivateKey = NULL;
    PBIG_INTEGER pBI_PublicKey = NULL;

    
    if (GValueLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: G length is (%d)\n", GValueLength));
        return;
    } 
    if (PValueLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: P length is (%d)\n", PValueLength));
        return;
    } 
    if (PrivateKeyLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: private key length is (%d)\n", PrivateKeyLength));
        return;
    } 
    if (*PublicKeyLength < PValueLength) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: public key length(%d) must be large or equal than P length(%d)\n",
            *PublicKeyLength, PValueLength));
        return;
    } 
    if (!(PValue[PValueLength - 1] & 0x1)) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_PublicKey_Generate: P value must be odd\n"));
        return;
    } 

    
    BigInteger_Init(&pBI_G);
    BigInteger_Init(&pBI_P);
    BigInteger_Init(&pBI_PrivateKey);
    BigInteger_Init(&pBI_PublicKey);
    BigInteger_Bin2BI(GValue, GValueLength, &pBI_G);
    BigInteger_Bin2BI(PValue, PValueLength, &pBI_P);
    BigInteger_Bin2BI(PrivateKey, PrivateKeyLength, &pBI_PrivateKey);

    
    BigInteger_Montgomery_ExpMod(pBI_G, pBI_PrivateKey, pBI_P, &pBI_PublicKey);

    
    BigInteger_BI2Bin(pBI_PublicKey, PublicKey, PublicKeyLength);

    BigInteger_Free(&pBI_G);
    BigInteger_Free(&pBI_P);
    BigInteger_Free(&pBI_PrivateKey);
    BigInteger_Free(&pBI_PublicKey);
} 



void DH_SecretKey_Generate (
    IN UINT8 PublicKey[],
    IN UINT PublicKeyLength,
    IN UINT8 PValue[],
    IN UINT PValueLength,
    IN UINT8 PrivateKey[],
    IN UINT PrivateKeyLength,
    OUT UINT8 SecretKey[],
    INOUT UINT *SecretKeyLength)
{
    PBIG_INTEGER pBI_P = NULL;
    PBIG_INTEGER pBI_SecretKey = NULL;
    PBIG_INTEGER pBI_PrivateKey = NULL;
    PBIG_INTEGER pBI_PublicKey = NULL;

    
    if (PublicKeyLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: public key length is (%d)\n", PublicKeyLength));
        return;
    } 
    if (PValueLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: P length is (%d)\n", PValueLength));
        return;
    } 
    if (PrivateKeyLength == 0) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: private key length is (%d)\n", PrivateKeyLength));
        return;
    } 
    if (*SecretKeyLength < PValueLength) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: secret key length(%d) must be large or equal than P length(%d)\n",
            *SecretKeyLength, PValueLength));
        return;
    } 
    if (!(PValue[PValueLength - 1] & 0x1)) {
	DBGPRINT(RT_DEBUG_ERROR, ("DH_SecretKey_Generate: P value must be odd\n"));
        return;
    } 

    
    BigInteger_Init(&pBI_P);
    BigInteger_Init(&pBI_PrivateKey);
    BigInteger_Init(&pBI_PublicKey);
    BigInteger_Init(&pBI_SecretKey);

    BigInteger_Bin2BI(PublicKey, PublicKeyLength, &pBI_PublicKey);
    BigInteger_Bin2BI(PValue, PValueLength, &pBI_P);
    BigInteger_Bin2BI(PrivateKey, PrivateKeyLength, &pBI_PrivateKey);

    
    BigInteger_Montgomery_ExpMod(pBI_PublicKey, pBI_PrivateKey, pBI_P, &pBI_SecretKey);

    
    BigInteger_BI2Bin(pBI_SecretKey, SecretKey, SecretKeyLength);

    BigInteger_Free(&pBI_P);
    BigInteger_Free(&pBI_PrivateKey);
    BigInteger_Free(&pBI_PublicKey);
    BigInteger_Free(&pBI_SecretKey);
} 
