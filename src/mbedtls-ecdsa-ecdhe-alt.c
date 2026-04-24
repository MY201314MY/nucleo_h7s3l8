#include <string.h>
#include <stm32h7rsxx_hal.h>
#include <stm32h7rsxx_hal_pka.h>

#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/bignum.h>
#include <mbedtls/platform_util.h>
#include <mbedtls/error.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ecdsa_alt, LOG_LEVEL_DBG);

static PKA_HandleTypeDef hpka = { .Instance = PKA, .State = HAL_PKA_STATE_READY };

/* SECP192R1 parameters */
static const uint32_t secp192r1_modulusSize = 24;
static const uint32_t secp192r1_orderSize   = 24;
static const uint32_t secp192r1_coefSign    = 1;
static const uint8_t  secp192r1_p[24]       = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t  secp192r1_a[24]       = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};
static const uint8_t  secp192r1_b[24]       = {0x64, 0x21, 0x05, 0x19, 0xE5, 0x9C, 0x80, 0xE7, 0x0F, 0xA7, 0xE9, 0xAB, 0x72, 0x24, 0x30, 0x49, 0xFE, 0xB8, 0xDE, 0xEC, 0xC1, 0x46, 0xB9, 0xB1};
static const uint8_t  secp192r1_m[24]       = {0x00};
static const uint8_t  secp192r1_Gx[24]      = {0x18, 0x8D, 0xA8, 0x0E, 0xB0, 0x30, 0x90, 0xF6, 0x7C, 0xBF, 0x20, 0xEB, 0x43, 0xA1, 0x88, 0x00, 0xF4, 0xFF, 0x0A, 0xFD, 0x82, 0xFF, 0x10, 0x12};
static const uint8_t  secp192r1_Gy[24]      = {0x07, 0x19, 0x2B, 0x95, 0xFF, 0xC8, 0xDA, 0x78, 0x63, 0x10, 0x11, 0xED, 0x6B, 0x24, 0xCD, 0xD5, 0x73, 0xF9, 0x77, 0xA1, 0x1E, 0x79, 0x48, 0x11};
static const uint8_t  secp192r1_Gz[24]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static const uint8_t  secp192r1_n[24]       = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x99, 0xDE, 0xF8, 0x36, 0x14, 0x6B, 0xC9, 0xB1, 0xB4, 0xD2, 0x28, 0x31};

/* SECP256R1 parameters */
static const uint32_t secp256r1_modulusSize = 32;
static const uint32_t secp256r1_orderSize   = 32;
static const uint32_t secp256r1_coefSign    = 1;
static const uint8_t  secp256r1_p[32]       = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t  secp256r1_a[32]       = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};
static const uint8_t  secp256r1_b[32]       = {0x5A, 0xC6, 0x35, 0xD8, 0xAA, 0x3A, 0x93, 0xE7, 0xB3, 0xEB, 0xBD, 0x55, 0x76, 0x98, 0x86, 0xBC, 0x65, 0x1D, 0x06, 0xB0, 0xCC, 0x53, 0xB0, 0xF6, 0x3B, 0xCE, 0x3C, 0x3E, 0x27, 0xD2, 0x60, 0x4B};
static const uint8_t  secp256r1_m[32]       = {0x00};
static const uint8_t  secp256r1_Gx[32]      = {0x6b, 0x17, 0xd1, 0xf2, 0xe1, 0x2c, 0x42, 0x47, 0xf8, 0xbc, 0xe6, 0xe5, 0x63, 0xa4, 0x40, 0xf2, 0x77, 0x03, 0x7d, 0x81, 0x2d, 0xeb, 0x33, 0xa0, 0xf4, 0xa1, 0x39, 0x45, 0xd8, 0x98, 0xc2, 0x96};
static const uint8_t  secp256r1_Gy[32]      = {0x4f, 0xe3, 0x42, 0xe2, 0xfe, 0x1a, 0x7f, 0x9b, 0x8e, 0xe7, 0xeb, 0x4a, 0x7c, 0x0f, 0x9e, 0x16, 0x2b, 0xce, 0x33, 0x57, 0x6b, 0x31, 0x5e, 0xce, 0xcb, 0xb6, 0x40, 0x68, 0x37, 0xbf, 0x51, 0xf5};
static const uint8_t  secp256r1_Gz[32]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static const uint8_t  secp256r1_n[32]       = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84, 0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51};

