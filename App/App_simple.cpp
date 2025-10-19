/*  Simple framebuffer demo page to build upon
	
*/

#include <stdlib.h>
#include "pc_or_rp2040.h"

#include "app_global_utils.h"
#include "app_simple.h"


#ifdef PC
#include "..\pc_simulator\oleddisplaysim\pc_peripheral_stubs.h"

// Dirty extern for rather inefficient character system 




#else
extern "C" {
#include "hardware/interp.h"

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/divider.h"
#include "hardware/interp.h"
#include "hardware/dma.h"
#include "..\hw_defs.h"
}
#endif


// Hacky, but sprite and fonts declared here or externed and then shoved in a namespace to get around
// vscode build errors
namespace assets
{
	// SPRITE in ROM
const uint16_t ship[16 * 16] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,30753,30753,30753,30753,30753,30753,30753,30753,0,0,0,0,0,0,
0,0,30753,30753,18546,18546,18546,18546,18546,18546,30753,30753,0,0,0,0,
0,0,0,0,30753,30753,18546,18546,18546,18546,18546,18546,30753,0,0,0,
0,0,0,0,0,0,30753,30753,30753,18546,18546,18546,30753,30753,0,0,
0,0,0,0,5458,5458,5458,5458,5458,30753,18546,5458,5458,30753,0,0,
0,0,0,5458,164,164,5458,164,5458,30753,18546,164,5458,30753,0,0,
0,0,0,164,246,246,164,246,5458,30753,18546,164,5458,30753,0,0,
0,0,0,164,246,246,164,246,5458,30753,18546,164,5458,30753,0,0,
0,0,0,5458,164,164,5458,164,5458,30753,18546,164,5458,30753,0,0,
0,0,0,0,5458,5458,5458,5458,5458,30753,18546,5458,5458,30753,0,0,
0,0,0,0,0,0,30753,30753,30753,18546,18546,18546,30753,30753,0,0,
0,0,0,0,30753,30753,18546,18546,18546,18546,18546,18546,30753,0,0,0,
0,0,30753,30753,18546,18546,18546,18546,18546,18546,30753,30753,0,0,0,0,
0,0,30753,30753,30753,30753,30753,30753,30753,30753,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, };

#include "glcdfont.cpp"
//extern unsigned char font[];
}

/*
	60Hz interrupt
	Wipe and redraw the screen here
*/

void __no_inline_not_in_flash_func(simpledemo::run60Hz)(bool init, AppAudio::SoundCodes* audio,  uint16_t* framebuffer)
{
	
	if (init)
	{
		m_allow_rotate = 0;

		audio->play = true;
		audio->reset = false;
		audio->volume = 16386;


		// Copy ROM const into RAM for big speedup
		for (int x = 0; x < (16 * 16); x++)
		{
			m_scratch->spaceship[x] = assets::ship[x];
		}

	}


	// Fill the background blue using 16-bit DMA 
	// 32-bit dma would be a bit quicker, but we'll keep it simple 
	// (and 32-bit will only align to even pixel addresses)
	dma_fill16(dma_gfx_0, &framebuffer[0], BLUE, TV_WIDTH * (TV_HEIGHT) );


#ifndef PC
	// make sure DMA is done before drawing stuff!
	// Maybe do some non-GFX code while it's blanking stuff
	while ((dma_hw->ch[dma_gfx_0].ctrl_trig & DMA_CH0_CTRL_TRIG_BUSY_BITS))
	{
	}
#endif

	uint16_t pencolour;
	// Note: Not super-efficient to run this every time you want a colour, best to 
	// do it and save your favourites to variables
	app_global_utils::packRGB332_extend16(255, 255, 255, &pencolour);


	// Draw some text (in an inefficient manner)
	char titlearray[] = "Insert Game Here";
	int x = 100;
	int y = 80;
	drawString(framebuffer, x, y, &titlearray[0], sizeof(titlearray), pencolour);

	
	// Draw a line
	y = 100;

	for (int a = 50; a < 250; a++)
	{
		framebuffer[a + (y * TV_WIDTH) ] = pencolour;
	}

	//Draw a sprite
	x = 140;
	y = 120;

	//Keypress demo move sprite a bit
	// Get inputs
	uint32_t inputword = m_inputs.get_inputs();
	if ((inputword & 1 << AppInputs::B_1P_1) == 0) // P1 button 1 pressed
	{
		x = 180;
	}

	for (int a = 0; a < 16; a++)
	{
		for (int b = 0; b < 16; b++)
		{
			framebuffer[x + a + ((y + b) * TV_WIDTH)] = m_scratch->spaceship[a + b*16];
		}
	}


		
}


void simpledemo::cpu2_helper_loop(bool init, uint16_t* framebuffer)
{
	// You can use the second CPU here
	// I'll let you worry about syncing stuff
}



// Slow text system. I wouldn't call this in a real game as they're from packed bitfields
void simpledemo::drawString(uint16_t* bufferptr, short x, short y, char* c, uint8_t string_length, uint16_t colour)
{
	uint8_t i, j;

	for (uint16_t l = 0; l < string_length; l++)
	{
		for (i = 0; i < 6; i++)
		{
			unsigned char line;
			if (i == 5)
				line = 0x0;
			else
			{
				uint16_t index = c[l];
				index *= 5;
				index += i;
				line = assets::font[index];
			}
			for (j = 0; j < 8; j++) {
				if (line & 0x1 << j) {
					bufferptr[l * 6 + x + i + ((y + j) * TV_WIDTH)] = colour;
				}
				else
				{
					// Remove this if you want transparent
					//bufferptr[l * 6 + x + i + ((y + j) * TV_WIDTH)] = BLACK;
				}
				/*	else if (bg != color) {
							drawPixel(x + i, y + j, bg);
						}*/
			}
			line >>= 1;
		}
	}
}





inline void simpledemo::dma_fill16(lane_t lane, uint16_t* dst, uint16_t val, uint32_t length)
{


#ifdef PC
	for (uint32_t x = 0; x < length; x++)
	{
		*dst = val;
		dst++;
	}

#else
	while ((dma_hw->ch[lane].ctrl_trig & DMA_CH0_CTRL_TRIG_BUSY_BITS))
	{
	}

	m_dma_lanedata[lane] = val;
	dma_channel_config c0 = dma_channel_get_default_config(lane);  // default configs

	channel_config_set_transfer_data_size(&c0, DMA_SIZE_16);              // 16-bit txfers
	channel_config_set_read_increment(&c0, false);                        // no read incrementing
	channel_config_set_write_increment(&c0, true);                      // write incrementing
//	channel_config_set_chain_to(&c0, lane);                         // chain to other channel
	dma_channel_configure(
		lane,                 // Channel to be configured
		&c0,                        // The configuration we just created
		dst,          // write address 
		&m_dma_lanedata[lane],            // Eead address
		length,                    // Number of transfers
		true                       // start immediately.
	);

#endif

}



