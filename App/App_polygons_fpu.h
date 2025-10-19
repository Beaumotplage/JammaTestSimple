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
#include "..\hw_defs.h"

#endif

class polygons_fp : public AppVideoModes
{
public:

	typedef struct {

		float yaw;
		float pitch;
		float roll;
		float x;
		float y;
		float z;
	}pos;


	typedef struct
	{
		uint16_t start[10]; // strange PC behaviour writes to here
		int16_t vertices[VERTICES][3];
		int16_t polygons[POLYGONS][3];
		uint16_t colours[POLYGONS];
		float transform_a[VERTICES][4];
		float transform_b[VERTICES][4];
		int32_t sum_z_a[POLYGONS];
		int32_t sum_z_b[POLYGONS];

#ifdef CULLNORMALS
		int16_t normals[POLYGONS];
#endif
	}polygonbuff_t;

	polygonbuff_t* m_scratch;
	uint16_t null; // drawing canvas is just used for linebuffer modes 

	polygons_fp(polygonbuff_t* polygonram) :
		AppVideoModes{&null},
		m_scratch{ polygonram }
	{
	};

	void run60Hz(bool init, AppAudio::SoundCodes* audio, uint16_t* framebuffer) override;
	void runcpu2(bool init, uint16_t* framebuffer) override
	{
		cpu2_helper_loop(init, framebuffer);
	};

private:
	void cull_and_rasterise(uint16_t* framebuffer, float(*transform)[4], int32_t* sum_z);
	void transform_and_sort(uint16_t* framebuffer, float(*transform)[4], int32_t* sum_z);
	void init_polys( AppAudio::SoundCodes* audio, uint16_t* framebuffer);


	inline void dma_fill8(lane_t lane, uint8_t* dst, uint8_t val, uint32_t length);
	inline void dma_fill16(lane_t lane, uint16_t* dst, uint16_t val, uint32_t length);
	inline void dma_fill32(lane_t lane, uint16_t* dst, uint16_t val, uint32_t length);

	void cpu2_helper_loop(bool init, uint16_t* framebuffer);

	void mergeSort(int32_t arr[], int l, int r);

	void merge(int32_t arr[], int l, int m, int r);

	pos m_pos;
	uint16_t m_dma_line_counter;
	uint16_t* m_next_top_layer;
	uint16_t m_clear = 0;
	uint32_t m_dma_trigger = 0;
	bool m_first_pass = 1;

	volatile bool m_cpu_double_buffer = 0;

 	volatile int m_sub_cpu_state = 0;

};

