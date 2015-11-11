/* 
 * File:   LLFlashInputStream.cpp
 * Author: thorsten
 *
 * Created on November 5, 2014, 12:29 PM
 */

#include "LLFlashInputStream.h"
#include "LLInputStream.h"
#include "LLFlashUtil.h"

namespace com_myfridget
{
    LLFlashInputStream::LLFlashInputStream(uint32_t startAddress)
        : address(startAddress) {
    }

    int LLFlashInputStream::read(unsigned char* b, int l) {
        LLFlashUtil::read(b, address, l);
        address += l;
        return l;
    }
    
    unsigned char LLFlashInputStream::read() {
        return 0; // XXX NOT IMPLEMENTED
    }
    
    void LLFlashInputStream::close() {
    }
    
    bool LLFlashInputStream::eos() {
        return false; // XXX no end of flash memory
    }
}

