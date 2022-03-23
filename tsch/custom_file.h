
#ifndef __TSCH_CUSTOM_H__
#define __TSCH_CUSTOM_H__


/********** Includes **********/

#include "contiki.h"
// #include "sys/rtimer.h"

/******** Configuration *******/


/************ Types ***********/

/* \brief Structure for a log. Union of different types of logs */
typedef struct {
    uint8_t custom_is_data;
    uint8_t custom_seqno;
    uint8_t custom_retransmissions;
    uint8_t custom_time_slot;
    linkaddr_t custom_addr;
} packet_sts;

// A structure to represent a queue
typedef struct
{
    int front;
    int rear; 
    int size;
    unsigned capacity;
    packet_sts list[20];
} my_custom_queue;

//typedef struct Queue my_custom_queue;

/********** Functions *********/

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
//my_custom_queue *createQueue(unsigned capacity);


// Queue is full when size becomes
// equal to the capacity
int isFull(my_custom_queue *queue);


// Queue is empty when size is 0
int isEmpty(my_custom_queue *queue);


// Function to add an item to the queue.
// It changes rear and size
void enqueue(my_custom_queue *queue, packet_sts item);


// Function to remove an item from queue.
// It changes front and size
packet_sts dequeue(my_custom_queue *queue);

// Function to get front of queue
packet_sts front(my_custom_queue *queue);


// Function to get rear of queue
packet_sts rear(my_custom_queue *queue);

/************ Macros **********/

#endif /* __TSCH_CUSTOM_H__ */
