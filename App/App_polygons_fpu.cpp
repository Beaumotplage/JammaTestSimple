/* Demo 'screensaver'
	Really just an excuse for me to play around with 'sprites' and stuff

*/

#include <stdlib.h>
#include "pc_or_rp2040.h"

#include "app_polygons_fpu.h"
#include "app_global_utils.h"
#include "app_trigtables.h"
#include "App_shapes.h"
//#include "polybackground.cpp"


#ifdef CUBE
#define CAMERA_Z 1000
#define SHIFT_IN 3
#endif
#ifdef SONIC
#define CAMERA_Z 4000
#define SHIFT_IN 3
#endif
#ifdef ROBOCOP
#define CAMERA_Z 4000
#define SHIFT_IN 3
#endif
#ifdef LANDSCAPE
#define CAMERA_Z 0
#define SHIFT_IN 2
#endif

#ifdef PC
#include "..\pc_simulator\oleddisplaysim\pc_peripheral_stubs.h"
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

#define DMA_WAIT 


static uint32_t g_blue = (0b0011100011000000);

static uint32_t assert;

// X,Y
#define X (0)
#define Y (1)
#define Z (2)
#define W (3)

// Normals cull point 
#define NORMALS_CULL (-10)
#define MOVE
//#define DMAPLOT
//#define ROMBACKGROUND



static bool ready = 0;


void __no_inline_not_in_flash_func(polygons_fp::run60Hz)(bool init, AppAudio::SoundCodes* audio, uint16_t* framebuffer)
{
	if (init)
	{
		init_polys(audio, framebuffer);
	}

	if (m_cpu_double_buffer == 0)
	{
		float(*transform)[4] = m_scratch->transform_a;
		int32_t* sum_z = &m_scratch->sum_z_a[0];
		m_sub_cpu_state = 1;

		cull_and_rasterise(framebuffer, transform, sum_z);
		while (m_sub_cpu_state != 0)
		{

		}
		m_cpu_double_buffer = 1;
	}
	else
	{
		float(*transform)[4] = m_scratch->transform_b;
		int32_t* sum_z = &m_scratch->sum_z_b[0];
		m_sub_cpu_state = 2;
		cull_and_rasterise(framebuffer, transform, sum_z);

		while (m_sub_cpu_state != 0)
		{

		}
		m_cpu_double_buffer = 0;
	}
		 
}


void polygons_fp::cpu2_helper_loop(bool init, uint16_t* framebuffer)
{
//	if (!init)
	{

		if (m_sub_cpu_state == 1)
		{
			float(*transform)[4] = m_scratch->transform_b;
			int32_t* sum_z = &m_scratch->sum_z_b[0];

			transform_and_sort(framebuffer, transform, sum_z);
			m_sub_cpu_state = 0;
		}
		else if (m_sub_cpu_state == 2)
		{
			float(*transform)[4] = m_scratch->transform_a;
			int32_t* sum_z = &m_scratch->sum_z_a[0];

			transform_and_sort(framebuffer, transform, sum_z);
			m_sub_cpu_state = 0;
		}
	}
}


