



#include <linux/sunrpc/auth_gss.h>
#include <linux/sunrpc/gss_err.h>
#include <linux/sunrpc/gss_asn1.h>

struct krb5_ctx {
	int			initiate; 
	struct crypto_blkcipher	*enc;
	struct crypto_blkcipher	*seq;
	s32			endtime;
	u32			seq_send;
	struct xdr_netobj	mech_used;
};

extern spinlock_t krb5_seq_lock;


#define GSS_KRB5_TOK_HDR_LEN	(16)

#define KG_TOK_MIC_MSG    0x0101
#define KG_TOK_WRAP_MSG   0x0201

enum sgn_alg {
	SGN_ALG_DES_MAC_MD5 = 0x0000,
	SGN_ALG_MD2_5 = 0x0001,
	SGN_ALG_DES_MAC = 0x0002,
	SGN_ALG_3 = 0x0003,		
	SGN_ALG_HMAC_MD5 = 0x0011,	
	SGN_ALG_HMAC_SHA1_DES3_KD = 0x0004
};
enum seal_alg {
	SEAL_ALG_NONE = 0xffff,
	SEAL_ALG_DES = 0x0000,
	SEAL_ALG_1 = 0x0001,		
	SEAL_ALG_MICROSOFT_RC4 = 0x0010,
	SEAL_ALG_DES3KD = 0x0002
};

#define CKSUMTYPE_CRC32			0x0001
#define CKSUMTYPE_RSA_MD4		0x0002
#define CKSUMTYPE_RSA_MD4_DES		0x0003
#define CKSUMTYPE_DESCBC		0x0004
#define CKSUMTYPE_RSA_MD5		0x0007
#define CKSUMTYPE_RSA_MD5_DES		0x0008
#define CKSUMTYPE_NIST_SHA		0x0009
#define CKSUMTYPE_HMAC_SHA1_DES3	0x000c


#define KG_CCACHE_NOMATCH                        (39756032L)
#define KG_KEYTAB_NOMATCH                        (39756033L)
#define KG_TGT_MISSING                           (39756034L)
#define KG_NO_SUBKEY                             (39756035L)
#define KG_CONTEXT_ESTABLISHED                   (39756036L)
#define KG_BAD_SIGN_TYPE                         (39756037L)
#define KG_BAD_LENGTH                            (39756038L)
#define KG_CTX_INCOMPLETE                        (39756039L)
#define KG_CONTEXT                               (39756040L)
#define KG_CRED                                  (39756041L)
#define KG_ENC_DESC                              (39756042L)
#define KG_BAD_SEQ                               (39756043L)
#define KG_EMPTY_CCACHE                          (39756044L)
#define KG_NO_CTYPES                             (39756045L)


#define ENCTYPE_NULL            0x0000
#define ENCTYPE_DES_CBC_CRC     0x0001	
#define ENCTYPE_DES_CBC_MD4     0x0002	
#define ENCTYPE_DES_CBC_MD5     0x0003	
#define ENCTYPE_DES_CBC_RAW     0x0004	

#define ENCTYPE_DES3_CBC_SHA    0x0005	
#define ENCTYPE_DES3_CBC_RAW    0x0006	
#define ENCTYPE_DES_HMAC_SHA1   0x0008
#define ENCTYPE_DES3_CBC_SHA1   0x0010
#define ENCTYPE_UNKNOWN         0x01ff

s32
make_checksum(char *, char *header, int hdrlen, struct xdr_buf *body,
		   int body_offset, struct xdr_netobj *cksum);

u32 gss_get_mic_kerberos(struct gss_ctx *, struct xdr_buf *,
		struct xdr_netobj *);

u32 gss_verify_mic_kerberos(struct gss_ctx *, struct xdr_buf *,
		struct xdr_netobj *);

u32
gss_wrap_kerberos(struct gss_ctx *ctx_id, int offset,
		struct xdr_buf *outbuf, struct page **pages);

u32
gss_unwrap_kerberos(struct gss_ctx *ctx_id, int offset,
		struct xdr_buf *buf);


u32
krb5_encrypt(struct crypto_blkcipher *key,
	     void *iv, void *in, void *out, int length);

u32
krb5_decrypt(struct crypto_blkcipher *key,
	     void *iv, void *in, void *out, int length); 

int
gss_encrypt_xdr_buf(struct crypto_blkcipher *tfm, struct xdr_buf *outbuf,
		    int offset, struct page **pages);

int
gss_decrypt_xdr_buf(struct crypto_blkcipher *tfm, struct xdr_buf *inbuf,
		    int offset);

s32
krb5_make_seq_num(struct crypto_blkcipher *key,
		int direction,
		u32 seqnum, unsigned char *cksum, unsigned char *buf);

s32
krb5_get_seq_num(struct crypto_blkcipher *key,
	       unsigned char *cksum,
	       unsigned char *buf, int *direction, u32 *seqnum);
