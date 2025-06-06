// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Cryptographic API.
 *
 * Null algorithms, aka Much Ado About Nothing.
 *
 * These are needed for IPsec, and may be useful in general for
 * testing & debugging.
 *
 * The null cipher is compliant with RFC2410.
 *
 * Copyright (c) 2002 James Morris <jmorris@intercode.com.au>
 */

#include <crypto/null.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/skcipher.h>
#include <crypto/scatterwalk.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>

static int null_init(struct shash_desc *desc)
{
	return 0;
}

static int null_update(struct shash_desc *desc, const u8 *data,
		       unsigned int len)
{
	return 0;
}

static int null_final(struct shash_desc *desc, u8 *out)
{
	return 0;
}

static int null_digest(struct shash_desc *desc, const u8 *data,
		       unsigned int len, u8 *out)
{
	return 0;
}

static int null_hash_setkey(struct crypto_shash *tfm, const u8 *key,
			    unsigned int keylen)
{ return 0; }

static int null_skcipher_setkey(struct crypto_skcipher *tfm, const u8 *key,
				unsigned int keylen)
{ return 0; }

static int null_setkey(struct crypto_tfm *tfm, const u8 *key,
		       unsigned int keylen)
{ return 0; }

static void null_crypt(struct crypto_tfm *tfm, u8 *dst, const u8 *src)
{
	memcpy(dst, src, NULL_BLOCK_SIZE);
}

static int null_skcipher_crypt(struct skcipher_request *req)
{
	if (req->src != req->dst)
		memcpy_sglist(req->dst, req->src, req->cryptlen);
	return 0;
}

static struct shash_alg digest_null = {
	.digestsize		=	NULL_DIGEST_SIZE,
	.setkey   		=	null_hash_setkey,
	.init   		=	null_init,
	.update 		=	null_update,
	.finup 			=	null_digest,
	.digest 		=	null_digest,
	.final  		=	null_final,
	.base			=	{
		.cra_name		=	"digest_null",
		.cra_driver_name	=	"digest_null-generic",
		.cra_blocksize		=	NULL_BLOCK_SIZE,
		.cra_module		=	THIS_MODULE,
	}
};

static struct skcipher_alg skcipher_null = {
	.base.cra_name		=	"ecb(cipher_null)",
	.base.cra_driver_name	=	"ecb-cipher_null",
	.base.cra_priority	=	100,
	.base.cra_blocksize	=	NULL_BLOCK_SIZE,
	.base.cra_ctxsize	=	0,
	.base.cra_module	=	THIS_MODULE,
	.min_keysize		=	NULL_KEY_SIZE,
	.max_keysize		=	NULL_KEY_SIZE,
	.ivsize			=	NULL_IV_SIZE,
	.setkey			=	null_skcipher_setkey,
	.encrypt		=	null_skcipher_crypt,
	.decrypt		=	null_skcipher_crypt,
};

static struct crypto_alg cipher_null = {
	.cra_name		=	"cipher_null",
	.cra_driver_name	=	"cipher_null-generic",
	.cra_flags		=	CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize		=	NULL_BLOCK_SIZE,
	.cra_ctxsize		=	0,
	.cra_module		=	THIS_MODULE,
	.cra_u			=	{ .cipher = {
	.cia_min_keysize	=	NULL_KEY_SIZE,
	.cia_max_keysize	=	NULL_KEY_SIZE,
	.cia_setkey		= 	null_setkey,
	.cia_encrypt		=	null_crypt,
	.cia_decrypt		=	null_crypt } }
};

MODULE_ALIAS_CRYPTO("digest_null");
MODULE_ALIAS_CRYPTO("cipher_null");

static int __init crypto_null_mod_init(void)
{
	int ret = 0;

	ret = crypto_register_alg(&cipher_null);
	if (ret < 0)
		goto out;

	ret = crypto_register_shash(&digest_null);
	if (ret < 0)
		goto out_unregister_algs;

	ret = crypto_register_skcipher(&skcipher_null);
	if (ret < 0)
		goto out_unregister_shash;

	return 0;

out_unregister_shash:
	crypto_unregister_shash(&digest_null);
out_unregister_algs:
	crypto_unregister_alg(&cipher_null);
out:
	return ret;
}

static void __exit crypto_null_mod_fini(void)
{
	crypto_unregister_alg(&cipher_null);
	crypto_unregister_shash(&digest_null);
	crypto_unregister_skcipher(&skcipher_null);
}

module_init(crypto_null_mod_init);
module_exit(crypto_null_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Null Cryptographic Algorithms");
