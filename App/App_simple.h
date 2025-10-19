#pragma once
#include <stdint.h>
#include "pc_or_rp2040.h"
#include "app_global_utils.h"
#include "App_videomodes.h"
#include "App_shapes.h"

#include "..\hw_defs.h"
#ifdef PC


#include "..\pc_simulator\oleddisplaysim\pc_peripheral_stubs.h"

#else


#endif

class simpledemo : public AppVideoModes
{
public:


	typedef struct
	{
		uint16_t start[10]; // leave this here for now, as strange bug on PC otherwise
		uint16_t spaceship[16 * 16];
		int16_t variable1;
		// Put all RAM variables here rather than make them 'static' in the functions
	}simplebuff_t;

	simplebuff_t* m_scratch;
	uint16_t null; // drawing canvas is just used for linebuffer modes 

	simpledemo(simplebuff_t* simpleram) :
		AppVideoModes{&null},
		m_scratch{ simpleram }
	{
	};

	void run60Hz(bool init, AppAudio::SoundCodes* audio, uint16_t* null) override;
	void runcpu2(bool init, uint16_t* framebuffer) override
	{
		cpu2_helper_loop(init, framebuffer);
	};
private:


	inline void dma_fill16(lane_t lane, uint16_t* dst, uint16_t val, uint32_t length);

	void drawString(uint16_t* bufferptr, short x, short y, char* c, uint8_t string_length, uint16_t colour);
	void cpu2_helper_loop(bool init, uint16_t* framebuffer);

	// May be unused, need to check.
	// Better to put variables in the 'scratch' structure above.
	uint16_t m_dma_line_counter;
	uint16_t* m_next_top_layer;
	uint16_t m_clear = 0;
	uint32_t m_dma_trigger = 0;
	bool m_first_pass = 1;

	


#ifdef PC
	interp_t interp0;
#endif
};

