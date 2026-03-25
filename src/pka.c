#include <stdlib.h>
#include <zephyr/syscalls/time_units.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include <stm32h7rsxx_hal.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pka, LOG_LEVEL_DBG);

static PKA_HandleTypeDef hpka;

void pka_ecc_double_base_ladder()
{
    HAL_StatusTypeDef status = HAL_OK;

    /* vector 1 inputs and outputs */
    uint32_t input1_modulusSize                                      = 24;
    uint32_t input1_orderSize                                        = 24;
    uint32_t input1_coefSign                                         = 1;
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_MODULUS[24]          = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_A_COEFF[24]          = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_SCALAR_K[24]         = {0x25, 0x29, 0x7D, 0xD0, 0xD3, 0x4D, 0x26, 0xAB, 0x41, 0xC7, 0x18, 0xC5, 0x96, 0x4B, 0x41, 0xA7, 0xAD, 0x8C, 0xB6, 0x59, 0xA2, 0x84, 0xD5, 0x17};
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_SCALAR_M[24]         = {0xDC, 0xA9, 0x91, 0x69, 0x87, 0x7B, 0x16, 0xAF, 0xD5, 0xC9, 0x53, 0xBE, 0x11, 0xA5, 0x13, 0xC3, 0x14, 0x0F, 0xDC, 0xBC, 0x96, 0x40, 0x1A, 0xD0};
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT1_X[24] = {0xE7, 0xE5, 0x90, 0xDA, 0xC1, 0x2D, 0x4A, 0x2F, 0x21, 0x5E, 0x6E, 0x16, 0x3B, 0x52, 0x27, 0xFB, 0x48, 0x59, 0x81, 0xDF, 0x68, 0x02, 0xAA, 0xF7};
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT1_Y[24] = {0x65, 0x18, 0xE1, 0xF1, 0x67, 0xB1, 0x7A, 0xA8, 0x90, 0x87, 0xD1, 0x53, 0x3B, 0xD1, 0x26, 0x76, 0xC8, 0x12, 0x4A, 0xE8, 0x91, 0x9C, 0x71, 0x45};
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT1_Z[24] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT2_X[24] = {0xC8, 0xC1, 0x4C, 0x6E, 0x83, 0xDA, 0x2D, 0xAB, 0x99, 0xC4, 0xDB, 0xE7, 0xDB, 0x39, 0x5E, 0x54, 0x28, 0x55, 0x89, 0x45, 0x88, 0xBC, 0x74, 0x06};
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT2_Y[24] = {0x2B, 0x35, 0x8F, 0x4B, 0xE4, 0xA1, 0xA9, 0xEB, 0x7F, 0x46, 0x21, 0xEE, 0x9F, 0xF9, 0xD7, 0xCB, 0xE1, 0xEB, 0x42, 0xCE, 0x87, 0xC6, 0xD9, 0xE0};
    uint8_t input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT2_Z[24] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    uint8_t output1_X[24]                                            = {0x19, 0x51, 0xE5, 0x16, 0x72, 0x1D, 0x98, 0xB6, 0x9E, 0xEC, 0x34, 0x0A, 0x2B, 0xA1, 0xC2, 0x08, 0x03, 0x1C, 0xB6, 0xC7, 0xD3, 0x81, 0xCD, 0xEA};
    uint8_t output1_Y[24]                                            = {0x57, 0x1F, 0xA8, 0xFE, 0xCB, 0xA8, 0x5A, 0xD9, 0xBA, 0xFD, 0x47, 0xA6, 0x60, 0xA9, 0x5F, 0xF5, 0xE3, 0xCE, 0x22, 0x50, 0x42, 0xA9, 0x4D, 0x31};
    
    uint8_t buffer_x[24];
    uint8_t buffer_y[24];

    LOG_INF("PKA ECC Double Base Ladder");
    PKA_ECCDoubleBaseLadderInTypeDef in;
    PKA_ECCDoubleBaseLadderOutTypeDef out;

    /* Set input parameters */
    in.modulusSize = input1_modulusSize;
    in.primeOrderSize = input1_orderSize;
    in.modulus = input1_1PKA_ECC_DoubleBaseLadder_IN_MODULUS;
    in.coefSign = input1_coefSign;
    in.coefA = input1_1PKA_ECC_DoubleBaseLadder_IN_A_COEFF;
    in.integerK = input1_1PKA_ECC_DoubleBaseLadder_IN_SCALAR_K;
    in.integerM = input1_1PKA_ECC_DoubleBaseLadder_IN_SCALAR_M;
    in.basePointX1 = input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT1_X;
    in.basePointY1 = input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT1_Y;
    in.basePointZ1 = input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT1_Z;
    in.basePointX2 = input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT2_X;
    in.basePointY2 = input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT2_Y;
    in.basePointZ2 = input1_1PKA_ECC_DoubleBaseLadder_IN_INITIAL_POINT2_Z;

    /* set output parameters */
    out.ptX = &buffer_x[0];
    out.ptY = &buffer_y[0];

    /* Start PKA ECC Double Base Ladder operation */
    status = HAL_PKA_ECCDoubleBaseLadder(&hpka, &in, 5000);
    if (status != HAL_OK)
    {
        LOG_ERR("HAL_PKA_ECCDoubleBaseLadder failed with ret=%d", status);
    }
    /* retrieve computation result */
    HAL_PKA_ECCDoubleBaseLadder_GetResult(&hpka, &out);

    /* check retrieved result with expected result */
    if ((memcmp((uint8_t*)out.ptX, (uint8_t*)output1_X, 24) != 0) ||
        (memcmp((uint8_t*)out.ptY, (uint8_t*)output1_Y, 24) != 0))
    {
        /* HAL PKA Operation error */
        LOG_ERR("pka operation error!");
    }

    LOG_INF("pka success");
}

static void crypto_pka_init(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    hpka.Instance = PKA;

    __HAL_RCC_RNG_CLK_ENABLE();
    __HAL_RCC_PKA_CLK_ENABLE();

    status = HAL_PKA_Init(&hpka);

    if(status == HAL_OK)
    {
        LOG_INF("HAL_PKA_Init success.");
    }
    else
    {
        LOG_ERR("HAL_PKA_Init failed with ret=%d", status);
    }
}

int pka_operations(const struct shell *sh, size_t argc, char *argv[])
{
    int operation = atoi(argv[1]);
    
    LOG_DBG("operation %d selected", operation);

    if(operation == 0) {
        crypto_pka_init();
    }
    else if(operation == 1) {
        pka_ecc_double_base_ladder();
    }
    else {
        LOG_DBG("unknown operation");
    }

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(pka_commands,
	SHELL_CMD(num, NULL,
		"operation",
		pka_operations),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(pka, &pka_commands,
		   "example for pka", NULL);

