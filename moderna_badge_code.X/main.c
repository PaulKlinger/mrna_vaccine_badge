#include "config.h"

#include <stdbool.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#include "sequence.h"
#include "moderna_sequence.h"


FUSES = 
{
	.APPEND = 0,
	.BODCFG = ACTIVE_DIS_gc | LVL_BODLEVEL0_gc | SAMPFREQ_1KHZ_gc | SLEEP_DIS_gc,
	.BOOTEND = 0,
	.OSCCFG = FREQSEL_16MHZ_gc,
	.SYSCFG0 = CRCSRC_NOCRC_gc | RSTPINCFG_UPDI_gc,
	.SYSCFG1 = SUT_64MS_gc,
	.WDTCFG = PERIOD_OFF_gc | WINDOW_OFF_gc,
};

volatile uint8_t button_pressed = 0;

ISR(PORTB_PORT_vect) {
    /* This is only used to wake the MCU, so we do nothing here. */
    VPORTB.INTFLAGS |= PORT_INT2_bm;
}


volatile bool last_button_state = false;
volatile bool debounced_button_state = false;
#define DEBOUNCE_THRESHOLD 10 // = 80ms
volatile uint8_t debounce_counter = 0;
volatile uint16_t held_time = 0;
volatile uint16_t button_released = 0;

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

void switch_off_leds() {
    VPORTA.OUT = (PIN6_bm | PIN7_bm);
    VPORTB.OUT = (PIN6_bm | PIN7_bm);
}

void go_to_sleep(){
    while (~PORTB.IN & PIN2_bm) {}
    _delay_ms(50);
    switch_off_leds();
    RTC.PITCTRLA &= ~RTC_PITEN_bm;
    _delay_ms(100);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
    while (~PORTB.IN & PIN2_bm) {}
    _delay_ms(50);
    button_released = 0;
    RTC.PITCTRLA |= RTC_PITEN_bm;
}


void show_base(Base b) {
    switch (b) {
        case A: // green
            // cast to prevent warning caused by integer promotion to signed...
            VPORTB.OUT = (uint8_t) ~PIN7_bm;
            break;
        case C: // blue
            VPORTB.OUT = (uint8_t) ~PIN6_bm;
            break;
        case G: // yellow
            VPORTA.OUT = (uint8_t) ~PIN6_bm;
            break;
        case T: // red
            VPORTA.OUT = (uint8_t) ~PIN7_bm;
            break;
    }
};


void init_rtc_pit() {
    while (RTC.STATUS > 0) {}
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
    while (RTC.PITSTATUS > 0) {}
    RTC.PITCTRLA = RTC_PERIOD_CYC256_gc | RTC_PITEN_bm;
    RTC.PITINTCTRL = RTC_PITEN_bm;
    RTC.CTRLA = RTC_PRESCALER_DIV1_gc | RTC_RTCEN_bm;
}


int main(void) {
    
    /* Configure clock prescaler for 1MHz  */
    _PROTECTED_WRITE(
    CLKCTRL.MCLKCTRLB,
    CLKCTRL_PDIV_16X_gc /* Prescaler division: 16X */
    | CLKCTRL_PEN_bm /* Prescaler enable: enabled */
    );
    
    
    /* Enable pullups for low power consumption (20uA -> 0.1uA (afair))*/
    PORTA.PIN0CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN1CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN2CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN3CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN4CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL |= PORT_PULLUPEN_bm;
    
    PORTB.PIN0CTRL |= PORT_PULLUPEN_bm;
    PORTB.PIN1CTRL |= PORT_PULLUPEN_bm;
    PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
    PORTB.PIN3CTRL |= PORT_PULLUPEN_bm;
    PORTB.PIN4CTRL |= PORT_PULLUPEN_bm;
    PORTB.PIN5CTRL |= PORT_PULLUPEN_bm;
    PORTB.PIN6CTRL |= PORT_PULLUPEN_bm;
    PORTB.PIN7CTRL |= PORT_PULLUPEN_bm;
    
    PORTC.PIN0CTRL |= PORT_PULLUPEN_bm;
    PORTC.PIN1CTRL |= PORT_PULLUPEN_bm;
    PORTC.PIN2CTRL |= PORT_PULLUPEN_bm;
    PORTC.PIN3CTRL |= PORT_PULLUPEN_bm;
    PORTC.PIN4CTRL |= PORT_PULLUPEN_bm;
    PORTC.PIN5CTRL |= PORT_PULLUPEN_bm;
    
    
    VPORTB.DIR = (PIN6_bm | PIN7_bm);
    VPORTA.DIR = (PIN6_bm | PIN7_bm);
    
    
    /* Enable interrupt on power button
     * (this is only for wakeup)*/
    PORTB.PIN2CTRL = (PORTB.PIN2CTRL & ~PORT_ISC_gm) | PORT_ISC_BOTHEDGES_gc;
    
    
    init_rtc_pit();
    
    sei();
    
    while (1) {
        go_to_sleep();
    
        for (uint32_t i=0; i< moderna_sequence.n_bases; i++) {
            show_base(read_base(&moderna_sequence, i));
            _delay_ms(LIGHT_DURATION_MS);
            switch_off_leds();
            if (button_released > 125) {
                button_released = 0;
                // long press, switch
                for (uint8_t j=0; j<10; j++) {
                    VPORTA.OUT = (PIN6_bm | PIN7_bm);
                    VPORTB.OUT = (uint8_t) ~(PIN7_bm | PIN6_bm);
                    _delay_ms(300);
                    VPORTB.OUT = (PIN6_bm | PIN7_bm);
                    VPORTA.OUT = (uint8_t) ~(PIN7_bm | PIN6_bm);
                    _delay_ms(300);
                }
            } else if (button_released > 0) {
                button_released = 0;
                break;
            }
            _delay_ms(INTERVAL_DURATION_MS);
        }
    }
}