/* SECP384R1 parameters */
static const uint32_t secp384r1_modulusSize = 48;
static const uint32_t secp384r1_orderSize   = 48;
static const uint32_t secp384r1_coefSign    = 1;
static const uint8_t  secp384r1_p[48]       = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t  secp384r1_a[48]       = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};
static const uint8_t  secp384r1_b[48]       = {0xB3, 0x31, 0x2F, 0xA7, 0xE2, 0x3E, 0xE7, 0xE4, 0x98, 0x8E, 0x05, 0x6B, 0xE3, 0xF8, 0x2D, 0x19, 0x18, 0x1D, 0x9C, 0x6E, 0xFE, 0x81, 0x41, 0x12, 0x03, 0x14, 0x08, 0x8F, 0x50, 0x13, 0x87, 0x5A, 0xC6, 0x56, 0x39, 0x8D, 0x8A, 0x2E, 0xD1, 0x9D, 0x2A, 0x85, 0xC8, 0xED, 0xD3, 0xEC, 0x2A, 0xEF};
static const uint8_t  secp384r1_m[48]       = {0x00};
static const uint8_t  secp384r1_Gx[48]      = {0xAA, 0x87, 0xCA, 0x22, 0xBE, 0x8B, 0x05, 0x37, 0x8E, 0xB1, 0xC7, 0x1E, 0xF3, 0x20, 0xAD, 0x74, 0x6E, 0x1D, 0x3B, 0x62, 0x8B, 0xA7, 0x9B, 0x98, 0x59, 0xF7, 0x41, 0xE0, 0x82, 0x54, 0x2A, 0x38, 0x55, 0x02, 0xF2, 0x5D, 0xBF, 0x55, 0x29, 0x6C, 0x3A, 0x54, 0x5E, 0x38, 0x72, 0x76, 0x0A, 0xB7};
static const uint8_t  secp384r1_Gy[48]      = {0x36, 0x17, 0xDE, 0x4A, 0x96, 0x26, 0x2C, 0x6F, 0x5D, 0x9E, 0x98, 0xBF, 0x92, 0x92, 0xDC, 0x29, 0xF8, 0xF4, 0x1D, 0xBD, 0x28, 0x9A, 0x14, 0x7C, 0xE9, 0xDA, 0x31, 0x13, 0xB5, 0xF0, 0xB8, 0xC0, 0x0A, 0x60, 0xB1, 0xCE, 0x1D, 0x7E, 0x81, 0x9D, 0x7A, 0x43, 0x1D, 0x7C, 0x90, 0xEA, 0x0E, 0x5F};
static const uint8_t  secp384r1_Gz[48]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static const uint8_t  secp384r1_n[48]       = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0x63, 0x4D, 0x81, 0xF4, 0x37, 0x2D, 0xDF, 0x58, 0x1A, 0x0D, 0xB2, 0x48, 0xB0, 0xA7, 0x7A, 0xEC, 0xEC, 0x19, 0x6A, 0xCC, 0xC5, 0x29, 0x73};

