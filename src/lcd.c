//
// Created by sahara on 03.12.22.
//

#include "../include/esp_display/lcd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include "../include/esp_display/raw_comm.h"

#define LCD_BACKLIGHT 0x01
#define LCD_CURSOR 0x02
#define LCD_BLINK 0x04

struct lcd_display_s {
    char *device;
    int fd;
    uint8_t flags;
    int rows;
    int columns;
    int x;
    int y;
    char *content;
};

static int init_serial_port(const char *device) {
    int fd = open(device, O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "Error %i opening %s: %s\n", errno, device, strerror(errno));
        return -1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty)) {
        fprintf(stderr, "Error %i getting serial settings: %s\n", errno, strerror(errno));
        return -1;
    }

    tty.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
    tty.c_cflag |= CS8 | CREAD | CLOCAL;
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~(OPOST | ONLCR);
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 0;

    cfsetspeed(&tty, B115200);

    if (tcsetattr(fd, TCSANOW, &tty)) {
        fprintf(stderr, "Error %i setting serial settings: %s\n", errno, strerror(errno));
        return -1;
    }

    return fd;
}

static int await_ack_or_error(int fd) {
    struct lcd_raw_response_s *response = lcd_raw_await_response_logging(fd);
    int ret = response->code == LCD_RESPONSE_ACK ? 0 : 1;

    free(response);

    return ret;
}

lcd_display_p lcd_connect(const char *device, int columns, int rows) {
    lcd_display_p lcd = malloc(sizeof(struct lcd_display_s));

    size_t device_len = strlen(device);
    lcd->device = malloc(device_len + 1);
    memcpy(lcd->device, device, device_len + 1);

    if ((lcd->fd = init_serial_port(lcd->device)) < 0) {
        goto error;
    }

    lcd_raw_init(lcd->fd);
    lcd_raw_await_response_logging(lcd->fd);

    lcd->columns = columns;
    lcd->rows = rows;
    lcd->flags = 0x00;
    lcd->x = 0;
    lcd->y = 0;
    lcd->content = NULL;

    return lcd;

error:
    if (lcd->fd > 0) {
        close(lcd->fd);
    }

    free(lcd->device);
    free(lcd);

    return NULL;
}

int lcd_disconnect(lcd_display_p lcd) {
    if (lcd->fd > 0) close(lcd->fd);

    if (lcd->content != NULL) free(lcd->content);
    free(lcd->device);
    free(lcd);

    return 0;
}

int lcd_set_backlight(lcd_display_p lcd, bool backlight) {
    if (backlight) {
        lcd->flags |= LCD_BACKLIGHT;
    } else {
        lcd->flags &= ~LCD_BACKLIGHT;
    }

    lcd_raw_set_flags(lcd->fd, lcd->flags);
    return await_ack_or_error(lcd->fd);
}

int lcd_set_blink(lcd_display_p lcd, bool blink) {
    if (blink) {
        lcd->flags |= LCD_BLINK;
    } else {
        lcd->flags &= ~LCD_BLINK;
    }

    lcd_raw_set_flags(lcd->fd, lcd->flags);
    return await_ack_or_error(lcd->fd);
}

int lcd_set_cursor(lcd_display_p lcd, bool cursor) {
    if (cursor) {
        lcd->flags |= LCD_CURSOR;
    } else {
        lcd->flags &= ~LCD_CURSOR;
    }

    lcd_raw_set_flags(lcd->fd, lcd->flags);
    return await_ack_or_error(lcd->fd);
}

int lcd_clear(lcd_display_p lcd) {
    lcd_raw_clear(lcd->fd);

    int resp = await_ack_or_error(lcd->fd);

    if (resp != 0) return resp;

    lcd->x = 0;
    lcd->y = 0;

    return 0;
}

int lcd_set_cursor_position(lcd_display_p lcd, int column, int row) {
    if ((column < 0) || (row < 0)) return 1;
    if ((column >= lcd->columns) || (row >= lcd->rows)) return 1;

    lcd_raw_set_cursor(lcd->fd, column, row);

    int resp = await_ack_or_error(lcd->fd);

    if (resp != 0) return resp;

    lcd->x = column;
    lcd->y = row;

    return 0;
}

int lcd_print(lcd_display_p lcd, const char *str) {
    lcd_raw_print(lcd->fd, str);

    return await_ack_or_error(lcd->fd);
}
