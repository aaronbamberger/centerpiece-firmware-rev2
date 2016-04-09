#include "util.h"

#include <pic16f1788.h>

/**
 * Enables processor interrupts
 */
void enable_global_interrupts()
{
    INTCONbits.GIE = 1;
}

/**
 * Disables processor interrupts
 */
void disable_global_interrupts()
{
    INTCONbits.GIE = 0;
}

/**
 * Returns the lesser of the two arguments
 * 
 * @param[in] first First number to compare
 * @param[in] second Second number to compare
 * @return The lesser of first and second
 */
int min(int first, int second)
{
    if (first < second) {
        return first;
    } else {
        return second;
    }
}

/**
 * Returns the greater of the two arguments
 * 
 * @param[in] first First number to compare
 * @param[in] second Second number to compare
 * @return The greater of first and second
 */
int max(int first, int second)
{
    if (first > second) {
        return first;
    } else {
        return second;
    }
}