#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#define BUFFER_SIZE 5

typedef struct {
    int buffer[BUFFER_SIZE];
    size_t head;
    size_t tail;
    sem_t empty;
    sem_t full;
    sem_t mutex;
} RingBuffer;

void ring_buffer_init(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
    sem_init(&rb->empty, 0, BUFFER_SIZE);
    sem_init(&rb->full, 0, 0);
    sem_init(&rb->mutex, 0, 1);
}

void ring_buffer_free(RingBuffer *rb) {
    sem_destroy(&rb->empty);
    sem_destroy(&rb->full);
    sem_destroy(&rb->mutex);
}

void ring_buffer_write(RingBuffer *rb, int data) {
    sem_wait(&rb->empty);  // Wait for an empty slot
    sem_wait(&rb->mutex);  // Lock the buffer

    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % BUFFER_SIZE;

    sem_post(&rb->mutex);  // Unlock the buffer
    sem_post(&rb->full);   // Signal a filled slot
}

int ring_buffer_read(RingBuffer *rb) {
    sem_wait(&rb->full);   // Wait for a filled slot
    sem_wait(&rb->mutex);  // Lock the buffer

    int data = rb->buffer[rb->tail];
    printf("tail: %d\n", rb->tail);
    rb->tail = (rb->tail + 1) % BUFFER_SIZE;

    sem_post(&rb->mutex);  // Unlock the buffer
    sem_post(&rb->empty);  // Signal an empty slot

    return data;
}

void *producer(void *param) {
    RingBuffer *rb = (RingBuffer *)param;
    for (int i = 0; i < 10; i++) {
        ring_buffer_write(rb, i);
        printf("Produced: %d\n", i);
    }
    return NULL;
}

void *consumer(void *param) {
    RingBuffer *rb = (RingBuffer *)param;
    for (int i = 0; i < 10; i++) {
        int data = ring_buffer_read(rb);
        printf("Consumed: %d\n", data);
    }
    return NULL;
}

int main() {
    RingBuffer rb;
    ring_buffer_init(&rb);

    pthread_t producer_thread, consumer_thread;
    pthread_create(&producer_thread, NULL, producer, (void *)&rb);
    pthread_create(&consumer_thread, NULL, consumer, (void *)&rb);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    ring_buffer_free(&rb);
    return 0;
}
