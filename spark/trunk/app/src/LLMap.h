/* 
 * File:   LLMap.h
 * Author: thorsten
 *
 * Created on November 14, 2014, 11:34 PM
 */

#ifndef _com_myfridget_LLMAP_H
#define	_com_myfridget_LLMAP_H

namespace com_myfridget
{
    /**
     * Value map backed by the given input string.
     * The input value must be in the form "key1=value1;key2=value2;"
     */
    class LLMap
    {
    public:
        LLMap(int capacity, char* input);
        virtual ~LLMap();
        
        virtual const char* getValue(const char* key);
        
    private:
        int size;
        char** entries;
    };
}

#endif	/* _com_myfridget_LLMAP_H */

