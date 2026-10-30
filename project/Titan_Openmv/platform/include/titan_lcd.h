/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_LCD_H
#define TITAN_LCD_H
#include <stdbool.h>
#include <stdint.h>

#define TITAN_LCD_WIDTH 800U
#define TITAN_LCD_HEIGHT 480U
#define TITAN_LCD_REFRESH 56U /* 30 MHz / (1024 * 525) = 55.80 Hz. */

/* VM-thread API. Failures are negative MicroPython/POSIX errno values. */
int titan_lcd_open(void);
int titan_lcd_close(void);
bool titan_lcd_is_open(void);
uint16_t *titan_lcd_draw_buffer(void);
int titan_lcd_present(void);
int titan_lcd_set_backlight(unsigned percent);
unsigned titan_lcd_get_backlight(void);
void titan_lcd_deinit_all(void);
bool titan_lcd_pin_owned(uint16_t pin);
bool titan_lcd_timer_owned(unsigned channel);
void titan_lcd_line_isr(void);

/* Stop the peripheral before MicroPython sweeps objects or resets UMA/GC. */
void titan_display_deinit_all(void);
#endif
