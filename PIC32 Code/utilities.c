#include "NU32.h"         // config bits, constants, funcs for startup and UART
#include "utilities.h"


static volatile States state = IDLE;
static volatile unsigned int buffermax = 0;
unsigned int StoringData, StoringDataControl;
static volatile char data_array = 0;
static volatile unsigned int data_points;

static int d1[1000], d2[1000];
static float d3[1000];


// Get the current state
States util_state_get() {

	return state;

}


// Atomically set the current state
void util_state_set(States s) {

	state = s;

}


// Initialize the buffer to record n samples
void util_buffer_init(unsigned int n, int control) {

	// set buffermax to n sample
	buffermax = n;

	// allow data to be written, define matrix for writing data, reset data_points to 0
	StoringData = 1;
	data_points = 0;

	// test to see if should implement current control loop or position control loop
	if(control==0) {StoringDataControl = 1;}  // current control loop
	if(control==1) {StoringDataControl = 2;}	// position control loop

}


// Write data to the current buffer position, if not full
void util_buffer_write(control_data d) {

	char buffer[200];

	if(data_points<buffermax) 
	{		
		d1[data_points] = d.reference;
		d2[data_points] = d.sensor;
		d3[data_points] = d.control;

		data_points++;
	}
	else 
	{
		StoringData = 0;
		StoringDataControl = 0;
	}	

}


// Wait for the buffer to be full and output it to the client
void util_buffer_output() {

	char buffer[200];

	while(StoringData) {
		sprintf(buffer,"\%d \r\n", 0); //won't exit the loop for some strange reason if just leave a semi-colon
	}



	// Output number of samples
	sprintf(buffer,"\%d \r\n", buffermax);
	NU32_WriteUART1(buffer);

	// Output each sample, one sample per line
	unsigned int i = 0;
	while(i<buffermax) {
		
		sprintf(buffer,"\%d %d %f \r\n", d1[i], d2[i], d3[i]);
		NU32_WriteUART1(buffer);		
  
		i++;
	}

}
