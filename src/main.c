#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <hardware/gpio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "bignumber.h"

#define COOKIE_BUTTON_GPIO 14
#define BUTTON_INTERRUPT_TYPE GPIO_IRQ_EDGE_FALL
#define XAXIS_ADC_GPIO 40
#define YAXIS_ADC_GPIO 41
#define JOYSTICK_SEL_GPIO 42

#define NUM_BUILDINGS 4
// update_hz: how many times (per second) cookies should run updates
#define UPDATE_HZ 100
// ADC_hz: how many times (per second) ADC should run new trials (default: update_hz)
#define ADC_HZ UPDATE_HZ

// joystick_sensitivity: out of 4096, what value should the joystick reach to be considered a full "flick"
#define JOYSTICK_SENS 300
// joystick_buffer: how much additional room is afforded on the backswing, AKA when joystick is triggered one direction, how much
//                  room must it go the other direction to "un-trigger"
#define JOYSTICK_BUFF 300

// index_X_max: how many horizontal selections (i.e. slots for buy, buy10, sell, etc.) are there total
#define INDX_MAX 2
// index_Y_max: how many vertical selections (i.e. buildings, cookie, upgrade, etc.) are there total
#define INDY_MAX NUM_BUILDINGS-1

// (mult, and all other needed arrs) indices for each purchase (generator)
#define IDX_CLICKER 0
#define IDX_GRANDMA 1
#define IDX_FARM 2
#define IDX_MINE 3
// cont for each gen ...

// for testing
#define START_Q 0


// bro I gotta make them BIG NUMBERS
BigNumber cookies = {0,0}; // total num of cookies in "bank" idk
BigNumber cookieQ = {0,0}; // cookie Queue, hahaha how clever
int num[NUM_BUILDINGS]; // number of each cookie generator purchased
BigNumber baseCPS[NUM_BUILDINGS]; // num of cookies each cookie gen generates (per second)
BigNumber xtraCPS[NUM_BUILDINGS]; // extra CPS (from upgrades)
char building_names[NUM_BUILDINGS][16] = {
    "Clicker",
    "Grandma",
    "Farm",
    "Mine"
};

BigNumber* bonusCPS = NULL; // extra CPS for cross-upgrades? (may not use?)
BigNumber totalCPS = {0,0}; // total CPS (may not use?)

BigNumber* cost = NULL; // technically just directly related to number of each gen purchased

int selX = 0; // -1 if left, 0 if center, 1 if right (used for selections)
int selY = 0; // -1 if down, 0 if center, 1 if up (used for selections)
int selS = 0; // 1 if pressed, 0 if released (used for selections)
int indX = 0; // used for current selection
int indY = 0; // used for current selection

