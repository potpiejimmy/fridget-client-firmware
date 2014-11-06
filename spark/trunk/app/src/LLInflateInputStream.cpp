/* 
 * File:   LLInflateInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 4:10 PM
 */

#include "LLInflateInputStream.h"

#include <cstdio>
#include "LLBufferedByteInputStream.h"
#include "LLInputStream.h"

namespace com_myfridget
{

    LLInflateInputStream::LLInflateInputStream(LLBufferedByteInputStream* in) 
    : in(in), dictionary(NULL), dictionarySize(0) {
    }
    
    LLInflateInputStream::~LLInflateInputStream() {
        if (dictionary) delete dictionary;
    }

    int LLInflateInputStream::read(unsigned char* b, int len) {
        if (!dictionary) buildDictionary();
        return 0;
    }
    
    void LLInflateInputStream::buildDictionary() {
        // first byte in stream must be dictionary size:
        unsigned char size = in->readByte();
        printf("DICTIONARY SIZE %d\n", size);
        dictionary = new DictionaryEntry[size];
        dictionarySize = size;
        int count = 0;
        int index = 0;
        unsigned int code = 1;
        while (count < size) {
            code <<= 1;
            int c = in->readByte(); // number of symbols of current len
            count += c;
            for (int i=0; i<c; i++) {
                dictionary[index++].code = code;
                code++;
            }
        }
        // now read the symbols:
        for (int i=0; i<size; i++) {
            dictionary[i].symbol = in->readByte();
        }
        printDictionary();
    }

    static char* binary_fmt(unsigned int x, char* pbuf, int bufsize)
    {
        char *s = pbuf+bufsize; *--s=0;
        do {*--s='0'+x%2; x>>=1;} while (x);
        return s;
    }
    
    void LLInflateInputStream::printDictionary() {
        char buf[32];
        for (int i=0; i<dictionarySize; i++) {
            printf("%u\t%s\n", dictionary[i].symbol, binary_fmt(dictionary[i].code, buf, 32)+1);
        }
    }
    
    void LLInflateInputStream::close() {
        
    }
    
    bool LLInflateInputStream::eos() {
        return in->eos();
    }
}
