



#include <linux/err.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <linux/crypto.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/sunrpc/gss_krb5.h>
#include <linux/sunrpc/xdr.h>

#ifdef RPC_DEBUG
# define RPCDBG_FACILITY        RPCDBG_AUTH
#endif

u32
krb5_encrypt(
	struct crypto_blkcipher *tfm,
	void * iv,
	void * in,
	void * out,
	int length)
{
	u32 ret = -EINVAL;
	struct scatterlist sg[1];
	u8 local_iv[16] = {0};
	struct blkcipher_desc desc = { .tfm = tfm, .info = local_iv };

	if (length % crypto_blkcipher_blocksize(tfm) != 0)
		goto out;

	if (crypto_blkcipher_ivsize(tfm) > 16) {
		dprintk("RPC:       gss_k5encrypt: tfm iv size too large %d\n",
			crypto_blkcipher_ivsize(tfm));
		goto out;
	}

	if (iv)
		memcpy(local_iv, iv, crypto_blkcipher_ivsize(tfm));

	memcpy(out, in, length);
	sg_init_one(sg, out, length);

	ret = crypto_blkcipher_encrypt_iv(&desc, sg, sg, length);
out:
	dprintk("RPC:       krb5_encrypt returns %d\n", ret);
	return ret;
}

u32
krb5_decrypt(
     struct crypto_blkcipher *tfm,
     void * iv,
     void * in,
     void * out,
     int length)
{
	u32 ret = -EINVAL;
	struct scatterlist sg[1];
	u8 local_iv[16] = {0};
	struct blkcipher_desc desc = { .tfm = tfm, .info = local_iv };

	if (length % crypto_blkcipher_blocksize(tfm) != 0)
		goto out;

	if (crypto_blkcipher_ivsize(tfm) > 16) {
		dprintk("RPC:       gss_k5decrypt: tfm iv size too large %d\n",
			crypto_blkcipher_ivsize(tfm));
		goto out;
	}
	if (iv)
		memcpy(local_iv,iv, crypto_blkcipher_ivsize(tfm));

	memcpy(out, in, length);
	sg_init_one(sg, out, length);

	ret = crypto_blkcipher_decrypt_iv(&desc, sg, sg, length);
out:
	dprintk("RPC:       gss_k5decrypt returns %d\n",ret);
	return ret;
}

static int
checksummer(struct scatterlist *sg, void *data)
{
	struct hash_desc *desc = data;

	return crypto_hash_update(desc, sg, sg->length);
}


s32
make_checksum(char *cksumname, char *header, int hdrlen, struct xdr_buf *body,
		   int body_offset, struct xdr_netobj *cksum)
{
	struct hash_desc                desc; 
	struct scatterlist              sg[1];
	int err;

	desc.tfm = crypto_alloc_hash(cksumname, 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(desc.tfm))
		return GSS_S_FAILURE;
	cksum->len = crypto_hash_digestsize(desc.tfm);
	desc.flags = CRYPTO_TFM_REQ_MAY_SLEEP;

	err = crypto_hash_init(&desc);
	if (err)
		goto out;
	sg_init_one(sg, header, hdrlen);
	err = crypto_hash_update(&desc, sg, hdrlen);
	if (err)
		goto out;
	err = xdr_process_buf(body, body_offset, body->len - body_offset,
			      checksummer, &desc);
	if (err)
		goto out;
	err = crypto_hash_final(&desc, cksum->data);

out:
	crypto_free_hash(desc.tfm);
	return err ? GSS_S_FAILURE : 0;
}

struct encryptor_desc {
	u8 iv[8]; 
	struct blkcipher_desc desc;
	int pos;
	struct xdr_buf *outbuf;
	struct page **pages;
	struct scatterlist infrags[4];
	struct scatterlist outfrags[4];
	int fragno;
	int fraglen;
};

