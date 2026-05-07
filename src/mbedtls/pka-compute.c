#include <string.h>
#include <stm32h7rsxx_hal.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/bignum.h>
#include <mbedtls/platform_util.h>
#include <mbedtls/error.h>

#include "mbedtls-curve-alt.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pka, LOG_LEVEL_DBG);

static PKA_HandleTypeDef hpka = { .Instance = PKA, .State = HAL_PKA_STATE_READY };

int pka_compute_public_key(mbedtls_ecp_group_id gid, const uint8_t *priv_key, uint8_t *Qx, uint8_t *Qy)
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

void pka_check_curve()
{
    static const uint8_t  secp192r1_k[24]       = {0xb1, 0xe7, 0x93, 0x39, 0xd0, 0x2a, 0x6c, 0x03, 0x31, 0xb0, 0x57, 0xb2, 0xe8, 0x4f, 0xb9, 0x3a, 0x26, 0x48, 0x34, 0x1d, 0x7c, 0x89, 0x1f, 0x3e};
    static const uint8_t  secp256r1_k[32]       = {0xb1, 0xe7, 0x93, 0x39, 0xd0, 0x2a, 0x6c, 0x03, 0x31, 0xb0, 0x57, 0xb2, 0xe8, 0x4f, 0xb9, 0x3a, 0x26, 0x48, 0x34, 0x1d, 0x7c, 0x89, 0x1f, 0x3e, 0x46, 0x59, 0x1c, 0xdb, 0x62, 0x00, 0xa8, 0x05};
    static const uint8_t  secp384r1_k[48]       = {
        0xb1, 0xe7, 0x93, 0x39, 0xd0, 0x2a, 0x6c, 0x03, 0x31, 0xb0, 0x57, 0xb2, 0xe8, 0x4f, 0xb9, 0x3a, 0x26, 0x48, 0x34, 0x1d, 0x7c, 0x89, 0x1f, 0x3e, 
        0xb1, 0xe7, 0x93, 0x39, 0xd0, 0x2a, 0x6c, 0x03, 0x31, 0xb0, 0x57, 0xb2, 0xe8, 0x4f, 0xb9, 0x3a, 0x26, 0x48, 0x34, 0x1d, 0x7c, 0x89, 0x1f, 0x3e
    };
    uint8_t Qx[48] = {0};
    uint8_t Qy[48] = {0};

    LOG_INF("Check PKA with SECP192R1, SECP256R1 and SECP384R1 curves.");

    LOG_HEXDUMP_INF(secp192r1_k, sizeof(secp192r1_k), "d (SECP192R1)");
    pka_compute_public_key(MBEDTLS_ECP_DP_SECP192R1, secp192r1_k, Qx, Qy);
    LOG_HEXDUMP_INF(Qx, 24, "Qx (SECP192R1)");
    LOG_HEXDUMP_INF(Qy, 24, "Qy (SECP192R1)");
    printk("\r\n");

    LOG_HEXDUMP_INF(secp256r1_k, sizeof(secp256r1_k), "d (SECP256R1)");
    pka_compute_public_key(MBEDTLS_ECP_DP_SECP256R1, secp256r1_k, Qx, Qy);
    LOG_HEXDUMP_INF(Qx, 32, "Qx (SECP256R1)");
    LOG_HEXDUMP_INF(Qy, 32, "Qy (SECP256R1)");
    printk("\r\n");

    LOG_HEXDUMP_INF(secp384r1_k, sizeof(secp384r1_k), "d (SECP384R1)");
    pka_compute_public_key(MBEDTLS_ECP_DP_SECP384R1, secp384r1_k, Qx, Qy);
    LOG_HEXDUMP_INF(Qx, 48, "Qx (SECP384R1)");
    LOG_HEXDUMP_INF(Qy, 48, "Qy (SECP384R1)");
    printk("\r\n");
}