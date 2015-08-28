#include "NU32.h"         // config bits, constants, funcs for startup and UART
#include "position.h"
#include "encoder.h"
#include "utilities.h"
#include "motor.h"

#define BUF_SIZE 200
#define EINTMAX 500

// 20 kHz PWM - Timer3 - OC1 (D0)
// 5 kHz current control loop - Timer2
// 200 Hz position control loop - Timer4

static volatile float Kp = 0, Ki = 0, Kd = 0;
static volatile int traj[2000];
static volatile int Eint = 0, Eprev = 0, Eder = 0;
static volatile int counter = 0;
//static volatile int reference_milliamps;


// Position control loop ISR - 200 Hz
void __ISR(_TIMER_4_VECTOR, IPL4SOFT) PositionController() {

	// state querying code
	switch(util_state_get()) {
		case IDLE:
			break;
		case PWM:
			break;
		case TUNE:
			break;
		case TRACK:
			if(StoringData) {
				int E = 0, desired_current_mA = 0;
				control_data d;

				d.reference = traj[counter]/10;
				d.sensor = encoder_angle();
				E = d.reference - d.sensor;
				Eint = Eint + E;
				// integral anti-windup
				if(Eint > EINTMAX) {Eint = EINTMAX;}
				if(Eint < -EINTMAX) {Eint = -EINTMAX;}
				//
				Eder = E - Eprev;
				d.control = Kp*((float) E) + Ki*((float) Eint) + Kd*((float) Eder);

				// set desired current to follow from position control --> current controller
				desired_current_mA = ((int) d.control);
				if(d.control < -300) {desired_current_mA = -300;}
				if(d.control > 300) {desired_current_mA = 300;}
				motor_amps_set(desired_current_mA);

				if(StoringDataControl==2) {
					util_buffer_write(d);
				}

				Eprev = E;
				counter++;
			}
			else {
				Eint = 0;
				Eprev = 0;
				counter = 0;
			}
			break;
		default:
			//error, unknown state
			NU32_LED2 = 0;
	}

	IFS0bits.T4IF = 0;											// clear interrupt flag

}



// Initializes the module and the peripherals it uses
void position_init(void) {

	// initializing the position control loop ISR - 200 Hz - Timer4
	__builtin_disable_interrupts(); 	// INT step 2: disable interrupts at CPU

																		// INT step 3: 	setup TMR4 to call ISR at frequency of 200 Hz
	PR4 = 6249; 											// 							set period register to 2,500
	TMR4 = 0;													// 							initialize count to 0
	T4CONbits.TCKPS = 0b110;					// 							set prescaler to 1:64
	T4CONbits.ON = 1; 								// 							turn on Timer4
	IPC4bits.T4IP = 4; 								// INT step 4: 	priority 4
	IPC4bits.T4IS = 0; 								// 							subpriority 0
	IFS0bits.T4IF = 0; 								// INT step 5: 	clear interrupt flag
	IEC0bits.T4IE = 1; 								// INT step 6: 	enable interrupt
	__builtin_enable_interrupts(); 		// INT step 7: 	enable interrupts at CPU

}


// Load a trajectory from the client and return its length
unsigned int position_load() {

	char buffer[BUF_SIZE];

	unsigned int traj_len;
	NU32_ReadUART1(buffer,BUF_SIZE);
	sscanf(buffer, "%d", &traj_len);

	__builtin_disable_interrupts();

	int i=0, angle=0;
	while(i<traj_len) {
	
		NU32_ReadUART1(buffer,BUF_SIZE);
		sscanf(buffer, "%d", &angle);
	
		traj[i] = angle;
	
		i++;
	}

	__builtin_enable_interrupts(); 

	return traj_len;
}

// Have PIC32 hold final trajectory position for N more samples
void hold_final_traj_pos(int additional_samples, int traj_len) {

	int i=0, j=0, k = traj_len - 1;
	while(i<additional_samples) {

		j = traj_len + i;
		traj[j] = traj[k];
	
		i++;
	}

}


// Read the position gains from the client
void position_gains_read(float gain1, float gain2, float gain3) {

	Kp = gain1;
	Ki = gain2;
	Kd = gain3;

}



// Send the position gains to the client
void position_gains_send() {

	char buffer[200];

	sprintf(buffer,"\%f \r\n", Kp);
	NU32_WriteUART1(buffer);
	sprintf(buffer,"\%f \r\n", Ki);
	NU32_WriteUART1(buffer);
	sprintf(buffer,"\%f \r\n", Kd);
	NU32_WriteUART1(buffer);

}                
