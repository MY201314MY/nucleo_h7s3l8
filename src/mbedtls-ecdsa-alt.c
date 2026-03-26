#include <mbedtls/ecdsa.h>
#include <mbedtls/error.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/sha256.h>
#include <stm32h7rsxx_hal.h>
#include <string.h>

/* --- 曲线参数 (SECP256R1) --- */
static const uint8_t P256_Prime[] = {
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
static const uint8_t P256_absA[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03
};
static const uint8_t P256_B[] = {
    0x5a, 0xc6, 0x35, 0xd8, 0xaa, 0x3a, 0x93, 0xe7, 0xb3, 0xeb, 0xbd, 0x55, 0x76, 0x98, 0x86, 0xbc, 
    0x65, 0x1d, 0x06, 0xb0, 0xcc, 0x53, 0xb0, 0xf6, 0x3b, 0xce, 0x3c, 0x3e, 0x27, 0xd2, 0x60, 0x4b
};
static const uint8_t P256_Order[] = {
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17, 0x9e, 0x84, 0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x51
};
static const uint8_t P256_GX[] = {
    0x6b, 0x17, 0xd1, 0xf2, 0xe1, 0x2c, 0x42, 0x47, 0xf8, 0xbc, 0xe6, 0xe5, 0x63, 0xa4, 0x40, 0xf2, 
    0x77, 0x03, 0x7d, 0x81, 0x2d, 0xeb, 0x33, 0xa0, 0xf4, 0xa1, 0x39, 0x45, 0xd8, 0x98, 0xc2, 0x96
};
static const uint8_t P256_GY[] = {
    0x4f, 0xe3, 0x42, 0xe2, 0xfe, 0x1a, 0x7f, 0x9b, 0x8e, 0xe7, 0xeb, 0x4a, 0x7c, 0x0f, 0x9e, 0x16, 
    0x2b, 0xce, 0x33, 0x57, 0x6b, 0x31, 0x5e, 0xce, 0xcb, 0xb6, 0x40, 0x68, 0x37, 0xbf, 0x51, 0xf5
};

PKA_HandleTypeDef hpka = {.Instance = PKA};

/* --- 1. 生成密钥对加速 --- */
int mbedtls_ecdsa_genkey(mbedtls_ecdsa_context *ctx, mbedtls_ecp_group_id gid,
                          int (*f_rng)(void *, unsigned char *, size_t), void *p_rng) {
    if (gid != MBEDTLS_ECP_DP_SECP256R1) return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;

    /* 全部设为 static：确保 PKA 访问的是数据段地址而非受限的栈地址 */
    static PKA_ECCMulInTypeDef in;
    static PKA_ECCMulOutTypeDef out;
    static uint8_t d_raw[32]; 
    static uint8_t q_raw[65]; 

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    memset(d_raw, 0, sizeof(d_raw));
    memset(q_raw, 0, sizeof(q_raw));

    /* 1. 生成随机私钥 */
    if (f_rng(p_rng, d_raw, 32) != 0) return MBEDTLS_ERR_ECP_RANDOM_FAILED;

    /* 2. 配置输入 */
    in.modulusSize = 32;
    in.modulus     = P256_Prime;
    in.coefA       = P256_absA;
    in.coefSign    = 1;
    in.scalarMul   = d_raw; 
    in.pointX      = P256_GX;
    in.pointY      = P256_GY;

    /* 3. 配置输出 */
    out.ptX = &q_raw[1];  // X 坐标存入 q_raw[1..32]
    out.ptY = &q_raw[33]; // Y 坐标存入 q_raw[33..64]
    q_raw[0] = 0x04;      // 非压缩格式头

    /* 4. 硬件加速运算 */
    if (HAL_PKA_ECCMul(&hpka, &in, 5000) != HAL_OK) {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }
    
    HAL_PKA_ECCMul_GetResult(&hpka, &out);

    /* 5. 将结果读回 mbedTLS 结构体 */
    mbedtls_mpi_read_binary(&(ctx->MBEDTLS_PRIVATE(d)), d_raw, 32);
    mbedtls_ecp_point_read_binary(&(ctx->MBEDTLS_PRIVATE(grp)), &(ctx->MBEDTLS_PRIVATE(Q)), q_raw, 65);

    return 0;
}

/* --- 2. 签名加速 --- */
int mbedtls_ecdsa_sign(mbedtls_ecp_group *grp, mbedtls_mpi *r, mbedtls_mpi *s,
                       const mbedtls_mpi *d, const unsigned char *buf, size_t blen,
                       int (*f_rng)(void *, unsigned char *, size_t), void *p_rng) {
    if (grp->id != MBEDTLS_ECP_DP_SECP256R1) return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    if (blen != 32) return MBEDTLS_ERR_ECP_BAD_INPUT_DATA; // 硬件通常要求 Hash 长度匹配

    static PKA_ECDSASignInTypeDef in;
    static PKA_ECDSASignOutTypeDef out;
    uint8_t d_raw[32], k_raw[32], r_raw[32], s_raw[32];

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));

    if (f_rng(p_rng, k_raw, 32) != 0) return MBEDTLS_ERR_ECP_RANDOM_FAILED;
    mbedtls_mpi_write_binary(d, d_raw, 32);

    in.primeOrderSize = 32;
    in.modulusSize    = 32;
    in.coefSign       = 1;
    in.coef           = P256_absA;
    in.coefB          = P256_B;
    in.modulus        = P256_Prime;
    in.basePointX     = P256_GX;
    in.basePointY     = P256_GY;
    in.primeOrder     = P256_Order;
    in.integer        = k_raw;
    in.privateKey     = d_raw;
    in.hash           = buf; // 必须保证 buf 不为 NULL

    if (HAL_PKA_ECDSASign(&hpka, &in, 5000) != HAL_OK) return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;

    out.RSign = r_raw;
    out.SSign = s_raw;
    HAL_PKA_ECDSASign_GetResult(&hpka, &out, NULL);

    mbedtls_mpi_read_binary(r, r_raw, 32);
    mbedtls_mpi_read_binary(s, s_raw, 32);

    return 0;
}

/* --- 3. 验签加速 --- */
int mbedtls_ecdsa_verify(mbedtls_ecp_group *grp, const unsigned char *buf, size_t blen,
                          const mbedtls_ecp_point *Q, const mbedtls_mpi *r, const mbedtls_mpi *s) {
    if (grp->id != MBEDTLS_ECP_DP_SECP256R1) return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;

    static PKA_ECDSAVerifInTypeDef in;
    uint8_t q_raw[65], r_raw[32], s_raw[32];
    size_t olen;

    memset(&in, 0, sizeof(in));

    mbedtls_ecp_point_write_binary(grp, Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, q_raw, 65);
    mbedtls_mpi_write_binary(r, r_raw, 32);
    mbedtls_mpi_write_binary(s, s_raw, 32);

    in.primeOrderSize   = 32;
    in.modulusSize      = 32;
    in.coefSign         = 1;
    in.coef             = P256_absA;
    in.modulus          = P256_Prime;
    in.basePointX       = P256_GX;
    in.basePointY       = P256_GY;
    in.primeOrder       = P256_Order;
    in.pPubKeyCurvePtX  = &q_raw[1]; 
    in.pPubKeyCurvePtY  = &q_raw[33]; 
    in.RSign            = r_raw;
    in.SSign            = s_raw;
    in.hash             = buf;

    if (HAL_PKA_ECDSAVerif(&hpka, &in, 5000) != HAL_OK) return MBEDTLS_ERR_ECP_VERIFY_FAILED;

    return 0;
}