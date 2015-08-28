#include "NU32.h"         // config bits, constants, funcs for startup and UART
#include "isense.h"
#include "utilities.h"
#include "motor.h"

#define EINTMAX 500
// 20 kHz PWM - Timer3 - OC1 (D0)
// 5 kHz current control loop - Timer2
// 200 Hz position control loop

// AN10/RB10 digital output

static volatile float Kp = 0, Ki = 0;
static volatile int Eint = 0;
static volatile int flag = 0, counter = 0, temp;
static volatile int reference_milliamps;


// Current control loop ISR - 5 kHz
void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Controller() {

	char buffer[200];

	// state querying code
	switch(util_state_get()) {
		case IDLE:
			//set PWM to 0
			motor_pwm_set(0);
			//reset Eint to 0
			Eint = 0;
			break;
		case PWM:
			// set PWM to user specified value
			break;
		case TUNE:
			if(StoringData) {
				int E = 0, PWM = 0, temp1;
				control_data d;

				d.reference = 200*flag;
				d.sensor = isense_amps();
				E = d.reference - d.sensor;
				Eint = Eint + E;
				// integral anti-windup
				if(Eint > EINTMAX) {Eint = EINTMAX;}
				if(Eint < -EINTMAX) {Eint = -EINTMAX;}
				//
				d.control = Kp*((float) E) + Ki*((float) Eint);

				PWM = ((int) d.control);
				if(d.control < -100) {PWM = -100;}
				if(d.control > 100) {PWM = 100;}

				temp1 = motor_pwm_set(PWM);


				if(StoringDataControl==1) {
					util_buffer_write(d);
				}

				counter++;
				if(counter==19)
				{
					counter = 0;

					if(flag == 0) {temp = 1;}
					if(flag == 1) {	temp = 0;}

					flag = temp;
				}
			}
			else {
				counter = 0;
				Eint = 0;
			}
			break;
		case TRACK:
			if(StoringData) {
				int E = 0, PWM = 0, temp1;
				control_data d;

				d.reference = reference_milliamps;
				d.sensor = isense_amps();
				E = d.reference - d.sensor;
				Eint = Eint + E;
				// integral anti-windup
				if(Eint > EINTMAX) {Eint = EINTMAX;}
				if(Eint < -EINTMAX) {Eint = -EINTMAX;}
				//
				d.control = Kp*((float) E) + Ki*((float) Eint);

				PWM = ((int) d.control);
				if(d.control < -100) {PWM = -100;}
				if(d.control > 100) {PWM = 100;}

				temp1 = motor_pwm_set(PWM);

				if(StoringDataControl==1) {
					util_buffer_write(d);
				}

			}
			else {
				Eint = 0;
				reference_milliamps = 0;
			}
			break;
		default:
			//error, unknown state
			NU32_LED2 = 0;
	}

	IFS0bits.T2IF = 0;											// clear interrupt flag

}



// Initializes the module and the peripherals it uses
void motor_init(void) {

	// initializing the current control loop ISR - 5 kHz - Timer2
	__builtin_disable_interrupts(); 	// INT step 2: disable interrupts at CPU

																		// INT step 3: 	setup TMR2 to call ISR at frequency of 5 kHz
	PR2 = 15999; 											// 							set period register to 16,000
	TMR2 = 0;													// 							initialize count to 0
	T2CONbits.TCKPS = 0; 							// 							set prescaler to 1:1
	T2CONbits.ON = 1; 								// 							turn on Timer2
	IPC2bits.T2IP = 5; 								// INT step 4: 	priority 5
	IPC2bits.T2IS = 0; 								// 							subpriority 0
	IFS0bits.T2IF = 0; 								// INT step 5: 	clear interrupt flag
	IEC0bits.T2IE = 1; 								// INT step 6: 	enable interrupt
	__builtin_enable_interrupts(); 		// INT step 7: 	enable interrupts at CPU


	// initializing the PWM output - 20 kHz - Timer3
	OC1CONbits.OCTSEL = 1;   // Select Timer3 for comparison

  T3CONbits.TCKPS = 0;     // Timer3 prescaler N=1 (1:1)
  PR3 = 3999;              // period = (PR3+1) * N * 12.5 ns = 20 kHz
  TMR3 = 0;                // initial TMR3 count is 0

  OC1CONbits.OCM = 0b110;  // PWM mode with no fault pin; other OC1CON bits are defaults

  OC1RS = 0;           		 // duty cycle = OC1RS/(PR3+1) = 0%
  OC1R = 0;            		 // initialize before turning OC1 on; afterward it is read-only

  T3CONbits.ON = 1;        // turn on Timer3
  OC1CONbits.ON = 1;       // turn on OC1


	// initializing the direction pin output - AN10/RB10 digital output
	AD1PCFGbits.PCFG10 = 1;   // pin AN10 is digital pin
	TRISBbits.TRISB10 = 0;		// RB10 is an output pin
	LATBbits.LATB10 = 1;

}



// Specifies the user's PWM value
int motor_pwm_set(int duty_percent) {

	OC1RS = ((int) ((((double) abs(duty_percent))/((double) 100))*((double) 4000)));

	if (duty_percent < 0) {
		LATBbits.LATB10 = 0;
	}

	if (duty_percent >= 0) {
		LATBbits.LATB10 = 1;
	}

	//if (duty_percent != 0) {
	//	util_state_set(PWM);
	//}

	return OC1RS;
}


// Specifies the motor current reference, in milliamps
void motor_amps_set(int milliamps) {

	reference_milliamps = milliamps;

}


// Read the gains from the client
void motor_gains_read(float gain1, float gain2) {

	Kp = gain1;
	Ki = gain2;

}



// Send the motor gains to the client
void motor_gains_write() {

	char buffer[200];

	sprintf(buffer,"\%f \r\n", Kp);
	NU32_WriteUART1(buffer);
	sprintf(buffer,"\%f \r\n", Ki);
	NU32_WriteUART1(buffer);

}                
