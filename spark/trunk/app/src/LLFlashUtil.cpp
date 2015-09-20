/* 
 * File:   LLFlashUtil.cpp
 * Author: thorsten
 *
 */
#include "LLFlashUtil.h"

#include "application.h"
#ifdef PLATFORM_PHOTON
#include "LLsst25vf_spi.h"
#else
#include "sst25vf_spi.h"
#endif

#ifdef PLATFORM_PHOTON
#define FLASH_USER_MEMORY_OFFSET 0
#else
#define FLASH_USER_MEMORY_OFFSET 0x80000
#endif

namespace com_myfridget
{
    void LLFlashUtil::init() {
#ifdef PLATFORM_PHOTON
        sFLASH_Init();
#endif
    }
    
    void LLFlashUtil::stop() {
#ifdef PLATFORM_PHOTON
        sFLASH_DeInit();
#endif
    }
    
    bool LLFlashUtil::flash(const uint8_t *pBuffer, uint32_t address, uint32_t len) {

        // XXX WARNING: with the current implementation, len should be == sFLASH_PAGESIZE
        // and the address must be a multiple of sFLASH_PAGESIZE
        
        const uint8_t *writeBuffer = pBuffer;
        uint8_t readBuffer[len];

        /* Erase the current SPI flash page */
#ifdef PLATFORM_PHOTON
        int eraseSectorSize = 0x8000; // 32K sector erase on Photon
#else
        int eraseSectorSize = 0x1000; // 4K sector erase on Core
#endif
        if (((FLASH_USER_MEMORY_OFFSET + address) % eraseSectorSize) == 0)
            sFLASH_EraseSector(FLASH_USER_MEMORY_OFFSET + address);
//        Serial.println("Erased sector");

        /* write */
        sFLASH_WriteBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
//        Serial.println("Wrote sector");
        sFLASH_ReadBuffer(readBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
//        Serial.println("Read sector");
        
//        Serial.println(String("FLASHED ")+pBuffer[0]+","+pBuffer[1]+","+pBuffer[2]+","+pBuffer[3]+","+pBuffer[4]);
//        Serial.println(String("READ ")+readBuffer[0]+","+readBuffer[1]+","+readBuffer[2]+","+readBuffer[3]+","+readBuffer[4]);

        return memcmp(writeBuffer, readBuffer, len) == 0;
    }
    
    void LLFlashUtil::read(uint8_t* pBuffer, uint32_t address, uint32_t len) {
        
        sFLASH_ReadBuffer(pBuffer, FLASH_USER_MEMORY_OFFSET + address, len);
    }
}
