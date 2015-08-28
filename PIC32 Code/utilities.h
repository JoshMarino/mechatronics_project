#ifndef util__H__
#define util__H__

extern unsigned int StoringData, StoringDataControl;

typedef enum { IDLE, PWM, TUNE, TRACK } States;          // the various states


typedef struct {                        // structure used for storing control loop data
  int reference;                        // you will modify this to contain the values
  int sensor;                           // and types that you want to send, the current
  float control;                        // members are for demonstration purposes
} control_data;


States util_state_get();               // get the current state


void util_state_set(States s);         // atomically set the current state. 

void util_buffer_init(unsigned int n, int control);  // initialize the buffer to record n samples

void util_buffer_write(control_data d); // write data to the current buffer position, if not full

void util_buffer_output();              // wait for the buffer to be full and output it to the client

#endif
