//
// Created by sahara on 03.12.22.
//

#include "../include/esp_display/raw_comm.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LCD_RESPONSE_NONE (-1)

void print_and_free_log(struct lcd_raw_log_s *log) {
    char *str = malloc(log->length + 1);
    memcpy(str, log->str, log->length);
    str[log->length] = 0;

    printf("%s\n", str);

    free(str);
    free(log->str);
    free(log);
}

void lcd_raw_init(int fd) {
    uint8_t msg = 0x00;
    write(fd, &msg, 1);
}

void lcd_raw_set_flags(int fd, uint8_t flags) {
    uint8_t msg[] = { 0x01, flags };
    write(fd, msg, 2);
}

void lcd_raw_clear(int fd) {
    uint8_t msg = 0x02;
    write(fd, &msg, 1);
}

void lcd_raw_set_cursor(int fd, int column, int row) {
    uint8_t msg[] = { 0x03, (uint8_t) column, (uint8_t) row };
    write(fd, msg, 3);
}

void lcd_raw_print(int fd, const char *str) {
    uint8_t len = strlen(str);
    uint8_t *msg = malloc(2 + len);

    msg[0] = 0x04;
    msg[1] = len;
    memcpy(msg + 2, str, len); // NOLINT(bugprone-not-null-terminated-result)

    write(fd, msg, len + 2);
}

struct lcd_raw_response_s *lcd_raw_await_response(int fd) {
    static int response_state = LCD_RESPONSE_NONE;
    uint8_t byte;

    while (1) {
        if (read(fd, &byte, 1) == 0) continue;

        if (byte <= LCD_RESPONSE_LOG) break;
    }

    int foo = byte;

    printf("%d\n", foo);

    struct lcd_raw_response_s *response;
    struct lcd_raw_error_s *error;
    struct lcd_raw_log_s *log;

    switch (byte) {
        case LCD_RESPONSE_READY:
        case LCD_RESPONSE_ACK:
            response = malloc(sizeof(struct lcd_raw_response_s));
            response->code = byte;
            return response;
        case LCD_RESPONSE_ERROR:
            error = malloc(sizeof(struct lcd_raw_error_s));
            error->code = byte;

            read(fd, &error->error_code, 1);
            
            return (struct lcd_raw_response_s *) error;
        case LCD_RESPONSE_LOG:
            log = malloc(sizeof(struct lcd_raw_log_s));
            log->code = byte;

            read(fd, &log->length, 1);
            
            log->str = malloc(log->length);
            read(fd, log->str, log->length);

            return (struct lcd_raw_response_s *) log;
    }

    return NULL;
}

struct lcd_raw_response_s *lcd_raw_await_response_logging(int fd) {
    struct lcd_raw_response_s *response;

    while (1) {
        response = lcd_raw_await_response(fd);
        printf("%d\n", response->code);

        if (response->code != LCD_RESPONSE_LOG) return response;

        print_and_free_log((struct lcd_raw_log_s *) response);
        continue;
    }
}
