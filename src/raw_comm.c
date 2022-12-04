//
// Created by sahara on 03.12.22.
//

#include "../include/esp_display/raw_comm.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LCD_RESPONSE_NONE (-1)

uint8_t await_byte(int fd) {
    uint8_t byte;

    while (read(fd, &byte, 1) == 0);

    return byte;
}

void await_read(int fd, char *dest, int n) {
    while (n-- > 0) {
        *(dest++) = await_byte(fd);
    }
}

void print_and_free_log(struct lcd_raw_log_s *log) {
    char *str = malloc(log->length + 1);
    memcpy(str, log->str, log->length);
    str[log->length] = 0;

    printf("Log: %s\n", str);

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
        byte = await_byte(fd);

        if (byte <= LCD_RESPONSE_LOG) break;

        putchar(byte);
    }

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
            error->error_code = await_byte(fd);
            
            return (struct lcd_raw_response_s *) error;
        case LCD_RESPONSE_LOG:
            log = malloc(sizeof(struct lcd_raw_log_s));

            log->code = byte;
            log->length = await_byte(fd);
            
            log->str = malloc(log->length);
            await_read(fd, log->str, log->length);

            return (struct lcd_raw_response_s *) log;
    }

    return NULL;
}

struct lcd_raw_response_s *lcd_raw_await_response_logging(int fd) {
    struct lcd_raw_response_s *response;

    while (1) {
        response = lcd_raw_await_response(fd);

        if (response->code != LCD_RESPONSE_LOG) return response;

        print_and_free_log((struct lcd_raw_log_s *) response);
        continue;
    }
}
