/* 
 * File:   rgb_hsv.h
 * Author: Aaron
 *
 * Created on March 25, 2016, 4:33 PM
 */

#ifndef RGB_HSV_H
#define	RGB_HSV_H

#include <stdint.h>

#define PWM_OUTPUT_MAX_SCALE 1023.0

#ifdef	__cplusplus
extern "C" {
#endif

void hsv_to_rgb(const float* hsv, float* rgb);
void rgb_to_pwm_output_scale(const float* rgb, uint16_t* pwm_out);

#ifdef	__cplusplus
}
#endif

#endif	/* RGB_HSV_H */

