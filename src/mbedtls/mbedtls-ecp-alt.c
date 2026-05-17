#include <string.h>
#include <stm32h7rsxx_hal.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/bignum.h>
#include <mbedtls/platform_util.h>
#include <mbedtls/error.h>

#include "mbedtls-curve.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ecp_alt, LOG_LEVEL_DBG);

static PKA_HandleTypeDef hpka = { .Instance = PKA, .State = HAL_PKA_STATE_READY };
/*
 * Multiplication R = m * P
 */
int mbedtls_ecp_mul_restartable(mbedtls_ecp_group *grp, mbedtls_ecp_point *R,
                                const mbedtls_mpi *m, const mbedtls_ecp_point *P,
                                int (*f_rng)(void *, unsigned char *, size_t), void *p_rng,
                                mbedtls_ecp_restart_ctx *rs_ctx)
{
    HAL_StatusTypeDef status;
    size_t length = mbedtls_mpi_size(&grp->P);
    
    uint8_t d_buf[48], Px[48], Py[48];
    uint8_t res_x[48], res_y[48];
    uint8_t _P[48], _N[48], _B[48];

    if (mbedtls_mpi_cmp_int(m, 0) == 0) return mbedtls_ecp_set_zero(R);
    if (length > sizeof(d_buf)) return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;

    mbedtls_mpi_write_binary(m, d_buf, length);
    mbedtls_mpi_write_binary(&P->MBEDTLS_PRIVATE(X), Px, length);
    mbedtls_mpi_write_binary(&P->MBEDTLS_PRIVATE(Y), Py, length);
    mbedtls_mpi_write_binary(&grp->P, _P, length);
    mbedtls_mpi_write_binary(&grp->N, _N, length);
    mbedtls_mpi_write_binary(&grp->B, _B, length);

    PKA_ECCMulInTypeDef  pka_in  = {0};
    PKA_ECCMulOutTypeDef pka_out = {0};

    pka_in.modulusSize   = length;
    pka_in.coefSign = 1;
    if(length == 24)
    {
        pka_in.coefA         = secp192r1_a;
    }
    else if (length == 32)
    {
        pka_in.coefA         = secp256r1_a;
    }
    else
    {
        pka_in.coefA         = secp384r1_a;
    }

    pka_in.modulus       = _P; 
    pka_in.pointX        = Px;
    pka_in.pointY        = Py;
    
    pka_in.scalarMulSize = length;
    pka_in.scalarMul     = d_buf;
    pka_in.primeOrder    = _N;

    pka_in.coefB = _B;

    status = HAL_PKA_ECCMul(&hpka, &pka_in, 2000);
    if (status != HAL_OK) {
        LOG_ERR("PKA ECCMul Failed: %d, PKA_Err: 0x%08x", 
                 status, HAL_PKA_GetError(&hpka));
        return -1;
    }

    pka_out.ptX = res_x;
    pka_out.ptY = res_y;
    HAL_PKA_ECCMul_GetResult(&hpka, &pka_out);

    mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(X), res_x, length);
    mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(Y), res_y, length);
    mbedtls_mpi_lset(&R->MBEDTLS_PRIVATE(Z), 1);

    return 0;
}

int mbedtls_ecp_muladd_restartable(
    mbedtls_ecp_group *grp, mbedtls_ecp_point *R,
    const mbedtls_mpi *m, const mbedtls_ecp_point *P,
    const mbedtls_mpi *n, const mbedtls_ecp_point *Q,
    mbedtls_ecp_restart_ctx *rs_ctx)
{
    size_t size = 0;
    uint8_t m_bin[66] = {0}, n_bin[66] = {0};
    uint8_t Px_bin[66] = {0}, Py_bin[66] = {0};
    uint8_t Qx_bin[66] = {0}, Qy_bin[66] = {0};
    uint8_t rx[66] = {0}, ry[66] = {0};

    PKA_ECCDoubleBaseLadderInTypeDef in = {0};
    PKA_ECCDoubleBaseLadderOutTypeDef out = {0};

    LOG_DBG("ecp_muladd");

    switch(grp->id) {
        case MBEDTLS_ECP_DP_SECP192R1:
            size = 24;
            in.modulus = secp192r1_p;
            in.coefA = secp192r1_a;
            in.coefSign = secp192r1_coefSign;
            in.modulusSize = secp192r1_modulusSize;
            in.primeOrderSize = secp192r1_orderSize;
            in.basePointZ1 = secp192r1_Gz;
            in.basePointZ2 = secp192r1_Gz;
            mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(Z), secp192r1_Gz, size);
            break;

        case MBEDTLS_ECP_DP_SECP256R1:
            size = 32;
            in.modulus = secp256r1_p;
            in.coefA = secp256r1_a;
            in.coefSign = secp256r1_coefSign;
            in.modulusSize = secp256r1_modulusSize;
            in.primeOrderSize = secp256r1_orderSize;
            in.basePointZ1 = secp256r1_Gz;
            in.basePointZ2 = secp256r1_Gz;
            mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(Z), secp256r1_Gz, size);
            break;

        case MBEDTLS_ECP_DP_SECP384R1:
            size = 48;
            in.modulus = secp384r1_p;
            in.coefA = secp384r1_a;
            in.coefSign = secp384r1_coefSign;
            in.modulusSize = secp384r1_modulusSize;
            in.primeOrderSize = secp384r1_orderSize;
            in.basePointZ1 = secp384r1_Gz;
            in.basePointZ2 = secp384r1_Gz;
            mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(Z), secp384r1_Gz, size);
            break;

        default:
            return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }

    mbedtls_mpi_write_binary(m, m_bin, size);
    mbedtls_mpi_write_binary(n, n_bin, size);

    mbedtls_mpi_write_binary(&P->MBEDTLS_PRIVATE(X), Px_bin, size);
    mbedtls_mpi_write_binary(&P->MBEDTLS_PRIVATE(Y), Py_bin, size);

    mbedtls_mpi_write_binary(&Q->MBEDTLS_PRIVATE(X), Qx_bin, size);
    mbedtls_mpi_write_binary(&Q->MBEDTLS_PRIVATE(Y), Qy_bin, size);

    in.integerK      = m_bin;
    in.integerM      = n_bin;

    in.basePointX1   = Px_bin;
    in.basePointY1   = Py_bin;

    in.basePointX2   = Qx_bin;
    in.basePointY2   = Qy_bin;

    out.ptX = rx;
    out.ptY = ry;

    if (HAL_PKA_ECCDoubleBaseLadder(&hpka, &in, 5000) != HAL_OK)
    {
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    HAL_PKA_ECCDoubleBaseLadder_GetResult(&hpka, &out);

    mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(X), rx, size);
    mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(Y), ry, size);

    return 0;
}

