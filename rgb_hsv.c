#include "rgb_hsv.h"

#include <math.h>
#include <stdint.h>

/**
 * Convert an HSV value to RGB
 * This code is adapted from the equations on the HSV wikipedia article (https://en.wikipedia.org/wiki/HSL_and_HSV)
 * and the code found here: https://www.ruinelli.ch/rgb-to-hsv
 * 
 * @param[in] hsv A pointer to a 3-element array, representing HSV values
 * @param[out] rgb A pointer to a 3-element array, which will be filled with
 * the RGB representation of the given HSV values
 */
void hsv_to_rgb(const float* hsv, float* rgb)
{
    float hue = hsv[0];
    float saturation = hsv[1];
    float value = hsv[2];
    
    if ((hue >= 360.0) || (hue < 0.0)) {
        hue = 0.0;
    }
    
    float hue_sector = hue / 60.0;
    int hue_sector_whole = (int)floor(hue_sector);
    float hue_sector_frac = hue_sector - hue_sector_whole;
    
    float part1 = value * (1.0 - saturation);
    float part2 = value * (1.0 - (saturation * hue_sector_frac));
    float part3 = value * (1.0 - (saturation * (1.0 - hue_sector_frac)));
    
    switch (hue_sector_whole) {
        case 0:
            rgb[0] = value;
            rgb[1] = part3;
            rgb[2] = part1;
            break;
            
        case 1:
            rgb[0] = part2;
            rgb[1] = value;
            rgb[2] = part1;
            break;
            
        case 2:
            rgb[0] = part1;
            rgb[1] = value;
            rgb[2] = part3;
            break;
            
        case 3:
            rgb[0] = part1;
            rgb[1] = part2;
            rgb[2] = value;
            break;
            
        case 4:
            rgb[0] = part3;
            rgb[1] = part1;
            rgb[2] = value;
            break;
            
        case 5:
            rgb[0] = value;
            rgb[1] = part1;
            rgb[2] = part2;
            break;
            
        default:
            rgb[0] = 0.0;
            rgb[1] = 0.0;
            rgb[2] = 0.0;
            break;
    }
}

/**
 * Scale RGB values [0,1] to PWM output values (10-bit)
 * 
 * @param[in] rgb A pointer to a 3-element array of RGB input values
 * @param[out] pwm_out A pointer to a 3-element array, which will be filled
 * with the RGB values properly scaled for PWM output
 */
void rgb_to_pwm_output_scale(const float* rgb, uint16_t* pwm_out)
{
    pwm_out[0] = (uint16_t)(rgb[0] * PWM_OUTPUT_MAX_SCALE);
    pwm_out[1] = (uint16_t)(rgb[1] * PWM_OUTPUT_MAX_SCALE);
    pwm_out[2] = (uint16_t)(rgb[2] * PWM_OUTPUT_MAX_SCALE);
}