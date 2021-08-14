#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "button.h"

ISR(PORTB_PORT_vect) {
    /* This is only used to wake the MCU, so we do nothing here. */
    VPORTB.INTFLAGS |= PORT_INT2_bm;
}


void init_button() {
    while (RTC.STATUS > 0) {}
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
    while (RTC.PITSTATUS > 0) {}
    RTC.PITCTRLA = RTC_PERIOD_CYC256_gc | RTC_PITEN_bm;
    RTC.PITINTCTRL = RTC_PITEN_bm;
    RTC.CTRLA = RTC_PRESCALER_DIV1_gc | RTC_RTCEN_bm;
    
    button_released = 0;
}

volatile static bool last_button_state = false;
volatile static bool debounced_button_state = false;
volatile static uint8_t debounce_counter = 0;
volatile static uint16_t held_time = 0;

ISR(RTC_PIT_vect) {
    bool last_debounced_button_state = debounced_button_state;
    /* debouncing */
    bool button_state = ~VPORTB.IN & PIN2_bm;
    if (button_state == last_button_state) {
        if (debounce_counter < DEBOUNCE_THRESHOLD) {
            debounce_counter++;
        } else {
            debounced_button_state = button_state;
        }
    } else {
        debounce_counter = 0;
    }
    last_button_state = button_state;
    

    /* release "flag" */
    if (last_debounced_button_state && !debounced_button_state) {
        button_released = held_time;
    }
    
    /* hold time counting */
    if (debounced_button_state) {
        held_time++;
    } else {
        held_time = 0;
    }
    
    last_debounced_button_state = debounced_button_state;
    
    RTC.PITINTFLAGS |= RTC_PI_bm; // clear interrupt flag
}
