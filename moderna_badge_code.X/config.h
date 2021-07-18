/* 
 * File:   config.h
 * Author: kling
 *
 * Created on 03 July 2021, 18:21
 */

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif


    #define F_CPU 1000000
    
    #define LIGHT_DURATION_MS 100
    #define INTERVAL_DURATION_MS 50
    
    enum sequence_setting {
        MODERNA = 36,
        PFIZER = 42
    };
    
    struct config {
        enum sequence_setting sequence;
    };

    void save_config(struct config);

    struct config load_config();
    
#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