static int pka_compute_public_key(mbedtls_ecp_group_id gid, const uint8_t *priv_key, uint8_t *Qx, uint8_t *Qy)
{
    HAL_StatusTypeDef status = HAL_OK;
    PKA_ECCDoubleBaseLadderInTypeDef in = {0};
    PKA_ECCDoubleBaseLadderOutTypeDef out = {0};

    if (gid == MBEDTLS_ECP_DP_SECP256R1) {
        in.modulusSize = secp256r1_modulusSize;
        in.primeOrderSize = secp256r1_orderSize;
        in.modulus = secp256r1_p;
        in.coefSign = secp256r1_coefSign;
        in.coefA = secp256r1_a;
        in.integerK = priv_key;
        in.integerM = secp256r1_m;
        in.basePointX1 = secp256r1_Gx;
        in.basePointY1 = secp256r1_Gy;
        in.basePointZ1 = secp256r1_Gz;
        in.basePointX2 = NULL;
        in.basePointY2 = NULL;
        in.basePointZ2 = NULL;
    } else if (gid == MBEDTLS_ECP_DP_SECP192R1) {
        in.modulusSize = secp192r1_modulusSize;
        in.primeOrderSize = secp192r1_orderSize;
        in.modulus = secp192r1_p;
        in.coefSign = secp192r1_coefSign;
        in.coefA = secp192r1_a;
        in.integerK = priv_key;
        in.integerM = secp192r1_m;
        in.basePointX1 = secp192r1_Gx;
        in.basePointY1 = secp192r1_Gy;
        in.basePointZ1 = secp192r1_Gz;
        in.basePointX2 = NULL;
        in.basePointY2 = NULL;
        in.basePointZ2 = NULL;
    } else if (gid == MBEDTLS_ECP_DP_SECP384R1) {
        in.modulusSize = secp384r1_modulusSize;
        in.primeOrderSize = secp384r1_orderSize;
        in.modulus = secp384r1_p;
        in.coefSign = secp384r1_coefSign;
        in.coefA = secp384r1_a;
        in.integerK = priv_key;
        in.integerM = secp384r1_m;
        in.basePointX1 = secp384r1_Gx;
        in.basePointY1 = secp384r1_Gy;
        in.basePointZ1 = secp384r1_Gz;
        in.basePointX2 = NULL;
        in.basePointY2 = NULL;
        in.basePointZ2 = NULL;
    } else {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    out.ptX = Qx;
    out.ptY = Qy;

    status = HAL_PKA_ECCDoubleBaseLadder(&hpka, &in, 5000);
    if (status != HAL_OK) {
        LOG_ERR("HAL_PKA_ECCDoubleBaseLadder failed with ret=%d", status);
        return -ENODEV;
    }
    HAL_PKA_ECCDoubleBaseLadder_GetResult(&hpka, &out);
    return 0;
}

int mbedtls_ecdsa_genkey(mbedtls_ecp_keypair *ctx, mbedtls_ecp_group_id gid,
                         int (*f_rng)(void *, unsigned char *, size_t), void *p_rng)
{
    int ret = 0;
    size_t priv_len = 0, pub_len = 0;
    uint8_t rand_priv[66] = {0};
    uint8_t Qx[66] = {0}, Qy[66] = {0};

    switch (gid) {
        case MBEDTLS_ECP_DP_SECP192R1:
            priv_len = pub_len = 24;
            break;
        case MBEDTLS_ECP_DP_SECP256R1:
            priv_len = pub_len = 32;
            break;
        case MBEDTLS_ECP_DP_SECP384R1:
            priv_len = pub_len = 48;
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
            mbedtls_mpi_read_binary(&mpi_n, secp192r1_n, priv_len);
            break;
        case MBEDTLS_ECP_DP_SECP256R1:
            mbedtls_mpi_read_binary(&mpi_n, secp256r1_n, priv_len);
            break;
        case MBEDTLS_ECP_DP_SECP384R1:
            mbedtls_mpi_read_binary(&mpi_n, secp384r1_n, priv_len);
            break;
        default:
            mbedtls_mpi_free(&mpi_n);
            return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    do {
        if ((ret = f_rng(p_rng, rand_priv, priv_len)) != 0)
            return ret;
        mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(d), rand_priv, priv_len);
    } while (mbedtls_mpi_cmp_int(&ctx->MBEDTLS_PRIVATE(d), 1) < 0 ||
             mbedtls_mpi_cmp_mpi(&ctx->MBEDTLS_PRIVATE(d), &mpi_n) >= 0);
    mbedtls_mpi_free(&mpi_n);

    LOG_HEXDUMP_INF(rand_priv, priv_len, "d");

    if (pka_compute_public_key(gid, rand_priv, Qx, Qy) != 0) {
        memset(Qx, 0, pub_len);
        memset(Qy, 0, pub_len);
        LOG_ERR("L:%d", __LINE__);
        return MBEDTLS_ERR_ECP_ALLOC_FAILED;
    }

    if ((ret = mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(d), rand_priv, priv_len)) != 0)
    {
        LOG_ERR("L:%d", __LINE__);
        return ret;
    }

    if ((ret = mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(X), Qx, pub_len)) != 0)
    {
        LOG_ERR("L:%d", __LINE__);
        return ret;
    }
    if ((ret = mbedtls_mpi_read_binary(&ctx->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Y), Qy, pub_len)) != 0)
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

