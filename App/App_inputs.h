#pragma once
#include <stdint.h>




class AppInputs
{
public:

	// Should match the order in the schematic
	typedef enum
	{
		B_UP1 = 0,
		B_START2,
		B_START1,
		B_COIN2,
		B_COIN1,
		B_TEST,
		B_TILT,
		B_SERVICE,

		B_1P_1,
		B_RIGHT2,
		B_RIGHT1,
		B_LEFT2,
		B_LEFT1,
		B_DOWN2,
		B_DOWN1,
		B_UP2,

		B_1P_5,
		B_2P_4,
		B_1P_4,
		B_2P_3,
		B_1P_3,
		B_2P_2,
		B_1P_2,
		B_2P_1,


		B_DIP4,
		B_DIP3,
		B_DIP2,
		B_DIP1,
		B_DIP0,
		B_2P_6,
		B_1P_6,
		B_2P_5,

	}inputs_t;

	typedef enum
	{
		AD12V,
		AD_NEG5V,
		AD5V,
	}inputs_adc_t;

	
	uint32_t get_inputs(void);
	int16_t get_adc(inputs_adc_t channel);
	
private:
};