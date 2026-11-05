#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define BUTTON_GPIO 14
#define BUTTON_INTERRUPT_TYPE GPIO_IRQ_EDGE_RISE

static int cookies = 0;

void init_spi_program() {

}

void init_i2c_program() {

}

void isr_button_click() {
    gpio_acknowledge_irq(BUTTON_GPIO, BUTTON_INTERRUPT_TYPE);
    cookies++;
    // IO_BANK0_BASE
}

void init_buttonirq() {
    // turn on GPIO pin connected to button, set function to SIO, direction input
    gpio_init(BUTTON_GPIO);
    gpio_set_function(BUTTON_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);

    // enable irq for button and set handler for 
    irq_set_exclusive_handler(IO_IRQ_BANK0, isr_button_click);
    gpio_set_irq_enabled(BUTTON_GPIO, BUTTON_INTERRUPT_TYPE, true);
    nvic_hw->iser[0] = (1 << 21);
    // gpio_irq
}

int main () {
    // gotta
    stdio_init_all();

    // a bunch of inits down here
        // init SPI
        // init I2C
        // init interrupts
    init_buttonirq();
        // init PWM/sound stuff

    // main loop
        // update cookies (maybe put on interrupt?)
            // separate func for click?
        // update display
    for (;;)
    {
        printf("%d\n", cookies);
        // cookies++;
        sleep_ms(500);
    }
}