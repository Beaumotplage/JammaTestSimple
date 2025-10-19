#pragma once
/* Hardware definitions for stuff like pins and DMA */


/* 13/05/24 Pinouts:
    (Resistor values are ideal, can be something close-ish) 
    RGB arrangement allows standard 3:3:2 colour using just the lower 8-bits if you're short on RAM

   GPIO     Purpose 

    0       Red 5 1K
    1       Red 6 500R
    2       Red 7 250R
    3       Green 5 1K
    4       Green 6 500R
    5       Green 7 250R
    6       Blue 6 500R
    7       Blue 7 250R

    8       Red 3 (LSB) 4K
    9       Red 4 2K
    10      Green 3 (LSB) 4K
    11      Green 4 2K
    12      Blue 3 (LSB) 4K
    13      Blue 4 2K
    14      Blue 5 1K
    15      CSYNC 470R


    (next side of board)
    16      SPI_RX
    17      SPI_CS
    18      SPI_CLK
    19      N/C (SPI TX)
    20      LED (470R or thereabouts)
    21      Service button
    22      AUDIO - 500R -> 10nf->220uF 'T' filter (see below)


    Watch for ground pins when counting.
    Make sure you join video ground to the Pico ground.

    Resistors just need to be approx for colours (470R is ok for 250R).
    You can even make them using series and parallel 500R's:  500R || 500R (250R), 500R, 500R*2, 500R*4, 500R*8.
    For a colour, connect the 5 resistors to the pico, then join the other ends and take to the video.

    For audio:
    22 --500R-------(+220uF-)-----amplifier
             |
             10nf
             |
             GND
    500R and 10nf ('103') forms a lowpass filter to remove PWM  ( 1/(2*PI*RC) = 16kHz  )
    220uF is approx (100uF, 470uF are ok) - a chunky electrolytic around that size. Make sure -ve side goes to audio amp line in. 
    Takes out speaker-melting DC

            
*/
/* PINS */

// Give the I/O pins that we're using some names that make sense - usable in main()
enum testcard_pins {RED_PIN = 0,  
              CSYNC_PIN=15,
              PIN_MISO = 16,
              PIN_CS = 17,
              PIN_SCK = 18,
              PIN_MOSI = 19,
              TEST_PIN = 20, /* Move me back to 23*/
              AUDIO_PIN = 22,
              AUDIO_PIN2 = 23, /* inverse PWM */
              AUDIO_ON = 24, /* NOT_STANDBY */ 
              ADC_12V = 26,
              ADC_NEG5V = 27,
              ADC_5V = 28,
              } ;



#define SPI_PORT spi0


/*  DMA   */

typedef enum
{
     audio_pwm_dma_chan =0,
     audio_trigger_dma_chan =1,
     audio_sample_dma_chan =2,

     rgb_chan_layer_top_data = 3, 
     rgb_chan_layer_top_cfg =4,

     rgb_chan_layer_bottom_data = 5, 
     rgb_chan_layer_bottom_cfg = 6,

     dma_gfx_0 = 7,
     dma_gfx_1 =8,
     dma_gfx_2 =9,
     dma_gfx_3 = 10,
    dma_gfx_4 = 11,
    dma_gfx_5 = 12,
    dma_gfx_6 = 13,
    dma_gfx_7 = 14,
    dma_gfx_8 = 15,
    
}lane_t;