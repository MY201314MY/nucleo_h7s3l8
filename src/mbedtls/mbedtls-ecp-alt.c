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
int mbedtls_ecp_mul(mbedtls_ecp_group *grp, mbedtls_ecp_point *R,
                    const mbedtls_mpi *m, const mbedtls_ecp_point *P,
                    int (*f_rng)(void *, unsigned char *, size_t), void *p_rng)
{
    HAL_StatusTypeDef status;
    size_t plen = mbedtls_mpi_size(&grp->P);
    
    uint8_t d_buf[48], qx_buf[48], qy_buf[48];
    uint8_t res_x[48], res_y[48];
    uint8_t p_buf[48], n_buf[48], b_buf[48];

    if (mbedtls_mpi_cmp_int(m, 0) == 0) return mbedtls_ecp_set_zero(R);
    if (plen > sizeof(d_buf)) return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;

    mbedtls_mpi_write_binary(m, d_buf, plen);
    mbedtls_mpi_write_binary(&P->MBEDTLS_PRIVATE(X), qx_buf, plen);
    mbedtls_mpi_write_binary(&P->MBEDTLS_PRIVATE(Y), qy_buf, plen);
    mbedtls_mpi_write_binary(&grp->P, p_buf, plen);
    mbedtls_mpi_write_binary(&grp->N, n_buf, plen);
    mbedtls_mpi_write_binary(&grp->B, b_buf, plen);

    PKA_ECCMulInTypeDef  pka_in  = {0};
    PKA_ECCMulOutTypeDef pka_out = {0};

    pka_in.modulusSize   = plen;
    pka_in.coefSign = 1;
    if(plen == 24)
    {
        pka_in.coefA         = secp192r1_a;
    }
    else if (plen == 32)
    {
        pka_in.coefA         = secp256r1_a;
    }
    else
    {
        pka_in.coefA         = secp384r1_a;
    }

    pka_in.modulus       = p_buf; 
    pka_in.pointX        = qx_buf;
    pka_in.pointY        = qy_buf;
    
    pka_in.scalarMulSize = plen;
    pka_in.scalarMul     = d_buf;
    pka_in.primeOrder    = n_buf;

    pka_in.coefB = b_buf;

    status = HAL_PKA_ECCMul(&hpka, &pka_in, 2000);
    if (status != HAL_OK) {
        LOG_ERR("PKA ECCMul Failed: %d, PKA_Err: 0x%08x", 
                 status, HAL_PKA_GetError(&hpka));
        return -1;
    }

    pka_out.ptX = res_x;
    pka_out.ptY = res_y;
    HAL_PKA_ECCMul_GetResult(&hpka, &pka_out);

    mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(X), res_x, plen);
    mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(Y), res_y, plen);
    mbedtls_mpi_lset(&R->MBEDTLS_PRIVATE(Z), 1);

    return 0;
}
