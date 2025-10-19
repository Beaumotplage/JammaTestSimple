#pragma once

#include <stdint.h>
#include "App_videomodes.h"
#include "App_polygons_fpu.h"
#include "App_simple.h"
#include "app_global_utils.h"
#include "app_inputs.h"
#include "app_audio.h"

/*
	Main video handler routines (just intended to have one of these classes)
*/
class AppMain
{
public:


	typedef enum
	{
		MODE_SIMPLE,
		MODE_3D_FPU,
		MODE_NUM
	}mode_t;




	void frame_interrupt(void); // 60Hz

	void subcpuloop(void);  // CPU2 - helper core
	
	union
	{

		/*
			Double framebuffer for polygons
			No linebuffer/15kHz mode.
		*/
		struct
		{
			uint16_t framebuffers[2][TV_WIDTH * TV_HEIGHT];
			polygons_fp::polygonbuff_t polygon_fp_ram;
			simpledemo::simplebuff_t simpledemo_ram;
		}framebuffered;

	}m_scratchram;

	uint16_t* m_framebufferpointer = &m_scratchram.framebuffered.framebuffers[0][0];
	bool m_doublebuffer_frame = 0;

	int m_doublebuffer_line = 0;
	

	AppInputs inputs;
	AppAudio m_audio;

	AppMain()
	{
		// layerReset();
	}


private:

	
	/* Instances of testcard classes 
	*  They all use a video_modes template class and require at least an address to scratchram 1:1 and 4:3 memory buffers
	*  Some modes use a 1:1, square display buffer to allow rotating for horizontal and vertical.
	*/
	simpledemo simpledemo_page{ &m_scratchram.framebuffered.simpledemo_ram};
	polygons_fp polygon_fp_page{ &m_scratchram.framebuffered.polygon_fp_ram };

	/* Array of page pointers*/
	// must match MODE enum order above!!!!!!!!!
	AppVideoModes* const m_pages_list[MODE_NUM] = {
							& simpledemo_page,
							& polygon_fp_page
							};



	uint16_t m_scanline = 0;
	mode_t m_mode = MODE_SIMPLE;
	uint16_t testbuttoncounter = 0;
	bool m_init = 1;
	bool m_ready = 0;
};



