#include <stdio.h>
#include <string.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecp.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ecdhe, LOG_LEVEL_DBG);

/**
 * @brief Performs an ECDHE test using low-level compute_shared APIs.
 * This bypasses the private structure constraints of mbedtls_ecdh_context in 3.x
 * and directly triggers the ECP multiplication hooks for ATECC608 acceleration.
 */

int _ecdhe(int type)
{
    int ret = 1;
    mbedtls_ecp_group_id curve;
    size_t key_len = 0;

    mbedtls_ecp_group grp;
    mbedtls_mpi d_cli, d_srv, z_cli, z_srv;
    mbedtls_ecp_point Q_cli, Q_srv;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    unsigned char secret_cli[66] = {0};
    unsigned char secret_srv[66] = {0};

    switch(type) {
        case 0:
            curve = MBEDTLS_ECP_DP_SECP192R1;
            key_len = 24;
            LOG_INF("[ECDHE] Using SECP192R1");
            break;
        case 1:
            curve = MBEDTLS_ECP_DP_SECP256R1;
            key_len = 32;
            LOG_INF("[ECDHE] Using SECP256R1");
            break;
        case 2:
            curve = MBEDTLS_ECP_DP_SECP384R1;
            key_len = 48;
            LOG_INF("[ECDHE] Using SECP384R1");
            break;
        default:
            LOG_ERR("Unknown type: %d", type);
            return -1;
    }

    LOG_INF("=====================================================");
    LOG_INF("--- ECDHE (Low-Level API) ---");
    LOG_INF("=====================================================");

    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&Q_cli);
    mbedtls_ecp_point_init(&Q_srv);
    mbedtls_mpi_init(&d_cli);
    mbedtls_mpi_init(&d_srv);
    mbedtls_mpi_init(&z_cli);
    mbedtls_mpi_init(&z_srv);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    LOG_INF("step 1: seeding RNG and loading curve...");
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);
    if (ret != 0) {
        LOG_ERR("mbedtls_ctr_drbg_seed failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }
    ret = mbedtls_ecp_group_load(&grp, curve);
    if (ret != 0) {
        LOG_ERR("mbedtls_ecp_group_load failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }

    LOG_INF("step 2: generating ephemeral keypairs...");
    ret = mbedtls_ecdh_gen_public(&grp, &d_cli, &Q_cli, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        LOG_ERR("client keypair generation failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }
    ret = mbedtls_ecdh_gen_public(&grp, &d_srv, &Q_srv, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        LOG_ERR("server keypair generation failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }

    LOG_INF("step 3: computing shared secrets...");
    LOG_INF("  -> client side calculating...");
    ret = mbedtls_ecdh_compute_shared(&grp, &z_cli, &Q_srv, &d_cli, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        LOG_ERR("client side computation failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }
    LOG_INF("  -> server side calculating...");
    ret = mbedtls_ecdh_compute_shared(&grp, &z_srv, &Q_cli, &d_srv, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        LOG_ERR("server side computation failed: -0x%04x", (unsigned int)-ret);
        goto exit;
    }

    LOG_INF("step 4: exporting results and verifying match...");
    ret = mbedtls_mpi_write_binary(&z_cli, secret_cli, key_len);
    if (ret != 0) goto exit;
    ret = mbedtls_mpi_write_binary(&z_srv, secret_srv, key_len);
    if (ret != 0) goto exit;

    if (memcmp(secret_cli, secret_srv, key_len) == 0) {
        LOG_INF("[success] shared Secrets match! (%d bytes)", (int)key_len);
    } else {
        LOG_ERR("[failure] shared Secrets mismatch!");
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