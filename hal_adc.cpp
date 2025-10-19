#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hw_defs.h"

#define N_SAMPLES 10
uint16_t sample_buf[N_SAMPLES];


void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count) {
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);
    for (size_t i = 0; i < count; i = i + 1)
        buf[i] = adc_fifo_get_blocking();
    adc_run(false);
    adc_fifo_drain();
}


    
void hal_adc_init()
{
    adc_init();
    //adc_set_temp_sensor_enabled(true);
#if 0
    gpio_disable_pulls(ADC_12V);
    gpio_set_input_enabled(ADC_12V, false);
    gpio_disable_pulls(ADC_NEG5V);
    gpio_set_input_enabled(ADC_NEG5V , false);
    gpio_disable_pulls(ADC_5V);
    gpio_set_input_enabled(ADC_5V, false);
#endif

//    gpio_set_dir(ADC_12V, false);
    gpio_set_function(ADC_12V, GPIO_FUNC_SIO);
    gpio_set_input_enabled(ADC_12V, false);
    gpio_disable_pulls(ADC_12V);
 
    gpio_set_function(ADC_NEG5V, GPIO_FUNC_SIO);
    gpio_set_input_enabled(ADC_NEG5V , false);
    gpio_disable_pulls(ADC_NEG5V);

    gpio_set_function(ADC_5V, GPIO_FUNC_SIO);
    gpio_set_input_enabled(ADC_5V, false);
    gpio_disable_pulls(ADC_5V);
    



}

static float result_table[3];

// 12-bit ADC
// Old fixed point routines expect 1.0 = 1024 number scaling
#define  conversion_factor  (3.3f / ((float)(1 << 12)))


int16_t hal_adc_get(int gpio)
{

    int16_t result = 0;
    switch (gpio)
    {
        case ADC_12V:
            result = 1024.0f * result_table[0]  * conversion_factor *  ((4700.0f/1000.0f)+1.0f);
        break;
        case ADC_NEG5V:
            result = 1024.0f * (3.3f - (3.3f -result_table[1] * conversion_factor) * ((4700.0f/1000.0f)+1.0f));
        break;
        case ADC_5V:
            result = 1024.0f * result_table[2] * conversion_factor *  ((4700.0f/2000.0f)+1.0f);
        break;

        default:
        //NOT AN ADC PIN
        break;
    }
    
    return (result);

}


void hal_adc_poll()
{
    static int x = 0;

    uint32_t result = adc_read();
    
    result_table[x] = result;
    x++;
    if (x == 3)
    {
        x = 0;
    }
    adc_select_input(x);

}

