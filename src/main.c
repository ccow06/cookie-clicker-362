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
#include "bignumber.h"

#define COOKIE_BUTTON_GPIO 14
#define BUTTON_INTERRUPT_TYPE GPIO_IRQ_EDGE_RISE
#define NUM_BUILDINGS 4
#define UPDATE_HZ 1
// update_hz is to keep it consistent how many times the cookies update per sec
// 

// (mult, and all other needed arrs) indices for each purchase (generator)
#define IDX_CLICKER 0
#define IDX_GRANDMA 1
#define IDX_FARM 2
#define IDX_MINE 3
// cont for each gen ...

// for testing
#define START_Q 1


// bro I gotta make them BIG NUMBERS
BigNumber cookies = {0,0}; // total num of cookies in "bank" idk
BigNumber cookieQ = {0,0}; // cookie Queue, hahaha how clever
int* num = NULL; // number of each cookie generator purchased
BigNumber* baseCPS = NULL; // num of cookies each cookie gen generates (per second)
BigNumber* xtraCPS = NULL; // extra CPS (from upgrades)

BigNumber* bonusCPS = NULL; // extra CPS for cross-upgrades? (may not use?)
BigNumber totalCPS = {0,0}; // total CPS (may not use?)

BigNumber* cost = NULL; // technically just directly related to number of each gen purchased

void init_arrays() {
    num = calloc(NUM_BUILDINGS, sizeof(*num));
    baseCPS = calloc(NUM_BUILDINGS, sizeof(*baseCPS));
    xtraCPS = calloc(NUM_BUILDINGS, sizeof(*xtraCPS));
    
    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        num[i] = START_Q;
        xtraCPS[i] = (BigNumber){0,0};
    }
    baseCPS[IDX_CLICKER] = (BigNumber){0.1,0};
    baseCPS[IDX_GRANDMA] = (BigNumber){1,0};
    baseCPS[IDX_FARM] = (BigNumber){8,0};
    baseCPS[IDX_MINE] = (BigNumber){17,0};

}

void init_spi_program() {

}

void init_i2c_program() {

}

void isr_button_click() {
    gpio_acknowledge_irq(COOKIE_BUTTON_GPIO, BUTTON_INTERRUPT_TYPE);
    cookies.mantissa++;
}

// updates cookieQ and adds to total cookie count, then resets cookieQ
void isr_cookie_update() {
    timer0_hw->intr = (1 << 0);
    timer0_hw->alarm[0] = timer0_hw->timerawl + (1000000 / UPDATE_HZ); // 1M us = 1s, divided by updates per sec

    addBN(totalCPS, cookieQ, &cookieQ);
    addBN(cookieQ, cookies, &cookies);
    
    cookieQ = (BigNumber){0,0};
    printf("cookie!");
}

void updateCPS() {

    BigNumber temp;

    for (int i = 0; i < NUM_BUILDINGS; i++)
    {
        if (num[i])
        {
            multBN((BigNumber){num[i],0}, baseCPS[i], &temp);
            addBN(temp, xtraCPS[i], &temp);
            divBN(temp, (BigNumber){UPDATE_HZ,0}, &temp);
            addBN(temp, totalCPS, &totalCPS);
        }
    }
}

void init_timer_irq() {
    // irq_set_enabled(TIMER0_IRQ_0, true);
    irq_set_exclusive_handler(TIMER0_IRQ_0, isr_cookie_update);

    nvic_hw->icpr[0] = (1 << 0);
    nvic_hw->iser[0] = (1 << 0);
    timer0_hw->inte = (1 << 0);
    timer0_hw->alarm[0] = timer0_hw->timerawl + (1000000 / UPDATE_HZ); // 1M us = 1s, divided by updates per sec
}

void init_buttonirq() {
    // turn on GPIO pin connected to button, set function to SIO, direction input
    gpio_init(COOKIE_BUTTON_GPIO);
    gpio_set_function(COOKIE_BUTTON_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(COOKIE_BUTTON_GPIO, GPIO_IN);

    // enable irq for button and set handler for 
    irq_set_exclusive_handler(IO_IRQ_BANK0, isr_button_click);
    gpio_set_irq_enabled(COOKIE_BUTTON_GPIO, BUTTON_INTERRUPT_TYPE, true);
    nvic_hw->iser[0] = (1 << 21);
}

int main () {
    // gotta
    stdio_init_all();

    
    // maybe loading happens here? no idea how EEPROM works

    
    // a bunch of inits down here
    init_arrays();
        // init SPI
        // init I2C
        // init interrupts
    init_buttonirq();
        // init PWM/sound stuff
    
    updateCPS();
    
    // main loop
        // update cookies (maybe put on interrupt?)
            // separate func for click?
        // update display
    for (;;)
    {
        printf("%d\n", (int)cookies.mantissa);
        // cookies++;
        sleep_ms(1000);
    }
}