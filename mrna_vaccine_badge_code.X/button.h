/* 
 * File:   button.h
 * Author: kling
 *
 * Created on 18 July 2021, 22:14
 */

#ifndef BUTTON_H
#define	BUTTON_H

#ifdef	__cplusplus
extern "C" {
#endif

    #define DEBOUNCE_THRESHOLD 10 // = 80ms
    #define LONG_PRESS_LENGTH 125 // = 1s

    /* non-zero if button was released (must be cleared in main loop)
     * value / 8 = time button was held down in ms
     */
    volatile uint16_t button_released;
    
    void init_button();

#ifdef	__cplusplus
}
#endif

#endif	/* BUTTON_H */

