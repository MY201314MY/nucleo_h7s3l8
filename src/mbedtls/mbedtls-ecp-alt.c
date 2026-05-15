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
    
    // 必须确保 4 字节对齐，否则 H7RS 的 PKA 可能因地址异常报错
    static uint8_t d_buf[48], qx_buf[48], qy_buf[48];
    static uint8_t res_x[48], res_y[48];
    static uint8_t p_buf[48], a_buf[48], n_buf[48];

    if (mbedtls_mpi_cmp_int(m, 0) == 0) return mbedtls_ecp_set_zero(R);
    if (plen > sizeof(d_buf)) return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;

    // 1. 导出常规参数
    mbedtls_mpi_write_binary(m, d_buf, plen);
    mbedtls_mpi_write_binary(&P->MBEDTLS_PRIVATE(X), qx_buf, plen);
    mbedtls_mpi_write_binary(&P->MBEDTLS_PRIVATE(Y), qy_buf, plen);
    mbedtls_mpi_write_binary(&grp->P, p_buf, plen);
    mbedtls_mpi_write_binary(&grp->N, n_buf, plen);

    // 2. 特殊处理参数 A：PKA 需要 |A|
    // 对于 NIST 曲线，A = -3。MbedTLS 内部 A 可能存为 p-3
    // 我们直接将其设为绝对值 3
    if (mbedtls_mpi_cmp_int(&grp->A, -3) == 0 || 
        mbedtls_mpi_cmp_mpi(&grp->A, &grp->P) > 0) { // 逻辑判断 A 是否为负
        memset(a_buf, 0, plen);
        a_buf[plen - 1] = 0x03; 
    } else {
        mbedtls_mpi_write_binary(&grp->A, a_buf, plen);
    }

    // 3. 准备 PKA 输入
    PKA_ECCMulInTypeDef  pka_in  = {0};
    PKA_ECCMulOutTypeDef pka_out = {0};

    pka_in.modulusSize   = plen;
    pka_in.modulus       = p_buf; 
    pka_in.coefA         = a_buf;
    pka_in.pointX        = qx_buf;
    pka_in.pointY        = qy_buf;
    pka_in.scalarMulSize = plen;
    pka_in.scalarMul     = d_buf;
    pka_in.primeOrder    = n_buf; 
    
    // 对于 NIST 曲线 (a=-3)，符号必须为 1
    pka_in.coefSign = (mbedtls_mpi_cmp_int(&grp->A, 0) < 0 || 
                       mbedtls_mpi_cmp_int(&grp->A, -3) == 0) ? 1 : 0;

    // 4. 执行计算
    status = HAL_PKA_ECCMul(&hpka, &pka_in, 2000);
    if (status != HAL_OK) {
        LOG_ERR("PKA ECCMul Failed: %d, PKA_Err: 0x%08x", 
                 status, HAL_PKA_GetError(&hpka));
        return -1;
    }

    // 5. 获取结果 (注意这里 res_x 和 res_y 都要赋值)
    pka_out.ptX = res_x;
    pka_out.ptY = res_y;
    HAL_PKA_ECCMul_GetResult(&hpka, &pka_out);

    LOG_WRN("ModSize: %d, P: %p, A: %p, X: %p, Scalar: %p", 
         pka_in.modulusSize, pka_in.modulus, pka_in.coefA, 
         pka_in.pointX, pka_in.scalarMul);

    // 6. 将结果导回 Mbed TLS
    mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(X), res_x, plen);
    mbedtls_mpi_read_binary(&R->MBEDTLS_PRIVATE(Y), res_y, plen);
    mbedtls_mpi_lset(&R->MBEDTLS_PRIVATE(Z), 1);

    return 0;
}