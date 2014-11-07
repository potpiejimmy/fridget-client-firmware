/* 
 * File:   LLRLEInputStream.cpp
 * Author: thorsten
 *
 * Created on November 5, 2014, 12:29 PM
 */

#include "LLRLEInputStream.h"
#include "LLInputStream.h"

namespace com_myfridget
{
    LLRLEInputStream::LLRLEInputStream(LLInputStream* in)
        : in(in), currentBit(0), currentLen(0) {
        
    }

    int LLRLEInputStream::read(unsigned char* b, int l) {
        int count;
        for (count=0; count<l && !in->eos(); count++) b[count] = read();
        return count;
    }
    
    unsigned char LLRLEInputStream::read() {
        if (currentLen>=8) { // speed optimization
            currentLen -= 8;
            return currentBit ? 0xFF : 0x00;
        } else {
            unsigned char result = 0;
            for (int i=0; i<8; i++) {
                if (!currentLen) decodeNext();
                result |= (currentBit<<(7-i));
                currentLen--;
            }
            return result;
        }
    }
    
    void LLRLEInputStream::decodeNext() {
        unsigned int rle = in->read();
        currentBit = rle & 0x80 ? 1 : 0;
        currentLen = rle & 0x3f;
        int bitPos = 6;
        bool moreLen = rle & 0x40;
        while (moreLen) {
            rle = in->read();
            moreLen = rle & 0x80;
            currentLen |= (rle & 0x7f)<<bitPos;
            bitPos += 7;
        }
        currentLen++; // encoded is LEN - 1
    }
    
    void LLRLEInputStream::close() {
        in->close();
    }
    
    bool LLRLEInputStream::eos() {
        return in->eos();
    }
}

