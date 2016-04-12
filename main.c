/* 
 * File:   main.c
 * Author: Aaron
 *
 * Created on March 25, 2016, 4:24 PM
 */

#define _XTAL_FREQ 16000000

#include "proc_config.h"
#include "rgb_hsv.h"
#include "gpio.h"
#include "timers.h"
#include "adc.h"
#include "pwm.h"
#include "util.h"
#include "spi.h"
#include "mpu6500.h"

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
#define GYRO_MOTION_THRESHOLD 5000

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

typedef enum {
    GYRO_READ_START,
    GYRO_READ_HIGH,
    GYRO_READ_LOW
} GyroReadPart;

volatile bool update_color_flag = false;
float hsv[] = {0.0, 1.0, 1.0};
float rgb_out[] = {0.0, 0.0, 0.0};
uint16_t pwm_out[] = {0, 0, 0};
volatile uint16_t adc_result;
volatile bool adc_result_updated = false;
volatile float hue_breathe_adjustment = 0.0;
volatile bool new_gyro_samp_ready = false;
volatile GyroReadPart gyro_read_part = GYRO_READ_START;
volatile bool gyro_read_in_progress = false;
volatile int16_t last_gyro_reading = 0;

/*
 * 
 */
int main(int argc, char** argv) {

    // Change system clock to 32MHz
    // Select 8MHz internal oscillator (multiplied to 32MHz by 4x PLL)
    OSCCONbits.IRCF = 0xE;
    
    init_gpio();
    init_timers();
    //init_adc();
    init_pwm();
    init_spi();
    init_mpu();
    enable_spi_interrupt();
    
    // Configure and enable external interrupt
    OPTION_REGbits.INTEDG = 1;
    INTCONbits.INTF = 0;
    INTCONbits.INTE = 1;
    
    // Enable peripheral interrupts
    INTCONbits.PEIE = 1;
    // Enable global interrupts
    enable_global_interrupts();
    
    // Start an ADC conversion to start the scan of pots
    //start_adc_conversion();
    
    while (1) {
        // Enable the LED driver
        PWM_ENABLE = 1;
        
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
        
        if (new_gyro_samp_ready) {
            if ((last_gyro_reading > GYRO_MOTION_THRESHOLD) || (last_gyro_reading < -GYRO_MOTION_THRESHOLD)) {
                USER_LED = 1;
            } else {
                USER_LED = 0;
            }
            
            new_gyro_samp_ready = false;
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
volatile unsigned int hue_test_counter = 0;

/*
 * ISR
 */
void interrupt main_isr(void)
{
    // Poll interrupt flag registers
    
    // Timer 0 interrupt
    if (INTCONbits.TMR0IF) {
        
        // Toggle user LED ever 1s
        /*
        if (++heartbeat_led_counter >= 122) {
            // Toggle the state of the user LED
            if (led_state) {
                USER_LED = 1;
                led_state = false;
            } else {
                USER_LED = 0;
                led_state = true;
            }
            
            heartbeat_led_counter = 0;
        }
        */
        
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
        
        // TODO: Just for testing until IMU is working
        if (hue_test_counter++ > 2) {
            adc_result++;
            if (adc_result > 4096) {
                adc_result = 0;
            }
            hue_test_counter = 0;
            
            adc_result_updated = true;
        }
        
        // Reset the interrupt flag
        INTCONbits.TMR0IF = 0;
    }
    
    // External Interrupt
    if (INTCONbits.INTF) {
        if (!gyro_read_in_progress) {
            // If we're not in the middle of reading the gyro, start the read transaction
            gyro_read_in_progress = true;
            last_gyro_reading = 0;
            gyro_read_part = GYRO_READ_START;
            assert_spi_cs();
            start_read_gyro_z();
            //read_gyro_z_high();
        }
        
        // Reset the interrupt flag
        INTCONbits.INTF = 0;
    }
    
    // SPI Interrupt
    if (PIR1bits.SSP1IF) {
        switch (gyro_read_part) {
        case GYRO_READ_START:
            read_spi_byte(); // Dummy read to clear the buffer
            gyro_read_part = GYRO_READ_HIGH;
            send_spi_byte(0x00);
            break;

        case GYRO_READ_HIGH:
            last_gyro_reading |= ((int16_t)read_spi_byte() << 8);
            gyro_read_part = GYRO_READ_LOW;
            send_spi_byte(0x00);
            break;

        case GYRO_READ_LOW:
            last_gyro_reading |= read_spi_byte();
            deassert_spi_cs();
            gyro_read_in_progress = false;
            new_gyro_samp_ready = true;
            break;
        }
        
        // Reset the interrupt flag
        PIR1bits.SSP1IF = 0;
    }
    
    /*
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
    */
    
    return;
}