void __no_inline_not_in_flash_func(polygons_fp::init_polys)(AppAudio::SoundCodes* audio, uint16_t* framebuffer)
{
	
	m_allow_rotate = 0;
	audio->play = true;
	audio->reset = false;
	audio->volume = 16386;
	m_pos.roll = 0;
	m_pos.pitch = -17694;
	m_pos.yaw = 100;
	m_pos.x = -3308;
	m_pos.y = -7500;
	m_pos.z = -500;


	for (int16_t x = 0; x < VERTICES; x++)
	{
		m_scratch->vertices[x][0] = vertices_rom[x][0];
		m_scratch->vertices[x][1] = vertices_rom[x][1];
		m_scratch->vertices[x][2] = vertices_rom[x][2];
	}

	for (int16_t x = 0; x < POLYGONS; x++)
	{
		m_scratch->polygons[x][0] = indices_rom[x][0];
		m_scratch->polygons[x][1] = indices_rom[x][1];
		m_scratch->polygons[x][2] = indices_rom[x][2];

		int z0 = m_scratch->vertices[m_scratch->polygons[x][0]][Z];
		int z1 = m_scratch->vertices[m_scratch->polygons[x][1]][Z];
		int z2 = m_scratch->vertices[m_scratch->polygons[x][2]][Z];


		// Height based colours (sort of)
		int16_t intensity = (z0 + z1 + z2) >> 3;


		if (intensity < 5)
		{
			// blue sea
			app_global_utils::packRGB332_extend16(0, 0, (intensity + 1) * 30, &m_scratch->colours[x]);

		}
		else if (intensity < 63)
		{
			app_global_utils::packRGB332_extend16(0, intensity + 63, 0, &m_scratch->colours[x]);
		}
		else
		{
			uint16_t white = intensity;
			if (white > 255)
				white = 255;

			app_global_utils::packRGB332_extend16((uint8_t)white, (uint8_t)white, (uint8_t)white, &m_scratch->colours[x]);

		}

	}


	ready = 1;
	//	dma_line_init();
	

}

