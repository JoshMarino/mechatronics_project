#include "NU32.h"         // config bits, constants, funcs for startup and UART
#include "encoder.h" 			// Includer header module
#include "utilities.h" 		// Includer header module


#define BUF_SIZE 200

int main() 
{
  char buffer[BUF_SIZE];

  NU32_Startup(); 			// cache on, min flash wait, interrupts on, LED/button init, UART init

  NU32_LED1 = 1;        // turn off the LEDs
  NU32_LED2 = 1;        

  __builtin_disable_interrupts();
  // initialize other modules here
	encoder_init()										// inizalizes the SPI peripheral
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
				sprintf(buffer,"\%d", encoder_reset());
				//NU32_WriteUART1(buffer);
				break;
			}

			// Read encoder ticks
			case 'r':
			{
				sprintf(buffer,"\%d", encoder_ticks());
				NU32_WriteUART1(buffer);
				break;
			}

			// Return encoder angle
			case 'a':
			{
				sprintf(buffer,"\%d", encoder_angle());
				NU32_WriteUART1(buffer);
				break;
			}

			// Query the current state
			case 's':
			{
				sprintf(buffer,"\%d", util_state_get());
				NU32_WriteUART1(buffer);
				break;
			}

			// Handle q for quit. Later you may want to return to idle state here 
      case 'q':
      {
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
