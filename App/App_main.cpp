/* 
    Main Application 
   Calls the selected testcard/demo 
*/

#include <stdint.h>
#include "App_inputs.h" 
#include "App_main.h"
#include "app_global_utils.h"
#ifndef PC
#include "jammatest.h" // hack
#else
extern void HAL_video_setLinebuffer(void);
extern void HAL_video_setFramebuffer(void);

#endif

/**********************************************
 Frame Interrupt
 Scan the inputs and then call the testcard page
 */
void AppMain::frame_interrupt(void)
{
	// time to change screen usually (frames)
	constexpr uint16_t BUTTONTIME_FAST{ 2u };
	constexpr uint16_t BUTTONTIME_SLOW{ 120u }; /* 2seconds*/


	uint32_t inputword = inputs.get_inputs();

	// Change pages on Service button
	if ((inputword & (1u << AppInputs::B_SERVICE)) == 0)
	{
		testbuttoncounter++;

		if(testbuttoncounter == BUTTONTIME_FAST)
		{
	
			m_mode = (mode_t)(((uint8_t)m_mode) + 1);

			if (m_mode == MODE_NUM)
				m_mode = (mode_t)0;

			m_ready = 0;
			m_init = 1;
			

		}

	}
	else
	{
		testbuttoncounter = 0;
	}


	m_doublebuffer_frame ^= 0x1;
	
	m_framebufferpointer = &m_scratchram.framebuffered.framebuffers[m_doublebuffer_frame][0];
	
	m_pages_list[m_mode]->run60Hz(m_init, &m_audio.m_soundcodes, m_framebufferpointer);
	
	if (m_init)
	{	
		m_ready = 1;
		m_init = 0;
	}

}



void AppMain::subcpuloop()
{
	while (1)
	{
		m_pages_list[m_mode]->runcpu2(m_init, m_framebufferpointer);
	}

}