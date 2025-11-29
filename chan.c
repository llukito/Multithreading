#include "chan.h"

#include <stdlib.h>

// typedef struct
// {  
//     int buffer_size;
//     Queue *queue
//     pthread_mutex_t lock;
//     sem_t read, write;
// } chan_t; // (same as blocking queue)

void chanInit(chan_t *chan, int elem_size, int buffer_size)
{
    chan->queue = malloc(sizeof(Queue));
    queueInit(chan->queue, elem_size);

    sem_init(&chan->read, 0, 0);
    sem_init(&chan->write, 0, buffer_size);
    pthread_mutex_init(&chan->lock, NULL);
    if(buffer_size == 0){
        chan->is_buffered = false;
    }
}

void chanSend(chan_t *chan, void *elem)
{   
    if(chan->is_buffered)
   sem_wait(&chan->write);

   pthread_mutex_lock(&chan->lock);
    enQueue(chan->queue, elem);
   pthread_mutex_unlock(&chan->lock);

   sem_post(&chan->read);
   if(!chan->is_buffered)
    sem_wait(&chan->write);
}

void chanRecv(chan_t *chan, void *elem)
{
    sem_wait(&chan->read);

   pthread_mutex_lock(&chan->lock);
    deQueue(chan->queue, elem);
   pthread_mutex_unlock(&chan->lock);

   sem_post(&chan->write);
}

void chanDestroy(chan_t *chan)
{
   queueDestroy(chan->queue);
   pthread_mutex_destroy(&chan->lock);

   sem_destroy(&chan->read);
   sem_destroy(&chan->write);
}