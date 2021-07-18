#include "config.h"

#include <stdbool.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "button.h"
#include "sequence.h"
#include "moderna_sequence.h"
#include "pfizer_sequence.h"


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

void switch_off_leds() {
    VPORTA.OUT = (PIN6_bm | PIN7_bm);
    VPORTB.OUT = (PIN6_bm | PIN7_bm);
}

void go_to_sleep(){
    while (~PORTB.IN & PIN2_bm) {}
    _delay_ms(50);
    switch_off_leds();
    /* disable periodic interrupt, so it doesn't wake up MCU */
    RTC.PITCTRLA &= ~RTC_PITEN_bm;
    _delay_ms(100);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
    while (~PORTB.IN & PIN2_bm) {}
    _delay_ms(50);
    button_released = 0;
    /* enable periodic interrupt again */
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
    
    init_button();
    sei();
    
    go_to_sleep();
    
    const struct sequence *current_sequence = &moderna_sequence;
    while (1) {
        for (uint32_t i=0; i< current_sequence->n_bases; i++) {
            show_base(read_base(current_sequence, i));
            _delay_ms(LIGHT_DURATION_MS);
            switch_off_leds();
            if (button_released) {
                break;
            }
            _delay_ms(INTERVAL_DURATION_MS);
        }
        
        if (button_released > LONG_PRESS_LENGTH) {
            button_released = 0;
            // long press, switch between sequences
            while (1) {
                if (button_released > LONG_PRESS_LENGTH) {
                    button_released = 0;
                    break; // start displaying sequence
                } else if (button_released) {
                    button_released = 0;
                    if (current_sequence == &moderna_sequence) {
                        current_sequence = &pfizer_sequence;
                    } else if (current_sequence == &pfizer_sequence) {
                        current_sequence = &moderna_sequence;
                    }
                }
                if (current_sequence == &moderna_sequence) {
                    VPORTB.OUT = (PIN6_bm | PIN7_bm);
                    VPORTA.OUT = (uint8_t) ~(PIN7_bm | PIN6_bm);
                } else if (current_sequence == &pfizer_sequence) {
                    VPORTA.OUT = (PIN6_bm | PIN7_bm);
                    VPORTB.OUT = (uint8_t) ~(PIN7_bm | PIN6_bm);
                }
            }
        } else {
            button_released = 0;
            go_to_sleep();
        }
    }
}
