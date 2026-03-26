#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/sha256.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ecdsa, LOG_LEVEL_DBG);

/*
    [00:00:08.376,000] <dbg> crypto: example_crypto_operations: operation 7 selected
    [00:00:08.384,000] <inf> ecdsa:   . Seeding the random number generator...
    [00:00:08.398,000] <inf> ecdsa:  ok  . Generating key pair...
    [00:00:09.902,000] <inf> ecdsa:  ok (key size: 256 bits)
    [00:00:09.908,000] <inf> ecdsa:   . Computing message hash...
    [00:00:09.915,000] <inf> ecdsa:  ok
    [00:00:09.918,000] <inf> ecdsa:   . Signing message hash...
    [00:00:11.480,000] <inf> ecdsa:  ok (signature length = 72)
    [00:00:11.486,000] <inf> ecdsa:   . Preparing verification context...
    [00:00:11.493,000] <inf> ecdsa:  ok  . Verifying signature...
    [00:00:17.423,000] <inf> ecdsa:  ok
*/

int _ecdsa()
{
    int ret = 1;
    mbedtls_ecdsa_context ctx_sign, ctx_verify;
    mbedtls_ecp_point Q;
    mbedtls_ecp_point_init(&Q);
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    unsigned char message[100];
    unsigned char hash[32];
    unsigned char sig[MBEDTLS_ECDSA_MAX_LEN];
    size_t sig_len;
    const char *pers = "ecdsa";

    mbedtls_ecdsa_init(&ctx_sign);
    mbedtls_ecdsa_init(&ctx_verify);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    memset(sig, 0, sizeof(sig));
    memset(message, 0x25, sizeof(message));

    /*
     * Generate a key pair for signing
     */
    LOG_INF("  . Seeding the random number generator...");

    mbedtls_entropy_init(&entropy);
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) {
        LOG_INF(" failed  ! mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }

    LOG_INF(" ok  . Generating key pair...");

    if ((ret = mbedtls_ecdsa_genkey(&ctx_sign, MBEDTLS_ECP_DP_SECP256R1,
                                    mbedtls_ctr_drbg_random, &ctr_drbg)) != 0) {
        LOG_INF(" failed  ! mbedtls_ecdsa_genkey returned %d", ret);
        goto exit;
    }

    mbedtls_ecp_group_id grp_id = mbedtls_ecp_keypair_get_group_id(&ctx_sign);
    const mbedtls_ecp_curve_info *curve_info =
        mbedtls_ecp_curve_info_from_grp_id(grp_id);
    LOG_INF(" ok (key size: %d bits)", (int) curve_info->bit_size);

    /*
     * Compute message hash
     */
    LOG_INF("  . Computing message hash...");

    if ((ret = mbedtls_sha256(message, sizeof(message), hash, 0)) != 0) {
        LOG_INF(" failed  ! mbedtls_sha256 returned %d", ret);
        goto exit;
    }

    LOG_INF(" ok");

    /*
     * Sign message hash
     */
    LOG_INF("  . Signing message hash...");

    if ((ret = mbedtls_ecdsa_write_signature(&ctx_sign, MBEDTLS_MD_SHA256,
                                             hash, sizeof(hash),
                                             sig, sizeof(sig), &sig_len,
                                             mbedtls_ctr_drbg_random, &ctr_drbg)) != 0) {
        LOG_INF(" failed  ! mbedtls_ecdsa_write_signature returned %d", ret);
        goto exit;
    }
    LOG_INF(" ok (signature length = %u)", (unsigned int) sig_len);

    /*
     * Transfer public information to verifying context
     *
     * We could use the same context for verification and signatures, but we
     * chose to use a new one in order to make it clear that the verifying
     * context only needs the public key (Q), and not the private key (d).
     */
    LOG_INF("  . Preparing verification context...");

    if ((ret = mbedtls_ecp_export(&ctx_sign, NULL, NULL, &Q)) != 0) {
        LOG_INF(" failed  ! mbedtls_ecp_export returned %d", ret);
        goto exit;
    }

    if ((ret = mbedtls_ecp_set_public_key(grp_id, &ctx_verify, &Q)) != 0) {
        LOG_INF(" failed  ! mbedtls_ecp_set_public_key returned %d", ret);
        goto exit;
    }

    /*
     * Verify signature
     */
    LOG_INF(" ok  . Verifying signature...");

    if ((ret = mbedtls_ecdsa_read_signature(&ctx_verify,
                                            hash, sizeof(hash),
                                            sig, sig_len)) != 0) {
        LOG_INF(" failed  ! mbedtls_ecdsa_read_signature returned %d", ret);
        goto exit;
    }

    LOG_INF(" ok");
exit:

    mbedtls_ecdsa_free(&ctx_verify);
    mbedtls_ecdsa_free(&ctx_sign);
    mbedtls_ecp_point_free(&Q);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return 0;
}