void __no_inline_not_in_flash_func(polygons_fp::transform_and_sort)( uint16_t* framebuffer, float(*transform)[4], int32_t* sum_z)
{
	

	// Make a list of address pointers for DMA to draw the background from the right point of a panoramic 

#ifdef ROMBACKGROUND
	static int scroll_offset = 0;

	scroll_offset = m_pos.yaw >> 5;

	/*
	if (scroll_offset > skyline_width)
	{
		scroll_offset = 0;
	}
	if (scroll_offset < 0)
	{
		scroll_offset = skyline_width;
	}
	*/

	const uint16_t* romlist[TV_HEIGHT / 2 + 1];

	for (int x = 0; x < TV_HEIGHT / 2; x++)
	{
		romlist[x] = &skyline[scroll_offset + x * skyline_width];

	}
	romlist[TV_HEIGHT / 2] = 0;


#ifdef PC
	uint32_t index = 0;

	uint16_t* ramtgt = (uint16_t*)&framebuffer[0];
	uint16_t* romtgt;// = &skyline[0];
	int y = 0;
	int x_offset = 0;
	int x = 0;
	int z = 0;
	while (romlist[y] != 0)
	{
		for (uint32_t x = 0; x < TV_WIDTH; x++)
		{
			ramtgt[z] = *(romlist[y] + x);
			z++;
		}
		y++;
	}
#else
	/*
		uint32_t index = (((uint32_t)scanline) * space_width);

		uint32_t* romtgt = (uint32_t*)&space[0];
		//    xip_ctrl_hw->stream_addr = romtgt[index + x_offset];

		while (!(xip_ctrl_hw->stat & XIP_STAT_FIFO_EMPTY))
			(void)xip_ctrl_hw->stream_fifo;
		xip_ctrl_hw->stream_addr = (uint32_t)&space[index + x_offset / 2];
		xip_ctrl_hw->stream_ctr = LENGTH;

		dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
		channel_config_set_read_increment(&cfg, false);
		channel_config_set_write_increment(&cfg, true);
		channel_config_set_dreq(&cfg, DREQ_XIP_STREAM);
		dma_channel_configure(
			dma_chan,
			&cfg,
			(void*)(uint32_t)&bottom_layer[0],                 // Write addr
			(const void*)XIP_AUX_BASE,  // Read addr
			LENGTH, // Transfer count
			true                        // Start immediately!
		);
		*/
#endif
#else

//	dma_fill32(dma_gfx_0, &framebuffer[0], GREEN, (TV_WIDTH * TV_HEIGHT) / 2);
#endif

	uint16_t grass;
	app_global_utils::packRGB332_extend16(0, 128, 0, &grass);

	//	dma_fill32(dma_gfx_0, &framebuffer[(TV_WIDTH * TV_HEIGHT) / 2], grass, (TV_WIDTH* TV_HEIGHT) / 4);
#if 0
	if ((m_inputs.get_inputs() & 1 << AppInputs::B_UP1) == 0)
	{
		m_pos.pitch -= 100;
	}
	if ((m_inputs.get_inputs() & 1 << AppInputs::B_DOWN1) == 0)
	{
		m_pos.pitch += 100;
	}
	if ((m_inputs.get_inputs() & 1 << AppInputs::B_LEFT1) == 0)
	{
		m_pos.yaw += 100;
	}
	if ((m_inputs.get_inputs() & 1 << AppInputs::B_RIGHT1) == 0)
	{
		m_pos.yaw -= 100;
	}
#endif
	static bool direction = 0;
#ifdef MOVE
	if (!direction)
	{
		m_pos.y += 4;
		if (m_pos.y > 0)
		{
			direction = 1;
		}
	}
	else
	{
		m_pos.y -= 4;

		if (m_pos.y < -3500)
		{
			direction = 0;
		}
	}
#endif
	/*Load*/
	float fp_scale = 1.0f / 32768.0f;
	float sin_yaw = (float)g_sincos.sine((short)m_pos.yaw) * fp_scale;
	float cos_yaw = (float)g_sincos.cosine((short)m_pos.yaw) * fp_scale;
	
	float sin_pitch = (float)g_sincos.sine((short)m_pos.pitch) * fp_scale;
	float cos_pitch = (float)g_sincos.cosine((short)m_pos.pitch) * fp_scale;

	float sin_roll = (float)g_sincos.sine((short)m_pos.roll) * fp_scale;
	float cos_roll = (float)g_sincos.cosine((short)m_pos.roll) * fp_scale;

	/* Rotation/Positions transforms on all vertices */
	for (int16_t i = 0; i < VERTICES; i++)
	{


		transform[i][0] = (float)(((int32_t)m_scratch->vertices[i][0]) << (SHIFT_IN - 0));
		transform[i][1] = (float)(((int32_t)m_scratch->vertices[i][1]) << (SHIFT_IN - 0));
		transform[i][2] = (float)(((int32_t)m_scratch->vertices[i][2]) << (SHIFT_IN - 1));

		/*	Rotate
		Matrix temp{ 1, 0, 0, 0,
					 0, float(cos(angle)), float(-sin(angle)), 0,
					 0, float(sin(angle)), float(cos(angle)), 0,
					 0, 0, 0, 1 };
		return temp;
		*/

		// Translate first

		transform[i][0] += m_pos.x;// ((transform[i][3] * 5 * 256) >> 8);
		transform[i][1] += m_pos.y;
		transform[i][2] += m_pos.z;// ((transform[i][3] * 5 * 256) >> 8);

		// Then rotations
		//Up/Down

		float temp1;

		// PITCH (up/down)
		temp1 = ((transform[i][1] * cos_pitch) + (transform[i][2] * -sin_pitch));
		transform[i][2] = ((transform[i][1] * sin_pitch) + (transform[i][2] * cos_pitch));
		transform[i][3] = 255.0f;// vertices[i][3];
		transform[i][1] = temp1;

		// ROLL (clock)
		temp1 = ((transform[i][0] * cos_roll) + (transform[i][1] * -sin_roll));
		transform[i][1] = ((transform[i][0] * sin_roll) + (transform[i][1] * cos_roll));
		//	transform[i][3] = 255.0f;// vertices[i][3];
		transform[i][0] = temp1;

		// YAW (left/right)
		temp1 = ((transform[i][0] * cos_yaw) + (transform[i][2] * -sin_yaw));
		transform[i][2] = ((transform[i][0] * sin_yaw) + (transform[i][2] * cos_yaw));

		transform[i][0] = temp1;


		/* Camera transform */
		/*
		1 0 0 0
		0 1 0 0
		0 0 1 50
		0 0 0 1
		*/


	}


	/* Sort the Polygons by depth */

	for (int16_t i = 0; i < POLYGONS; i++)
	{
		float z0 = transform[m_scratch->polygons[i][0]][Z];
		float z1 = transform[m_scratch->polygons[i][1]][Z];
		float z2 = transform[m_scratch->polygons[i][2]][Z];

		// Now do a depth sort before we transform things into 2D

		sum_z[i] = -1;
		if ((z0 > 0) && (z1 > 0) && (z2 > 0))
		{
			sum_z[i] = (int32_t)(z0 + z1 + z2);
		}

		// Crude data packing trick
		sum_z[i] = (sum_z[i] << 16) + i;


	}



	/* Perspective and Screen transforms*/

	for (int16_t i = 0; i < VERTICES; i++)
	{
		//Perspective Transform
		/*
		_perspectiveTransform = {
								 d / AR, 0,		0,		0,
								 0,		d,		0,		0,
								 0,		0,		d,		0,
								 0,		0,		1,		0
		};
		*/


		float inv_aspectRatio = 220.0f / 320.0f; // Aspect Ratio calculation
		transform[i][0] = (transform[i][0] * inv_aspectRatio);
		transform[i][3] = transform[i][2];

		/* Dehomogonize
		_x = _x / _w;
		_y = _y / _w;
		_z = _z / _w;
		_w = _w / _w;
		*/

		float invert = 1.0f;

		if (transform[i][3] != 0)
		{
			invert = invert / transform[i][3];
		}
		//	invert *= SHIFTINP;
		transform[i][0] = transform[i][0] * invert;
		transform[i][1] = transform[i][1] * invert;
		transform[i][2] = transform[i][2] * invert;
		transform[i][3] = 1.0f;

		/*
		#define SCREEN_WIDTH (320)
		#define SCREEN_HEIGHT (240)*/
#define HALF_WIDTH (TV_WIDTH/2)
#define HALF_HEIGHT (TV_HEIGHT/2)
#define HALF (128)		
		/* Screen transform
			w / 2,	0,	0,	w/ 2,
			0,	-h/ 2, 0,  h/2,
			0, 0, d / 2, d / 2,
			0,   0,   0,   1
			*/

		transform[i][0] = HALF_WIDTH * (transform[i][0] + transform[i][3]);


		transform[i][1] = HALF_HEIGHT * (transform[i][3] - transform[i][1]);


	}

	mergeSort(&sum_z[0], 0, POLYGONS - 1);


}

