# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sample_with_mcuboot)

# Verify that this sample is built through sysbuild to ensure MCUboot is
# automatically included and that sample specific MCUboot configurations are
# used when building MCUboot.
test_sysbuild()

target_sources(app PRIVATE 
    src/main.c
    src/operation.c
    
    src/mbedtls/pka.c
    src/mbedtls/mbedtls-ecp-alt.c
    src/mbedtls/mbedtls-sha-alt.c
    src/mbedtls/mbedtls-ecdsa-alt.c
    src/mbedtls/mbedtls-ecdhe-alt.c
    src/mbedtls/mbedtls-exp-mod-alt.c

    src/check/aes.c
    src/check/sha.c
    src/check/pka.c
    src/check/crypto.c
    src/check/rsa.c
    src/check/ecdsa.c
    src/check/ecdhe.c

    src/net/sntp.c
    src/net/http.c
    src/net/https.c
    src/net/mqtts.c
    src/net/shell.c
)

