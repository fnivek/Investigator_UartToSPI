#include <msp430.h> 

/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

	// Set up master clk
	DCOCTL |= DCO2 | DCO1 | DCO0;
	BCSCTL1 |= RSEL3 | RSEL2 | RSEL1 | RSEL0;
	
	return 0;
}
