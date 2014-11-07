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
    LLRLEInputStream::LLRLEInputStream(LLInputStream* in, unsigned char* buf, int len)
        : in(in) {
        
    }

    int LLRLEInputStream::read(unsigned char* b, int l) {
	    // XXX TODO
        return -1; 
    }
    
    unsigned char LLRLEInputStream::read() {
	    // XXX TODO
        return -1; 
    }
    
    void LLRLEInputStream::close() {
        in->close();
    }
    
    bool LLRLEInputStream::eos() {
        return in->eos();
    }
}

