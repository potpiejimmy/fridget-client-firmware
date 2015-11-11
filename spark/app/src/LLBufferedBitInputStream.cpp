/* 
 * File:   LLBufferedBitInputStream.cpp
 * Author: thorsten
 *
 * Created on November 5, 2014, 12:29 PM
 */

#include "LLBufferedBitInputStream.h"
#include "LLInputStream.h"

namespace com_myfridget
{
    LLBufferedBitInputStream::LLBufferedBitInputStream(LLInputStream* in, unsigned char* buf, int len)
        : in(in), buf(buf), len(len), pos(0), bitPos(0), availPos(0) {
        
    }

    int LLBufferedBitInputStream::read(unsigned char* b, int l) {
        return -1; // not implemented, use read() or readBit() instead
    }
    
    unsigned char LLBufferedBitInputStream::read() {
        if (empty()) {
            if (!fillBuffer()) return -1; // XXX WARNING reading beyond EOF
        }
        bitPos = 0; // if reading the whole byte, reset bit position
        return buf[pos++];
    }
    
    void LLBufferedBitInputStream::close() {
        in->close();
    }
    
    bool LLBufferedBitInputStream::eos() {
        return in->eos() && empty();
    }

    bool LLBufferedBitInputStream::readBit() {
        if (empty()) {
            if (!fillBuffer()) return false; // XXX WARNING reading beyond EOF
        }
        bool bit = (buf[pos] & (1<<(7-bitPos))) > 0;
        bitPos++;
        if (bitPos==8) {
            bitPos = 0;
            pos++;
        }
        return bit;
    }
    
    bool LLBufferedBitInputStream::empty() {
        return pos>=availPos;
    }
    
    bool LLBufferedBitInputStream::fillBuffer() {
        int read = in->read(buf, len);
        if (read < 0) return false;
        availPos = read;
        pos = 0;
        bitPos = 0;
        return true;
    }
}

