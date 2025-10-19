#pragma once


#include <stdint.h>
#include "app_inputs.h"
#include "app_audio.h"
#include "App_global_utils.h"


/* Template class for testcards, video demo etc */
class AppVideoModes
{
public:

	

	AppVideoModes(uint16_t* framecanvas) :
		/* Point the buffers to the right place*/
		m_framecanvas{ framecanvas}
	{};

	uint16_t* m_framecanvas; // For testcards that draw a picture at 60Hz, but have 15kHz interrupt to put it into the video


	virtual void run60Hz(bool init, AppAudio::SoundCodes* audio, uint16_t* null) = 0;
	
	virtual void runcpu2(bool init, uint16_t* null)
	{

	}


	AppInputs m_inputs;

	bool m_allow_rotate = 0;

	uint32_t m_dma_lanedata[16]; // 32-bit location to use when DMA is filling locations with one value


	void drawString(short x, short y, char* c, uint8_t string_length, uint16_t colour);
	void drawChar(short x, short y, char c, uint16_t colour);
	void solidcolour(uint16_t* target, uint16_t colourcode, uint32_t size);

	void packRGB332_extend16(uint8_t red, uint8_t green, uint8_t blue, uint16_t* colourcode);

	void todecimal_tuth(int input, char* outputs);
	void wordtohex(uint32_t input, char* outputs);

private:
	int m_screenangle = 0;
	int m_target_screenangle = 0;

};


