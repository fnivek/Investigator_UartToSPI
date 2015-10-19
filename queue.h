/*
 * queue
 *
 *  Created on: Oct 18, 2015
 *      Author: Kevin French
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <msp430.h>
#include <stdint.h>
#include <limits.h>

// Must be less than max(unsigned int)
#define MAX_TX_BUF_SIZE 100

typedef struct queue {
	uint8_t elements[MAX_TX_BUF_SIZE];
	unsigned int front;
	unsigned int count;
} queue;

// Pops an element off a queue and places its value in value
// returns 1 for success and 0 for underflow
uint8_t QueuePop(queue* q, uint8_t* value);

// Return the current size of the queue
uint8_t QueueSize(queue *q);

// Insert a new value into the queue
// Return 1 for success 0 for QueueFull
uint8_t QueueInsert(queue* q, uint8_t value);


#endif /* QUEUE_H_ */
