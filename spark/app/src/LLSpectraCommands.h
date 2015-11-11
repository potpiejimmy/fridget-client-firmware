/* 
 * File:   LLSpectraCommands.h
 * Author: wolfram
 *
 * Created on October 29, 2014
 */

#ifndef _com_myfridget_LLSPECTRACOMMANDS_H_
#define	_com_myfridget_LLSPECTRACOMMANDS_H_

//uses the application.h
//addtionally we reuse here the global _buf variable and _BUF_SIZE defined in application.cpp (not really nice, I know. Come on, it's firmware...)
#include "application.h"
#include "LLInputStream.h"

// Shows the image on spectra display that starts at the given address of external flash memory.
// I assume that starting from this address 30KB data is available describing the image.
// The format is: 120.000 bits first (=15KB) describing the red color. 0=white, 1=red.
// 300 lines with 400bits(50Bytes) each, one line after the other.
// The next 120.000 bits (15KB) describe the black color. 0=white, 1=black. Some ordering.
// See spec ApplicationNote_EPD441_Spectra_v01.pdf in common/docs/Specifications
namespace com_myfridget
{
    void ShowImage(LLInputStream* in, uint8_t step, uint8_t count);
	float ReadBatteryVoltage();
}

#endif	/* _com_myfridget_LLSPECTRACOMMANDS_H_ */
