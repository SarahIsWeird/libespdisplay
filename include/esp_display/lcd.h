//
// Created by sahara on 03.12.22.
//

#ifndef LIBESPDISPLAY_LCD_H
#define LIBESPDISPLAY_LCD_H

#include <stdint.h>
#include <stdbool.h>

struct lcd_display_s;
typedef struct lcd_display_s *lcd_display_p;

lcd_display_p lcd_connect(const char *device, int columns, int rows);
int lcd_disconnect(lcd_display_p lcd);

int lcd_set_backlight(lcd_display_p lcd, bool backlight);
int lcd_set_blink(lcd_display_p lcd, bool blink);
int lcd_set_cursor(lcd_display_p lcd, bool cursor);

int lcd_clear(lcd_display_p lcd);
int lcd_set_cursor_position(lcd_display_p lcd, int column, int row);
int lcd_print(lcd_display_p lcd, const char *str);

#endif //LIBESPDISPLAY_LCD_H
