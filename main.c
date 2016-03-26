/* 
 * File:   main.c
 * Author: Aaron
 *
 * Created on March 25, 2016, 4:24 PM
 */

#include "proc_config.h"
#include "rgb_hsv.h"
#include "gpio.h"
#include "timers.h"
#include "adc.h"
#include "pwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <xc.h>

#include <pic16f1788.h>

#define BREATHE_STEP_AMOUNT 0.001
#define SAT_MIN 0.85
#define SAT_MAX 1.0
#define VAL_MIN 0.5
#define VAL_MAX 1.0
#define HUE_BREATHE_STEP 0.05;
#define HUE_BREATHE_ADJ_MAX 10.0
#define HUE_BREATHE_ADJ_MIN -10.0
#define BREATHE_DELAY 3
#define HUE_BREATHE_DELAY 5

volatile bool update_color_flag = false;
float hsv[] = {0.0, 1.0, 1.0};
float rgb_out[] = {0.0, 0.0, 0.0};
uint16_t pwm_out[] = {0, 0, 0};
volatile uint16_t adc_result;
volatile bool adc_result_updated = false;
volatile float hue_breathe_adjustment = 0.0;

typedef enum {
    BREATHE_STATE_SAT_DOWN,
    BREATHE_STATE_SAT_UP,
    BREATHE_STATE_VAL_DOWN,
    BREATHE_STATE_VAL_UP
} BreatheState;

typedef enum {
    HUE_BREATHE_UP,
    HUE_BREATHE_DOWN
} HueBreatheDirection;

/*
 * 
 */
int main(int argc, char** argv) {

    // Change system clock to 32MHz
    // Select 8MHz internal oscillator (multiplied to 32MHz by 4x PLL)
    OSCCONbits.IRCF = 0xE;
    
    init_gpio();
    init_timers();
    init_adc();
    init_pwm();
    
    // Enable peripheral interrupts
    INTCONbits.PEIE = 1;
    // Enable global interrupts
    INTCONbits.GIE = 1;
    
    // Start an ADC conversion to start the scan of pots
    start_adc_conversion();
    
    while (1) {
        // Have the LED driver output enable mirror the mode switch
        LATCbits.LATC0 = PORTBbits.RB2;
        
        if (adc_result_updated) {
            float new_hue = ((float)adc_result / 4096.0) * 360.0;
            hsv[0] = new_hue + hue_breathe_adjustment;
            
            hsv_to_rgb(hsv, rgb_out);
            rgb_to_pwm_output_scale(rgb_out, pwm_out);
            
            CCPR1L = (pwm_out[0] >> 2);
            CCP1CONbits.DC1B = pwm_out[0] & 0x03;
            CCPR2L = (pwm_out[1] >> 2);
            CCP2CONbits.DC2B = pwm_out[1] & 0x03;
            CCPR3L = (pwm_out[2] >> 2);
            CCP3CONbits.DC3B = pwm_out[2] & 0x03;
            
            adc_result_updated = false;
        }
    }
    
    return (EXIT_SUCCESS);
}

volatile bool led_state = false;
volatile unsigned int heartbeat_led_counter = 0;
volatile unsigned int breathe_counter = 0;
volatile unsigned int hue_breathe_counter = 0;
volatile BreatheState current_breathe_state = BREATHE_STATE_SAT_DOWN;
volatile HueBreatheDirection current_hue_breathe_direction = HUE_BREATHE_UP;

/*
 * ISR
 */
void interrupt main_isr(void)
{
    // Poll interrupt flag registers
    if (INTCONbits.TMR0IF) { // Timer 0 overflow flag
        
        // Toggle user LED ever 1s)
        if (++heartbeat_led_counter >= 122) {
            // Toggle the state of the user LED
            if (led_state) {
                LATBbits.LATB1 = 1;
                led_state = false;
            } else {
                LATBbits.LATB1 = 0;
                led_state = true;
            }
            
            heartbeat_led_counter = 0;
        }
        
        if (++hue_breathe_counter >= HUE_BREATHE_DELAY) {
            switch (current_hue_breathe_direction) {
            case HUE_BREATHE_UP:
                hue_breathe_adjustment += HUE_BREATHE_STEP;
                if (hue_breathe_adjustment >= HUE_BREATHE_ADJ_MAX) {
                    current_hue_breathe_direction = HUE_BREATHE_DOWN;
                }
                break;

            case HUE_BREATHE_DOWN:
                hue_breathe_adjustment -= HUE_BREATHE_STEP;
                if (hue_breathe_adjustment <= HUE_BREATHE_ADJ_MIN) {
                    current_hue_breathe_direction = HUE_BREATHE_UP;
                }
                break;
            }
            
            hue_breathe_counter = 0;
        }
        
        if (++breathe_counter >= BREATHE_DELAY) {
            switch (current_breathe_state) {
            case BREATHE_STATE_SAT_DOWN:
                hsv[1] -= BREATHE_STEP_AMOUNT;
                if (hsv[1] <= SAT_MIN) {
                    hsv[1] = SAT_MIN;
                    current_breathe_state = BREATHE_STATE_SAT_UP;
                }
                break;

            case BREATHE_STATE_SAT_UP:
                hsv[1] += BREATHE_STEP_AMOUNT;
                if (hsv[1] >= SAT_MAX) {
                    hsv[1] = SAT_MAX;
                    current_breathe_state = BREATHE_STATE_VAL_DOWN;
                }
                break;

            case BREATHE_STATE_VAL_DOWN:
                hsv[2] -= BREATHE_STEP_AMOUNT;
                if (hsv[2] <= VAL_MIN) {
                    hsv[2] = VAL_MIN;
                    current_breathe_state = BREATHE_STATE_VAL_UP;
                }
                break;

            case BREATHE_STATE_VAL_UP:
                hsv[2] += BREATHE_STEP_AMOUNT;
                if (hsv[2] >= VAL_MAX) {
                    hsv[2] = VAL_MAX;
                    current_breathe_state = BREATHE_STATE_SAT_DOWN;
                }
                break;
            }
            breathe_counter = 0;
        }
        
        // Reset the interrupt flag
        INTCONbits.TMR0IF = 0;
    }
    
    if (PIR1bits.ADIF) { // ADC conversion complete flag
        // Read the adc
        adc_result = (ADRESH << 8) | ADRESL;
        
        // Clamp ADC readings to 0
        if (adc_result < 0) {
            adc_result = 0;
        }
        
        // Set a flag indicating which adc was updated
        adc_result_updated = true;
        
        // Reset the interrupt flag
        PIR1bits.ADIF = 0;
        
        // Schedule a new ADC conversion
        start_adc_conversion();
    }
    
    return;
}
