/* 
 * File:   LLFlashUtil.cpp
 * Author: thorsten
 *
 */
#include "LLFlashUtil.h"

#include "application.h"
#include "sst25vf_spi.h"

#define FLASH_USER_MEMORY_OFFSET 0x80000

namespace com_myfridget
{
    bool LLFlashUtil::flash(const uint8_t *pBuffer, uint32_t address, uint32_t len) {

        // XXX WARNING: with the current implementation, len should be == sFLASH_PAGESIZE
        // and the address must be a multiple of sFLASH_PAGESIZE
        
//        const uint8_t *writeBuffer = pBuffer;
//        uint8_t readBuffer[len];

        /* Erase the current SPI flash page */
        sFLASH_EraseSector(((uint32_t)((FLASH_USER_MEMORY_OFFSET + address) / sFLASH_PAGESIZE)) * sFLASH_PAGESIZE);
        delayRealMicros(20000);

        /* write */
        sFLASH_WriteBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
//        sFLASH_ReadBuffer(readBuffer, FLASH_USER_MEMORY_OFFSET + address, len);

        return TRUE; //memcmp(writeBuffer, readBuffer, len) == 0;
    }
    
    void LLFlashUtil::read(uint8_t* pBuffer, uint32_t address, uint32_t len) {
        sFLASH_ReadBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
    }
}
