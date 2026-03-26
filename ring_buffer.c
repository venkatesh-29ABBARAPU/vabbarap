#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 1024

typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    size_t head;   // write pointer
    size_t tail;   // read pointer
    size_t count;  // number of elements currently stored
} RingBuffer;

// Initialize buffer
void rb_init(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

// Check if buffer is full
bool rb_is_full(RingBuffer *rb) {
    return rb->count == RING_BUFFER_SIZE;
}

// Check if buffer is empty
bool rb_is_empty(RingBuffer *rb) {
    return rb->count == 0;
}

// Write one byte into buffer
bool rb_write(RingBuffer *rb, uint8_t data) {
    if (rb_is_full(rb)) {
        return false; // no space
    }
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % RING_BUFFER_SIZE;
    rb->count++;
    return true;
}

// Read one byte from buffer
bool rb_read(RingBuffer *rb, uint8_t *data) {
    if (rb_is_empty(rb)) {
        return false; // nothing to read
    }
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    rb->count--;
    return true;
}

// Example usage
int main() {
    RingBuffer rb;
    rb_init(&rb);

    // Write some data
    for (int i = 0; i < 10; i++) {
        rb_write(&rb, i);
    }

    // Read back
    uint8_t val;
    while (rb_read(&rb, &val)) {
        printf("Read: %d\n", val);
    }

    return 0;
}
