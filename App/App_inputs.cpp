#include "pc_or_rp2040.h"
#include "app_inputs.h"

#include "..\hw_defs.h"

extern uint32_t hal_spi_get_inputs(void);
extern int16_t hal_adc_get(int gpio);


//TODO: This is stupid, bin it and get direct?
int16_t AppInputs::get_adc(inputs_adc_t channel)
{
	//TODO: Replace with HAL calls
	int16_t result;
	switch (channel)
	{
	case AD12V:
	{
		result = hal_adc_get(ADC_12V);
	}break;
	case AD_NEG5V:
	{
		result = hal_adc_get(ADC_NEG5V);
	}break;
	case AD5V:
	{
		result = hal_adc_get(ADC_5V);
	}break;
	default:
	{
		result = 0;
	}
	}
	return (result);
}


	

uint32_t AppInputs::get_inputs(void)
{
	return (hal_spi_get_inputs());
}