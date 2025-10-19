
extern "C" 
{       

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"
#include "hardware/adc.h"



#include "hw_defs.h"

#include "media\modfile.h"
#include "app\hxcmod.h"
}
#include "app\app_main.h"

#define ADC_CHANNEL 2

modcontext modloaded;
unsigned char* modfile;
tracker_buffer_state trackbuf_state1;
tracker_buffer_state trackbuf_state2;

// The fixed location the sample DMA channel writes to and the PWM DMA channel
// reads from
uint32_t single_sample = 0;
uint32_t* single_sample_ptr = &single_sample;


#define REPETITION_RATE 16

static unsigned char audio_buffer[5000];
static unsigned char  audio_buffer2[5000];

extern AppMain app_main;

void dma_irh() {
    static int doublebuffer = 0;

    dma_hw->ints1 = (1u << audio_trigger_dma_chan);


    if (!doublebuffer)
    {
        dma_hw->ch[audio_sample_dma_chan].al1_read_addr = (uint32_t)audio_buffer;
        dma_hw->ch[audio_trigger_dma_chan].al3_read_addr_trig = (uint32_t)&single_sample_ptr;

        app_main.m_audio.run(&modloaded, &audio_buffer2[0], sizeof(audio_buffer2), &trackbuf_state1);

        doublebuffer = 1;    
    }
    else
    {
        dma_hw->ch[audio_sample_dma_chan].al1_read_addr = (uint32_t)audio_buffer2;
        dma_hw->ch[audio_trigger_dma_chan].al3_read_addr_trig = (uint32_t)&single_sample_ptr;

        app_main.m_audio.run(&modloaded, &audio_buffer[0], sizeof(audio_buffer), &trackbuf_state1);

        doublebuffer = 0;
    }




}

void audio_main(void) 
{
   // stdio_init_all();


    hxcmod_init(&modloaded);

    hxcmod_setcfg(&modloaded, 22000, 1, 1);

    //	modfile = unpack(data_cartoon_dreams_n_fantasies_mod->data,data_cartoon_dreams_n_fantasies_mod->csize ,data_cartoon_dreams_n_fantasies_mod->data, data_cartoon_dreams_n_fantasies_mod->size);
  // modfile = escape1;

//    hxcmod_load(&modloaded, (void*)escape1, sizeof(escape1));
    hxcmod_load(&modloaded, (void*)rave1, sizeof(rave1));

    gpio_init(AUDIO_PIN2);

    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    gpio_set_function(AUDIO_PIN2, GPIO_FUNC_PWM);
    gpio_init(AUDIO_ON);
    gpio_set_dir(AUDIO_ON, true);
    gpio_put(AUDIO_ON, true);



    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_config config = pwm_get_default_config();
//    pwm_config_set_clkdiv(&config, (15.0f/12.5f) *22.1f / REPETITION_RATE);

    //22.1kHz, 8-bit
    pwm_config_set_clkdiv(&config, (150.0e6f/(22100.0f * 256.0f)) / REPETITION_RATE);

    pwm_config_set_wrap(&config, 255);
    pwm_init(audio_pin_slice, &config, true);

//    pwm_hw->slice[audio_pin_slice].cc = 100; // Test {W<}


//    audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN2);

   // pwm_config config = pwm_get_default_config();
  //  pwm_config_set_clkdiv(&config, (15.0/12.5f) *22.1f / REPETITION_RATE);
   // pwm_config_set_wrap(&config, 255);
   // pwm_init(audio_pin_slice, &config, true);

   pwm_set_chan_level(audio_pin_slice,PWM_CHAN_A, 1);
   pwm_set_chan_level(audio_pin_slice,PWM_CHAN_B, 0);

    pwm_set_enabled(audio_pin_slice, true);


    // Setup PWM DMA channel
    dma_channel_config pwm_dma_chan_config = dma_channel_get_default_config(audio_pwm_dma_chan);
    // Transfer 32-bits at a time
    channel_config_set_transfer_data_size(&pwm_dma_chan_config, DMA_SIZE_32);
    // Read from a fixed location, always writes to the same address
    channel_config_set_read_increment(&pwm_dma_chan_config, false);
    channel_config_set_write_increment(&pwm_dma_chan_config, false);
    // Chain to sample DMA channel when done
    channel_config_set_chain_to(&pwm_dma_chan_config, audio_sample_dma_chan);
    // Transfer on PWM cycle end
    channel_config_set_dreq(&pwm_dma_chan_config, DREQ_PWM_WRAP0 + audio_pin_slice);

    dma_channel_configure(
        audio_pwm_dma_chan,
        &pwm_dma_chan_config,
        // Write to PWM slice CC register
        &pwm_hw->slice[audio_pin_slice].cc,
        // Read from single_sample
        &single_sample,
        // Transfer once per desired sample repetition
        REPETITION_RATE,
        // Don't start yet
        false
    );

    // Setup trigger DMA channel
    dma_channel_config trigger_dma_chan_config = dma_channel_get_default_config(audio_trigger_dma_chan);
    // Transfer 32-bits at a time
    channel_config_set_transfer_data_size(&trigger_dma_chan_config, DMA_SIZE_32);
    // Always read and write from and to the same address
    channel_config_set_read_increment(&trigger_dma_chan_config, false);
    channel_config_set_write_increment(&trigger_dma_chan_config, false);
    // Transfer on PWM cycle end
    channel_config_set_dreq(&trigger_dma_chan_config, DREQ_PWM_WRAP0 + audio_pin_slice);

    dma_channel_configure(
        audio_trigger_dma_chan,
        &trigger_dma_chan_config,
        // Write to PWM DMA channel read address trigger
        &dma_hw->ch[audio_pwm_dma_chan].al3_read_addr_trig,
        // Read from location containing the address of single_sample
        &single_sample_ptr,
        // Need to trigger once for each audio sample but as the PWM DREQ is
        // used need to multiply by repetition rate
        REPETITION_RATE *  sizeof(audio_buffer),
        false
    );

    // Fire interrupt when trigger DMA channel is done
    dma_channel_set_irq1_enabled(audio_trigger_dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_1, dma_irh);
        irq_set_priority(DMA_IRQ_1,255);
    irq_set_enabled(DMA_IRQ_1, true);

    // Setup sample DMA channel
    dma_channel_config sample_dma_chan_config = dma_channel_get_default_config(audio_sample_dma_chan);
    // Transfer 8-bits at a time
    channel_config_set_transfer_data_size(&sample_dma_chan_config, DMA_SIZE_8);
    // Increment read address to go through audio buffer
    channel_config_set_read_increment(&sample_dma_chan_config, true);
    // Always write to the same address
    channel_config_set_write_increment(&sample_dma_chan_config, false);

    dma_channel_configure(
        audio_sample_dma_chan,
        &sample_dma_chan_config,
        // Write to single_sample
        &single_sample,
        // Read from audio buffer
        audio_buffer,
        // Only do one transfer (once per PWM DMA completion due to chaining)
        1,
        // Don't start yet
        false
    );

    // Kick things off with the trigger DMA channel
    dma_channel_start(audio_trigger_dma_chan);
#if 0
    while(1) {
        __wfi();
    }
    #endif
}