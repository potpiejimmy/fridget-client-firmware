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
        : in(in), compression(-1), currentBit(0), currentLen(0), nibble(0), haveNibble(false), readBitPos(0) {
        
    }

    int LLRLEInputStream::read(unsigned char* b, int l) {
        int count;
        for (count=0; count<l && !in->eos(); count++) b[count] = read();
        return count;
    }
    
    unsigned char LLRLEInputStream::read() {
        if (compression==-1) compression = readNibble();
        if (!compression) return in->read();
        if (currentLen>=8) { // speed optimization
            currentLen -= 8;
            return currentBit ? 0xFF : 0x00;
        } else {
            unsigned char result = 0;
            for (int i=0; i<8; i++) {
                while (!currentLen) decodeNext();
                result |= (currentBit<<(7-i));
                currentLen--;
            }
            return result;
        }
    }
    
    unsigned char LLRLEInputStream::readNibble() {
        if (haveNibble) {
            haveNibble = false;
            return nibble&0x0f;
        } else {
            haveNibble = true;
            nibble = in->read();
            return (nibble&0xf0)>>4;
        }
    }
    
    void LLRLEInputStream::decodeNext() {
        unsigned int rle = readNibble();
        unsigned char nextBit = (rle & 0x08) >> 3;
        if (nextBit != currentBit) {
            // bit changed, reset bit pos
            readBitPos = 0;
        }
        currentBit = nextBit;
        currentLen = (rle & 0x07) << readBitPos;
        if (!readBitPos) currentLen++; // encoded is LEN - 1
        readBitPos += 3;
    }
    
    void LLRLEInputStream::close() {
        in->close();
    }
    
    bool LLRLEInputStream::eos() {
        return in->eos();
    }
}

