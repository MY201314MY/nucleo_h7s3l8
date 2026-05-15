#include <string.h>
#include <stm32h7rsxx_hal.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/bignum.h>
#include <mbedtls/platform_util.h>
#include <mbedtls/error.h>

#include "mbedtls-curve.h"

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
