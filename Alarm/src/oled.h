#ifndef OLED_H
#define OLED_H

#include <stdint.h>

void oled_init();
void oled_clear();
void oled_display_text(const char *text, uint8_t x, uint8_t y);
void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void oled_draw_bitmap(const uint8_t *bitmap);

#endif // OLED_H
