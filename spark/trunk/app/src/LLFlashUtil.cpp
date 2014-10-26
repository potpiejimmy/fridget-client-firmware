/* 
 * File:   LLFlashUtil.cpp
 * Author: thorsten
 *
 */
#include <stdio.h>

#include "LLFlashUtil.h"

#include "sst25vf_spi.h"

#define FLASH_USER_MEMORY_OFFSET 0x80000

namespace com_myfridget
{
    void LLFlashUtil::flash(const uint8_t *pBuffer, uint32_t address, uint32_t len) {
        sFLASH_EraseSector(FLASH_USER_MEMORY_OFFSET + address);
        sFLASH_WriteBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
    }
    
    void LLFlashUtil::read(uint8_t* pBuffer, uint32_t address, uint32_t len) {
        sFLASH_ReadBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
    }
}
