#include <stdio.h>
#include <string.h>

/* mbedTLS headers */
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecp.h>

/* Zephyr headers */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ecdhe, LOG_LEVEL_DBG);

/**
 * @brief Performs an ECDHE test using low-level compute_shared APIs.
 * This bypasses the private structure constraints of mbedtls_ecdh_context in 3.x
 * and directly triggers the ECP multiplication hooks for ATECC608 acceleration.
 */

int _ecdhe(void)
{
    int ret = 1;

    mbedtls_ecp_group grp;
    mbedtls_mpi d_cli, d_srv, z_cli, z_srv;
    mbedtls_ecp_point Q_cli, Q_srv;
    
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    
    unsigned char secret_cli[32] = { 0 };
    unsigned char secret_srv[32] = { 0 };

    LOG_INF("=====================================================");
    LOG_INF("--- starting SECP256R1 ECDHE (Low-Level API)      ---");
    LOG_INF("=====================================================");

    /* Step 1: Initialization */
    LOG_INF("step 1: initializing ECP structures and RNG...");
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&Q_cli);
    mbedtls_ecp_point_init(&Q_srv);
    mbedtls_mpi_init(&d_cli);
    mbedtls_mpi_init(&d_srv);
    mbedtls_mpi_init(&z_cli);
    mbedtls_mpi_init(&z_srv);
    
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    /* Step 2: RNG Seeding and Group Loading */
    LOG_INF("step 2: seeding RNG and loading SECP256R1 curve...");
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);
    if (ret != 0) {
        LOG_ERR("mbedtls_ctr_drbg_seed failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }

    ret = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    if (ret != 0) {
        LOG_ERR("mbedtls_ecp_group_load failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }

    /* Step 3: Keypair Generation */
    LOG_INF("step 3: generating ephemeral keypairs using mbedtls_ecdh_gen_public...");
    
    // Client
    ret = mbedtls_ecdh_gen_public(&grp, &d_cli, &Q_cli, 
                                  mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        LOG_ERR("client keypair generation failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }

    // Server
    ret = mbedtls_ecdh_gen_public(&grp, &d_srv, &Q_srv, 
                                  mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        LOG_ERR("server keypair generation failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }

    /* Step 4: Shared Secret Computation */
    /* This triggers your mbedtls_ecp_mul hook (atcab_ecdh) */
    LOG_INF("step 4: computing shared secrets via mbedtls_ecdh_compute_shared...");

    LOG_INF("  -> client side calculating...");
    ret = mbedtls_ecdh_compute_shared(&grp, &z_cli, &Q_srv, &d_cli, 
                                      mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        LOG_ERR("client side computation failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }

    LOG_INF("  -> server side calculating...");
    ret = mbedtls_ecdh_compute_shared(&grp, &z_srv, &Q_cli, &d_srv, 
                                      mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        LOG_ERR("server side computation failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }

    /* Step 5: Comparison */
    LOG_INF("step 5: exporting results and verifying match...");
    
    ret = mbedtls_mpi_write_binary(&z_cli, secret_cli, 32);
    if (ret != 0) goto exit;
    
    ret = mbedtls_mpi_write_binary(&z_srv, secret_srv, 32);
    if (ret != 0) goto exit;

    if (memcmp(secret_cli, secret_srv, 32) == 0) {
        LOG_INF("[success] shared Secrets match! (32 bytes)");
    } else {
        LOG_ERR("[failure] shared Secrets mismatch!");
        ret = -1;
    }

exit:
    mbedtls_ecp_group_free(&grp);
    mbedtls_ecp_point_free(&Q_cli);
    mbedtls_ecp_point_free(&Q_srv);
    mbedtls_mpi_free(&d_cli);
    mbedtls_mpi_free(&d_srv);
    mbedtls_mpi_free(&z_cli);
    mbedtls_mpi_free(&z_srv);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return ret;
}