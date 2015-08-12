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
        
        const uint8_t *writeBuffer = pBuffer;
        uint8_t readBuffer[len];

        /* Erase the current SPI flash page */
        sFLASH_EraseSector(((uint32_t)((FLASH_USER_MEMORY_OFFSET + address) / sFLASH_PAGESIZE)) * sFLASH_PAGESIZE);
        delayRealMicros(50000);

        /* write */
        sFLASH_WriteBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
        sFLASH_ReadBuffer(readBuffer, FLASH_USER_MEMORY_OFFSET + address, len);

        return memcmp(writeBuffer, readBuffer, len) == 0;
#else
        FLASH_EraseMemory(FLASH_INTERNAL, FLASH_USER_MEMORY_OFFSET + address, len);
        
        FLASH_Update(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
        
//        Serial.println(String("FLASHED ")+pBuffer[0]+","+pBuffer[1]+","+pBuffer[2]+","+pBuffer[3]+","+pBuffer[4]);
//        uint8_t testread[5];
//        read(testread, address, 5);
//        Serial.println(String("READ ")+testread[0]+","+testread[1]+","+testread[2]+","+testread[3]+","+testread[4]);
        return TRUE;
#endif
    }
    
    void LLFlashUtil::read(uint8_t* pBuffer, uint32_t address, uint32_t len) {
        
#ifndef PLATFORM_PHOTON
        sFLASH_ReadBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
#else
        for (int i=0; i<len; i++)
            pBuffer[i] = *(__IO uint8_t*) (FLASH_USER_MEMORY_OFFSET + address + i);
#endif
    }
}
