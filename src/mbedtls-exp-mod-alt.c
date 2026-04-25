#include <mbedtls/bignum.h>
#include <mbedtls/error.h>
#include <zephyr/kernel.h>
#include "stm32h7rsxx_hal.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(rsa_alt, LOG_LEVEL_DBG);

static PKA_HandleTypeDef hpka = { .Instance = PKA, .State = HAL_PKA_STATE_READY };

static uint8_t n_buf[512] = {0};
static uint8_t a_buf[512] = {0};
static uint8_t e_buf[512] = {0};
static uint8_t out_buf[512] = {0};

int mbedtls_mpi_exp_mod(mbedtls_mpi *X, const mbedtls_mpi *A,
                        const mbedtls_mpi *E, const mbedtls_mpi *N,
                        mbedtls_mpi *prec_RR)
{
    LOG_INF("mbedtls_mpi_exp_mod called with A=%d, E=%d, N=%d", mbedtls_mpi_size(A)*8, mbedtls_mpi_size(E)*8, mbedtls_mpi_size(N)*8);

    size_t n_len = mbedtls_mpi_size(N);
    size_t e_len = mbedtls_mpi_size(E);

    mbedtls_mpi_write_binary(N, n_buf, n_len);
    mbedtls_mpi_write_binary(A, a_buf, n_len);
    mbedtls_mpi_write_binary(E, e_buf, e_len);

    PKA_ModExpInTypeDef in = {0};
    in.OpSize = n_len;
    in.expSize = e_len;
    in.pOp1 = a_buf;
    in.pExp = e_buf;
    in.pMod = n_buf;

    if (HAL_PKA_ModExp(&hpka, &in, HAL_MAX_DELAY) != HAL_OK)
    {
        LOG_ERR("HAL_PKA_ModExp failed with ret=%d", HAL_PKA_GetError(&hpka));
        return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
    }

    HAL_PKA_ModExp_GetResult(&hpka, out_buf);

    return mbedtls_mpi_read_binary(X, out_buf, n_len);
}