void __no_inline_not_in_flash_func(polygons_fp::cull_and_rasterise)(uint16_t* framebuffer, float(*transform)[4], int32_t* sum_z)
{
	//MOVE ME!!!
		dma_fill32(dma_gfx_0, &framebuffer[0], GREEN, (TV_WIDTH * TV_HEIGHT) / 2);

	/* Cull/Normals */
#ifdef CULLNORMALS
	for (int16_t i = 0; i < POLYGONS; i++)
	{

		//Get the vertices
		int32_t x0 = transform[m_scratch->polygons[i][0]][X];
		int32_t y0 = transform[m_scratch->polygons[i][0]][Y];
		int32_t z0 = transform[m_scratch->polygons[i][0]][Z];

		int32_t x1 = transform[m_scratch->polygons[i][1]][X];
		int32_t y1 = transform[m_scratch->polygons[i][1]][Y];
		int32_t z1 = transform[m_scratch->polygons[i][1]][Z];

		int32_t x2 = transform[m_scratch->polygons[i][2]][X];
		int32_t y2 = transform[m_scratch->polygons[i][2]][Y];
		int32_t z2 = transform[m_scratch->polygons[i][2]][Z];


		int32_t vector_a[3];
		vector_a[0] = x1 - x0;
		vector_a[1] = y1 - y0;
		vector_a[2] = z1 - z0;

		int32_t vector_b[3];
		vector_b[0] = x2 - x0;
		vector_b[1] = y2 - y0;
		vector_b[2] = z2 - z0;

		//Normal (cross product)

		//temp.SetX(_y* other.GetZ() - _z * other.GetY());
		//temp.SetY(_z* other.GetX() - _x * other.GetZ());
		//temp.SetZ(_x* other.GetY() - _y * other.GetX());

		int32_t normal[3];
		normal[X] = ((vector_b[Y] * vector_a[Z]) - (vector_b[Z] * vector_a[Y])) >> 8;
		normal[Y] = ((vector_b[Z] * vector_a[X]) - (vector_b[X] * vector_a[Z])) >> 8;
		normal[Z] = ((vector_b[X] * vector_a[Y]) - (vector_b[Y] * vector_a[X])) >> 8;


		/*
		1 0 0 0
		0 1 0 0
		0 0 1 CAMERA_Z
		0 0 0 1*/
		int32_t eye[3];
		eye[X] = 0 - x0;
		eye[Y] = 0 - y0;
		eye[Z] = -CAMERA_Z - z0 - 10000;

		//Normal dotproduct
		// normal.DotProduct(eyeVector)
		//float temp = _x * other.GetX() + _y * other.GetY() + _z * other.GetZ();
		int32_t dotproduct = ((normal[X] * eye[X]) + (normal[Y] * eye[Y]) + (normal[Z] * eye[Z])) >> 8;

		m_scratch->normals[i] = dotproduct;
	}
#endif

#ifndef PC
	// make sure DMA is done before drawing stuff!
	while ((dma_hw->ch[dma_gfx_0].ctrl_trig & 0x01000000))// &&
	{
	}
#endif

	// Now take visible polygons (normal > 0) order 3 vertical points and split into 2 triangles, then dtaw them
	for (int16_t w = POLYGONS - 1; w >= 0; w--)
	{
		int16_t y_first = 0;
		int16_t y_second = 0;
		int16_t y_third = 0;

		uint16_t v = sum_z[w] & 0xFFFF;
		// Shouldn't be needed

		if (v > POLYGONS - 1)
		{
			v = POLYGONS - 1;
		}

		//As well as culling things that are behind us, we can cull normals facing away
		// However on a flat landscape, it's a waste of maths and memory
#ifdef CULLNORMALS
//		if ((m_scratch->normals[v] > NORMALS_CULL) && (sum_z[w] > (1000 << 16)))
#else
		if (sum_z[w] > (1 << 16))
#endif
		{
			int x0;
			int y0;
			int x1;
			int y1;
			int x2;
			int y2;

			int _a = (int)transform[m_scratch->polygons[v][0]][Y];
			int _b = (int)transform[m_scratch->polygons[v][1]][Y];
			int _c = (int)transform[m_scratch->polygons[v][2]][Y];

			// Order the vertices. Zero is top of the screen (maybe I should change that?!)

			if (_a > _b)
			{
				if (_b > _c)
				{
					//a,b,c
					x0 = (int)transform[m_scratch->polygons[v][0]][X];
					y0 = (int)transform[m_scratch->polygons[v][0]][Y];
					x1 = (int)transform[m_scratch->polygons[v][1]][X];
					y1 = (int)transform[m_scratch->polygons[v][1]][Y];
					x2 = (int)transform[m_scratch->polygons[v][2]][X];
					y2 = (int)transform[m_scratch->polygons[v][2]][Y];
				}
				else
				{
					if (_c > _a)
					{
						//c,a,b
						x0 = (int)transform[m_scratch->polygons[v][2]][X];
						y0 = (int)transform[m_scratch->polygons[v][2]][Y];
						x1 = (int)transform[m_scratch->polygons[v][0]][X];
						y1 = (int)transform[m_scratch->polygons[v][0]][Y];
						x2 = (int)transform[m_scratch->polygons[v][1]][X];
						y2 = (int)transform[m_scratch->polygons[v][1]][Y];
					}
					else
					{
						//a,c,b
						x0 = (int)transform[m_scratch->polygons[v][0]][X];
						y0 = (int)transform[m_scratch->polygons[v][0]][Y];
						x1 = (int)transform[m_scratch->polygons[v][2]][X];
						y1 = (int)transform[m_scratch->polygons[v][2]][Y];
						x2 = (int)transform[m_scratch->polygons[v][1]][X];
						y2 = (int)transform[m_scratch->polygons[v][1]][Y];
					}
				}
			}
			else // (b > a)
			{
				if (_a > _c)
				{
					//b,a,c
					x0 = (int)transform[m_scratch->polygons[v][1]][X];
					y0 = (int)transform[m_scratch->polygons[v][1]][Y];
					x1 = (int)transform[m_scratch->polygons[v][0]][X];
					y1 = (int)transform[m_scratch->polygons[v][0]][Y];
					x2 = (int)transform[m_scratch->polygons[v][2]][X];
					y2 = (int)transform[m_scratch->polygons[v][2]][Y];
				}
				else
				{
					//b
					if (_c > _b)
					{
						//c,b,a
						x0 = (int)transform[m_scratch->polygons[v][2]][X];
						y0 = (int)transform[m_scratch->polygons[v][2]][Y];
						x1 = (int)transform[m_scratch->polygons[v][1]][X];
						y1 = (int)transform[m_scratch->polygons[v][1]][Y];
						x2 = (int)transform[m_scratch->polygons[v][0]][X];
						y2 = (int)transform[m_scratch->polygons[v][0]][Y];
					}
					else
					{
						//b,c,a
						x0 = (int)transform[m_scratch->polygons[v][1]][X];
						y0 = (int)transform[m_scratch->polygons[v][1]][Y];
						x1 = (int)transform[m_scratch->polygons[v][2]][X];
						y1 = (int)transform[m_scratch->polygons[v][2]][Y];
						x2 = (int)transform[m_scratch->polygons[v][0]][X];
						y2 = (int)transform[m_scratch->polygons[v][0]][Y];
					}
				}
			}


			divmod_result_t uresult;
			//int16_t t = m_scratch->num_triangles;

			int16_t grad[3];
			int16_t c[3];
			int16_t y_coord[3];


			y_coord[0] = y0;
			y_coord[1] = y1;
			y_coord[2] = y2;

			if (y_coord[0] > TV_HEIGHT - 1)
			{
				y_coord[0] = TV_HEIGHT - 1;
			}
			if (y_coord[1] > TV_HEIGHT - 1)
			{
				y_coord[1] = TV_HEIGHT - 1;
			}
			if (y_coord[2] > TV_HEIGHT - 1)
			{
				y_coord[2] = TV_HEIGHT - 1;
			}


			if (y_coord[0] < 0)
			{
				y_coord[0] = 0;
			}
			if (y_coord[1] < 0)
			{
				y_coord[1] = 0;
			}
			if (y_coord[2] >= 0)
			{
				//				y_coord[2] = 0;



							/* Get gradients for calculating intercepts
								Inverted as we want x for changes in y
							*/
				if (y1 - y0 == 0)
				{
					uresult = hw_divider_divmod_s32(((x1 - x0) << 8), 1);
				}
				else
				{
					uresult = hw_divider_divmod_s32(((x1 - x0) << 8), y1 - y0);
				}
				grad[0] = to_quotient_u32(uresult);
				c[0] = x0 - ((grad[0] * y0) >> 8);


				if (y2 - y0 == 0)
				{
					uresult = hw_divider_divmod_s32(((x2 - x0) << 8), 1);
				}
				else
				{
					uresult = hw_divider_divmod_s32(((x2 - x0) << 8), (y2 - y0));
				}
				grad[1] = to_quotient_u32(uresult);
				c[1] = x0 - ((grad[1] * y0) >> 8);


				if (y2 - y1 == 0)
				{
					uresult = hw_divider_divmod_s32(((x2 - x1) << 8), 1);
				}
				else
				{
					uresult = hw_divider_divmod_s32(((x2 - x1) << 8), (y2 - y1));
				}
				grad[2] = to_quotient_u32(uresult);
				c[2] = x2 - ((grad[2] * y2) >> 8);

				uint16_t pencolour = m_scratch->colours[v];


				// TODO: - they were sorted by depth! int i = sum_z[h - 1] & 0xFFF;

				/* Now Plot */


				for (int y = y_coord[0]; y >= y_coord[1]; y--)
				{
					x1 = c[0] + ((y * grad[0]) >> 8);
					x2 = c[1] + ((y * grad[1]) >> 8);

					if (x2 < x1)
					{
						int temp = x2;
						x2 = x1;
						x1 = temp;
					}

					if (x1 < 0)
						x1 = 0;

					if (x2 >= TV_WIDTH - 1)
						x2 = TV_WIDTH - 1;

					int delta = x2 - x1;

					if (delta > 0)
					{
#ifndef DMAPLOT
						for (int a = 0; a <= delta; a++)
						{
							framebuffer[a + x1 + (y * TV_WIDTH)] = pencolour;
						}
#else

						dma_fill16(dma_gfx_1, &framebuffer[x1 + (y * TV_WIDTH)], pencolour, delta + 1);
#endif

					}
				}

				for (int y = y_coord[1]; y >= y_coord[2]; y--)
				{
					x1 = c[1] + ((y * grad[1]) >> 8);
					x2 = c[2] + ((y * grad[2]) >> 8);

					if (x2 < x1)
					{
						int temp = x2;
						x2 = x1;
						x1 = temp;
					}

					if (x1 < 0)
						x1 = 0;

					if (x2 >= TV_WIDTH - 1)
						x2 = TV_WIDTH - 1;

					int delta = x2 - x1;

					if (delta > 0)
					{
#ifndef DMAPLOT
						for (int a = 0; a <= delta; a++)
						{
							framebuffer[a + x1 + (y * TV_WIDTH)] = pencolour;
						}
#else					
						dma_fill16(dma_gfx_0, &framebuffer[x1 + (y * TV_WIDTH)], pencolour, delta + 1);
#endif


					}
				}
			}

		}
	}

}





