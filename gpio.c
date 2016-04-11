#include "gpio.h"

#include <pic16f1788.h>

void init_gpio()
{
    // Set all gpio to digital
    ANSELA = 0;
    ANSELB = 0;
    
    // Set all gpios to 0
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    
    // Init PWM enable
    PWM_ENABLE_DIRECTION = 0; // Output
    
    // Init User LED Gpio
    USER_LED_DIRECTION = 0; // Output
}
