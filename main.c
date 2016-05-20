/* 
 * File:   main.c
 * Author: Aaron
 *
 * Created on March 25, 2016, 4:24 PM
 */

#define _XTAL_FREQ 16000000
#define INTERRUPTS_PER_SEC 122
#define SECS_PER_MIN 60
#define MINS_PER_HOUR 60

#define ENABLE_HEARTBEAT_LED 0
#define ENABLE_HUE_BREATHING 0
#define ENABLE_SAT_BREATHING 0
#define ENABLE_GYRO_MOTION_LED 0

#define NO_IMU_MODE 1

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
#define HUE_BREATHE_STEP 0.01;
#define HUE_BREATHE_ADJ_MAX 5.0
#define HUE_BREATHE_ADJ_MIN -5.0
#define BREATHE_DELAY 3
#define HUE_BREATHE_DELAY 1
#define GYRO_MOTION_THRESHOLD 1000
#define ROTATION_HUE_ADJUST 0.000005

#if NO_IMU_MODE
#define STARTING_HUE 320.0
#define HUE_CHANGE_DELAY 150
#define HUE_CHANGE_AMOUNT 0.005
#else
#define STARTING_HUE 0.0
#endif

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
float current_hue = STARTING_HUE;
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
volatile bool color_update_flag = true;
volatile uint16_t uptime_ticks;
volatile uint16_t uptime_seconds;
volatile uint16_t uptime_minutes;
volatile uint16_t uptime_hours;

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
    
    // Enable the LED driver
    PWM_ENABLE = 1;
    
#if NO_IMU_MODE
    int hue_change_counter = 0;
    bool start_hue_change = false;
#endif
    
    while (1) {
#if !NO_IMU_MODE
        if (new_gyro_samp_ready) {
            if (last_gyro_reading > GYRO_MOTION_THRESHOLD) {
                float hue_adjustment = ((float)last_gyro_reading - (float)GYRO_MOTION_THRESHOLD) * (float)ROTATION_HUE_ADJUST;
                current_hue += hue_adjustment;
                if (current_hue >= 360.0) {
                    current_hue = 0.0;
                }
            } else if (last_gyro_reading < -GYRO_MOTION_THRESHOLD) {
                float hue_adjustment = ((float)last_gyro_reading + (float)GYRO_MOTION_THRESHOLD) * (float)ROTATION_HUE_ADJUST;
                current_hue += hue_adjustment;
                if (current_hue < 0.0) {
                    current_hue = 360.0;
                }
            }
            
#if ENABLE_GYRO_MOTION_LED
            if ((last_gyro_reading > GYRO_MOTION_THRESHOLD) || (last_gyro_reading < -GYRO_MOTION_THRESHOLD)) {
                USER_LED = 1;
            } else {
                USER_LED = 0;
            }
#endif
            
            new_gyro_samp_ready = false;
        }
#else
        if (uptime_hours >= 3) {
            start_hue_change = true;
        }
        
        if (start_hue_change) {
            if(++hue_change_counter >= HUE_CHANGE_DELAY) {
                hue_change_counter = 0;
                
                current_hue += HUE_CHANGE_AMOUNT;
                if (current_hue >= 360.0) {
                    current_hue = 0.0;
                }
            }
        }
#endif
        
        if (color_update_flag) {
            //float new_hue = ((float)adc_result / 4096.0) * 360.0;
            hsv[0] = current_hue + hue_breathe_adjustment;
            
            hsv_to_rgb(hsv, rgb_out);
            rgb_to_pwm_output_scale(rgb_out, pwm_out);
            
            CCPR1L = (pwm_out[0] >> 2);
            CCP1CONbits.DC1B = pwm_out[0] & 0x03;
            CCPR2L = (pwm_out[1] >> 2);
            CCP2CONbits.DC2B = pwm_out[1] & 0x03;
            CCPR3L = (pwm_out[2] >> 2);
            CCP3CONbits.DC3B = pwm_out[2] & 0x03;
            
            color_update_flag = false;
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
        // Manage the uptime counters
        if (++uptime_ticks >= INTERRUPTS_PER_SEC) {
            uptime_ticks = 0;
            uptime_seconds++;
            if (uptime_seconds >= SECS_PER_MIN) {
                uptime_seconds = 0;
                uptime_minutes++;
                if (uptime_minutes >= MINS_PER_HOUR) {
                    uptime_minutes = 0;
                    uptime_hours++;
                }
            }
        }
        
#if ENABLE_HEARTBEAT_LED
        // Toggle user LED ever 1s
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
#endif

#if ENABLE_HUE_BREATHING
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
#endif
        
#if ENABLE_SAT_BREATHING
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
#endif
        
        
        
        color_update_flag = true;
        
        // TODO: Just for testing until IMU is working
        /*
        if (hue_test_counter++ > 2) {
            adc_result++;
            if (adc_result > 4096) {
                adc_result = 0;
            }
            hue_test_counter = 0;
            
            adc_result_updated = true;
        }
        */
        
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
    
    return;
}
