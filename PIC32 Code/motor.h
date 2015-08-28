#ifndef MOTOR__H__
#define MOTOR__H__


void motor_init(void);                    // Initializes the module and the peripherals it uses

int motor_pwm_set(int duty_percent);    	// Specifies the user's PWM value 

void motor_milliamps_set(int milliamps);            // specifies the motor current reference, in milliamps

void motor_gains_read(float gain1, float gain2);      // read the gains from the client

void motor_gains_write();                // sends the motor gains to the client


#endif
