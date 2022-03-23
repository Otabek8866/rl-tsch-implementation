#include "dev/radio.h"
#include "contiki.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"
#include "net/mac/framer/framer-802154.h"
#include "net/mac/tsch/tsch.h"
#include "sys/critical.h"

#include "sys/log.h"

// C program for array implementation of queue
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>


// typedef struct {
//     uint8_t custom_is_data;
//     uint8_t custom_seqno;
//     uint8_t custom_retransmissions;
//     uint8_t custom_time_slot;
//     linkaddr_t custom_addr;
// } packet_sts;

// // A structure to represent a queue
// struct Queue
// {
//     int front, rear, size;
//     unsigned capacity;
//     packet_sts *list;
// };

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
// my_custom_queue *createQueue(unsigned capacity)
// {
//     my_custom_queue *queue;// {.front = 0, .rear = capacity - 1, .size = 0, .capacity = capacity};
//     // struct Queue *queue = &newqueue;
//     //*queue = {0, capacity - 1, 0, capacity};
//     queue->capacity = capacity;
//     queue->front = queue->size = 0;

//     // This is important, see the enqueue
//     queue->rear = capacity - 1;
//     //queue->list = (packet_sts *)malloc(queue->capacity * 27);//sizeof(packet_sts));
//     return queue;
// }

// Queue is full when size becomes
// equal to the capacity
int isFull(my_custom_queue *queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(my_custom_queue *queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(my_custom_queue *queue, packet_sts item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->list[queue->rear] = item;
    queue->size = queue->size + 1;
    //printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue.
// It changes front and size
packet_sts dequeue(my_custom_queue *queue)
{
    // if (isEmpty(queue)){
    //     packet_sts p = NULL;
    //     return p;
    // }
    packet_sts item = queue->list[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
packet_sts front(my_custom_queue *queue)
{
    // if (isEmpty(queue)){
    //     packet_sts p = NULL;
    //     return p;
    // }
    return queue->list[queue->front];
}

// Function to get rear of queue
packet_sts rear(my_custom_queue *queue)
{
    // if (isEmpty(queue)){
    //     packet_sts p = NULL;
    //     return p;
    // }
    return queue->list[queue->rear];
}

// // Driver program to test above functions./
// int main()
// {
//     struct Queue *queue = createQueue(1000);

//     enqueue(queue, 10);
//     enqueue(queue, 20);
//     enqueue(queue, 30);
//     enqueue(queue, 40);

//     printf("%d dequeued from queue\n\n",
//            dequeue(queue));

//     printf("Front item is %d\n", front(queue));
//     printf("Rear item is %d\n", rear(queue));

//     return 0;
// }