/*
 * queue.c
 *
 *  Created on: Oct 18, 2015
 *      Author: Kevin French
 */

#include "queue.h"

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
