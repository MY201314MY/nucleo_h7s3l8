#include <string.h>
#include <stm32h7rsxx_hal.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/bignum.h>
#include <mbedtls/platform_util.h>
#include <mbedtls/error.h>

#include "mbedtls-curve.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ecdsa_alt, LOG_LEVEL_INF);

static PKA_HandleTypeDef hpka = { .Instance = PKA, .State = HAL_PKA_STATE_READY };

int mbedtls_ecdsa_genkey(mbedtls_ecp_keypair *ctx, mbedtls_ecp_group_id gid,
                         int (*f_rng)(void *, unsigned char *, size_t), void *p_rng)
{
    int ret = 0;
    size_t length = 0;
    uint8_t rand_priv[66] = {0};
    uint8_t Qx[66] = {0}, Qy[66] = {0};

    LOG_DBG("genkey...");

    switch (gid) {
        case MBEDTLS_ECP_DP_SECP192R1:
            length = 24;
            break;
        case MBEDTLS_ECP_DP_SECP256R1:
            length = 32;
            break;
        case MBEDTLS_ECP_DP_SECP384R1:
            length = 48;
            break;
        default:
            return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    if ((ret = mbedtls_ecp_group_load(&ctx->MBEDTLS_PRIVATE(grp), gid)) != 0)
    {
        LOG_ERR("L:%d", __LINE__);
        return ret;
    }

    mbedtls_mpi mpi_n;
    mbedtls_mpi_init(&mpi_n);

    switch (gid) {
        case MBEDTLS_ECP_DP_SECP192R1:
            mbedtls_mpi_read_binary(&mpi_n, secp192r1_n, length);
            break;
        case MBEDTLS_ECP_DP_SECP256R1:
            mbedtls_mpi_read_binary(&mpi_n, secp256r1_n, length);
            break;
        case MBEDTLS_ECP_DP_SECP384R1:
            mbedtls_mpi_read_binary(&mpi_n, secp384r1_n, length);
            break;
        default:
            mbedtls_mpi_free(&mpi_n);
            return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    do {
        if ((ret = f_rng(p_rng, rand_priv, length)) != 0)
            return ret;
        mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(d), rand_priv, length);
    } while (mbedtls_mpi_cmp_int(&ctx->MBEDTLS_PRIVATE(d), 1) < 0 ||
             mbedtls_mpi_cmp_mpi(&ctx->MBEDTLS_PRIVATE(d), &mpi_n) >= 0);
    mbedtls_mpi_free(&mpi_n);

    LOG_HEXDUMP_INF(rand_priv, length, "d");

    if (pka_compute_public_key(gid, rand_priv, Qx, Qy) != 0) {
        memset(Qx, 0, length);
        memset(Qy, 0, length);
        LOG_ERR("L:%d", __LINE__);
        return MBEDTLS_ERR_ECP_ALLOC_FAILED;
    }

    if ((ret = mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(d), rand_priv, length)) != 0)
    {
        LOG_ERR("L:%d", __LINE__);
        return ret;
    }

    if ((ret = mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(X), Qx, length)) != 0)
    {
        LOG_ERR("L:%d", __LINE__);
        return ret;
    }
    if ((ret = mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Y), Qy, length)) != 0)
    {
        LOG_ERR("L:%d", __LINE__);
        return ret;
    }
    if ((ret = mbedtls_mpi_lset(&ctx->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Z), 1)) != 0)
    {
        LOG_ERR("L:%d", __LINE__);
        return ret;
    }

    return 0;
}