void init_arrays() {
    // num = calloc(NUM_BUILDINGS, sizeof(*num));
    // baseCPS = calloc(NUM_BUILDINGS, sizeof(*baseCPS));
    // xtraCPS = calloc(NUM_BUILDINGS, sizeof(*xtraCPS));
    
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

void purchase_building() {
    
}

void isr_joystick_check() {
    timer0_hw->intr = 1u << 1;

    // channel 0 (x-axis)
    adc_hw->cs &= (0xffff0fff); // enable channel 0
    adc_hw->cs |= 0x4; // init single-shot conversion
    while((adc_hw->cs & 0x100) != 0x100); // wait until the ready bit is enabled
    uint16_t adcX_results = adc_hw->result & 0xffff;
    // channel 1 (y-axis)
    adc_hw->cs |= (1u << 12); // enable channel 1
    adc_hw->cs |= 0x4; // init single-shot conversion
    while((adc_hw->cs & 0x100) != 0x100); // wait until the ready bit is enabled
    uint16_t adcY_results = adc_hw->result & 0xffff;
    // channel 2 (sel) (because button interrupts DON'T WORK!!!!)
    adc_hw->cs &= (0xffff0fff); // clear channel
    adc_hw->cs |= (2u << 12); // enable channel 2
    adc_hw->cs |= 0x4; // init single-shot conversion
    while((adc_hw->cs & 0x100) != 0x100); // wait until the ready bit is enabled
    uint16_t adcS_results = adc_hw->result & 0xffff;

    // X movement
    if (selX)
    {
        if (adcX_results < (4096 - (JOYSTICK_SENS + JOYSTICK_BUFF)) && adcX_results > (JOYSTICK_SENS + JOYSTICK_BUFF))
        {
            selX = 0;
        }
    }
    else 
    {
        if (adcX_results < JOYSTICK_SENS)
        {
            selX = -1;
        }
        else if (adcX_results > (4096 - JOYSTICK_SENS))
        {
            selX = 1;
        }
        // else, keep at 0

        // do something with the selection
        indX += selX;
        if (indX < 0)
            indX = 0;
        else if (indX > INDX_MAX)
            indX = INDX_MAX;
    }
    // Y movement
    if (selY)
    {
        if (adcY_results < (4096 - (JOYSTICK_SENS + JOYSTICK_BUFF)) && adcY_results > (JOYSTICK_SENS + JOYSTICK_BUFF))
        {
            selY = 0;
        }
    }
    else 
    {
        if (adcY_results < JOYSTICK_SENS)
        {
            selY = -1;
        }
        else if (adcY_results > (4096 - JOYSTICK_SENS))
        {
            selY = 1;
        }
        // else, keep at 0

        // TODO: do something WITH the selection
        indY += selY;
        if (indY < 0)
            indY = 0;
        else if (indY > INDY_MAX)
            indY = INDY_MAX;
    }
    if (selS)
    {
        if (adcS_results > JOYSTICK_SENS)
        {
            selS = 0;
            printf("Released!\n");
            purchase_building();
        }
    }
    else
    {
        if (adcS_results < JOYSTICK_SENS)
        {
            selS = 1;
            printf("Pressed...\n");
        }
    }
    // printf("SELX: %d - SELY: %d\n", selX, selY); // for testing
    // printf("ADC-X: %d - ADX-Y: %d\n", adcX_results, adcY_results); // for testing
    // printf("%d\n", adcS_results);

    timer0_hw->alarm[1] = timer0_hw->timerawl + (1000000u / ADC_HZ);
}

void init_adc_program() {

    adc_init();
    gpio_init(XAXIS_ADC_GPIO);
    gpio_init(YAXIS_ADC_GPIO);
    gpio_init(JOYSTICK_SEL_GPIO);

    resets_hw->reset = RESET_ADC; // reset ADC
    adc_hw->cs = ADC_CS_EN_BITS; // enable ADC

    gpio_set_function(XAXIS_ADC_GPIO, GPIO_FUNC_NULL);
    gpio_set_function(YAXIS_ADC_GPIO, GPIO_FUNC_NULL);
    gpio_set_function(JOYSTICK_SEL_GPIO, GPIO_FUNC_NULL);

    // enable timer0_1
    timer0_hw->inte |= 1u << 1;
    irq_set_exclusive_handler(TIMER0_IRQ_1, isr_joystick_check);
    irq_set_enabled(TIMER0_IRQ_1, true);

    nvic_hw->icpr[0] |= 1u << 1;
    nvic_hw->iser[0] |= 1u << 1;
    uint32_t temp = timer0_hw->timerawl + (1000000u / ADC_HZ) + 10u;
    timer0_hw->alarm[1] = temp; // 1M us = 1s, divided by updates per sec

    pads_bank0_hw->io[XAXIS_ADC_GPIO] = 0; // gpio_disable_pulls
    pads_bank0_hw->io[YAXIS_ADC_GPIO] = 0; // gpio_disable_pulls
    pads_bank0_hw->io[JOYSTICK_SEL_GPIO] = 0;

}

void init_spi_program() {

}

void init_i2c_program() {

}

// updates cookieQ and adds to total cookie count, then resets cookieQ
void isr_cookie_update() {
    timer0_hw->intr = 1u;
    // gpio_acknowledge_irq(COOKIE_BUTTON_GPIO, BUTTON_INTERRUPT_TYPE)

    addBN(totalCPS, cookieQ, &cookieQ);
    addBN(cookieQ, cookies, &cookies);
    
    cookieQ = (BigNumber){0,0};
    // printf("cookie!\n");

    timer0_hw->alarm[0] = timer0_hw->timerawl + (1000000 / UPDATE_HZ); // 1M us = 1s, divided by updates per sec
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
    timer0_hw->inte |= 1u;
    irq_set_exclusive_handler(TIMER0_IRQ_0, isr_cookie_update);
    irq_set_enabled(TIMER0_IRQ_0, true);

    nvic_hw->icpr[0] |= 1u;
    nvic_hw->iser[0] |= 1u;
    uint32_t temp = timer0_hw->timerawl + (1000000u / UPDATE_HZ);
    timer0_hw->alarm[0] = temp; // 1M us = 1s, divided by updates per sec
    // timer0_hw->armed = 1u;
}

void isr_button_click() {
    gpio_acknowledge_irq(COOKIE_BUTTON_GPIO, BUTTON_INTERRUPT_TYPE);
    // gpio_acknowledge_irq(JOYSTICK_SEL_GPIO, BUTTON_INTERRUPT_TYPE);
    cookies.mantissa++;
}

void init_buttonirq() {
    // turn on GPIO pin connected to cookie button, set function to SIO, direction input
    gpio_init(COOKIE_BUTTON_GPIO);
    gpio_set_function(COOKIE_BUTTON_GPIO, GPIO_FUNC_SIO);
    gpio_set_dir(COOKIE_BUTTON_GPIO, GPIO_IN);

    // // same thing as above but for joystick
    // gpio_init(JOYSTICK_SEL_GPIO);
    // gpio_set_function(JOYSTICK_SEL_GPIO, GPIO_FUNC_SIO);
    // gpio_set_dir(JOYSTICK_SEL_GPIO, GPIO_IN);

    // enable irq for button and set handler for 
    irq_set_exclusive_handler(IO_IRQ_BANK0, isr_button_click);
    gpio_set_irq_enabled(COOKIE_BUTTON_GPIO, BUTTON_INTERRUPT_TYPE, true);
    // gpio_set_irq_enabled(JOYSTICK_SEL_GPIO, BUTTON_INTERRUPT_TYPE, true);
    nvic_hw->iser[0] = (1 << 21);
}

// mostly for consolidated testing, generally nothing back comes of simply printing to the console it's fine
void print_display() {
    printf("%d\n", (int)cookies.mantissa);
    printf("CPS: %.2f\n", totalCPS.mantissa * UPDATE_HZ);
    printf("Option (x,y): %d, %d\n", indX, indY);
    printf("Selection: %s\n", building_names[indY]);
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
    init_timer_irq();
    init_buttonirq();
        // init PWM/sound stuff
        // init ADC
    init_adc_program();
    
    updateCPS();
    
    // main loop
        // update cookies (maybe put on interrupt?)
            // separate func for click?
        // update display
    for (;;)
    {
        print_display();
        // cookies++;
        sleep_ms(1000);
    }
}