inline void polygons_fp::dma_fill8(lane_t lane, uint8_t* dst, uint8_t val, uint32_t length)
{


#ifdef PC
	for (uint32_t x = 0; x < length; x++)
	{
		*dst = val;
		dst++;
	}

#else

#ifdef DMA_WAIT
	while ((dma_hw->ch[lane].ctrl_trig & DMA_CH0_CTRL_TRIG_BUSY_BITS))
	{
	}
#endif

	m_dma_lanedata[lane] = val;
	dma_channel_config c0 = dma_channel_get_default_config(lane);  // default configs

	channel_config_set_transfer_data_size(&c0, DMA_SIZE_8);              // 8-bit txfers
	channel_config_set_read_increment(&c0, false);                        // no read incrementing
	channel_config_set_write_increment(&c0, true);                      // write incrementing
//	channel_config_set_chain_to(&c0, lane);                         // chain to other channel
	dma_channel_configure(
		lane,                 // Channel to be configured
		&c0,                        // The configuration we just created
		dst,          // write address (RGB PIO TX FIFO)
		&m_dma_lanedata[lane],            // The initial read address (pixel color array)
		length,                    // Number of transfers - e.g. 320x240 .
		true                       // start immediately.
	);

#endif

}



inline void polygons_fp::dma_fill16(lane_t lane, uint16_t* dst, uint16_t val, uint32_t length)
{


#ifdef PC
	for (uint16_t x = 0; x < length; x++)
	{
		*dst = val;
		dst++;
	}

#else

#ifdef DMA_WAIT
	while ((dma_hw->ch[lane].ctrl_trig & DMA_CH0_CTRL_TRIG_BUSY_BITS))
	{
	}
#endif


	m_dma_lanedata[lane] = val;
	dma_channel_config c0 = dma_channel_get_default_config(lane);  // default configs

	channel_config_set_transfer_data_size(&c0, DMA_SIZE_16);              // 16-bit txfers
	channel_config_set_read_increment(&c0, false);                        // no read incrementing
	channel_config_set_write_increment(&c0, true);                      // write incrementing
//	channel_config_set_chain_to(&c0, lane);                         // chain to other channel
	dma_channel_configure(
		lane,                 // Channel to be configured
		&c0,                        // The configuration we just created
		dst,          // write address (RGB PIO TX FIFO)
		&m_dma_lanedata[lane],            // The initial read address (pixel color array)
		length,                    // Number of transfers - e.g. 320x240 .
		true                       // start immediately.
	);

#endif

}


