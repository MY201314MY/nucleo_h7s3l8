# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sample_with_mcuboot)

SET(ATCA_BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
SET(ATCA_PRINTF ON CACHE INTERNAL "")
SET(ATCA_HAL_I2C ON CACHE INTERNAL "")
SET(ATCA_NO_HEAP ON CACHE INTERNAL "")
add_subdirectory(cryptoauthlib/lib)

include_directories(app ${CMAKE_CURRENT_BINARY_DIR}/cryptoauthlib/lib 
                        ${CMAKE_CURRENT_SOURCE_DIR}/cryptoauthlib
                        ${CMAKE_CURRENT_SOURCE_DIR}/cryptoauthlib/lib
                        ${CMAKE_CURRENT_SOURCE_DIR}/cryptoauthlib/third_party/hal/zephyr)

# Verify that this sample is built through sysbuild to ensure MCUboot is
# automatically included and that sample specific MCUboot configurations are
# used when building MCUboot.
test_sysbuild()

target_sources(app PRIVATE src/main.c 
    src/crypto.c 
    src/operation.c 
    src/ecdsa.c 
    src/net/sntp.c
    src/net/http.c
    src/net/https.c
    src/net/mqtts.c
    src/net/terminal.c
)

target_link_libraries(cryptoauth PUBLIC zephyr_interface)
target_link_libraries(app PUBLIC cryptoauth)
