#pragma once
#include <stdint.h>

#define TV_WIDTH 320
#define TV_HEIGHT 224
#define PIXEL_COUNT (TV_WIDTH * TV_HEIGHT)

#define TV_SQUARE 256


// Square canvas of screen height with sqrt2 extra in each dimention for rotation.
//46
#define SQUARE_OFFSET (const int)(0.5 + 224.0/2*0.41)
//316
//#define SQUARE_LINE (const int)(0.5 + 224.0*1.41)

//100k (same as (224 * 224 * 2))
//#define SQUARE_CANVAS (SQUARE_LINE * SQUARE_LINE)
#define SQUARE_CANVAS (TV_SQUARE * TV_SQUARE * 2)


//#define SQUARE_START ((SQUARE_LINE * SQUARE_OFFSET) + SQUARE_OFFSET)

#define SQUARE_START (TV_SQUARE * TV_SQUARE/2)

/*
	XXXXXXXX
	xx    xx
	xx    xx
	XXXXXXXX

*/




// Pixel color array that is DMA's to the PIO machines and
// a pointer to the ADDRESS of this color array.
// Note that this array is automatically initialized to all 0's (black)
#define DRAWSTART 64


// Video timing constants
#define V_ACTIVE (TV_HEIGHT - 1)    // (active - 1)

#define RGB_ACTIVE (TV_WIDTH -1)    // (horizontal active)/2 - 1

// 0 is transparency, so instead use 16th bit for black (doesn't go to screen, but ensures hardware plots a pixel)
#define BLACK (0x8000)
#define WHITE (0xFFFF)
#define CLEAR (0x0000)

// Conventional 8-bit colour in 8 LSB  
// Upper 8 MSB extends it to 15-bit 
//BLACK, B5, B4, B3, G4, G3, R4, R3,    B7, B6, G7, G6, G5, R7, R6, R5	
#define RED (0b0000001100000111)
#define GREEN (0b0000110000111000)
#define BLUE (0b011100011000000)


namespace app_global_utils
{
	extern void packRGB332_extend16(uint8_t red, uint8_t green, uint8_t blue, uint16_t* colourcode);
	extern void unpackRGB555_dualmodes(uint16_t colourcode, uint8_t* red, uint8_t* green, uint8_t* blue);
	extern void packRGB332(uint8_t red, uint8_t green, uint8_t blue, uint8_t* colourcode);


};