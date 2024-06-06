#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int *buffer;
    size_t size;
    size_t head;
    size_t tail;
    bool full;
} RingBuffer;

// Initialize the ring buffer
void ring_buffer_init(RingBuffer *rb, size_t size) {
    rb->buffer = (int *)malloc(size * sizeof(int));
    printf("adress of buffer: %p\n", rb->buffer);
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->full = false;
}

// Free the ring buffer
void ring_buffer_free(RingBuffer *rb) {
    free(rb->buffer);
}

// Check if the ring buffer is empty
bool ring_buffer_empty(RingBuffer *rb) {
    return (!rb->full && (rb->head == rb->tail));
}

// Check if the ring buffer is full
bool ring_buffer_full(RingBuffer *rb) {
    return rb->full;
}

// Advance the pointer and wrap around if necessary
static void advance_pointer(RingBuffer *rb) {
    if (rb->full) {
        rb->tail = (rb->tail + 1) % rb->size;
    }
    rb->head = (rb->head + 1) % rb->size;
    rb->full = (rb->head == rb->tail);
}

// Retreat the pointer and wrap around if necessary
static void retreat_pointer(RingBuffer *rb) {
    rb->full = false;
    printf("tail old: %d\n", rb->tail);
    rb->tail = (rb->tail + 1) % rb->size;
    printf("tail new: %d\n", rb->tail);
}

// Write data to the ring buffer
void ring_buffer_write(RingBuffer *rb, int data) {
    rb->buffer[rb->head] = data;
    advance_pointer(rb);
}

// Read data from the ring buffer
int ring_buffer_read(RingBuffer *rb) {
    if (ring_buffer_empty(rb)) {
        // Buffer is empty, return an error value or handle the underflow
        fprintf(stderr, "Buffer is empty!\n");
        return -1; // Error value
    }
    
    int data = rb->buffer[rb->tail];
    retreat_pointer(rb);
    return data;
}

// Test the ring buffer
int main() {
    RingBuffer rb;
    ring_buffer_init(&rb, 5); // Initialize ring buffer with size 5
    
    // Write data to the ring buffer
    ring_buffer_write(&rb, 1);
    ring_buffer_write(&rb, 2);
    ring_buffer_write(&rb, 3);
    ring_buffer_write(&rb, 4);
    ring_buffer_write(&rb, 5);

    // Try writing to a full buffer
    ring_buffer_write(&rb, 6); // This will overwrite the first element (1)
    
    // Read data from the ring buffer
    while (!ring_buffer_empty(&rb)) {
        int data = ring_buffer_read(&rb);
        printf("%d\n", data);
    }
    
    // Cleanup
    ring_buffer_free(&rb);
    
    return 0;
}