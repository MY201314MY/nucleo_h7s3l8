#include <mbedtls/ecdsa.h>
#include <mbedtls/error.h>
#include <mbedtls/ecp.h>
#include <stm32h7rsxx_hal.h>
#include <string.h>

/* --- SECP256R1 Curve Parameters (NIST P-256) --- */
static const uint8_t secp256r1_p[] = {
    0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};
static const uint8_t secp256r1_a[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03
};
static const uint8_t secp256r1_b[] = {
    0x5A,0xC6,0x35,0xD8,0xAA,0x3A,0x93,0xE7,0xB3,0xEB,0xBD,0x55,0x76,0x98,0x86,0xBC,
    0x65,0x1D,0x06,0xB0,0xCC,0x53,0xB0,0xF6,0x3B,0xCE,0x3C,0x3E,0x27,0xD2,0x60,0x4B
};
static const uint8_t secp256r1_Gx[] = {
    0x6B,0x17,0xd1,0xF2,0xE1,0x2C,0x42,0x47,0xF8,0xBC,0xE6,0xE5,0x63,0xA4,0x40,0xF2,
    0x77,0x03,0x7D,0x81,0x2D,0xEB,0x33,0xA0,0xF4,0xA1,0x39,0x45,0xD8,0x98,0xC2,0x96
};
static const uint8_t secp256r1_Gy[] = {
    0x4F,0xE3,0x42,0xE2,0xFE,0x1A,0x7F,0x9B,0x8E,0xE7,0xEB,0x4A,0x7C,0x0F,0x9E,0x16,
    0x2B,0xCE,0x33,0x57,0x6B,0x31,0x5E,0xCE,0xCB,0xB6,0x40,0x68,0x37,0xBF,0x51,0xf5
};
static const uint8_t secp256r1_n[] = {
    0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51
};

PKA_HandleTypeDef hpka = {
    .Instance = PKA,
    .State = HAL_PKA_STATE_READY
};

// =========================================================================
// 1. Key Generation (NIST P-256)
// =========================================================================
int mbedtls_ecdsa_genkey(mbedtls_ecdsa_context *ctx, mbedtls_ecp_group_id gid,
                          int (*f_rng)(void *, unsigned char *, size_t), void *p_rng)
{
    return mbedtls_ecp_gen_key(gid, ctx, f_rng, p_rng);
}

// =========================================================================
// 2. ECDSA Signature Acceleration
// =========================================================================
int mbedtls_ecdsa_sign(mbedtls_ecp_group *grp, mbedtls_mpi *r, mbedtls_mpi *s,
                       const mbedtls_mpi *d, const unsigned char *buf, size_t blen,
                       int (*f_rng)(void *, unsigned char *, size_t), void *p_rng)
{
    if (grp->id != MBEDTLS_ECP_DP_SECP256R1) return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;

    /* Buffers are static to ensure they reside in RAM (SRAM) and not on the thread stack */
    static uint8_t k_bin[32], d_buf[32], r_bin[32], s_bin[32];
    static PKA_ECDSASignInTypeDef in;
    static PKA_ECDSASignOutTypeDef out;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));

    if (f_rng(p_rng, k_bin, 32) != 0) return MBEDTLS_ERR_ECP_RANDOM_FAILED;
    mbedtls_mpi_write_binary(d, d_buf, 32);

    in.primeOrderSize = 32;
    in.modulusSize    = 32;
    in.coefSign       = 1;
    in.coef           = secp256r1_a;
    in.coefB          = secp256r1_b;
    in.modulus        = secp256r1_p;
    in.basePointX     = secp256r1_Gx;
    in.basePointY     = secp256r1_Gy;
    in.integer        = k_bin;
    in.hash           = buf;
    in.privateKey     = d_buf;
    in.primeOrder     = secp256r1_n;

    out.RSign = r_bin;
    out.SSign = s_bin;

    if (HAL_PKA_ECDSASign(&hpka, &in, 5000) != HAL_OK) return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    HAL_PKA_ECDSASign_GetResult(&hpka, &out, NULL);

    mbedtls_mpi_read_binary(r, r_bin, 32);
    mbedtls_mpi_read_binary(s, s_bin, 32);

    return 0;
}

// =========================================================================
// 3. ECDSA Verification Acceleration
// =========================================================================
int mbedtls_ecdsa_verify(mbedtls_ecp_group *grp, const unsigned char *buf, size_t blen,
                          const mbedtls_ecp_point *Q, const mbedtls_mpi *r, const mbedtls_mpi *s)
{
    if (grp->id != MBEDTLS_ECP_DP_SECP256R1) return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;

    static uint8_t r_bin[32], s_bin[32], q_bin[65];
    static PKA_ECDSAVerifInTypeDef in;
    size_t q_len = 0;

    memset(&in, 0, sizeof(in));

    mbedtls_mpi_write_binary(r, r_bin, 32);
    mbedtls_mpi_write_binary(s, s_bin, 32);
    mbedtls_ecp_point_write_binary(grp, Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &q_len, q_bin, sizeof(q_bin));

    in.primeOrderSize  = 32;
    in.modulusSize     = 32;
    in.coefSign        = 1;
    in.coef            = secp256r1_a;
    in.modulus         = secp256r1_p;
    in.basePointX      = secp256r1_Gx;
    in.basePointY      = secp256r1_Gy;
    in.pPubKeyCurvePtX = &q_bin[1];
    in.pPubKeyCurvePtY = &q_bin[33];
    in.RSign           = r_bin;
    in.SSign           = s_bin;
    in.hash            = buf;
    in.primeOrder      = secp256r1_n;

    if (HAL_PKA_ECDSAVerif(&hpka, &in, 5000) != HAL_OK) {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    if (HAL_PKA_ECDSAVerif_IsValidSignature(&hpka) == 1) {
        return 0; 
    } else {
        return MBEDTLS_ERR_ECP_VERIFY_FAILED; 
    }
}