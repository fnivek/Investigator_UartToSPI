#include <msp430.h>
#include <stdint.h>

#define RX BIT1
#define TX BIT2

// Transfer one byte over SPI
void SPITransmit(uint8_t out) {
	// Place holder
}

// Transfer one byte over UART
void UARTTransmit(uint8_t out) {
	// Wait for transmit to be ready
	while (!(IFG2 & UCA0TXIFG));

	UCA0TXBUF = out;
	__delay_cycles(100000);
}

// Initilize UART on UCA0
void InitUART() {
	// Put in reset
	UCA0CTL1 = UCSWRST;

	// Parity disabled, LSB first, 8 bit char, one stop bit, UART mode, asynchronous
	UCA0CTL0 = 0;
	// Source from SMCLK
	UCA0CTL1 |= UCSSEL_3;
	// Baud rate of 9600
	// BRCLK= 20.304MHz UCBRx = 132, UCBRSx = 0 UCBRFx = 3, oversample mode
	UCA0BR0 = 132;
	UCA0BR1 = 0;
	UCA0MCTL = UCBRF_3 | UCBRS_0 | UCOS16;

	// Setup GPIO
	P1SEL = TX | RX;
	P1SEL2 = TX | RX;
	P1DIR |= TX;
	P1DIR &= ~RX;

	// Enable
	UCA0CTL1 &= ~UCSWRST;


}

/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

	// Set up master clk
	DCOCTL |= DCO2 | DCO1 | DCO0;
	BCSCTL1 |= RSEL3 | RSEL2 | RSEL1 | RSEL0;

	// Initilize UART
	InitUART();

	while(!(IFG2 & UCA0RXIFG));

	uint8_t echo = UCA0RXBUF;

	while(1) {
		UARTTransmit(echo);
		while(!(IFG2 & UCA0RXIFG));
		echo = UCA0RXBUF;
	}

	return 0;
}