inline void polygons_fp::dma_fill32(lane_t lane, uint16_t* dst, uint16_t val, uint32_t length)
{


	uint32_t* output = (uint32_t*)dst;


#ifdef PC

	for (uint32_t x = 0; x < length; x++)
	{
		*output = m_dma_lanedata[lane];
		output++;
	}

#else

#ifdef DMA_WAIT
	while ((dma_hw->ch[lane].ctrl_trig & DMA_CH0_CTRL_TRIG_BUSY_BITS))
	{
	}
#endif
	m_dma_lanedata[lane] = (((uint32_t)val) << 16) | val;


	dma_channel_config c0 = dma_channel_get_default_config(lane);  // default configs

	channel_config_set_transfer_data_size(&c0, DMA_SIZE_32);              // 16-bit txfers
	channel_config_set_read_increment(&c0, false);                        // no read incrementing
	channel_config_set_write_increment(&c0, true);                      // write incrementing
//	channel_config_set_chain_to(&c0, lane);                         // chain to other channel
	dma_channel_configure(
		lane,                 // Channel to be configured
		&c0,                        // The configuration we just created
		output,          // write address (RGB PIO TX FIFO)
		&m_dma_lanedata[lane],            // The initial read address (pixel color array)
		length,                    // Number of transfers - e.g. 320x240 .
		true                       // start immediately.
	);


#endif

}


