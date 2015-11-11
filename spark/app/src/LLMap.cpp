/* 
 * File:   LLMap.cpp
 * Author: thorsten
 *
 * Created on November 14, 2014, 11:34 PM
 */

#include "LLMap.h"

#include <string.h>

namespace com_myfridget
{
    LLMap::LLMap(int capacity) :
        size(0), capacity(capacity), entries(new char*[capacity]) {
    }
    LLMap::~LLMap() {
        delete entries;
    }
    
    void LLMap::parse(char* input) {
        size = 0;
        char* begin = input;
        while (*input && size < capacity) {
            if (*input==((size%2)?';':'=')) {
                *input = 0;
                entries[size++] = begin;
                begin = input+1;
            }
            input++;
        }
    }
    
    const char* LLMap::getValue(const char* key) {
        for (int i=0; i<size; i+=2) {
            if (!strcmp(key, entries[i])) return entries[i+1];
        }
        return 0;
    }
}
