/* 
 * File:   sequence.h
 * Author: kling
 *
 * Created on 03 July 2021, 20:42
 */

#ifndef SEQUENCE_H
#define	SEQUENCE_H

#ifdef	__cplusplus
extern "C" {
#endif

    #include <stdint.h>
    
    struct sequence {
        uint8_t *data;
        uint32_t n_bases;
    };
    
    typedef enum Base {
        A = 0, // green
        C = 1, // blue
        G = 2, // yellow
        T = 3  // red
    } Base;


    Base read_base(const struct sequence *seq, uint32_t i);


#ifdef	__cplusplus
}
#endif

#endif	/* SEQUENCE_H */

