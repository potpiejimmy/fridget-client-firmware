/* 
 * File:   LLInflateInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 4:10 PM
 */

#include "LLInflateInputStream.h"

namespace com_myfridget
{

    LLInflateInputStream::LLInflateInputStream(LLBufferedBitInputStream* in) 
    : in(in), dictionary(0), dictionarySize(0) {
    }
    
    LLInflateInputStream::~LLInflateInputStream() {
        if (dictionary) delete dictionary;
    }

    int LLInflateInputStream::read(unsigned char* b, int len) {
        int count;
        for (count=0; count<len && !in->eos(); count++) b[count] = read();
        return count;
    }
    
    unsigned char LLInflateInputStream::read() {
        if (!dictionary) buildDictionary();
		return inflateNextByte();
    }
    
    void LLInflateInputStream::buildDictionary() {
        // first byte in stream must be dictionary size:
        dictionarySize = in->read();
        if (!dictionarySize) dictionarySize = 0x100; // encoded 0x00 is 256
        //printf("DICTIONARY SIZE %d\n", dictionarySize);
        dictionary = new unsigned char[dictionarySize];
         
		 // read the number of symbols per length
        int count = 0;
        int index = 0;
        while (count < dictionarySize) {
            prefixLengths[index] = in->read();
            count += prefixLengths[index];
			index++;
        }
        // now read the symbols:
        for (int i=0; i<dictionarySize; i++) {
            dictionary[i] = in->read();
        }
    }
	
	unsigned char LLInflateInputStream::inflateNextByte() {	
		unsigned int read = 1; // holds current bits read from stream
		unsigned int code = 1; // holds huffman code to compare with
		int symbolPos = 0; // current index of symbol in dictionary
		for (int i=0; i<com_myfridget_LLINFLATEINPUTSTREAM_MAX_PREFIX_LEN; i++) {
			code <<= 1; read<<=1;
			read |= in->readBit() ? 1 : 0;
			int noOfSymbols = prefixLengths[i];
			for (int j=0; j<noOfSymbols; j++) {
			    // compare against all symbols of that length:
				if (read == code) return dictionary[symbolPos]; // found
				code++; symbolPos++;
			}
		}
		return 0; // must never happen
	}

//    static char* binary_fmt(unsigned int x, char* pbuf, int bufsize)
//    {
//        char *s = pbuf+bufsize; *--s=0;
//        do {*--s='0'+x%2; x>>=1;} while (x);
//        return s;
//    }
    
//    void LLInflateInputStream::printDictionary() {
//        char buf[32];
//        for (int i=0; i<dictionarySize; i++) {
//            printf("%u\t%s\n", dictionary[i].symbol, binary_fmt(dictionary[i].code, buf, 32)+1);
//        }
//    }
    
    void LLInflateInputStream::close() {
        
    }
    
    bool LLInflateInputStream::eos() {
        return in->eos();
    }
}