static int
encryptor(struct scatterlist *sg, void *data)
{
	struct encryptor_desc *desc = data;
	struct xdr_buf *outbuf = desc->outbuf;
	struct page *in_page;
	int thislen = desc->fraglen + sg->length;
	int fraglen, ret;
	int page_pos;

	
	BUG_ON(desc->fragno > 3);

	page_pos = desc->pos - outbuf->head[0].iov_len;
	if (page_pos >= 0 && page_pos < outbuf->page_len) {
		
		int i = (page_pos + outbuf->page_base) >> PAGE_CACHE_SHIFT;
		in_page = desc->pages[i];
	} else {
		in_page = sg_page(sg);
	}
	sg_set_page(&desc->infrags[desc->fragno], in_page, sg->length,
		    sg->offset);
	sg_set_page(&desc->outfrags[desc->fragno], sg_page(sg), sg->length,
		    sg->offset);
	desc->fragno++;
	desc->fraglen += sg->length;
	desc->pos += sg->length;

	fraglen = thislen & 7; 
	thislen -= fraglen;

	if (thislen == 0)
		return 0;

	sg_mark_end(&desc->infrags[desc->fragno - 1]);
	sg_mark_end(&desc->outfrags[desc->fragno - 1]);

	ret = crypto_blkcipher_encrypt_iv(&desc->desc, desc->outfrags,
					  desc->infrags, thislen);
	if (ret)
		return ret;

	sg_init_table(desc->infrags, 4);
	sg_init_table(desc->outfrags, 4);

	if (fraglen) {
		sg_set_page(&desc->outfrags[0], sg_page(sg), fraglen,
				sg->offset + sg->length - fraglen);
		desc->infrags[0] = desc->outfrags[0];
		sg_assign_page(&desc->infrags[0], in_page);
		desc->fragno = 1;
		desc->fraglen = fraglen;
	} else {
		desc->fragno = 0;
		desc->fraglen = 0;
	}
	return 0;
}

int
gss_encrypt_xdr_buf(struct crypto_blkcipher *tfm, struct xdr_buf *buf,
		    int offset, struct page **pages)
{
	int ret;
	struct encryptor_desc desc;

	BUG_ON((buf->len - offset) % crypto_blkcipher_blocksize(tfm) != 0);

	memset(desc.iv, 0, sizeof(desc.iv));
	desc.desc.tfm = tfm;
	desc.desc.info = desc.iv;
	desc.desc.flags = 0;
	desc.pos = offset;
	desc.outbuf = buf;
	desc.pages = pages;
	desc.fragno = 0;
	desc.fraglen = 0;

	sg_init_table(desc.infrags, 4);
	sg_init_table(desc.outfrags, 4);

	ret = xdr_process_buf(buf, offset, buf->len - offset, encryptor, &desc);
	return ret;
}

struct decryptor_desc {
	u8 iv[8]; 
	struct blkcipher_desc desc;
	struct scatterlist frags[4];
	int fragno;
	int fraglen;
};

static int
decryptor(struct scatterlist *sg, void *data)
{
	struct decryptor_desc *desc = data;
	int thislen = desc->fraglen + sg->length;
	int fraglen, ret;

	
	BUG_ON(desc->fragno > 3);
	sg_set_page(&desc->frags[desc->fragno], sg_page(sg), sg->length,
		    sg->offset);
	desc->fragno++;
	desc->fraglen += sg->length;

	fraglen = thislen & 7; 
	thislen -= fraglen;

	if (thislen == 0)
		return 0;

	sg_mark_end(&desc->frags[desc->fragno - 1]);

	ret = crypto_blkcipher_decrypt_iv(&desc->desc, desc->frags,
					  desc->frags, thislen);
	if (ret)
		return ret;

	sg_init_table(desc->frags, 4);

	if (fraglen) {
		sg_set_page(&desc->frags[0], sg_page(sg), fraglen,
				sg->offset + sg->length - fraglen);
		desc->fragno = 1;
		desc->fraglen = fraglen;
	} else {
		desc->fragno = 0;
		desc->fraglen = 0;
	}
	return 0;
}

int
gss_decrypt_xdr_buf(struct crypto_blkcipher *tfm, struct xdr_buf *buf,
		    int offset)
{
	struct decryptor_desc desc;

	
	BUG_ON((buf->len - offset) % crypto_blkcipher_blocksize(tfm) != 0);

	memset(desc.iv, 0, sizeof(desc.iv));
	desc.desc.tfm = tfm;
	desc.desc.info = desc.iv;
	desc.desc.flags = 0;
	desc.fragno = 0;
	desc.fraglen = 0;

	sg_init_table(desc.frags, 4);

	return xdr_process_buf(buf, offset, buf->len - offset, decryptor, &desc);
}
