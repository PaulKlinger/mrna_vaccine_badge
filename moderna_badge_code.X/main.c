#include "config.h"


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
    button_pressed = 1;
    VPORTB.INTFLAGS |= PORT_INT2_bm;
}

void switch_off_leds() {
    VPORTA.OUT = (PIN6_bm | PIN7_bm);
    VPORTB.OUT = (PIN6_bm | PIN7_bm);
}

void go_to_sleep(){
    while (~PORTB.IN & PIN2_bm) {}
    _delay_ms(50);
    switch_off_leds();
    _delay_ms(100);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
    
    while (~PORTB.IN & PIN2_bm) {}
    _delay_ms(50);
    button_pressed = 0;
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
    sei();
    
    while (1) {
        go_to_sleep();
    
        for (uint32_t i=0; i< moderna_sequence.n_bases; i++) {
            show_base(read_base(&moderna_sequence, i));
            _delay_ms(LIGHT_DURATION_MS);
            switch_off_leds();
            if (button_pressed) {
                button_pressed = 0;
                break;
            }
            _delay_ms(INTERVAL_DURATION_MS);
        }
    }
}
