#ifndef RA8_CAPTURE_H
#define RA8_CAPTURE_H
#include <stdint.h>
#include <stdbool.h>
struct framebuffer;
int ra8_capture_buffer_change(struct framebuffer *fb);
bool ra8_capture_pipeline(int enable);
int ra8_capture_reset_barrier(void);
#endif
