#include "encoder.h"
#include <xc.h>

/*----- Sends a command to quadrature chip and returns a response -----
  ----- Commands: 0x01 - reads tick count || 0x00 - reset encoder -----*/
static int encoder_command(int read) { // send  a command to the encoder chip
  SPI4BUF = read; 		 // send the command

  while (!SPI4STATbits.SPIRBF) { ; } // wait for the response
  
  SPI4BUF;             // garbage was transfered over, ignore it
  SPI4BUF = 5;         // write garbage, but the corresponding read will have the data
 
  while (!SPI4STATbits.SPIRBF) { ; }

  return SPI4BUF;
}


/*----- Initialize SPI peripheral -----*/
void encoder_init(void) {
  // SPI initialization for reading from the encoder chip
  SPI4CON = 0; 		          // stop and reset SPI4
  SPI4BUF;		              // read to clear the rx buffer
  SPI4BRG = 0x4; 	          // bit rate to 8MHz, SPI4BRG = 80000000/(2*desired)-1
  SPI4STATbits.SPIROV = 0;  // clear the overflow
  SPI4CONbits.MSTEN = 1;    // master mode
  SPI4CONbits.MSSEN = 1;    // Slave select enable
  SPI4CONbits.MODE16 = 1;   // 16 bit mode
  SPI4CONbits.MODE32 = 0; 
  SPI4CONbits.SMP = 1;      // sample at the end of the clock
  SPI4CONbits.ON = 1;       // turn spi on
}


/*----- Read encoder tick count -----*/
int encoder_ticks(void) {
  encoder_command(1);	        // we need to read twice to get a valid reading
  return encoder_command(1);
}


/*----- Reset encoder value by sending a command of 0x00 -----*/
void encoder_reset(void) {
	encoder_command(0);
}


/*------ Read tick count from quadrature chip and return the angle -----*/
int encoder_angle(void) {

	int angle, ticks;

	ticks = encoder_ticks();

	// Convert tick counts to angle: 4 ticks for each line, 334 lines
	angle = ticks*4*334;

	return angle;
}