int mbedtls_ecdh_gen_public( mbedtls_ecp_group *grp,
                             mbedtls_mpi *d, mbedtls_ecp_point *Q,
                             int (*f_rng)(void *, unsigned char *, size_t),
                             void *p_rng )
{
    int ret;
    size_t len = 0;
    uint8_t priv_key[66] = {0};
    uint8_t Qx[66] = {0}, Qy[66] = {0};

    switch(grp->id) {
        case MBEDTLS_ECP_DP_SECP192R1: len = 24; break;
        case MBEDTLS_ECP_DP_SECP256R1: len = 32; break;
        case MBEDTLS_ECP_DP_SECP384R1: len = 48; break;
        default: return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }

    if ((ret = f_rng(p_rng, priv_key, len)) != 0)
        return ret;

    LOG_HEXDUMP_INF(priv_key, len, "d");

    ret = pka_compute_public_key(grp->id, priv_key, Qx, Qy);
    if (ret != 0)
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;

    mbedtls_mpi_read_binary(d, priv_key, len);
    mbedtls_mpi_read_binary(&Q->MBEDTLS_PRIVATE(X), Qx, len);
    mbedtls_mpi_read_binary(&Q->MBEDTLS_PRIVATE(Y), Qy, len);
    mbedtls_mpi_lset(&Q->MBEDTLS_PRIVATE(Z), 1);

    return ret;
}

int mbedtls_ecdh_compute_shared( mbedtls_ecp_group *grp,
                                 mbedtls_mpi *z,
                                 const mbedtls_ecp_point *Q,
                                 const mbedtls_mpi *d,
                                 int (*f_rng)(void *, unsigned char *, size_t),
                                 void *p_rng )
{
    size_t size = 0;
    uint8_t d_bin[66] = {0}, Qx_bin[66] = {0}, Qy_bin[66] = {0};
    uint8_t sx[66] = {0}, sy[66] = {0};

    PKA_ECCDoubleBaseLadderInTypeDef in = {0};
    PKA_ECCDoubleBaseLadderOutTypeDef out = {0};

    switch(grp->id) {
        case MBEDTLS_ECP_DP_SECP192R1:
            size = 24;
            in.modulus = secp192r1_p;
            in.coefA = secp192r1_a;
            in.coefSign = secp192r1_coefSign;
            in.modulusSize = secp192r1_modulusSize;
            in.primeOrderSize = secp192r1_orderSize;
            in.integerM = secp192r1_m;
            in.basePointZ1 = secp192r1_Gz;
            break;

        case MBEDTLS_ECP_DP_SECP256R1:
            size = 32;
            in.modulus = secp256r1_p;
            in.coefA = secp256r1_a;
            in.coefSign = secp256r1_coefSign;
            in.modulusSize = secp256r1_modulusSize;
            in.primeOrderSize = secp256r1_orderSize;
            in.integerM = secp256r1_m;
            in.basePointZ1 = secp256r1_Gz;
            break;

        case MBEDTLS_ECP_DP_SECP384R1:
            size = 48;
            in.modulus = secp384r1_p;
            in.coefA = secp384r1_a;
            in.coefSign = secp384r1_coefSign;
            in.modulusSize = secp384r1_modulusSize;
            in.primeOrderSize = secp384r1_orderSize;
            in.integerM = secp384r1_m;
            in.basePointZ1 = secp384r1_Gz;
            break;

        default:
            return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }

    mbedtls_mpi_write_binary(d, d_bin, size);
    mbedtls_mpi_write_binary(&Q->MBEDTLS_PRIVATE(X), Qx_bin, size);
    mbedtls_mpi_write_binary(&Q->MBEDTLS_PRIVATE(Y), Qy_bin, size);

    in.integerK = d_bin;
    in.basePointX1 = Qx_bin;
    in.basePointY1 = Qy_bin;

    out.ptX = sx;
    out.ptY = sy;

    if (HAL_PKA_ECCDoubleBaseLadder(&hpka, &in, 5000) != HAL_OK)
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;

    HAL_PKA_ECCDoubleBaseLadder_GetResult(&hpka, &out);

    return mbedtls_mpi_read_binary(z, sx, size);
}

