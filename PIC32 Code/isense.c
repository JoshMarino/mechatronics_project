#include "NU32.h"         // config bits, constants, funcs for startup and UART
#include "isense.h"

#define SAMPLE_TIME 10 			// 10 core timer ticks = 250 ns



// Initialize the current sensor
void isense_init() {

	AD1PCFGbits.PCFG14 = 0; 	// AN14 is an adc pin (shared with B14)
	AD1CON3bits.ADCS = 2; 		// ADC clock period is Tad = 2*(ADCS+1)*Tpb = 2*3*12.5ns = 75ns
	AD1CON1bits.ADON = 1; 		// turn on A/D converter

}



// Read the current sensor, in ADC ticks
short isense_ticks() {

	short adcval_average = 0, adcval_sum = 0;
	unsigned int elapsed = 0, finish_time = 0;
	char i = 0;

	while (i<5) {

		// set the core timer count to zero
		_CP0_SET_COUNT(0);

		// start sampling, connect pin AN14 to MUXA for sampling
		AD1CHSbits.CH0SA = 14;
		AD1CON1bits.SAMP = 1;

		elapsed = _CP0_GET_COUNT();
		finish_time = elapsed + SAMPLE_TIME;

		// sample for more than 250 ns
		while (_CP0_GET_COUNT() < finish_time) {
			;
		}

		// stop sampling and start converting
		AD1CON1bits.SAMP = 0;

		// wait for the conversion process to finish
		while (!AD1CON1bits.DONE) {
			;
		}

		// read the buffer with the result, for averaging effect
		adcval_sum = adcval_sum +  ADC1BUF0;

		i = i + 1;
	}

	adcval_average = adcval_sum / ((short) 5);

	return adcval_average;

	// set the core timer count to zero
	_CP0_SET_COUNT(0);
}



// Read the current sensor, in mA
int isense_amps() {

	int current_mA;
	short adc_val;

	adc_val = isense_ticks();
	current_mA = 1.9835*((int) adc_val) - 1001.91852;

	return current_mA;
}
