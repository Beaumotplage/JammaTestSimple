#pragma once
#include "pc_peripheral_stubs.h"

/******************* DMA ********************/

/* Stub commands like DMA wait:
dma_hw->ch[DMA_LANE0].ctrl_trig & 0x01000000)
*/


dma_hardware dma;
dma_hardware* dma_hw = &dma;




/***************** Divider ***************/


divmod_result_t hw_divider_divmod_s32(int32_t udividend, int32_t udivisor)
{
	divmod_result_t result;
	//if (udivisor == 0)
	//	udivisor = 1;

		result.quotient = udividend / udivisor;
		result.remainder = udividend - (udivisor * result.quotient);
	
	
	return result;
}


divmod_result_t hw_divider_divmod_u32(uint32_t udividend, uint32_t udivisor)
{
	divmod_result_t result;
	result.quotient = udividend / udivisor;
	result.remainder = udividend - (udivisor * result.quotient);
	return result;
}

uint32_t to_quotient_u32(divmod_result_t data)
{
	return data.quotient;
}

uint32_t to_remainder_u32(divmod_result_t data)
{
	return data.remainder;
}



/***************** Interpolator ***************/

// in class

