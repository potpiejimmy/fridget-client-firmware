/* 
 * File:   LLFlashUtil.cpp
 * Author: thorsten
 *
 */
#include <stdio.h>

#include "LLFlashUtil.h"

#include <cstring>
#include "spark_wiring.h"
#include "sst25vf_spi.h"

#define FLASH_USER_MEMORY_OFFSET 0x80000

namespace com_myfridget
{
    bool LLFlashUtil::flash(const uint8_t *pBuffer, uint32_t address, uint32_t len) {

        const uint8_t *writeBuffer = pBuffer;
        uint8_t readBuffer[len];

        sFLASH_WriteBuffer(writeBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
        sFLASH_ReadBuffer(readBuffer, FLASH_USER_MEMORY_OFFSET + address, len);

        /* Is the Data Buffer successfully programmed to SPI Flash memory */
        if (memcmp(writeBuffer, readBuffer, len))
        {
            /* Erase the problematic SPI Flash pages */
            sFLASH_EraseSector(((uint32_t)((FLASH_USER_MEMORY_OFFSET + address) / sFLASH_PAGESIZE)) * sFLASH_PAGESIZE);
            delay(20);

            /* try again */
            sFLASH_WriteBuffer(writeBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
            sFLASH_ReadBuffer(readBuffer, FLASH_USER_MEMORY_OFFSET + address, len);

            return memcmp(writeBuffer, readBuffer, len);
        }

        return TRUE;
    }
    
    void LLFlashUtil::read(uint8_t* pBuffer, uint32_t address, uint32_t len) {
        sFLASH_ReadBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
    }
}
