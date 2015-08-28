#ifndef ISENSE_H__
#define ISENSE_H__


void isense_init();     // initialize the current sensor

short isense_ticks();   // read the current sensor, in ADC ticks

int isense_amps();      // read the current sensor, in mA


#endif
