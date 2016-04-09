/* 
 * File:   util.h
 * Author: Aaron
 *
 * Created on March 26, 2016, 6:17 PM
 */

#ifndef UTIL_H
#define	UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

void enable_global_interrupts();
void disable_global_interrupts();
int min(int first, int second);
int max(int first, int second);

#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

