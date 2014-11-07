/* 
 * File:   main.cpp
 * Author: thorsten
 *
 * Created on November 4, 2014, 10:45 PM
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "FileInputStream.h"
#include "LLBufferedBitInputStream.h"
#include "LLInflateInputStream.h"
#include "LLRLEInputStream.h"

using namespace std;
using namespace com_myfridget;
/*
 * 
 */
int main(int argc, char** argv) {

    cout << "Hello world.";
    
    unsigned char buf[1024];
    unsigned char outbuf[4096];
    const int epdsize = 30000;
    
    FileInputStream fis("/Users/thorsten/Downloads/download");
    FILE* outfile = fopen("/Users/thorsten/Downloads/output", "wb");
    
    LLBufferedBitInputStream bis(&fis, buf, 1024);
    LLInflateInputStream iis(&bis);
    LLRLEInputStream rle(&iis);

    int processed = 0;
    while (processed < epdsize) {
        int need = (epdsize - processed);
        if (need > 4096) need = 4096;
        int len = rle.read(outbuf, need);
        processed += len;
        fwrite(outbuf, 1, len, outfile);
    }
    fclose(outfile);
    
    printf("\n\n");
    
    return 0;
}

