#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define RING_BUFFER_SIZE 1024  // Power of 2
#define RING_BUFFER_MASK (RING_BUFFER_SIZE - 1)

typedef struct {
    uint8_t *buffer;           // Pointer for DMA-coherent allocation
    volatile size_t head;      // Write pointer
    volatile size_t tail;      // Read pointer
    volatile size_t count;     // Element count
} RingBuffer;

void rb_init(RingBuffer *rb, uint8_t *buf) {
    rb->buffer = buf;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

static inline bool rb_is_full(const RingBuffer *rb) {
    return rb->count == RING_BUFFER_SIZE;
}

static inline bool rb_is_empty(const RingBuffer *rb) {
    return rb->count == 0;
}

static inline size_t rb_available_space(const RingBuffer *rb) {
    return RING_BUFFER_SIZE - rb->count;
}

static inline size_t rb_available_data(const RingBuffer *rb) {
    return rb->count;
}

bool rb_write(RingBuffer *rb, uint8_t data) {
    if (rb_is_full(rb)) return false;
    
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) & RING_BUFFER_MASK;  // Bitmask optimization
    rb->count++;
    return true;
}

bool rb_read(RingBuffer *rb, uint8_t *data) {
    if (rb_is_empty(rb)) return false;
    
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) & RING_BUFFER_MASK;
    rb->count--;
    return true;
}

// Bulk operations for DMA
size_t rb_write_bulk(RingBuffer *rb, const uint8_t *data, size_t len) {
    size_t available = rb_available_space(rb);
    size_t to_write = (len < available) ? len : available;
    
    // Handle wrap-around
    size_t first_chunk = RING_BUFFER_SIZE - rb->head;
    if (first_chunk >= to_write) {
        memcpy(&rb->buffer[rb->head], data, to_write);
    } else {
        memcpy(&rb->buffer[rb->head], data, first_chunk);
        memcpy(rb->buffer, &data[first_chunk], to_write - first_chunk);
    }
    
    rb->head = (rb->head + to_write) & RING_BUFFER_MASK;
    rb->count += to_write;
    return to_write;
}

size_t rb_read_bulk(RingBuffer *rb, uint8_t *data, size_t len) {
    size_t available = rb_available_data(rb);
    size_t to_read = (len < available) ? len : available;
    
    // Handle wrap-around
    size_t first_chunk = RING_BUFFER_SIZE - rb->tail;
    if (first_chunk >= to_read) {
        memcpy(data, &rb->buffer[rb->tail], to_read);
    } else {
        memcpy(data, &rb->buffer[rb->tail], first_chunk);
        memcpy(&data[first_chunk], rb->buffer, to_read - first_chunk);
    }
    
    rb->tail = (rb->tail + to_read) & RING_BUFFER_MASK;
    rb->count -= to_read;
    return to_read;
}