/* Merge Sort. Shamlessly stolen from
* https://www.geeksforgeeks.org/c-program-for-merge-sort/

- Declare left variable to 0 and right variable to n-1
- Find mid by medium formula. mid = (left+right)/2
- Call merge sort on (left,mid)
- Call merge sort on (mid+1,rear)
- Continue till left is less than right
- Then call merge function to perform merge sort.

*/

// Merges two subarrays of arr[].
// First subarray is arr[l..m] 
// Second subarray is arr[m+1..r] 
static int32_t L[1 + POLYGONS / 2], R[1 + POLYGONS / 2];

void polygons_fp::merge(int32_t arr[], int l, int m, int r)
{
	int i, j, k;
	int n1 = m - l + 1;
	int n2 = r - m;

	// Create temp arrays 
	//int L[n1], R[n2];

	// Copy data to temp arrays 
	// L[] and R[] 
	for (i = 0; i < n1; i++)
		L[i] = arr[l + i];
	for (j = 0; j < n2; j++)
		R[j] = arr[m + 1 + j];

	// Merge the temp arrays back 
	// into arr[l..r] 
	// Initial index of first subarray 
	i = 0;

	// Initial index of second subarray 
	j = 0;

	// Initial index of merged subarray 
	k = l;
	while (i < n1 && j < n2) {
		if (L[i] <= R[j]) {
			arr[k] = L[i];
			i++;
		}
		else {
			arr[k] = R[j];
			j++;
		}
		k++;
	}

	// Copy the remaining elements 
	// of L[], if there are any 
	while (i < n1) {
		arr[k] = L[i];
		i++;
		k++;
	}

	// Copy the remaining elements of 
	// R[], if there are any 
	while (j < n2) {
		arr[k] = R[j];
		j++;
		k++;
	}
}

// l is for left index and r is 
// right index of the sub-array 
// of arr to be sorted 
void polygons_fp::mergeSort(int32_t arr[], int l, int r)
{
	if (l < r) {
		// Same as (l+r)/2, but avoids 
		// overflow for large l and r 
		int m = l + (r - l) / 2;

		// Sort first and second halves 
		mergeSort(arr, l, m);
		mergeSort(arr, m + 1, r);

		merge(arr, l, m, r);
	}
}

