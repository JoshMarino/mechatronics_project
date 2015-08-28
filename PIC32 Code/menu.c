#include "NU32.h"         // config bits, constants, funcs for startup and UART
#include "encoder.h" 			// Includer header module
#include "utilities.h" 		// Includer header module
#include "isense.h"				// Includer header module
#include "motor.h"				// Includer header module


#define BUF_SIZE 200

int main() 
{
  char buffer[BUF_SIZE];

  NU32_Startup(); 			// cache on, min flash wait, interrupts on, LED/button init, UART init

  NU32_LED1 = 1;        // turn off the LEDs
  NU32_LED2 = 1;        

  __builtin_disable_interrupts();
  // initialize other modules here
	encoder_init();										// initializes the SPI peripheral
	isense_init();										// initializes the current sensor
	motor_init();											// initializes motor peripherals
	position_init();
  __builtin_enable_interrupts();


	while(1)
  {
    NU32_ReadUART1(buffer,BUF_SIZE);	    // we expect the next character to be a menu command

    NU32_LED2 = 1;                        // clear the error led

    switch (buffer[0]) {

			// Dummy command for demonstration purposes
      case 'd':
      {
        int n = 0;
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &n);
        sprintf(buffer,"%d \r\n", n + 1);       // return the number + 1
        NU32_WriteUART1(buffer);
        break;
      }

			// Reset encoder count
			case 'p':
			{
				sprintf(buffer,"\%d \r\n", encoder_reset());
				//NU32_WriteUART1(buffer);
				break;
			}

			// Read encoder ticks
			case 'r':
			{
				sprintf(buffer,"\%d \r\n", encoder_ticks());
				NU32_WriteUART1(buffer);
				break;
			}

			// Return encoder angle
			case 'a':
			{
				sprintf(buffer,"\%d \r\n", encoder_angle());
				NU32_WriteUART1(buffer);
				break;
			}

			// Query the current state
			case 's':
			{
				sprintf(buffer,"\%d \r\n", util_state_get());
				NU32_WriteUART1(buffer);
				break;
			}

			// Read ADC and return isense_ticks
			case 'c':
			{
				sprintf(buffer,"\%d \r\n", isense_ticks());
				NU32_WriteUART1(buffer);
				break;
			}

			// Read ADC and return isense_amps
			case 't':
			{
				sprintf(buffer,"\%d \r\n", isense_amps());
				NU32_WriteUART1(buffer);
				break;
			}

			// Set PWM duty cycle and direction
			case 'z':
			{
        int n = 0;
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &n);
				util_state_set(PWM);
				sprintf(buffer,"\%d \r\n", motor_pwm_set(n));
				NU32_WriteUART1(buffer);
				//util_state_set(IDLE);
				break;
			}

			// Read PI current gains from client
			case 'l':
			{
        float Kp = 0, Ki = 0;
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kp);
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Ki);
				motor_gains_read(Kp,Ki);
				break;
			}

			// Write PI current gains to client
			case 'g':
			{
				motor_gains_write();
				break;
			}

			// Recording samples of tuning data
			case 'e':
			{
				util_buffer_init(200, 0);
				util_state_set(TUNE);
				util_buffer_output();
				util_state_set(IDLE);
				break;
			}

			// Have current controller track a reference
			case 'j':
			{
				util_buffer_init(200, 0);

        int ref_mA = 0;
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &ref_mA);
				motor_amps_set(ref_mA);

				util_state_set(TRACK);
				util_buffer_output();
				util_state_set(IDLE);

				break;
			}

			// Reads a trajectory length, followed by each angle, storing data. Returns trajectory length.
			case 'y':
			{
				unsigned int traj_len = position_load();

				sprintf(buffer,"\%d \r\n", traj_len);
				NU32_WriteUART1(buffer);

				break;
			}

			// Follow loaded trajectory
			case 'o':
			{
				// reset encoder angle to 0 deg
				encoder_reset();

				// read in trajectory length
        int traj_len = 0;
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &traj_len);

				// read in additional samples to story
        int additional_samples = 0;
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &additional_samples);

				// hold final trajectory position for N more samples
				hold_final_traj_pos(additional_samples, traj_len);

				// follow loaded trajectory
				int buffer_len = traj_len + additional_samples;
				util_buffer_init(buffer_len, 1);
				util_state_set(TRACK);
				util_buffer_output();
				util_state_set(IDLE);

				break;
			}

			// Read PID position gains from client
			case 'x':
			{
        float Kp = 0, Ki = 0, Kd = 0;
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kp);
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Ki);
        NU32_ReadUART1(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kd);
				position_gains_read(Kp,Ki,Kd);
				break;
			}

			// Write PID position gains to client
			case 'f':
			{
				position_gains_send();
				break;
			}

			// Handle q for quit. Later you may want to return to idle state here 
      case 'q':
      {
				util_state_set(IDLE);
        break;
      }

			// Turn on LED2 to indicate an error
      default:
      {
        NU32_LED2 = 0;
        break;
      }
    }
  }
  return 0;
}
