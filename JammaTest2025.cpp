
// VGA graphics library
#include "JAMMATest.h"


//extern "C" 
//{       

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "pico/multicore.h"

 
extern int io_spi_main();
extern int audio_main(void);
extern void hal_adc_init();


bool g_flash_busy;
bool g_stalled = 0;
/* Mailbox to CPU to stall it, when the other is hogging the FLASH for a bulk read*/
/* You need this, otherwise it'll crash */
    static int core1_rx_val = 0;

void __no_inline_not_in_flash_func  (core0_sio_irq)(void) 
{
            g_stalled = 1;
    while (g_flash_busy)
    {

    // Just record the latest entry
    while (multicore_fifo_rvalid())
        core1_rx_val = multicore_fifo_pop_blocking();

    multicore_fifo_clear_irq();

    }
    g_stalled = 0;
}

void stall_cpu(void)
{
    g_flash_busy =1;
    multicore_fifo_push_blocking(0x7777);

    while (! g_stalled)
    {};

}

void unstall_cpu(void)
{
    g_flash_busy = 0;

}


extern void Halvideo_hsync_start();
void core1_main() 
{

    // Go to endless loop in app, waiting for code from main cpu
   extern void hal_video_cpu2(void);
    hal_video_cpu2();
}



int main() 
{

    // Initialize the VGA screen
  
    HAL_video_setFramebuffer();

    io_spi_main(); // WARNING - check CS pin isn't hijacked by other code
    hal_adc_init();

    multicore_launch_core1(core1_main);

    audio_main();
   
        
    while(true) 
    {
        

   }

}


