#pragma once


/* DMA */

/* Stub commands like DMA wait: 
dma_hw->ch[DMA_LANE0].ctrl_trig & 0x01000000)
*/
#include <stdint.h>
typedef struct {
	uint32_t ctrl_trig;
	uint32_t unused_test;
}dma_stub_t;

typedef struct {
	dma_stub_t ch[12];
}dma_hardware;

extern dma_hardware dma;
extern dma_hardware* dma_hw;





/***************** Divider ***************/
typedef struct
{
	uint32_t quotient;
	uint32_t remainder;
}divmod_result_t;


divmod_result_t hw_divider_divmod_u32(uint32_t udividend, uint32_t udivisor);
divmod_result_t hw_divider_divmod_s32(int32_t udividend, int32_t udivisor);

uint32_t to_quotient_u32(divmod_result_t data);
uint32_t to_remainder_u32(divmod_result_t data);


/***************** Interpolator ***************/


typedef struct
{
	int32_t accum[2];
	uint32_t shift[2];
	uint32_t mask[2];
	int32_t base[2];
	uint32_t pop[2];
	uint32_t sign_bit[2];
	uint32_t sign_extend[2];
}interp_t;


/*  DMA   */
// Copied from hw_defs.h in the RP2040 code
#if 0
typedef enum
{
	audio_pwm_dma_chan = 0,
	audio_trigger_dma_chan = 1,
	audio_sample_dma_chan = 2,

	rgb_chan_layer0_data = 3,
	rgb_chan_layer0_cfg = 4,

	rgb_chan_layer2_data = 7,
	rgb_chan_layer2_cfg = 8,

	dma_gfx_0 = 9,
	dma_gfx_1 = 10,
	dma_gfx_2 = 11,
	dma_gfx_3 = 5,
	dma_gfx_4 = 6,

}lane_t;
#endif
#define __no_inline_not_in_flash_func 

