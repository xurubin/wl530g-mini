#ifndef MVWLAN_CRYPT_H
#define MVWLAN_CRYPT_H

/* Maximum length for algorithm names (-1 for nul termination) used in ioctl()
 */
#define MVWLAN_CRYPT_ALG_NAME_LEN       16


struct mvwlan_crypto_ops {
	char *name;

	/* init new crypto context (e.g., allocate private data space,
	 * select IV, etc.); returns NULL on failure or pointer to allocated
	 * private data on success */
	void * (*init)(void);

	/* deinitialize crypto context and free allocated private data */
	void (*deinit)(void *priv);

	/* encrypt/decrypt return < 0 on error or number of bytes written
	 * to out_buf; len is number of bytes in in_buf */
	int (*encrypt_mpdu)(u8 *buf, u8 *obuf, int len, void *priv, int broadcast);
	int (*decrypt_mpdu)(u8 *buf, u8 *obuf, int len, void *priv);

	/* These functions are called for full MSDUs, i.e. full frames.
	 * These can be NULL if full MSDU operations are not needed. */
	int (*encrypt_msdu)(u8 *da, u8 *sa, u8 *buf, int len, void *priv, int broadcast);
	int (*decrypt_msdu)(u8 *da, u8 *sa, u8 *buf, int len, void *priv);

	int (*set_key)(int idx, void *key, int len, void *priv);
	int (*get_key)(int idx, void *key, int len, void *priv);

	int (*set_key_idx)(int idx, void *priv);
	int (*get_key_idx)(void *priv);

	/* maximum number of bytes added by encryption; encrypt buf is
	 * allocated with extra_prefix_len bytes, copy of in_buf, and
	 * extra_postfix_len; encrypt need not use all this space, but
	 * the result must start at the beginning of the buffer and correct
	 * length must be returned */
	int extra_prefix_len, extra_postfix_len;
};


int mvWLAN_register_crypto_ops(struct mvwlan_crypto_ops *ops);
int mvWLAN_unregister_crypto_ops(struct mvwlan_crypto_ops *ops);
struct mvwlan_crypto_ops * mvWLAN_get_crypto_ops(const char *name);

#ifdef SELF_CTS
void crc32forCts(unsigned char *buf, int len);
#endif

#endif /* MVWLAN_CRYPT_H */