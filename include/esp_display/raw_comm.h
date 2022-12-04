//
// Created by sahara on 03.12.22.
//

#ifndef LIBESPDISPLAY_RAW_COMM_H
#define LIBESPDISPLAY_RAW_COMM_H

#include <stdint.h>

#include <unistd.h>

#define LCD_RESPONSE_READY 0
#define LCD_RESPONSE_ACK 1
#define LCD_RESPONSE_ERROR 2
#define LCD_RESPONSE_LOG 3

struct lcd_raw_response_s {
    uint8_t code;
};

struct lcd_raw_error_s {
    uint8_t code;
    uint8_t error_code;
};

struct lcd_raw_log_s {
    uint8_t code;
    uint16_t length;
    char *str;
};

void lcd_raw_init(int fd);
void lcd_raw_set_flags(int fd, uint8_t flags);
void lcd_raw_clear(int fd);
void lcd_raw_set_cursor(int fd, int column, int row);
void lcd_raw_print(int fd, const char *str);

struct lcd_raw_response_s *lcd_raw_await_response(int fd);
struct lcd_raw_response_s *lcd_raw_await_response_logging(int fd);

#endif //LIBESPDISPLAY_RAW_COMM_H
