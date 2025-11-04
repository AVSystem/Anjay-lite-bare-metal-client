/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay Lite LwM2M SDK
 * All rights reserved.
 *
 * Licensed under AVSystem Anjay Lite LwM2M Client SDK - Non-Commercial License.
 * See the attached LICENSE file for details.
 */

#ifndef CIRC_BUF_H
#define CIRC_BUF_H

#include <stddef.h>
#include <stdint.h>

// NOTE: this implementation is suitable only for single-producer,
// single-consumer scenarios, i.e. only one end may write to the buffer, and
// only one end may read from the buffer at a time.
typedef struct {
    volatile size_t begin;
    volatile size_t end;
    size_t len;
    volatile uint8_t *const storage;
} circ_buf_t;

static inline size_t circ_buf_avail(circ_buf_t *buf) {
    size_t res = buf->begin <= buf->end ? buf->end - buf->begin
                                        : buf->len - buf->begin + buf->end;
    return res;
}

static inline size_t circ_buf_free(circ_buf_t *buf) {
    return buf->len - circ_buf_avail(buf) - 1;
}

static inline void circ_buf_push(circ_buf_t *buf, uint8_t to_push) {
    buf->storage[buf->end] = to_push;
    buf->end = (buf->end + 1) % buf->len;
}

static inline uint8_t circ_buf_pop(circ_buf_t *buf) {
    uint8_t res = buf->storage[buf->begin];
    buf->begin = (buf->begin + 1) % buf->len;
    return res;
}

static inline uint8_t circ_buf_seek(circ_buf_t *buf, size_t at) {
    uint8_t res = buf->storage[(buf->begin + at) % buf->len];
    return res;
}

static inline void circ_buf_advance_n(circ_buf_t *buf, size_t n) {
    buf->begin = (buf->begin + n) % buf->len;
}

static inline void circ_buf_advance(circ_buf_t *buf) {
    circ_buf_advance_n(buf, 1);
}

static inline void circ_buf_flush(circ_buf_t *buf) {
    buf->begin = 0;
    buf->end = 0;
}

static inline int
circ_buf_append(circ_buf_t *buf, const uint8_t *to_append, size_t len) {
    if (len > circ_buf_free(buf)) {
        return -1;
    }
    for (size_t i = 0; i < len; i++) {
        circ_buf_push(buf, to_append[i]);
    }
    return 0;
}

#endif // CIRC_BUF_H