int mbedtls_ecdsa_sign(mbedtls_ecp_group *grp, mbedtls_mpi *r, mbedtls_mpi *s,
                       const mbedtls_mpi *d, const unsigned char *buf, size_t blen,
                       int (*f_rng)(void *, unsigned char *, size_t), void *p_rng)
{
    size_t size = 0;
    const uint8_t *p = NULL, *a = NULL, *b = NULL, *gx = NULL, *gy = NULL, *n = NULL;
    uint32_t modulusSize = 0, orderSize = 0, coefSign = 1;
    uint8_t k_bin[66] = {0}, d_buf[66] = {0}, r_bin[66] = {0}, s_bin[66] = {0};
    PKA_ECDSASignInTypeDef in = {0};
    PKA_ECDSASignOutTypeDef out = {0};

    LOG_DBG("sign...");

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));

    switch (grp->id) {
        case MBEDTLS_ECP_DP_SECP192R1:
            size = 24; p = secp192r1_p; a = secp192r1_a; b = secp192r1_b; gx = secp192r1_Gx; gy = secp192r1_Gy; n = secp192r1_n; modulusSize = secp192r1_modulusSize; orderSize = secp192r1_orderSize; coefSign = secp192r1_coefSign; break;
        case MBEDTLS_ECP_DP_SECP256R1:
            size = 32; p = secp256r1_p; a = secp256r1_a; b = secp256r1_b; gx = secp256r1_Gx; gy = secp256r1_Gy; n = secp256r1_n; modulusSize = secp256r1_modulusSize; orderSize = secp256r1_orderSize; coefSign = secp256r1_coefSign; break;
        case MBEDTLS_ECP_DP_SECP384R1:
            size = 48; p = secp384r1_p; a = secp384r1_a; b = secp384r1_b; gx = secp384r1_Gx; gy = secp384r1_Gy; n = secp384r1_n; modulusSize = secp384r1_modulusSize; orderSize = secp384r1_orderSize; coefSign = secp384r1_coefSign; break;
        default:
            return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }

    if (f_rng(p_rng, k_bin, size) != 0) return MBEDTLS_ERR_ECP_RANDOM_FAILED;
    mbedtls_mpi_write_binary(d, d_buf, size);

    in.primeOrderSize = orderSize;
    in.modulusSize    = modulusSize;
    in.coefSign       = coefSign;
    in.coef           = a;
    in.coefB          = b;
    in.modulus        = p;
    in.basePointX     = gx;
    in.basePointY     = gy;
    in.integer        = k_bin;
    in.hash           = buf;
    in.privateKey     = d_buf;
    in.primeOrder     = n;

    out.RSign = r_bin;
    out.SSign = s_bin;

    if (HAL_PKA_ECDSASign(&hpka, &in, 5000) != HAL_OK) return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    HAL_PKA_ECDSASign_GetResult(&hpka, &out, NULL);

    mbedtls_mpi_read_binary(r, r_bin, size);
    mbedtls_mpi_read_binary(s, s_bin, size);

    return 0;
}

int mbedtls_ecdsa_verify(mbedtls_ecp_group *grp, const unsigned char *buf, size_t blen,
                          const mbedtls_ecp_point *Q, const mbedtls_mpi *r, const mbedtls_mpi *s)
{
    size_t size = 0, q_len = 0;
    const uint8_t *p = NULL, *a = NULL, *gx = NULL, *gy = NULL, *n = NULL;
    uint32_t modulusSize = 0, orderSize = 0, coefSign = 1;
    uint8_t r_bin[66] = {0}, s_bin[66] = {0}, q_bin[2*66+1] = {0};
    PKA_ECDSAVerifInTypeDef in = {0};

    LOG_DBG("verify...");

    memset(&in, 0, sizeof(in));

    switch (grp->id) {
        case MBEDTLS_ECP_DP_SECP192R1:
            size = 24; p = secp192r1_p; a = secp192r1_a; gx = secp192r1_Gx; gy = secp192r1_Gy; n = secp192r1_n; modulusSize = secp192r1_modulusSize; orderSize = secp192r1_orderSize; coefSign = secp192r1_coefSign; break;
        case MBEDTLS_ECP_DP_SECP256R1:
            size = 32; p = secp256r1_p; a = secp256r1_a; gx = secp256r1_Gx; gy = secp256r1_Gy; n = secp256r1_n; modulusSize = secp256r1_modulusSize; orderSize = secp256r1_orderSize; coefSign = secp256r1_coefSign; break;
        case MBEDTLS_ECP_DP_SECP384R1:
            size = 48; p = secp384r1_p; a = secp384r1_a; gx = secp384r1_Gx; gy = secp384r1_Gy; n = secp384r1_n; modulusSize = secp384r1_modulusSize; orderSize = secp384r1_orderSize; coefSign = secp384r1_coefSign; break;
        default:
            return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }

    mbedtls_mpi_write_binary(r, r_bin, size);
    mbedtls_mpi_write_binary(s, s_bin, size);
    mbedtls_ecp_point_write_binary(grp, Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &q_len, q_bin, sizeof(q_bin));

    in.primeOrderSize  = orderSize;
    in.modulusSize     = modulusSize;
    in.coefSign        = coefSign;
    in.coef            = a;
    in.modulus         = p;
    in.basePointX      = gx;
    in.basePointY      = gy;
    in.pPubKeyCurvePtX = &q_bin[1];
    in.pPubKeyCurvePtY = &q_bin[1+size];
    in.RSign           = r_bin;
    in.SSign           = s_bin;
    in.hash            = buf;
    in.primeOrder      = n;

    if (HAL_PKA_ECDSAVerif(&hpka, &in, 5000) != HAL_OK) {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    if (HAL_PKA_ECDSAVerif_IsValidSignature(&hpka) == 1) {
        return 0; 
    } else {
        return MBEDTLS_ERR_ECP_VERIFY_FAILED; 
    }
}
