#include <msp430.h>
#include <stdint.h>
#include <limits.h>

#define RX BIT1
#define TX BIT2
// Must be less than max(unsigned int)
#define MAX_TX_BUF_SIZE 100

typedef struct queue {
	uint8_t elements[MAX_TX_BUF_SIZE];
	unsigned int front;
	unsigned int count;
} queue;

// Pops an element off a queue and places its value in value
// returns 1 for success and 0 for underflow
uint8_t QueuePop(queue* q, uint8_t* value) {
	// Check for underflow
	if (!q->count) return 0;

	// Pop the value off
	--q->count;
	(*value) = q->elements[q->front--];

	// Wrap front if necisary
	if (q->front >= MAX_TX_BUF_SIZE) {
		q->front = MAX_TX_BUF_SIZE - 1;
	}

	// Success
	return 1;
}

// Return the current size of the queue
uint8_t QueueSize(queue *q) {
	return q->count;
}

// Insert a new value into the queue
// Return 1 for success 0 for QueueFull
uint8_t QueueInsert(queue* q, uint8_t value) {
	// Check if full
	if(q->count >= MAX_TX_BUF_SIZE) return 0;

	unsigned int offset = q->count + q->front;
	// Check if need to wrap to front
	if(offset >= MAX_TX_BUF_SIZE)
	{
		offset -= MAX_TX_BUF_SIZE;
	}
	else if(offset < q->front && MAX_TX_BUF_SIZE < UINT_MAX)
	{
		offset += UINT_MAX - MAX_TX_BUF_SIZE;
	}

	q->elements[offset] = value;
	++q->count;
	return 1;
}

queue UARTTXBuf;
queue SPITXBuf;

// Transfer one byte over SPI
void SPITransmit(uint8_t out) {
	if (!QueueInsert(&SPITXBuf, out)) {
		// Handle Queue overflow
		__disable_interrupt();
		for (;;); // This is here to intentionally break the code
	}

	// Turn the transmit interrupt back on
	if(QueueSize(&SPITXBuf) == 1) {
		IE2 |= UCB0TXIE;
	}
}

// Transfer one byte over UART
void UARTTransmit(uint8_t out) {
	if (!QueueInsert(&UARTTXBuf, out)) {
		// Handle Queue overflow
		__disable_interrupt();
		for (;;); // This is here to intentionally break the code
	}

	// Turn the transmit interrupt back on
	if(QueueSize(&UARTTXBuf) == 1) {
		IE2 |= UCA0TXIE;
	}
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

// Initilize SPI Master on UCB0
// SPI P1.6 MISO, P1.7 MOSI, P1.5 CLK
// Uses UARTx on pins ...
void InitSPI() {
	// Disable USCI
	UCB0CTL1 |= UCSWRST;

	// Set up pin functions and Directions
	P1SEL |= BIT5| BIT6 | BIT7;
	P1SEL2 |= BIT5| BIT6 | BIT7;
	P1DIR |= BIT5 | BIT7;			// P1.5, P1.7 Out
	P1DIR &= ~BIT6;					// P1.6 In

	// Set mode and clock select
	UCB0CTL0 = UCMSB | UCMST | UCSYNC;
	UCB0CTL1 = UCSSEL_3 | UCSWRST;

	// Set clock divider to 255
	UCB0BR0 = 0xFF;
	UCB0BR1 = 0x0;

	// Turn on USCI in SPI mode
	UCB0CTL1 &= ~UCSWRST;
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void TXISR() {

	// UART
	uint8_t out = 0;
	if(QueuePop(&UARTTXBuf, &out)) {
		UCA0TXBUF = out;
	}
	else {
		// Done transfering disable transmit interrupt
		IE2 &= ~UCA0TXIE;
	}

	// SPI
	out = 0;
	if(QueuePop(&SPITXBuf, &out)) {
		UCB0TXBUF = out;
	}
	else {
		// Done transfering disable transmit interrupt
		IE2 &= ~UCB0TXIE;
	}

}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void RXISR() {
	// UART
	if(IFG2 & UCB0RXIFG) {
		UARTTransmit(UCB0RXBUF);
	}

	// SPI
	if (IFG2 & UCA0RXIFG) {
		SPITransmit(UCA0RXBUF);
	}

}

/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

	// Set up master clk
	DCOCTL |= DCO2 | DCO1 | DCO0;
	BCSCTL1 |= RSEL3 | RSEL2 | RSEL1 | RSEL0;

	// Initilize communications
	InitUART();
	InitSPI();

	// Enable interrupts
	__enable_interrupt();
	IE2 = UCA0TXIE | UCA0RXIE | UCB0TXIE | UCB0RXIE;

	//while(!(IFG2 & UCA0RXIFG));

	//uint8_t echo = UCA0RXBUF;

	while(1) {
		//UARTTransmit(echo);
		//while(!(IFG2 & UCA0RXIFG));
		//echo = UCA0RXBUF;
	}

	return 0;
}
