/* 
 * File:   LLBufferedByteInputStream.cpp
 * Author: thorsten
 *
 * Created on November 5, 2014, 12:29 PM
 */

#include "LLBufferedByteInputStream.h"
#include "LLInputStream.h"

namespace com_myfridget
{
    LLBufferedByteInputStream::LLBufferedByteInputStream(LLInputStream* in, unsigned char* buf, int len)
        : in(in), buf(buf), len(len), pos(0), bitPos(0), availPos(0) {
        
    }

    int LLBufferedByteInputStream::read(unsigned char* b, int l) {
        return -1; // not implemented, use readByte or readBit instead
    }
    
    void LLBufferedByteInputStream::close() {
        in->close();
    }
    
    bool LLBufferedByteInputStream::eos() {
        return in->eos() && empty();
    }

    unsigned char LLBufferedByteInputStream::readByte() {
        if (empty()) {
            if (!fillBuffer()) return -1; // XXX WARNING reading beyond EOF
        }
        bitPos = 0; // if reading the whole byte, reset bit position
        return buf[pos++];
    }
    
    bool LLBufferedByteInputStream::readBit() {
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
    
    bool LLBufferedByteInputStream::empty() {
        return pos>=availPos;
    }
    
    bool LLBufferedByteInputStream::fillBuffer() {
        int read = in->read(buf, len);
        if (read < 0) return false;
        availPos = read;
        pos = 0;
        bitPos = 0;
        return true;
    }
}

