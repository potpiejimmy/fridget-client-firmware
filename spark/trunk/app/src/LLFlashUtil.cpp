/* 
 * File:   LLFlashUtil.cpp
 * Author: thorsten
 *
 */
#include "LLFlashUtil.h"

#include "application.h"
#ifdef PLATFORM_PHOTON
#include "flash_mal.h"
#else
#include "sst25vf_spi.h"
#endif

#ifdef PLATFORM_PHOTON
#define FLASH_USER_MEMORY_OFFSET 0x80C0000
#else
#define FLASH_USER_MEMORY_OFFSET 0x80000
#endif

namespace com_myfridget
{
    bool LLFlashUtil::flash(const uint8_t *pBuffer, uint32_t address, uint32_t len) {

#ifndef PLATFORM_PHOTON
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
#else
        FLASH_Update(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
        return TRUE;
#endif
    }
    
    void LLFlashUtil::read(uint8_t* pBuffer, uint32_t address, uint32_t len) {
        
#ifndef PLATFORM_PHOTON
        sFLASH_ReadBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
#else
        memcpy(pBuffer, (const void*) *(__IO uint32_t*) (FLASH_USER_MEMORY_OFFSET + address), len);
#endif
    }
}