void pka_check_curve()
{
    static const uint8_t  secp192r1_k[24]       = {0xb1, 0xe7, 0x93, 0x39, 0xd0, 0x2a, 0x6c, 0x03, 0x31, 0xb0, 0x57, 0xb2, 0xe8, 0x4f, 0xb9, 0x3a, 0x26, 0x48, 0x34, 0x1d, 0x7c, 0x89, 0x1f, 0x3e};
    static const uint8_t  secp256r1_k[32]       = {0xb1, 0xe7, 0x93, 0x39, 0xd0, 0x2a, 0x6c, 0x03, 0x31, 0xb0, 0x57, 0xb2, 0xe8, 0x4f, 0xb9, 0x3a, 0x26, 0x48, 0x34, 0x1d, 0x7c, 0x89, 0x1f, 0x3e, 0x46, 0x59, 0x1c, 0xdb, 0x62, 0x00, 0xa8, 0x05};
    static const uint8_t  secp384r1_k[48]         = {0xb1, 0xe7, 0x93, 0x39, 0xd0, 0x2a, 0x6c, 0x03, 0x31, 0xb0, 0x57, 0xb2, 0xe8, 0x4f, 0xb9, 0x3a, 0x26, 0x48, 0x34, 0x1d, 0x7c, 0x89, 0x1f, 0x3e, 
                                                     0xb1, 0xe7, 0x93, 0x39, 0xd0, 0x2a, 0x6c, 0x03, 0x31, 0xb0, 0x57, 0xb2, 0xe8, 0x4f, 0xb9, 0x3a, 0x26, 0x48, 0x34, 0x1d, 0x7c, 0x89, 0x1f, 0x3e};
    uint8_t Qx[48];
    uint8_t Qy[48];
    
    pka_compute_public_key(MBEDTLS_ECP_DP_SECP192R1, secp192r1_k, Qx, Qy);
    LOG_HEXDUMP_INF(Qx, 24, "Qx");
    LOG_HEXDUMP_INF(Qy, 24, "Qy");

    pka_compute_public_key(MBEDTLS_ECP_DP_SECP256R1, secp256r1_k, Qx, Qy);
    LOG_HEXDUMP_INF(Qx, 32, "Qx");
    LOG_HEXDUMP_INF(Qy, 32, "Qy");

    pka_compute_public_key(MBEDTLS_ECP_DP_SECP384R1, secp384r1_k, Qx, Qy);
    LOG_HEXDUMP_INF(Qx, 48, "Qx");
    LOG_HEXDUMP_INF(Qy, 48, "Qy");
}