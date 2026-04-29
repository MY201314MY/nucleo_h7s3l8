#include <string.h>
#include <stm32h7rsxx_hal.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/bignum.h>
#include <mbedtls/platform_util.h>
#include <mbedtls/error.h>

#include "mbedtls-alt.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ecdhe_alt, LOG_LEVEL_DBG);

static PKA_HandleTypeDef hpka = { .Instance = PKA, .State = HAL_PKA_STATE_READY };

int mbedtls_ecdh_gen_public( mbedtls_ecp_group *grp,
                             mbedtls_mpi *d, mbedtls_ecp_point *Q,
                             int (*f_rng)(void *, unsigned char *, size_t),
                             void *p_rng )
{
    int ret;
    size_t len = 0;
    uint8_t priv_key[66] = {0};
    uint8_t Qx[66] = {0}, Qy[66] = {0};

    LOG_DBG("gen public");

    switch(grp->id) {
        case MBEDTLS_ECP_DP_SECP192R1: len = 24; break;
        case MBEDTLS_ECP_DP_SECP256R1: len = 32; break;
        case MBEDTLS_ECP_DP_SECP384R1: len = 48; break;
        default: return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }

    if ((ret = f_rng(p_rng, priv_key, len)) != 0)
        return ret;

    /*
        LOG_HEXDUMP_INF(priv_key, len, "d");
    */

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

    LOG_DBG("compute shared");

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
