#include "application.h"

// Activate the display by setting the TC_EN signal
// assuming the TC_EN entry of the TCon module is 
// connected to D3 of spark
// this method will also set D3 to ouput mode
// Call this method before any other command below
// D4 will be used for TC_CS entry of TCon
// D4 will be set to HIGH initially to make sure that TC_CS is only
// made active when a command is send to TCon
void ActivateDisplay();

void DeactivateDisplay();

// clears the spectra display to white screen
void ClearDisplay();

// UpdateDisplay
int RefreshDisplay();

// shows the image on the spectra display
// expected argument is the image as byte array
// this method is specific to 4.44" display with 400x300 pixels
// each pixel color is encoded with two bits
// 00 = black
// 11 = white
// 01 = red
// 10 should not be used, will be translated to black
// this method expects the byte array to be of at least 400x300x2 bit = 30.000 Byte length
// succeeding bytes will be ignored
void ShowImageOnDisplay(byte* image);


