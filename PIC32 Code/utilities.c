#include "NU32.h"         // config bits, constants, funcs for startup and UART
#include "utilities.h"


static volatile States state = IDLE;


// Get the current state
States util_state_get() {

	return state;

}


// Atomically set the current state
void util_state_set(States s) {

	state = s;

}




/*
#include <stdio.h>

// the new data type days_t defined below has 7 possible entries, corresponding to integers 0 to 6
typedef enum {Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday} days_t;

days_t day; // the variable day is of type days_t, which means its value can be Monday or Tuesday or ...

int main(void) {
	day = Monday;							// the variable day now has the numerical value 1, since Sunday is 0, Monday is 1, etc.
	printf("%d\n",day);				// you can see the value of day is 1 by printing day to the screen as an int
	return 0;
}
*/
