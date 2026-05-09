#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <stm32h7rsxx_hal.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(aes, LOG_LEVEL_DBG);

static CRYP_HandleTypeDef hcryp = {0};

static const uint32_t pKeySAES[8] = {
                            0xE3C08A8F,0x06C6E3AD,0x95A70557,0xB23F7548,0x3CE33021,0xA9C72B70,0x25666204,0xC69C0B72};
static const uint32_t pInitVectSAES[4] = {
                            0x12153524,0xC0895E81,0xB2C28465,0x00000002};
static const uint32_t HeaderSAES[7] = {
                            0xD609B1F0,0x56637A0D,0x46DF998D,0x88E52E00,0xB2C28465,0x12153524,0xC0895E81};

#define PLAINTEXT_SIZE    12

uint32_t Plaintext[12] = {0x08000F10,0x11121314,0x15161718,0x191A1B1C
                         ,0x1D1E1F20,0x21222324,0x25262728,0x292A2B2C
                         ,0x2D2E2F30,0x31323334,0x35363738,0x393A0002};

uint32_t Ciphertext[12] = {0xE2006EB4,0x2F527702,0x2D9B1992,0x5BC419D7
                          ,0xA592666C,0x925FE2EF,0x718EB4E3,0x08EFEAA7
                          ,0xC5273B39,0x4118860A,0x5BE2A97F,0x56AB7836};
uint32_t ExpectedTAG[4]= {0x5CA597CD,0xBB3EDB8D,0x1A1151EA,0x0AF7B436};

/* Used for storing the encrypted text */
uint32_t EncryptedText[12]={0};

/* Used for storing the decrypted text */
uint32_t DecryptedText[12]={0};

int crypto_aes_init()
{
    HAL_StatusTypeDef status = HAL_OK;

    hcryp.Instance = SAES;
    hcryp.Init.DataType = CRYP_DATATYPE_32B;
    hcryp.Init.KeySize = CRYP_KEYSIZE_256B;
    hcryp.Init.pKey = (uint32_t *)pKeySAES;
    hcryp.Init.pInitVect = (uint32_t *)pInitVectSAES;
    hcryp.Init.Algorithm = CRYP_AES_GCM;
    hcryp.Init.Header = (uint32_t *)HeaderSAES;
    hcryp.Init.HeaderSize = 7;
    hcryp.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_WORD;
    hcryp.Init.HeaderWidthUnit = CRYP_HEADERWIDTHUNIT_WORD;
    hcryp.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ALWAYS;
    hcryp.Init.KeyMode = CRYP_KEYMODE_NORMAL;
    hcryp.Init.KeySelect = CRYP_KEYSEL_NORMAL;

    __HAL_RCC_SAES_CLK_ENABLE();
    
    status = HAL_CRYP_Init(&hcryp);

    if (status != HAL_OK)
    {
        LOG_ERR("HAL_CRYP_Init failed with %d", status);
        return -EAGAIN;
    }

    return 0;
}

static int crypto_aes_operation(const struct shell *sh, size_t argc, char *argv[])
{
    int ret = -1;
    int operation = atoi(argv[1]);
    HAL_StatusTypeDef status = HAL_OK;
    
    LOG_DBG("operation %d selected", operation);

    if(operation == 0)
    {
        status = HAL_CRYP_Encrypt(&hcryp, Plaintext, PLAINTEXT_SIZE, EncryptedText, 0xFF);
        LOG_INF("status %d", status);
        if (status == HAL_OK)
        {
            LOG_HEXDUMP_INF((uint8_t*)EncryptedText, sizeof(EncryptedText), "encrypt");
        }
    }
    else if(operation == 1)
    {

    }
    else if(operation == 2)
    {
        
    }
    else
    {
        LOG_DBG("unknown operation");
    }
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(aes_commands,
	SHELL_CMD(num, NULL,
		"aes operaion",
		crypto_aes_operation),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(aes, &aes_commands,
		   "example for aes encrypt/decrypt", NULL);
