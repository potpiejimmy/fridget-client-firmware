/* 
 * File:   LLMemoryInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 4:10 PM
 */

#include "LLMemoryInputStream.h"
#include "application.h"

namespace com_myfridget
{

    LLMemoryInputStream::LLMemoryInputStream(unsigned char* memptr) 
    : memptr(memptr) {
    }
    
    LLMemoryInputStream::~LLMemoryInputStream() {
    }

    int LLMemoryInputStream::read(unsigned char* b, int len) {
        memcpy(b, memptr, len);
        memptr += len;
        return len;
    }
    
    unsigned char LLMemoryInputStream::read() {
        return *memptr++;
    }
    
    void LLMemoryInputStream::close() {
        
    }
    
    bool LLMemoryInputStream::eos() {
        return false;
    }
}
